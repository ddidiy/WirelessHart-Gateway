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
// C++ Implementation: AppGeneralCommand
//
// Description: 
//
//
// Author: Catrina Mihailescu <catrina@r2d2>, (C) 2010
//
//
//
#include <WHartHost/applicationcmds/IAppCommandVisitor.h>
#include <WHartHost/applicationcmds/AppGeneralCommand.h>

#include <boost/format.hpp>

namespace hart7 {
namespace hostapp {

AppGeneralCommand::AppGeneralCommand (int p_nCmdNo, std::string p_szDataBytes, int p_commandID, int p_deviceID, 
		bool p_bypassIOCache /*false*/) : m_nCmdNo(p_nCmdNo),  m_szDataBytes(p_szDataBytes), 
		m_nCommandID(p_commandID), m_nDeviceID(p_deviceID), m_bypassIOCache(p_bypassIOCache)
{
}

bool AppGeneralCommand::Accept(IAppCommandVisitor& visitor)
{
	visitor.Visit(*this);
	return true;
}

void AppGeneralCommand::DumpToStream(std::ostream& p_rStream) const
{
	p_rStream << "AppGeneralCommand[CommandID=" << dbCommand.commandID << " Status=" << (int)dbCommand.commandStatus << "]";
}

//
int AppGeneralCommand::GetCmdNo() 
{ 
	return m_nCmdNo; 
}
std::string AppGeneralCommand::GetDataBytes() 
{
	return m_szDataBytes; 
}
int AppGeneralCommand::GetCommandID() 
{ 
	return m_nCommandID; 
}
int AppGeneralCommand::GetDeviceID()
{ 
	return m_nDeviceID; 
}
bool AppGeneralCommand::GetBypassIOCache()
{
	return m_bypassIOCache;
}

}
}
