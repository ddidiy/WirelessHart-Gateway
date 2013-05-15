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
/// @file       Session.cpp
/// @author     Marius Negreanu
/// @date       Tue Sep  2 16:45:54 EEST 2008
/// @brief      Session management class
/// $Id: Session.cpp,v 1.27.8.1 2013/05/15 19:19:17 owlman Exp $
/////////////////////////////////////////////////////////////////////////////

#include "IO.h"
#include "../Shared/h.h"

pthread_mutex_t CSession::mut;

void CSession::deleteCookie( const char* reason)
{
	fputs("Set-Cookie: CGISESSID=\"\"; Max-Age=\"0\"; Version=\"1\" expires=Thu, 01 Jan 1970 00:00:00 GMT\r\n", io.output );
	FLOG("reason:[%s]",reason );
	unlink(m_szSessionFname);
}

const char* lastError =NULL;
//////////////////////////////////////////////////////////////////////////////
///
//Set-Cookie: PREF=ID=41ca09025646e814:TM=1225184208:LM=1225184208:S=hjYHTLHkWrd55Y_t; expires=Thu, 28-Oct-2010 08:56:48 GMT; path=/; domain=.google.ro
//////////////////////////////////////////////////////////////////////////////
bool CSession::Start(void)
{
	CCookie::Value * ck(NULL) ;
	int nCookieLifetime = 0;

	if (	!CConfig::Load(INI_FILE)
	||	!CIniParser::FindGroup("CGI_DB") )
	{
		lastError = "CGI_DB not found in "INI_FILE ;
		nCookieLifetime=3600 ;
	}
	else
	{	//READ_DEFAULT_VARIABLE_INT("COOKIE_LIFETIME", nCookieLifetime, 3600 );// don't want the log
		if( !GetVar( "COOKIE_LIFETIME", &nCookieLifetime, 0, false) )
		{	nCookieLifetime = 3600;
		}
	}

	if ( m_oCookies.ReadCookies( io.env.HttpCookie() )
	&& ( ck=m_oCookies.GetCookie("CGISESSID"))
	&&  ck->value[0] != '\0' )
	{
		//LOG("Cookie:[%s]", ck->value);
		sprintf(m_szSessionFname, __SESSION_DIR"/%.*s", sizeof(m_szSid), ck->value );
		struct stat sb ;

		if ( stat(m_szSessionFname,&sb) )
		{
			ERR("stat(%s) failed:%s", m_szSessionFname, strerror(errno));
			lastError = "stat failed" ;
			deleteCookie("Unable to find cookie file") ;
			CConfig::Release() ;
			return false ;
		}
		if ( (sb.st_mode & S_IFMT)  != S_IFREG ) return false ;
		if ( (sb.st_mode & S_IRWXU) != (S_IRUSR|S_IWUSR) )
		{
			deleteCookie( "Invalid session file" );
			CConfig::Release() ;
			return false ;
		}
		if ( sb.st_atime + nCookieLifetime <= time(NULL) )
		{
			deleteCookie("expired cookie");
			lastError = "expired cookie" ;
			CConfig::Release() ;
			return false ;
		}
		strncpy(m_szSid,ck->value,__SID_LEN-1);
		m_szSid[sizeof(m_szSid)-1]=0;

		if ( ! refreshCookie( nCookieLifetime ) )
		{
			CConfig::Release() ;
			return false ;
		}
		return CIniParser::Load(m_szSessionFname,"r+") ;
	}
	ERR("Unable to find CGISESSID cookie" );
	lastError = "Unable to find CGISESSID cookie" ;

	if ( mkdir( __SESSION_DIR, 0771) && errno != EEXIST)
	{
		LOG("SessionDirCreate["__SESSION_DIR"] failed");
		lastError = "SessionDirCreate["__SESSION_DIR"] failed" ;
		CConfig::Release() ;
		return false ;
	}

	newSessId(  ) ;
	bool bCreate=false ;
	if ( access(m_szSessionFname, R_OK|W_OK) )
		bCreate=true ;

	bool rp = CIniParser::Load(m_szSessionFname,(bCreate?"w+":"r+")) ;
	if ( !rp )
	{
		deleteCookie("Session File loading failed.");
		CConfig::Release() ;
		return false ;
	}
	// No CConfig::Release here since we might Session::SetVar after Session::Start
	return refreshCookie( nCookieLifetime ) ;
}


