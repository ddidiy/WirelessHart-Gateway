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

#ifndef APPSETBURSTNOTIFICATIONCMD_H_
#define APPSETBURSTNOTIFICATIONCMD_H_


#include <WHartHost/applicationcmds/AbstractAppCommand.h>

namespace hart7 {
namespace hostapp {

/**
 * @brief Represents a set burst notif command from application perspective
 * Command Pattern
 */
class IAppCommandVisitor;

class AppSetBurstNotificationCmd: public AbstractAppCommand
{
public:
	AppSetBurstNotificationCmd(	const PublishChannelSetT& p_rList, 
		const BurstMessageSetT& p_rburstMessageList, 
		const TriggerSetT& p_rtriggersList);

	virtual bool Accept(IAppCommandVisitor& visitor);

protected:
	void DumpToStream(std::ostream& p_rStream) const;

public:
	const PublishChannelSetT& GetChannelList();
	const BurstMessageSetT& GetBurstMessageList();
	const TriggerSetT& GetTriggersList();

private:
	const PublishChannelSetT& m_rList;
	const BurstMessageSetT&	m_rburstMessageList;
	const TriggerSetT& m_rtriggersList;


};

}
}

#endif

