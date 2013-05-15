#include <nlib/sqlitexx/Connection.h>
#include <nlib/sqlitexx/Exception.h>
#include <nlib/sqlitexx/ResultSet.h>

#include <string.h>
#include <stdio.h>

#include "sqlite-3.5.8/sqlite3.h"

#include <boost/format.hpp>

namespace nlib {
namespace sqlitexx {

static const std::string PARAMETER_FORMAT = "?%1$03d";

Connection::Connection()
{
	database = NULL;
}

Connection::~Connection()
{
	Close();
}
void Connection::Open(const std::string& dbPath, int timeout)
{
	if (database)
	{
		THROW_EXCEPTION1(Exception, "Database already opened!");
	}

	if (::sqlite3_open(dbPath.c_str(), &database) != SQLITE_OK)
	{
		THROW_EXCEPTION1(Exception, "Unable to open database!");
	}

	//journal=off
	{
		sqlite3_stmt*  sqlStmt = NULL; 
	    int32_t ret = sqlite3_prepare(database, "PRAGMA main.journal_mode = OFF;", -1, &sqlStmt, NULL); 
	    if( ret == SQLITE_OK ) 
	    { 
	    	sqlite3_step(sqlStmt); 
	        const unsigned char* colValue = sqlite3_column_text(sqlStmt, 0);  //<----- returns "off" 
	        if (!colValue)
	        {
		        //THROW_EXCEPTION1(Exception, "Unable to get pragma journal_mode value!");
				LOG_ERROR( "Unable to get pragma journal_mode value!");
	        }
			else
	        LOG_DEBUG("setting PRAGMA main.journal_mode = OFF; with result(string)= " << (const char*)colValue);

	        sqlite3_finalize(sqlStmt); 
	    }
	    else
	    {
		    THROW_EXCEPTION1(Exception, "Unable to set pragma journal_mode!");
	    } 
    }
    //synchronous=off
    {
		sqlite3_stmt*  sqlStmt = NULL; 
	    int32_t ret = sqlite3_prepare(database, "PRAGMA synchronous = OFF;", -1, &sqlStmt, NULL); 
	    if( ret == SQLITE_OK ) 
	    { 
	    	sqlite3_step(sqlStmt); 
	        int colValue = sqlite3_column_int(sqlStmt, 0);  //<----- returns int -> 0 | OFF | 1 | NORMAL | 2 | FULL; 
	        LOG_DEBUG("setting PRAGMA synchronous = OFF; with result(int)= " << colValue);
	        sqlite3_finalize(sqlStmt); 
	    }
	    else
	    {
		    THROW_EXCEPTION1(Exception, "Unable to set pragma synchronous!");
	    } 
    }
    
    
	if (::sqlite3_busy_timeout(database, timeout * 1000) != SQLITE_OK)
	{
		THROW_EXCEPTION1(Exception, "Unable to set database timeout!");
	}


	LOG_ERROR ("sqlite3_threadsafe =" << (int)sqlite3_threadsafe() );

	LOG_DEBUG("Open database:" << dbPath);
}

void Connection::LogIfLastError(int p_nResultCode, const char* p_szContext)
{
	if (p_nResultCode == SQLITE_OK)
	{
		return;
	}
	LOG_ERROR("LogIfLastError : "<< p_szContext << " err=" << sqlite3_errmsg(database) << " res="<< (int)p_nResultCode);
}

void Connection::Close()
{
	if (database)
	{
		::sqlite3_close(database);
		database = NULL;
		LOG_DEBUG("Close database");
	}
}

Command::Command(Connection& connection_, const std::string& query_)
: connection(connection_), query(query_)
{

}

Command::~Command()
{
}

//

ResultSetPtr Command::ExecuteQuery()
{
	LOG_DEBUG("Executing query:" << query);

	char **data = NULL;
	int rows = 0, cols = 0;
	detail::String errMessage;

	int errCode = ::sqlite3_get_table(connection.database, query.c_str(),
	&data, &rows, &cols, &errMessage);

	if (SQLITE_OK == errCode)
	{
		return ResultSetPtr(new ResultSet(data,rows,cols));
	}
	THROW_EXCEPTION2(Exception, boost::str( boost::format("%1% on query: '%2%' ") % std::string(errMessage.Value()) % std::string(query.c_str())), errCode);
}

void Command::ExecuteNonQuery()
{
	LOG_DEBUG("Executing query:" << query);

	detail::String errMessage;

	int errCode = sqlite3_exec(connection.database, query.c_str(), NULL, NULL, &errMessage);
	if (SQLITE_OK != errCode)
	{
		THROW_EXCEPTION2(Exception, boost::str( boost::format("%1% on query: '%2%' ") % std::string(errMessage.Value()) % std::string(query.c_str())), errCode);
	}
}

ResultSetPtr Command::ExecuteQuery(sqlite3* database, const char* p_szQuery )
{
	LOG_DEBUG("Executing query:" << p_szQuery);

	char **data = NULL;
	int rows = 0, cols = 0;
	detail::String errMessage;

	int errCode = ::sqlite3_get_table(database, p_szQuery, &data, &rows, &cols, &errMessage);

	if (SQLITE_OK == errCode)
	{
		return ResultSetPtr(new ResultSet(data,rows,cols));
	}
	THROW_EXCEPTION2(Exception, boost::str( boost::format("%1% on query: '%2%' ") % std::string(errMessage.Value()) % std::string(p_szQuery)), errCode);
}

void Command::ExecuteNonQuery(sqlite3* database, const char* p_szQuery )
{
	LOG_DEBUG("Executing query:" << p_szQuery);

	detail::String errMessage;

	int errCode = sqlite3_exec(database, p_szQuery, NULL, NULL, &errMessage);
	if (SQLITE_OK != errCode)
	{
		THROW_EXCEPTION2(Exception, boost::str( boost::format("%1% on query: '%2%' ") % std::string(errMessage.Value()) % std::string(p_szQuery)), errCode);
	}
}


void Replace(std::string& source, const std::string& find, const std::string& replace)
{
	if ( find.empty() || source.empty() || source.find(find) == std::string::npos || find == replace )
	{
		THROW_EXCEPTION1(Exception, boost::str( boost::format("Unable to find: %1% onto %2% for replacing with %3%!") % find % source % replace ));
	}
	size_t i;
	while ((i = source.find( find )) != std::string::npos) //replace all occurences
	{
		source.replace( i, find.length(), replace );
	}
}

void Command::BindParam(int parameterPos, const int& value)
{
	try
	{
		std::string parameterName = boost::str( boost::format(PARAMETER_FORMAT) % parameterPos );
		std::string parameterValue = boost::lexical_cast<std::string>(value);

		Replace(query, parameterName,	parameterValue);
	}
	catch(std::exception& ex)
	{
		THROW_EXCEPTION1(Exception, boost::str( boost::format("Unable to bind int parameter: %1% on position %2%! Exception: %3%") % value % parameterPos % ex.what()));
	}
}

void Command::BindParam(int parameterPos, const double& value)
{
	try
	{
		std::string parameterName = boost::str( boost::format(PARAMETER_FORMAT) % parameterPos );
		std::string parameterValue = boost::lexical_cast<std::string>(value);

		Replace(query, parameterName,	parameterValue);
	}
	catch(std::exception& ex)
	{
		THROW_EXCEPTION1(Exception, boost::str( boost::format("Unable to bind int parameter: %1% on position %2%! Exception: %3%") % value % parameterPos % ex.what()));
	}
}

void Command::BindParam(int parameterPos, const std::string& value)
{
	try
	{
		std::string parameterName = boost::str( boost::format(PARAMETER_FORMAT) % parameterPos );

		std::string parameterValue;
		parameterValue.reserve(value.size() + 2 + 10);
		parameterValue.push_back('\''); //starting '
		//any ' char should be duplicated for sqlite escapping ...
		for (std::string::const_iterator it = value.begin(); it != value.end(); it++)
		{
			parameterValue.push_back(*it);
			if ('\'' == *it)
				parameterValue.push_back('\'');
		}
		parameterValue.push_back('\''); //endding '

		Replace(query, parameterName,	parameterValue);
	}
	catch(std::exception& ex)
	{
		THROW_EXCEPTION1(Exception, boost::str( boost::format("Unable to bind string parameter: %1% on position %2%! Exception: %3%") % value % parameterPos % ex.what()));
	}
}

void Command::BindParam(int parameterPos, const DateTime& value)
{
	try
	{
		BindParam(parameterPos, nlib::sqlitexx::detail::ToDbString(value));
	}
	catch(std::exception& ex)
	{
		THROW_EXCEPTION1(Exception, boost::str( boost::format("Unable to bind date time parameter on position %1%! Exception: %2%") % parameterPos % ex.what()));
	}
}

int Command::GetLastInsertRowID()
{
	return sqlite3_last_insert_rowid(connection.database);
}

Transaction::Transaction(Connection& connection_)
: connection(connection_)
{
	started = false;
}

Transaction::~Transaction()
{
	Rollback();
}

void Transaction::Begin()
{
	if (started)
	{
		THROW_EXCEPTION1(Exception, "Nested transaction not supported!");
	}

	Command command(connection, "begin transaction;");
	command.ExecuteNonQuery();
	started = true;
}

void Transaction::Commit()
{
	if (started)
	{
		Command command(connection, "commit transaction;");
		command.ExecuteNonQuery();
		started = false;
	}
}

void Transaction::Rollback()
{
	if (started)
	{
		started = false;
		Command command(connection, "rollback transaction;");
		command.ExecuteNonQuery();
	}
}


CSqliteStmtHelper::CSqliteStmtHelper(Connection& connection)
: m_oConnection(connection), m_szSql(NULL), m_pStmt(NULL), m_nStmtNeedReset(0)
{	
	
}


CSqliteStmtHelper::~CSqliteStmtHelper()
{
	Release();
}

void CSqliteStmtHelper::Release()
{
	if (m_pStmt)
	{
		ResetStep();
		sqlite3_finalize(m_pStmt);
		m_pStmt = NULL;
	}

	if (m_szSql)
	{
		free(m_szSql);
		m_szSql = NULL;
	}
}


int CSqliteStmtHelper::Prepare (const char* p_szSql)
{
	if (!m_oConnection.GetRawDbObj())
	{
		LOG_ERROR("CSqliteStmtHelper::Prepare - no database connection");
		return SQLITE_ERROR;
	}
	Release();

	m_szSql = strdup(p_szSql);

	int res = sqlite3_prepare_v2(m_oConnection.GetRawDbObj(), m_szSql, strlen(m_szSql) + 1, &m_pStmt, NULL);

	if (res != SQLITE_OK)
	{
		LOG_ERROR("CSqliteStmtHelper::Prepare " << sqlite3_errmsg(m_oConnection.GetRawDbObj()) << " res="<< (int)res);
		LOG_ERROR("CSqliteStmtHelper::Prepare query=" << m_szSql);

		free(m_szSql);
		m_szSql = NULL;
	}
	
	return res;
}



int CSqliteStmtHelper::Step (bool p_bExec)
{
	if (!m_pStmt)
	{
		LOG_ERROR("CSqliteStmtHelper::Step called without prepare first");
		return SQLITE_ERROR;		
	}

	int res = sqlite3_step(m_pStmt);

	if (res != SQLITE_OK && res != SQLITE_ROW  && res != SQLITE_DONE)
	{
		LOG_ERROR("CSqliteStmtHelper::Step " << sqlite3_errmsg(m_oConnection.GetRawDbObj()) << " res="<< (int)res);
	}
	else
	{	m_nStmtNeedReset = 1;
	}

	if (p_bExec || res != SQLITE_ROW)
	{
		//clear stmt
		ResetStep();
	}

	return res;
}

void	CSqliteStmtHelper::ResetStep()
{
	if (m_nStmtNeedReset)
	{
		sqlite3_clear_bindings(m_pStmt);
		sqlite3_reset(m_pStmt);	
		m_nStmtNeedReset = 0;
	}
}


int		CSqliteStmtHelper::BindInt ( int p_nPos, int p_nValue )
{
	ResetStep();
	int res = sqlite3_bind_int ( m_pStmt, p_nPos, p_nValue);
	m_oConnection.LogIfLastError(res, "BindInt");
	return res;
}

//int		BindInt64 ( int p_nPos, int p_nValue );

int		CSqliteStmtHelper::BindDouble ( int p_nPos, double p_dValue )
{
	ResetStep();
	int res = sqlite3_bind_double ( m_pStmt, p_nPos, p_dValue);
	m_oConnection.LogIfLastError(res, "BindDouble");
	return res;
}

