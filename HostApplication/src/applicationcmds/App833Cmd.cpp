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
// C++ Implementation: App833Cmd
//
// Description: 
//
//
// Author: Catrina Mihailescu <catrina@r2d2>, (C) 2010
//
//
//
#include <WHartHost/applicationcmds/IAppCommandVisitor.h>
#include <WHartHost/applicationcmds/App833Cmd.h>


namespace hart7 {
namespace hostapp {

App833Cmd::App833Cmd(int deviceID, const MAC &deviceMAC):m_deviceID(deviceID),
		m_deviceNeighborsHealth(deviceID,deviceMAC)
{
}


bool App833Cmd::Accept(IAppCommandVisitor& visitor)
{
	visitor.Visit(*this);
	return true;
}

void App833Cmd::DumpToStream(std::ostream& p_rStream) const
{
	p_rStream << "App833Cmd[CommandID=" << dbCommand.commandID << " Status=" << (int)dbCommand.commandStatus << "]";
}

//
int App833Cmd::GetFromDeviceID() 
{
	return m_deviceID;
}
std::list<NeighbourSignalLevel>& App833Cmd::GetNeighbSignalLevelList()
{
	return m_devNeighbSignalLevels;
}
DeviceNeighborsHealth& App833Cmd::GetNeighbHealthList()
{ 
	return m_deviceNeighborsHealth;
}


}
}
