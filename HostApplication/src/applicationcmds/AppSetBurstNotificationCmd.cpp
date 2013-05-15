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

#include <WHartHost/applicationcmds/AppSetBurstNotificationCmd.h>
#include <WHartHost/applicationcmds/IAppCommandVisitor.h>


namespace hart7 {
namespace hostapp {


AppSetBurstNotificationCmd::AppSetBurstNotificationCmd(	const PublishChannelSetT& p_rList, 
		const BurstMessageSetT& p_rburstMessageList, 
		const TriggerSetT& p_rtriggersList) :
		m_rList(p_rList),
		m_rburstMessageList(p_rburstMessageList),
		m_rtriggersList(p_rtriggersList)
{
}

bool AppSetBurstNotificationCmd::Accept(IAppCommandVisitor& visitor)
{
	visitor.Visit(*this);
	return true;
}


void AppSetBurstNotificationCmd::DumpToStream(std::ostream& p_rStream) const
{
	p_rStream << "AppSetBurstNotificationCmd[CommandID=" << dbCommand.commandID << " Status=" << (int)dbCommand.commandStatus << "]";
}

//
const PublishChannelSetT& AppSetBurstNotificationCmd::GetChannelList()
{ 
	return m_rList; 
}
const BurstMessageSetT& AppSetBurstNotificationCmd::GetBurstMessageList() 
{ 
	return m_rburstMessageList; 
}
const TriggerSetT& AppSetBurstNotificationCmd::GetTriggersList() 
{ 
	return m_rtriggersList; 
}


}
}