bool CSession::refreshCookie(unsigned int p_unCookieLifetime )
{
	char outstr[200], szTmp[256];
	time_t t;
	struct tm *tmp;

	t = time(NULL)+ p_unCookieLifetime;

	tmp = gmtime(&t);
	if (tmp == NULL)
		return false;

	if (strftime(outstr, sizeof(outstr), "%a, %d-%b-%Y %T GMT", tmp) == 0)
		return false ;

	sprintf(szTmp, "Set-Cookie: CGISESSID=%s; language=en; Max-Age=%d; Version=1; expires=%s; path=/", m_szSid, p_unCookieLifetime, outstr);
	fprintf( io.output, "%s\r\n", szTmp );
//	FLOG("%s", szTmp );
	return true ;
}

//////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////
bool CSession::SetVar( const char* p_kszKey, int p_iValue )
{
	if ( ! CIniParser::SetVar( "" , p_kszKey, p_iValue, 0, 1 ) )
	{
		ERR("SetVar(%s,%d) failed", p_kszKey, p_iValue );
		return false ;
	}
	return true ;
}


//////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////
bool CSession::SetVar( const char* p_kszKey, char const * p_kszValue, int p_iValLen)
{
	if ( ! CIniParser::SetVar( "" , p_kszKey, p_kszValue, 0,1 ) )
	{
		ERR("SetVar(%s,%s) failed", p_kszKey, p_kszValue );
		return false ;
	}
	return true ;
}


//////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////
/*bool CSession::GetVar( const char* p_kszKey, char p_szVal[], int p_iValLen )
{
	if ( !CIniParser::GetVar( "", p_kszKey, p_szVal, p_iValLen )
	   )
	{
		FLOG("Unable to find [v].%s", p_kszKey);
		return false ;
	}
	return true ;
}
*/

//////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////
bool CSession::Destroy(void)
{
	CCookie::Value * ck(NULL) ;

	if ( (ck=m_oCookies.GetCookie("CGISESSID")) )
	{
		sprintf(m_szSessionFname, __SESSION_DIR"/%.*s", sizeof(m_szSid), ck->value );
	}
	else if ( *m_szSid )
	{
		sprintf(m_szSessionFname, __SESSION_DIR"/%.*s", sizeof(m_szSid), m_szSid );
	}
	deleteCookie("Destroy") ;
	CConfig::Release() ;
	if ( -1 == unlink(m_szSessionFname) ) return false ;
	return true ;
}


//////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////
bool CSession::IsLoggedIn( int& p_riUid )
{
	CCookie::Value * ck(NULL) ;

	if ( (ck=m_oCookies.GetCookie("CGISESSID")) )
	{
		sprintf(m_szSessionFname, __SESSION_DIR"/%.*s", sizeof(m_szSid), ck->value );
		if ( !CIniParser::GetVar( "", "userid", &p_riUid)
		   )
		{
			p_riUid=-1 ;
			FLOG("Unable to find [v].userid");
			return false ;
		}
		return true ;
	}
	FLOG("not logged in because CGISESSID is empty");
	return false ;
}


//////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////
void CSession::newSessId( )
{
	srand(time(NULL));
	for ( unsigned i = 0;i < sizeof(m_szSid)-1;i++)
	{
		char c = abs(rand()) % 62;

		if (c < 10)
			m_szSid[i] = c + 48;
		else if (c < 36)
			m_szSid[i] = c + 55;
		else
			m_szSid[i] = c + 61;
	}
	m_szSid[sizeof(m_szSid)-1]=0;
	sprintf(m_szSessionFname, __SESSION_DIR"/%.*s", sizeof(m_szSid), m_szSid );
}
