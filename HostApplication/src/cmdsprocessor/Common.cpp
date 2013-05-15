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

#include <string.h>

#include <nlib/log.h>

#include <WHartHost/cmdsprocessor/CommandsProcessor.h>
#include <WHartHost/gateway/GatewayIO.h>
#include <WHartHost/model/PublisherInfo.h>

#include <list>

#include "Common.h"



namespace hart7 {
namespace hostapp {


static void InitSameCmdChannelSet( const PublishChannelSetT& p_rSet,
						   PublishChannelSetT::const_iterator& p_oStart /*[in/out]*/)
{
	uint8_t cmdNo = p_oStart->cmdNo;

	for ( ; p_oStart != p_rSet.end() ; ++p_oStart)
	{	
		if (p_oStart->cmdNo != cmdNo)
		{	//no more channels for current command
			break;
		}
	}
}

bool IsWHCmdNoValid(int cmdNo)
{
	switch (cmdNo)
	{
	case CMDID_C001_ReadPrimaryVariable:	
	case CMDID_C002_ReadLoopCurrentAndPercentOfRange:
	case CMDID_C003_ReadDynamicVariablesAndLoopCurrent:
	case CMDID_C009_ReadDeviceVariablesWithStatus:
	case CMDID_C033_ReadDeviceVariables:
	case CMDID_C178_PublishedDynamicData:
		return true;
	//case CMDID_C093/C123
	default:
		break;
	}
	return false;
}


void* BuildBurstValueCmdRequest ( const PublishChannelSetT& p_rSet,
								  PublishChannelSetT::const_iterator& p_oStart /*[in/out]*/)
{
	static C001_ReadPrimaryVariable_Req req_1;
	static C002_ReadLoopCurrentAndPercentOfRange_Req req_2;
	static C003_ReadDynamicVariablesAndLoopCurrent_Req req_3;
	static C009_ReadDeviceVariablesWithStatus_Req req_9;
	static C033_ReadDeviceVariables_Req req_33;
	static C178_PublishedDynamicData_Req req_178;

	if (p_oStart == p_rSet.end())
	{	return 0;
	}

	uint16_t cmdNo = p_oStart->cmdNo;

	void* retval = 0;

	InitSameCmdChannelSet(p_rSet, p_oStart);

	switch (cmdNo)
	{
	case CMDID_C001_ReadPrimaryVariable:
		retval = &req_1;
		break;
	case CMDID_C002_ReadLoopCurrentAndPercentOfRange:
		retval = &req_2;
		break;
	case CMDID_C003_ReadDynamicVariablesAndLoopCurrent:
		retval = &req_3;
		break;
	case CMDID_C009_ReadDeviceVariablesWithStatus:
		req_9.variablesSize = 8;
		for (int i = 0 ; i < req_9.variablesSize ; i++) 
		{	req_9.slotDVC[i] = DeviceVariableCodes_None;
		}
		retval = &req_9;
		break;
	case CMDID_C033_ReadDeviceVariables:
		req_33.variablesSize = 4;
		for (int i = 0 ; i < req_33.variablesSize ; i++) 
		{	req_33.variables[i] = (uint8_t)(DeviceVariableCodes_None);
		}
		retval = &req_33;
		break;
    case CMDID_C178_PublishedDynamicData:
        memset(req_178.Address, 0, sizeof(WHartUniqueID));
        req_178.ByteCount = 0;
        req_178.Command = 178;
        req_178.Header_STX = 0x82;
        req_178.Parity = 0;
        retval = &req_178;
        break;
	//case CMDID_C093/C123
	default:
		break;
	}

	return retval;
}

void* BuildReadValueCmdRequest(const PublishChannelSetT& p_rSet)
{
	if (p_rSet.empty())
	{	return 0;
	}

	static C001_ReadPrimaryVariable_Req req_1;
	static C002_ReadLoopCurrentAndPercentOfRange_Req req_2;
	static C003_ReadDynamicVariablesAndLoopCurrent_Req req_3;
	static C009_ReadDeviceVariablesWithStatus_Req req_9;
	static C033_ReadDeviceVariables_Req req_33;
	static C178_PublishedDynamicData_Req req_178;

	const PublishChannel& channel = *(p_rSet.begin());

	uint16_t cmdNo = channel.cmdNo;

	void* retval = 0;

	switch (cmdNo)
	{
	case CMDID_C001_ReadPrimaryVariable:	
		retval = &req_1;
		break;
	case CMDID_C002_ReadLoopCurrentAndPercentOfRange:
		retval = &req_2;
		break;
	case CMDID_C003_ReadDynamicVariablesAndLoopCurrent:
		retval = &req_3;
		break;
	case CMDID_C009_ReadDeviceVariablesWithStatus:
		memset(&req_9,0,sizeof(req_9));
		req_9.slotDVC[(req_9.variablesSize)++] = (DeviceVariableCodes)(channel.deviceVariable);
		retval = &req_9;
		break;
	case CMDID_C033_ReadDeviceVariables:
		memset(&req_33,0,sizeof(req_33));
		req_33.variables[(req_33.variablesSize)++] = (DeviceVariableCodes)(channel.deviceVariable);
		retval = &req_33;
		break;
    case CMDID_C178_PublishedDynamicData:
        memset(req_178.Address, 0, sizeof(WHartUniqueID));
        req_178.ByteCount = 0;
        req_178.Command = 178;
        req_178.Header_STX = 0x82;
        req_178.Parity = 0;
        retval = &req_178;
        break;
	//case CMDID_C093/C123
	default:
		break;
	}

	return retval;
}


void SendRoutesListRequests(const DevicePtr& device, int p_nIndex, int p_nSize, const AbstractAppCommandPtr& appCmd, CommandsProcessor* p_pProcessor)
{
	C802_ReadRouteList_Req whReq;
	whReq.m_ucRouteIndex = p_nIndex;
	whReq.m_ucNoOfEntriesToRead = p_nSize;

	stack::WHartCommand whCmd;
	whCmd.commandID = CMDID_C802_ReadRouteList;
	whCmd.command = &whReq;

	gateway::GatewayIO::AppData appData;
	appData.appCmd = appCmd;
	appData.innerDataHandle = device->id;

	//LOG_INFO_APP("[Common::SendWHRequest]:CMDID_C802_ReadRouteList. Device=" << device->Mac());
	p_pProcessor->gateway.SendWHRequest(appData, device->Mac().Address(), whCmd, false, true);
}

bool AppSetBurstConfigurationCmd_SendReadBurstConfigCmd(AppSetBurstConfigurationCmd& p_rBurstConfig,
                                                        const AbstractAppCommandPtr& p_rAppCmd,
                                                        CommandsProcessor* p_pProcessor)
{
    DevicePtr dev = p_rBurstConfig.GetDevice();
    p_rBurstConfig.pDevices->UpdatePublisherState(dev->Mac(), -1, SETPUBLISHER_STATE_READ_BURSTCONFIG, SETPUBLISHER_ERROR_NONE, "");

	if ( dev != 0 && dev->GetPublisherInfo().burstNoTotalCmd105 < 0 /*unset*/)
	{	//send 105 request
		C105_ReadBurstModeConfiguration_Req req;
		req.burstMessage = false;
		req.requestIsEmpty = false;
		stack::WHartCommand whCmd;
		whCmd.commandID = CMDID_C105_ReadBurstModeConfiguration;
		whCmd.command = &req;

		gateway::GatewayIO::AppData appData;
		appData.appCmd = p_rAppCmd;

		LOG_INFO_APP("[SetBurstConfiguration]:CMD105 - REQUEST[" << dev->Mac() << "]=(burstMessage=" << (int)req.burstMessage << ", requestIsEmpty=" << (int)req.requestIsEmpty << ")");
		try
		{
		    //LOG_INFO_APP("[Common::SendWHRequest]:CMDID_C105_ReadBurstModeConfiguration. Device=" << dev->Mac());
			p_pProcessor->gateway.SendWHRequest(appData, dev->Mac().Address(), whCmd, false, true);
		}
		catch (std::exception e)
		{
			dev->IssueSetBurstConfigurationCmd = true;
	        LOG_ERROR_APP("[SetBurstConfiguration] - Error on sending command 105 for deviceMAC=" << dev->Mac() << " with: burtsMessage=" << (int)req.burstMessage << ", requestIsEmpty=" << (int)req.requestIsEmpty << ".");
			throw e;
		}
		return true;
	}

	return false;
}

bool AppSetBurstConfigurationCmd_SendTurnOffBurstMessagesCmd(AppSetBurstConfigurationCmd& p_rBurstConfig,
                                                             const AbstractAppCommandPtr& p_rAppCmd,
                                                             CommandsProcessor* p_pProcessor)
{
	DevicePtr dev = p_rBurstConfig.GetDevice();
    p_rBurstConfig.pDevices->UpdatePublisherState(dev->Mac(), -1, SETPUBLISHER_STATE_TURNOFF_BURST, SETPUBLISHER_ERROR_NONE, "");

    if (dev == 0)
	{
        return false;
	}

	PublisherInfo& pubInfo = dev->GetPublisherInfo();

	stack::CHartCmdWrapperList cmdsList;

	//send 109 requests (grouped) to disable all device supported burst messages
	for (int i = 0 ; i < pubInfo.burstNoTotalCmd105 ; ++i)
	{	PublisherInfo::BurstMessageStateMap::const_iterator bmStateIt = pubInfo.burstMessagesState.find(i);

		//don't send 109 for current burst message if it is already turned off or if a fatal error was previously encountered
		if ( (bmStateIt != pubInfo.burstMessagesState.end()) && 
			 (bmStateIt->second == BURST_STATE_SET || bmStateIt->second == BURST_STATE_OFF || bmStateIt->second == BURST_STATE_ERROR) )
		{
		    continue;
		}

		C109_BurstModeControl_Req req;
		req.requestIsEmpty = false;
		req.burstMessageIsEmpty = false;
		req.burstMessage = (uint8_t)i;
		req.burstModeCode = BurstModeControlCodes_Off;

		stack::CHartCmdWrapper::Ptr cmd(new stack::CHartCmdWrapper);
		cmd->LoadParsed(CMDID_C109_BurstModeControl, sizeof(req), &req);
		cmd->GetRawFromParsed();
		cmdsList.push_back(cmd);
	}
	
	if (!cmdsList.empty())
	{	
		if (pubInfo.flushDelayedResponses)
		{	C106_FlushDelayedResponses_Req req_106;
			stack::CHartCmdWrapper::Ptr cmd(new stack::CHartCmdWrapper);
			cmd->LoadParsed(CMDID_C106_FlushDelayedResponses, sizeof(req_106), &req_106);
			cmd->GetRawFromParsed();
			cmdsList.push_front(cmd);
			pubInfo.flushDelayedResponses = false;
		}

		gateway::GatewayIO::AppData appData;
		appData.appCmd = p_rAppCmd;
		LOG_INFO_APP("[SetBurstConfiguration]:CMD106 - REQUEST[" << dev->Mac() << "]=(" << cmdsList << ")");
		try
		{
		    //LOG_INFO_APP("[Common::SendWHRequest]:CMDID_C106_FlushDelayedResponses. Device=" << dev->Mac());
			p_pProcessor->gateway.SendWHRequest(appData, dev->Mac().Address(), cmdsList);
		}
		catch (std::exception e)
		{
			dev->IssueSetBurstConfigurationCmd = true;
            LOG_ERROR_APP("[SetBurstConfiguration] - Error on turning of burst messages for deviceMAC=" << dev->Mac() << ".");
			throw e;
		}
		return true;
	}

	return false;
}

bool AppSetBurstConfigurationCmd_SendGetSubDeviceCountCmd(AppSetBurstConfigurationCmd& p_rBurstConfig,
                                                          const AbstractAppCommandPtr& p_rAppCmd,
                                                          CommandsProcessor* p_pProcessor)
{
    DevicePtr dev = p_rBurstConfig.GetDevice();
    p_rBurstConfig.pDevices->UpdatePublisherState(dev->Mac(), -1, SETPUBLISHER_STATE_GET_SUBDEVICEINDEX, SETPUBLISHER_ERROR_NONE, "");

    if ( dev != 0 )
    {
        C074_ReadIOSystemCapabilities_Req req;
        stack::WHartCommand whCmd;
        whCmd.commandID = CMDID_C074_ReadIOSystemCapabilities;
        whCmd.command = &req;

        gateway::GatewayIO::AppData appData;
        appData.appCmd = p_rAppCmd;

        LOG_INFO_APP("[SetBurstConfiguration]:CMD074 - REQUEST[" << dev->Mac() << "] = ()");
        try
        {
            //LOG_INFO_APP("[Common::SendWHRequest]:CMDID_C074_ReadIOSystemCapabilities. Device=" << dev->Mac());
            p_pProcessor->gateway.SendWHRequest(appData, dev->Mac().Address(), whCmd, false, true);
        }
        catch (std::exception e)
        {
            dev->IssueSetBurstConfigurationCmd = true;
            LOG_ERROR_APP("[SetBurstConfiguration] - Error on getting total number of sub-devices for deviceMAC=" << dev->Mac() << ".");
            throw e;
        }
        return true;
    }
    return false;
}

bool AppSetBurstConfigurationCmd_SendGetSubDeviceIdCmd(AppSetBurstConfigurationCmd& p_rBurstConfig,
                                                       const AbstractAppCommandPtr& p_rAppCmd,
                                                       CommandsProcessor* p_pProcessor)
{
    DevicePtr dev = p_rBurstConfig.GetDevice();
    p_rBurstConfig.pDevices->UpdatePublisherState(dev->Mac(), -1, SETPUBLISHER_STATE_GET_SUBDEVICEINDEX, SETPUBLISHER_ERROR_NONE, "");

    if ( dev != 0 )
    {
        C084_ReadSubDeviceIdentitySummary_Req req;
        req.subDeviceIndex = p_rBurstConfig.m_nCurrentSubDeviceIndex;

        stack::WHartCommand whCmd;
        whCmd.commandID = CMDID_C084_ReadSubDeviceIdentitySummary;
        whCmd.command = &req;

        gateway::GatewayIO::AppData appData;
        appData.appCmd = p_rAppCmd;

        LOG_INFO_APP("[SetBurstConfiguration]:CMD084 - REQUEST[" << dev->Mac() << "] = (subDeviceIndex=" << p_rBurstConfig.m_nCurrentSubDeviceIndex << ")");
        try
        {
            //LOG_INFO_APP("[Common::SendWHRequest]:CMDID_C084_ReadSubDeviceIdentitySummary. Device=" << dev->Mac());
            p_pProcessor->gateway.SendWHRequest(appData, dev->Mac().Address(), whCmd, false, true);
        }
        catch (std::exception e)
        {
            dev->IssueSetBurstConfigurationCmd = true;
            LOG_ERROR_APP("[SetBurstConfiguration] - Error on getting sub-device id assigned to subDeviceIndex=" << (int)req.subDeviceIndex << " for deviceMAC=" << dev->Mac() << ".");
            throw e;
            return false;
        }
        return true;
    }
    return false;
}


bool AppSetBurstConfigurationCmd_SendConfigureBurstMessagesCmd(AppSetBurstConfigurationCmd& p_rBurstConfig,
                                                               const AbstractAppCommandPtr& p_rAppCmd,
                                                               CommandsProcessor* p_pProcessor)
{
    DevicePtr dev = p_rBurstConfig.GetDevice();
    if (dev == 0)
    {
        return false;
    }

    PublisherInfo& pubInfo = dev->GetPublisherInfo();
    std::list<BurstMessage>& burstsToConfigure = p_rBurstConfig.GetBurstMessagesToConfigure();

    if (!burstsToConfigure.empty())
    {
        p_rBurstConfig.pDevices->UpdatePublisherState(dev->Mac(), -1, SETPUBLISHER_STATE_CONFIGURE_BURST, SETPUBLISHER_ERROR_NONE, "");
    }

    while (!burstsToConfigure.empty())
    {
        BurstMessage& currentBurstMessage = burstsToConfigure.front();

        LOG_INFO_APP("[SetBurstConfiguration]:CMD_LIST ******* Send REQUEST for deviceMAC=" << dev->Mac() << ", burstMessage=" << (int)currentBurstMessage.burstMessage);

        stack::CHartCmdWrapperList cmdsList;
        cmdsList.clear();

        hart7::hostapp::MAC macEmpty(0);
        if (dev->IsAdapter())
        {
            if (!(currentBurstMessage.subDeviceMAC == macEmpty))
            {
                C102_MapSubDeviceToBurstMessage_Req req_102;
                req_102.burstMessage = currentBurstMessage.burstMessage;
                req_102.subDeviceIndex = dev->subDevicesMap[currentBurstMessage.subDeviceMAC];
                stack::CHartCmdWrapper::Ptr cmd_102(new stack::CHartCmdWrapper);
                cmd_102->LoadParsed(CMDID_C102_MapSubDeviceToBurstMessage, sizeof(req_102), &req_102);
                cmd_102->GetRawFromParsed();
                cmdsList.push_back(cmd_102);

                LOG_INFO_APP("[SetBurstConfiguration]:CMD102 - REQUEST[" << dev->Mac() << "]=("
                         << "burstMessage=" << (int)req_102.burstMessage
                         << ", subDeviceIndex=" <<(int)req_102.subDeviceIndex
                         << ") - subDeviceMAC=" << currentBurstMessage.subDeviceMAC << ")");
            }
        }

        if (pubInfo.flushDelayedResponses)
        {   C106_FlushDelayedResponses_Req req;
            stack::CHartCmdWrapper::Ptr cmd(new stack::CHartCmdWrapper);
            cmd->LoadParsed(CMDID_C106_FlushDelayedResponses, sizeof(req), &req);
            cmd->GetRawFromParsed();
            cmdsList.push_back(cmd);
            LOG_INFO_APP("[SetBurstConfiguration]:CMD106 - REQUEST[" << dev->Mac() << "]=()");
        }

        {
            C103_WriteBurstPeriod_Req req_103;
            req_103.burstMessage = currentBurstMessage.burstMessage;
            req_103.updatePeriod.u32 = (uint32_t)( (currentBurstMessage.updatePeriod * 1000/*convert to milisecond units*/) * 32 /*convert to 1/32 ms units*/);
            req_103.maxUpdatePeriod.u32 = (uint32_t)( (currentBurstMessage.maxUpdatePeriod * 1000/*convert to milisecond units*/) * 32 /*convert to 1/32 ms units*/);
            stack::CHartCmdWrapper::Ptr cmd_103(new stack::CHartCmdWrapper);
            cmd_103->LoadParsed(CMDID_C103_WriteBurstPeriod, sizeof(req_103), &req_103);
            cmd_103->GetRawFromParsed();
            cmdsList.push_back(cmd_103);
            LOG_INFO_APP("[SetBurstConfiguration]:CMD103 - REQUEST[" << dev->Mac() << "]=("
                << "burstMessage=" << (int)req_103.burstMessage << ", "
                << "updatePeriod=" <<(int)req_103.updatePeriod.u32 << ", "
                << "maxUpdatePeriod=" << (int)req_103.maxUpdatePeriod.u32 << ")");
        }

        {
            Trigger dummy; /*just for searching*/
            dummy.burstMessage = currentBurstMessage.burstMessage;
            dummy.cmdNo = currentBurstMessage.cmdNo;
            TriggerSetT::iterator triggerIt = pubInfo.triggersList.find(dummy);
            C104_WriteBurstTrigger_Req req_104;
            if (triggerIt != pubInfo.triggersList.end())
            {
                req_104.burstMessage = triggerIt->burstMessage;
                req_104.selectionCode = (BurstMessageTriggerMode)(triggerIt->modeSelection);
                req_104.classificationCode = (DeviceVariableClassificationCodes)(triggerIt->classification);
                req_104.unitsCode = (UnitsCodes)(triggerIt->unitCode);
                req_104.triggerLevel = triggerIt->triggerLevel;

                stack::CHartCmdWrapper::Ptr cmd_104(new stack::CHartCmdWrapper);
                cmd_104->LoadParsed(CMDID_C104_WriteBurstTrigger, sizeof(req_104), &req_104);
                cmd_104->GetRawFromParsed();
                cmdsList.push_back(cmd_104);
                LOG_INFO_APP("[SetBurstConfiguration]:CMD104 - REQUEST[" << dev->Mac() << "]=("
                    << "burstMessage=" << (int)req_104.burstMessage << ", "
                    << "selectionCode=" << (int)triggerIt->modeSelection << ", "
                    << "classificationCode=" << (int)triggerIt->classification << ", "
                    << "unitsCode=" << (int)triggerIt->unitCode << ", "
                    << "triggerLevel=" << triggerIt->triggerLevel << ")");
            }
        }

        {
            C107_WriteBurstDeviceVariables_Req req_107;
            memset(&req_107, 0, sizeof(req_107));

            req_107.burstMessage = currentBurstMessage.burstMessage;

            //init vars
            for( ; req_107.noOfDeviceVariables < MaxNoOfDeviceVariables ; req_107.noOfDeviceVariables++)
            {
                req_107.deviceVariableCode[req_107.noOfDeviceVariables] = DeviceVariableCodes_None;
            }

            //put cursor at the start of our block our channels
            PublishChannelSetT::iterator cursor = FindFirstChannel(pubInfo.channelList, currentBurstMessage.cmdNo);
            //can have same cmdNo in multiple burst messages; so find the one that we need
            while (cursor != pubInfo.channelList.end() && cursor->burstMessage != currentBurstMessage.burstMessage) ++cursor;
            if (cursor == pubInfo.channelList.end() )
            {   //we don't configure anymore because we don't have all the needed info
                LOG_WARN_APP("[SetBurstConfiguration]: Cannot configure burstmessage=" << (int)(currentBurstMessage.burstMessage)
                    << " on device with deviceId=" << (int)(dev->id) << " because there are no defined channels for it");
                burstsToConfigure.pop_front();
                continue;
            }
            //scroll with cursor until the end of the channels block and process each channel
            std::ostringstream ostr;
            ostr << "[";
            while(cursor != pubInfo.channelList.end() && cursor->burstMessage == currentBurstMessage.burstMessage)
            {
                req_107.deviceVariableCode[cursor->deviceVariableSlot] = (DeviceVariableCodes)(cursor->deviceVariable);
                ostr << (int)cursor->deviceVariableSlot << ":" << (int)cursor->deviceVariable;
                ++cursor;
                ostr << ((cursor != pubInfo.channelList.end() && cursor->burstMessage == currentBurstMessage.burstMessage) ? ", " : "]");
            }

            stack::CHartCmdWrapper::Ptr cmd_107(new stack::CHartCmdWrapper);
            cmd_107->LoadParsed(CMDID_C107_WriteBurstDeviceVariables, sizeof(req_107), &req_107);
            cmd_107->GetRawFromParsed();
            cmdsList.push_back(cmd_107);

            LOG_INFO_APP("[SetBurstConfiguration]:CMD107 - REQUEST[" << dev->Mac() << "]=("
                << "burstMessage=" << (int)req_107.burstMessage << ", "
                << "noOfDeviceVariables=" << (int)req_107.noOfDeviceVariables << ", "
                << "deviceVariableCode=" << ostr.str() << ")");
        }

        {
            C108_WriteBurstModeCommandNumber_Req req_108;
            req_108.commandNo = currentBurstMessage.cmdNo;
            req_108.burstMessageIsEmpty = false;
            req_108.burstMessage = currentBurstMessage.burstMessage;
            stack::CHartCmdWrapper::Ptr cmd_108(new stack::CHartCmdWrapper);
            cmd_108->LoadParsed(CMDID_C108_WriteBurstModeCommandNumber, sizeof(req_108), &req_108);
            cmd_108->GetRawFromParsed();
            cmdsList.push_back(cmd_108);
            LOG_INFO_APP("[SetBurstConfiguration]:CMD108 - REQUEST[" << dev->Mac() << "]=("
                << "burstMessage=" << (int)req_108.burstMessage << ", "
                << "commandNo=" << (int)req_108.commandNo << ", "
                << "burstMessageIsEmpty=" << (int)req_108.burstMessageIsEmpty << ")");
        }

        {
            C109_BurstModeControl_Req req_109;
            req_109.requestIsEmpty = false;
            req_109.burstMessageIsEmpty = false;
            req_109.burstMessage = currentBurstMessage.burstMessage;
            req_109.burstModeCode = BurstModeControlCodes_EnableBurstOnTDMADataLinkLayer;
            stack::CHartCmdWrapper::Ptr cmd_109(new stack::CHartCmdWrapper);
            cmd_109->LoadParsed(CMDID_C109_BurstModeControl, sizeof(req_109), &req_109);
            cmd_109->GetRawFromParsed();
            cmdsList.push_back(cmd_109);
            LOG_INFO_APP("[SetBurstConfiguration]:CMD109 - REQUEST[" << dev->Mac() << "]=("
                << "burstMessage=" << (int)req_109.burstMessage << ", "
                << "requestIsEmpty=" << (int)req_109.requestIsEmpty << ", "
                << "burstMessageIsEmpty=" << (int)req_109.burstMessageIsEmpty << ", "
                << "burstModeCode=" << (int)req_109.burstModeCode << ")");
        }
        gateway::GatewayIO::AppData appData;
        appData.appCmd = p_rAppCmd;
        appData.innerDataHandle = currentBurstMessage.burstMessage;

        LOG_INFO_APP("[SetBurstConfiguration]:CMD_LIST - REQUEST[" << dev->Mac() << ", " << (int)(currentBurstMessage.burstMessage) << "]=(" << cmdsList << ")");

        if (pubInfo.flushDelayedResponses)
        {
            pubInfo.flushDelayedResponses = false;
        }

        try
        {
            //LOG_INFO_APP("[Common::SendWHRequest]:CMDID_C102_MapSubDeviceToBurstMessage/CMDID_C106_FlushDelayedResponses/103/104/107/108/109. Device=" << dev->Mac());
            p_pProcessor->gateway.SendWHRequest(appData, dev->Mac().Address(), cmdsList);
        }
        catch (std::exception e)
        {
            dev->IssueSetBurstConfigurationCmd = true;
            throw e;
        }

        burstsToConfigure.pop_front();

        return true;
    }

    return false;
}

void AppSetBurstConfigurationCmd_Finished(AppSetBurstConfigurationCmd& p_rBurstConfig)
{
	DevicePtr dev = p_rBurstConfig.GetDevice();

	if (dev != 0)
	{
		PublisherInfo& pubInfo = dev->GetPublisherInfo();

		pubInfo.noOfBurstMessagesThatFailedToConfigure = p_rBurstConfig.GetNbOfUnconfiguredBurstMessages();
		pubInfo.lastConfigurationTime = CMicroSec();//now

		dev->IssueSetBurstConfigurationCmd = true;
		
		std::stringstream str;
		str << "[SetBurstConfiguration]:FINISH - deviceMAC=" << dev->Mac().ToString()
			<< ", noOfBurstMessagesThatFailedToConfigure = " << (int)(pubInfo.noOfBurstMessagesThatFailedToConfigure)
			<< ", totalNbOfBurst = " << (int)(pubInfo.burstNoTotalCmd105);

		bool noError = true;
		for (int i = 0 ; i < pubInfo.burstNoTotalCmd105 ; i++)
		{
		    str << " bm" << i << "_state=" << pubInfo.burstMessagesState[i];

		    if (pubInfo.burstMessagesState[i] == BURST_STATE_ERROR)
		        noError = false;
		}

		if (noError)
            p_rBurstConfig.pDevices->UpdatePublisherState(dev->Mac(), -1, SETPUBLISHER_STATE_DONE, SETPUBLISHER_ERROR_NONE, "");

		LOG_INFO_APP(str.str());
	}
}

void SendRequest009(const DeviceVariableCodes deviceVariables[MaxNoOfDeviceVariables], const int varsNo, const int p_nInnerDataHandler, const MAC& mac,
					const AbstractAppCommandPtr& p_rAppCmd, const CommandsProcessor* p_pProcessor)
{
    C009_ReadDeviceVariablesWithStatus_Req whReq;
    for (int i = 0; i < MaxNoOfDeviceVariables; ++i) {
		whReq.slotDVC[i] = deviceVariables[i];
	}
    whReq.variablesSize = varsNo;
    stack::WHartCommand whCmd;
    whCmd.commandID = CMDID_C009_ReadDeviceVariablesWithStatus;
    whCmd.command = &whReq;

    gateway::GatewayIO::AppData appData;
    appData.appCmd = p_rAppCmd;
    appData.innerDataHandle = p_nInnerDataHandler;

    try
    {
        //LOG_INFO_APP("[Common::SendWHRequest]:CMDID_C009_ReadDeviceVariablesWithStatus. Device=" << mac);
        p_pProcessor->gateway.SendWHRequest(appData, mac.Address(), whCmd, false, true);

        std::ostringstream ostr;
        for (int i = 0; i < varsNo; i++)
            ostr << deviceVariables[i] << ((i < varsNo-1) ? ", " : "");

        LOG_INFO_APP("[DiscoveryBurstConfig]:CMD009 - REQUEST[" << mac << "]=(slotDVC=[" << ostr.str() << "], variablesSize=" << varsNo << ")");
    }
    catch (std::exception e)
    {
        p_rAppCmd->pDevices->FindDevice(mac)->IssueDiscoveryBurstConfigCmd = hart7::hostapp::AUTODETECT_NONE;
        LOG_ERROR_APP("[DiscoveryBurstConfig] - Error on sending COMMAND 009 for deviceMAC=" << mac << ". EXCEPTION: " << e.what());
        throw e;
    }
}

void SendRequest074(const int p_nInnerDataHandler, const MAC& mac, const AbstractAppCommandPtr& p_rAppCmd, const CommandsProcessor* p_pProcessor)
{
    C074_ReadIOSystemCapabilities_Req req;
    stack::WHartCommand whCmd;
    whCmd.commandID = CMDID_C074_ReadIOSystemCapabilities;
    whCmd.command = &req;

    gateway::GatewayIO::AppData appData;
    appData.appCmd = p_rAppCmd;
    appData.innerDataHandle = p_nInnerDataHandler;

    LOG_INFO_APP("[DiscoveryBurstConfig]:CMD074 - REQUEST[" << mac << "] = ()");
    try
    {
        //LOG_INFO_APP("[Common::SendWHRequest]:CMDID_C074_ReadIOSystemCapabilities. Device=" << mac);
        p_pProcessor->gateway.SendWHRequest(appData, mac.Address(), whCmd, false, true);
    }
    catch (std::exception e)
    {
        p_rAppCmd->pDevices->FindDevice(mac)->IssueDiscoveryBurstConfigCmd = hart7::hostapp::AUTODETECT_NONE;
        LOG_ERROR_APP("[DiscoveryBurstConfig] - Error on sending COMMAND 074 for deviceMAC=" << mac << ". EXCEPTION: " << e.what());
        throw e;
    }
}


void SendRequest084(const uint16_t p_nSubDeviceIndex, const int p_nInnerDataHandler, const MAC& mac,
                    const AbstractAppCommandPtr& p_rAppCmd, const CommandsProcessor* p_pProcessor)
{
    C084_ReadSubDeviceIdentitySummary_Req whReq;
    whReq.subDeviceIndex = p_nSubDeviceIndex;

    stack::WHartCommand whCmd;
    whCmd.commandID = CMDID_C084_ReadSubDeviceIdentitySummary;
    whCmd.command = &whReq;

    gateway::GatewayIO::AppData appData;
    appData.appCmd = p_rAppCmd;
    appData.innerDataHandle = p_nInnerDataHandler;

    try
    {
        //LOG_INFO_APP("[Common::SendWHRequest]:CMDID_C084_ReadSubDeviceIdentitySummary. Device=" << mac);
        p_pProcessor->gateway.SendWHRequest(appData, mac.Address(), whCmd, false, true);
        LOG_INFO_APP("[DiscoveryBurstConfig]:CMD084 - REQUEST[" << mac << "]=(subDeviceIndex=" << (int)p_nSubDeviceIndex << ")");
    }
    catch (std::exception e)
    {
        p_rAppCmd->pDevices->FindDevice(mac)->IssueDiscoveryBurstConfigCmd = hart7::hostapp::AUTODETECT_NONE;
        LOG_ERROR_APP("[DiscoveryBurstConfig] - Error on sending COMMAND 84 for deviceMAC=" << mac << ". EXCEPTION: " << e.what());
        throw e;
    }
}

void SendRequest101(const uint8_t p_nBurstMessage, const int p_nInnerDataHandler, const MAC& mac,
                    const AbstractAppCommandPtr& p_rAppCmd, const CommandsProcessor* p_pProcessor)
{
    C101_ReadSubDeviceToBurstMessageMap_Req whReq;
    whReq.burstMessage = p_nBurstMessage;

    stack::WHartCommand whCmd;
    whCmd.commandID = CMDID_C101_ReadSubDeviceToBurstMessageMap;
    whCmd.command = &whReq;

    gateway::GatewayIO::AppData appData;
    appData.appCmd = p_rAppCmd;
    appData.innerDataHandle = p_nInnerDataHandler;

    try
    {
        //LOG_INFO_APP("[Common::SendWHRequest]:CMDID_C101_ReadSubDeviceToBurstMessageMap. Device=" << mac);
        p_pProcessor->gateway.SendWHRequest(appData, mac.Address(), whCmd, false, true);
        LOG_INFO_APP("[DiscoveryBurstConfig]:CMD101 - REQUEST[" << mac << "]=(burstMessage=" << (int)p_nBurstMessage << ")");
    }
    catch (std::exception e)
    {
        p_rAppCmd->pDevices->FindDevice(mac)->IssueDiscoveryBurstConfigCmd = hart7::hostapp::AUTODETECT_NONE;
        LOG_ERROR_APP("[DiscoveryBurstConfig] - Error on sending COMMAND 101 for deviceMAC=" << mac << " with: burstMessage=" << (int)p_nBurstMessage << ". EXCEPTION: " << e.what());
        throw e;
    }
}

void SendRequest105(const uint8_t p_nBurstMessage, const int p_nInnerDataHandler, const MAC& mac,
                    const AbstractAppCommandPtr& p_rAppCmd, const CommandsProcessor* p_pProcessor)
{
    C105_ReadBurstModeConfiguration_Req whReq;
    whReq.burstMessage = p_nBurstMessage;
    whReq.requestIsEmpty = 0;

    stack::WHartCommand whCmd;
    whCmd.commandID = CMDID_C105_ReadBurstModeConfiguration;
    whCmd.command = &whReq;

    gateway::GatewayIO::AppData appData;
    appData.appCmd = p_rAppCmd;
    appData.innerDataHandle = p_nInnerDataHandler;

    try
	{
        //LOG_INFO_APP("[Common::SendWHRequest]:CMDID_C105_ReadBurstModeConfiguration. Device=" << mac);
		p_pProcessor->gateway.SendWHRequest(appData, mac.Address(), whCmd, false, true);
	    LOG_INFO_APP("[DiscoveryBurstConfig]:CMD105 - REQUEST[" << mac << "]=(burstMessage=" << (int)p_nBurstMessage << ", requestIsEmpty=0)");
	}
	catch (std::exception e)
	{
		p_rAppCmd->pDevices->FindDevice(mac)->IssueDiscoveryBurstConfigCmd = hart7::hostapp::AUTODETECT_NONE;
	    LOG_ERROR_APP("[DiscoveryBurstConfig] - Error on sending COMMAND 105 for deviceMAC=" << mac << " with: burstMessage=" << (int)p_nBurstMessage << ", requestIsEmpty=0. EXCEPTION: " << e.what());
		throw e;
	}
}

}
}
