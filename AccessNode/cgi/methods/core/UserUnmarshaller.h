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

#ifndef _USER_UNMARSHALLER_H_
#define _USER_UNMARSHALLER_H_

#include "methods/methods.h"

//////////////////////////////////////////////////////////////////////////////
/// @class CUserUnmarshaller
/// @ingroup LibUnmarshaller
//////////////////////////////////////////////////////////////////////////////
class CUserUnmarshaller : public MethodUnmarshaller
{
public:
	static MethodUnmarshaller* GetInstance()
	{
		if ( !m_pInstance )
		{
			FLOG();
			m_pInstance = new CUserUnmarshaller;
		}
		return m_pInstance ;
	}
public:
	int isValidSession( RPCCommand& cmd );

	/// @param user type:string
	/// @param pass type:string
	int login( RPCCommand& cmd ) ;

	int logout( RPCCommand& cmd ) ;

	// @param user type:string
	/// @param serialNo type:string
	int reset_password( RPCCommand& cmd ) ;

	/// @param serialNo type:string
	/// @param user type:string
	int update( RPCCommand& cmd ) ;
private:
	static CUserUnmarshaller* m_pInstance ;
	CUserImpl m_oUser;
};
#endif	/* _USER_UNMARSHALLER_H_ */
