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

#include "RequestProcessor.h"

#include "Common.h"

#include <WHartHost/cmdsprocessor/CommandsProcessor.h>
#include "SaveRespErr.h"

#include <fstream>
#include <nlib/exception.h>




namespace hart7 {
namespace hostapp {


void RequestProcessor::Process(AbstractAppCommandPtr p_rRequest, CommandsProcessor& p_rProcessor)
{
	m_pProcessor = &p_rProcessor;
	m_pRequest = p_rRequest;
	m_pRequest->Accept(*this);
}


//topology
void RequestProcessor::Visit(AppTopologyCommand& p_rTopologyCmd)
{
    ////LOG_INFO_APP("[RequestProcessor::Visit]: AppTopologyCommand");
	//get uniqueDeviceIDs
	C814_ReadDeviceListEntries_Req	whReq;
	whReq.m_ucDeviceListCode = 0 /*= Active device list. There is no implementation for: 1 = Whitelisted devices, 2 = Blacklisted devices*/;
	whReq.m_ucNoOfListEntriesToRead = MAX_DEVICE_LIST_ENTRIES_NO;
	whReq.m_unStartingListIndex = 0;

	stack::WHartCommand whCmd;
	whCmd.commandID = CMDID_C814_ReadDeviceListEntries;
	whCmd.command = &whReq;
	
	gateway::GatewayIO::AppData appData;
	appData.appCmd = m_pRequest;
	//LOG_INFO_APP("[RequestProcessor::SendWHRequest]:C814_ReadDeviceListEntries_Req. Device=GW");
	m_pProcessor->gateway.SendWHRequest(appData, stack::Gateway_UniqueID(), whCmd, false, true);
	
}
void RequestProcessor::Visit(AppSetTopoNotificationCmd& p_rTopoNotificationCmd)
{

	DevicePtr devGW = p_rTopoNotificationCmd.pDevices->GatewayDevice();
    //LOG_INFO_APP("[RequestProcessor::Visit]: AppSetTopoNotificationCmd. Device=" << devGW->Mac());

	C837_WriteUpdateNotificationBitMask_Req whReq;
	memcpy(whReq.UniqueID, devGW->Mac().Address().bytes, sizeof(WHartUniqueID));

	unsigned short notifBitMask = devGW->GetNotificationBitMask();

	notifBitMask = notifBitMask | NotificationMaskCodesMask_NetworkTopology | 
						NotificationMaskCodesMask_NetworkSchedule | 
						NotificationMaskCodesMask_DeviceConfiguration;

	devGW->SetNotificationBitMask(notifBitMask);

	whReq.ChangeNotificationFlags = notifBitMask;

	gateway::GatewayIO::AppData appData;
	appData.appCmd = m_pRequest;
	appData.appCmd->pDevices = p_rTopoNotificationCmd.pDevices;
	appData.appCmd->pCommands = p_rTopoNotificationCmd.pCommands;

	hart7::stack::CHartCmdWrapper::Ptr pCmdReq(new hart7::stack::CHartCmdWrapper);

	pCmdReq->LoadParsed(CMDID_C837_WriteUpdateNotificationBitMask,sizeof(whReq),&whReq);

	hart7::stack::TMetaCmdsInfo oMetaInfo;

	oMetaInfo.m_u16MetaCmdId = CMDID_C64765_NivisMetaCommand;
	oMetaInfo.m_oMetaCmdUniqueId = devGW->Mac().Address();
	oMetaInfo.m_oCmdNivisMeta.m_u16ExtDeviceType = stack::NetworkManager_Nickname();
	memcpy(oMetaInfo.m_oInnerCmdUniqueId.bytes,whReq.UniqueID,sizeof(WHartUniqueID));

	pCmdReq->MetaCmdInfoSet(&oMetaInfo);

	hart7::stack::CHartCmdWrapperList l;
	l.push_back(pCmdReq);

	//LOG_INFO_APP("[RequestProcessor::SendWHRequest]:CMDID_C837_WriteUpdateNotificationBitMask. Device=GW");
	m_pProcessor->gateway.SendWHRequest(appData, devGW->Mac().Address(), l);
}


//notification bit mask
void RequestProcessor::Visit(AppSetNotificationBitMask& p_rBitMaskCmd)
{
	//this is only a response from gw
	assert(false);
}

//notifications
void RequestProcessor::Visit(AppNoBurstRspNotification& p_rNotification)
{

}
void RequestProcessor::Visit(AppTopologyNotification& p_rTopoNotification)
{
	//this is only a response from gw
	assert(false);
}
void RequestProcessor::Visit(App000Cmd& p_rCmd000)
{
	//this is only a response from gw
	assert(false);
}
void RequestProcessor::Visit(App020Cmd& p_rCmd020)
{
	//this is only a response from gw
	assert(false);
}
void RequestProcessor::Visit(App769Cmd& p_rCmd769)
{
	//this is only a response from gw
	assert(false);
}
void RequestProcessor::Visit(App814Cmd& p_rCmd814)
{
	//this is only a response from gw
	assert(false);
}

void RequestProcessor::Visit(App785Cmd& p_rCmd785)
{
	//this is only a response from gw
	LOG_WARN_APP("[RequestProcessor] Invalid App785Cmd request");
}

void RequestProcessor::Visit(App787Cmd& p_rCmd787)
{
	//this is only a response from gw
	assert(false);
}
void RequestProcessor::Visit(App788Cmd& p_rCmd788)
{
	//this is only a response from gw
	assert(false);
}
void RequestProcessor::Visit(App789Cmd& p_rCmd789)
{
	//this is only a response from gw
	assert(false);
}
void RequestProcessor::Visit(App790Cmd& p_rCmd790)
{
	//this is only a response from gw
	assert(false);
}
void RequestProcessor::Visit(App791Cmd& p_rCmd791)
{
	//this is only a response from gw
	assert(false);
}
void RequestProcessor::Visit(App832Cmd& p_rCmd832)
{
	//this is only a response from gw
	assert(false);
}
void RequestProcessor::Visit(App833Cmd& p_rCmd833)
{
	//this is only a response from gw
	assert(false);
}
void RequestProcessor::Visit(App834Cmd& p_rCmd834)
{
	//this is only a response from gw
	assert(false);
}
void RequestProcessor::Visit(AppNotificationDevCmd& p_rNotifDevCmd)
{
	//this is only a response from gw
	assert(false);
}
void RequestProcessor::Visit(AppReportsNotification& p_rReportsNotification)
{
	//this is only a response from gw
	assert(false);
}
void RequestProcessor::Visit(AppDevConfigNotification& p_rDevConfigNotification)
{
	//this is only a response from gw
	assert(false);
}
void RequestProcessor::Visit(AppBurstNotification& p_rBurstNotification)
{
	//this is only a response from gw
	assert(false);
}


//burst
void RequestProcessor::Visit(AppUnsetBurstNotificationCmd& p_rBurstNotificationCmd)
{
    //LOG_INFO_APP("[RequestProcessor::Visit]: AppUnsetBurstNotificationCmd. Device=" << p_rBurstNotificationCmd.pDevices->FindDevice(p_rBurstNotificationCmd.dbCommand.deviceID)->Mac());

	C837_WriteUpdateNotificationBitMask_Req whReq;
	memcpy(whReq.UniqueID, p_rBurstNotificationCmd.pDevices->FindDevice(p_rBurstNotificationCmd.dbCommand.deviceID)->Mac().Address().bytes, sizeof(WHartUniqueID));
	unsigned short notifBitMask = p_rBurstNotificationCmd.pDevices->FindDevice(p_rBurstNotificationCmd.dbCommand.deviceID)->GetNotificationBitMask();
	notifBitMask = notifBitMask & (~NotificationMaskCodesMask_BurstMode);
	p_rBurstNotificationCmd.pDevices->FindDevice(p_rBurstNotificationCmd.dbCommand.deviceID)->SetNotificationBitMask(notifBitMask);
	whReq.ChangeNotificationFlags = notifBitMask;

	stack::WHartCommand whCmd;
	whCmd.commandID = CMDID_C837_WriteUpdateNotificationBitMask;
	whCmd.command = &whReq;

	gateway::GatewayIO::AppData appData;
	appData.appCmd = m_pRequest;

	//LOG_INFO_APP("[RequestProcessor::SendWHRequest]:CMDID_C837_WriteUpdateNotificationBitMask. Device=GW");
	m_pProcessor->gateway.SendWHRequest(appData, stack::Gateway_UniqueID(), whCmd, false, true);
	
}

void RequestProcessor::Visit(AppSetBurstNotificationCmd& p_rBurstNotificationCmd)
{
//    LOG_INFO_APP("[SetBurstNotification]: Device=" << p_rBurstNotificationCmd.pDevices->FindDevice(p_rBurstNotificationCmd.dbCommand.deviceID)->Mac());

	DevicePtr device = p_rBurstNotificationCmd.pDevices->FindDevice(p_rBurstNotificationCmd.dbCommand.deviceID);
	device->SetHasNotification(true);
    try
    {
        p_rBurstNotificationCmd.pDevices->ProcessDBChannels(p_rBurstNotificationCmd.dbCommand.deviceID, const_cast<PublishChannelSetT&>(p_rBurstNotificationCmd.GetChannelList()));
        p_rBurstNotificationCmd.pDevices->ProcessDBTriggers(p_rBurstNotificationCmd.dbCommand.deviceID, const_cast<TriggerSetT&>(p_rBurstNotificationCmd.GetTriggersList()));
        std::list<BurstMessage> added, updated, deleted;
        p_rBurstNotificationCmd.pDevices->ProcessDBBursts(p_rBurstNotificationCmd.dbCommand.deviceID, const_cast<BurstMessageSetT&> (p_rBurstNotificationCmd.GetBurstMessageList()), added, updated, deleted);
        MAC mac = device->Mac();
        p_rBurstNotificationCmd.pDevices->UpdatePublishersCache(mac, device->GetPublisherInfo());

        //this is the way we should do it because it is the first time
        added.clear();
        deleted.clear();
        updated.clear();
        const BurstMessageSetT &bursts = p_rBurstNotificationCmd.GetBurstMessageList();
        for (std::set<BurstMessage>::const_iterator i = bursts.begin(); i != bursts.end(); ++i)
        {
            added.push_back(*i);
        }
        p_rBurstNotificationCmd.pDevices->PubDeviceBurstsChangeReceived(device->id, added, deleted, updated);
        LOG_INFO_APP("[SetBurstNotification]: device id=" << device->id << " is going to subscribe for notification");
    }
    catch (std::exception& ex)
    {
        LOG_ERROR_APP("[SetBurstNotification]: failed to update db for the publisher id= " << device->id);
        device->IssueSetBurstNotificationCmd = true;
        throw ex;
    }

    try {
        device->SetNotificationBitMask(device->GetNotificationBitMask() | NotificationMaskCodesMask_BurstMode);

        C837_WriteUpdateNotificationBitMask_Req whReq837;
        memcpy(whReq837.UniqueID, device->Mac().Address().bytes, sizeof(WHartUniqueID));
        whReq837.ChangeNotificationFlags = device->GetNotificationBitMask();

        stack::WHartCommand whCmd837;
        whCmd837.commandID = CMDID_C837_WriteUpdateNotificationBitMask;
        whCmd837.command = &whReq837;
        gateway::GatewayIO::AppData appData;
        appData.appCmd = m_pRequest;
		m_pProcessor->gateway.SendWHRequest(appData, stack::Gateway_UniqueID(), whCmd837, false, true);
        LOG_INFO_APP("[SetBurstNotification]: CMDID_C837_WriteUpdateNotificationBitMask. Device=GW");

		// send CMD050 to device for getting device variable assignments
        stack::WHartCommand whCmd050;
        C050_ReadDynamicVariableAssignments_Req  whReq050;
        whCmd050.commandID = CMDID_C050_ReadDynamicVariableAssignments;
        whCmd050.command = &whReq050;
        m_pProcessor->gateway.SendWHRequest(appData, device->Mac().Address(), whCmd050, false, true);
        LOG_INFO_APP("[SetBurstNotification]: CMDID_C050_ReadDynamicVariableAssignments - deviceMAC=" << device->Mac());
    }
	catch (std::exception e)
	{
		LOG_ERROR_APP("[SetBurstNotification]: failed to send command");
		p_rBurstNotificationCmd.pDevices->FindDevice(p_rBurstNotificationCmd.dbCommand.deviceID)->IssueSetBurstNotificationCmd = true;
		throw e;
	}
	
}

void RequestProcessor::Visit(AppSetBurstConfigurationCmd& p_rBurstConfigurationCmd)
{
	DevicePtr dev = p_rBurstConfigurationCmd.GetDevice();
    //LOG_INFO_APP("[RequestProcessor::Visit]: AppSetBurstConfigurationCmd. Device=" << dev->Mac());

	if (!dev)
	{	LOG_ERROR_APP("[AppSetBurstConfiguration]: device id=" << p_rBurstConfigurationCmd.dbCommand.deviceID << "not found");
		SaveRespErr(*p_rBurstConfigurationCmd.pCommands, p_rBurstConfigurationCmd.dbCommand).CommandFailed(DBCommand::rsFailure_InvalidDevice);
		return;
	}

	
	MAC mac = dev->Mac();

	//make sure we're in the initial state
	p_rBurstConfigurationCmd.SetState(AppSetBurstConfigurationCmd::SET_BURSTCONF_READ_CONFIG_STATE);

	bool reqWasSent = false;

	switch(p_rBurstConfigurationCmd.GetState())
	{
	case AppSetBurstConfigurationCmd::SET_BURSTCONF_READ_CONFIG_STATE:
	    LOG_INFO_APP("[SetBurstConfiguration]: REQUEST - SET_BURSTCONF_READ_CONFIG_STATE - deviceMAC=" << p_rBurstConfigurationCmd.GetDevice()->Mac());

	    p_rBurstConfigurationCmd.SetState(AppSetBurstConfigurationCmd::SET_BURSTCONF_READ_CONFIG_STATE);
		reqWasSent = AppSetBurstConfigurationCmd_SendReadBurstConfigCmd(p_rBurstConfigurationCmd, m_pRequest, m_pProcessor);
		if (reqWasSent)
		{
		    break;
		}
		//else drop in next state
	case AppSetBurstConfigurationCmd::SET_BURSTCONF_TURNOFF_BURSTMESSAGES_STATE:
        LOG_INFO_APP("[SetBurstConfiguration]: REQUEST - SET_BURSTCONF_TURNOFF_BURSTMESSAGES_STATE - deviceMAC=" << p_rBurstConfigurationCmd.GetDevice()->Mac());

        p_rBurstConfigurationCmd.SetState(AppSetBurstConfigurationCmd::SET_BURSTCONF_TURNOFF_BURSTMESSAGES_STATE);
		reqWasSent = AppSetBurstConfigurationCmd_SendTurnOffBurstMessagesCmd(p_rBurstConfigurationCmd, m_pRequest, m_pProcessor);
		if (reqWasSent)
		    break;
		//else drop in next state
    case AppSetBurstConfigurationCmd::SET_BURSTCONF_GET_SUBDEVICEINDEX_STATE:
        if (dev->IsAdapter())
        {
            LOG_INFO_APP("[SetBurstConfiguration]: REQUEST - SET_BURSTCONF_GET_SUBDEVICEINDEX_STATE - deviceMAC=" << p_rBurstConfigurationCmd.GetDevice()->Mac());

            p_rBurstConfigurationCmd.SetState(AppSetBurstConfigurationCmd::SET_BURSTCONF_GET_SUBDEVICEINDEX_STATE);
            reqWasSent = AppSetBurstConfigurationCmd_SendGetSubDeviceCountCmd(p_rBurstConfigurationCmd, m_pRequest, m_pProcessor);
            if (reqWasSent)
                break;
            //else drop in next state
        }
	case AppSetBurstConfigurationCmd::SET_BURSTCONF_CONFIGURE_BURSTMESSAGES_STATE:
        LOG_INFO_APP("[SetBurstConfiguration]: REQUEST - SET_BURSTCONF_CONFIGURE_BURSTMESSAGES_STATE - deviceMAC=" << p_rBurstConfigurationCmd.GetDevice()->Mac());

	    p_rBurstConfigurationCmd.SetState(AppSetBurstConfigurationCmd::SET_BURSTCONF_CONFIGURE_BURSTMESSAGES_STATE);
        reqWasSent = AppSetBurstConfigurationCmd_SendConfigureBurstMessagesCmd(p_rBurstConfigurationCmd, m_pRequest, m_pProcessor);
        break;
	}

	if (!reqWasSent)
	{	LOG_WARN_APP("[SetBurstConfiguration]: burst configuration already set for deviceMAC=" << dev->Mac().ToString());
		AppSetBurstConfigurationCmd_Finished(p_rBurstConfigurationCmd);
		p_rBurstConfigurationCmd.pCommands->SetCommandResponded(p_rBurstConfigurationCmd.dbCommand, nlib::CurrentUniversalTime(), "success", NULL);
	}

	p_rBurstConfigurationCmd.pDevices->UpdatePublishersCache(mac, dev->GetPublisherInfo());
}


//read value
void RequestProcessor::Visit(AppReadValueCmd& p_rReadValue)
{
    DevicePtr dev = p_rReadValue.pDevices->FindDevice(p_rReadValue.GetDeviceId());
    //LOG_INFO_APP("[RequestProcessor::Visit]: AppReadValueCmd. Device=" << dev->Mac());

	int cmdNo = p_rReadValue.GetCmdNo();

	stack::WHartCommand whCmd;
	whCmd.commandID = cmdNo;

	whCmd.command = BuildReadValueCmdRequest(p_rReadValue.GetChannelList());
	
	if (whCmd.command == 0)
		assert(false); //this verification should be done before -> see the function which test the db input parameters

	gateway::GatewayIO::AppData appData;
	appData.appCmd = m_pRequest;

	//LOG_INFO_APP("[RequestProcessor::SendWHRequest]:" << cmdNo << ". Device=" << dev->Mac());
	m_pProcessor->gateway.SendWHRequest(appData, dev->Mac().Address(), whCmd, p_rReadValue.GetBypassIOCache(), true);
}


//general wh cmd
void RequestProcessor::Visit(AppGeneralCommand& p_rGeneralcmd)
{
    int deviceID = p_rGeneralcmd.GetDeviceID();
    DevicePtr device = p_rGeneralcmd.pDevices->FindDevice(deviceID);
    //LOG_INFO_APP("[RequestProcessor::Visit]: AppGeneralCommand. Device=" << device->Mac());

	LOG_INFO_APP("REQUEST for general command with cmdNo: " << (int) (p_rGeneralcmd.GetCmdNo()) << " and DataBytes: " << (std::string) (p_rGeneralcmd.GetDataBytes()));

	gateway::GatewayIO::AppData appData;
	appData.appCmd = m_pRequest;
	appData.cmdType = gateway::GatewayIO::AppData::CmdType_General;

	int cmdNo = p_rGeneralcmd.GetCmdNo();
	std::string dataBytes = p_rGeneralcmd.GetDataBytes();

	//LOG_INFO_APP("[RequestProcessor::SendWHRequest]:" << cmdNo << ". Device=" << device->Mac());
	m_pProcessor->gateway.SendWHRequest(appData, device->Mac().Address(), cmdNo, dataBytes, p_rGeneralcmd.GetBypassIOCache());
}


//reports
void RequestProcessor::Visit(AppRoutesReportCmd& p_rRoutesCmd)
{
    //LOG_INFO_APP("[RequestProcessor::Visit]: AppRoutesReportCmd. Device=AllRegistered");

	LOG_DEBUG_APP("RequestProcessor::AppRoutesReportCmd");

	for ( DevicesManager::const_iterator_by_nick dev = p_rRoutesCmd.pDevices->BeginRegisteredByNick();
		  dev != p_rRoutesCmd.pDevices->EndRegisteredByNick() ; dev++ )
	{
	    SendRoutesListRequests(dev->second, 0, C802_MAX_ROUTES_LIST, m_pRequest, m_pProcessor);
	}
}

void RequestProcessor::Visit(AppServicesReportCmd& p_rServicesCmd)
{
    //LOG_INFO_APP("[RequestProcessor::Visit]: AppServicesReportCmd. Device=AllRegistered");

	LOG_DEBUG_APP("RequestProcessor::AppServicesReportCmd");

	//ask for services list from every device
	for ( DevicesManager::const_iterator_by_nick dev = p_rServicesCmd.pDevices->BeginRegisteredByNick ();
		  dev != p_rServicesCmd.pDevices->EndRegisteredByNick() ; dev++ )
	{	C800_ReadServiceList_Req whReq;
		whReq.m_ucServiceIndex = 0;
		whReq.m_ucNoOfEntriesToRead = C800_MAX_SERVICES_LIST;
	
		stack::WHartCommand whCmd;
		whCmd.commandID = CMDID_C800_ReadServiceList;
		whCmd.command = &whReq;
	
		gateway::GatewayIO::AppData appData;
		appData.appCmd = m_pRequest;
		appData.innerDataHandle = dev->second->id;

		//LOG_INFO_APP("[RequestProcessor::SendWHRequest]:CMDID_C800_ReadServiceList. Device=" << dev->second->Mac());
		m_pProcessor->gateway.SendWHRequest(appData, dev->second->Mac().Address(), whCmd, false, true);
	}
}

void RequestProcessor::Visit(AppDeviceHealthReportCmd& p_rDeviceHealthCmd)
{
    //LOG_INFO_APP("[RequestProcessor::Visit]: AppDeviceHealthReportCmd. Device=AllRegistered");

	std::vector<DeviceHealth>& devicesHealth = p_rDeviceHealthCmd.GetDevicesHealth();
	for (unsigned int i = 0; i < devicesHealth.size(); ++i)
	{
		// filter for devices read from db

		C779_ReportDeviceHealth_Req whReq;

		stack::WHartCommand whCmd;
		whCmd.commandID = CMDID_C779_ReportDeviceHealth;
		whCmd.command = &whReq;

		gateway::GatewayIO::AppData appData;
		appData.appCmd = m_pRequest;
		appData.innerDataHandle = i;

		//LOG_INFO_APP("[RequestProcessor::SendWHRequest]:CMDID_C779_ReportDeviceHealth. Device=" << devicesHealth[i].m_oDeviceMac);
		m_pProcessor->gateway.SendWHRequest(appData, devicesHealth[i].m_oDeviceMac.Address(), whCmd, false, true);
	}

}
void RequestProcessor::Visit(AppSuperframesReportCmd& p_rSuperframesCmd)
{
    //LOG_INFO_APP("[RequestProcessor::Visit]: AppSuperframesReportCmd. Device=AllRegistered");

	LOG_DEBUG_APP("RequestProcessor::AppSuperframesReportCmd");

	//ask for services list from every device
	for ( DevicesManager::const_iterator_by_nick dev = p_rSuperframesCmd.pDevices->BeginRegisteredByNick ();
		  dev != p_rSuperframesCmd.pDevices->EndRegisteredByNick() ;
		  dev++ )
	{	C783_ReadSuperframeList_Req whReq;
		whReq.m_ucSuperframeIndex = 0;
		whReq.m_ucNoOfEntriesToRead = C783_MAX_SUPERFRAMES_LIST;
	
		stack::WHartCommand whCmd;
		whCmd.commandID = CMDID_C783_ReadSuperframeList;
		whCmd.command = &whReq;
	
		gateway::GatewayIO::AppData appData;
		appData.appCmd = m_pRequest;
		appData.innerDataHandle = dev->second->id;

		//LOG_INFO_APP("[RequestProcessor::SendWHRequest]:CMDID_C783_ReadSuperframeList. Device=" << dev->second->Mac());
		m_pProcessor->gateway.SendWHRequest(appData, dev->second->Mac().Address(), whCmd, false, true);
	}
}

void RequestProcessor::Visit(AppDeviceScheduleLinksReportCmd& p_rDeviceScheduleLinksCmd)
{
    //LOG_INFO_APP("[RequestProcessor::Visit]: AppDeviceScheduleLinksReportCmd. Device=AllRegistered");

	LOG_DEBUG_APP("RequestProcessor::AppDeviceScheduleLinksReportCmd");

	//ask for services list from every device
	int counter = 0;
	for(DevicesScheduleLinks::const_iterator it =p_rDeviceScheduleLinksCmd.m_oDevicesLinkList.begin(); it != p_rDeviceScheduleLinksCmd.m_oDevicesLinkList.end(); ++it)
	{
		C784_ReadLinkList_Req whReq;
		whReq.m_unLinkIndex = 0;
		whReq.m_ucNoOfLinksToRead = C784_MAX_LINKS_LIST;

		stack::WHartCommand whCmd;
		whCmd.commandID = CMDID_C784_ReadLinkList;
		whCmd.command = &whReq;

		gateway::GatewayIO::AppData appData;
		appData.appCmd = m_pRequest;
		appData.innerDataHandle = counter++; //vector index

		//LOG_INFO_APP("[RequestProcessor::SendWHRequest]:CMDID_C784_ReadLinkList. Device=" << it->m_oDeviceMac);
		m_pProcessor->gateway.SendWHRequest(appData, it->m_oDeviceMac.Address(), whCmd, false, true);
	}
}

void RequestProcessor::Visit(AppNeighborHealthReportCmd& p_rNeighborHealthCmd)
{
    //LOG_INFO_APP("[RequestProcessor::Visit]: AppNeighborHealthReportCmd. Device=AllRegistered");

	LOG_DEBUG_APP("RequestProcessor::AppNeighborHealthReportCmd");

	//ask for neighbor health reports from all devices;
	DevicesNeighborsHealth::iterator it = p_rNeighborHealthCmd.m_oNeighborsHealthCache.begin();
	int counter = 0;
	for ( ; it != p_rNeighborHealthCmd.m_oNeighborsHealthCache.end() ; ++it)
	{
		C780_ReportNeighborHealthList_Req whReq;
		whReq.m_ucNeighborTableIndex = 0;
		whReq.m_ucNoOfNeighborEntriesToRead = C780_MAX_NEIGHBORS_LIST;
	
		stack::WHartCommand whCmd;
		whCmd.commandID = CMDID_C780_ReportNeighborHealthList;
		whCmd.command = &whReq;
	
		gateway::GatewayIO::AppData appData;
		appData.appCmd = m_pRequest;
		appData.innerDataHandle = counter++; //vector index to easily fetch the neighbors later

		//LOG_INFO_APP("[RequestProcessor::SendWHRequest]:CMDID_C780_ReportNeighborHealthList. Device=" << it->m_oMac);
		m_pProcessor->gateway.SendWHRequest(appData, it->m_oMac.Address(), whCmd, false, true);
	}
}

// read burst message
void RequestProcessor::Visit(AppDiscoveryBurstConfigCmd& p_rAppCmd)
{
    //LOG_INFO_APP("[RequestProcessor::Visit]: AppDiscoveryBurstConfigCmd. Device=" << p_rAppCmd.m_oDeviceMAC);

    p_rAppCmd.m_nCurrentBurst = 0;
    SendRequest105(0, 0, p_rAppCmd.m_oDeviceMAC, m_pRequest, m_pProcessor);
}

void RequestProcessor::Visit(AppFirmwareTransfersCmd& p_rAppCmd)
{
    //LOG_INFO_APP("[RequestProcessor::Visit]: AppFirmwareTransfersCmd");

	p_rAppCmd.m_file = boost::shared_ptr<MMappedFile>(new MMappedFile());
	unsigned int fileSize;
	if (!p_rAppCmd.m_file->init(p_rAppCmd.m_fileName.c_str(), fileSize))
	{
		THROW_EXCEPTION1(nlib::Exception, "file mapping cannot be done!");
	}
	LOG_INFO("bulk -> Mapped file size is: " << fileSize );

	p_rAppCmd.m_currentTransferState = AppFirmwareTransfersCmd::TransferOpen;

	C112_TransferService_Req whReq;
	stack::WHartCommand whCmd;
	whCmd.commandID = CMDID_C112_TransferService;
	whCmd.command = &whReq;

	gateway::GatewayIO::AppData appData;
	appData.appCmd = m_pRequest;
	appData.innerDataHandle = p_rAppCmd.m_currentOffset;

	//LOG_INFO_APP("[RequestProcessor::SendWHRequest]:CMDID_C112_TransferService. Device=GW");
	m_pProcessor->gateway.SendWHRequest(appData, stack::Gateway_UniqueID(), whCmd, false, true);
}

} // namespace hostapp
} // namespace hart7
