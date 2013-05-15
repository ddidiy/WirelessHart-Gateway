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

#include "SmUnmarshaller.h"

CIsa100::SmUnmarshaller* CIsa100::SmUnmarshaller::m_pInstance =NULL;


int CIsa100::SmUnmarshaller::setDevice( RPCCommand& cmd )
{
	char * deviceValue, * deviceType;
	cmd.outObj = json_object_new_object();
	JSON_GET_MANDATORY_STRING_PTR( "deviceValue", deviceValue );
	JSON_GET_DEFAULT_STRING_PTR( "deviceType", deviceType, "DEVICE" );

	
	int nNetworkType = 0;
	JSON_GET_DEFAULT_INT( "NetworkType", nNetworkType, nNetworkType );
	int nRet = m_oSm.setDevice( deviceValue, deviceType, nNetworkType );

	JSON_RETURN_BOOL( nRet, "isa100.sm.setDevice" );
}

int CIsa100::SmUnmarshaller::delDevice( RPCCommand& cmd )
{
	char * szDeviceValue, * deviceType;
	

	cmd.outObj = json_object_new_object();
	JSON_GET_MANDATORY_STRING_PTR( "deviceValue", szDeviceValue );
	JSON_GET_DEFAULT_STRING_PTR( "deviceType", deviceType, "DEVICE" );
	

	int nNetworkType = 0;
	JSON_GET_DEFAULT_INT( "NetworkType", nNetworkType, nNetworkType );

	int nRet = m_oSm.delDevice( szDeviceValue, deviceType, nNetworkType );
	

	JSON_RETURN_BOOL( nRet, "isa100.sm.delDevice" );
}

int CIsa100::SmUnmarshaller::getLogLevel( RPCCommand& cmd )
{
	const char * pLogLevel = m_oSm.getLogLevel( );
	cmd.outObj = json_object_new_object();
	if ( !pLogLevel )
	{
		json_object_object_add( cmd.outObj, "error",  json_object_new_string("isa100.sm.getLogLevel") );
		return false ;
	}

	json_object_object_add(cmd.outObj, "result", json_object_new_string( pLogLevel) );
	return true;
}

int CIsa100::SmUnmarshaller::setLogLevel( RPCCommand& cmd )
{
	const char * pLogLevel;
	cmd.outObj = json_object_new_object();
	JSON_GET_MANDATORY_STRING_PTR( "logLevel", pLogLevel );

	int nRet = m_oSm.setLogLevel( pLogLevel );

	JSON_RETURN_BOOL( nRet, "isa100.sm.setLogLevel");
}
