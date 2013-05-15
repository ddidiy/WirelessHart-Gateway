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

#ifndef __SOURCEROUTE_H_
#define __SOURCEROUTE_H_


#include <Shared/MicroSec.h>

#include <WHartStack/WHartTypes.h>

#include <list>
#include <string>
#include <iostream>


namespace hart7 {
namespace hostapp {


struct SourceRoute
{
	SourceRoute( int p_nDeviceId, 
		   uint8_t p_nRouteId,
		   char* p_pszDevices) :
		m_nDeviceId(p_nDeviceId),
		m_nRouteId(p_nRouteId)
		{
			m_toDelete = false;
			m_strDevices = p_pszDevices;
			time = CMicroSec().GetElapsedTimeStr();
		}
	SourceRoute( int p_nDeviceId, 
			uint8_t p_nRouteId) :
			m_nDeviceId(p_nDeviceId),
			m_nRouteId(p_nRouteId)
		{
			m_toDelete = true;
			time = CMicroSec().GetElapsedTimeStr();
		}

	//
	bool m_toDelete;  //used in caching process

	int m_nDeviceId;
	uint8_t m_nRouteId;
	std::string m_strDevices;
	
	std::string time;
};

typedef std::list<SourceRoute> SourceRoutesListT;

std::ostream& operator<< (std::ostream& p_rStream, const SourceRoute& p_rSrcRoute);

}
}

#endif //__SOURCEROUTE_H_
