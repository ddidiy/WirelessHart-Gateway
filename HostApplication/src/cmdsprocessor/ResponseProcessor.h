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

#ifndef RESPONSEPROCESSOR_H_
#define RESPONSEPROCESSOR_H_

#include <WHartHost/cmdsprocessor/ProcessorExceptions.h>

#include <WHartHost/model/DBCommand.h>
#include <WHartHost/model/Device.h>
#include <WHartHost/applicationcmds/IAppCommandVisitor.h>
#include <WHartHost/model/DeviceReading.h>
#include <WHartHost/gateway/GatewayIO.h>

#include <nlib/log.h>


namespace hart7 {
namespace hostapp {


class CommandsProcessor;

/*
 *	Visitor Pattern
 */

class ResponseProcessor : public IAppCommandVisitor
{

public:
	void Process(gateway::GatewayIO::GWResponse &p_rResponse, CommandsProcessor& p_rProcessor);

private:
	CommandsProcessor* m_pProcessor;
	gateway::GatewayIO::GWResponse* m_pResponse;

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
	void Process769Notification(AppTopologyNotification& p_rTopoNotification);
	void Process785Notification(AppTopologyNotification& p_rTopoNotification);
	void Process814Notification(AppTopologyNotification& p_rTopoNotification);
	void Process832Notification(AppTopologyNotification& p_rTopoNotification);
	void Process833Notification(AppTopologyNotification& p_rTopoNotification);
	void Process834Notification(AppTopologyNotification& p_rTopoNotification);
	
	

	void Visit(App000Cmd& p_rCmd000);
	void Visit(App020Cmd& p_rCmd020);
	void Visit(App769Cmd& p_rCmd769);
	void Visit(App787Cmd& p_rCmd787);
	void Visit(App788Cmd& p_rCmd788);
	void Visit(App789Cmd& p_rCmd789);
	void Visit(App790Cmd& p_rCmd790);
	void Visit(App791Cmd& p_rCmd791);
	void Visit(App814Cmd& p_rCmd814);
	void Visit(App832Cmd& p_rCmd832);
	void Visit(App833Cmd& p_rCmd833);
	void Visit(App834Cmd& p_rCmd834);
	void Visit(App785Cmd& p_rCmd785);

	void Visit(AppNotificationDevCmd& p_rNotifDevCmd);
	void Visit(AppReportsNotification& p_rReportsNotification);
	void Visit(AppDevConfigNotification& p_rDevConfigNotification);
	void Visit(AppBurstNotification& p_rBurstNotification);
	


// set burst notif
private:
	void Visit(AppSetBurstNotificationCmd& p_rBurstNotificationCmd);
	void Visit(AppUnsetBurstNotificationCmd& p_rBurstNotificationCmd);

// set burst config
private:

	void Visit(AppSetBurstConfigurationCmd& p_rBurstConfigurationCmd);

	/*true if processing ok ; false if processing fails*/
	bool AppSetBurstConfigurationCmd_ProcessReadBurstConfigResponse(AppSetBurstConfigurationCmd& p_rBurstConfigurationCmd);

	/*true if processing ok ; false if processing fails*/
	bool AppSetBurstConfigurationCmd_ProcessTurnOffBurstMessagesResponse(AppSetBurstConfigurationCmd& p_rBurstConfigurationCmd);

	/*true if processing ok ; false if processing fails*/
	bool AppSetBurstConfigurationCmd_ProcessConfigureBurstMessagesResponse(AppSetBurstConfigurationCmd& p_rBurstConfigurationCmd);

	/*true if processing ok ; false if processing fails*/
	bool AppSetBurstConfigurationCmd_ProcessGetSubDeviceIndexResponse(AppSetBurstConfigurationCmd& p_rBurstConfigurationCmd);

// dicovery burst
private:
    void Visit(AppDiscoveryBurstConfigCmd& p_rAppCmd);
    bool AppDiscoveryBurstConfigCmd_ProcessResponse105(AppDiscoveryBurstConfigCmd& p_rAppCmd, bool burstActive);
    bool AppDiscoveryBurstConfigCmd_ProcessResponse009(AppDiscoveryBurstConfigCmd& p_rAppCmd);
    bool AppDiscoveryBurstConfigCmd_ProcessResponse084(AppDiscoveryBurstConfigCmd& p_rAppCmd, DevicePtr device);
    bool AppDiscoveryBurstConfigCmd_ProcessNextBurst(AppDiscoveryBurstConfigCmd& p_rAppCmd, DevicePtr device);

