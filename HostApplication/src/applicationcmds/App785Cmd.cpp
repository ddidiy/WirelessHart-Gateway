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

#include <WHartHost/applicationcmds/IAppCommandVisitor.h>
#include <WHartHost/applicationcmds/App785Cmd.h>


namespace hart7 {
namespace hostapp {


App785Cmd::App785Cmd(int deviceID,const MAC& deviceMAC, bool isDevice):
	m_nDeviceID(deviceID), m_oMac(deviceMAC), m_isDevice(isDevice)
{
	m_isToBeAccessPoint = false;
}


bool App785Cmd::Accept(IAppCommandVisitor& visitor)
{
	visitor.Visit(*this);
	return true;
}


void App785Cmd::DumpToStream(std::ostream& p_rStream) const
{
	p_rStream << "App785Cmd[CommandID=" << dbCommand.commandID << " Status=" << (int)dbCommand.commandStatus << "]";
}


int App785Cmd::GetDeviceID()
{
	return m_nDeviceID;
}

std::list<GraphNeighbor>& App785Cmd::GetGraphList()
{
	return m_oGraphList;
}

MAC App785Cmd::GetDeviceMac()
{
	return m_oMac;
}
void App785Cmd::SetIsToBeAccessPoint()
{
	m_isToBeAccessPoint = true;
}
bool App785Cmd::IsToBeAccessPoint()
{
	return m_isToBeAccessPoint;	
}
bool App785Cmd::IsDevice()
{
	return m_isDevice;
}

}
}
