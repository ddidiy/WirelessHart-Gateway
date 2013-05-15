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


#ifndef APP833CMD_H_
#define APP833CMD_H_


#include <WHartHost/applicationcmds/AbstractAppCommand.h>

#include <WHartHost/model/MAC.h>
#include <WHartHost/model/reports/NeighborHealth.h>
#include <WHartHost/model/reports/NeighbSignalLevels.h>


namespace hart7 {
namespace hostapp {

/**
 * @brief Represents a topology command from application perspective
 * Command Pattern
 */
class IAppCommandVisitor;
class App833Cmd;
typedef boost::shared_ptr<App833Cmd> App833CmdPtr;

class App833Cmd: public AbstractAppCommand
{
public:
	App833Cmd(int deviceID, const MAC &deviceMAC);
	virtual bool Accept(IAppCommandVisitor& visitor);

protected:
	void DumpToStream(std::ostream& p_rStream) const;


public:
	int GetFromDeviceID();
	std::list<NeighbourSignalLevel>& GetNeighbSignalLevelList();
	DeviceNeighborsHealth& GetNeighbHealthList();


private:
	int m_deviceID;
	std::list<NeighbourSignalLevel> m_devNeighbSignalLevels;
	DeviceNeighborsHealth m_deviceNeighborsHealth;
};

}
}

#endif