    void Visit(AppFirmwareTransfersCmd& p_rAppCmd);

//read value
private:
	void Visit(AppReadValueCmd& p_rReadValueCmd);
	bool ReadValueResponseError(AppReadValueCmd& p_rReadValueCmd);
	bool ProcessReadValueCmd1(AppReadValueCmd& p_rReadValueCmd);
	bool ProcessReadValueCmd2(AppReadValueCmd& p_rReadValueCmd);
	bool ProcessReadValueCmd3(AppReadValueCmd& p_rReadValueCmd);
    bool ProcessReadValueCmd9(AppReadValueCmd& p_rReadValueCmd);
    bool ProcessReadValueCmd33(AppReadValueCmd& p_rReadValueCmd);
    bool ProcessReadValueCmd178(AppReadValueCmd& p_rReadValueCmd);
	//fills p_rReading with the required info and returns its address
	DeviceReading* InitDeviceReading( DeviceReading& p_rReading, /*output param*/
		int p_nDeviceID, 
		int p_nCommandID, 
		int p_nChannelNo, 
		struct timeval&	p_rTv, 
		unsigned char p_nValueStatus,
		float p_flValue, 
		bool p_bHasStatus,
		DeviceReading::ReadingType p_enumReadingType);

//general wh cmd
private:
	void Visit(AppGeneralCommand& p_rGeneralCommand);
	
//reports
private:
	void Visit(AppRoutesReportCmd& p_rRoutesCmd);
	void SendReadSourceRouteRequest(const DevicePtr& p_rDevice, AppRoutesReportCmd& p_rRoutesCmd, int p_nRouteId);
	void ProcessRouteListReport(AppRoutesReportCmd& p_rRoutesCmd);
	void ProcessSourceRouteReport(AppRoutesReportCmd& p_rRoutesCmd);
	void Visit(AppServicesReportCmd& p_rServicesCmd);
	void Visit(AppDeviceHealthReportCmd& p_rDeviceHealthCmd);
	void Visit(AppSuperframesReportCmd& p_rSuperframesCmd);
	void Visit(AppDeviceScheduleLinksReportCmd& p_rDeviceScheduleLinkCmd);
	void Visit(AppNeighborHealthReportCmd& p_rNeighborHealthCmd);
//NoBurstReports
private:
	void Process000RespReport(AppNoBurstRspNotification& p_rNotification);
	void Process020RespReport(AppNoBurstRspNotification& p_rNotification);
	void Process769RespReport(AppNoBurstRspNotification& p_rNotification);
	void Process779RespReport(AppNoBurstRspNotification& p_rNotification);
	void Process780RespReport(AppNoBurstRspNotification& p_rNotification);
	void Process783RespReport(AppNoBurstRspNotification& p_rNotification);
	void Process784RespReport(AppNoBurstRspNotification& p_rNotification);
	void Process785RespReport(AppNoBurstRspNotification& p_rNotification);
	void Process787RespReport(AppNoBurstRspNotification& p_rNotification);
	void Process788RespReport(AppNoBurstRspNotification& p_rNotification);
	void Process789RespReport(AppNoBurstRspNotification& p_rNotification);
	void Process790RespReport(AppNoBurstRspNotification& p_rNotification);
	void Process791RespReport(AppNoBurstRspNotification& p_rNotification);
	void Process800RespReport(AppNoBurstRspNotification& p_rNotification);
	void Process801RespReport(AppNoBurstRspNotification& p_rNotification);
	void Process802RespReport(AppNoBurstRspNotification& p_rNotification);
	void Process803RespReport(AppNoBurstRspNotification& p_rNotification);
	void Process814RespReport(AppNoBurstRspNotification& p_rNotification);
	void Process818RespReport(AppNoBurstRspNotification& p_rNotification);
	void Process832RespReport(AppNoBurstRspNotification& p_rNotification);
	void Process833RespReport(AppNoBurstRspNotification& p_rNotification);
	void Process834RespReport(AppNoBurstRspNotification& p_rNotification);
	void Process965RespReport(AppNoBurstRspNotification& p_rNotification);
	void Process966RespReport(AppNoBurstRspNotification& p_rNotification);
	void Process967RespReport(AppNoBurstRspNotification& p_rNotification);
	void Process968RespReport(AppNoBurstRspNotification& p_rNotification);
	void Process969RespReport(AppNoBurstRspNotification& p_rNotification);
	void Process970RespReport(AppNoBurstRspNotification& p_rNotification);
	void Process971RespReport(AppNoBurstRspNotification& p_rNotification);
	void Process973RespReport(AppNoBurstRspNotification& p_rNotification);
	void Process974RespReport(AppNoBurstRspNotification& p_rNotification);
	void Process975RespReport(AppNoBurstRspNotification& p_rNotification);
	void Process976RespReport(AppNoBurstRspNotification& p_rNotification);
	void Process977RespReport(AppNoBurstRspNotification& p_rNotification);
};

} //namespace hostapp
} //namespace hart7

#endif /*RESPONSEPROCESSOR_H_*/
