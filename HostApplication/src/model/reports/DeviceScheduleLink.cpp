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



#include <WHartHost/model/reports/DeviceScheduleLink.h>



namespace hart7 {
namespace hostapp {


std::ostream& operator<< (std::ostream& p_rStream, const DeviceScheduleLink& p_rDevSchedLink)
{
	p_rStream << "superframeID=" << p_rDevSchedLink.m_nSuperframeId << " m_nPeerId=" << p_rDevSchedLink.m_nPeerId
		<< " m_nSlotIndex=" << p_rDevSchedLink.m_nSlotIndex << " m_nChannelOffset=" << p_rDevSchedLink.m_nChannelOffset
		<< " m_nTransmit" << p_rDevSchedLink.m_nTransmit << " m_nReceive=" << p_rDevSchedLink.m_nReceive
		<< " m_nShared" << p_rDevSchedLink.m_nShared << " m_nLinkType" << p_rDevSchedLink.m_nLinkType 
		<< " todelete=" << (p_rDevSchedLink.m_toDelete? "true" : "false");

	return p_rStream;
}




}
}

