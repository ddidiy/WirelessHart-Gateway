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

/////////////////////////////////////////////////////////////////////////////
/// @file       Session.hpp
/// @author     Marius Negreanu
/// @date       Tue Sep  2 16:45:54 EEST 2008
/// @brief      Session management class
/// $Id: Session.h,v 1.25.8.1 2013/05/15 19:19:18 owlman Exp $
/////////////////////////////////////////////////////////////////////////////


#ifndef _SESSION_H_
#define _SESSION_H_

#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstdlib>
#include <cstdio>

#include <pthread.h>

#include "../Shared/Config.h"

#include "Cookie.h"

//since we're having problems convincing browsers to discard the cookie from a previous session under Apache, just store them persistently
#ifndef HW_I386
#define __SESSION_DIR NIVIS_TMP"cgisession"
#else
#define __SESSION_DIR NIVIS_ACTIVITY_FILES"cgisession"
#endif

#define __SID_LEN 32

/////////////////////////////////////////////////////////////////////////////
/// @class
/////////////////////////////////////////////////////////////////////////////
class CSession : public CConfig
{
public:
	CSession( IO & p_io)
		:  io(p_io)
	{
		memset( m_szSid, 0, sizeof(m_szSid) );
		//mut=NULL ;
	}
	~CSession()
	{
		Release() ;
	}
	void Release()
	{
		CConfig::Release() ;
	}

	void Lock() { FLOG(); pthread_mutex_lock(&mut); }
	void Unlock() { pthread_mutex_unlock(&mut); FLOG(); }
	// starts or resume a session
	bool Start(void) ;
	bool SetVar( const char* p_kszKey, int p_iValue ) ;
	bool SetVar( const char* p_kszKey, char const * p_kszValue, int p_iValLen) ;
	bool Destroy(void) ;
	bool IsLoggedIn( int& p_riUid ) ;

	IO&	io ;

protected:
	/// This can be optimized to use read(/dev/urandom)
	void newSessId( ) ;
	void deleteCookie( const char* reason);
	bool refreshCookie( unsigned int p_unCookieLifetime ) ;

protected:
	CCookie m_oCookies ;
	char m_szSessionFname[ sizeof(__SESSION_DIR)+ 2 +__SID_LEN ] ;
	char m_szSid[__SID_LEN] ;
	static pthread_mutex_t mut;
} ;
#endif
