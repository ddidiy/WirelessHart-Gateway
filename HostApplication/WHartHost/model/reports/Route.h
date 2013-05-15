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

#ifndef __ROUTE_H_
#define __ROUTE_H_


#include <Shared/MicroSec.h>


#include <WHartStack/WHartTypes.h>

#include <list>
#include <iostream>


namespace hart7 {
namespace hostapp {

struct Route
{
	Route( uint8_t p_nRouteId, 
		   int p_nDeviceId, 
		   int p_nPeerId, 
		   int p_nGraphId, 
		   uint8_t p_nSourceRoute) :
		m_nRouteId(p_nRouteId),
		m_nDeviceId(p_nDeviceId),
		m_nPeerId(p_nPeerId),
		m_nGraphId(p_nGraphId),
		m_nSourceRoute(p_nSourceRoute)
		{m_toDelete=false;
		time = CMicroSec().GetElapsedTimeStr();}

	Route( uint8_t p_nRouteId, 
		int p_nDeviceId) :
		m_nRouteId(p_nRouteId),
		m_nDeviceId(p_nDeviceId)
		{m_toDelete=true;
		time = CMicroSec().GetElapsedTimeStr();}

	bool m_toDelete;  //used in caching process

	uint8_t m_nRouteId;
	int m_nDeviceId;
	int m_nPeerId;
	int m_nGraphId;
	uint8_t m_nSourceRoute;

	std::string time;
};

std::ostream& operator<< (std::ostream& p_rStream, const Route& p_rRoute);

typedef std::list<Route> RoutesListT;

}
}

#endif //__ROUTE_H_
