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

#include <WHartHost/applicationcmds/AppNeighborHealthReportCmd.h>
#include <WHartHost/applicationcmds/IAppCommandVisitor.h>
#include <WHartHost/model/MAC.h>


namespace hart7 {
namespace hostapp {


AppNeighborHealthReportCmd::AppNeighborHealthReportCmd(std::list<std::pair<int, MAC> >& p_rDevicesList)
{
	
	if (p_rDevicesList.empty())
	{
		LOG_ERROR_APP("[AppNeighborHealthReportCmd]: we shouldn't get here!");
		assert(false);		//the list must not be empty
	}

	m_nNbOfNeededReports = p_rDevicesList.size();
	std::list<std::pair<int, MAC> >::iterator it = p_rDevicesList.begin();

	if (p_rDevicesList.size() == 1)
	{
		const WHartUniqueID &devAddr = it->second.Address();
		const WHartUniqueID &gwAddr = stack::Gateway_UniqueID();
		const WHartUniqueID &nnAddr = stack::NetworkManager_UniqueID();
		m_isDevice = false;
		if (memcmp(devAddr.bytes, gwAddr.bytes, sizeof(devAddr.bytes)) &&
			memcmp(devAddr.bytes, nnAddr.bytes, sizeof(devAddr.bytes)))
		{
			m_isDevice = true;
		}
		m_isToBeAccessPoint = false;
	}


	for( ; it != p_rDevicesList.end() ; ++it)
	{	DeviceNeighborsHealth devNeighbsHealth(it->first, it->second);
		m_oNeighborsHealthCache.push_back(devNeighbsHealth);
	}
}

bool AppNeighborHealthReportCmd::Accept(IAppCommandVisitor& visitor)
{
	visitor.Visit(*this);
	return true;
}


void AppNeighborHealthReportCmd::DumpToStream(std::ostream& p_rStream) const
{
	p_rStream << "AppNeighborHealthReportCmd[CommandID=" << dbCommand.commandID << " Status=" << (int)dbCommand.commandStatus << "]";
}

//
void AppNeighborHealthReportCmd::ReportReceived() 
{ 
	--m_nNbOfNeededReports; 
}
bool AppNeighborHealthReportCmd::ReceivedAllReports() 
{ 
	return m_nNbOfNeededReports <= 0;
}
DevicesNeighborsHealth& AppNeighborHealthReportCmd::GetDevicesNeighbHealth() 
{
	return m_oNeighborsHealthCache;
}

//
void AppNeighborHealthReportCmd::SetIsToBeAccessPoint()
{
	m_isToBeAccessPoint = true;
}
bool AppNeighborHealthReportCmd::IsToBeAccessPoint()
{
	return m_isToBeAccessPoint;	
}
bool AppNeighborHealthReportCmd::IsDevice()
{
	return m_isDevice;
}



}
}

