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

#ifndef _CONFIG_UNMARSHALLER_H_
#define _CONFIG_UNMARSHALLER_H_

#include "methods/methods.h"

//////////////////////////////////////////////////////////////////////////////
/// @class CConfigUnmarshaller
/// @ingroup LibUnmarshaller
//////////////////////////////////////////////////////////////////////////////
class CConfigUnmarshaller : public MethodUnmarshaller
{
public:
	static MethodUnmarshaller* GetInstance()
	{
		if ( !m_pInstance )
			m_pInstance = new CConfigUnmarshaller();
		return m_pInstance ;
	}
public:
	int getVariable( RPCCommand& cmd ) ;
	int getGroups( RPCCommand& cmd ) ;
	int getGroupVariables( RPCCommand& cmd ) ;
	int setGroupVariables( RPCCommand& cmd ) ;
	int getConfig( RPCCommand& cmd ) ;
    int getConfigPart( RPCCommand& cmd );
	int setVariable( RPCCommand& cmd ) ;
private:
	static CConfigUnmarshaller* m_pInstance ;
	CConfigImpl m_oConfig ;
};
#endif	/* _CONFIG_UNMARSHALLER_H_ */
