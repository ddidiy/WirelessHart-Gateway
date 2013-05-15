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

#ifndef APPNOBURSTRSPNOTIFICATION_H_
#define APPNOBURSTRSPNOTIFICATION_H_


/* DO NOT USE THIS COMMAND FOR BURST_MSG BECAUSE THERE SHOULD BE AN OPTIMIZED WAY FOR THAT*/


#include <WHartHost/applicationcmds/AbstractAppCommand.h>


namespace hart7 {
namespace hostapp {

/**
 * @brief Represents a topology command from application perspective
 * Command Pattern
 */
class IAppCommandVisitor;

class AppNoBurstRspNotification;
typedef boost::shared_ptr<AppNoBurstRspNotification> AppNoBurstRspNotificationPtr;

class AppNoBurstRspNotification: public AbstractAppCommand
{
public:
	virtual bool Accept(IAppCommandVisitor& visitor);

protected:
	void DumpToStream(std::ostream& p_rStream) const;

public:
	enum CMD_Type
	{
		CMD_Report_000 = 1,
		CMD_Report_020,
		CMD_Report_769,
		CMD_Report_779,
		CMD_Report_780,
		CMD_Report_783,
		CMD_Report_784,
		CMD_Report_785,
		CMD_Report_787,
		CMD_Alert_788,
		CMD_Alert_789,
		CMD_Alert_790,
		CMD_Alert_791,		
		CMD_Report_800,
		CMD_Report_801,
		CMD_Report_802,
		CMD_Report_803,
		CMD_Report_814,
		CMD_Report_818,
		CMD_Report_832,
		CMD_Report_833,
		CMD_Report_834,
		CMD_Report_965,
		CMD_Report_966,
		CMD_Report_967,
		CMD_Report_968,
		CMD_Report_969,
		CMD_Report_970,
		CMD_Report_971,
		CMD_Report_973,
		CMD_Report_974,
		CMD_Report_975,
		CMD_Report_976,
		CMD_Report_977
		//continue for more..
	};

public:
	void SetCmdType(CMD_Type type);
	CMD_Type GetCmdType();
	void SetAddr(WHartUniqueID &addr);
	WHartUniqueID GetAddr();

private:
	WHartUniqueID	m_addr;
	CMD_Type		m_cmdType;

};

}
}

#endif
