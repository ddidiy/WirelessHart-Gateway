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

#include "GwUnmarshaller.h"


CIsa100::Modbus::GwUnmarshaller* CIsa100::Modbus::GwUnmarshaller::m_pInstance=NULL;

int CIsa100::Modbus::GwUnmarshaller::setRow( RPCCommand& cmd )
{
	char * rowValue, * section, * mapType;
	cmd.outObj = json_object_new_object();
	JSON_GET_MANDATORY_STRING_PTR( "rowValue", rowValue );
	JSON_GET_DEFAULT_STRING_PTR( "section", section, "HOSTS" );
	JSON_GET_DEFAULT_STRING_PTR( "mapType", mapType, "0" );

	int nRet = m_oGw.setRow( rowValue, section, mapType);

	JSON_RETURN_BOOL( nRet, "isa100.modbus.gw.setRow" );
}

int CIsa100::Modbus::GwUnmarshaller::delRow( RPCCommand& cmd )
{
	char * rowValue, * section, * mapType;
	cmd.outObj = json_object_new_object();
	JSON_GET_MANDATORY_STRING_PTR( "rowValue", rowValue );
	JSON_GET_DEFAULT_STRING_PTR( "section", section, "HOSTS" );
	JSON_GET_DEFAULT_STRING_PTR( "mapType", mapType, "0" );

	int nRet = m_oGw.delRow( rowValue, section, mapType );

	JSON_RETURN_BOOL( nRet, "isa100.modbus.gw.delRow" );
}
