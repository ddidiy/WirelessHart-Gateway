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

#include <sqlite3.h>
#include "../../../Shared/h.h"
#include "SqldalImpl.h"
#include "JsonRPC.h"

#include <cctype>



//////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////
int CSqldalImpl::Open( const char* p_pDbFile, const char* p_pkszMode )
{
	int srv, mode ;
	// pre
	if ( ! p_pDbFile || *p_pDbFile==0 )
	{
		FERR("NULL file name");
		Close();
		return false ;
	}
	if ( ! p_pkszMode || *p_pkszMode==0 )
	{
		FERR("NULL opening mode");
		Close();
		return false ;
	}

	mode = strModeToInt(p_pkszMode);
	if ( m_pDBConn && mode == m_mode ) return true ;

	m_mode = mode ;
	memmove( m_pDbFile, p_pDbFile, strlen(p_pDbFile) );
	srv = sqlite3_enable_shared_cache( false );
	if (srv != SQLITE_OK)
	{
		WARN("FAIL:sqlite3_enable_shared_cache:[%d]:[%s]",  sqlite3_errcode(m_pDBConn), sqlite3_errmsg(m_pDBConn) );
	}
	srv = sqlite3_open_v2 ( m_pDbFile, &m_pDBConn, m_mode, NULL );
	if (srv != SQLITE_OK)
	{
		ERR("sqlite3_open:[%d]:[%s]", sqlite3_errcode(m_pDBConn), sqlite3_errmsg(m_pDBConn) );
		Close();
		return false ;
	}
	LOG("[%s] Openned in [%s] mode", m_pDbFile, p_pkszMode );
	return true ;
}
//////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////
int CSqldalImpl::Close()
{
	int srv ;

	if ( ! m_pDBConn ) return true;
        while (sqlite3_next_stmt(m_pDBConn, NULL) != NULL)
	{
		sqlite3_finalize(sqlite3_next_stmt(m_pDBConn, NULL));
	}

	srv = sqlite3_close(m_pDBConn);
	if ( srv != SQLITE_OK )
	{
		WARN("FAIL:sqlite3_close:[%d]:[%s]", sqlite3_errcode(m_pDBConn), sqlite3_errmsg(m_pDBConn));
		return false ;
	}
	m_pDBConn = NULL;

	sync() ;
	return true ;
}
//////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////
int CSqldalImpl::Reopen(const char* p_pkszNewMode )
{
	if ( ! p_pkszNewMode || 0 == p_pkszNewMode[0] ) return false ; // do not reopen, keep current

	if ( strModeToInt(p_pkszNewMode) == m_mode && m_pDBConn ) return true ; // not changed, keep current
	LOG("Try reopen:[%s]",p_pkszNewMode );	// log only when mode changes
	if ( ! Close() )
	{
		return false ;
	}
	return Open(m_pDbFile,p_pkszNewMode);
}
//////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////
long CSqldalImpl::LastInsertRowID()
{
	if ( ! m_pDBConn )
	{
		WARN("%s called on closed DB.", __FUNCTION__) ;
		return 0;
	}
	sqlite_int64 id = sqlite3_last_insert_rowid(m_pDBConn);
	return id ;
}
//////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////
int CSqldalImpl::RowsAffected()
{
	if ( ! m_pDBConn )
	{
		WARN("%s called on closed DB.", __FUNCTION__) ;
		return 0;
	}
	return sqlite3_changes(m_pDBConn) ;
}
//////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////
int CSqldalImpl::Execute(const char* sql_query, IO& io, int (*printer)(void*, IO&,bool), const char* p_pkszMode/*=""*/, int p_Timeout/*=10*/ )
{
	pthread_mutex_lock(&m_sqlMut);

	int rc(SQLITE_OK), first(true);
	const char* zLeftOver ;
	sqlite3_stmt *stmt = NULL ;
	int mode = m_mode ; // save current open mode.

	Reopen(p_pkszMode) ;

	time_t takingTooLong=time(NULL)+p_Timeout ;
	LOG("EXEC:[%s]", sql_query);

	
	for( unsigned nRetry=0; sql_query[0] && (rc==SQLITE_OK || (rc==SQLITE_SCHEMA)); ++nRetry)
	{	
		int srv = 0;
		if ( takingTooLong <=time(NULL) )
		{
			WARN("FAIL:unable to run query in %i seconds (%u attempts, last retCode %d:%s). SQL [%s]",
				p_Timeout, nRetry, srv, sqlite3_errmsg(m_pDBConn), sql_query);
			Reopen(intModeToStr(mode));
            pthread_mutex_unlock(&m_sqlMut);
            return 0;
		}

		srv = sqlite3_prepare_v2(m_pDBConn, sql_query, strlen(sql_query), &stmt, &zLeftOver);

		if ( SQLITE_BUSY == srv )
		{
			WARN("sqlite3_prepare_retry:[%d]:[%s]", sqlite3_errcode(m_pDBConn), sqlite3_errmsg(m_pDBConn) );
			usleep(10000) ;// 10ms delay
			if ( SQLITE_OK != sqlite3_finalize(stmt) )
				WARN("FAIL:sqlite3_finalize:[%d]:[%s]", sqlite3_errcode(m_pDBConn), sqlite3_errmsg(m_pDBConn) );
			continue ;
		}
		if ( SQLITE_OK != srv )
		{
			WARN("FAIL:sqlite3_prepare:[%d]:[%s]", sqlite3_errcode(m_pDBConn), sqlite3_errmsg(m_pDBConn) );
			if ( SQLITE_OK != sqlite3_finalize(stmt) )
				WARN("FAIL:sqlite3_finalize:[%d]:[%s]", sqlite3_errcode(m_pDBConn), sqlite3_errmsg(m_pDBConn) );
			Reopen( intModeToStr(mode) ) ;
			pthread_mutex_unlock(&m_sqlMut);
			return 0 ;
		}
		// this happens for a comment or white-space
		if( !stmt )
		{
			sql_query = zLeftOver;
			continue;
		}

		// returned rows
		unsigned rowRetries=0;
		for ( int rv=SQLITE_ROW; rv==SQLITE_ROW ; )
		{
			rv = sqlite3_step(stmt) ;
			if ( SQLITE_ROW == rv )
			{
				printer( stmt, io, first );
				first=false;
				rowRetries=0;
				continue;
			}
			if ( rowRetries >= 100 ) /* Break if we're unable to get a row in one second(100*10ms) */
			{
				WARN("FAIL:sqlite3_step:Unable to retrieve row; query:[%s]", sql_query);
				Reopen( intModeToStr(mode) );
				pthread_mutex_unlock(&m_sqlMut);
				return 0;
			}
			if ( SQLITE_BUSY == rv )
			{
				WARN("FAIL:sqlite3_step:[%d]:[%s]", sqlite3_errcode(m_pDBConn), sqlite3_errmsg(m_pDBConn) );
				usleep(20*1000) ; // 20ms delay
				rv=SQLITE_ROW;
				rowRetries++;
				continue;
			}
			else if ( SQLITE_DONE != rv )
			{
				WARN("FAIL:sqlite3_step:[%d]:[%s]", sqlite3_errcode(m_pDBConn), sqlite3_errmsg(m_pDBConn) );
				usleep(20*1000) ; // 20ms delay
				break;
			}
		}
		sql_query = zLeftOver;
	
		if ( SQLITE_OK != sqlite3_finalize(stmt) )
		{
			WARN("FAIL:sqlite3_finalize:[%d]:[%s]", sqlite3_errcode(m_pDBConn), sqlite3_errmsg(m_pDBConn) );
            Reopen(intModeToStr(mode));
            pthread_mutex_unlock(&m_sqlMut);
            return 0;
		}
	}

	Reopen( intModeToStr(mode) );
	pthread_mutex_unlock(&m_sqlMut);
	if ( true == first ) return 2 ; // the header was not printed
	return 1;
}
//////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////
char * CSqldalImpl::buildQuery(const char*sql_query, va_list&ap)
{
	/* Guess we need no more than 100 bytes. */
	int n, size = 100;
	char *p, *np;

	if ((p = (char*)malloc(size)) == NULL)
		return NULL;

	while (1) {
		/* Try to print in the allocated space. */
		n = vsnprintf(p, size, sql_query, ap);
		/* If that worked, return the string. */
		if (n > -1 && n < size)
			return p;
		/* Else try again with more space. */
		if (n > -1)    /* glibc 2.1 */
			size = n+1; /* precisely what is needed */
		else           /* glibc 2.0 */
			size *= 2;  /* twice the old size */
		if ((np = (char*)realloc (p, size)) == NULL) {
			free(p);
			return NULL;
		} else {
			p = np;
		}
	}
	return p ;
}
//////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////
int CSqldalImpl::strModeToInt(const char* p_pkszMode)
{
	const char *it=p_pkszMode;
	int mode(0);

	while( it && *it )
	{
		while ( it && ( *it=='|' || isblank(*it) ) ) ++it ;
		if ( ! *it ) break;

		if ( !strncmp("read",it,4 ) )
		{
			it+=4;
			mode |= SQLITE_OPEN_READONLY ;
		}
		else if ( !strncmp("write",it,5) )
		{
			it+=5;
			mode |= SQLITE_OPEN_READWRITE ;
			continue ;
		}
		else if ( !strncmp("create",it,6) )
		{
			it+=6;
			mode |= SQLITE_OPEN_CREATE ;
			continue ;
		}
		else
		{
			break ;
		}
	}
	return mode ;
}
const char* CSqldalImpl::intModeToStr(int p_iMode)
{
	if ( p_iMode == SQLITE_OPEN_READWRITE ) return "write";
	if ( p_iMode == SQLITE_OPEN_CREATE ) return "create";
	if ( p_iMode == SQLITE_OPEN_READONLY ) return "read";
	return "read" ;
}