 //use SQLITE_TRANSIENT if you want that sqlite to make a copy
int		CSqliteStmtHelper::BindText ( int p_nPos, const char* p_szText, void(* p_fctFree)(void*) )
{
	ResetStep();
	int res = sqlite3_bind_text ( m_pStmt, p_nPos, p_szText, -1, p_fctFree);
	m_oConnection.LogIfLastError(res, "BindText");
	return res;	
}

int		CSqliteStmtHelper::BindDateTime( int p_nPos,const nlib::DateTime& p_oDateTime )
{
	ResetStep();

	std::string oszDate = detail::ToDbString(p_oDateTime); /// TODO: optimize
	
	int res = sqlite3_bind_text ( m_pStmt, p_nPos, oszDate.c_str(), -1, SQLITE_TRANSIENT); //
	m_oConnection.LogIfLastError(res, "BindDateTime");
	return res;

}

int		CSqliteStmtHelper::BindNull ( int p_nPos)
{
	ResetStep();
	int res = sqlite3_bind_null ( m_pStmt, p_nPos);
	m_oConnection.LogIfLastError(res, "BindNull");
	return res;
}


int		CSqliteStmtHelper::Column_Check ( int p_nCol, int p_nColType)
{
	int numColumns = sqlite3_column_count(m_pStmt);

	if (p_nCol >= numColumns)
	{
		LOG_ERROR("CSqliteStmtHelper::Column_Check colType="<< p_nColType <<" col=" << p_nCol << " >= " << numColumns);
		return SQLITE_ERROR;
	}

	int colType = sqlite3_column_type(m_pStmt, p_nCol);
	if (colType != p_nColType && colType != SQLITE_NULL)
	{
		LOG_ERROR("CSqliteStmtHelper::Column_Check colType="<< p_nColType <<"!= "<< colType <<" col=" << p_nCol );
		return SQLITE_ERROR;
	}

	return SQLITE_OK;
}

// get value
int		CSqliteStmtHelper::Column_GetInt ( int p_nCol, int * p_pnValue)
{
	int res = Column_Check(p_nCol,SQLITE_INTEGER);
	if ( res != SQLITE_OK)
	{
		return res;
	}

	*p_pnValue = sqlite3_column_int(m_pStmt,p_nCol);
	return SQLITE_OK;
}

//int		Column_GetInt64 ( int p_nCol, int * p_pnValue);
int		CSqliteStmtHelper::Column_GetDouble ( int p_nCol, double * p_pdValue)
{
	int res = Column_Check(p_nCol,SQLITE_FLOAT);
	if ( res != SQLITE_OK)
	{
		return res;
	}
	*p_pdValue = sqlite3_column_double(m_pStmt,p_nCol);
	return SQLITE_OK;
}

int		CSqliteStmtHelper::Column_GetText ( int p_nCol, char* p_pText, int p_nMax)
{
	int res = Column_Check(p_nCol,SQLITE_TEXT);
	if ( res != SQLITE_OK)
	{
		return res;
	}
	
	if (p_nMax <= 0)
	{
		p_pText[0] = 0;
		return SQLITE_OK;
	}

	const char * val = (const char *)sqlite3_column_text(m_pStmt, p_nCol) ;

	if (!val)
	{
		p_pText[0] = 0;
		return	 SQLITE_OK;
	}
	strncpy (p_pText, val, p_nMax);

	return SQLITE_OK;
}

int		CSqliteStmtHelper::Column_GetText ( int p_nCol, std::string* p_poText)
{
	int res = Column_Check(p_nCol,SQLITE_TEXT);
	if ( res != SQLITE_OK)
	{
		return res;
	}

	const char * val = (const char *)sqlite3_column_text(m_pStmt, p_nCol) ;

	if (!val)
	{
		*p_poText = "";
		return	 SQLITE_OK;
	}
	*p_poText = val;

	return SQLITE_OK;
}

int		CSqliteStmtHelper::Column_GetDateTime ( int p_nCol, nlib::DateTime* p_poDateTime )
{
	char szDate[64];
	int res = Column_GetText(p_nCol, szDate, sizeof(szDate));
	if ( res != SQLITE_OK)
	{
		return res;
	}	

	if (Column_IsNull(p_nCol))
	{
		LOG_DEBUG("Column_GetDateTime: NULL");
		*p_poDateTime = DateTime();
		return SQLITE_OK;
	}
	
	//YYYY-MM-DD HH:mm:ss
	int year, month, day, hour, minute, second;

	if(sscanf(szDate,"%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second) < 6)
	{
		LOG_ERROR("Column_GetDateTime: invalid datetime=" << szDate <<"  expected YYYY-MM-DD HH:mm:ss");
		*p_poDateTime = DateTime();
		return SQLITE_ERROR;
	}

	LOG_DEBUG("Column_GetDateTime: datetime="<<szDate);
	*p_poDateTime = CreateTime(year, month, day, hour, minute, second);			
	return SQLITE_OK;
}


}// namespace sqlitexx
}// namespace nlib
