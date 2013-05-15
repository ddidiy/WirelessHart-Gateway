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



#include <WHartHost/model/reports/Service.h>



namespace hart7 {
namespace hostapp {


std::ostream& operator<< (std::ostream& p_rStream, const Service& p_rSignal)
{
	p_rStream << "m_nServiceId=" << p_rSignal.m_nServiceId << " m_nDeviceId=" << p_rSignal.m_nDeviceId
		<< " m_nPeerId=" << p_rSignal.m_nPeerId << " m_nApplicationDomain=" << (int)p_rSignal.m_nApplicationDomain
		<< " m_nSourceFlag=" << (int)p_rSignal.m_nSourceFlag << " m_nSinkFlag=" << (int)p_rSignal.m_nSinkFlag
		<< " m_nIntermittentFlag=" << (int)p_rSignal.m_nIntermittentFlag << " m_nPeriod=" << p_rSignal.m_nPeriod
		<< " m_nRouteId=" << (int)p_rSignal.m_nRouteId << " todelete=" << (p_rSignal.m_toDelete? "true" : "false");

	return p_rStream;
}




}
}

