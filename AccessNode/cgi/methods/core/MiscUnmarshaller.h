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

#ifndef _MISC_UNMARSHALLER_H_
#define _MISC_UNMARSHALLER_H_

#include <vector>
#include "methods/methods.h"

//////////////////////////////////////////////////////////////////////////////
/// @class CMiscUnmarshaller
/// @ingroup LibUnmarshaller
//////////////////////////////////////////////////////////////////////////////
class CMiscUnmarshaller : public MethodUnmarshaller
{
public:
	static MethodUnmarshaller* GetInstance()
	{
		if ( !m_pInstance )
			m_pInstance = new CMiscUnmarshaller();
		return m_pInstance ;
	}

public:
	int remoteCmd( RPCCommand& cmd ) ;
	int getVersion( RPCCommand& cmd ) ;
	int fileUpload( RPCCommand& cmd ) ;
	int fileDownload( RPCCommand& cmd ) ;
	int getGatewayNetworkInfo( RPCCommand& cmd ) ;
	int setGatewayNetworkInfo( RPCCommand& cmd ) ;
	int getNtpServers( RPCCommand& cmd ) ;
	int setNtpServers( RPCCommand& cmd ) ;
	int restartApp( RPCCommand& cmd ) ;
	int softwareReset( RPCCommand& cmd ) ;
	int hardReset( RPCCommand& cmd ) ;
	int applyConfigChanges( RPCCommand& cmd ) ;
	int discoverRouters( RPCCommand& cmd ) ;

private:
	static CMiscUnmarshaller* m_pInstance ;
	CMiscImpl m_oMisc ;
};
#endif	/* _MISC_UNMARSHALLER_H_ */
