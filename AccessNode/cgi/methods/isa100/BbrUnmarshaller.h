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

#ifndef _BBR_UNMARSHALLER_H_
#define _BBR_UNMARSHALLER_H_

#include "methods/methods.h"
#include "BbrImpl.h"

//////////////////////////////////////////////////////////////////////////////
/// @class CIsa100Unmarshaller
/// @ingroup LibUnmarshaller
//////////////////////////////////////////////////////////////////////////////
namespace CIsa100 {
	class BbrUnmarshaller : public MethodUnmarshaller
	{
	public:
		static MethodUnmarshaller* GetInstance()
		{
			if ( !m_pInstance )
				m_pInstance = new BbrUnmarshaller();
			return m_pInstance ;
		}
	public:
		int setEngMode( RPCCommand& cmd ) ;
		int getEngMode( RPCCommand& cmd ) ;
	private:
		static BbrUnmarshaller* m_pInstance ;
		BbrImpl m_oBbr ;
	};
};
#endif	//_BBR_UNMARSHALLER_H_
