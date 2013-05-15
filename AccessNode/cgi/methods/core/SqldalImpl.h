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

#ifndef _SQL_DAL_IMPL_H_
#define _SQL_DAL_IMPL_H_

#include "any.hpp"
#include <vector>
#include "IO.h"
#include <pthread.h>

enum { CLOSED, OPENED } ;



struct sqlite3 ;

class CSqldalImpl
{
public:
	CSqldalImpl()
		: m_pDBConn(NULL)
	{
		pthread_mutex_init( &m_sqlMut, NULL );
	}
	~CSqldalImpl()
	{
		Close() ;
	}
public:
//////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////
	int Open( const char* p_pDbFile, const char* p_pkszMode ) ;
//////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////
	int Close() ;

	int Reopen( const char* p_p_kszNewMode ) ;
//////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////
	long LastInsertRowID() ;
//////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////
	int RowsAffected() ;
//////////////////////////////////////////////////////////////////////////////
/// To be used within the control of the programmer, since it takes
/// format string placeholders.
//////////////////////////////////////////////////////////////////////////////
//	int vaExecute(const char* sql_query, IO& io, ...) ;
//////////////////////////////////////////////////////////////////////////////
/// To be used with external/mean-user input, which can contain placeholders.
//////////////////////////////////////////////////////////////////////////////
	//int Execute(const char*, IO&, int (*)(void *NotUsed, int argc, char **argv, char **azColName)
	//, const char*p_kszMode=NULL, int p_Timeout=10) ;

	//////////////////////////////////////////////////////////////////////////////
    /// @brief Executes sql querry
    /// @param [in] sql_query     the sql querry
	/// @param [in/out] io        io object for the request
    /// @param [in] printer       pointer to printer method
    /// @param [in] p_pkszMode    db operation mode
	/// @param [in] timeout       prepare timout
    /// @retval 0 on failure, 1 on success, 2 on success(no header printed)
    //////////////////////////////////////////////////////////////////////////////
    int Execute(const char* sql_query, IO& io, int (*printer)(void*, IO&,bool), const char* p_pkszMode="", int timeout=10 ) ;

protected:
	char* buildQuery(const char*sql_query, va_list& ap );
	int strModeToInt(const char* p_pkszMode) ;
	const char* intModeToStr(int p_iMode) ;
	sqlite3 *m_pDBConn ;
	char m_pDbFile[512] ;
	pthread_mutex_t m_sqlMut ;
	int m_mode ;
} ;


#endif	/* _SQL_DAL_IMPL_H_ */
