/*
* Copyright (C) 2013 Nivis LLC.
* Email:   opensource@nivis.com
* Website: http://www.nivis.com
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, version 3 of the License.
* 
* Redistribution and use in source and binary forms must retain this
* copyright notice.

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

/*
 * $Id: SqldalUnmarshaller.cpp,v 1.44.12.1 2013/05/15 19:19:18 owlman Exp $
 */

#include "SqldalUnmarshaller.h"
#include "../../../WCI/scrsrv/Csv.h"
#include <sqlite3.h>

CSqldalUnmarshaller* CSqldalUnmarshaller::m_pInstance =NULL;

static int csvPrinter(void*p, IO& io, bool first)
{
	sqlite3_stmt *stmt = (sqlite3_stmt*)p ;
	const unsigned int numColumns = sqlite3_column_count(stmt);
	CCsv csv ;
	for (unsigned int i = 0; i < numColumns; i++)
	{
		switch (sqlite3_column_type(stmt, i))
		{
		case SQLITE_INTEGER:
			csv.Put((uint64_t)sqlite3_column_int64(stmt,i));
			break;
		case SQLITE_FLOAT:
		    char floatVal[50];
		    snprintf( floatVal, 50, "%f", sqlite3_column_double(stmt,i) );
			csv.Put(floatVal, strlen(floatVal));
			break;
		case SQLITE_BLOB:
			csv.Put( (const char*)sqlite3_column_blob(stmt, i));
		case SQLITE_TEXT:
			csv.Put( (const char*)sqlite3_column_text(stmt,i));
			break;
		case SQLITE_NULL:
			csv.Put( "\"NULL\"" );
			break;
		default:
			break;
		}
	}
    LOG("]\r\n");
	fprintf( io.output, "%s\r\n", csv.GetLine() ) ;
	csv.Reset() ;
	return 0;
}

static int jsonCallback(void* p, IO& io, bool first)
{
	sqlite3_stmt *stmt = (sqlite3_stmt*)p ;
	const unsigned int numColumns = sqlite3_column_count(stmt);
	if ( first )
		fputs( " \"result\" : [ [", io.output ) ;
	else
		fputs(",[", io.output) ;

	for (unsigned int i = 0; i < numColumns; i++)
	{
		const char* comma=",";
		if ( i==(numColumns-1) ) comma="" ;

		switch (sqlite3_column_type(stmt, i))
		{
		case SQLITE_INTEGER:
			fprintf( io.output, "%lli%s", (int64_t)sqlite3_column_int64(stmt,i) , comma );
			break;
		case SQLITE_FLOAT:
			fprintf( io.output, "%f%s", (double)sqlite3_column_double(stmt,i) , comma );
			break;
		case SQLITE_BLOB:
			fprintf( io.output, "\"%s\"%s", (const char*)sqlite3_column_blob(stmt, i) , comma );
		case SQLITE_TEXT:
		{
			struct json_object* str = json_object_new_string((const char*)sqlite3_column_text(stmt,i));
			fprintf( io.output, "%s%s", json_object_to_json_string(str) , comma );
			json_object_put(str);
			break;
		}
		case SQLITE_NULL:
			fprintf( io.output, "\"NULL\"%s", comma);
			break;
		default:
			LOG("unknown\n");
			break;
		}
	}
	fputs( "]", io.output ) ;
	return 0;
}

int CSqldalUnmarshaller::open( RPCCommand& cmd )
{
	char dbFile[256]={0};
	char mode[256]={0};
	cmd.outObj = json_object_new_object() ;

	JSON_GET_MANDATORY_STRING( "dbFile", dbFile, sizeof(dbFile) ) ;
	JSON_GET_DEFAULT_STRING("mode", mode, sizeof(mode), "read");

	int rv ;
	if ( !strcmp(dbFile,":memory:") )
		rv = m_oSqlDal.Open(NULL, mode);
	else
		rv = m_oSqlDal.Open(dbFile, mode );
	JSON_RETURN_BOOL(rv, "sqldal.open");
}

int CSqldalUnmarshaller::lastInsertedRowId( RPCCommand& cmd )
{
	cmd.outObj = json_object_new_object() ;

	long rv = m_oSqlDal.LastInsertRowID() ;
	json_object_object_add(cmd.outObj, "result", json_object_new_int(rv) );
	return true ;
}

int CSqldalUnmarshaller::rowsAffected( RPCCommand& cmd )
{
	cmd.outObj = json_object_new_object() ;

	long rv = m_oSqlDal.RowsAffected() ;
	json_object_object_add(cmd.outObj, "result", json_object_new_int(rv) );
	return true ;
}


int CSqldalUnmarshaller::execute( RPCCommand& cmd )
{
	char* query = NULL ;
	char mode[256] ;
	int timeout ;

	cmd.outObj = json_object_new_object() ;
	JSON_GET_MANDATORY_STRING_PTR( "query", query ) ;
	JSON_GET_DEFAULT_STRING("mode", mode, sizeof(mode)-1, "");
	JSON_GET_DEFAULT_INT("timeout", timeout, 10 );

	if ( query[0] == 0 )
	{
		json_object_object_add(cmd.outObj, "error", json_object_new_string("sqldal.execute"));
		return false ;
	}

	//from now on we do our own serializing
	json_object_put(cmd.outObj);
	cmd.outObj = NULL ;

	fprintf( cmd.io.output, JSON_HEADER()"{ " );

	int rv = m_oSqlDal.Execute( query, cmd.io, jsonCallback, mode, timeout );
	if ( ! rv )
	{
		fprintf( cmd.io.output, " \"result\": null, \"error\" : \"590: DB Error\" }" );
		return true;
	}
	if ( rv ==2 ) fputs(" \"result\": [ ", cmd.io.output );
	fputs(" ], \"error\": null }", cmd.io.output );
	return true ;
}



/**
 * @brief Export as CSV the result of a SQL query.
 */
int CSqldalUnmarshaller::getCsv( RPCCommand& cmd )
{
	char* query = NULL ;
	// Alocate cmd.outObj only when you use json_object function on it
	// otherwise JsonOk know that we're done all the output
	// so that it won't write the headers.
	//
	// cmd.outObj = json_object_new_object() ;

	JSON_GET_MANDATORY_STRING_PTR( "query", query ) ;

	if ( query[0] == 0 )
	{
		return false ;
	}

	fprintf( cmd.io.output, _ATTACHEMENT("report.csv") CSV_HEADER() );
	/* Serialize
	 * This must be optimized later, since it's O(col*row)
	 * Loop through records */
	/*int rv = */m_oSqlDal.Execute( query, cmd.io, csvPrinter ) ;
	return true ;
}

int CSqldalUnmarshaller::close( RPCCommand& cmd )
{
	cmd.outObj = json_object_new_object() ;

	m_oSqlDal.Close() ;
	json_object_object_add(cmd.outObj, "result", json_object_new_boolean(true) );
	return true ;
}
