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

#ifndef _MODBUS_GW_UNMARSHALLER_
#define _MODBUS_GW_UNMARSHALLER_


#include "methods/methods.h"

#include "GwImpl.h"

namespace CIsa100 {
namespace Modbus {

	class GwUnmarshaller : public MethodUnmarshaller {
	public:
		static MethodUnmarshaller* GetInstance()
		{
			if ( !m_pInstance )
				m_pInstance = new GwUnmarshaller();
			return m_pInstance ;
		}
		GwUnmarshaller() { }
		int setRow( RPCCommand& cmd ) ;
		int delRow( RPCCommand& cmd ) ;
	protected:
		static GwUnmarshaller* m_pInstance ;
		GwImpl m_oGw ;
	} ;

}	// namespace  Modbus
}	// namespace  CIsa100

#endif	/*_MODBUS_GW_UNMARSHALLER_*/
