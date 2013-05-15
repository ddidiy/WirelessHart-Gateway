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



#include <WHartHost/model/reports/Route.h>



namespace hart7 {
namespace hostapp {


std::ostream& operator<< (std::ostream& p_rStream, const Route& p_rRoute)
{
	p_rStream << "routeID=" << (int)p_rRoute.m_nRouteId << " deviceID=" << p_rRoute.m_nDeviceId
		<< " peerID=" << p_rRoute.m_nPeerId << " graphID=" << p_rRoute.m_nGraphId 
		<< " sourceRoute=" << (int)p_rRoute.m_nSourceRoute << " todelete=" << (p_rRoute.m_toDelete? "true" : "false");

	return p_rStream;
}




}
}
