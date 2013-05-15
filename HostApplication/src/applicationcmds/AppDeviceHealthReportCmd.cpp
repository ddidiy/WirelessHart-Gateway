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

//
// C++ Implementation: AppDeviceHealthReportCmd
//
// Description: 
//
//
// Author: Catrina Mihailescu <catrina@r2d2>, (C) 2010
//
//
//


#include <WHartHost/applicationcmds/AppDeviceHealthReportCmd.h>
#include <WHartHost/applicationcmds/IAppCommandVisitor.h>

#include <nlib/log.h>

namespace hart7 {
namespace hostapp {


AppDeviceHealthReportCmd::AppDeviceHealthReportCmd(std::list<std::pair<int, MAC> > &list)
{

	if (list.size() == 0)
	{
		LOG_ERROR_APP("[AppDeviceHealthReportCmd]: we shouldn't get here!");
		assert(false);	//the list should not be empty
	}

	m_oDevicesHealth.reserve(list.size());

	for (std::list<std::pair<int, MAC> >::const_iterator i = list.begin(); i != list.end(); ++i)
	{
		m_oDevicesHealth.push_back(DeviceHealth());
		DeviceHealth &devHealth = *m_oDevicesHealth.rbegin();
		devHealth.m_nDeviceId = i->first;
		devHealth.m_oDeviceMac = i->second;
	}

	m_nNbOfNeededReports = list.size();
}

bool AppDeviceHealthReportCmd::Accept(IAppCommandVisitor& visitor)
{
	visitor.Visit(*this);
	return true;
}


void AppDeviceHealthReportCmd::DumpToStream(std::ostream& p_rStream) const
{
	p_rStream << "AppDeviceHealthReportCmd [CommandID=" << dbCommand.commandID << " Status=" << (int)dbCommand.commandStatus << "]";
}

//
void AppDeviceHealthReportCmd::ReportReceived()
{
	--m_nNbOfNeededReports; 
}
bool AppDeviceHealthReportCmd::ReceivedAllReports()
{ 
	return m_nNbOfNeededReports == 0;
}
std::vector<DeviceHealth>& AppDeviceHealthReportCmd::GetDevicesHealth()
{
	return m_oDevicesHealth;
}


}
}
