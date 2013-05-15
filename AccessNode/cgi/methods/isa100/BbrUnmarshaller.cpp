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

#include "BbrUnmarshaller.h"

CIsa100::BbrUnmarshaller* CIsa100::BbrUnmarshaller::m_pInstance =NULL;


int CIsa100::BbrUnmarshaller::setEngMode( RPCCommand& cmd )
{
	int mode, channel, pwrlvl ;
	cmd.outObj = json_object_new_object();
	JSON_GET_MANDATORY_INT( "mode", mode );
	JSON_GET_MANDATORY_INT( "channel", channel );
	JSON_GET_MANDATORY_INT( "pwrlvl", pwrlvl );

	int nRet = m_oBbr.setEngMode( mode, channel, pwrlvl );

	JSON_RETURN_BOOL( nRet, "isa100.setEngMode" );
}

int CIsa100::BbrUnmarshaller::getEngMode( RPCCommand& cmd )
{
	cmd.outObj = json_object_new_object();

	int nRet = m_oBbr.getEngMode( );
	if ( nRet == -1 )
	{
		json_object_object_add( cmd.outObj, "error", json_object_new_string("getEngMode failed") ) ;
		return false ;
	}

	json_object_object_add( cmd.outObj, "result", json_object_new_int(nRet) );
	return true ;
}
