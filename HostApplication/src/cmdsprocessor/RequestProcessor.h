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

#ifndef REQUESTPROCESSOR_H_
#define REQUESTPROCESSOR_H_

#include <nlib/log.h>

#include <WHartHost/cmdsprocessor/ProcessorExceptions.h>

#include <WHartHost/model/DBCommand.h>
#include <WHartHost/applicationcmds/IAppCommandVisitor.h>

namespace hart7 {
namespace hostapp {

class CommandsProcessor;

/*
 * Visitor Pattern
 */
class RequestProcessor : public IAppCommandVisitor
{

public:
	void Process(AbstractAppCommandPtr p_rRequest, CommandsProcessor& p_rProcessor);


private:
	AbstractAppCommandPtr m_pRequest;
	CommandsProcessor* m_pProcessor;

//topology 
private:
	void Visit(AppTopologyCommand& p_rTopologyCmd);
	void Visit(AppSetTopoNotificationCmd& p_rTopoNotificationCmd);

//notification bit mask
private:
	 void Visit(AppSetNotificationBitMask& p_rBitMaskCmd);

//notifications
private:
	void Visit(AppNoBurstRspNotification& p_rNotification);
	void Visit(AppTopologyNotification& p_rTopoNotification);
	void Visit(App000Cmd& p_rCmd000);
	void Visit(App020Cmd& p_rCmd020);
	void Visit(App769Cmd& p_rCmd769);
	void Visit(App787Cmd& p_rCmd787);
	void Visit(App785Cmd& p_rCmd785);
	void Visit(App788Cmd& p_rCmd788);
	void Visit(App789Cmd& p_rCmd789);
	void Visit(App790Cmd& p_rCmd790);
	void Visit(App791Cmd& p_rCmd791);
	void Visit(App814Cmd& p_rCmd814);
	void Visit(App832Cmd& p_rCmd832);
	void Visit(App833Cmd& p_rCmd833);
	void Visit(App834Cmd& p_rCmd834);
	void Visit(AppNotificationDevCmd& p_rNotifDevCmd);
	void Visit(AppReportsNotification& p_rReportsNotification);
	void Visit(AppDevConfigNotification& p_rDevConfigNotification);
	void Visit(AppBurstNotification& p_rBurstNotification);

//burst notif
private:
	void Visit(AppSetBurstNotificationCmd& p_rBurstNotificationCmd);
	void Visit(AppUnsetBurstNotificationCmd& p_rBurstNotificationCmd);
	
//burst config
private:
	void Visit(AppSetBurstConfigurationCmd& p_rBurstConfigurationCmd);
	
//read burst
private:
    void Visit(AppDiscoveryBurstConfigCmd& p_rDiscoveryBurstConfigCmd);

//read value
private:
	void Visit(AppReadValueCmd& p_rReadValue);
	
//general wh cmd
private:
	void Visit(AppGeneralCommand& p_rGeneralcmd);
	
//reports
private:
	void Visit(AppRoutesReportCmd& p_rRoutesCmd);
	void Visit(AppServicesReportCmd& p_rServicesCmd);
	void Visit(AppDeviceHealthReportCmd& p_rDeviceHealthCmd);
	void Visit(AppSuperframesReportCmd& p_rSuperframesCmd);
	void Visit(AppDeviceScheduleLinksReportCmd& p_rDeviceScheduleLinkCmd);
	void Visit(AppNeighborHealthReportCmd& p_rNeighborHealthCmd);

// firmware
private:
	void Visit(AppFirmwareTransfersCmd& p_rFirmwareTransfer);
};

}
}

#endif /*REQUESTPROCESSOR_H_*/
