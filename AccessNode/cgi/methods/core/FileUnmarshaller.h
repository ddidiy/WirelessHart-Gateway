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

#ifndef _FILE_UNMARSHALLER_H_
#define _FILE_UNMARSHALLER_H_

#include "methods/methods.h"

class CFileUnmarshaller : public MethodUnmarshaller
{
public:
	static MethodUnmarshaller* GetInstance()
	{
		if ( !m_pInstance )
			m_pInstance = new CFileUnmarshaller();
		return m_pInstance ;
	}
public:
	int create( RPCCommand& cmd ) ;
	int exists( RPCCommand& cmd ) ;
	int remove( RPCCommand& cmd ) ;
private:
	static CFileUnmarshaller* m_pInstance ;
	CFileImpl m_oFile ;
} ;



#endif	/* _FILE_UNMARSHALLER_H_ */
