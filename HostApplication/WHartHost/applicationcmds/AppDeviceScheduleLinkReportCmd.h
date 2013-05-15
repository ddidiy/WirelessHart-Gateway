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

#ifndef __APPDEVICESCHEDULELINKREPORTCMD_H_
#define __APPDEVICESCHEDULELINKREPORTCMD_H_


#include <list>

#include <WHartHost/applicationcmds/AbstractAppCommand.h>
#include <WHartHost/database/DevicesManager.h>
#include <WHartHost/model/reports/DeviceScheduleLink.h>

namespace hart7 {
namespace hostapp {

class IAppCommandVisitor;
class AppDeviceScheduleLinksReportCmd;

typedef boost::shared_ptr<AppDeviceScheduleLinksReportCmd> AppDeviceScheduleLinksReportCmdPtr;

class AppDeviceScheduleLinksReportCmd: public AbstractAppCommand
{
public:
	AppDeviceScheduleLinksReportCmd(std::list<std::pair<int, MAC> > &list);

	virtual bool Accept(IAppCommandVisitor& visitor);

protected:
	void DumpToStream(std::ostream& p_rStream) const;

public:
	void ReportReceived();
	bool ReceivedAllReports();

public:
	void AddDeviceScheduleLink(int p_nPozitie, DeviceScheduleLink& p_rLink);
	DevicesScheduleLinks& GetDevicesScheduleLinks();


private:
	int m_nNbOfNeededReports;
public :
	DevicesScheduleLinks m_oDevicesLinkList;


};

}
}

#endif //__APPDEVICESCHEDULELINKREPORTCMD_H_

