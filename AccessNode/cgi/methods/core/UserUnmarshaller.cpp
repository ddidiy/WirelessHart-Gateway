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

#include "UserUnmarshaller.h"

CUserUnmarshaller* CUserUnmarshaller::m_pInstance =NULL ;

int CUserUnmarshaller::isValidSession( RPCCommand& cmd )
{
    cmd.outObj = json_object_new_object();
    json_object_object_add(cmd.outObj, "result", json_object_new_boolean(true));
    return true;
}

int CUserUnmarshaller::login( RPCCommand& cmd )
{
	const char *user, *pass;

	cmd.outObj = json_object_new_object();

	JSON_GET_MANDATORY_STRING_PTR("user", user);
	JSON_GET_MANDATORY_STRING_PTR("pass", pass);

	// Invoke actual method
	//m_oUser.Load(io) ;
	bool bRet = m_oUser.login(cmd.io, user, pass);

	JSON_RETURN_BOOL( bRet, "user.login" );
}


int CUserUnmarshaller::logout( RPCCommand& cmd )
{
	cmd.outObj = json_object_new_object();

	// Invoke actual method
	bool bRet = m_oUser.logout(cmd.io);

	JSON_RETURN_BOOL( bRet, "user.logout" );
}

int CUserUnmarshaller::reset_password( RPCCommand& cmd )
{
	const char *serialNo,*user ;

	cmd.outObj = json_object_new_object();

	JSON_GET_MANDATORY_STRING_PTR("user", user);
	JSON_GET_MANDATORY_STRING_PTR("serialNo", serialNo);

	// Invoke actual method
	bool bRet  = m_oUser.reset_password( cmd.io, user, serialNo );

	JSON_RETURN_BOOL( bRet, "user.reset_password" );
}

int CUserUnmarshaller::update( RPCCommand& cmd )
{
	char	*pass;

	cmd.outObj = json_object_new_object();

	JSON_GET_MANDATORY_STRING_PTR("pass", pass);

	// Invoke actual method
	bool bRet  = m_oUser.update( cmd.io, pass);

	JSON_RETURN_BOOL( bRet, "user.update" );
}
