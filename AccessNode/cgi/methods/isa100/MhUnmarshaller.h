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

#ifndef _MH_UNMARSHALLER_
#define _MH_UNMARSHALLER_


#include "methods/methods.h"

#include "MhImpl.h"

namespace CIsa100 {
	class MhUnmarshaller : public MethodUnmarshaller {
	public:
	static MethodUnmarshaller* GetInstance()
	{
		if ( !m_pInstance )
			m_pInstance = new MhUnmarshaller();
		return m_pInstance ;
	}
		MhUnmarshaller() { }
	int setPublisher( RPCCommand& cmd ) ;
	int delPublisher( RPCCommand& cmd ) ;
	static MhUnmarshaller* m_pInstance ;
	MhImpl m_oMh ;
	} ;
} ;

#endif	/*_MH_UNMARSHALLER_*/
