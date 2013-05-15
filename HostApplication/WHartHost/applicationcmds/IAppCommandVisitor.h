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

#ifndef IAPPCOMMANDVISITOR_H_
#define IAPPCOMMANDVISITOR_H_

#include <WHartHost/applicationcmds/AbstractAppCommand.h>
#include <WHartHost/applicationcmds/AppTopologyCommand.h>
#include <WHartHost/applicationcmds/AppSetBurstNotificationCmd.h>
#include <WHartHost/applicationcmds/AppUnsetBurstNotificationCmd.h>
#include <WHartHost/applicationcmds/AppSetBurstConfigurationCmd.h>
#include <WHartHost/applicationcmds/AppSetTopoNotificationCmd.h>
#include <WHartHost/applicationcmds/AppSetNotificationBitMask.h>
#include <WHartHost/applicationcmds/AppNoBurstRspNotification.h>
#include <WHartHost/applicationcmds/AppTopologyNotification.h>
#include <WHartHost/applicationcmds/AppReportsNotification.h>
#include <WHartHost/applicationcmds/AppDevConfigNotification.h>
#include <WHartHost/applicationcmds/AppBurstNotification.h>
#include <WHartHost/applicationcmds/AppReadValueCmd.h>
#include <WHartHost/applicationcmds/App000Cmd.h>
#include <WHartHost/applicationcmds/App020Cmd.h>
#include <WHartHost/applicationcmds/App769Cmd.h>
#include <WHartHost/applicationcmds/App785Cmd.h>
#include <WHartHost/applicationcmds/App787Cmd.h>
#include <WHartHost/applicationcmds/App788Cmd.h>
#include <WHartHost/applicationcmds/App789Cmd.h>
#include <WHartHost/applicationcmds/App790Cmd.h>
#include <WHartHost/applicationcmds/App791Cmd.h>
#include <WHartHost/applicationcmds/App814Cmd.h>
#include <WHartHost/applicationcmds/App832Cmd.h>
#include <WHartHost/applicationcmds/App833Cmd.h>
#include <WHartHost/applicationcmds/App834Cmd.h>
#include <WHartHost/applicationcmds/AppGeneralCommand.h>
#include <WHartHost/applicationcmds/AppNotificationDevCmd.h>
#include <WHartHost/applicationcmds/AppRoutesReportCmd.h>
#include <WHartHost/applicationcmds/AppServicesReportCmd.h>
#include <WHartHost/applicationcmds/AppDeviceHealthReportCmd.h>
#include <WHartHost/applicationcmds/AppSuperframesReportCmd.h>
#include <WHartHost/applicationcmds/AppDeviceScheduleLinkReportCmd.h>
#include <WHartHost/applicationcmds/AppNeighborHealthReportCmd.h>
#include <WHartHost/applicationcmds/AppDiscoveryBurstConfigCmd.h>
#include <WHartHost/applicationcmds/AppFirmwareTransfersCmd.h>

namespace hart7 {
namespace hostapp {


/**
 * @brief The Visitor class.
 */
class IAppCommandVisitor
{
public:
	virtual ~IAppCommandVisitor()
	{
	}

	virtual void Visit(AppTopologyCommand& p_rTopologyCmd) = 0;
	virtual void Visit(AppSetBurstNotificationCmd& p_rBurstNotificationCmd) = 0;
	virtual void Visit(AppUnsetBurstNotificationCmd& p_rBurstNotificationCmd) = 0;
	virtual void Visit(AppSetBurstConfigurationCmd& p_rBurstConfigurationCmd) = 0;
	virtual void Visit(AppSetTopoNotificationCmd& p_rTopoNotificationCmd) = 0;
	virtual void Visit(AppSetNotificationBitMask& p_rBitMaskCmd) = 0;
	virtual void Visit(AppNoBurstRspNotification& p_rNotification) = 0;
	virtual void Visit(AppTopologyNotification& p_rTopoNotification) = 0;
	virtual void Visit(AppReportsNotification& p_rReportsNotification) = 0;
	virtual void Visit(AppDevConfigNotification& p_rDevConfigNotification) = 0;
	virtual void Visit(AppBurstNotification& p_rBurstNotification) = 0;
	virtual void Visit(AppReadValueCmd& p_rReadValue) = 0;
	virtual void Visit(App000Cmd& p_rCmd0) = 0;
	virtual void Visit(App020Cmd& p_rCmd020) = 0;
	virtual void Visit(App769Cmd& p_rCmd769) = 0;
	virtual void Visit(App785Cmd& p_rCmd785) = 0;
	virtual void Visit(App787Cmd& p_rCmd787) = 0;
	virtual void Visit(App788Cmd& p_rCmd788) = 0;
	virtual void Visit(App789Cmd& p_rCmd789) = 0;
	virtual void Visit(App790Cmd& p_rCmd790) = 0;
	virtual void Visit(App791Cmd& p_rCmd791) = 0;
	virtual void Visit(App814Cmd& p_rCmd814) = 0;
	virtual void Visit(App832Cmd& p_rCmd832) = 0;
	virtual void Visit(App833Cmd& p_rCmd833) = 0;
	virtual void Visit(App834Cmd& p_rCmd834) = 0;	
	virtual void Visit(AppGeneralCommand& p_rGeneralcmd) = 0;
	virtual void Visit(AppNotificationDevCmd& p_rNotifDevCmd) = 0;
	virtual void Visit(AppRoutesReportCmd& p_rRoutesCmd) = 0;
	virtual void Visit(AppServicesReportCmd& p_rServicesCmd) = 0;
	virtual void Visit(AppDeviceHealthReportCmd& p_rDeviceHealthCmd) = 0;
	virtual void Visit(AppSuperframesReportCmd& p_rSuperframesCmd) = 0;
	virtual void Visit(AppDeviceScheduleLinksReportCmd& p_rDeviceScheduleLinkCmd) = 0;
	virtual void Visit(AppNeighborHealthReportCmd& p_rNeighborHealthCmd) = 0;
	virtual void Visit(AppDiscoveryBurstConfigCmd& p_rDiscoveryBurstConfigCmd) = 0;
	virtual void Visit(AppFirmwareTransfersCmd& p_rFwTransferCmd) = 0;

};


}  // namespace hostapp
}  // namespace hart7

#endif /* IGSERVICEVISITOR_H_ */
