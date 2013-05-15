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

#include "MhUnmarshaller.h"


CIsa100::MhUnmarshaller* CIsa100::MhUnmarshaller::m_pInstance=NULL;

int CIsa100::MhUnmarshaller::setPublisher( RPCCommand& cmd )
{	char * szEUI64, * szConcentrator;
	struct array_list *pChannels ;
	
	cmd.outObj = json_object_new_object();

	JSON_GET_MANDATORY_STRING_PTR( "eui64", szEUI64 );
#if defined( RELEASE_ISA )
	JSON_GET_MANDATORY_STRING_PTR( "concentrator", szConcentrator );
	JSON_GET_MANDATORY_ARRAY( "channels", pChannels );
#elif defined( RELEASE_WHART )
	char * szTrigger;
	JSON_GET_MANDATORY_STRING_PTR( "burst", szConcentrator );
	JSON_GET_MANDATORY_ARRAY( "variables", pChannels );
	JSON_GET_MANDATORY_STRING_PTR( "trigger", szTrigger );
#endif

	ChannelsVector oVectorChannels;
	oVectorChannels.reserve( pChannels->length );
	for ( int i=0; i < pChannels->length; ++i ) 
	{	oVectorChannels.push_back( json_object_get_string((json_object*)array_list_get_idx( pChannels, i )) ) ;
	}

#if defined( RELEASE_ISA )
	int nRet = m_oMh.setPublisher( szEUI64, szConcentrator, &oVectorChannels );
#elif defined( RELEASE_WHART )
	int nRet = m_oMh.setPublisher( szEUI64, szConcentrator, &oVectorChannels, szTrigger );
#endif

	JSON_RETURN_BOOL( nRet, "isa100.mh.setPublisher" );
}

int CIsa100::MhUnmarshaller::delPublisher( RPCCommand& cmd )
{	char * szEUI64;
	cmd.outObj = json_object_new_object();
	JSON_GET_MANDATORY_STRING_PTR( "eui64", szEUI64 );
#if defined( RELEASE_ISA )
	int nRet = m_oMh.delPublisher( szEUI64 );
#elif defined( RELEASE_WHART )
	char * szConcentrator;
	JSON_GET_MANDATORY_STRING_PTR( "burst", szConcentrator );
	int nRet = m_oMh.delPublisher( szEUI64, szConcentrator );
#endif
	
	JSON_RETURN_BOOL( nRet, "isa100.mh.delPublisher" );
}
