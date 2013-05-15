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

#ifndef __APPNEIGHBORHEALTHREPORTCMD_H_
#define __APPNEIGHBORHEALTHREPORTCMD_H_

#include <list>
#include <vector>
#include <utility>

#include <WHartHost/applicationcmds/AbstractAppCommand.h>
#include <WHartHost/database/DevicesManager.h>
#include <WHartHost/model/reports/NeighborHealth.h>
#include <WHartHost/model/MAC.h>

namespace hart7 {
namespace hostapp {

class IAppCommandVisitor;
class AppNeighborHealthReportCmd;

typedef boost::shared_ptr<AppNeighborHealthReportCmd> AppNeighborHealthReportCmdPtr;

class AppNeighborHealthReportCmd: public AbstractAppCommand
{
public:
	AppNeighborHealthReportCmd(std::list<std::pair<int, MAC> >& p_rDevicesList);
	virtual bool Accept(IAppCommandVisitor& visitor);

protected:
	void DumpToStream(std::ostream& p_rStream) const;

public:
	void ReportReceived();
	bool ReceivedAllReports();
	DevicesNeighborsHealth& GetDevicesNeighbHealth();

//if the command is sent just for one device
public:
	void SetIsToBeAccessPoint();
	bool IsToBeAccessPoint();
	bool IsDevice();

private:
	int m_nNbOfNeededReports;

public:
	DevicesNeighborsHealth m_oNeighborsHealthCache;

private:
	bool m_isDevice;			//try to find out if device is accessPoint from neighbInfo
	bool m_isToBeAccessPoint;	//try to find out if device is accessPoint from neighbInfo
};

}
}

#endif //__APPNEIGHBORHEALTHREPORTCMD_H_

