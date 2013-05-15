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

#include <WHartHost/applicationcmds/AppDeviceScheduleLinkReportCmd.h>
#include <WHartHost/applicationcmds/IAppCommandVisitor.h>


namespace hart7 {
namespace hostapp {


bool AppDeviceScheduleLinksReportCmd::Accept(IAppCommandVisitor& visitor)
{
	visitor.Visit(*this);
	return true;
}


void AppDeviceScheduleLinksReportCmd::DumpToStream(std::ostream& p_rStream) const
{
	p_rStream << "AppDeviceScheduleLinksReportCmd[CommandID=" << dbCommand.commandID << " Status=" << (int)dbCommand.commandStatus << "]";
}

AppDeviceScheduleLinksReportCmd::AppDeviceScheduleLinksReportCmd(std::list<std::pair<int, MAC> > &list)
{
	m_oDevicesLinkList.reserve(list.size());

	for(std::list<std::pair<int,MAC> >::const_iterator i = list.begin(); i != list.end(); ++i)
	{
		m_oDevicesLinkList.push_back(DevicesLinks(i->first, i->second));
	}

	m_nNbOfNeededReports = list.size();
}

//
void AppDeviceScheduleLinksReportCmd::ReportReceived()
{ 
	--m_nNbOfNeededReports; 
}
bool AppDeviceScheduleLinksReportCmd::ReceivedAllReports()
{ 
	return m_nNbOfNeededReports <= 0;
}

//
void AppDeviceScheduleLinksReportCmd::AddDeviceScheduleLink(int p_nPozitie, DeviceScheduleLink& p_rLink)
{ 
	m_oDevicesLinkList[p_nPozitie].linkList.push_back(p_rLink); 
}
DevicesScheduleLinks& AppDeviceScheduleLinksReportCmd::GetDevicesScheduleLinks()
{
	return m_oDevicesLinkList;
}


}
}

