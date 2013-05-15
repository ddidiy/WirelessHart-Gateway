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
#include <WHartHost/applicationcmds/AppReadValueCmd.h>


namespace hart7 {
namespace hostapp {

AppReadValueCmd::AppReadValueCmd ( uint16_t p_nCmdNo, uint16_t p_nDeviceId,
		DeviceReading::ReadingType p_enumReadingType, const PublishChannelSetT &p_rList,
		bool bypassIOCache /*= false*/) : m_nCmdNo(p_nCmdNo),
		m_nDeviceId(p_nDeviceId),
		m_enumReadingType(p_enumReadingType),
		m_oList(p_rList),
		m_bypassIOCache(bypassIOCache)
{
}

bool AppReadValueCmd::Accept(IAppCommandVisitor& visitor)
{
	visitor.Visit(*this);
	return true;
}

void AppReadValueCmd::DumpToStream(std::ostream& p_rStream) const
{
	p_rStream << "AppReadValueCmd[CommandID=" << dbCommand.commandID << " Status=" << (int)dbCommand.commandStatus << "]";
}

//
uint16_t AppReadValueCmd::GetCmdNo()
{ 
	return m_nCmdNo; 
}
int AppReadValueCmd::GetDeviceId() 
{ 
	return m_nDeviceId; 
}
DeviceReading::ReadingType AppReadValueCmd::GetReadingType()
{ 
	return m_enumReadingType; 
}
PublishChannelSetT& AppReadValueCmd::GetChannelList()
{ 
	return m_oList; 
}
bool AppReadValueCmd::GetBypassIOCache()
{
	return m_bypassIOCache;
}


}
}
