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



#include <WHartHost/model/reports/Superframe.h>



namespace hart7 {
namespace hostapp {


std::ostream& operator<< (std::ostream& p_rStream, const Superframe& p_rSuperframe)
{
	p_rStream << "m_nSuperframeId=" << p_rSuperframe.m_nSuperframeId << " m_nDeviceId=" << p_rSuperframe.m_nDeviceId
		<< " m_nNumberOfTimeSlots=" << p_rSuperframe.m_nNumberOfTimeSlots << " m_nActive=" << p_rSuperframe.m_nActive
		<< " m_nHandheldSuperframe=" << p_rSuperframe.m_nHandheldSuperframe 
		<< " todelete=" << (p_rSuperframe.m_toDelete? "true" : "false");

	return p_rStream;
}




}
}

