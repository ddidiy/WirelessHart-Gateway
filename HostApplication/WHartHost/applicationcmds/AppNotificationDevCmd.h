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


#ifndef APPNOTIFICATIONDEVCMD_H_
#define APPNOTIFICATIONDEVCMD_H_


#include <WHartHost/applicationcmds/AbstractAppCommand.h>
#include <WHartHost/model/NotificationDevInfo.h>


namespace hart7 {
namespace hostapp {

class AppNotificationDevCmd: public AbstractAppCommand
{
public:
	enum NotificationState
	{
		GetNeighbours_state = 0,
		GetGraphs_state,
		GetNickname_state,
		GetInfo_state
	};

public:
	AppNotificationDevCmd(MAC& p_rDeviceMac);
	virtual bool Accept(IAppCommandVisitor& visitor);

protected:
	void DumpToStream(std::ostream& p_rStream) const;

public:
	NotificationDevInfo m_devInfo;

public:
	NotificationState m_state;

};




}
}


#endif

