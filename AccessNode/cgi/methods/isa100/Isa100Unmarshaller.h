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

#ifndef _ISA100_UNMARSHALLER_H_
#define _ISA100_UNMARSHALLER_H_

//#include <pthread.h>

#include "methods/methods.h"

#include "MhUnmarshaller.h"
#include "SmUnmarshaller.h"
#include "BbrUnmarshaller.h"
#include "modbus/GwUnmarshaller.h"
#include "modbus/ServerUnmarshaller.h"

//////////////////////////////////////////////////////////////////////////////
/// @class CIsa100Unmarshaller
/// @ingroup LibUnmarshaller
//////////////////////////////////////////////////////////////////////////////
class CIsa100Unmarshaller : public MethodUnmarshaller
{
public:
	static MethodUnmarshaller* GetInstance()
	{
		if ( !m_pInstance )
			m_pInstance = new CIsa100Unmarshaller();
		return m_pInstance ;
	}

public:
	int getSystemStatus( RPCCommand& cmd ) ;
	int setModbusRow( RPCCommand& cmd ) ;
	int delModbusRow( RPCCommand& cmd ) ;

private:
	CIsa100Unmarshaller()
	{
		pthread_mutex_init( &m_oMutex, NULL );
	};
	
	static CIsa100Unmarshaller* m_pInstance ;
	CIsa100Impl m_oIsa100 ;
	pthread_mutex_t m_oMutex;
};
#endif	//_ISA100_UNMARSHALLER_H_
