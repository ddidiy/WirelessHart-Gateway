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

#include "MiscImpl.h" // for SMTP_SendMail
#include <cstdio>

#ifndef __USERDB_FILE
#define __USERDB_FILE NIVIS_ACTIVITY_FILES"userdb.ini"
#endif

#define MAX_PASS_LEN 64

#ifndef __MAX_USERNAME
#define __MAX_USERNAME 256
#endif


#include "UserImpl.h"
#include "md5.h"

// If gSalt is changed, please make sure to change in CUserImpl::send_email
// which save the hash dirrectly
const char *gSalt="$1$";

#include "../UserSet.h" // for User struct
int usrcmp(const void*a, const void*b)
{
	return strcmp( ((User*)a)->userName, ((User*)b)->userName);
}

static int isValidUser( const char* p_kszUser )
{
	struct User key;
	key.userName = p_kszUser ;
	void *res = bsearch( &key, UserSet, MAX_USER_ID, sizeof(struct User), usrcmp );
	if ( res  )
	{
		return ((User*)res)->uid ;
	}
	LOG("ERROR: Invalid user [%s]", p_kszUser);
	return -1 ;
}

bool CUserImpl::load(void)
{
	if ( m_DBLoaded ) return true ;
	if ( !CIniParser::Load(__USERDB_FILE, "r+") )
	{
		ERR("Unable to load userdb["__USERDB_FILE"] file");
		return (m_DBLoaded = false) ;
	}
	return (m_DBLoaded = true) ;
}
bool CUserImpl::login( IO& io, const char * p_kszUser, const char * p_kszPass)
{
	int uid =-1;
	if ( !load() ) return false ;
	if ( (uid=isValidUser(p_kszUser)) == -1 )
		return false ;
	if ( ! isValidSerial(p_kszPass) )
	{
		if ( !CIniParser::FindGroup(p_kszUser) )
		{
			ERR("%s:Unable to find user.",p_kszUser);
			return false ;
		}

		char p[MAX_PASS_LEN] ;
		if ( !CIniParser::GetVar(p_kszUser, "pass", p,MAX_PASS_LEN ) )
		{
			ERR("%s: Unable to read pass.", p_kszUser);
			return false ;
		}

		char cp[120];
		md5_crypt(p_kszPass, gSalt, cp) ;

		if ( strcmp( cp, p ) )
		{
			WARN("%s Password missmatch: uaPass:[%s] != lcPass:[%s]", p_kszUser, cp, p);
			log2flash("CGI: WARN: %s login %s Password missmatch", io.env.Get("REMOTE_ADDR"), p_kszUser);
			return false ;
		}
	}

	// put the userid in the session
	io.session.SetVar("userid", uid ) ;
	io.session.SetVar("username", p_kszUser,__MAX_USERNAME ) ;
	FLOG("CGI: %s %s login ok", io.env.Get("REMOTE_ADDR"), p_kszUser );
	log2flash("CGI: %s %s login ok", io.env.Get("REMOTE_ADDR"), p_kszUser );
	return true ;
}
bool CUserImpl::logout( IO& io )
{
	log2flash("CGI: %s logout", io.env.Get("REMOTE_ADDR"));
	io.session.Destroy();
	return true ;
}
bool CUserImpl::update( IO& io, const char * p_kszPass)
{
	int uid(-1);
	char user[__MAX_USERNAME] ;
	if (!load() ) return false ;

	if (	strlen(p_kszPass) < 8
	                ||	strlen(p_kszPass) > 16 )
	{
		ERR("Password constraint failed");
		return false ;
	}

	io.session.GetVar("username", user, __MAX_USERNAME ) ;
	if ( -1 == (uid=isValidUser(user)) )
	{
		ERR("Invalid username[%s]", user);
		return false;
	}
	if ( !CIniParser::FindGroup( user ) )
	{
		ERR("%s:Unable to find user.", user);
		return false ;
	}

	char epass[120];
	md5_crypt( p_kszPass, gSalt, epass) ;

	if ( !CIniParser::SetVar( user, "pass", epass,0,1) )
	{
		ERR("Unable to set password for [admin]");
		return false ;
	}
	FLOG("user.update(%s,%s)",user,epass);
	log2flash("CGI: %s update", io.env.Get("REMOTE_ADDR"));

	return true ;
}
bool CUserImpl::reset_password( IO& io, const char * p_kszUser, const char * p_kszSerialNo )
{
	int uid =-1;
	if ( -1 == (uid=isValidUser(p_kszUser))
	||	! isValidSerial(p_kszSerialNo)
	||	! load()
	   )
		return false ;
	if ( !CIniParser::FindGroup(p_kszUser) )
	{
		ERR("%s:Unable to find user.",p_kszUser);
		return false ;
	}

	// put the userid in the session
	io.session.SetVar("userid", uid ) ;
	io.session.SetVar("username", p_kszUser,__MAX_USERNAME ) ;
	LOG("%s Logged in", p_kszUser);
	log2flash("CGI: %s %s login ok", io.env.Get("REMOTE_ADDR"), p_kszUser );
	return true ;
}
bool CUserImpl::isValidSerial(const char *p_kszSerialNo )
{
	CIniParser oP;
	if ( ! oP.Load( NIVIS_PROFILE"config.ini") )
	{
		ERR("Unable to load "NIVIS_PROFILE"config.ini");
		return false ;
	}
	oP.FindGroup("GLOBAL");
	char an_id[256]={0,};
	if ( ! oP.GetVar( NULL, "AN_ID", an_id, sizeof(an_id) ) )
	{
		ERR("AN_ID not found");
		return false ;
	}

	char *tmpSerialNo = strdup(p_kszSerialNo);
	int d=0;
	for ( unsigned s=0; s<strlen(an_id); )
	{
		if ( an_id[s] == ' ' )
		{
			s++;
			continue ;
		}
		an_id[d++]=an_id[s++];
	}
	an_id[d]='\0';
	d=0;
	for ( unsigned s=0; s<strlen(tmpSerialNo); )
	{
		if ( tmpSerialNo[s] == ' ' )
		{
			s++;
			continue ;
		}
		tmpSerialNo[d++]=tmpSerialNo[s++];
	}
	tmpSerialNo[d]='\0';
	if ( strcmp( tmpSerialNo, an_id) )
	{
		WARN("SerialNo missmatch[%s]", an_id);
		free(tmpSerialNo);
		return false ;
	}
	free(tmpSerialNo);
	return true ;
}
