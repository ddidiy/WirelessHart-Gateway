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


#include <WHartHost/applicationcmds/AppNoBurstRspNotification.h>
#include <WHartHost/applicationcmds/IAppCommandVisitor.h>


namespace hart7 {
namespace hostapp {

bool AppNoBurstRspNotification::Accept(IAppCommandVisitor& visitor)
{
	visitor.Visit(*this);
	return true;
}

void AppNoBurstRspNotification::DumpToStream(std::ostream& p_rStream) const
{
	p_rStream << "AppNoBurstRspNotification[CommandID=" << dbCommand.commandID << " Status=" << (int)dbCommand.commandStatus << "]";
}

void AppNoBurstRspNotification::SetCmdType(CMD_Type type)
{
	m_cmdType = type;
}
AppNoBurstRspNotification::CMD_Type AppNoBurstRspNotification::GetCmdType()
{
	return m_cmdType;
}
void AppNoBurstRspNotification::SetAddr(WHartUniqueID &addr)
{
	memcpy(m_addr.bytes, addr.bytes, sizeof(m_addr.bytes));
}
WHartUniqueID AppNoBurstRspNotification::GetAddr()
{
	return m_addr;
}


}
}
