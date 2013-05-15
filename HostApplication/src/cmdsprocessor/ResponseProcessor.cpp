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


#include "ResponseProcessor.h"

#include <WHartStack/WHartStack.h>

#include <WHartHost/cmdsprocessor/CommandsProcessor.h>
#include <WHartHost/gateway/GatewayIO.h>
#include <WHartHost/model/reports/Route.h>
#include <WHartHost/model/reports/SourceRoute.h>
#include <WHartHost/model/reports/Service.h>
#include <WHartHost/model/reports/Superframe.h>
#include <WHartHost/model/reports/DeviceScheduleLink.h>
#include <WHartHost/model/reports/DeviceHealth.h>
#include <WHartHost/model/reports/NeighbSignalLevels.h>
#include <WHartHost/PublisherConf.h>
#include <stdint.h>
#include <stdio.h>
#include <iomanip>
#include <sstream>
#include <limits>
#include <boost/format.hpp>

#include "Common.h"
#include "SaveRespErr.h"


#include <WHartHost/Utils.h>


namespace hart7 {
namespace hostapp {

static const Slot* FindMatchingVariableSlot(const C009_ReadDeviceVariablesWithStatus_Resp& p_rResp, const PublishChannel& p_rChannel)
{
    for (int i = 0 ; i < p_rResp.variablesSize ; i++)
    {   if (p_rResp.slots[i].deviceVariableCode == p_rChannel.deviceVariable)
        {   return &(p_rResp.slots[i]);
        }
    }
    return 0;
}

static const float* FindMatchingVariableValue(const C033_ReadDeviceVariables_Resp& p_rResp, const PublishChannel& p_rChannel)
{
    for (int i = 0 ; i < p_rResp.variablesSize ; i++)
    {   if (p_rResp.variables[i].code == p_rChannel.deviceVariable)
        {   return &(p_rResp.variables[i].value);
        }
    }
    return 0;
}

static const float* FindMatchingVariableValue(const C178_PublishedDynamicData_Resp& p_rResp, const DevicePtr device, const PublishChannel& p_rChannel)
{
    if (p_rChannel.deviceVariable == DeviceVariableCodes_PrimaryVariable ||
        p_rChannel.deviceVariable == p_rResp.VarCodes_Primary ||
        p_rChannel.deviceVariable == device->GetDynamicVariableAssignment(p_rResp.VarCodes_Primary))
        return &(p_rResp.VarValue_Primary);
    else if (p_rChannel.deviceVariable == DeviceVariableCodes_SecondaryVariable ||
             p_rChannel.deviceVariable == p_rResp.VarCodes_Secondary ||
             p_rChannel.deviceVariable == device->GetDynamicVariableAssignment(p_rResp.VarCodes_Secondary))
        return &(p_rResp.VarValue_Secondary);
    else if (p_rChannel.deviceVariable == DeviceVariableCodes_TertiaryVariable ||
             p_rChannel.deviceVariable == p_rResp.VarCodes_Tertiary ||
             p_rChannel.deviceVariable == device->GetDynamicVariableAssignment(p_rResp.VarCodes_Tertiary))
        return &(p_rResp.VarValue_Tertiary);
    else if (p_rChannel.deviceVariable == DeviceVariableCodes_QuaternaryVariable ||
             p_rChannel.deviceVariable == p_rResp.VarCodes_Quaternary ||
             p_rChannel.deviceVariable == device->GetDynamicVariableAssignment(p_rResp.VarCodes_Quaternary))
        return &(p_rResp.VarValue_Quaternary);
    else if (p_rChannel.deviceVariable == p_rResp.VarCodes_Percent)
        return &(p_rResp.VarValue_Percent);
    else if (p_rChannel.deviceVariable == p_rResp.VarCodes_LoopC)
        return &(p_rResp.VarValue_LoopC);
    return 0;
}

void ResponseProcessor::Process(gateway::GatewayIO::GWResponse &p_rResponse, CommandsProcessor& p_rProcessor)
{
    m_pProcessor = &p_rProcessor;
    m_pResponse = &p_rResponse;

    m_pResponse->appData.appCmd->Accept(*this);
}

//topology
void ResponseProcessor::Visit(AppTopologyCommand& p_rTopologyCmd)
{
    //LOG_INFO_APP("[ResponseProcessor::Visit]:AppTopologyCommand. Device=");
    //topology means talking to gateway
    try
    {
        switch (p_rTopologyCmd.m_state)
        {
            case AppTopologyCommand::GetUniqueIDs_state:
            {
                //errors
                LOG_DEBUG_APP("[TOPOLOGY]: begin state = GetUniqueIDs");
                if (SaveRespErr::IsErr_814(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
                {
                    AppTopologyCommand noTopolgy;
                    p_rTopologyCmd.pDevices->ProcessTopology(noTopolgy.m_Report.m_DevicesList, TopologyReport::NO_TOPO_GW_INDEX, true);
                    SaveRespErr(*p_rTopologyCmd.pCommands, p_rTopologyCmd.dbCommand).SaveErr_814(m_pResponse->hostErr,
                                                                                                 m_pResponse->whCmd.responseCode);
                    return;
                }

                C814_ReadDeviceListEntries_Resp *pwhResp = (C814_ReadDeviceListEntries_Resp*) m_pResponse->whCmd.command;
                if (!pwhResp)
                {
                    LOG_ERROR_APP("[TOPOLOGY]: Incorrect response received for CMD814.");
                    return;
                }

                if (pwhResp->m_unStartingListIndex == 0)
                    p_rTopologyCmd.m_Report.m_DevicesList.reserve(pwhResp->m_unTotalNoOfEntriesInList);

                LOG_DEBUG_APP("[TOPOLOGY]: device_list received has elements no. = " << (int) pwhResp->m_ucNoOfListEntriesRead
                            << " from indexNo = " << pwhResp->m_unStartingListIndex);
                LOG_DEBUG_APP("[TOPOLOGY]: device_list -> begin");
                for (int i = 0; i < pwhResp->m_ucNoOfListEntriesRead; i++)
                {
                    p_rTopologyCmd.m_Report.m_DevicesList.push_back(TopologyReport::Device());
                    TopologyReport::Device &device = *p_rTopologyCmd.m_Report.m_DevicesList.rbegin();
                    device.DeviceMAC = MAC(pwhResp->m_aDeviceUniqueIds[i]);
                    LOG_INFO_APP("[TOPOLOGY]: device[" << i << "] has mac=" << device.DeviceMAC);

                    const WHartUniqueID &devAddr = device.DeviceMAC.Address();
                    const WHartUniqueID &gwAddr = stack::Gateway_UniqueID();
                    const WHartUniqueID &nnAddr = stack::NetworkManager_UniqueID();
                    if (!memcmp(devAddr.bytes, gwAddr.bytes, sizeof(devAddr.bytes)))
                    {
                        device.DeviceType = Device::dtGateway;
                        p_rTopologyCmd.m_Report.m_GWIndex = i;
                        continue;
                    }
                    if (!memcmp(devAddr.bytes, nnAddr.bytes, sizeof(devAddr.bytes)))
                    {
                        device.DeviceType = Device::dtSystemManager;
                        continue;
                    }

                    //
                    device.DeviceType = Device::dtRoutingDeviceNode;
                }
                LOG_DEBUG_APP("[TOPOLOGY]: device_list -> end");

                if (pwhResp->m_unTotalNoOfEntriesInList == 0)
                {
                    p_rTopologyCmd.pCommands->SetCommandResponded(p_rTopologyCmd.dbCommand, nlib::CurrentUniversalTime(), "success", NULL);
                    return;
                }

                if (p_rTopologyCmd.m_Report.m_DevicesList.size() < pwhResp->m_unTotalNoOfEntriesInList)
                {

                    LOG_INFO_APP("[TOPOLOGY]: device_list received total elements = " << p_rTopologyCmd.m_Report.m_DevicesList.size()
                                << "from the expected total elements = " << pwhResp->m_unTotalNoOfEntriesInList
                                << ", so send the same cmd...");

                    //get uniqueDeviceIDs
                    C814_ReadDeviceListEntries_Req whReq;
                    whReq.m_ucDeviceListCode = 0 /*= Active device list. There is no implementation for: 1 = Whitelisted devices, 2 = Blacklisted devices*/;
                    whReq.m_ucNoOfListEntriesToRead = MAX_DEVICE_LIST_ENTRIES_NO;
                    whReq.m_unStartingListIndex = pwhResp->m_unStartingListIndex + pwhResp->m_ucNoOfListEntriesRead;
                    stack::WHartCommand whCmd;
                    whCmd.commandID = CMDID_C814_ReadDeviceListEntries;
                    whCmd.command = &whReq;

                    //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C814_ReadDeviceListEntries. Device=GW");
                    m_pProcessor->gateway.SendWHRequest(m_pResponse->appData, stack::Gateway_UniqueID(), whCmd, false, true);
                    return;
                }

                //prepare for next state
                p_rTopologyCmd.m_state = AppTopologyCommand::GetNicknames_state;
                p_rTopologyCmd.m_sumOfAllOffsets = 0;
                for (int i = 0; i < (int) p_rTopologyCmd.m_Report.m_DevicesList.size(); i++)
                {
                    C832_ReadNetworkDeviceIdentity_Req req;
                    memcpy(req.DeviceUniqueID, p_rTopologyCmd.m_Report.m_DevicesList[i].DeviceMAC.Address().bytes,
                           sizeof(req.DeviceUniqueID));
                    stack::WHartCommand whCmd;
                    whCmd.commandID = CMDID_C832_ReadNetworkDeviceIdentity;
                    whCmd.command = &req;

                    m_pResponse->appData.innerDataHandle = i + 1;

                    //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C832_ReadNetworkDeviceIdentity. Device=NM");
                    m_pProcessor->gateway.SendWHRequest(m_pResponse->appData, stack::NetworkManager_UniqueID(), whCmd, false, true);

                    p_rTopologyCmd.m_sumOfAllOffsets += i + 1;
                }
                assert(p_rTopologyCmd.m_sumOfAllOffsets == (int) (p_rTopologyCmd.m_Report.m_DevicesList.size()
                            * (p_rTopologyCmd.m_Report.m_DevicesList.size() + 1) / 2));

                LOG_DEBUG_APP("[TOPOLOGY]: end state = GetUniqueIDs with devices_no=" << p_rTopologyCmd.m_Report.m_DevicesList.size());
            }
            break;

            case AppTopologyCommand::GetNicknames_state:
            {

                LOG_DEBUG_APP("[TOPOLOGY]: begin state = GetNicknames for device="
                            << p_rTopologyCmd.m_Report.m_DevicesList[m_pResponse->appData.innerDataHandle - 1].DeviceMAC);

                assert(m_pResponse->appData.innerDataHandle > 0 && m_pResponse->appData.innerDataHandle
                            <= (int) p_rTopologyCmd.m_Report.m_DevicesList.size());
                p_rTopologyCmd.m_sumOfAllOffsets -= m_pResponse->appData.innerDataHandle;
                assert(p_rTopologyCmd.m_sumOfAllOffsets >= 0);

                //errors
                if (!SaveRespErr::IsErr_832(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
                {
                    C832_ReadNetworkDeviceIdentity_Resp *pwhResp = (C832_ReadNetworkDeviceIdentity_Resp*) m_pResponse->whCmd.command;
                    if (!pwhResp)
                    {
                        LOG_ERROR_APP("[TOPOLOGY]: Incorrect response received for CMD832.");
                        return;
                    }
                    p_rTopologyCmd.m_Report.m_DevicesList[m_pResponse->appData.innerDataHandle - 1].DeviceNickName
                                = NickName(pwhResp->Nickname);

                    std::string longTag;
                    for (unsigned int i = 0; i < sizeof(pwhResp->LongTag); ++i)
                        longTag += ConvertToHex((unsigned char) pwhResp->LongTag[i]);

                    p_rTopologyCmd.m_Report.m_DevicesList[m_pResponse->appData.innerDataHandle - 1].LongTag = longTag;

                    LOG_INFO_APP("[TOPOLOGY]: nickname received = "
                                << p_rTopologyCmd.m_Report.m_DevicesList[m_pResponse->appData.innerDataHandle - 1].DeviceNickName);
                }
                else
                {
                    //if gw's disconnected, the command with multiple states should stop
                    if (m_pResponse->hostErr == gateway::GatewayIO::HostDisconnected)
                    {
                        SaveRespErr(*p_rTopologyCmd.pCommands, p_rTopologyCmd.dbCommand).SaveErr_832(m_pResponse->hostErr,
                                                                                                     m_pResponse->whCmd.responseCode);
                        return;
                    }
                }

                if (p_rTopologyCmd.m_sumOfAllOffsets == 0)
                {
                    p_rTopologyCmd.m_state = AppTopologyCommand::GetNeighbsPerGraphID_state;

                    for (int i = 0; i < (int) p_rTopologyCmd.m_Report.m_DevicesList.size(); i++)
                    {
                        C785_ReadGraphList_Req req;
                        req.m_ucGraphListIndex = 0;
                        stack::WHartCommand whCmd;
                        whCmd.commandID = CMDID_C785_ReadGraphList;
                        whCmd.command = &req;

                        m_pResponse->appData.innerDataHandle = i + 1;
                        //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C785_ReadGraphList. Device=" << p_rTopologyCmd.m_Report.m_DevicesList[i].DeviceMAC);
                        m_pProcessor->gateway.SendWHRequest(m_pResponse->appData,
                                                            (const WHartUniqueID&) p_rTopologyCmd.m_Report.m_DevicesList[i].DeviceMAC.Address(),
                                                            whCmd, false, true);

                        p_rTopologyCmd.m_sumOfAllOffsets += i + 1;
                    }
                }
                LOG_DEBUG_APP("[TOPOLOGY]: end state = GetNicknames with devices_no=" << p_rTopologyCmd.m_Report.m_DevicesList.size());
                break;
            }
            case AppTopologyCommand::GetNeighbsPerGraphID_state:
            {
                LOG_DEBUG_APP("[TOPOLOGY]: begin state = GetNeighbsPerGraphID for device="
                            << p_rTopologyCmd.m_Report.m_DevicesList[m_pResponse->appData.innerDataHandle - 1].DeviceMAC
                            << "with graphlist_size="
                            << p_rTopologyCmd.m_Report.m_DevicesList[m_pResponse->appData.innerDataHandle - 1].GraphsList.size());

                if (!SaveRespErr::IsErr_785(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
                {
                    C785_ReadGraphList_Resp *pwhResp = (C785_ReadGraphList_Resp*) m_pResponse->whCmd.command;
                    if (!pwhResp)
                    {
                        LOG_ERROR_APP("[TOPOLOGY]: Incorrect response received for CMD785. DeviceMAC="
                                    << p_rTopologyCmd.m_Report.m_DevicesList[m_pResponse->appData.innerDataHandle - 1].DeviceMAC);
                        return;
                    }

                    if (pwhResp->m_ucGraphListIndex == 0)
                        p_rTopologyCmd.m_Report.m_DevicesList[m_pResponse->appData.innerDataHandle - 1].GraphsList.reserve(
                                                                                                                           pwhResp->m_ucTotalNoOfGraphs);

                    p_rTopologyCmd.m_Report.m_DevicesList[m_pResponse->appData.innerDataHandle - 1].GraphsList.push_back(
                                                                                                                         TopologyReport::Device::Graph());
                    TopologyReport::Device::Graph &graphID = *p_rTopologyCmd.m_Report.m_DevicesList[m_pResponse->appData.innerDataHandle
                                - 1].GraphsList.rbegin();
                    graphID.GraphID = pwhResp->m_unGraphId;

                    //SET ACCESS_POINT TYPE
                    const WHartUniqueID &devAddr =
                                p_rTopologyCmd.m_Report.m_DevicesList[m_pResponse->appData.innerDataHandle - 1].DeviceMAC.Address();
                    const WHartUniqueID &gwAddr = stack::Gateway_UniqueID();
                    const WHartUniqueID &nnAddr = stack::NetworkManager_UniqueID();
                    bool isDevice = false;
                    if (memcmp(devAddr.bytes, gwAddr.bytes, sizeof(devAddr.bytes)) && memcmp(devAddr.bytes, nnAddr.bytes,
                                                                                             sizeof(devAddr.bytes)))
                    {
                        isDevice = true;
                    }

                    graphID.neighbList.reserve(pwhResp->m_ucNoOfNeighbors);
                    for (int i = 0; i < pwhResp->m_ucNoOfNeighbors; i++)
                    {
                        NickName nick(pwhResp->m_aNicknameOfNeighbor[i]);
                        if (isDevice)
                        {
                            if (nick.Address() == stack::Gateway_Nickname())
                                p_rTopologyCmd.m_Report.m_DevicesList[m_pResponse->appData.innerDataHandle - 1].DeviceType
                                            = Device::dtAccessPoint;
                        }
                        graphID.neighbList.push_back(nick);

                    }

                    if (p_rTopologyCmd.m_Report.m_DevicesList[m_pResponse->appData.innerDataHandle - 1].GraphsList.size()
                                < pwhResp->m_ucTotalNoOfGraphs)
                    {
                        C785_ReadGraphList_Req req;
                        req.m_ucGraphListIndex = pwhResp->m_ucGraphListIndex + 1;
                        stack::WHartCommand whCmd;
                        whCmd.commandID = CMDID_C785_ReadGraphList;
                        whCmd.command = &req;

                        //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C785_ReadGraphList. Device=" << p_rTopologyCmd.m_Report.m_DevicesList[m_pResponse->appData.innerDataHandle - 1].DeviceMAC);
                        m_pProcessor->gateway.SendWHRequest(m_pResponse->appData, (const WHartUniqueID&) p_rTopologyCmd.m_Report.m_DevicesList[m_pResponse->appData.innerDataHandle - 1].DeviceMAC.Address(), whCmd, false, true);
                        return;
                    }
                }
                else
                {
                    //if gw's disconnected, the command with multiple states should stop
                    if (m_pResponse->hostErr == gateway::GatewayIO::HostDisconnected)
                    {
                        SaveRespErr(*p_rTopologyCmd.pCommands, p_rTopologyCmd.dbCommand).SaveErr_785(m_pResponse->hostErr,
                                                                                                     m_pResponse->whCmd.responseCode);
                        return;
                    }
                }

                p_rTopologyCmd.m_sumOfAllOffsets -= m_pResponse->appData.innerDataHandle;
                if (p_rTopologyCmd.m_sumOfAllOffsets == 0)
                {
                    p_rTopologyCmd.m_state = AppTopologyCommand::GetDeviceDetails_state;

                    for (int i = 0; i < (int) p_rTopologyCmd.m_Report.m_DevicesList.size(); i++)
                    {
                        C000_ReadUniqueIdentifier_Req req;
                        stack::WHartCommand whCmd;
                        whCmd.commandID = CMDID_C000_ReadUniqueIdentifier;
                        whCmd.command = &req;

                        m_pResponse->appData.innerDataHandle = i + 1;
                        //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C000_ReadUniqueIdentifier. Device=" << p_rTopologyCmd.m_Report.m_DevicesList[i].DeviceMAC);
                        m_pProcessor->gateway.SendWHRequest(m_pResponse->appData,
                                                            p_rTopologyCmd.m_Report.m_DevicesList[i].DeviceMAC.Address(), whCmd, false,
                                                            true);

                        p_rTopologyCmd.m_sumOfAllOffsets += i + 1;
                    }
                }
                LOG_DEBUG_APP("[TOPOLOGY]: end state = GetNeighbsPerGraphID with devices_no="
                            << p_rTopologyCmd.m_Report.m_DevicesList.size());
                break;
            }
            case AppTopologyCommand::GetDeviceDetails_state:
            {

                LOG_DEBUG_APP("[TOPOLOGY]: begin state = GetDeviceDetails for device="
                            << p_rTopologyCmd.m_Report.m_DevicesList[m_pResponse->appData.innerDataHandle - 1].DeviceMAC);

                if (!SaveRespErr::IsErr_000(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
                {
                    C000_ReadUniqueIdentifier_Resp *pwhResp = (C000_ReadUniqueIdentifier_Resp*) m_pResponse->whCmd.command;
                    if (!pwhResp)
                    {
                        LOG_ERROR_APP("[TOPOLOGY]: Incorrect response received for CMD000. DeviceMAC="
                                    << p_rTopologyCmd.m_Report.m_DevicesList[m_pResponse->appData.innerDataHandle - 1].DeviceMAC);
                        return;
                    }

                    const WHartUniqueID &gwAddr = stack::Gateway_UniqueID();
                    const WHartUniqueID &nnAddr = stack::NetworkManager_UniqueID();
                    MAC &deviceMAC = p_rTopologyCmd.m_Report.m_DevicesList[m_pResponse->appData.innerDataHandle - 1].DeviceMAC;
                    bool isDevice = false;
                    if (memcmp(deviceMAC.Address().bytes, gwAddr.bytes, sizeof(deviceMAC.Address().bytes))
                                && memcmp(deviceMAC.Address().bytes, nnAddr.bytes, sizeof(deviceMAC.Address().bytes)))
                    {
                        isDevice = true;
                    }

                    if (pwhResp->deviceProfile == DeviceProfileCodes_WIRELESSHART_GATEWAY)
                        if (isDevice == true)
                            p_rTopologyCmd.m_Report.m_DevicesList[m_pResponse->appData.innerDataHandle - 1].DeviceType
                                        = Device::dtAccessPoint;

                    p_rTopologyCmd.m_Report.m_DevicesList[m_pResponse->appData.innerDataHandle - 1].softwareRev
                                = pwhResp->softwareRevisionLevel;
                    p_rTopologyCmd.m_Report.m_DevicesList[m_pResponse->appData.innerDataHandle - 1].deviceCode
                                = pwhResp->expandedDeviceType;
                    p_rTopologyCmd.m_Report.m_DevicesList[m_pResponse->appData.innerDataHandle - 1].adapter = ((pwhResp->flags
                                & FlagAssignmentsMask_ProtocolBridge) > 0);
                }
                else
                {
                    //if gw's disconnected, the command with multiple states should stop
                    if (m_pResponse->hostErr == gateway::GatewayIO::HostDisconnected)
                    {
                        SaveRespErr(*p_rTopologyCmd.pCommands, p_rTopologyCmd.dbCommand).SaveErr_000(m_pResponse->hostErr,
                                                                                                     m_pResponse->whCmd.responseCode);
                        return;
                    }
                }

                p_rTopologyCmd.m_sumOfAllOffsets -= m_pResponse->appData.innerDataHandle;
                if (p_rTopologyCmd.m_sumOfAllOffsets == 0)
                {
                    for (int i = 0; i < (int) p_rTopologyCmd.m_Report.m_DevicesList.size(); i++)
                    {
                        //should be added to speed-up
                        {
                            C837_WriteUpdateNotificationBitMask_Req whReq;
                            memcpy(whReq.UniqueID, p_rTopologyCmd.m_Report.m_DevicesList[i].DeviceMAC.Address().bytes,
                                   sizeof(WHartUniqueID));

                            //is device in db?
                            DevicePtr dev = p_rTopologyCmd.pDevices->FindDevice(p_rTopologyCmd.m_Report.m_DevicesList[i].DeviceMAC);
                            unsigned short notifBitMask = 0;
                            if (dev)
                            {
                                notifBitMask = dev->GetNotificationBitMask();

                                if (notifBitMask & NotificationMaskCodesMask_DeviceConfiguration)
                                    continue;

                                notifBitMask = notifBitMask | NotificationMaskCodesMask_DeviceConfiguration;
                                dev->SetNotificationBitMask(notifBitMask);

                                whReq.ChangeNotificationFlags = notifBitMask;
                            }
                            else
                            {
                                notifBitMask = notifBitMask | NotificationMaskCodesMask_DeviceConfiguration;
                                whReq.ChangeNotificationFlags = notifBitMask;
                            }

                            p_rTopologyCmd.m_Report.m_DevicesList[i].NotificationMask = notifBitMask;

                            stack::WHartCommand whCmd;
                            whCmd.commandID = CMDID_C837_WriteUpdateNotificationBitMask;
                            whCmd.command = &whReq;

                            hostapp::AbstractAppCommandPtr tmp = m_pResponse->appData.appCmd;
                            m_pResponse->appData.appCmd
                                        = AbstractAppCommandPtr(
                                                                new AppSetNotificationBitMask(
                                                                                              p_rTopologyCmd.m_Report.m_DevicesList[i].DeviceMAC,
                                                                                              notifBitMask));
                            m_pResponse->appData.appCmd->pCommands = p_rTopologyCmd.pCommands;
                            m_pResponse->appData.appCmd->pDevices = p_rTopologyCmd.pDevices;
                            //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C837_WriteUpdateNotificationBitMask. Device=GW");
                            m_pProcessor->gateway.SendWHRequest(m_pResponse->appData, stack::Gateway_UniqueID(), whCmd, false, true);
                            m_pResponse->appData.appCmd = tmp;
                        }
                    }
                    p_rTopologyCmd.m_state = AppTopologyCommand::GetDeviceStatistics_state;
                    for (int i = 0; i < (int) p_rTopologyCmd.m_Report.m_DevicesList.size(); i++)
                    {
                        C840_ReadDeviceStatistics_Req req;
                        memcpy(req.UniqueID, p_rTopologyCmd.m_Report.m_DevicesList[i].DeviceMAC.Address().bytes, sizeof(req.UniqueID));
                        stack::WHartCommand whCmd;
                        whCmd.commandID = CMDID_C840_ReadDeviceStatistics;
                        whCmd.command = &req;

                        m_pResponse->appData.innerDataHandle = i + 1;
                        LOG_INFO_APP("[TOPOLOGY]: send command 840 for mac"
                                    << p_rTopologyCmd.m_Report.m_DevicesList[i].DeviceMAC.ToString());
                        //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C840_ReadDeviceStatistics. Device=GW");
                        m_pProcessor->gateway.SendWHRequest(m_pResponse->appData, stack::Gateway_UniqueID(), /*header,*/whCmd, false, true);

                        p_rTopologyCmd.m_sumOfAllOffsets += i + 1;
                    }

                    //should be added to speed-up
                    {
                        p_rTopologyCmd.pDevices->ProcessTopology(p_rTopologyCmd.m_Report.m_DevicesList, p_rTopologyCmd.m_Report.m_GWIndex,
                                                                 p_rTopologyCmd.dbCommand.generatedType == DBCommand::cgtManual);
                        p_rTopologyCmd.pCommands->SetCommandResponded(p_rTopologyCmd.dbCommand, nlib::CurrentUniversalTime(), "success",
                                                                      &p_rTopologyCmd.m_Report);
                        //force command to be unknown
                        p_rTopologyCmd.dbCommand.commandID = DBCommand::NO_COMMAND_ID;
                    }
                }
                LOG_DEBUG_APP("[TOPOLOGY]: end state = GetDeviceDetails with devices_no=" << p_rTopologyCmd.m_Report.m_DevicesList.size());
                break;
            }
            case AppTopologyCommand::GetDeviceStatistics_state:
            {
                LOG_DEBUG_APP("[TOPOLOGY]: begin state = GetDeviceStatistics for device="
                            << p_rTopologyCmd.m_Report.m_DevicesList[m_pResponse->appData.innerDataHandle - 1].DeviceMAC);

                if (!SaveRespErr::IsErr_840(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
                {
                    C840_ReadDeviceStatistics_Resp *pwhResp = (C840_ReadDeviceStatistics_Resp*) m_pResponse->whCmd.command;
                    if (!pwhResp)
                    {
                        LOG_ERROR_APP("[TOPOLOGY]: Incorrect response received for CMD840. DeviceMAC="
                                    << p_rTopologyCmd.m_Report.m_DevicesList[m_pResponse->appData.innerDataHandle - 1].DeviceMAC);
                        return;
                    }

                    p_rTopologyCmd.m_Report.m_DevicesList[m_pResponse->appData.innerDataHandle - 1].joinCount = pwhResp->JoinCount;
                    LOG_INFO_APP("[TOPOLOGY]: GetDeviceStatistics received for device="
                                << p_rTopologyCmd.m_Report.m_DevicesList[m_pResponse->appData.innerDataHandle - 1].DeviceMAC
                                << " with JoinCount=" << pwhResp->JoinCount);
                }
                else
                {
                    //if gw's disconnected, the command with multiple states should stop
                    if (m_pResponse->hostErr == gateway::GatewayIO::HostDisconnected)
                    {
                        SaveRespErr(*p_rTopologyCmd.pCommands, p_rTopologyCmd.dbCommand).SaveErr_840(m_pResponse->hostErr,
                                                                                                     m_pResponse->whCmd.responseCode);
                        return;
                    }
                }

                p_rTopologyCmd.m_sumOfAllOffsets -= m_pResponse->appData.innerDataHandle;

                if (p_rTopologyCmd.m_sumOfAllOffsets == 0)
                {
                    p_rTopologyCmd.m_state = AppTopologyCommand::GetBatteryLife_state;
                    for (int i = 0; i < (int) p_rTopologyCmd.m_Report.m_DevicesList.size(); i++)
                    {

                        C778_ReadBatteryLife_Req req;
                        stack::WHartCommand whCmd;
                        whCmd.commandID = CMDID_C778_ReadBatteryLife;
                        whCmd.command = &req;

                        m_pResponse->appData.innerDataHandle = i + 1;
                        //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C778_ReadBatteryLife. Device=" << p_rTopologyCmd.m_Report.m_DevicesList[i].DeviceMAC);
                        m_pProcessor->gateway.SendWHRequest(m_pResponse->appData,
                                                            p_rTopologyCmd.m_Report.m_DevicesList[i].DeviceMAC.Address(), /*header,*/
                                                            whCmd, false, true);

                        p_rTopologyCmd.m_sumOfAllOffsets += i + 1;
                    }
                }
                LOG_DEBUG_APP("[TOPOLOGY]: end state = GetDeviceStatistics with devices_no=" << p_rTopologyCmd.m_Report.m_DevicesList.size());
                break;
            }
            case AppTopologyCommand::GetBatteryLife_state:
            {
                LOG_DEBUG_APP("[TOPOLOGY]: begin state = GetBatteryLife for device="
                            << p_rTopologyCmd.m_Report.m_DevicesList[m_pResponse->appData.innerDataHandle - 1].DeviceMAC);

                if (!SaveRespErr::IsErr_778(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
                {
                    C778_ReadBatteryLife_Resp *pwhResp = (C778_ReadBatteryLife_Resp*) m_pResponse->whCmd.command;
                    if (!pwhResp)
                    {
                        LOG_ERROR_APP("[TOPOLOGY]: Incorrect response received for CMD778. DeviceMAC="
                                    << p_rTopologyCmd.m_Report.m_DevicesList[m_pResponse->appData.innerDataHandle - 1].DeviceMAC);
                        return;
                    }
                    //set battery life
                    p_rTopologyCmd.m_Report.m_DevicesList[m_pResponse->appData.innerDataHandle - 1].BatteryLifeStatus
                                = pwhResp->m_unBatteryLifeRemaining;
                }
                else
                {
                    //if gw's disconnected, the command with multiple states should stop
                    if (m_pResponse->hostErr == gateway::GatewayIO::HostDisconnected)
                    {
                        SaveRespErr(*p_rTopologyCmd.pCommands, p_rTopologyCmd.dbCommand).SaveErr_778(m_pResponse->hostErr,
                                                                                                     m_pResponse->whCmd.responseCode);
                        return;
                    }
                }

                p_rTopologyCmd.m_sumOfAllOffsets -= m_pResponse->appData.innerDataHandle;
                if (p_rTopologyCmd.m_sumOfAllOffsets == 0)
                {
                    //should be added to speed-up
                    {
                        for (int i = 0; i < (int) p_rTopologyCmd.m_Report.m_DevicesList.size(); i++)
                        {
                            //is device in db?
                            DevicePtr dev = p_rTopologyCmd.pDevices->FindDevice(p_rTopologyCmd.m_Report.m_DevicesList[i].DeviceNickName);
                            if (dev)
                            {
                                dev->ResetChanged();
                                dev->BatteryLifeStatus(p_rTopologyCmd.m_Report.m_DevicesList[i].BatteryLifeStatus);
                                dev->SetRejoinCount(p_rTopologyCmd.m_Report.m_DevicesList[i].joinCount);
                                if (dev->Changed())
                                    p_rTopologyCmd.pDevices->UpdateDevice(dev, false, false);
                            }
                            else
                            {
                                LOG_WARN_APP("[TOPOLOGY]: device not found=" << p_rTopologyCmd.m_Report.m_DevicesList[i].DeviceMAC);
                            }
                        }
                    }

                }
                LOG_DEBUG_APP("[TOPOLOGY]: end state = GetBatteryLife with devices_no=" << p_rTopologyCmd.m_Report.m_DevicesList.size());
                break;
            }
            default:
                assert(false);
        }
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Visit::AppTopologyCommand]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Visit::AppTopologyCommand]: System error!");
    }
}

void ResponseProcessor::Visit(AppSetTopoNotificationCmd& p_rTopoNotificationCmd)
{
    //LOG_INFO_APP("[ResponseProcessor::Visit]:AppSetTopoNotificationCmd. Device=");

    try
    {
        if (m_pResponse->appData.cmdType != gateway::GatewayIO::AppData::CmdType_Multiple ||
            m_pResponse->pCmdWrappersList == NULL ||
            m_pResponse->pCmdWrappersList->size() != 1)
        {
            SaveRespErr(*p_rTopoNotificationCmd.pCommands, p_rTopoNotificationCmd.dbCommand).CommandFailed(
                DBCommand::rsFailure_InternalError);
            return;
        }

        stack::CHartCmdWrapperList::iterator cmdIt = m_pResponse->pCmdWrappersList->begin();

        if (SaveRespErr::IsErr_837(m_pResponse->hostErr, (*cmdIt)->GetResponseCode()))
        {
            SaveRespErr(*p_rTopoNotificationCmd.pCommands, p_rTopoNotificationCmd.dbCommand).SaveErr_837(m_pResponse->hostErr,
                m_pResponse->whCmd.responseCode);
            return;
        }

        AppTopologyNotificationPtr appTopologyNotificationPtr(new AppTopologyNotification());
        appTopologyNotificationPtr->pCommands = p_rTopoNotificationCmd.pCommands;
        appTopologyNotificationPtr->pDevices = p_rTopoNotificationCmd.pDevices;
        m_pProcessor->gateway.RegisterTopologyMsgFromDev(appTopologyNotificationPtr);

        AppReportsNotificationPtr appReportsNotificationPtr(new AppReportsNotification());
        appReportsNotificationPtr->pCommands = p_rTopoNotificationCmd.pCommands;
        appReportsNotificationPtr->pDevices = p_rTopoNotificationCmd.pDevices;
        m_pProcessor->gateway.RegisterReportsMsgFromDev(appReportsNotificationPtr);

        AppDevConfigNotificationPtr appDevConfigNotificationPtr(new AppDevConfigNotification());
        appDevConfigNotificationPtr->pCommands = p_rTopoNotificationCmd.pCommands;
        appDevConfigNotificationPtr->pDevices = p_rTopoNotificationCmd.pDevices;
        m_pProcessor->gateway.RegisterDevConfigMsgFromDev(appDevConfigNotificationPtr);

        AppNoBurstRspNotificationPtr appNoBurstRspNotificationPtr(new AppNoBurstRspNotification());
        appNoBurstRspNotificationPtr->pCommands = p_rTopoNotificationCmd.pCommands;
        appNoBurstRspNotificationPtr->pDevices = p_rTopoNotificationCmd.pDevices;
        m_pProcessor->gateway.RegisterForNoBurstRspNotif(appNoBurstRspNotificationPtr);

        p_rTopoNotificationCmd.pCommands->SetCommandResponded(p_rTopoNotificationCmd.dbCommand, nlib::CurrentUniversalTime(), "success", NULL);
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[SET_TOPOLOGY]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[SET_TOPOLOGY]: System error!");
    }
}

//notification bit mask
void ResponseProcessor::Visit(AppSetNotificationBitMask& p_rBitMaskCmd)
{
    //LOG_INFO_APP("[ResponseProcessor::Visit]:AppSetNotificationBitMask. Device=");
    try
    {
        if (SaveRespErr::IsErr_837(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_ERROR_APP("[DeviceNotifBitMask]: error setting bit_mask for device=" << p_rBitMaskCmd.m_devMac);
            return;
        }

        LOG_INFO_APP("[DeviceNotifBitMask]: device=" << p_rBitMaskCmd.m_devMac << " has bitmask(int)="
                    << (int) p_rBitMaskCmd.m_notificationBitMask);
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Visit::AppSetNotificationBitMask]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Visit::AppSetNotificationBitMask]: System error!");
    }
}

//topology notification
void ResponseProcessor::Visit(AppNoBurstRspNotification& p_rNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Visit]:AppNoBurstRspNotification. Device=");
    try
    {
        switch (p_rNotification.GetCmdType())
        {
            case AppNoBurstRspNotification::CMD_Report_000:
            {
                Process000RespReport(p_rNotification);
                break;
            }
            case AppNoBurstRspNotification::CMD_Report_020:
            {
                Process020RespReport(p_rNotification);
                break;
            }
            case AppNoBurstRspNotification::CMD_Report_769:
            {
                Process769RespReport(p_rNotification);
                break;
            }
            case AppNoBurstRspNotification::CMD_Report_779:
            {
                Process779RespReport(p_rNotification);
                break;
            }
            case AppNoBurstRspNotification::CMD_Report_780:
            {
                Process780RespReport(p_rNotification);
                break;
            }
            case AppNoBurstRspNotification::CMD_Report_783:
            {
                Process783RespReport(p_rNotification);
                break;
            }
            case AppNoBurstRspNotification::CMD_Report_784:
            {
                Process784RespReport(p_rNotification);
                break;
            }
            case AppNoBurstRspNotification::CMD_Report_785:
            {
                Process785RespReport(p_rNotification);
                break;
            }
            case AppNoBurstRspNotification::CMD_Report_787:
            {
                Process787RespReport(p_rNotification);
            }
            break;
            case AppNoBurstRspNotification::CMD_Alert_788:
            {
                Process788RespReport(p_rNotification);
                break;
            }
            case AppNoBurstRspNotification::CMD_Alert_789:
            {
                Process789RespReport(p_rNotification);
                break;
            }
            case AppNoBurstRspNotification::CMD_Alert_790:
            {
                Process790RespReport(p_rNotification);
                break;
            }
            case AppNoBurstRspNotification::CMD_Alert_791:
            {
                Process791RespReport(p_rNotification);
                break;
            }
            case AppNoBurstRspNotification::CMD_Report_800:
            {
                Process800RespReport(p_rNotification);
                break;
            }
            case AppNoBurstRspNotification::CMD_Report_801:
            {
                Process801RespReport(p_rNotification);
                break;
            }
            case AppNoBurstRspNotification::CMD_Report_802:
            {
                Process802RespReport(p_rNotification);
                break;
            }
            case AppNoBurstRspNotification::CMD_Report_803:
            {
                Process803RespReport(p_rNotification);
                break;
            }
            case AppNoBurstRspNotification::CMD_Report_814:
            {
                Process814RespReport(p_rNotification);
                break;
            }
            case AppNoBurstRspNotification::CMD_Report_818:
            {
                Process818RespReport(p_rNotification);
                break;
            }
            case AppNoBurstRspNotification::CMD_Report_832:
            {
                Process832RespReport(p_rNotification);
                break;
            }
            case AppNoBurstRspNotification::CMD_Report_833:
            {
                Process833RespReport(p_rNotification);
                break;
            }
            case AppNoBurstRspNotification::CMD_Report_834:
            {
                Process834RespReport(p_rNotification);
                break;
            }
            case AppNoBurstRspNotification::CMD_Report_965:
            {
                Process965RespReport(p_rNotification);
                break;
            }
            case AppNoBurstRspNotification::CMD_Report_966:
            {
                Process966RespReport(p_rNotification);
                break;
            }
            case AppNoBurstRspNotification::CMD_Report_967:
            {
                Process967RespReport(p_rNotification);
                break;
            }
            case AppNoBurstRspNotification::CMD_Report_968:
            {
                Process968RespReport(p_rNotification);
                break;
            }
            case AppNoBurstRspNotification::CMD_Report_969:
            {
                Process969RespReport(p_rNotification);
                break;
            }
            case AppNoBurstRspNotification::CMD_Report_970:
            {
                Process970RespReport(p_rNotification);
                break;
            }
            case AppNoBurstRspNotification::CMD_Report_971:
            {
                Process971RespReport(p_rNotification);
                break;
            }
            case AppNoBurstRspNotification::CMD_Report_973:
            {
                Process973RespReport(p_rNotification);
                break;
            }
            case AppNoBurstRspNotification::CMD_Report_974:
            {
                Process974RespReport(p_rNotification);
                break;
            }
            case AppNoBurstRspNotification::CMD_Report_975:
            {
                Process975RespReport(p_rNotification);
                break;
            }
            case AppNoBurstRspNotification::CMD_Report_976:
            {
                Process976RespReport(p_rNotification);
                break;
            }
            case AppNoBurstRspNotification::CMD_Report_977:
            {
                Process977RespReport(p_rNotification);
                break;
            }
            default:
                LOG_ERROR_APP("[AppNoBurstRspNotification]: a notification was treated yet!");
        }
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Visit::AppNoBurstRspNotification]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Visit::AppNoBurstRspNotification]: System error!");
    }
}

void ResponseProcessor::Process000RespReport(AppNoBurstRspNotification& p_rNotification)
{
    try
    {
        //LOG_INFO_APP("[ResponseProcessor::Process000RespReport]:AppNoBurstRspNotification. Device=");
        MAC devMac(p_rNotification.GetAddr().bytes);
        if (SaveRespErr::IsErr_000(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_ERROR_APP("[RespReport-000]: received with error for mac=" << devMac);
            return;
        }
        DevicePtr devicePtr = p_rNotification.pDevices->FindDevice(devMac);
        if (!devicePtr)
        {
            LOG_WARN_APP("[RespReport-000]: Device not in DB, with mac= " << devMac);
            return;
        }
        LOG_DEBUG_APP("[RespReport-000]: has been received for mac=" << devMac);

        C000_ReadUniqueIdentifier_Resp *pwhResp = (C000_ReadUniqueIdentifier_Resp*) m_pResponse->whCmd.command;
        if (!pwhResp)
        {
            LOG_ERROR_APP("[RespReport-000]: Incorrect response received for CMD000. DeviceMAC=" << devMac);
            return;
        }

        devicePtr->ResetChanged();

        devicePtr->SetSoftwareRevision(pwhResp->softwareRevisionLevel);
        devicePtr->SetDeviceCode(pwhResp->expandedDeviceType);

        LOG_DEBUG_APP("[RespReport-000]: IsAdapter[" << devMac << "]="
                    << (((pwhResp->flags & FlagAssignmentsMask_ProtocolBridge) > 0) ? "true" : "false"));

        devicePtr->SetAdapter(((pwhResp->flags & FlagAssignmentsMask_ProtocolBridge) > 0));

        const WHartUniqueID &gwAddr = stack::Gateway_UniqueID();
        const WHartUniqueID &nnAddr = stack::NetworkManager_UniqueID();
        bool isDevice = false;
        if (memcmp(devicePtr->Mac().Address().bytes, gwAddr.bytes, sizeof(devicePtr->Mac().Address().bytes)) && memcmp(
            devicePtr->Mac().Address().bytes, nnAddr.bytes, sizeof(devicePtr->Mac().Address().bytes)))
        {
            isDevice = true;
            devicePtr->Type(Device::dtRoutingDeviceNode);
        }

        if (pwhResp->deviceProfile == DeviceProfileCodes_WIRELESSHART_GATEWAY)
            if (isDevice == true)
                devicePtr->Type(Device::dtAccessPoint);

        if (devicePtr->Changed())
            p_rNotification.pDevices->UpdateDevice(devicePtr, false, false);
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Process000RespReport]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Process000RespReport]: System error!");
    }
}

void ResponseProcessor::Process020RespReport(AppNoBurstRspNotification& p_rNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Process020RespReport]:AppNoBurstRspNotification. Device=");
    try
    {
        MAC devMac(p_rNotification.GetAddr().bytes);
        if (SaveRespErr::IsErr_020(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_ERROR_APP("[RespReport-020]: received with error for mac=" << devMac);
            return;
        }
        DevicePtr dev = p_rNotification.pDevices->FindDevice(devMac);
        if (!dev)
        {
            LOG_WARN_APP("[RespReport-020]: Device not in DB, with mac= " << devMac);
            return;
        }
        LOG_INFO_APP("[RespReport-020]: has been received for mac=" << devMac);

        C020_ReadLongTag_Resp *pwhResp = (C020_ReadLongTag_Resp*) m_pResponse->whCmd.command;
        if (!pwhResp)
        {
            LOG_ERROR_APP("[RespReport-020]: Incorrect response received for CMD020. DeviceMAC=" << devMac);
            return;
        }

        dev->ResetChanged();

        std::string longTag;
        for (unsigned int i = 0; i < sizeof(pwhResp->longTag); ++i)
            longTag += ConvertToHex((unsigned char) pwhResp->longTag[i]);

        dev->SetTAG(longTag);

        if (dev->Changed())
            p_rNotification.pDevices->UpdateDevice(dev, false, false);
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Process020RespReport]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Process020RespReport]: System error!");
    }
}

void ResponseProcessor::Process769RespReport(AppNoBurstRspNotification& p_rNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Process769RespReport]:AppNoBurstRspNotification. Device=");
    try
    {
        MAC devMac(p_rNotification.GetAddr().bytes);
        if (SaveRespErr::IsErr_769(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_ERROR_APP("[RespReport-769]: received with error for mac=" << devMac);
            return;
        }

        C769_ReadJoinStatus_Resp *pwhResp = (C769_ReadJoinStatus_Resp *) m_pResponse->whCmd.command;
        if (!pwhResp)
        {
            LOG_ERROR_APP("[RespReport-769]: Incorrect response received for CMD769. DeviceMAC=" << devMac);
            return;
        }

        LOG_INFO_APP("[RespReport-769::SendWHRequest]: has been received for mac=" << devMac << " with status=" << std::hex << (int) (pwhResp->joinStatus));

        DevicePtr dev = p_rNotification.pDevices->FindDevice(devMac);
        if (!dev)
        {
            LOG_WARN_APP("[RespReport-769]: Device not in DB, with mac= " << devMac << ", so it is added in db");

            //WITHOUT 832
            {
                //insert bd
                dev = DevicePtr(new Device());
                dev->Mac(devMac);
                dev->Nickname(NickName(0));
                dev->Status((Device::DeviceStatus) pwhResp->joinStatus);
                if (dev->Type() == Device::dtUnknown)
                {
                    dev->Type(Device::dtRoutingDeviceNode);
                }
                if (MAC(stack::Gateway_UniqueID().bytes) == devMac)
                {
                    dev->Type(Device::dtGateway);
                }
                if (devMac == MAC(stack::NetworkManager_UniqueID().bytes))
                {
                    dev->Type(Device::dtSystemManager);
                    dev->Nickname(stack::NetworkManager_Nickname());
                }
                p_rNotification.pDevices->AddNewDevice(dev);
                return;
            }
        }

        {
            //update bd
            bool statusChanged = false;
            if (dev->Status() != (Device::DeviceStatus) pwhResp->joinStatus)
            {
                dev->Status((Device::DeviceStatus) pwhResp->joinStatus);
                statusChanged = true;
            }
            // upate device rejoinCount
            if (Device::dtSystemManager != dev->Type() &&
                Device::dtGateway != dev->Type() &&
                Device::dsRegistered == (Device::DeviceStatus) pwhResp->joinStatus)
            {
                int rejoinCount = dev->GetRejoinCount();
                dev->SetRejoinCount(++rejoinCount);
                LOG_DEBUG_APP("[RespReport-769]: SetRejoinCount(" << rejoinCount << ") for DeviceMAC=" << devMac);
                dev->SetPublishStatus(Device::PS_NO_DATA);
                LOG_DEBUG_APP("[RespReport-769]: SetPublishStatus(PS_NO_DATA) for DeviceMAC=" << devMac);
            }

            p_rNotification.pDevices->UpdateDevice(dev, false, statusChanged);
            LOG_DEBUG_APP("[RespReport-769]: device with mac= " << devMac << " with status=" << (int) (pwhResp->joinStatus) << " updated in db");

            if (Device::dsJoinFailed == (Device::DeviceStatus) pwhResp->joinStatus ||
                Device::dsJoinRequested == (Device::DeviceStatus) pwhResp->joinStatus)
            {
                p_rNotification.pDevices->CleanReports(dev->id);
                LOG_INFO_APP("[RespReport-769]:Join " << ((Device::DeviceStatus) pwhResp->joinStatus == Device::dsJoinFailed ? "failed" : "requested")
                            << " for mac=" << dev->Mac().ToString() << "so clean reports...");
            }
        }

        // If command 769 comes for NM or GW then NM reports will be cleaned
        if (Device::dtSystemManager == dev->Type() ||
            Device::dtGateway == dev->Type())
        {
            p_rNotification.pDevices->CleanReports(dev->id);
            if (Device::dtGateway == dev->Type())
            {
                p_rNotification.pDevices->ResetTopology();
            }
            LOG_INFO_APP("[RespReport-769]: Clean reports due to a " << (Device::dtGateway == dev->Type() ? "GW" : "NM") << " rejoin.");
        }

        if (Device::dtAccessPoint == dev->Type() &&
            Device::dsJoinRequested == pwhResp->joinStatus)
        {
            LOG_INFO_APP("[RespReport-769]: Clean reports for NM & GW due to an AP  rejoin");
            p_rNotification.pDevices->CleanReports(p_rNotification.pDevices->GatewayDevice()->id);
            p_rNotification.pDevices->CleanReports(p_rNotification.pDevices->SystemManagerDevice()->id);
        }

        // cleans log for setting publishers
        if (Device::dsRegistered == pwhResp->joinStatus)
        {
            p_rNotification.pDevices->DeleteSetPublishersLog(dev->id);
        }

        if (Device::dtAccessPoint == dev->Type() &&
            Device::dsRegistered == pwhResp->joinStatus)
        {
            bool resetTopology = false;
            DevicesManager::const_iterator_by_mac itDevices = p_rNotification.pDevices->BeginAllByMac();
            for (; itDevices != p_rNotification.pDevices->EndAllByMac(); ++itDevices)
            {
                DevicePtr device = (DevicePtr) itDevices->second;
                if (Device::dsRegistered == device->Status() &&
                    (Device::dtSystemManager != device->Type() &&
                     Device::dtGateway != device->Type() &&
                     Device::dtAccessPoint != device->Type()))
                {
                    resetTopology = true;
                }
            }

            if (resetTopology)
            {
                p_rNotification.pDevices->ResetTopology();
                LOG_INFO_APP("[RespReport-769]: Reset Topology due to an AP rejoin.");
            }

            p_rNotification.pDevices->IssueTopologyCommand();
            LOG_INFO_APP("[RespReport-769]: Get Topology due to an AP rejoin.");
        }
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Process769RespReport]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Process769RespReport]: System error!");
    }
}

void ResponseProcessor::Process779RespReport(AppNoBurstRspNotification& p_rNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Process779RespReport]:AppNoBurstRspNotification. Device=");
    try
    {
        MAC devMac(p_rNotification.GetAddr().bytes);
        LOG_INFO_APP("[RespReport-779]: has been received for mac=" << devMac);
        if (SaveRespErr::IsErr_779(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_ERROR_APP("[RespReport-779]: received with error for mac" << devMac);
            return;
        }

        DevicePtr dev = p_rNotification.pDevices->FindDevice(devMac);
        if (!dev)
        {
            LOG_WARN_APP("[RespReport-779]: Device not in DB, with mac= " << devMac);
            return;
        }
        C779_ReportDeviceHealth_Resp *pResp = (C779_ReportDeviceHealth_Resp*) m_pResponse->whCmd.command;
        if (!pResp)
        {
            LOG_ERROR_APP("[RespReport-779]: Incorrect response received for CMD779. DeviceMAC=" << devMac);
            return;
        }

        DeviceHealth devHealth(dev->id, dev->Mac(), pResp->m_ucPowerStatus, pResp->m_unNoOfPacketsGeneratedByDevice,
            pResp->m_unNoOfPacketsTerminatedByDevice, pResp->m_ucNoOfDataLinkLayerMICFailuresDetected,
            pResp->m_ucNoOfNetworkLayerMICFailuresDetected, pResp->m_ucNoOfCRCErrorsDetected, pResp->m_ucNoOfNonceCounterValuesNotReceived);

        p_rNotification.pDevices->ReplaceDeviceHealthReport(devHealth);

        dev->ResetChanged();
        dev->PowerStatus(pResp->m_ucPowerStatus);
        if (dev->Changed())
            p_rNotification.pDevices->UpdateDevice(dev, false, false);
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Process779RespReport]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Process779RespReport]: System error!");
    }
}

void ResponseProcessor::Process780RespReport(AppNoBurstRspNotification& p_rNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Process780RespReport]:AppNoBurstRspNotification. Device=");
    try
    {
        MAC devMac(p_rNotification.GetAddr().bytes);
        LOG_INFO_APP("[RespReport-780]: has been received for mac=" << devMac);
        if (SaveRespErr::IsErr_780(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_ERROR_APP("[RespReport-780]: received with error for mac" << devMac);
            return;
        }
        DevicePtr dev = p_rNotification.pDevices->FindDevice(devMac);
        if (!dev)
        {
            LOG_WARN_APP("[RespReport-780]: Device not in DB, with mac= " << devMac);
            return;
        }
        C780_ReportNeighborHealthList_Resp *pResp = (C780_ReportNeighborHealthList_Resp*) m_pResponse->whCmd.command;
        if (!pResp)
        {
            LOG_ERROR_APP("[RespReport-780]: Incorrect response received for CMD780. DeviceMAC=" << devMac);
            return;
        }

        if (pResp->m_ucNoOfNeighborEntriesRead <= 0)
        {
            LOG_WARN_APP("[RespReport-780]: the number of neighbors=" << (int) pResp->m_ucNoOfNeighborEntriesRead);
            return;
        }

        //init deviceNeighborHealths
        DeviceNeighborsHealth m_oNeighborsHealthCache(dev->id, dev->Mac());
        for (int i = 0; i < pResp->m_ucNoOfNeighborEntriesRead; i++)
        {
            DevicePtr device = p_rNotification.pDevices->FindDevice(NickName(pResp->m_aNeighbors[i].nicknameOfNeighbor));
            if (!device)
            {
                LOG_WARN_APP("[RespReport-780]: not found in db the neighbor =" << (NickName(pResp->m_aNeighbors[i].nicknameOfNeighbor)));
                continue;
            }
            NeighborHealth nh(device->id, pResp->m_aNeighbors[i].neighborFlags, pResp->m_aNeighbors[i].meanRSLSinceLastReport,
                pResp->m_aNeighbors[i].noOfPacketsTransmittedToThisNeighbor, pResp->m_aNeighbors[i].noOfFailedTransmits,
                pResp->m_aNeighbors[i].noOfPacketsReceivedFromThisNeighbor);
            m_oNeighborsHealthCache.m_oNeighborsList.push_back(nh);
        }
        p_rNotification.pDevices->ChangeDeviceNeighborsHealth(m_oNeighborsHealthCache);
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Process780RespReport]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Process780RespReport]: System error!");
    }
}

void ResponseProcessor::Process783RespReport(AppNoBurstRspNotification& p_rNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Process783RespReport]:AppNoBurstRspNotification. Device=");
    try
    {
        MAC devMac(p_rNotification.GetAddr().bytes);
        LOG_INFO_APP("[RespReport-783]: has been received for mac=" << devMac);
        if (SaveRespErr::IsErr_783(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_DEBUG_APP("[RespReport-783]: superframes received with error for mac=" << devMac);
            return;
        }

        DevicePtr dev = p_rNotification.pDevices->FindDevice(devMac);
        if (!dev)
        {
            LOG_WARN_APP("[RespReport-783]: Device not in DB, with mac= " << devMac);
            return;
        }

        C783_ReadSuperframeList_Resp *pResp = (C783_ReadSuperframeList_Resp*) m_pResponse->whCmd.command;
        if (!pResp)
        {
            LOG_ERROR_APP("[RespReport-783]: Incorrect response received for CMD783. DeviceMAC=" << devMac);
            return;
        }

        if (pResp->m_ucNoOfEntriesRead <= 0)
        {
            LOG_WARN_APP("[RespReport-783]: the number of entries=" << (int) pResp->m_ucNoOfEntriesRead);
            return;
        }

        //save Superframes
        SuperframesListT Superframes;
        for (int i = 0; i < pResp->m_ucNoOfEntriesRead; i++)
        {
            Superframe sFrame(pResp->m_aSuperframes[i].superframeId, dev->id, pResp->m_aSuperframes[i].noOfSlotsInSuperframe,
                ((pResp->m_aSuperframes[i].superframeModeFlags & SuperframeModeFlagsMask_Active) != 0),
                ((pResp->m_aSuperframes[i].superframeModeFlags & SuperframeModeFlagsMask_HandheldSuperframe) != 0));
            Superframes.push_back(sFrame);
        }
        p_rNotification.pDevices->ChangeSuperframes(Superframes);
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Process783RespReport]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Process783RespReport]: System error!");
    }
}

void ResponseProcessor::Process784RespReport(AppNoBurstRspNotification& p_rNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Process784RespReport]:AppNoBurstRspNotification. Device=");
    try
    {
        MAC devMac(p_rNotification.GetAddr().bytes);
        LOG_INFO_APP("[Report-784]: has been received for mac=" << devMac);
        if (SaveRespErr::IsErr_784(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_ERROR_APP("[RespReport-784]: received with error for mac" << devMac);
            return;
        }

        DevicePtr dev = p_rNotification.pDevices->FindDevice(devMac);
        if (!dev)
        {
            LOG_WARN_APP("[RespReport-784]: Device not in DB, with mac= " << devMac);
            return;
        }
        C784_ReadLinkList_Resp *pResp = (C784_ReadLinkList_Resp*) m_pResponse->whCmd.command;
        if (!pResp)
        {
            LOG_ERROR_APP("[RespReport-784]: Incorrect response received for CMD784. DeviceMAC=" << devMac);
            return;
        }

        if (pResp->m_ucNoOfLinksRead <= 0)
        {
            LOG_WARN_APP("[RespReport-784]: the number of links=" << (int) pResp->m_ucNoOfLinksRead);
            return;
        }

        //save DeviceScheduleLinks
        const uint16_t broadcastNN = 0xFFFF;
        int devID = -1;
        std::list<DeviceScheduleLink> linkList;
        for (int i = 0; i < pResp->m_ucNoOfLinksRead; i++)
        {
            if (broadcastNN != pResp->m_aLinks[i].nicknameOfNeighborForThisLink)
            {
                DevicePtr devTo = p_rNotification.pDevices->FindDevice(NickName(
                    (WHartShortAddress) pResp->m_aLinks[i].nicknameOfNeighborForThisLink));
                if (!devTo)
                {
                    LOG_WARN_APP("[RespReport-784]: unknown nickname=" << (int) pResp->m_aLinks[i].nicknameOfNeighborForThisLink);
                    continue;
                }
                devID = devTo->id;
            }
            DeviceScheduleLink link(pResp->m_aLinks[i].superframeId, devID, pResp->m_aLinks[i].slotNoForThisLink,
                pResp->m_aLinks[i].channelOffsetForThisLink, ((pResp->m_aLinks[i].linkOptions & LinkOptionFlagCodesMask_Transmit) != 0),
                ((pResp->m_aLinks[i].linkOptions & LinkOptionFlagCodesMask_Receive) != 0), ((pResp->m_aLinks[i].linkOptions
                            & LinkOptionFlagCodesMask_Shared) != 0), pResp->m_aLinks[i].linkType);
            linkList.push_back(link);
        }
        p_rNotification.pDevices->ChangeDevicesScheduleLinks(dev->id, linkList);
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Process784RespReport]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Process784RespReport]: System error!");
    }
}

void ResponseProcessor::Process785RespReport(AppNoBurstRspNotification& p_rNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Process785RespReport]:AppNoBurstRspNotification. Device=");
    try
    {
        MAC devMac(p_rNotification.GetAddr().bytes);
        LOG_INFO_APP("[RespReport-785]: has been received for mac=" << devMac);
        if (SaveRespErr::IsErr_785(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_ERROR_APP("[RespReport-785]: received with error for mac=" << devMac);
            return;
        }
        DevicePtr dev = p_rNotification.pDevices->FindDevice(devMac);
        if (!dev)
        {
            LOG_ERROR_APP("[RespReport-785]: Device not in DB, with mac=" << devMac);
            return;
        }
        C785_ReadGraphList_Resp *pwhResp = (C785_ReadGraphList_Resp*) m_pResponse->whCmd.command;
        if (!pwhResp)
        {
            LOG_ERROR_APP("[RespReport-785]: Incorrect response received for CMD785. DeviceMAC=" << devMac);
            return;
        }

        if (pwhResp->m_ucNoOfNeighbors <= 0)
        {
            LOG_WARN_APP("[RespReport-785]: the number of neighbors=" << (int) pwhResp->m_ucNoOfNeighbors);
            return;
        }

        DevicePtr toDevice;
        int graphID = 0;
        bool isAccesPoint = false;
        // cache info for cmd 785 m_pResponse
        std::list<GraphNeighbor> graphList;
        for (int i = 0; i < pwhResp->m_ucNoOfNeighbors; i++)
        {
            NickName nick(pwhResp->m_aNicknameOfNeighbor[i]);
            if (nick.Address() == stack::Gateway_Nickname())
                isAccesPoint = true;

            toDevice = p_rNotification.pDevices->FindDevice(nick);
            graphID = pwhResp->m_unGraphId;

            if (!toDevice)
            {
                LOG_WARN_APP("[RespReport-785]: toDevice not found with nickname=" << nick);
                continue;
            }
            graphList.push_back(GraphNeighbor(toDevice->id, graphID, i));
        }
        p_rNotification.pDevices->ChangeDeviceGraphs(dev->id, graphList);
        if (isAccesPoint) //update in db
        {
            if (dev && dev->Type() != Device::dtAccessPoint)
            {
                dev->Type(Device::dtAccessPoint);
                p_rNotification.pDevices->UpdateDevice(dev, false, false);
            }
        }
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Process785RespReport]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Process785RespReport]: System error!");
    }
}

void ResponseProcessor::Process787RespReport(AppNoBurstRspNotification& p_rNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Process787RespReport]:AppNoBurstRspNotification. Device=");
    try
    {
        MAC devMac(p_rNotification.GetAddr().bytes);
        LOG_INFO_APP("[RespReport-787]: has been received for mac=" << devMac);
        if (SaveRespErr::IsErr_787(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_ERROR_APP("[RespReport-787]: received with error for mac" << devMac);
            return;
        }

        DevicePtr dev = p_rNotification.pDevices->FindDevice(devMac);
        if (!dev)
        {
            LOG_WARN_APP("[RespReport-787]: Device not in DB, with mac= " << devMac);
            return;
        }
        C787_ReportNeighborSignalLevels_Resp *pwhResp = (C787_ReportNeighborSignalLevels_Resp*) m_pResponse->whCmd.command;
        if (!pwhResp)
        {
            LOG_ERROR_APP("[RespReport-787]: Incorrect response received for CMD787. DeviceMAC=" << devMac);
            return;
        }

        if (pwhResp->m_ucNoOfNeighborEntriesRead <= 0)
        {
            LOG_WARN_APP("[RespReport-787]: the number of neighbors=" << (int) pwhResp->m_ucNoOfNeighborEntriesRead);
            return;
        }

        LOG_DEBUG_APP("[RespReport-787]: neighb_list received has elements no. = " << (int) pwhResp->m_ucNoOfNeighborEntriesRead
                    << " from indexNo = " << pwhResp->m_ucNeighborTableIndex);
        LOG_DEBUG_APP("[RespReport-787]: neighb_list -> begin");
        NeighbourSignalLevels m_neighbList(dev->id, dev->Mac());
        for (int i = 0; i < pwhResp->m_ucNoOfNeighborEntriesRead; i++)
        {
            DevicePtr device = p_rNotification.pDevices->FindDevice(NickName(pwhResp->m_aNeighbors[i].nickname));
            if (!device)
            {
                LOG_WARN_APP("[RespReport-787]: not found in db the neighbor=" << (NickName(pwhResp->m_aNeighbors[i].nickname)));
                continue;
            }
            m_neighbList.neighbors.push_back(NeighbourSignalLevel(device->id, pwhResp->m_aNeighbors[i].RSLindB));
            LOG_DEBUG_APP("[RespReport-787]: device[" << i << "] has id=" << device->id);
        }
        LOG_DEBUG_APP("[RespReport-787]: neighb_list -> end");

        p_rNotification.pDevices->ChangeDevNeighbSignalLevels(dev->id, m_neighbList.neighbors);
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Process787RespReport]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Process787RespReport]: System error!");
    }
}

void ResponseProcessor::Process788RespReport(AppNoBurstRspNotification& p_rNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Process788RespReport]:AppNoBurstRspNotification. Device=");
    try
    {
        MAC devMac(p_rNotification.GetAddr().bytes);
        LOG_INFO_APP("[Alarm-788(RespReport)]: has been received for mac=" << devMac);
        if (SaveRespErr::IsErr_788(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_ERROR_APP("[Alarm-788(RespReport)]: received with error for mac=" << devMac);
            return;
        }

        DevicePtr dev = p_rNotification.pDevices->FindDevice(devMac);

        if (!dev)
        {
            LOG_WARN_APP("[Alarm-788(RespReport)]: Device not in DB, with mac= " << devMac);
            return;
        }

        int deviceId = dev->id;
        C788_AlarmPathDown_Resp *pwResp = (C788_AlarmPathDown_Resp*) m_pResponse->whCmd.command;
        if (!pwResp)
        {
            LOG_ERROR_APP("[Alarm-788(RespReport)]: Incorrect response received for CMD788. DeviceMAC=" << devMac);
            return;
        }

        //save in bd
        dev = p_rNotification.pDevices->FindDevice(NickName(pwResp->Nickname));
        if (!dev)
        {
            LOG_WARN_APP("[Alarm-788(RespReport)]: Device nickname not in db, nickname= " << NickName(pwResp->Nickname));
            return;
        }

        p_rNotification.pDevices->InsertAlarm(Alarm(deviceId, CMDID_C788_AlarmPathDown, dev->id));
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Process788RespReport]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Process788RespReport]: System error!");
    }
}

void ResponseProcessor::Process789RespReport(AppNoBurstRspNotification& p_rNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Process789RespReport]:AppNoBurstRspNotification. Device=");
    try
    {
        MAC devMac(p_rNotification.GetAddr().bytes);
        LOG_INFO_APP("[Alarm-789(RespReport)]: has been received for mac=" << devMac);
        if (SaveRespErr::IsErr_789(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_ERROR_APP("[Alarm-789(RespReport)]: received with error for mac" << devMac);
            return;
        }

        DevicePtr dev = p_rNotification.pDevices->FindDevice(devMac);

        if (!dev)
        {
            LOG_WARN_APP("[Alarm-789(RespReport)]: Device not in DB, with mac= " << devMac);
            return;
        }

        int deviceId = dev->id;
        C789_AlarmSourceRouteFailed_Resp *pwResp = (C789_AlarmSourceRouteFailed_Resp*) m_pResponse->whCmd.command;
        if (!pwResp)
        {
            LOG_ERROR_APP("[Alarm-789(RespReport)]: Incorrect response received for CMD789. DeviceMAC=" << devMac);
            return;
        }

        //save in bd
        dev = p_rNotification.pDevices->FindDevice(NickName(pwResp->m_unNicknameOfUnreachableNeighbor));
        if (!dev)
        {
            LOG_WARN_APP("[Alarm-789(RespReport)]: Device nickname not in db, nickname= " << NickName(
                pwResp->m_unNicknameOfUnreachableNeighbor));
            return;
        }
        p_rNotification.pDevices->InsertAlarm(Alarm(deviceId, CMDID_C789_AlarmSourceRouteFailed, dev->id,
            pwResp->m_ulNetworkLayerMICfromNPDUthatFailedRouting));
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Process789RespReport]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Process789RespReport]: System error!");
    }
}

void ResponseProcessor::Process790RespReport(AppNoBurstRspNotification& p_rNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Process790RespReport]:AppNoBurstRspNotification. Device=");
    try
    {
        MAC devMac(p_rNotification.GetAddr().bytes);
        LOG_INFO_APP("[Alarm-790(RespReport)]: has been received for mac=" << devMac);
        if (SaveRespErr::IsErr_790(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_ERROR_APP("[Response-790(RespReport)]: received with error for mac" << devMac);
            return;
        }

        DevicePtr dev = p_rNotification.pDevices->FindDevice(devMac);

        if (!dev)
        {
            LOG_WARN_APP("[Alarm-790(RespReport)]: Device not in DB, with mac= " << devMac);
            return;
        }

        C790_AlarmGraphRouteFailed_Resp *pwResp = (C790_AlarmGraphRouteFailed_Resp*) m_pResponse->whCmd.command;
        if (!pwResp)
        {
            LOG_ERROR_APP("[Alarm-790(RespReport)]: Incorrect response received for CMD790. DeviceMAC=" << devMac);
            return;
        }

        //save in bd
        p_rNotification.pDevices->InsertAlarm(Alarm(dev->id, CMDID_C790_AlarmGraphRouteFailed, pwResp->m_unGraphIdOfFailedRoute));
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Process790RespReport]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Process790RespReport]: System error!");
    }
}

void ResponseProcessor::Process791RespReport(AppNoBurstRspNotification& p_rNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Process791RespReport]:AppNoBurstRspNotification. Device=");
    try
    {
        MAC devMac(p_rNotification.GetAddr().bytes);
        LOG_INFO_APP("[Alarm-791(RespReport)]: has been received for mac=" << devMac);
        if (SaveRespErr::IsErr_791(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_ERROR_APP("[Alarm-791(RespReport)]: received with error for mac" << devMac);
            return;
        }

        DevicePtr dev = p_rNotification.pDevices->FindDevice(devMac);
        if (!dev)
        {
            LOG_WARN_APP("[Alarm-791(RespReport)]: Device not in DB, with mac= " << devMac);
            return;
        }

        int deviceId = dev->id;
        C791_AlarmTransportLayerFailed_Resp *pwResp = (C791_AlarmTransportLayerFailed_Resp*) m_pResponse->whCmd.command;
        if (!pwResp)
        {
            LOG_ERROR_APP("[Alarm-791(RespReport)]: Incorrect response received for CMD791. DeviceMAC=" << devMac);
            return;
        }

        //save in bd
        dev = p_rNotification.pDevices->FindDevice(NickName(pwResp->m_unNicknameOfUnreachablePeer));
        if (!dev)
        {
            LOG_WARN_APP("[Alarm-791(RespReport)]: Device nickname not in db, nickname= "
                        << NickName(pwResp->m_unNicknameOfUnreachablePeer));
            return;
        }
        p_rNotification.pDevices->InsertAlarm(Alarm(deviceId, CMDID_C791_AlarmTransportLayerFailed, dev->id));
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Process791RespReport]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Process791RespReport]: System error!");
    }
}

void ResponseProcessor::Process800RespReport(AppNoBurstRspNotification& p_rNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Process800RespReport]:AppNoBurstRspNotification. Device=");
    try
    {
        MAC devMac(p_rNotification.GetAddr().bytes);
        LOG_INFO_APP("[RespReport-800]: services received for mac=" << devMac);
        if (SaveRespErr::IsErr_800(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_DEBUG_APP("[RespReport-800]: services received with error for mac=" << devMac);
            return;
        }
        DevicePtr dev = p_rNotification.pDevices->FindDevice(devMac);
        if (!dev)
        {
            LOG_WARN_APP("[RespReport-800]: Device not in DB, with mac= " << devMac);
            return;
        }

        C800_ReadServiceList_Resp *pResp = (C800_ReadServiceList_Resp*) m_pResponse->whCmd.command;
        if (!pResp)
        {
            LOG_ERROR_APP("[RespReport-800]: Incorrect response received for CMD800. DeviceMAC=" << devMac);
            return;
        }

        if (pResp->m_ucNoOfEntriesRead <= 0)
        {
            LOG_WARN_APP("[RespReport-800]: the number of neighbors=" << (int) pResp->m_ucNoOfEntriesRead);
            return;
        }

        //save services
        ServicesListT serviceList;
        for (int i = 0; i < pResp->m_ucNoOfEntriesRead; i++)
        {
            NickName nick((WHartShortAddress) pResp->m_aServices[i].nicknameOfPeer);
            DevicePtr devTo = p_rNotification.pDevices->FindDevice(nick);
            if (!devTo)
            {
                LOG_WARN_APP("[RespReport-800]: unknown nickname=" << nick);
                continue;
            }
            Service s(pResp->m_aServices[i].serviceId, dev->id, devTo->id, (uint8_t)(pResp->m_aServices[i].serviceApplicationDomain),
                ((pResp->m_aServices[i].serviceRequestFlags & ServiceRequestFlagsMask_Source) != 0),
                ((pResp->m_aServices[i].serviceRequestFlags & ServiceRequestFlagsMask_Sink) != 0),
                ((pResp->m_aServices[i].serviceRequestFlags & ServiceRequestFlagsMask_Intermittent) != 0),
                pResp->m_aServices[i].period.u32, pResp->m_aServices[i].routeId);

            serviceList.push_back(s);
        }
        p_rNotification.pDevices->ChangeServices(serviceList);
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Process800RespReport]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Process800RespReport]: System error!");
    }
}

void ResponseProcessor::Process801RespReport(AppNoBurstRspNotification& p_rNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Process801RespReport]:AppNoBurstRspNotification. Device=");
    try
    {
        MAC devMac(p_rNotification.GetAddr().bytes);
        LOG_INFO_APP("[RespReport-801]: received for mac=" << devMac);
        if (SaveRespErr::IsErr_801(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_DEBUG_APP("[RespReport-801]: received with error for mac=" << devMac);
            return;
        }
        DevicePtr dev = p_rNotification.pDevices->FindDevice(devMac);
        if (!dev)
        {
            LOG_WARN_APP("[RespReport-801]: Device not in DB, with mac= " << devMac);
            return;
        }

        C801_DeleteService_Resp *pResp = (C801_DeleteService_Resp *) m_pResponse->whCmd.command;
        if (!pResp)
        {
            LOG_ERROR_APP("[RespReport-801]: Incorrect response received for CMD801. DeviceMAC=" << devMac);
            return;
        }

        p_rNotification.pDevices->DeleteService(dev->id, pResp->m_ucServiceId);
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Process801RespReport]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Process801RespReport]: System error!");
    }
}

void ResponseProcessor::Process802RespReport(AppNoBurstRspNotification& p_rNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Process802RespReport]:AppNoBurstRspNotification. Device=");
    try
    {
        MAC devMac(p_rNotification.GetAddr().bytes);

        LOG_INFO_APP("[RespReport-802]: received for mac=" << devMac);
        if (SaveRespErr::IsErr_802(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_DEBUG_APP("[RespReport-802]: received with error for mac=" << devMac);
            return;
        }
        DevicePtr dev = p_rNotification.pDevices->FindDevice(devMac);
        if (!dev)
        {
            LOG_WARN_APP("[RespReport-802]: Device not in DB, with mac= " << devMac);
            return;
        }

        C802_ReadRouteList_Resp *pResp = (C802_ReadRouteList_Resp*) m_pResponse->whCmd.command;
        if (!pResp)
        {
            LOG_ERROR_APP("[RespReport-802]: Incorrect response received for CMD802. DeviceMAC=" << devMac);
            return;
        }

        if (pResp->m_ucNoOfEntriesRead <= 0)
        {
            LOG_WARN_APP("[RespReport-802]: the number of neighbors=" << (int) pResp->m_ucNoOfEntriesRead);
            return;
        }

        //save routes in db and cache the route id
        RoutesListT routes;
        for (int i = 0; i < pResp->m_ucNoOfEntriesRead; i++)
        {
            DevicePtr devTo = p_rNotification.pDevices->FindDevice(NickName((WHartShortAddress) pResp->m_aRoutes[i].destinationNickname));
            if (!devTo)
            {
                LOG_WARN_APP("[RespReport-802]: unknown nickname=" << (int) pResp->m_aRoutes[i].destinationNickname);
                continue;
            }
            Route r(pResp->m_aRoutes[i].routeId, dev->id, devTo->id, pResp->m_aRoutes[i].graphId, pResp->m_aRoutes[i].sourceRouteAttached);

            routes.push_back(r);
        }
        p_rNotification.pDevices->ChangeRoutes(routes);
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Process802RespReport]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Process802RespReport]: System error!");
    }
}

void ResponseProcessor::Process803RespReport(AppNoBurstRspNotification& p_rNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Process803RespReport]:AppNoBurstRspNotification. Device=");
    try
    {
        MAC devMac(p_rNotification.GetAddr().bytes);
        LOG_INFO_APP("[Report-803]: received for mac=" << devMac);
        if (SaveRespErr::IsErr_803(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_DEBUG_APP("[RespReport-803]: received with error for mac=" << devMac);
            return;
        }
        DevicePtr dev = p_rNotification.pDevices->FindDevice(devMac);
        if (!dev)
        {
            LOG_WARN_APP("[RespReport-803]: Device not in DB, with mac= " << devMac);
            return;
        }

        C803_ReadSourceRoute_Resp *pResp = (C803_ReadSourceRoute_Resp*) m_pResponse->whCmd.command;
        if (!pResp)
        {
            LOG_ERROR_APP("[RespReport-803]: Incorrect response received for CMD803. DeviceMAC=" << devMac);
            return;
        }
        if (pResp->m_ucNoOfHops <= 0)
        {
            LOG_WARN_APP("[RespReport-803]: the number of hops=" << (int) pResp->m_ucNoOfHops);
            return;
        }

        SourceRoutesListT sourceRoutes;
        /*compose devices string*/
        char* devicesList = new char[pResp->m_ucNoOfHops * 4/*nb of hexchars per hop*/+ 1/*string terminator*/];
        memset(devicesList, 0, pResp->m_ucNoOfHops * 4/*nb of hexchars per hop*/+ 1/*string terminator*/);

        for (int i = 0; i < pResp->m_ucNoOfHops; i++)
        {
            sprintf(devicesList + i * 4, "%04X", pResp->m_aHopNicknames[i]);
        }

        SourceRoute sr(dev->id, pResp->m_ucRouteId, devicesList);

        sourceRoutes.push_back(sr);

        delete[] devicesList;
        p_rNotification.pDevices->ChangeSourceRoutes(sourceRoutes);
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Process803RespReport]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Process803RespReport]: System error!");
    }
}

void ResponseProcessor::Process814RespReport(AppNoBurstRspNotification& p_rNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Process814RespReport]:AppNoBurstRspNotification. Device=");
    //errors
    try
    {
        MAC deviceMac(p_rNotification.GetAddr().bytes);
        LOG_INFO_APP("[RespReport-814]: received for mac=" << deviceMac);
        if (SaveRespErr::IsErr_814(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_ERROR_APP("[RespReport-814]: received with hostErrorCode=" << m_pResponse->hostErr << " and respErrCode="
                        << (int) (m_pResponse->whCmd.responseCode) << " for mac=" << deviceMac);
            return;
        }

        C814_ReadDeviceListEntries_Resp *pwhResp = (C814_ReadDeviceListEntries_Resp*) m_pResponse->whCmd.command;
        if (!pwhResp)
        {
            LOG_ERROR_APP("[RespReport-814]: Incorrect response received for CMD814. DeviceMAC=" << deviceMac);
            return;
        }

        std::list<MAC> macs;
        LOG_DEBUG_APP("[RespReport-814]: device_list received has elements no. = " << (int) pwhResp->m_ucNoOfListEntriesRead
                    << " from indexNo = " << pwhResp->m_unStartingListIndex);
        LOG_DEBUG_APP("[RespReport-814]: device_list -> begin");
        for (int i = 0; i < pwhResp->m_ucNoOfListEntriesRead; i++)
        {
            MAC devMac(pwhResp->m_aDeviceUniqueIds[i]);

            bool isNM = memcmp(pwhResp->m_aDeviceUniqueIds[i], hart7::stack::NetworkManager_UniqueID().bytes, sizeof(WHartUniqueID)) == 0;
            bool isGW = memcmp(pwhResp->m_aDeviceUniqueIds[i], hart7::stack::Gateway_UniqueID().bytes, sizeof(WHartUniqueID)) == 0;

            if (isNM || isGW)
            {
                DevicePtr dev = p_rNotification.pDevices->FindDevice(devMac);
                if (!dev || !(dev->IsRegistered()))
                {
                    C832_ReadNetworkDeviceIdentity_Req req;
                    memcpy(req.DeviceUniqueID, devMac.Address().bytes, sizeof(WHartUniqueID));
                    LOG_INFO_APP("[RespReport-814]: send 832 req for mac=" << devMac);

                    stack::WHartCommand whCmd;
                    whCmd.commandID = CMDID_C832_ReadNetworkDeviceIdentity;
                    whCmd.command = &req;

                    gateway::GatewayIO::AppData appData;
                    appData.appCmd = AbstractAppCommandPtr(new App832Cmd(Device::dsRegistered, devMac));
                    appData.appCmd->pDevices = p_rNotification.pDevices;
                    appData.appCmd->pCommands = p_rNotification.pCommands;
                    //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C832_ReadNetworkDeviceIdentity. Device=NM");
                    m_pProcessor->gateway.SendWHRequest(appData, stack::NetworkManager_UniqueID(), whCmd);
                }
            }

            macs.push_back(devMac);
            LOG_DEBUG_APP("[RespReport-814]: device[" << i << "] has mac=" << MAC(pwhResp->m_aDeviceUniqueIds[i]));
        }
        LOG_DEBUG_APP("[RespReport-814]: device_list -> end");
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Process814RespReport]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Process814RespReport]: System error!");
    }
}

void ResponseProcessor::Process818RespReport(AppNoBurstRspNotification& p_rNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Process818RespReport]:AppNoBurstRspNotification. Device=");
    try
    {
        LOG_INFO_APP("[RespReport-818]: Response for report 818.");
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Process818RespReport]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Process818RespReport]: System error!");
    }
}

void ResponseProcessor::Process832RespReport(AppNoBurstRspNotification& p_rNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Process832RespReport]:AppNoBurstRspNotification. Device=");
    try
    {
        MAC deviceMac(p_rNotification.GetAddr().bytes);
        LOG_INFO_APP("[RespReport-832]: received for mac=" << deviceMac);
        if (SaveRespErr::IsErr_832(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_ERROR_APP("[RespReport-832]: nickname has failed! for mac=" << deviceMac);
            return;
        }

        C832_ReadNetworkDeviceIdentity_Resp *pwhResp = (C832_ReadNetworkDeviceIdentity_Resp*) m_pResponse->whCmd.command;
        if (!pwhResp)
        {
            LOG_ERROR_APP("[RespReport-832]: Incorrect response received for CMD832. DeviceMAC=" << deviceMac);
            return;
        }

        MAC devMac = MAC(pwhResp->DeviceUniqueID);
        std::string longTag;
        for (unsigned int i = 0; i < sizeof(pwhResp->LongTag); ++i)
            longTag += ConvertToHex((unsigned char) pwhResp->LongTag[i]);

        DevicePtr dev = p_rNotification.pDevices->FindDevice(devMac);
        if (dev)
        {//device-ul exista in bd
            //update bd
            bool nicknameChanged = false;
            if (dev->Nickname() != NickName(pwhResp->Nickname))
            {
                dev->Nickname(pwhResp->Nickname);
                nicknameChanged = true;
            }
            dev->SetTAG(longTag);
            p_rNotification.pDevices->UpdateDevice(dev, nicknameChanged, false);
            LOG_DEBUG_APP("[RespReport-832]: device with mac= " << devMac << " updated in db");
        }
        else
        {
            LOG_WARN_APP("[RespReport-832]: device with mac= " << devMac << " not in db, so it is added in db");
            //insert bd
            dev = DevicePtr(new Device());
            dev->Mac(devMac);
            dev->SetTAG(longTag);
            dev->Nickname(NickName(pwhResp->Nickname));
            dev->Status(Device::dsNetworkPacketsHeard);
            if (dev->Type() == Device::dtUnknown)
            {
                dev->Type(Device::dtRoutingDeviceNode);
            }
            if (MAC(stack::Gateway_UniqueID().bytes) == devMac)
            {
                dev->Type(Device::dtGateway);
            }
            if (devMac == MAC(stack::NetworkManager_UniqueID().bytes))
            {
                dev->Type(Device::dtSystemManager);
            }
            p_rNotification.pDevices->AddNewDevice(dev);
        }

        //also test for notification bit for device_configuration
        unsigned short notifMask = dev->GetNotificationBitMask();
        if (notifMask & NotificationMaskCodesMask_DeviceConfiguration)
            return;

        LOG_INFO_APP("[RespReport-832]: device=" << dev->Mac() << " has no DevConfigBitMask; set to send 837 req");

        notifMask = notifMask | NotificationMaskCodesMask_DeviceConfiguration;
        dev->SetNotificationBitMask(notifMask);

        stack::WHartCommand whCmd;
        C837_WriteUpdateNotificationBitMask_Req whReq;
        whReq.ChangeNotificationFlags = notifMask;
        memcpy(whReq.UniqueID, dev->Mac().Address().bytes, sizeof(WHartUniqueID));

        whCmd.commandID = CMDID_C837_WriteUpdateNotificationBitMask;
        whCmd.command = &whReq;

        m_pResponse->appData.appCmd = AbstractAppCommandPtr(new AppSetNotificationBitMask(devMac, notifMask));

        //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C837_WriteUpdateNotificationBitMask. Device=GW");
        m_pProcessor->gateway.SendWHRequest(m_pResponse->appData, stack::Gateway_UniqueID(), whCmd, false, true);
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Process832RespReport]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Process832RespReport]: System error!");
    }
}

void ResponseProcessor::Process833RespReport(AppNoBurstRspNotification& p_rNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Process833RespReport]:AppNoBurstRspNotification. Device=");
    try
    {
        MAC deviceMac(p_rNotification.GetAddr().bytes);

        LOG_INFO_APP("[RespReport-833]: received for mac=" << deviceMac);
        if (SaveRespErr::IsErr_833(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_ERROR_APP("[RespReport-833]: error received for mac=" << deviceMac);
            return;
        }

        DevicePtr device = p_rNotification.pDevices->FindDevice(deviceMac);
        if (!device)
        {
            LOG_WARN_APP("[RespReport-833]: Device not in DB, with mac= " << deviceMac);
            return;
        }

        C833_ReadNetworkDeviceNeighbourHealth_Resp *pwhResp = (C833_ReadNetworkDeviceNeighbourHealth_Resp*) m_pResponse->whCmd.command;
        if (!pwhResp)
        {
            LOG_ERROR_APP("[RespReport-833]: Incorrect response received for CMD833. DeviceMAC=" << deviceMac);
            return;
        }

        if (pwhResp->NeighbourCount <= 0)
        {
            LOG_WARN_APP("[RespReport-833]: the number of neighbors=" << (int) pwhResp->NeighbourCount);
            return;
        }

        DevicePtr toDevice;
        std::list<NeighbourSignalLevel> devNeighbSignalLevels;
        DeviceNeighborsHealth neighborsList(device->id, deviceMac);
        for (int i = 0; i < pwhResp->NeighbourCount; i++)
        {
            toDevice = p_rNotification.pDevices->FindDevice(NickName(pwhResp->Neighbours[i].NeighbourNickname));
            if (toDevice == NULL)
            {
                LOG_WARN_APP("[RespReport-833]: nickname received wasn't found in db: " << NickName(
                    pwhResp->Neighbours[i].NeighbourNickname));
                continue;
            }
            if (pwhResp->Neighbours[i].TransmittedPacketCount == 0 && pwhResp->Neighbours[i].TransmittedPacketWithNoACKCount == 0)
            {
                devNeighbSignalLevels.push_back(NeighbourSignalLevel(toDevice->id, pwhResp->Neighbours[i].NeighbourRSL));
                continue;
            }

            //devNeighborsHealth
            neighborsList.m_oNeighborsList.push_back(NeighborHealth(toDevice->id, hart7::hostapp::NeighborHealth::g_nNoClockSource,
                pwhResp->Neighbours[i].NeighbourRSL, pwhResp->Neighbours[i].TransmittedPacketCount,
                pwhResp->Neighbours[i].TransmittedPacketWithNoACKCount, pwhResp->Neighbours[i].ReceivedPacketCount));

        }
        MAC devMac(pwhResp->UniqueID);
        DevicePtr dev = p_rNotification.pDevices->FindDevice(devMac);
        p_rNotification.pDevices->ChangeDevNeighbSignalLevels(dev->id, devNeighbSignalLevels);
        p_rNotification.pDevices->ChangeDeviceNeighborsHealth(neighborsList);
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Process833RespReport]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Process833RespReport]: System error!");
    }
}

void ResponseProcessor::Process834RespReport(AppNoBurstRspNotification& p_rNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Process834RespReport]:AppNoBurstRspNotification. Device=");
    try
    {
        MAC deviceMac(p_rNotification.GetAddr().bytes);
        LOG_INFO_APP("[RespReport-834]: received for mac=" << deviceMac);
        if (SaveRespErr::IsErr_834(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_ERROR_APP("[RespReport-834]: received with error for mac=" << deviceMac);
            return;
        }

        C834_ReadNetworkTopologyInformation_Resp *pwhResp = (C834_ReadNetworkTopologyInformation_Resp*) m_pResponse->whCmd.command;
        if (!pwhResp)
        {
            LOG_ERROR_APP("[RespReport-834]: Incorrect response received for CMD834. DeviceMAC=" << deviceMac);
            return;
        }
        if (pwhResp->NeighboursNo <= 0)
        {
            LOG_WARN_APP("[RespReport-834]: the number of neighbors=" << (int) pwhResp->NeighboursNo);
            return;
        }

        DevicePtr toDevice;
        int graphID = 0;
        std::list<GraphNeighbor> graphList;
        for (int i = 0; i < pwhResp->NeighboursNo; i++)
        {
            toDevice = p_rNotification.pDevices->FindDevice(NickName(pwhResp->Neighbour[i]));
            graphID = pwhResp->IndexGraphId;
            if (!toDevice)
            {
                LOG_WARN_APP("[RespReport-834]: toDevice not found with nickname=" << NickName(pwhResp->Neighbour[i]));
                continue;
            }

            graphList.push_back(GraphNeighbor(toDevice->id, graphID, i));
        }
        MAC devMac(pwhResp->DeviceLongAddress);
        DevicePtr dev = p_rNotification.pDevices->FindDevice(devMac);
        if (!dev)
        {
            LOG_ERROR_APP("[RespReport-834]: device not found in bd with mac=" << devMac);
            return;
        }
        p_rNotification.pDevices->ChangeDeviceGraphs(dev->id, graphList);
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Process834RespReport]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Process834RespReport]: System error!");
    }
}

void ResponseProcessor::Process965RespReport(AppNoBurstRspNotification& p_rNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Process965RespReport]:AppNoBurstRspNotification. Device=");
    try
    {
        MAC devMac(p_rNotification.GetAddr().bytes);
        LOG_INFO_APP("[RespReport-965]: has been received for mac=" << devMac);
        if (SaveRespErr::IsErr_965(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_ERROR_APP("[RespReport-965]: received with error for mac=" << devMac);
            return;
        }

        DevicePtr dev = p_rNotification.pDevices->FindDevice(devMac);
        if (!dev)
        {
            LOG_WARN_APP("[RespReport-965]: Source device not in DB, with mac= " << devMac);
            return;
        }
        int sourceDevID = dev->id;

        C965_WriteSuperframe_Resp *pwResp = (C965_WriteSuperframe_Resp*) m_pResponse->whCmd.command;
        if (!pwResp)
        {
            LOG_ERROR_APP("[RespReport-965]: Incorrect response received for CMD965. DeviceMAC=" << devMac);
            return;
        }

        SuperframesListT Superframes;
        Superframe sFrame(pwResp->m_ucSuperframeID, sourceDevID, pwResp->m_unSuperframeSlotsNo, ((pwResp->m_ucSuperframeMode
                    & SuperframeModeFlagsMask_Active) != 0), ((pwResp->m_ucSuperframeMode & SuperframeModeFlagsMask_HandheldSuperframe)
                    != 0));
        Superframes.push_back(sFrame);
        p_rNotification.pDevices->ChangeSuperframes(Superframes);
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Process965RespReport]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Process965RespReport]: System error!");
    }
}

void ResponseProcessor::Process966RespReport(AppNoBurstRspNotification& p_rNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Process966RespReport]:AppNoBurstRspNotification. Device=");
    try
    {
        MAC devMac(p_rNotification.GetAddr().bytes);
        LOG_INFO_APP("[RespReport-966]: has been received for mac=" << devMac);
        if (SaveRespErr::IsErr_966(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_ERROR_APP("[RespReport-966]: received with error for mac=" << devMac);
            return;
        }

        DevicePtr dev = p_rNotification.pDevices->FindDevice(devMac);
        if (!dev)
        {
            LOG_WARN_APP("[RespReport-966]: Source device not in DB, with mac= " << devMac);
            return;
        }
        int sourceDevID = dev->id;

        C966_DeleteSuperframe_Resp *pwResp = (C966_DeleteSuperframe_Resp*) m_pResponse->whCmd.command;
        if (!pwResp)
        {
            LOG_ERROR_APP("[RespReport-966]: Incorrect response received for CMD966. DeviceMAC=" << devMac);
            return;
        }
        p_rNotification.pDevices->DeleteSuperframe(sourceDevID, pwResp->m_ucSuperframeID);
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Process966RespReport]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Process966RespReport]: System error!");
    }
}

void ResponseProcessor::Process967RespReport(AppNoBurstRspNotification& p_rNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Process967RespReport]:AppNoBurstRspNotification. Device=");
    try
    {
        MAC devMac(p_rNotification.GetAddr().bytes);
        LOG_INFO_APP("[RespReport-967]: has been received for mac=" << devMac);
        if (SaveRespErr::IsErr_967(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_ERROR_APP("[RespReport-967]: received with error for mac=" << devMac);
            return;
        }

        DevicePtr dev = p_rNotification.pDevices->FindDevice(devMac);
        if (!dev)
        {
            LOG_WARN_APP("[RespReport-967]: Source device not in DB, with mac= " << devMac);
            return;
        }
        int sourceDevID = dev->id;

        C967_WriteLink_Resp *pwResp = (C967_WriteLink_Resp*) m_pResponse->whCmd.command;
        if (!pwResp)
        {
            LOG_ERROR_APP("[RespReport-967]: Incorrect response received for CMD967. DeviceMAC=" << devMac);
            return;
        }

        //save in bd
        //save DeviceScheduleLinks
        const uint16_t broadcastNN = 0xFFFF;
        int devID = -1;
        if (broadcastNN != pwResp->m_unNeighborNickname)
        {
            dev = p_rNotification.pDevices->FindDevice(NickName(pwResp->m_unNeighborNickname));
            if (!dev)
            {
                LOG_WARN_APP("[RespReport-967]: Device nickname not in db, nickname= " << NickName(pwResp->m_unNeighborNickname));
                return;
            }
            devID = dev->id;
        }

        std::list<DeviceScheduleLink> linkList;
        DeviceScheduleLink link(pwResp->m_ucSuperframeID, devID, pwResp->m_unSlotNumber, pwResp->m_ucChannelOffset,
            ((pwResp->m_ucLinkOptions & LinkOptionFlagCodesMask_Transmit) != 0), ((pwResp->m_ucLinkOptions
                        & LinkOptionFlagCodesMask_Receive) != 0), ((pwResp->m_ucLinkOptions & LinkOptionFlagCodesMask_Shared) != 0),
            pwResp->m_eLinkType);
        linkList.push_back(link);
        p_rNotification.pDevices->ChangeDevicesScheduleLinks(sourceDevID, linkList);
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Process967RespReport]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Process967RespReport]: System error!");
    }
}

void ResponseProcessor::Process968RespReport(AppNoBurstRspNotification& p_rNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Process968RespReport]:AppNoBurstRspNotification. Device=");
    try
    {
        MAC devMac(p_rNotification.GetAddr().bytes);
        LOG_INFO_APP("[RespReport-968]: has been received for mac=" << devMac);
        if (SaveRespErr::IsErr_968(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_ERROR_APP("[RespReport-968]: received with error for mac=" << devMac);
            return;
        }

        DevicePtr dev = p_rNotification.pDevices->FindDevice(devMac);
        if (!dev)
        {
            LOG_WARN_APP("[RespReport-968]: Source device not in DB, with mac= " << devMac);
            return;
        }
        int sourceDevID = dev->id;

        C968_DeleteLink_Resp *pwResp = (C968_DeleteLink_Resp*) m_pResponse->whCmd.command;
        if (!pwResp)
        {
            LOG_ERROR_APP("[RespReport-968]: Incorrect response received for CMD968. DeviceMAC=" << devMac);
            return;
        }

        //delete in bd
        const uint16_t broadcastNN = 0xFFFF;
        int devID = -1;
        if (broadcastNN != pwResp->m_unNeighborNickname)
        {
            dev = p_rNotification.pDevices->FindDevice(NickName(pwResp->m_unNeighborNickname));
            if (!dev)
            {
                LOG_WARN_APP("[RespReport-968]: Device nickname not in db, nickname= " << NickName(pwResp->m_unNeighborNickname));
                return;
            }
            devID = dev->id;
        }

        p_rNotification.pDevices->DeleteLink(sourceDevID, pwResp->m_ucSuperframeID, devID, pwResp->m_unSlotNumber);
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Process968RespReport]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Process968RespReport]: System error!");
    }
}

void ResponseProcessor::Process969RespReport(AppNoBurstRspNotification& p_rNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Process969RespReport]:AppNoBurstRspNotification. Device=");
    try {
    MAC devMac(p_rNotification.GetAddr().bytes);
    LOG_INFO_APP("[RespReport-969]: has been received for mac=" << devMac);
    if (SaveRespErr::IsErr_969(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
    {
        LOG_ERROR_APP("[RespReport-969]: received with error for mac=" << devMac);
        return;
    }

    DevicePtr dev = p_rNotification.pDevices->FindDevice(devMac);
    if(!dev)
    {
        LOG_WARN_APP("[RespReport-969]: Source device not in DB, with mac= " << devMac);
        return;
    }
    int sourceDevID = dev->id;

    C969_WriteGraphNeighbourPair_Resp *pwResp = (C969_WriteGraphNeighbourPair_Resp*)m_pResponse->whCmd.command;
    if (!pwResp)
    {
        LOG_ERROR_APP("[RespReport-969]: Incorrect response received for CMD969. DeviceMAC=" << devMac);
        return;
    }

    //save in bd
    dev = p_rNotification.pDevices->FindDevice(NickName(pwResp->m_unNeighborNickname));
    if(!dev)
    {
        LOG_WARN_APP("[RespReport-969]: Device nickname not in db, nickname= "<<NickName(pwResp->m_unNeighborNickname));
        return;
    }

    std::list<GraphNeighbor> graphList;
    graphList.push_back(GraphNeighbor(dev->id, pwResp->m_unGraphID,-1));
    p_rNotification.pDevices->ChangeDeviceGraphs(sourceDevID, graphList);
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Process969RespReport]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Process969RespReport]: System error!");
    }
}

void ResponseProcessor::Process970RespReport(AppNoBurstRspNotification& p_rNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Process970RespReport]:AppNoBurstRspNotification. Device=");
    try
    {
        MAC devMac(p_rNotification.GetAddr().bytes);
        LOG_INFO_APP("[RespReport-970]: has been received for mac=" << devMac);
        if (SaveRespErr::IsErr_970(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_ERROR_APP("[RespReport-970]: received with error for mac=" << devMac);
            return;
        }

        DevicePtr dev = p_rNotification.pDevices->FindDevice(devMac);
        if (!dev)
        {
            LOG_WARN_APP("[RespReport-970]: Source device not in DB, with mac= " << devMac);
            return;
        }
        int sourceDevID = dev->id;

        C970_DeleteGraphConnection_Resp *pwResp = (C970_DeleteGraphConnection_Resp*) m_pResponse->whCmd.command;
        if (!pwResp)
        {
            LOG_ERROR_APP("[RespReport-970]: Incorrect response received for CMD970. DeviceMAC=" << devMac);
            return;
        }

        //save in bd
        dev = p_rNotification.pDevices->FindDevice(NickName(pwResp->m_unNeighborNickname));
        if (!dev)
        {
            LOG_WARN_APP("[RespReport-970]: Device nickname not in db, nickname= " << NickName(pwResp->m_unNeighborNickname));
            return;
        }

        p_rNotification.pDevices->DeleteGraph(sourceDevID, dev->id, pwResp->m_unGraphID);
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Process970RespReport]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Process970RespReport]: System error!");
    }
}

// TODO
void ResponseProcessor::Process971RespReport(AppNoBurstRspNotification& p_rNotification)
{
}

void ResponseProcessor::Process973RespReport(AppNoBurstRspNotification& p_rNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Process973RespReport]:AppNoBurstRspNotification. Device=");
    try
    {
        MAC devMac(p_rNotification.GetAddr().bytes);

        LOG_INFO_APP("[RespReport-973]: received for mac=" << devMac);
        if (SaveRespErr::IsErr_973(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_ERROR_APP("[RespReport-973]: received with error for mac=" << devMac);
            return;
        }
        DevicePtr dev = p_rNotification.pDevices->FindDevice(devMac);
        if (!dev)
        {
            LOG_WARN_APP("[RespReport-973]: Device not in DB, with mac= " << devMac);
            return;
        }

        C973_WriteService_Resp *pwhResponse = (C973_WriteService_Resp *) m_pResponse->whCmd.command;
        if (!pwhResponse)
        {
            LOG_ERROR_APP("[RespReport-973]: Incorrect response received for CMD973. DeviceMAC=" << devMac);
            return;
        }

        NickName nick((WHartShortAddress) pwhResponse->m_unPeerNickname);
        DevicePtr devTo = p_rNotification.pDevices->FindDevice(nick);
        if (!devTo)
        {
            LOG_WARN_APP("[RespReport-973]: unknown nickname=" << nick);
            return;
        }
        Service s(pwhResponse->m_ucServiceID, dev->id, devTo->id, (uint8_t)(pwhResponse->m_eApplicationDomain),
            ((pwhResponse->m_ucRequestFlags & ServiceRequestFlagsMask_Source) != 0), ((pwhResponse->m_ucRequestFlags
                        & ServiceRequestFlagsMask_Sink) != 0),
            ((pwhResponse->m_ucRequestFlags & ServiceRequestFlagsMask_Intermittent) != 0), pwhResponse->m_tPeriod.u32,
            pwhResponse->m_ucRouteID);
        ServicesListT serviceList;
        serviceList.push_back(s);

        //ServiceId     Addr  PeerAdd  Handler   Src  Sink   Int AppDomain  RouteId Period      OldPer rqPnd  Mngt     Status
        LOG_DEBUG_APP("[RespReport-973]: Service[" << dev->Mac().ToString() << "] = {"
            "ServiceId:" << (int) pwhResponse->m_ucServiceID << ", "
            "Addr:" << dev->Nickname().ToString() << ", "
            "PeerAdd:" << devTo->Nickname().ToString() << ", "
            "Src:" << (((pwhResponse->m_ucRequestFlags & ServiceRequestFlagsMask_Source) != 0) ? "TRUE" : "FALSE") << ", "
            "Sink:" << (((pwhResponse->m_ucRequestFlags & ServiceRequestFlagsMask_Sink) != 0) ? "TRUE" : "FALSE") << ", "
            "Int:" << (((pwhResponse->m_ucRequestFlags & ServiceRequestFlagsMask_Intermittent) != 0) ? "TRUE" : "FALSE") << ", "
            "AppDomain:" << ConvertToHex((unsigned char) pwhResponse->m_eApplicationDomain) << ", "
            "RouteId:" << ConvertToHex((unsigned char) pwhResponse->m_ucRouteID) << ", "
            "Period:" << (int) (pwhResponse->m_tPeriod.u32 / 32000) << "}");

        p_rNotification.pDevices->ChangeServices(serviceList);
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Process973RespReport]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Process973RespReport]: System error!");
    }
}

void ResponseProcessor::Process974RespReport(AppNoBurstRspNotification& p_rNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Process974RespReport]:AppNoBurstRspNotification. Device=");
    try
    {
        MAC devMac(p_rNotification.GetAddr().bytes);

        LOG_INFO_APP("[RespReport-974]: received for mac=" << devMac);
        if (SaveRespErr::IsErr_974(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_ERROR_APP("[RespReport-974]: received with error for mac=" << devMac);
            return;
        }
        DevicePtr dev = p_rNotification.pDevices->FindDevice(devMac);
        if (!dev)
        {
            LOG_WARN_APP("[RespReport-974]: Device not in DB, with mac= " << devMac);
            return;
        }

        C974_WriteRoute_Resp *pwhResponse = (C974_WriteRoute_Resp *) m_pResponse->whCmd.command;
        if (!pwhResponse)
        {
            LOG_ERROR_APP("[RespReport-974]: Incorrect response received for CMD974. DeviceMAC=" << devMac);
            return;
        }

        NickName nick((WHartShortAddress) pwhResponse->m_unPeerNickname);
        DevicePtr devTo = p_rNotification.pDevices->FindDevice(nick);
        if (!devTo)
        {
            LOG_WARN_APP("[RespReport-974]: unknown nickname=" << nick);
            return;
        }
        RoutesListT routes;
        Route r(pwhResponse->m_ucRouteID, dev->id, devTo->id, pwhResponse->m_unGraphID, 0);
        routes.push_back(r);

        p_rNotification.pDevices->ChangeRoutes(routes);
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Process974RespReport]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Process974RespReport]: System error!");
    }
}

void ResponseProcessor::Process975RespReport(AppNoBurstRspNotification& p_rNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Process975RespReport]:AppNoBurstRspNotification. Device=");
    try
    {
        MAC devMac(p_rNotification.GetAddr().bytes);

        LOG_INFO_APP("[RespReport-975]: received for mac=" << devMac);
        if (SaveRespErr::IsErr_975(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_ERROR_APP("[RespReport-975]: received with error for mac=" << devMac);
            return;
        }
        DevicePtr dev = p_rNotification.pDevices->FindDevice(devMac);
        if (!dev)
        {
            LOG_WARN_APP("[RespReport-975]: Device not in DB, with mac= " << devMac);
            return;
        }

        C975_DeleteRoute_Resp *pwhResponse = (C975_DeleteRoute_Resp *) m_pResponse->whCmd.command;
        if (!pwhResponse)
        {
            LOG_ERROR_APP("[RespReport-975]: Incorrect response received for CMD975. DeviceMAC=" << devMac);
            return;
        }

        p_rNotification.pDevices->DeleteRoute(dev->id, pwhResponse->m_ucRouteID);
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Process975RespReport]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Process975RespReport]: System error!");
    }
}

void ResponseProcessor::Process976RespReport(AppNoBurstRspNotification& p_rNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Process976RespReport]:AppNoBurstRspNotification. Device=");
    try
    {
        MAC devMac(p_rNotification.GetAddr().bytes);

        LOG_INFO_APP("[RespReport-976]: received for mac=" << devMac);
        if (SaveRespErr::IsErr_976(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_ERROR_APP("[RespReport-976]: received with error for mac=" << devMac);
            return;
        }
        DevicePtr dev = p_rNotification.pDevices->FindDevice(devMac);
        if (!dev)
        {
            LOG_WARN_APP("[RespReport-976]: Device not in DB, with mac= " << devMac);
            return;
        }

        C976_WriteSourceRoute_Resp *pwhResponse = (C976_WriteSourceRoute_Resp *) m_pResponse->whCmd.command;
        if (!pwhResponse)
        {
            LOG_ERROR_APP("[RespReport-976]: Incorrect response received for CMD976. DeviceMAC=" << devMac);
            return;
        }

        if (pwhResponse->m_ucHopsNo <= 0)
        {
            LOG_WARN_APP("[RespReport-834]: the number of hops=" << (int) pwhResponse->m_ucHopsNo);
            return;
        }

        SourceRoutesListT sourceRoutes;
        /*compose devices string*/
        char* devicesList = new char[pwhResponse->m_ucHopsNo * 4/*nb of hexchars per hop*/+ 1/*string terminator*/];
        memset(devicesList, 0, pwhResponse->m_ucHopsNo * 4/*nb of hexchars per hop*/+ 1/*string terminator*/);

        for (int i = 0; i < pwhResponse->m_ucHopsNo; i++)
        {
            sprintf(devicesList + i * 4, "%04X", pwhResponse->m_aNicknameHopEntries[i]);
        }

        SourceRoute sr(dev->id, pwhResponse->m_ucRouteID, devicesList);
        sourceRoutes.push_back(sr);

        delete[] devicesList;
        p_rNotification.pDevices->ChangeSourceRoutes(sourceRoutes);
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Process976RespReport]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Process976RespReport]: System error!");
    }
}

void ResponseProcessor::Process977RespReport(AppNoBurstRspNotification& p_rNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Process977RespReport]:AppNoBurstRspNotification. Device=");
    try
    {
        MAC devMac(p_rNotification.GetAddr().bytes);

        LOG_INFO_APP("[RespReport-977]: received for mac=" << devMac);
        if (SaveRespErr::IsErr_977(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_ERROR_APP("[RespReport-977]: received with error for mac=" << devMac);
            return;
        }
        DevicePtr dev = p_rNotification.pDevices->FindDevice(devMac);
        if (!dev)
        {
            LOG_WARN_APP("[RespReport-977]: Device not in DB, with mac= " << devMac);
            return;
        }

        C977_DeleteSourceRoute_Resp *pwhResponse = (C977_DeleteSourceRoute_Resp *) m_pResponse->whCmd.command;
        if (!pwhResponse)
        {
            LOG_ERROR_APP("[RespReport-977]: Incorrect response received for CMD977. DeviceMAC=" << devMac);
            return;
        }

        p_rNotification.pDevices->DeleteSourceRoute(dev->id, pwhResponse->m_ucRouteID);
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Process977RespReport]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Process977RespReport]: System error!");
    }
}

void ResponseProcessor::Visit(AppTopologyNotification& p_rTopoNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Visit]:AppTopologyNotification. Device=");
    try
    {
        if (SaveRespErr::IsErr_839(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            return;
        }

        C839_ChangeNotification_Resp *pwhResp = (C839_ChangeNotification_Resp*) m_pResponse->whCmd.command;
        if (!pwhResp)
        {
            LOG_ERROR_APP("[Notification-839]: Incorrect response received for CMD839.");
            return;
        }

        for (int cmds = 0; cmds < pwhResp->ChangeNotificationNo; cmds++)
        {
            switch (pwhResp->ChangeNotifications[cmds])
            {
                case CMDID_C769_ReadJoinStatus:
                    Process769Notification(p_rTopoNotification);
                break;
                case CMDID_C785_ReadGraphList:
                    Process785Notification(p_rTopoNotification);
                break;
                case CMDID_C814_ReadDeviceListEntries:
                    Process814Notification(p_rTopoNotification);
                break;
                case CMDID_C833_ReadNetworkDeviceNeighbourHealth:
                    Process833Notification(p_rTopoNotification);
                break;
                case CMDID_C834_ReadNetworkTopologyInformation:
                    Process834Notification(p_rTopoNotification);
                break;
                case CMDID_C832_ReadNetworkDeviceIdentity:
                    Process832Notification(p_rTopoNotification);
                break;
            }
        }
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Visit::AppTopologyNotification]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Visit::AppTopologyNotification]: System error!");
    }
}

void ResponseProcessor::Process769Notification(AppTopologyNotification& p_rTopoNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Process769Notification]:AppTopologyNotification. Device=");
    try
    {
        C839_ChangeNotification_Resp *pwhResp = (C839_ChangeNotification_Resp*) m_pResponse->whCmd.command;
        if (!pwhResp)
        {
            LOG_ERROR_APP("[Notification-769]: Incorrect response received for CMD839.");
            return;
        }

        LOG_INFO_APP("[Notification-769]: received with mac=" << MAC(pwhResp->DeviceAddress) << " so send 769 req");

        C769_ReadJoinStatus_Req req;

        stack::WHartCommand whCmd;
        whCmd.commandID = CMDID_C769_ReadJoinStatus;
        whCmd.command = &req;

        gateway::GatewayIO::AppData appData;
        MAC mac = MAC(pwhResp->DeviceAddress);
        appData.appCmd = AbstractAppCommandPtr(new App769Cmd(mac));

        appData.appCmd->pCommands = p_rTopoNotification.pCommands;
        appData.appCmd->pDevices = p_rTopoNotification.pDevices;

        //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C769_ReadJoinStatus. Device="<< mac);
        m_pProcessor->gateway.SendWHRequest(appData, mac.Address(), whCmd, false, true);
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Process769Notification]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Process769Notification]: System error!");
    }
}

void ResponseProcessor::Process785Notification(AppTopologyNotification& p_rTopoNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Process785Notification]:AppTopologyNotification. Device=");
    try
    {
        C839_ChangeNotification_Resp *pwhResp = (C839_ChangeNotification_Resp*) m_pResponse->whCmd.command;
        if (!pwhResp)
        {
            LOG_ERROR_APP("[Notification-785]: Incorrect response received for CMD839.");
            return;
        }

        stack::WHartCommand whCmd;
        gateway::GatewayIO::AppData appData;

        //C833_ReadNetworkDeviceNeighbourHealth_Req req833;

        C785_ReadGraphList_Req req785;

        MAC devMac(pwhResp->DeviceAddress);
        ;
        DevicePtr dev = p_rTopoNotification.pDevices->FindDevice(devMac);
        WHartUniqueID addr = stack::NetworkManager_UniqueID();

        if (dev != 0)
        {
            memcpy(addr.bytes, pwhResp->DeviceAddress, sizeof(addr.bytes));

            LOG_INFO_APP("[Notification-785]: with device(in db)=" << devMac);

            req785.m_ucGraphListIndex = 0;
            whCmd.commandID = CMDID_C785_ReadGraphList;
            whCmd.command = &req785;

            bool isDevice = true;
            if (dev->Type() == Device::dtAccessPoint || dev->Type() == Device::dtGateway || dev->Type() == Device::dtSystemManager)
                isDevice = false;

            App785Cmd* app785 = new App785Cmd(dev->id, dev->Mac(), isDevice);
            appData.appCmd = AbstractAppCommandPtr(app785);
        }
        else
        {
            LOG_WARN_APP("[Notification-785]: with device(not in db)=" << devMac << " so skip sending 785 req");
            return;
        }

        appData.appCmd->pCommands = p_rTopoNotification.pCommands;
        appData.appCmd->pDevices = p_rTopoNotification.pDevices;

        try
        {
            //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C785_ReadGraphList. Device="<< MAC(addr.bytes));
            m_pProcessor->gateway.SendWHRequest(appData, (const WHartUniqueID&) addr, whCmd);
        }
        catch (std::exception ex)
        {
            p_rTopoNotification.pDevices->DelDeviceInfoInProgress(devMac);
            throw ex;
        }
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Process785Notification]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Process785Notification]: System error!");
    }
}

void ResponseProcessor::Process814Notification(AppTopologyNotification& p_rTopoNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Process814Notification]:AppTopologyNotification. Device=");
    try
    {
        C839_ChangeNotification_Resp *pwhResp = (C839_ChangeNotification_Resp*) m_pResponse->whCmd.command;
        if (!pwhResp)
        {
            LOG_ERROR_APP("[Notification-814]: Incorrect response received for CMD839.");
            return;
        }

        LOG_INFO_APP("[Notification-814]: received with mac=" << MAC(pwhResp->DeviceAddress) << " so send 814 req");

        //get uniqueDeviceIDs
        C814_ReadDeviceListEntries_Req whReq;
        whReq.m_ucDeviceListCode = 0 /*= Active device list. There is no implementation for: 1 = Whitelisted devices, 2 = Blacklisted devices*/;
        whReq.m_ucNoOfListEntriesToRead = MAX_DEVICE_LIST_ENTRIES_NO;
        whReq.m_unStartingListIndex = 0;

        stack::WHartCommand whCmd;
        whCmd.commandID = CMDID_C814_ReadDeviceListEntries;
        whCmd.command = &whReq;

        gateway::GatewayIO::AppData appData;
        appData.appCmd = AbstractAppCommandPtr(new App814Cmd());
        appData.appCmd->pCommands = p_rTopoNotification.pCommands;
        appData.appCmd->pDevices = p_rTopoNotification.pDevices;
        //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C814_ReadDeviceListEntries. Device=GW");
        m_pProcessor->gateway.SendWHRequest(appData, stack::Gateway_UniqueID(), whCmd);
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Process814Notification]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Process814Notification]: System error!");
    }
}

void ResponseProcessor::Process832Notification(AppTopologyNotification& p_rTopoNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Process832Notification]:AppTopologyNotification. Device=");
    try
    {
        C839_ChangeNotification_Resp *pwhResp = (C839_ChangeNotification_Resp*) m_pResponse->whCmd.command;
        if (!pwhResp)
        {
            LOG_ERROR_APP("[Notification-832]: Incorrect response received for CMD839.");
            return;
        }

        MAC devMac = MAC(pwhResp->DeviceAddress);

        LOG_INFO_APP("[Notification-832]: received with mac=" << devMac << " so send 832 req");

        DevicePtr dev = p_rTopoNotification.pDevices->FindDevice(devMac);

        C832_ReadNetworkDeviceIdentity_Req req;

        memcpy(req.DeviceUniqueID, pwhResp->DeviceAddress, sizeof(req.DeviceUniqueID));
        stack::WHartCommand whCmd;
        whCmd.commandID = CMDID_C832_ReadNetworkDeviceIdentity;
        whCmd.command = &req;

        gateway::GatewayIO::AppData appData;
        if (dev)
            appData.appCmd = AbstractAppCommandPtr(new App832Cmd(dev->Status(), devMac));
        else
            appData.appCmd = AbstractAppCommandPtr(new App832Cmd(Device::dsNetworkPacketsHeard, devMac));

        appData.appCmd->pCommands = p_rTopoNotification.pCommands;
        appData.appCmd->pDevices = p_rTopoNotification.pDevices;

        //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C832_ReadNetworkDeviceIdentity. Device=NM");
        m_pProcessor->gateway.SendWHRequest(appData, stack::NetworkManager_UniqueID(), whCmd);
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Process832Notification]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Process832Notification]: System error!");
    }
}

void ResponseProcessor::Process833Notification(AppTopologyNotification& p_rTopoNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Process833Notification]:AppTopologyNotification. Device=");
    try
    {
        C839_ChangeNotification_Resp *pwhResp = (C839_ChangeNotification_Resp*) m_pResponse->whCmd.command;
        if (!pwhResp)
        {
            LOG_ERROR_APP("[Notification-833]: Incorrect response received for CMD839.");
            return;
        }

        C833_ReadNetworkDeviceNeighbourHealth_Req req;

        memcpy(req.UniqueID, pwhResp->DeviceAddress, sizeof(req.UniqueID));

        req.NeighbourIndex = 0;
        req.NeighbourEntriesToRead = MaxNeighbours;

        stack::WHartCommand whCmd;
        whCmd.commandID = CMDID_C833_ReadNetworkDeviceNeighbourHealth;
        whCmd.command = &req;

        gateway::GatewayIO::AppData appData;

        MAC devMac = MAC(pwhResp->DeviceAddress);
        DevicePtr dev = p_rTopoNotification.pDevices->FindDevice(devMac);

        if (dev != 0)
        {
            if (!dev->IsRegistered())
            {
                p_rTopoNotification.pDevices->RegisterDevice(dev);
//                LOG_INFO_APP("[Notification-833]: with device(in db)=" << devMac << "made registered");
            }
            LOG_INFO_APP("[Notification-833]: with device(in db)=" << devMac);
            appData.appCmd = AbstractAppCommandPtr(new App833Cmd(dev->id, dev->Mac()));
        }
        else
        {
            LOG_INFO_APP("[Notification-833]: with device(not in db)=" << devMac);
            if (p_rTopoNotification.pDevices->IsDeviceInfoInProgress(devMac))
            {
                return;
            }

            appData.appCmd = AbstractAppCommandPtr(new AppNotificationDevCmd(devMac)); //when device is new
            p_rTopoNotification.pDevices->AddDeviceInfoInProgress(devMac);
        }

        appData.appCmd->pCommands = p_rTopoNotification.pCommands;
        appData.appCmd->pDevices = p_rTopoNotification.pDevices;

        try
        {
            //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C833_ReadNetworkDeviceNeighbourHealth. Device=NM");
            m_pProcessor->gateway.SendWHRequest(appData, stack::NetworkManager_UniqueID(), whCmd);
        }
        catch (std::exception ex)
        {
            p_rTopoNotification.pDevices->DelDeviceInfoInProgress(devMac);
            throw ex;
        }
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Process833Notification]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Process833Notification]: System error!");
    }
}

void ResponseProcessor::Process834Notification(AppTopologyNotification& p_rTopoNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Process834Notification]:AppTopologyNotification. Device=");
    try
    {
        C839_ChangeNotification_Resp *pwhResp = (C839_ChangeNotification_Resp*) m_pResponse->whCmd.command;
        if (!pwhResp)
        {
            LOG_ERROR_APP("[Notification-834]: Incorrect response received for CMD839.");
            return;
        }

        C834_ReadNetworkTopologyInformation_Req req834;
        C833_ReadNetworkDeviceNeighbourHealth_Req req833;
        stack::WHartCommand whCmd;
        gateway::GatewayIO::AppData appData;

        MAC devMac = MAC(pwhResp->DeviceAddress);
        DevicePtr dev = p_rTopoNotification.pDevices->FindDevice(devMac);

        if (dev != 0)
        {
            if (!dev->IsRegistered())
            {
                p_rTopoNotification.pDevices->RegisterDevice(dev);
//                LOG_INFO_APP("[Notification-834]: with device(in db)=" << devMac << "made registered");
            }
            LOG_INFO_APP("[Notification-834]: with device(in db)=" << devMac);

            memcpy(req834.DeviceLongAddress, pwhResp->DeviceAddress, sizeof(req834.DeviceLongAddress));
            req834.GraphIndexNo = 0;
            whCmd.commandID = CMDID_C834_ReadNetworkTopologyInformation;
            whCmd.command = &req834;

            appData.appCmd = AbstractAppCommandPtr(new App834Cmd(dev->id));
        }
        else
        {

            LOG_INFO_APP("[Notification-834]: with device(not in db)=" << devMac);
            if (p_rTopoNotification.pDevices->IsDeviceInfoInProgress(devMac))
            {
                return;
            }

            memcpy(req833.UniqueID, pwhResp->DeviceAddress, sizeof(req833.UniqueID));
            req833.NeighbourIndex = 0;
            req833.NeighbourEntriesToRead = MaxNeighbours;
            whCmd.commandID = CMDID_C833_ReadNetworkDeviceNeighbourHealth;
            whCmd.command = &req833;

            appData.appCmd = AbstractAppCommandPtr(new AppNotificationDevCmd(devMac)); //when device is new
            p_rTopoNotification.pDevices->AddDeviceInfoInProgress(devMac);
        }

        appData.appCmd->pCommands = p_rTopoNotification.pCommands;
        appData.appCmd->pDevices = p_rTopoNotification.pDevices;

        try
        {
            //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C834_ReadNetworkTopologyInformation/CMDID_C833_ReadNetworkDeviceNeighbourHealth. Device=NM");
            m_pProcessor->gateway.SendWHRequest(appData, stack::NetworkManager_UniqueID(), whCmd);
        }
        catch (std::exception ex)
        {
            p_rTopoNotification.pDevices->DelDeviceInfoInProgress(devMac);
            throw ex;
        }
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Process834Notification]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Process834Notification]: System error!");
    }
}

void ResponseProcessor::Visit(App000Cmd& p_rCmd000)
{
    //LOG_INFO_APP("[ResponseProcessor::Visit]:App000Cmd. Device=");
    try
    {
        if (SaveRespErr::IsErr_000(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_ERROR_APP("[Response-000]: received with error for mac=" << p_rCmd000.GetDevice()->Mac());
            return;
        }

        DevicePtr devicePtr = p_rCmd000.GetDevice();

        C000_ReadUniqueIdentifier_Resp *pwhResp = (C000_ReadUniqueIdentifier_Resp*) m_pResponse->whCmd.command;
        if (!pwhResp)
        {
            LOG_ERROR_APP("[Response-000]: Incorrect response received for CMD000. DeviceMAC=" << p_rCmd000.GetDevice()->Mac());
            return;
        }

        devicePtr->ResetChanged();

        devicePtr->SetSoftwareRevision(pwhResp->softwareRevisionLevel);
        devicePtr->SetDeviceCode(pwhResp->expandedDeviceType);

        const WHartUniqueID &gwAddr = stack::Gateway_UniqueID();
        const WHartUniqueID &nnAddr = stack::NetworkManager_UniqueID();
        bool isDevice = false;
        if (memcmp(devicePtr->Mac().Address().bytes, gwAddr.bytes, sizeof(devicePtr->Mac().Address().bytes)) && memcmp(
            devicePtr->Mac().Address().bytes, nnAddr.bytes, sizeof(devicePtr->Mac().Address().bytes)))
        {
            isDevice = true;
            devicePtr->Type(Device::dtRoutingDeviceNode);
        }

        if (pwhResp->deviceProfile == DeviceProfileCodes_WIRELESSHART_GATEWAY)
            if (isDevice == true)
                devicePtr->Type(Device::dtAccessPoint);

        if (devicePtr->Changed())
            p_rCmd000.pDevices->UpdateDevice(devicePtr, false, false);
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Visit::App000Cmd]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Visit::App000Cmd]: System error!");
    }
}

void ResponseProcessor::Visit(App020Cmd& p_rCmd020)
{
    //LOG_INFO_APP("[ResponseProcessor::Visit]:App020Cmd. Device=");
    try
    {
        if (SaveRespErr::IsErr_020(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_ERROR_APP("[Response-020]: received with error for mac=" << p_rCmd020.GetDevice()->Mac());
            return;
        }

        DevicePtr devicePtr = p_rCmd020.GetDevice();

        C020_ReadLongTag_Resp *pwhResp = (C020_ReadLongTag_Resp*) m_pResponse->whCmd.command;
        if (!pwhResp)
        {
            LOG_ERROR_APP("[Response-020]: Incorrect response received for CMD020. DeviceMAC=" << p_rCmd020.GetDevice()->Mac());
            return;
        }

        devicePtr->ResetChanged();

        std::string longTag;
        for (unsigned int i = 0; i < sizeof(pwhResp->longTag); ++i)
            longTag += ConvertToHex((unsigned char) pwhResp->longTag[i]);

        devicePtr->SetTAG(longTag);

        if (devicePtr->Changed())
            p_rCmd020.pDevices->UpdateDevice(devicePtr, false, false);
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Visit::App020Cmd]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Visit::App020Cmd]: System error!");
    }
}

void ResponseProcessor::Visit(App769Cmd& p_rCmd769)
{
    //LOG_INFO_APP("[ResponseProcessor::Visit]:App769Cmd. Device=");
    try
    {
        MAC devMac = p_rCmd769.GetDeviceMac();
        if (SaveRespErr::IsErr_769(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_ERROR_APP("[Response-769]: received with error for mac=" << devMac);
            return;
        }

        C769_ReadJoinStatus_Resp *pwhResp = (C769_ReadJoinStatus_Resp *) m_pResponse->whCmd.command;
        if (!pwhResp)
        {
            LOG_ERROR_APP("[Response-769]: Incorrect response received for CMD769. DeviceMAC=" << devMac);
            return;
        }

        LOG_DEBUG_APP("[Response-769]: with status=" << std::hex << (int) (pwhResp->joinStatus) << " for dev mac="
                    << p_rCmd769.GetDeviceMac());

        DevicePtr dev = p_rCmd769.pDevices->FindDevice(devMac);
        if (dev)
        {
            if (Device::dsJoinFailed == (Device::DeviceStatus) pwhResp->joinStatus)
            {
                LOG_INFO_APP("[Response-769]:Join failed for mac=" << dev->Mac().ToString() << "so clean reports...");
                p_rCmd769.pDevices->CleanReports(dev->id);
            }
            if (Device::dsJoinRequested == (Device::DeviceStatus) pwhResp->joinStatus)
            {
                LOG_INFO_APP("[Response-769]:Join requested for mac=" << dev->Mac().ToString() << "so clean reports...");
                p_rCmd769.pDevices->CleanReports(dev->id);
            }
        }

        C832_ReadNetworkDeviceIdentity_Req req;

        memcpy(req.DeviceUniqueID, devMac.Address().bytes, sizeof(req.DeviceUniqueID));

        stack::WHartCommand whCmd;
        whCmd.commandID = CMDID_C832_ReadNetworkDeviceIdentity;
        whCmd.command = &req;

        gateway::GatewayIO::AppData appData;
        if (pwhResp->joinStatus == 0)
        {
            LOG_WARN_APP("[Response-769]: has been received with '0' for dev=" << p_rCmd769.GetDeviceMac() << " so skip sending 832 cmd");
            return;
        }
        else
        {
            appData.appCmd = AbstractAppCommandPtr(new App832Cmd((Device::DeviceStatus) pwhResp->joinStatus, devMac));
        }

        LOG_DEBUG_APP("[Response-769]: send 832 req for mac=" << devMac);

        appData.appCmd->pDevices = p_rCmd769.pDevices;
        appData.appCmd->pCommands = p_rCmd769.pCommands;

        //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C832_ReadNetworkDeviceIdentity. Device=NM");
        m_pProcessor->gateway.SendWHRequest(appData, stack::NetworkManager_UniqueID(), whCmd, false, true);
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Visit::App769Cmd]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Visit::App769Cmd]: System error!");
    }
}

void ResponseProcessor::Visit(App814Cmd& p_rCmd814)
{
    //LOG_INFO_APP("[ResponseProcessor::Visit]:App814Cmd. Device=");
    //errors
    try
    {
        if (SaveRespErr::IsErr_814(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_ERROR_APP("[Response-814]: received with hostErrorCode=" << m_pResponse->hostErr << " and respErrCode="
                        << (int) (m_pResponse->whCmd.responseCode));
            return;
        }

        C814_ReadDeviceListEntries_Resp *pwhResp = (C814_ReadDeviceListEntries_Resp*) m_pResponse->whCmd.command;
        if (!pwhResp)
        {
            LOG_ERROR_APP("[Response-814]: Incorrect response received for CMD814.");
            return;
        }

        if (pwhResp->m_unStartingListIndex == 0)
        {
            p_rCmd814.macs.reserve(pwhResp->m_unTotalNoOfEntriesInList);
        }

        LOG_DEBUG_APP("[Response-814]: device_list received has elements no. = " << (int) pwhResp->m_ucNoOfListEntriesRead
                    << " from indexNo = " << pwhResp->m_unStartingListIndex);
        LOG_DEBUG_APP("[Response-814]: device_list -> begin");

        for (int i = 0; i < pwhResp->m_ucNoOfListEntriesRead; i++)
        {
            MAC devMac(pwhResp->m_aDeviceUniqueIds[i]);

            bool isNM = memcmp(pwhResp->m_aDeviceUniqueIds[i], hart7::stack::NetworkManager_UniqueID().bytes, sizeof(WHartUniqueID)) == 0;
            bool isGW = memcmp(pwhResp->m_aDeviceUniqueIds[i], hart7::stack::Gateway_UniqueID().bytes, sizeof(WHartUniqueID)) == 0;

            if (isNM || isGW)
            {
                DevicePtr dev = p_rCmd814.pDevices->FindDevice(devMac);

                if (!dev || !(dev->IsRegistered()))
                {
                    C832_ReadNetworkDeviceIdentity_Req req;

                    memcpy(req.DeviceUniqueID, devMac.Address().bytes, sizeof(WHartUniqueID));

                    LOG_DEBUG_APP("[Response-814]: send 832 req for mac=" << devMac);

                    stack::WHartCommand whCmd;
                    whCmd.commandID = CMDID_C832_ReadNetworkDeviceIdentity;
                    whCmd.command = &req;

                    gateway::GatewayIO::AppData appData;
                    appData.appCmd = AbstractAppCommandPtr(new App832Cmd(Device::dsRegistered, devMac));

                    appData.appCmd->pDevices = p_rCmd814.pDevices;
                    appData.appCmd->pCommands = p_rCmd814.pCommands;

                    //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C832_ReadNetworkDeviceIdentity. Device=NM");
                    m_pProcessor->gateway.SendWHRequest(appData, stack::NetworkManager_UniqueID(), whCmd);
                }
            }
            p_rCmd814.macs.push_back(devMac);
            LOG_DEBUG_APP("[Response-814]: device[" << i << "] has mac=" << MAC(pwhResp->m_aDeviceUniqueIds[i]));
        }

        LOG_DEBUG_APP("[Response-814]: device_list -> end");

        if (p_rCmd814.macs.size() < pwhResp->m_unTotalNoOfEntriesInList)
        {

            LOG_DEBUG_APP("[Response-814]: device_list received total elements = " << p_rCmd814.macs.size()
                        << "from the expected total elements = " << pwhResp->m_unTotalNoOfEntriesInList << ", so send the same cmd...");

            //get uniqueDeviceIDs
            C814_ReadDeviceListEntries_Req whReq;
            whReq.m_ucDeviceListCode = 0 /*= Active device list. There is no implementation for: 1 = Whitelisted devices, 2 = Blacklisted devices*/;
            whReq.m_ucNoOfListEntriesToRead = MAX_DEVICE_LIST_ENTRIES_NO;
            whReq.m_unStartingListIndex = pwhResp->m_unStartingListIndex + pwhResp->m_ucNoOfListEntriesRead;
            stack::WHartCommand whCmd;
            whCmd.commandID = CMDID_C814_ReadDeviceListEntries;
            whCmd.command = &whReq;

            //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C814_ReadDeviceListEntries. Device=GW");
            m_pProcessor->gateway.SendWHRequest(m_pResponse->appData, stack::Gateway_UniqueID(), whCmd);
            return;
        }

        LOG_INFO_APP("[Response-814]: device_list received with elements = " << p_rCmd814.macs.size() << "  and registered_devices="
                    << /*deviceNo*/ (int)pwhResp->m_ucNoOfListEntriesRead << " have been unregistered.");
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Visit::App814Cmd]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Visit::App814Cmd]: System error!");
    }
}

void ResponseProcessor::Visit(App787Cmd& p_rCmd787)
{
    //LOG_INFO_APP("[ResponseProcessor::Visit]:App787Cmd. Device=");
    try
    {
        if (SaveRespErr::IsErr_787(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_ERROR_APP("[Response-787]: received with error for mac=" << p_rCmd787.GetNeighbList().devMac);
            if (p_rCmd787.GetNeighbList().neighbors.size())
                p_rCmd787.pDevices->ReplaceDevNeighbSignalLevels(p_rCmd787.GetNeighbList().deviceID, p_rCmd787.GetNeighbList().neighbors);

            return;
        }

        C787_ReportNeighborSignalLevels_Resp *pwhResp = (C787_ReportNeighborSignalLevels_Resp*) m_pResponse->whCmd.command;
        if (!pwhResp)
        {
            LOG_ERROR_APP("[Response-787]: Incorrect response received for CMD787. DeviceMAC=" << p_rCmd787.GetNeighbList().devMac);
            return;
        }

        LOG_DEBUG_APP("[Response-787]: neighb_list received has elements no. = " << (int) pwhResp->m_ucNoOfNeighborEntriesRead
                    << " from indexNo = " << pwhResp->m_ucNeighborTableIndex);
        LOG_DEBUG_APP("[Response-787]: neighb_list -> begin");
        for (int i = 0; i < pwhResp->m_ucNoOfNeighborEntriesRead; i++)
        {
            DevicePtr dev = p_rCmd787.pDevices->FindDevice(NickName(pwhResp->m_aNeighbors[i].nickname));
            if (!dev)
            {
                LOG_WARN_APP("[Response-787]: not found in db the neighbor=" << (NickName(pwhResp->m_aNeighbors[i].nickname)));
                continue;
            }
            p_rCmd787.GetNeighbList().neighbors.push_back(NeighbourSignalLevel(dev->id, pwhResp->m_aNeighbors[i].RSLindB));
            LOG_DEBUG_APP("[Response-787]: device[" << i << "] has id=" << dev->id);
        }
        LOG_DEBUG_APP("[Response-787]: neighb_list -> end");

        if (pwhResp->m_ucNeighborTableIndex + 1 < pwhResp->m_ucTotalNoOfNeighbors)
        {
            C787_ReportNeighborSignalLevels_Req whReq;
            whReq.m_ucNeighborTableIndex = pwhResp->m_ucNeighborTableIndex + pwhResp->m_ucNoOfNeighborEntriesRead;
            whReq.m_ucNoOfNeighborEntriesToRead = C787_MAX_NEIGHBORS_LIST;

            stack::WHartCommand whCmd;
            whCmd.commandID = CMDID_C787_ReportNeighborSignalLevels;
            whCmd.command = &whReq;

            //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C787_ReportNeighborSignalLevels. Device=" << p_rCmd787.GetNeighbList().devMac);
            m_pProcessor->gateway.SendWHRequest(m_pResponse->appData, p_rCmd787.GetNeighbList().devMac.Address(), whCmd);
        }

        p_rCmd787.pDevices->ReplaceDevNeighbSignalLevels(p_rCmd787.GetNeighbList().deviceID, p_rCmd787.GetNeighbList().neighbors);
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Visit::App787Cmd]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Visit::App787Cmd]: System error!");
    }
}

void ResponseProcessor::Visit(App788Cmd& p_rCmd788)
{
    //LOG_INFO_APP("[ResponseProcessor::Visit]:App788Cmd. Device=");
    try
    {
        if (SaveRespErr::IsErr_788(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_ERROR_APP("[Response-788]: received with error for deviceID=" << p_rCmd788.GetDeviceId());
            return;
        }

        int deviceId = p_rCmd788.GetDeviceId();
        C788_AlarmPathDown_Resp *pwResp = (C788_AlarmPathDown_Resp*) m_pResponse->whCmd.command;
        if (!pwResp)
        {
            LOG_ERROR_APP("[Response-788]: Incorrect response received for CMD788. DeviceID=" << p_rCmd788.GetDeviceId());
            return;
        }

        //save in bd
        DevicePtr dev = p_rCmd788.pDevices->FindDevice(NickName(pwResp->Nickname));
        if (!dev)
        {
            LOG_WARN_APP("[Response-788]: Device nickname not in db, nickname= " << NickName(pwResp->Nickname));
            return;
        }
        p_rCmd788.pDevices->InsertAlarm(Alarm(deviceId, CMDID_C788_AlarmPathDown, dev->id));
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Visit::App788Cmd]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Visit::App788Cmd]: System error!");
    }
}

void ResponseProcessor::Visit(App789Cmd& p_rCmd789)
{
    //LOG_INFO_APP("[ResponseProcessor::Visit]:App789Cmd. Device=");
    try
    {
        if (SaveRespErr::IsErr_789(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_ERROR_APP("[Response-789]: received with error for deviceid=" << p_rCmd789.GetDeviceId());
            return;
        }

        int deviceId = p_rCmd789.GetDeviceId();
        C789_AlarmSourceRouteFailed_Resp *pwResp = (C789_AlarmSourceRouteFailed_Resp*) m_pResponse->whCmd.command;
        if (!pwResp)
        {
            LOG_ERROR_APP("[Response-789]: Incorrect response received for CMD789. DeviceID=" << p_rCmd789.GetDeviceId());
            return;
        }

        //save in bd
        DevicePtr dev = p_rCmd789.pDevices->FindDevice(NickName(pwResp->m_unNicknameOfUnreachableNeighbor));
        if (!dev)
        {
            LOG_WARN_APP("[Response-789]: Device nickname not in db, nickname= " << NickName(pwResp->m_unNicknameOfUnreachableNeighbor));
            return;
        }
        p_rCmd789.pDevices->InsertAlarm(Alarm(deviceId, CMDID_C789_AlarmSourceRouteFailed, dev->id,
            pwResp->m_ulNetworkLayerMICfromNPDUthatFailedRouting));
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Visit::App789Cmd]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Visit::App789Cmd]: System error!");
    }
}

void ResponseProcessor::Visit(App790Cmd& p_rCmd790)
{
    //LOG_INFO_APP("[ResponseProcessor::Visit]:App790Cmd. Device=");
    try
    {
        if (SaveRespErr::IsErr_790(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_ERROR_APP("[Response-790]: received with error for deviceid=" << p_rCmd790.GetDeviceId());
            return;
        }

        int deviceId = p_rCmd790.GetDeviceId();
        C790_AlarmGraphRouteFailed_Resp *pwResp = (C790_AlarmGraphRouteFailed_Resp*) m_pResponse->whCmd.command;
        if (!pwResp)
        {
            LOG_ERROR_APP("[Response-790]: Incorrect response received for CMD790. DeviceID=" << p_rCmd790.GetDeviceId());
            return;
        }
        //save in bd
        p_rCmd790.pDevices->InsertAlarm(Alarm(deviceId, CMDID_C790_AlarmGraphRouteFailed, pwResp->m_unGraphIdOfFailedRoute));
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Visit::App790Cmd]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Visit::App790Cmd]: System error!");
    }
}

void ResponseProcessor::Visit(App791Cmd& p_rCmd791)
{
    //LOG_INFO_APP("[ResponseProcessor::Visit]:App791Cmd. Device=");
    try
    {
        if (SaveRespErr::IsErr_791(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_ERROR_APP("[Response-791]: received with error for deviceid=" << p_rCmd791.GetDeviceId());
            return;
        }

        int deviceId = p_rCmd791.GetDeviceId();
        C791_AlarmTransportLayerFailed_Resp *pwResp = (C791_AlarmTransportLayerFailed_Resp*) m_pResponse->whCmd.command;
        if (!pwResp)
        {
            LOG_ERROR_APP("[Response-791]: Incorrect response received for CMD791. DeviceID=" << p_rCmd791.GetDeviceId());
            return;
        }

        //save in bd
        DevicePtr dev = p_rCmd791.pDevices->FindDevice(NickName(pwResp->m_unNicknameOfUnreachablePeer));
        if (!dev)
        {
            LOG_WARN_APP("[Response-791]: Device nickname not in db, nickname= " << NickName(pwResp->m_unNicknameOfUnreachablePeer));
            return;
        }
        p_rCmd791.pDevices->InsertAlarm(Alarm(deviceId, CMDID_C791_AlarmTransportLayerFailed, dev->id));
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Visit::App791Cmd]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Visit::App791Cmd]: System error!");
    }
}

void ResponseProcessor::Visit(App832Cmd& p_rCmd832)
{
    //LOG_INFO_APP("[ResponseProcessor::Visit]:App832Cmd. Device=");
    try
    {
        if (SaveRespErr::IsErr_832(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_ERROR_APP("[Response-832]: nickname has failed! for mac=" << p_rCmd832.GetDeviceMac());
            return;
        }

        C832_ReadNetworkDeviceIdentity_Resp *pwhResp = (C832_ReadNetworkDeviceIdentity_Resp*) m_pResponse->whCmd.command;
        if (!pwhResp)
        {
            LOG_ERROR_APP("[Response-832]: Incorrect response received for CMD832. DeviceMAC=" << p_rCmd832.GetDeviceMac());
            return;
        }

        MAC devMac = MAC(pwhResp->DeviceUniqueID);
        std::string longTag;
        for (unsigned int i = 0; i < sizeof(pwhResp->LongTag); ++i)
            longTag += ConvertToHex((unsigned char) pwhResp->LongTag[i]);

        DevicePtr dev = p_rCmd832.pDevices->FindDevice(devMac);

        if (dev)
        {//device-ul exista in bd
            //update bd
            bool nicknameChanged = false;
            if (dev->Nickname() != NickName(pwhResp->Nickname))
            {
                dev->Nickname(pwhResp->Nickname);
                nicknameChanged = true;
            }
            bool statusChanged = false;
            if (dev->Status() != p_rCmd832.GetDeviceStatus())
            {
                dev->Status(p_rCmd832.GetDeviceStatus());
                statusChanged = true;
            }

            dev->SetTAG(longTag);
            p_rCmd832.pDevices->UpdateDevice(dev, nicknameChanged, statusChanged);
            LOG_DEBUG_APP("[Response-832]: device with mac= " << devMac << " updated in db");
        }
        else
        {
            //insert bd
            dev = DevicePtr(new Device());
            dev->Mac(devMac);
            dev->SetTAG(longTag);
            dev->Nickname(NickName(pwhResp->Nickname));
            dev->Status(p_rCmd832.GetDeviceStatus());
            if (dev->Type() == Device::dtUnknown)
            {
                dev->Type(Device::dtRoutingDeviceNode);
            }
            if (MAC(stack::Gateway_UniqueID().bytes) == devMac)
            {
                dev->Type(Device::dtGateway);
            }
            if (devMac == MAC(stack::NetworkManager_UniqueID().bytes))
            {
                dev->Type(Device::dtSystemManager);
            }
            p_rCmd832.pDevices->AddNewDevice(dev);
            LOG_DEBUG_APP("[Response-832]: device with mac= " << devMac << " was inserted in db");
        }
        //also SEND CMD 0
        {
            C000_ReadUniqueIdentifier_Req req;
            stack::WHartCommand whCmd;
            whCmd.commandID = CMDID_C000_ReadUniqueIdentifier;
            whCmd.command = &req;

            gateway::GatewayIO::AppData appData;

            LOG_DEBUG_APP("[Response-832]: now send cmd 0 for device=" << devMac);

            appData.appCmd = AbstractAppCommandPtr(new App000Cmd(dev));
            appData.appCmd->pCommands = p_rCmd832.pCommands;
            appData.appCmd->pDevices = p_rCmd832.pDevices;

            //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C000_ReadUniqueIdentifier. Device=" << devMac);
            m_pProcessor->gateway.SendWHRequest(appData, devMac.Address(), whCmd, false, true);
        }

        //also test for notification bit for device_configuration
        unsigned short notifMask = dev->GetNotificationBitMask();

        if (notifMask & NotificationMaskCodesMask_DeviceConfiguration)
            return;

        LOG_INFO_APP("[Response-832]: device=" << dev->Mac() << " has no DevConfigBitMask; set to send 837 req");
        notifMask = notifMask | NotificationMaskCodesMask_DeviceConfiguration;
        dev->SetNotificationBitMask(notifMask);

        stack::WHartCommand whCmd;
        C837_WriteUpdateNotificationBitMask_Req whReq;
        whReq.ChangeNotificationFlags = notifMask;
        memcpy(whReq.UniqueID, dev->Mac().Address().bytes, sizeof(WHartUniqueID));

        whCmd.commandID = CMDID_C837_WriteUpdateNotificationBitMask;
        whCmd.command = &whReq;

        m_pResponse->appData.appCmd = AbstractAppCommandPtr(new AppSetNotificationBitMask(devMac, notifMask));

        //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C837_WriteUpdateNotificationBitMask. Device=GW");
        m_pProcessor->gateway.SendWHRequest(m_pResponse->appData, stack::Gateway_UniqueID(), whCmd, false, true);
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Visit::App832Cmd]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Visit::App832Cmd]: System error!");
    }
}

void ResponseProcessor::Visit(App833Cmd& p_rCmd833)
{
    //LOG_INFO_APP("[ResponseProcessor::Visit]:App833Cmd. Device=");
    try
    {
        if (SaveRespErr::IsErr_833(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_ERROR_APP("[Response-833]: error received for deviceID=" << p_rCmd833.GetFromDeviceID());
            return;
        }

        C833_ReadNetworkDeviceNeighbourHealth_Resp *pwhResp = (C833_ReadNetworkDeviceNeighbourHealth_Resp*) m_pResponse->whCmd.command;
        if (!pwhResp)
        {
            LOG_ERROR_APP("[Response-833]: Incorrect response received for CMD833. DeviceID=" << p_rCmd833.GetFromDeviceID());
            return;
        }

        DevicePtr toDevice;
        for (int i = 0; i < pwhResp->NeighbourCount; i++)
        {
            toDevice = p_rCmd833.pDevices->FindDevice(NickName(pwhResp->Neighbours[i].NeighbourNickname));
            if (toDevice == NULL)
            {
                LOG_WARN_APP("[Response-833]: nickname received wasn't found in db: " << NickName(pwhResp->Neighbours[i].NeighbourNickname));
                continue;
            }
            if (pwhResp->Neighbours[i].TransmittedPacketCount == 0 && pwhResp->Neighbours[i].TransmittedPacketWithNoACKCount == 0)
            {
                p_rCmd833.GetNeighbSignalLevelList().push_back(NeighbourSignalLevel(toDevice->id, pwhResp->Neighbours[i].NeighbourRSL));
                continue;
            }

            //devNeighborsHealth
            p_rCmd833.GetNeighbHealthList().m_oNeighborsList.push_back(NeighborHealth(toDevice->id,
                hart7::hostapp::NeighborHealth::g_nNoClockSource, pwhResp->Neighbours[i].NeighbourRSL,
                pwhResp->Neighbours[i].TransmittedPacketCount, pwhResp->Neighbours[i].TransmittedPacketWithNoACKCount,
                pwhResp->Neighbours[i].ReceivedPacketCount));
        }

        // resend the 833 command if NeighbourCount = MaxNeighbours
        if (pwhResp->NeighbourCount == MaxNeighbours)
        {
            C833_ReadNetworkDeviceNeighbourHealth_Req req;

            memcpy(req.UniqueID, pwhResp->UniqueID, sizeof(req.UniqueID));

            req.NeighbourIndex = pwhResp->NeighbourIndex + pwhResp->NeighbourCount;
            req.NeighbourEntriesToRead = MaxNeighbours;

            stack::WHartCommand whCmd;
            whCmd.commandID = CMDID_C833_ReadNetworkDeviceNeighbourHealth;
            whCmd.command = &req;

            //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C833_ReadNetworkDeviceNeighbourHealth. Device=NM");
            m_pProcessor->gateway.SendWHRequest(m_pResponse->appData, stack::NetworkManager_UniqueID(), whCmd);
            return;
        }
        p_rCmd833.pDevices->ReplaceDevNeighbSignalLevels(p_rCmd833.GetFromDeviceID(), p_rCmd833.GetNeighbSignalLevelList());
        p_rCmd833.pDevices->ReplaceDeviceNeighborsHealth(p_rCmd833.GetNeighbHealthList());
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Visit::App833Cmd]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Visit::App833Cmd]: System error!");
    }
}

void ResponseProcessor::Visit(App834Cmd& p_rCmd834)
{
    //LOG_INFO_APP("[ResponseProcessor::Visit]:App834Cmd. Device=");
    try
    {
        if (SaveRespErr::IsErr_834(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_ERROR_APP("[Response-834]: received with error for deviceID=" << p_rCmd834.GetDeviceID());
            return;
        }

        C834_ReadNetworkTopologyInformation_Resp *pwhResp = (C834_ReadNetworkTopologyInformation_Resp*) m_pResponse->whCmd.command;
        if (!pwhResp)
        {
            LOG_ERROR_APP("[Response-834]: Incorrect response received for CMD834. DeviceID=" << p_rCmd834.GetDeviceID());
            return;
        }

        DevicePtr toDevice;
        int graphID = 0;

        if ((pwhResp->GraphIndexNo) < (pwhResp->TotalGraphsNo - 1))
        {
            C834_ReadNetworkTopologyInformation_Req req;

            memcpy(req.DeviceLongAddress, pwhResp->DeviceLongAddress, sizeof(req.DeviceLongAddress));
            req.GraphIndexNo = pwhResp->GraphIndexNo + 1;

            stack::WHartCommand whCmd;
            whCmd.commandID = CMDID_C834_ReadNetworkTopologyInformation;
            whCmd.command = &req;

            // cache info for cmd 834 m_pResponse
            for (int i = 0; i < pwhResp->NeighboursNo; i++)
            {
                toDevice = p_rCmd834.pDevices->FindDevice(NickName(pwhResp->Neighbour[i]));
                graphID = pwhResp->IndexGraphId;

                if (!toDevice)
                {
                    LOG_WARN_APP("[Response-834]: toDevice not found with nickname=" << NickName(pwhResp->Neighbour[i]));
                    continue;
                }

                p_rCmd834.GetGraphList().push_back(GraphNeighbor(toDevice->id, graphID, i));
            }

            //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C834_ReadNetworkTopologyInformation. Device=NM");
            m_pProcessor->gateway.SendWHRequest(m_pResponse->appData, stack::NetworkManager_UniqueID(), whCmd);
            return;
        }

        // cache info for cmd 834 m_pResponse
        for (int i = 0; i < pwhResp->NeighboursNo; i++)
        {
            toDevice = p_rCmd834.pDevices->FindDevice(NickName(pwhResp->Neighbour[i]));
            graphID = pwhResp->IndexGraphId;

            if (!toDevice)
            {
                LOG_WARN_APP("[Notification-834]: toDevice not found with nickname=" << NickName(pwhResp->Neighbour[i]));
                continue;
            }

            p_rCmd834.GetGraphList().push_back(GraphNeighbor(toDevice->id, graphID, i));
        }
        p_rCmd834.pDevices->ReplaceDeviceGraphs(p_rCmd834.GetDeviceID(), p_rCmd834.GetGraphList());
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Visit::App834Cmd]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Visit::App834Cmd]: System error!");
    }
}

void ResponseProcessor::Visit(App785Cmd& p_rCmd785)
{
    //LOG_INFO_APP("[ResponseProcessor::Visit]:App785Cmd. Device=");
    try
    {
        if (SaveRespErr::IsErr_785(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_ERROR_APP("[Response-785]: received with error for deviceID=" << p_rCmd785.GetDeviceID());
            return;
        }

        C785_ReadGraphList_Resp *pwhResp = (C785_ReadGraphList_Resp*) m_pResponse->whCmd.command;
        if (!pwhResp)
        {
            LOG_ERROR_APP("[Response-785]: Incorrect response received for CMD785. DeviceMAC=" << p_rCmd785.GetDeviceMac());
            return;
        }

        DevicePtr toDevice;
        int graphID = 0;

        if ((pwhResp->m_ucGraphListIndex) < (pwhResp->m_ucTotalNoOfGraphs - 1))
        {
            C785_ReadGraphList_Req req;

            req.m_ucGraphListIndex = pwhResp->m_ucGraphListIndex + 1;

            stack::WHartCommand whCmd;
            whCmd.commandID = CMDID_C785_ReadGraphList;
            whCmd.command = &req;

            // cache info for cmd 785 m_pResponse
            for (int i = 0; i < pwhResp->m_ucNoOfNeighbors; i++)
            {

                //SET ACCESS_POINT TYPE
                NickName nick(pwhResp->m_aNicknameOfNeighbor[i]);
                if (p_rCmd785.IsDevice())
                {
                    if (nick.Address() == stack::Gateway_Nickname())
                        p_rCmd785.SetIsToBeAccessPoint();
                }

                graphID = pwhResp->m_unGraphId;
                toDevice = p_rCmd785.pDevices->FindDevice(nick);
                if (!toDevice)
                {
                    LOG_WARN_APP("[Response-785]: toDevice not found with nickname=" << NickName(pwhResp->m_aNicknameOfNeighbor[i]));
                    continue;
                }

                p_rCmd785.GetGraphList().push_back(GraphNeighbor(toDevice->id, graphID, i));
            }
            //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:C785_ReadGraphList_Req. Device=" << p_rCmd785.GetDeviceMac());
            m_pProcessor->gateway.SendWHRequest(m_pResponse->appData, p_rCmd785.GetDeviceMac().Address(), whCmd);
            return;
        }

        // cache info for cmd 785 m_pResponse
        for (int i = 0; i < pwhResp->m_ucNoOfNeighbors; i++)
        {

            //SET ACCESS_POINT TYPE
            NickName nick(pwhResp->m_aNicknameOfNeighbor[i]);
            if (p_rCmd785.IsDevice())
            {
                if (nick.Address() == stack::Gateway_Nickname())
                    p_rCmd785.SetIsToBeAccessPoint();
            }

            toDevice = p_rCmd785.pDevices->FindDevice(nick);
            graphID = pwhResp->m_unGraphId;

            if (!toDevice)
            {
                LOG_WARN_APP("[Response-785]: toDevice not found with nickname=" << nick);
                continue;
            }

            p_rCmd785.GetGraphList().push_back(GraphNeighbor(toDevice->id, graphID, i));
        }
        p_rCmd785.pDevices->ReplaceDeviceGraphs(p_rCmd785.GetDeviceID(), p_rCmd785.GetGraphList());
        if (p_rCmd785.IsToBeAccessPoint()) //update in db
        {
            DevicePtr dev = p_rCmd785.pDevices->FindDevice(p_rCmd785.GetDeviceMac());
            if (dev && dev->Type() != Device::dtAccessPoint)
            {
                dev->Type(Device::dtAccessPoint);
                p_rCmd785.pDevices->UpdateDevice(dev, false, false);
            }
        }
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Visit::App785Cmd]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Visit::App785Cmd]: System error!");
    }
}


void ResponseProcessor::Visit(AppNotificationDevCmd& p_rNotifDevCmd)
{
    //LOG_INFO_APP("[ResponseProcessor::Visit]:AppNotificationDevCmd. Device=");
    try
    {
        switch (p_rNotifDevCmd.m_state)
        {
            case AppNotificationDevCmd::GetNeighbours_state:
            {
                if (!SaveRespErr::IsErr_833(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
                {

                    C833_ReadNetworkDeviceNeighbourHealth_Resp *pwhResp =
                                (C833_ReadNetworkDeviceNeighbourHealth_Resp*) m_pResponse->whCmd.command;
                    if (!pwhResp)
                    {
                        LOG_ERROR_APP("[NotificationDevInfo]: Incorrect response received for CMD833. DeviceMAC="
                                    << p_rNotifDevCmd.m_devInfo.DeviceMAC);
                        return;
                    }

                    if (pwhResp->NeighbourIndex == 0)
                        p_rNotifDevCmd.m_devInfo.NeighborsList.reserve(pwhResp->NeighbourCount);

                    //SET ACCESS_POINT TYPE
                    const WHartUniqueID &devAddr = p_rNotifDevCmd.m_devInfo.DeviceMAC.Address();
                    const WHartUniqueID &gwAddr = stack::Gateway_UniqueID();
                    const WHartUniqueID &nnAddr = stack::NetworkManager_UniqueID();
                    bool isDevice = false;
                    if (memcmp(devAddr.bytes, gwAddr.bytes, sizeof(devAddr.bytes)) && memcmp(devAddr.bytes, nnAddr.bytes,
                        sizeof(devAddr.bytes)))
                    {
                        isDevice = true;
                    }

                    LOG_DEBUG_APP("[NotificationDevInfo]: neighb_list received has elements no. = " << (int) pwhResp->NeighbourCount
                                << " from indexNo = " << pwhResp->NeighbourIndex);
                    LOG_DEBUG_APP("[NotificationDevInfo]: neighb_list -> begin");
                    for (int i = 0; i < pwhResp->NeighbourCount; i++)
                    {
                        p_rNotifDevCmd.m_devInfo.NeighborsList.push_back(NotificationDevInfo::Neighbour());
                        NotificationDevInfo::Neighbour &neighb = *p_rNotifDevCmd.m_devInfo.NeighborsList.rbegin();
                        neighb.DeviceNickName = NickName(pwhResp->Neighbours[i].NeighbourNickname);

                        //SET ACCESS_POINT TYPE
                        if (isDevice)
                        {
                            if (neighb.DeviceNickName.Address() == stack::Gateway_Nickname())
                                p_rNotifDevCmd.m_devInfo.DeviceType = Device::dtAccessPoint;
                        }

                        neighb.Health.m_nClockSource = NeighborHealth::g_nNoClockSource;
                        neighb.Health.m_nFailedTransmissions = pwhResp->Neighbours[i].TransmittedPacketWithNoACKCount;
                        neighb.Health.m_nTransmissions = pwhResp->Neighbours[i].TransmittedPacketCount;
                        neighb.Health.m_nReceptions = pwhResp->Neighbours[i].ReceivedPacketCount;
                        neighb.Health.m_nRSL = pwhResp->Neighbours[i].NeighbourRSL;

                        LOG_DEBUG_APP("[NotificationDevInfo]: device[" << i << "] has nickName=" << neighb.DeviceNickName);
                    }
                    LOG_DEBUG_APP("[NotificationDevInfo]: neighb_list -> end");

                    if (pwhResp->NeighbourCount == MaxNeighbours)
                    {
                        C833_ReadNetworkDeviceNeighbourHealth_Req req;
                        memcpy(req.UniqueID, p_rNotifDevCmd.m_devInfo.DeviceMAC.Address().bytes, sizeof(req.UniqueID));
                        req.NeighbourIndex = pwhResp->NeighbourIndex + pwhResp->NeighbourCount;
                        req.NeighbourEntriesToRead = MaxNeighbours;
                        stack::WHartCommand whCmd;
                        whCmd.commandID = CMDID_C833_ReadNetworkDeviceNeighbourHealth;
                        whCmd.command = &req;

                        try
                        {
                            //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C833_ReadNetworkDeviceNeighbourHealth. Device=NM");
                            m_pProcessor->gateway.SendWHRequest(m_pResponse->appData, stack::NetworkManager_UniqueID(), whCmd, false, true);
                        }
                        catch (std::exception ex)
                        {
                            p_rNotifDevCmd.pDevices->DelDeviceInfoInProgress(p_rNotifDevCmd.m_devInfo.DeviceMAC);
                            throw ex;
                        }
                        return;
                    }
                }
                else
                {
                    LOG_WARN_APP("[NotificationDevInfo]: neighbours has failed!");
                    //if gw's disconnected, the command with multiple states should stop
                    p_rNotifDevCmd.pDevices->DelDeviceInfoInProgress(p_rNotifDevCmd.m_devInfo.DeviceMAC);
                    if (m_pResponse->hostErr == gateway::GatewayIO::HostDisconnected)
                    {
                        SaveRespErr(*p_rNotifDevCmd.pCommands, p_rNotifDevCmd.dbCommand).SaveErr_833(m_pResponse->hostErr,
                            m_pResponse->whCmd.responseCode);
                        return;
                    }
                }

                p_rNotifDevCmd.m_state = AppNotificationDevCmd::GetGraphs_state;
                C834_ReadNetworkTopologyInformation_Req req;
                memcpy(req.DeviceLongAddress, p_rNotifDevCmd.m_devInfo.DeviceMAC.Address().bytes, sizeof(req.DeviceLongAddress));
                req.GraphIndexNo = 0;
                stack::WHartCommand whCmd;
                whCmd.commandID = CMDID_C834_ReadNetworkTopologyInformation;
                whCmd.command = &req;

                try
                {
                    //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C834_ReadNetworkTopologyInformation. Device=NM");
                    m_pProcessor->gateway.SendWHRequest(m_pResponse->appData, stack::NetworkManager_UniqueID(), whCmd, false, true);
                }
                catch (std::exception ex)
                {
                    p_rNotifDevCmd.pDevices->DelDeviceInfoInProgress(p_rNotifDevCmd.m_devInfo.DeviceMAC);
                    throw ex;
                }
            }
            break;
            case AppNotificationDevCmd::GetGraphs_state:
            {
                if (!SaveRespErr::IsErr_834(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
                {

                    C834_ReadNetworkTopologyInformation_Resp *pwhResp =
                                (C834_ReadNetworkTopologyInformation_Resp*) m_pResponse->whCmd.command;
                    if (!pwhResp)
                    {
                        LOG_ERROR_APP("[NotificationDevInfo]: Incorrect response received for CMD834. DeviceMAC="
                                    << p_rNotifDevCmd.m_devInfo.DeviceMAC);
                        return;
                    }

                    if (pwhResp->GraphIndexNo == 0)
                        p_rNotifDevCmd.m_devInfo.GraphsList.reserve(pwhResp->TotalGraphsNo);

                    p_rNotifDevCmd.m_devInfo.GraphsList.push_back(TopologyReport::Device::Graph());
                    TopologyReport::Device::Graph &graphID = *p_rNotifDevCmd.m_devInfo.GraphsList.rbegin();
                    graphID.GraphID = pwhResp->IndexGraphId;
                    graphID.neighbList.reserve(pwhResp->NeighboursNo);
                    for (int i = 0; i < pwhResp->NeighboursNo; i++)
                    {
                        NickName nick(pwhResp->Neighbour[i]);
                        graphID.neighbList.push_back(nick);
                    }

                    if ((pwhResp->GraphIndexNo) < (pwhResp->TotalGraphsNo - 1))
                    {
                        C834_ReadNetworkTopologyInformation_Req req;
                        memcpy(req.DeviceLongAddress, p_rNotifDevCmd.m_devInfo.DeviceMAC.Address().bytes, sizeof(req.DeviceLongAddress));
                        req.GraphIndexNo = pwhResp->GraphIndexNo + 1;
                        stack::WHartCommand whCmd;
                        whCmd.commandID = CMDID_C834_ReadNetworkTopologyInformation;
                        whCmd.command = &req;

                        try
                        {
                            //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C834_ReadNetworkTopologyInformation. Device=NM");
                            m_pProcessor->gateway.SendWHRequest(m_pResponse->appData, stack::NetworkManager_UniqueID(), whCmd, false, true);
                        }
                        catch (std::exception ex)
                        {
                            p_rNotifDevCmd.pDevices->DelDeviceInfoInProgress(p_rNotifDevCmd.m_devInfo.DeviceMAC);
                            throw ex;
                        }
                        return;
                    }
                }
                else
                {
                    LOG_WARN_APP("[NotificationDevInfo]: graphs has failed!");
                    //if gw's disconnected, the command with multiple states should stop
                    p_rNotifDevCmd.pDevices->DelDeviceInfoInProgress(p_rNotifDevCmd.m_devInfo.DeviceMAC);
                    if (m_pResponse->hostErr == gateway::GatewayIO::HostDisconnected)
                    {
                        SaveRespErr(*p_rNotifDevCmd.pCommands, p_rNotifDevCmd.dbCommand).SaveErr_834(m_pResponse->hostErr,
                            m_pResponse->whCmd.responseCode);
                        return;
                    }
                }

                p_rNotifDevCmd.m_state = AppNotificationDevCmd::GetNickname_state;
                C832_ReadNetworkDeviceIdentity_Req req;
                memcpy(req.DeviceUniqueID, p_rNotifDevCmd.m_devInfo.DeviceMAC.Address().bytes, sizeof(req.DeviceUniqueID));
                stack::WHartCommand whCmd;
                whCmd.commandID = CMDID_C832_ReadNetworkDeviceIdentity;
                whCmd.command = &req;

                try
                {
                    //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C832_ReadNetworkDeviceIdentity. Device=NM");
                    m_pProcessor->gateway.SendWHRequest(m_pResponse->appData, stack::NetworkManager_UniqueID(), whCmd, false, true);
                }
                catch (std::exception ex)
                {
                    p_rNotifDevCmd.pDevices->DelDeviceInfoInProgress(p_rNotifDevCmd.m_devInfo.DeviceMAC);
                    throw ex;
                }
            }
            break;
            case AppNotificationDevCmd::GetNickname_state:
            {
                if (!SaveRespErr::IsErr_832(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
                {

                    C832_ReadNetworkDeviceIdentity_Resp *pwhResp = (C832_ReadNetworkDeviceIdentity_Resp*) m_pResponse->whCmd.command;
                    if (!pwhResp)
                    {
                        LOG_ERROR_APP("[NotificationDevInfo]: Incorrect response received for CMD832. DeviceMAC="
                                    << p_rNotifDevCmd.m_devInfo.DeviceMAC);
                        return;
                    }

                    p_rNotifDevCmd.m_devInfo.DeviceNickName = NickName(pwhResp->Nickname);

                    std::string longTag;
                    for (unsigned int i = 0; i < sizeof(pwhResp->LongTag); ++i)
                        longTag += ConvertToHex((unsigned char) pwhResp->LongTag[i]);

                    p_rNotifDevCmd.m_devInfo.LongTag = longTag;

                    LOG_DEBUG_APP("[NotificationDevInfo]: nickname received = " << p_rNotifDevCmd.m_devInfo.DeviceNickName);
                }
                else
                {
                    LOG_ERROR_APP("[NotificationDevInfo]: nickname has failed!");
                    p_rNotifDevCmd.pDevices->DelDeviceInfoInProgress(p_rNotifDevCmd.m_devInfo.DeviceMAC);
                    return;
                }

                p_rNotifDevCmd.m_state = AppNotificationDevCmd::GetInfo_state;

                C000_ReadUniqueIdentifier_Req req;

                stack::WHartCommand whCmd;
                whCmd.commandID = CMDID_C000_ReadUniqueIdentifier;
                whCmd.command = &req;
                try
                {
                    //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C000_ReadUniqueIdentifier. Device=" << p_rNotifDevCmd.m_devInfo.DeviceMAC);
                    m_pProcessor->gateway.SendWHRequest(m_pResponse->appData, p_rNotifDevCmd.m_devInfo.DeviceMAC.Address(), whCmd, false, true);
                }
                catch (std::exception ex)
                {
                    p_rNotifDevCmd.pDevices->DelDeviceInfoInProgress(p_rNotifDevCmd.m_devInfo.DeviceMAC);
                    throw ex;
                }
            }
            break;
            case AppNotificationDevCmd::GetInfo_state:
            {
                if (!SaveRespErr::IsErr_000(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
                {
                    C000_ReadUniqueIdentifier_Resp *pwhResp = (C000_ReadUniqueIdentifier_Resp*) m_pResponse->whCmd.command;
                    if (!pwhResp)
                    {
                        LOG_ERROR_APP("[NotificationDevInfo]: Incorrect response received for CMD000. DeviceMAC="
                                    << p_rNotifDevCmd.m_devInfo.DeviceMAC);
                        return;
                    }

                    p_rNotifDevCmd.m_devInfo.softwareRev = pwhResp->softwareRevisionLevel;
                    p_rNotifDevCmd.m_devInfo.deviceCode = pwhResp->expandedDeviceType;

                }
                else
                {
                    LOG_ERROR_APP("[NotificationDevInfo]: command_0 has failed!");
                    p_rNotifDevCmd.pDevices->DelDeviceInfoInProgress(p_rNotifDevCmd.m_devInfo.DeviceMAC);
                    if (m_pResponse->hostErr == gateway::GatewayIO::HostDisconnected)
                    {
                        SaveRespErr(*p_rNotifDevCmd.pCommands, p_rNotifDevCmd.dbCommand).SaveErr_000(m_pResponse->hostErr,
                            m_pResponse->whCmd.responseCode);
                        return;
                    }
                }

                stack::WHartCommand whCmd;

                C837_WriteUpdateNotificationBitMask_Req whReq;

                bool subscribe = false;
                memcpy(whReq.UniqueID, p_rNotifDevCmd.m_devInfo.DeviceMAC.Address().bytes, sizeof(WHartUniqueID));

                DevicePtr dev;
                if (!(dev = p_rNotifDevCmd.pDevices->FindDevice(p_rNotifDevCmd.m_devInfo.DeviceMAC)))
                {
                    subscribe = true;
                    p_rNotifDevCmd.m_devInfo.notifBitMask = NotificationMaskCodesMask_DeviceConfiguration;
                    whReq.ChangeNotificationFlags = NotificationMaskCodesMask_DeviceConfiguration;
                    p_rNotifDevCmd.pDevices->AddNewDeviceInfo(p_rNotifDevCmd.m_devInfo);
                }
                else
                {
                    LOG_WARN("[NotificationDevInfo]: already device added in db");
                    unsigned short notifMask = dev->GetNotificationBitMask();
                    if (!(notifMask & NotificationMaskCodesMask_DeviceConfiguration))
                    {
                        subscribe = true;
                        notifMask = notifMask | NotificationMaskCodesMask_DeviceConfiguration;
                        dev->SetNotificationBitMask(notifMask);
                        whReq.ChangeNotificationFlags = notifMask;
                    }

                }
                p_rNotifDevCmd.pDevices->DelDeviceInfoInProgress(p_rNotifDevCmd.m_devInfo.DeviceMAC);

                if (subscribe)
                {
                    whCmd.commandID = CMDID_C837_WriteUpdateNotificationBitMask;
                    whCmd.command = &whReq;
                    m_pResponse->appData.appCmd = AbstractAppCommandPtr(new AppSetNotificationBitMask(p_rNotifDevCmd.m_devInfo.DeviceMAC,
                        p_rNotifDevCmd.m_devInfo.notifBitMask));
                    //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C837_WriteUpdateNotificationBitMask. Device=GW");
                    m_pProcessor->gateway.SendWHRequest(m_pResponse->appData, stack::Gateway_UniqueID(), whCmd, false, true);
                }
            }

            break;
            default:
                assert(false);
        }
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Visit::AppNotificationDevCmd]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Visit::AppNotificationDevCmd]: System error!");
    }
}

void ResponseProcessor::Visit(AppReportsNotification& p_rReportsNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Visit]:AppReportsNotification. Device=");
    try
    {
        if (SaveRespErr::IsErr_839(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            return;
        }
        C839_ChangeNotification_Resp* pwhResp = (C839_ChangeNotification_Resp*) m_pResponse->whCmd.command;
        if (!pwhResp)
        {
            LOG_ERROR_APP("[Notification-839]: Incorrect response received for CMD839.");
            return;
        }

//        LOG_INFO_APP("[ReportsNotification]: received for device=" << MAC(pwhResp->DeviceAddress));
        int cmds = 0;
        for (cmds = 0; cmds < pwhResp->ChangeNotificationNo; cmds++)
        {
            switch (pwhResp->ChangeNotifications[cmds])
            {
                case CMDID_C779_ReportDeviceHealth:
                {
                    LOG_INFO_APP("[Notification-779]: with device=" << MAC(pwhResp->DeviceAddress));

                    DevicePtr dev;
                    MAC devMac = MAC(pwhResp->DeviceAddress);
                    if (!(dev = p_rReportsNotification.pDevices->FindDevice(devMac)))
                    {
                        LOG_WARN_APP("[Notification-779]: with device=" << MAC(pwhResp->DeviceAddress) << " not found in db!");
                        continue;
                    }

                    C779_ReportDeviceHealth_Req whReq;

                    stack::WHartCommand whCmd;
                    whCmd.commandID = CMDID_C779_ReportDeviceHealth;
                    whCmd.command = &whReq;

                    gateway::GatewayIO::AppData appData;
                    std::list<std::pair<int, MAC> > list;
                    list.push_back(std::pair<int, MAC>(dev->id, devMac));
                    appData.appCmd = AbstractAppCommandPtr(new AppDeviceHealthReportCmd(list));
                    appData.appCmd->pCommands = p_rReportsNotification.pCommands;
                    appData.appCmd->pDevices = p_rReportsNotification.pDevices;
                    appData.innerDataHandle = 0;

                    //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C779_ReportDeviceHealth. Device=" << devMac);
                    m_pProcessor->gateway.SendWHRequest(appData, devMac.Address(), whCmd);
                    break;
                }
                case CMDID_C780_ReportNeighborHealthList:
                {
                    LOG_INFO_APP("[Notification-780]: with device=" << MAC(pwhResp->DeviceAddress));

                    DevicePtr dev;
                    MAC devMac = MAC(pwhResp->DeviceAddress);
                    if (!(dev = p_rReportsNotification.pDevices->FindDevice(devMac)))
                    {
                        LOG_WARN_APP("[Notification-780]: with device=" << MAC(pwhResp->DeviceAddress) << " not found in db!");
                        continue;
                    }

                    C780_ReportNeighborHealthList_Req whReq;
                    whReq.m_ucNeighborTableIndex = 0;
                    whReq.m_ucNoOfNeighborEntriesToRead = C780_MAX_NEIGHBORS_LIST;

                    stack::WHartCommand whCmd;
                    whCmd.commandID = CMDID_C780_ReportNeighborHealthList;
                    whCmd.command = &whReq;

                    gateway::GatewayIO::AppData appData;
                    std::list<std::pair<int, MAC> > list;
                    list.push_back(std::pair<int, MAC>(dev->id, devMac));
                    appData.appCmd = AbstractAppCommandPtr(new AppNeighborHealthReportCmd(list));
                    appData.appCmd->pCommands = p_rReportsNotification.pCommands;
                    appData.appCmd->pDevices = p_rReportsNotification.pDevices;
                    appData.innerDataHandle = 0;

                    //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C780_ReportNeighborHealthList. Device=" << devMac);
                    m_pProcessor->gateway.SendWHRequest(m_pResponse->appData, devMac.Address(), whCmd);
                    break;
                }
                case CMDID_C783_ReadSuperframeList:
                {
                    LOG_INFO_APP("[Notification-783]: with device=" << MAC(pwhResp->DeviceAddress));
                    C783_ReadSuperframeList_Req whReq;
                    whReq.m_ucSuperframeIndex = 0;
                    whReq.m_ucNoOfEntriesToRead = C783_MAX_SUPERFRAMES_LIST;

                    stack::WHartCommand whCmd;
                    whCmd.commandID = CMDID_C783_ReadSuperframeList;
                    whCmd.command = &whReq;

                    MAC devMac = MAC(pwhResp->DeviceAddress);

                    DevicePtr dev;
                    if (!(dev = p_rReportsNotification.pDevices->FindDevice(devMac)))
                    {
                        LOG_WARN_APP("[Notification-783]: with device=" << MAC(pwhResp->DeviceAddress) << " not found in db!");
                        continue;
                    }
                    gateway::GatewayIO::AppData appData;
                    appData.appCmd = AbstractAppCommandPtr(new AppSuperframesReportCmd(1));
                    appData.appCmd->pCommands = p_rReportsNotification.pCommands;
                    appData.appCmd->pDevices = p_rReportsNotification.pDevices;
                    appData.innerDataHandle = dev->id;
                    //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C783_ReadSuperframeList. Device=" << devMac);
                    m_pProcessor->gateway.SendWHRequest(appData, devMac.Address(), whCmd);
                    break;
                }
                case CMDID_C784_ReadLinkList:
                {
                    LOG_INFO_APP("[Notification-784]: with device=" << MAC(pwhResp->DeviceAddress));
                    C784_ReadLinkList_Req whReq;
                    whReq.m_unLinkIndex = 0;
                    whReq.m_ucNoOfLinksToRead = C784_MAX_LINKS_LIST;

                    stack::WHartCommand whCmd;
                    whCmd.commandID = CMDID_C784_ReadLinkList;
                    whCmd.command = &whReq;

                    MAC devMac = MAC(pwhResp->DeviceAddress);

                    DevicePtr dev;
                    if (!(dev = p_rReportsNotification.pDevices->FindDevice(devMac)))
                    {
                        LOG_WARN_APP("[Notification-784]: with device=" << MAC(pwhResp->DeviceAddress) << " not found in db!");
                        continue;
                    }
                    gateway::GatewayIO::AppData appData;
                    std::list<std::pair<int, MAC> > l;
                    l.push_back(std::pair<int, MAC>(dev->id, dev->Mac()));
                    appData.appCmd = AbstractAppCommandPtr(new AppDeviceScheduleLinksReportCmd(l));
                    appData.appCmd->pCommands = p_rReportsNotification.pCommands;
                    appData.appCmd->pDevices = p_rReportsNotification.pDevices;
                    appData.innerDataHandle = 0;
                    //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C784_ReadLinkList. Device=" << devMac);
                    m_pProcessor->gateway.SendWHRequest(appData, devMac.Address(), whCmd);
                    break;
                }
                case CMDID_C787_ReportNeighborSignalLevels:
                {
                    LOG_INFO_APP("[Notification-787]: with device=" << MAC(pwhResp->DeviceAddress));
                    C787_ReportNeighborSignalLevels_Req whReq;
                    whReq.m_ucNeighborTableIndex = 0;
                    whReq.m_ucNoOfNeighborEntriesToRead = C787_MAX_NEIGHBORS_LIST;

                    stack::WHartCommand whCmd;
                    whCmd.commandID = CMDID_C787_ReportNeighborSignalLevels;
                    whCmd.command = &whReq;

                    MAC devMac = MAC(pwhResp->DeviceAddress);

                    DevicePtr dev;
                    if (!(dev = p_rReportsNotification.pDevices->FindDevice(devMac)))
                    {
                        LOG_WARN_APP("[Notification-787]: with device=" << devMac << " not found in db!");
                        continue;
                    }
                    gateway::GatewayIO::AppData appData;
                    appData.appCmd = AbstractAppCommandPtr(new App787Cmd(dev->id, devMac));
                    appData.appCmd->pCommands = p_rReportsNotification.pCommands;
                    appData.appCmd->pDevices = p_rReportsNotification.pDevices;
                    //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C787_ReportNeighborSignalLevels. Device=" << devMac);
                    m_pProcessor->gateway.SendWHRequest(appData, devMac.Address(), whCmd);
                    break;
                }
                case CMDID_C788_AlarmPathDown:
                {
                    LOG_INFO_APP("[Notification-788]: with device=" << MAC(pwhResp->DeviceAddress));
                    C788_AlarmPathDown_Req whReq;

                    stack::WHartCommand whCmd;
                    whCmd.commandID = CMDID_C788_AlarmPathDown;
                    whCmd.command = &whReq;

                    MAC devMac = MAC(pwhResp->DeviceAddress);

                    DevicePtr dev;
                    if (!(dev = p_rReportsNotification.pDevices->FindDevice(devMac)))
                    {
                        LOG_WARN_APP("[Notification-788]: with device=" << devMac << " not found in db!");
                        continue;
                    }
                    gateway::GatewayIO::AppData appData;
                    appData.appCmd = AbstractAppCommandPtr(new App788Cmd(dev->id));
                    appData.appCmd ->pCommands = p_rReportsNotification.pCommands;
                    appData.appCmd->pDevices = p_rReportsNotification.pDevices;
                    //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C788_AlarmPathDown. Device=" << devMac);
                    m_pProcessor->gateway.SendWHRequest(appData, devMac.Address(), whCmd);
                    break;
                }
                case CMDID_C789_AlarmSourceRouteFailed:
                {
                    LOG_INFO_APP("[Notification-789]: with device=" << MAC(pwhResp->DeviceAddress));
                    C789_AlarmSourceRouteFailed_Req whReq;

                    stack::WHartCommand whCmd;
                    whCmd.commandID = CMDID_C789_AlarmSourceRouteFailed;
                    whCmd.command = &whReq;

                    MAC devMac = MAC(pwhResp->DeviceAddress);

                    DevicePtr dev;
                    if (!(dev = p_rReportsNotification.pDevices->FindDevice(devMac)))
                    {
                        LOG_WARN_APP("[Notification-789]: with device=" << devMac << " not found in db!");
                        continue;
                    }
                    gateway::GatewayIO::AppData appData;
                    appData.appCmd = AbstractAppCommandPtr(new App789Cmd(dev->id));
                    appData.appCmd ->pCommands = p_rReportsNotification.pCommands;
                    appData.appCmd->pDevices = p_rReportsNotification.pDevices;
                    //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C789_AlarmSourceRouteFailed. Device=" << devMac);
                    m_pProcessor->gateway.SendWHRequest(appData, devMac.Address(), whCmd);
                    break;
                }
                case CMDID_C790_AlarmGraphRouteFailed:
                {
                    LOG_INFO_APP("[Notification-790]: with device=" << MAC(pwhResp->DeviceAddress));
                    C790_AlarmGraphRouteFailed_Req whReq;

                    stack::WHartCommand whCmd;
                    whCmd.commandID = CMDID_C790_AlarmGraphRouteFailed;
                    whCmd.command = &whReq;

                    MAC devMac = MAC(pwhResp->DeviceAddress);

                    DevicePtr dev;
                    if (!(dev = p_rReportsNotification.pDevices->FindDevice(devMac)))
                    {
                        LOG_WARN_APP("[Notification-790]: with device=" << devMac << " not found in db!");
                        continue;
                    }
                    gateway::GatewayIO::AppData appData;
                    appData.appCmd = AbstractAppCommandPtr(new App790Cmd(dev->id));
                    appData.appCmd ->pCommands = p_rReportsNotification.pCommands;
                    appData.appCmd->pDevices = p_rReportsNotification.pDevices;
                    //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C790_AlarmGraphRouteFailed. Device=" << devMac);
                    m_pProcessor->gateway.SendWHRequest(appData, devMac.Address(), whCmd);
                    break;
                }
                case CMDID_C791_AlarmTransportLayerFailed:
                {
                    LOG_INFO_APP("[Notification-791]: with device=" << MAC(pwhResp->DeviceAddress));
                    C791_AlarmTransportLayerFailed_Req whReq;

                    stack::WHartCommand whCmd;
                    whCmd.commandID = CMDID_C791_AlarmTransportLayerFailed;
                    whCmd.command = &whReq;

                    MAC devMac = MAC(pwhResp->DeviceAddress);

                    DevicePtr dev;
                    if (!(dev = p_rReportsNotification.pDevices->FindDevice(devMac)))
                    {
                        LOG_WARN_APP("[Notification-791]: with device=" << devMac << " not found in db!");
                        continue;
                    }
                    gateway::GatewayIO::AppData appData;
                    appData.appCmd = AbstractAppCommandPtr(new App791Cmd(dev->id));
                    appData.appCmd ->pCommands = p_rReportsNotification.pCommands;
                    appData.appCmd->pDevices = p_rReportsNotification.pDevices;
                    //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C791_AlarmTransportLayerFailed. Device=" << devMac);
                    m_pProcessor->gateway.SendWHRequest(appData, devMac.Address(), whCmd);
                    break;
                }
                case CMDID_C800_ReadServiceList:
                {
                    LOG_INFO_APP("[Notification-800]: with device=" << MAC(pwhResp->DeviceAddress));
                    C800_ReadServiceList_Req whReq;
                    whReq.m_ucServiceIndex = 0;
                    whReq.m_ucNoOfEntriesToRead = C800_MAX_SERVICES_LIST;

                    stack::WHartCommand whCmd;
                    whCmd.commandID = CMDID_C800_ReadServiceList;
                    whCmd.command = &whReq;

                    MAC devMac = MAC(pwhResp->DeviceAddress);

                    DevicePtr dev;
                    if (!(dev = p_rReportsNotification.pDevices->FindDevice(devMac)))
                    {
                        LOG_WARN_APP("[Notification-800]: with device=" << MAC(pwhResp->DeviceAddress) << " not found in db!");
                        continue;
                    }
                    gateway::GatewayIO::AppData appData;
                    appData.appCmd = AbstractAppCommandPtr(new AppServicesReportCmd(1));
                    appData.appCmd->pCommands = p_rReportsNotification.pCommands;
                    appData.appCmd->pDevices = p_rReportsNotification.pDevices;
                    appData.innerDataHandle = dev->id;
                    //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C800_ReadServiceList. Device=" << devMac);
                    m_pProcessor->gateway.SendWHRequest(appData, devMac.Address(), whCmd);
                    break;
                }
                case CMDID_C803_ReadSourceRoute:
                case CMDID_C802_ReadRouteList:
                {
                    LOG_INFO_APP("[Notification-802or803]: with device=" << MAC(pwhResp->DeviceAddress));
                    C802_ReadRouteList_Req whReq;
                    whReq.m_ucRouteIndex = 0;
                    whReq.m_ucNoOfEntriesToRead = C802_MAX_ROUTES_LIST;

                    stack::WHartCommand whCmd;
                    whCmd.commandID = CMDID_C802_ReadRouteList;
                    whCmd.command = &whReq;

                    MAC devMac = MAC(pwhResp->DeviceAddress);

                    DevicePtr dev;
                    if (!(dev = p_rReportsNotification.pDevices->FindDevice(devMac)))
                    {
                        LOG_WARN_APP("[Notification-802or803]: with device=" << MAC(pwhResp->DeviceAddress) << " not found in db!");
                        continue;
                    }
                    gateway::GatewayIO::AppData appData;
                    appData.appCmd = AbstractAppCommandPtr(new AppRoutesReportCmd(1));
                    appData.appCmd->pCommands = p_rReportsNotification.pCommands;
                    appData.appCmd->pDevices = p_rReportsNotification.pDevices;
                    appData.innerDataHandle = dev->id;
                    //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C802_ReadRouteList. Device=" << devMac);
                    m_pProcessor->gateway.SendWHRequest(appData, devMac.Address(), whCmd);
                    break;
                }
                default:
                    LOG_WARN_APP("[ReportsNotification]: with device=" << MAC(pwhResp->DeviceAddress) << " and cmdNo="
                                << (int) pwhResp->ChangeNotifications[cmds]);
                break;
            }
        }
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Visit::AppReportsNotification]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Visit::AppReportsNotification]: System error!");
    }
}

void ResponseProcessor::Visit(AppDevConfigNotification& p_rDevConfigNotification)
{
    //LOG_INFO_APP("[ResponseProcessor::Visit]:AppDevConfigNotification. Device=");
    try
    {
        if (SaveRespErr::IsErr_839(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
            return;
        C839_ChangeNotification_Resp *pwhResp = (C839_ChangeNotification_Resp*) m_pResponse->whCmd.command;
        if (!pwhResp)
        {
            LOG_ERROR_APP("[DevConfigNotification]: Incorrect response received for CMD839.");
            return;
        }

//        LOG_INFO_APP("[DevConfigNotification]: received for device=" << MAC(pwhResp->DeviceAddress));
        for (int cmds = 0; cmds < pwhResp->ChangeNotificationNo; cmds++)
        {
            if (pwhResp->ChangeNotifications[cmds] == CMDID_C000_ReadUniqueIdentifier)
            {
                C000_ReadUniqueIdentifier_Req req;
                stack::WHartCommand whCmd;
                whCmd.commandID = CMDID_C000_ReadUniqueIdentifier;
                whCmd.command = &req;

                gateway::GatewayIO::AppData appData;
                MAC devMac = MAC(pwhResp->DeviceAddress);
                DevicePtr dev;
                if (!(dev = p_rDevConfigNotification.pDevices->FindDevice(devMac)))
                {
                    LOG_WARN_APP("[DevConfigNotification]: with 0 cmd and device(not in db)=" << devMac << ", so skip sending cmd");
                    continue;
                }

                LOG_INFO_APP("[DevConfigNotification]: with 0 cmd and device(in db)=" << devMac);
                appData.appCmd = AbstractAppCommandPtr(new App000Cmd(dev));
                appData.appCmd->pCommands = p_rDevConfigNotification.pCommands;
                appData.appCmd->pDevices = p_rDevConfigNotification.pDevices;
                //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C000_ReadUniqueIdentifier. Device=" << devMac);
                m_pProcessor->gateway.SendWHRequest(appData, devMac.Address(), whCmd);
                continue;
            }

            if (pwhResp->ChangeNotifications[cmds] == CMDID_C020_ReadLongTag)
            {
                C020_ReadLongTag_Req req;
                stack::WHartCommand whCmd;
                whCmd.commandID = CMDID_C020_ReadLongTag;
                whCmd.command = &req;

                gateway::GatewayIO::AppData appData;
                MAC devMac = MAC(pwhResp->DeviceAddress);
                DevicePtr dev;
                if (!(dev = p_rDevConfigNotification.pDevices->FindDevice(devMac)))
                {
                    LOG_WARN_APP("[DevConfigNotification]: with 20 cmd and device(not in db)=" << devMac << ", so skip sending cmd");
                    continue;
                }

                LOG_INFO_APP("[DevConfigNotification]: with 20 cmd and device(in db)=" << devMac);
                appData.appCmd = AbstractAppCommandPtr(new App020Cmd(dev));
                appData.appCmd->pCommands = p_rDevConfigNotification.pCommands;
                appData.appCmd->pDevices = p_rDevConfigNotification.pDevices;

                //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C020_ReadLongTag. Device=" << devMac);
                m_pProcessor->gateway.SendWHRequest(appData, devMac.Address(), whCmd);
                continue;
            }
        }
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Visit::AppDevConfigNotification]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Visit::AppDevConfigNotification]: System error!");
    }
}

void ResponseProcessor::Visit(AppBurstNotification& p_rBurstNotification)
{
    try
    {
        if (SaveRespErr::IsErr_839(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
            return;

        //send read cmds
        C839_ChangeNotification_Resp *pwhResp = (C839_ChangeNotification_Resp*) (m_pResponse->whCmd.command);
        if (!pwhResp)
        {
            LOG_ERROR_APP("[BurstNotification]: Incorrect response received for CMD839.");
            return;
        }

        WHartUniqueID devAddr;
        memcpy(devAddr.bytes, pwhResp->DeviceAddress, sizeof(WHartUniqueID));

        PublishChannelSetT subDeviceChannels;
        bool isSubDevice = false;
        DevicePtr devPtr = p_rBurstNotification.pDevices->FindDevice(pwhResp->DeviceAddress);
        if (!devPtr)
        {
            BurstMessageSetT::iterator itBurst = p_rBurstNotification.m_rBList.begin();
            PublishChannelSetT::iterator itChannel = p_rBurstNotification.m_rList.begin();

            for (; itBurst != p_rBurstNotification.m_rBList.end(); ++itBurst)
            {
                WHartUniqueID whID = itBurst->subDeviceMAC.Address();
                if (memcmp(whID.bytes, pwhResp->DeviceAddress, 5) == 0)
                {
                    for (; itChannel != p_rBurstNotification.m_rList.end(); ++itChannel)
                    {
                        if (itChannel->cmdNo == itBurst->cmdNo && itChannel->burstMessage == itBurst->burstMessage)
                        {
                            subDeviceChannels.insert((*itChannel));
                        }
                    }
                }
            }

            if (subDeviceChannels.size() <= 0)
            {
                LOG_ERROR_APP("[BurstNotification]: contains an 839 cmd with an invalid device/sub-device address");
                return;
            }
            else
            {
                isSubDevice = true;
            }
        }

        if (!isSubDevice && !devPtr)
        {
            LOG_WARN_APP("[BurstNotification]: Received notification for invalid device ");
            return;
        }

        LOG_DEBUG_APP("[BurstNotification]: Received Burst Notification for device " << devAddr << " and "
                    << ((int) pwhResp->ChangeNotificationNo) << " Change Notifications");

        CMicroSec recvTime;
        if (isSubDevice)
        {
            p_rBurstNotification.pDevices->PubNotificationReceived(p_rBurstNotification.m_deviceID, recvTime, *pwhResp);
        }
        else
        {
            p_rBurstNotification.pDevices->PubNotificationReceived(devPtr->id, recvTime, *pwhResp);
        }

        for (int i = 0; i < pwhResp->ChangeNotificationNo; i++)
        {
            uint16_t cmdNo = pwhResp->ChangeNotifications[i];

            PublishChannelSetT::iterator first;
            if (isSubDevice) // sub-device
                first = FindFirstChannel(subDeviceChannels, cmdNo);
            else
                // device
                first = FindFirstChannel(p_rBurstNotification.GetChannelList(), cmdNo);

            PublishChannelSetT::iterator last = first;

            stack::WHartCommand whCmd;
            whCmd.commandID = cmdNo;
            whCmd.command = BuildBurstValueCmdRequest(p_rBurstNotification.GetChannelList(), last/*[in/out]*/);

            if (whCmd.command != 0)
            {
                PublishChannelSetT channelSet(first, last);
                AppReadValueCmd* readCmd;
                if (isSubDevice) // sub-device
                    readCmd = new AppReadValueCmd(cmdNo, p_rBurstNotification.m_deviceID, DeviceReading::BurstValue, channelSet);
                else
                    // device
                    readCmd = new AppReadValueCmd(cmdNo, devPtr->id, DeviceReading::BurstValue, channelSet);

                gateway::GatewayIO::AppData appData;
                appData.appCmd = AbstractAppCommandPtr(readCmd);
                appData.appCmd->pCommands = p_rBurstNotification.pCommands;
                appData.appCmd->pDevices = p_rBurstNotification.pDevices;
                LOG_DEBUG_APP("[BurstNotification]: Sending Read Value Request with cmdNo=" << (int) cmdNo << " for dev=" << devAddr);
                //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:" << cmdNo << ". Device=" << MAC(devAddr.bytes));
                m_pProcessor->gateway.SendWHRequest(appData, devAddr, whCmd, false, true);
            }
        }
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Visit::AppBurstNotification]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Visit::AppBurstNotification]: System error!");
    }
}

//burst notif
void ResponseProcessor::Visit(AppSetBurstNotificationCmd& p_rBurstNotificationCmd)
{
    //LOG_INFO_APP("[ResponseProcessor::Visit]:AppSetBurstNotificationCmd. Device=");
    try
    {
        DevicePtr device = p_rBurstNotificationCmd.pDevices->FindDevice(p_rBurstNotificationCmd.dbCommand.deviceID);
        if (!device)
        {
            LOG_ERROR_APP("[SetBurstNotification]: device id=" << device->id << "not found");
            SaveRespErr(*p_rBurstNotificationCmd.pCommands, p_rBurstNotificationCmd.dbCommand).CommandFailed(DBCommand::rsFailure_InvalidDevice);
            return;
        }

        if (m_pResponse->whCmd.commandID == CMDID_C837_WriteUpdateNotificationBitMask) {
            device->IssueSetBurstNotificationCmd = true;

            if (SaveRespErr::IsErr_837(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
            {
                device->SetHasNotification(false);
                SaveRespErr(*p_rBurstNotificationCmd.pCommands, p_rBurstNotificationCmd.dbCommand).SaveErr_837(m_pResponse->hostErr, m_pResponse->whCmd.responseCode);
                return;
            }

            //we anticipated the result in requestProcessor
            device->SetHasNotification(true);

            AppBurstNotificationPtr burstNotificationPtr(new AppBurstNotification(device->id, p_rBurstNotificationCmd.GetChannelList(),
                p_rBurstNotificationCmd.GetBurstMessageList()));
            burstNotificationPtr->pCommands = p_rBurstNotificationCmd.pCommands;
            burstNotificationPtr->pDevices = p_rBurstNotificationCmd.pDevices;

            if (device->IsAdapter())
            {
                BurstMessageSetT &list = (BurstMessageSetT&) p_rBurstNotificationCmd.GetBurstMessageList();
                BurstMessageSetT::iterator i = list.begin();
                hart7::hostapp::MAC macEmpty(0);
                for (; i != list.end(); ++i)
                {
                    if (!(i->subDeviceMAC == macEmpty))
                        m_pProcessor->gateway.RegisterBurstMsgFromDev(burstNotificationPtr, i->subDeviceMAC.Address());
                }
            }

            //there should be also registered the adapter itself
            m_pProcessor->gateway.RegisterBurstMsgFromDev(burstNotificationPtr, device->Mac().Address());
            p_rBurstNotificationCmd.pCommands->SetCommandResponded(p_rBurstNotificationCmd.dbCommand, nlib::CurrentUniversalTime(), "success", NULL);
        }
        else if (m_pResponse->whCmd.commandID == CMDID_C050_ReadDynamicVariableAssignments)
        {
            if (m_pResponse->whCmd.responseCode == RCS_E64_CommandNotImplemented)
            {
                LOG_ERROR_APP("[SetBurstNotification]: COMMAND50 - Not implemented by deviceMAC=" << device->Mac());
                device->SetDynamicVariableAssignment(DeviceVariableCodes_PrimaryVariable, DeviceVariableCodes_PrimaryVariable);
                device->SetDynamicVariableAssignment(DeviceVariableCodes_SecondaryVariable, DeviceVariableCodes_SecondaryVariable);
                device->SetDynamicVariableAssignment(DeviceVariableCodes_TertiaryVariable, DeviceVariableCodes_TertiaryVariable);
                device->SetDynamicVariableAssignment(DeviceVariableCodes_QuaternaryVariable, DeviceVariableCodes_QuaternaryVariable);
            }
            else
            {
                C050_ReadDynamicVariableAssignments_Resp *pwhResp = (C050_ReadDynamicVariableAssignments_Resp*) (m_pResponse->whCmd.command);
                if (!pwhResp)
                {
                    LOG_ERROR_APP("[SetBurstNotification]: COMMAND50 - Incorrect response received for CMD 50.");
                    return;
                }
                device->SetDynamicVariableAssignment(DeviceVariableCodes_PrimaryVariable, pwhResp->variables[0]);
                device->SetDynamicVariableAssignment(DeviceVariableCodes_SecondaryVariable, pwhResp->variables[1]);
                device->SetDynamicVariableAssignment(DeviceVariableCodes_TertiaryVariable, pwhResp->variables[2]);
                device->SetDynamicVariableAssignment(DeviceVariableCodes_QuaternaryVariable, pwhResp->variables[3]);
                LOG_INFO_APP("[SetBurstNotification]: COMMAND50 - DynamicVariableAssignments[" << device->Mac() << "] = {" <<
                        "246=" << (int)pwhResp->variables[0] << ", " <<
                        "247=" << (int)pwhResp->variables[1] << ", " <<
                        "248=" << (int)pwhResp->variables[2] << ", " <<
                        "249=" << (int)pwhResp->variables[3] << "}");
            }
        }
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Visit::AppSetBurstNotificationCmd]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Visit::AppSetBurstNotificationCmd]: System error!");
    }
}

void ResponseProcessor::Visit(AppUnsetBurstNotificationCmd& p_rBurstNotificationCmd)
{
    //LOG_INFO_APP("[ResponseProcessor::Visit]:AppUnsetBurstNotificationCmd. Device=");
    try
    {
        DevicePtr dev = p_rBurstNotificationCmd.pDevices->FindDevice(p_rBurstNotificationCmd.dbCommand.deviceID);
        if (!dev)
        {
            LOG_ERROR_APP("[UnsetburstNotification]: device id=" << p_rBurstNotificationCmd.dbCommand.deviceID << "not found");
            SaveRespErr(*p_rBurstNotificationCmd.pCommands, p_rBurstNotificationCmd.dbCommand).CommandFailed(
                DBCommand::rsFailure_InvalidDevice);
            return;
        }

        dev->SetHasNotification(false);
        m_pProcessor->gateway.UnRegisterBurstMsgFromDev(dev->Mac().Address());
        p_rBurstNotificationCmd.pDevices->RemoveBursts(dev->id);
        p_rBurstNotificationCmd.pDevices->PubDeviceDeletionReceived(dev->id);

        if (SaveRespErr::IsErr_837(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            SaveRespErr(*p_rBurstNotificationCmd.pCommands, p_rBurstNotificationCmd.dbCommand).SaveErr_837(m_pResponse->hostErr,
                m_pResponse->whCmd.responseCode);
            return;
        }
        p_rBurstNotificationCmd.pCommands->SetCommandResponded(p_rBurstNotificationCmd.dbCommand, nlib::CurrentUniversalTime(), "success", NULL);
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Visit::AppUnsetBurstNotificationCmd]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Visit::AppUnsetBurstNotificationCmd]: System error!");
    }
}

//burst config
void ResponseProcessor::Visit(AppSetBurstConfigurationCmd& p_rBurstConfig)
{
    //check for device
    //LOG_INFO_APP("[ResponseProcessor::Visit]:AppSetBurstConfigurationCmd. Device=");
    try
    {
        DevicePtr dev = p_rBurstConfig.GetDevice();
        if (dev == 0)
        {
            LOG_ERROR_APP("[SetBurstConfiguration]: device id=" << p_rBurstConfig.dbCommand.deviceID << "not found");
            SaveRespErr(*p_rBurstConfig.pCommands, p_rBurstConfig.dbCommand).CommandFailed(DBCommand::rsFailure_InvalidDevice);
            return;
        }

        //is it cancelled?
        if (dev->configBurstDBCmdID != p_rBurstConfig.dbCommand.commandID)
        {
            //DO NOT SET FLAG ON DEV BECAUSE IT SET WHEN THE CANCELLED STATE WAS SET
            //dev->IssueSetBurstConfigurationCmd = true;

            LOG_WARN_APP("[SetBurstConfiguration]: db commandID=" << p_rBurstConfig.dbCommand.commandID << " (!= "
                        << dev->configBurstDBCmdID << ") was cancelled, so configuration was stopped");
            SaveRespErr(*p_rBurstConfig.pCommands, p_rBurstConfig.dbCommand).CommandFailed(DBCommand::rsFailure_CommandWasCancelled);
            return;
        }

        MAC mac = dev->Mac();
        bool doProcessResponse = true;

        //in current state, process own response and send next state request
        switch (p_rBurstConfig.GetState())
        {
            case AppSetBurstConfigurationCmd::AppSetBurstConfigurationCmd::SET_BURSTCONF_READ_CONFIG_STATE:
            {
                LOG_INFO_APP("[SetBurstConfiguration]: RESPONSE - SET_BURSTCONF_READ_CONFIG_STATE - deviceMAC=" << p_rBurstConfig.GetDevice()->Mac());

                if (!AppSetBurstConfigurationCmd_ProcessReadBurstConfigResponse(p_rBurstConfig))
                {
                    AppSetBurstConfigurationCmd_Finished(p_rBurstConfig);
                    break;
                }

                doProcessResponse = AppSetBurstConfigurationCmd_SendTurnOffBurstMessagesCmd(p_rBurstConfig, m_pResponse->appData.appCmd, m_pProcessor);
                p_rBurstConfig.SetState(AppSetBurstConfigurationCmd::SET_BURSTCONF_TURNOFF_BURSTMESSAGES_STATE);
                if (doProcessResponse) //request was sent, so response should be processed (when it arrives)
                {
                    break;
                }
                //else go to next state without processing response (no response since no request was sent in current stage)
            }
            case AppSetBurstConfigurationCmd::SET_BURSTCONF_TURNOFF_BURSTMESSAGES_STATE:
            {
                LOG_INFO_APP("[SetBurstConfiguration]: RESPONSE - SET_BURSTCONF_TURNOFF_BURSTMESSAGES_STATE - deviceMAC=" << p_rBurstConfig.GetDevice()->Mac());

                if (doProcessResponse)
                {
                    if (!AppSetBurstConfigurationCmd_ProcessTurnOffBurstMessagesResponse(p_rBurstConfig))
                    {
                        AppSetBurstConfigurationCmd_Finished(p_rBurstConfig);
                        break;
                    }
                }

                if (dev->IsAdapter())
                {
                    doProcessResponse = AppSetBurstConfigurationCmd_SendGetSubDeviceCountCmd(p_rBurstConfig, m_pResponse->appData.appCmd, m_pProcessor);
                    p_rBurstConfig.SetState(AppSetBurstConfigurationCmd::SET_BURSTCONF_GET_SUBDEVICEINDEX_STATE);
                }
                else
                {
                    doProcessResponse = AppSetBurstConfigurationCmd_SendConfigureBurstMessagesCmd(p_rBurstConfig,
                        m_pResponse->appData.appCmd, m_pProcessor);
                    p_rBurstConfig.SetState(AppSetBurstConfigurationCmd::SET_BURSTCONF_CONFIGURE_BURSTMESSAGES_STATE);
                }

                if (doProcessResponse) //request was sent, so response should be processed (when it arrives)
                {
                    break;
                }
                //else go to next state without processing response (no response since no request was sent in current stage)
            }
            case AppSetBurstConfigurationCmd::SET_BURSTCONF_GET_SUBDEVICEINDEX_STATE:
            {
                LOG_INFO_APP("[SetBurstConfiguration]: RESPONSE - SET_BURSTCONF_GET_SUBDEVICEINDEX_STATE - deviceMAC=" << p_rBurstConfig.GetDevice()->Mac());

                if (doProcessResponse)
                {
                    if (!AppSetBurstConfigurationCmd_ProcessGetSubDeviceIndexResponse(p_rBurstConfig))
                    {
                        AppSetBurstConfigurationCmd_Finished(p_rBurstConfig);
                        break;
                    }
                }
                doProcessResponse = AppSetBurstConfigurationCmd_SendConfigureBurstMessagesCmd(p_rBurstConfig, m_pResponse->appData.appCmd, m_pProcessor);
                p_rBurstConfig.SetState(AppSetBurstConfigurationCmd::SET_BURSTCONF_CONFIGURE_BURSTMESSAGES_STATE);
                if (doProcessResponse) //request was sent, so response should be processed (when it arrives)
                {
                    break;
                }
                //else go to next state without processing response (no response since no request was sent in current stage)
            }
            case AppSetBurstConfigurationCmd::SET_BURSTCONF_CONFIGURE_BURSTMESSAGES_STATE:
            {
                LOG_INFO_APP("[SetBurstConfiguration]: RESPONSE - SET_BURSTCONF_CONFIGURE_BURSTMESSAGES_STATE - deviceMAC=" << p_rBurstConfig.GetDevice()->Mac());

                if (doProcessResponse)
                {
                    AppSetBurstConfigurationCmd_ProcessConfigureBurstMessagesResponse(p_rBurstConfig);
                    if (AppSetBurstConfigurationCmd_SendConfigureBurstMessagesCmd(p_rBurstConfig, m_pResponse->appData.appCmd, m_pProcessor))
                    {
                        break;
                    }
                }

                AppSetBurstConfigurationCmd_Finished(p_rBurstConfig);
                p_rBurstConfig.pCommands->SetCommandResponded(p_rBurstConfig.dbCommand, nlib::CurrentUniversalTime(), "success", NULL);

                break;
            }
        }

        p_rBurstConfig.pDevices->UpdatePublishersCache(mac, dev->GetPublisherInfo());
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Visit::AppSetBurstConfigurationCmd]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Visit::AppSetBurstConfigurationCmd]: System error!");
    }
}

bool ResponseProcessor::AppSetBurstConfigurationCmd_ProcessReadBurstConfigResponse(AppSetBurstConfigurationCmd& p_rBurstConfig)
{
    try
    {
        DevicePtr dev = p_rBurstConfig.GetDevice();
        if (dev == 0)
        {
            SaveRespErr(*p_rBurstConfig.pCommands, p_rBurstConfig.dbCommand).CommandFailed(DBCommand::rsFailure_InvalidDevice);
            LOG_ERROR_APP("[SetBurstConfiguration]: device not initialized in burst configuration command");
            return false;
        }

        if (m_pResponse->whCmd.responseCode == RCS_E64_CommandNotImplemented)
        {
            LOG_ERROR_APP("[SetBurstConfiguration] Processing cmd 105 response for device " << dev->id << " - FATAL ERROR");

            SaveRespErr(*p_rBurstConfig.pCommands, p_rBurstConfig.dbCommand).CommandFailed(DBCommand::rsFailure_InternalError);

            dev->GetPublisherInfo().burstNoTotalCmd105 = 0; // fatal error => we abort the burst configuration
            PublisherConf().SetBurstNoTotalCmd105(p_rBurstConfig.GetPubConfigFileName().c_str(), dev->Mac(), 0);
            p_rBurstConfig.pDevices->UpdatePublisherState(dev->Mac(), -1, SETPUBLISHER_STATE_READ_BURSTCONFIG, SETPUBLISHER_ERROR_READ_BURSTCONFIG_CMD105_NOT_IMPL, "");
            return false;
        }

        if (SaveRespErr::IsErr_105(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_ERROR_APP("[SetBurstConfiguration] Processing cmd 105 response for device " << dev->id << " failed : hostErr="
                        << (int) (m_pResponse->hostErr) << " responseCode=" << (int) (m_pResponse->whCmd.responseCode));
            SaveRespErr(*p_rBurstConfig.pCommands, p_rBurstConfig.dbCommand).SaveErr_105(m_pResponse->hostErr, m_pResponse->whCmd.responseCode);
            dev->GetPublisherInfo().burstNoTotalCmd105 = PublisherInfo::g_nUnsetBurstNoTotalCmd105;
            p_rBurstConfig.pDevices->UpdatePublisherState(dev->Mac(), -1, SETPUBLISHER_STATE_READ_BURSTCONFIG, SETPUBLISHER_ERROR_READ_BURSTCONFIG,
                                                          "105-" + SaveRespErr::GetErrText(m_pResponse->whCmd.commandID, m_pResponse->hostErr, m_pResponse->whCmd.responseCode));
            return false;
        }

        C105_ReadBurstModeConfiguration_Resp *pwhResp = (C105_ReadBurstModeConfiguration_Resp*) m_pResponse->whCmd.command;
        if (!pwhResp)
        {
            LOG_ERROR_APP("[SetBurstConfiguration]: Incorrect response received for CMD105. DeviceMAC=" << dev->Mac());
            p_rBurstConfig.pDevices->UpdatePublisherState(dev->Mac(), -1, SETPUBLISHER_STATE_READ_BURSTCONFIG, SETPUBLISHER_ERROR_READ_BURSTCONFIG, "105-InvalidResponse");
            return false;
        }

        dev->GetPublisherInfo().burstNoTotalCmd105 = pwhResp->totalBurstMessagesCount;

        PublisherConf().SetBurstNoTotalCmd105(p_rBurstConfig.GetPubConfigFileName().c_str(), dev->Mac(), pwhResp->totalBurstMessagesCount);

        LOG_DEBUG_APP("[SetBurstConfiguration] Successfully processed cmd 105 response for for device " << dev->id
                    << "  - total number of supported burst messages= " << (int) (pwhResp->totalBurstMessagesCount));

        return true;
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[AppSetBurstConfigurationCmd_ProcessReadBurstConfigResponse]: Application error: " << e.what());
        return false;
    }
    catch (...)
    {
        LOG_ERROR_APP("[AppSetBurstConfigurationCmd_ProcessReadBurstConfigResponse]: System error!");
        return false;
    }
}

bool ResponseProcessor::AppSetBurstConfigurationCmd_ProcessTurnOffBurstMessagesResponse(AppSetBurstConfigurationCmd& p_rBurstConfig)
{
    try
    {
        DevicePtr dev = p_rBurstConfig.GetDevice();
        if (dev == 0)
        {
            LOG_ERROR_APP("[SetBurstConfiguration]: Error on turn-off burst messages. Invalid device or not connected.");
            SaveRespErr(*p_rBurstConfig.pCommands, p_rBurstConfig.dbCommand).CommandFailed(DBCommand::rsFailure_InvalidDevice);
            p_rBurstConfig.RegisterBurstMessageConfigurationStatus(BURST_STATE_NOT_SET);
            return false;
        }

        PublisherInfo& pubInfo = dev->GetPublisherInfo();

        if (m_pResponse->appData.cmdType != gateway::GatewayIO::AppData::CmdType_Multiple || m_pResponse->pCmdWrappersList == 0)
        {
            LOG_ERROR_APP("[SetBurstConfiguration] Processing burst messages turnoff response for deviceMAC==" << dev->Mac()
                        << " INTERNAL ERROR - " << " CmdTypeMultiple:" << (int) (m_pResponse->appData.cmdType
                        != gateway::GatewayIO::AppData::CmdType_Multiple) << " pCmdWrappersList:" << std::hex
                        << (int) (m_pResponse->pCmdWrappersList));

            SaveRespErr(*p_rBurstConfig.pCommands, p_rBurstConfig.dbCommand).CommandFailed(DBCommand::rsFailure_InternalError);
            p_rBurstConfig.RegisterBurstMessageConfigurationStatus(BURST_STATE_NOT_SET);
            p_rBurstConfig.pDevices->UpdatePublisherState(dev->Mac(), -1, SETPUBLISHER_STATE_TURNOFF_BURST, SETPUBLISHER_ERROR_TURNOFF_BURST, "InvalidResponse");
            return false;
        }

        stack::CHartCmdWrapperList::iterator cmdIt = m_pResponse->pCmdWrappersList->begin();

        //check for cmd 109 not implemented => ERROR
        if (((*cmdIt)->GetCmdId() != CMDID_C106_FlushDelayedResponses) && ((*cmdIt)->GetResponseCode() == RCS_E64_CommandNotImplemented))
        {
            for (int i = 0; i < pubInfo.burstNoTotalCmd105; ++i)
            {
                pubInfo.burstMessagesState[i] = BURST_STATE_ERROR;
                PublisherConf().SetBurstState(p_rBurstConfig.GetPubConfigFileName().c_str(), dev->Mac(), (uint8_t) i, BURST_STATE_ERROR, "109-NotImplemented");
                p_rBurstConfig.RegisterBurstMessageConfigurationStatus(BURST_STATE_ERROR);
            }
            LOG_ERROR_APP("[SetBurstConfiguration] Processing burst messages turn-off response for deviceMAC=" << dev->Mac() << " - FATAL ERROR");

            SaveRespErr(*p_rBurstConfig.pCommands, p_rBurstConfig.dbCommand).CommandFailed(DBCommand::rsFailure_InternalError);
            p_rBurstConfig.pDevices->UpdatePublisherState(dev->Mac(), -1, SETPUBLISHER_STATE_TURNOFF_BURST, SETPUBLISHER_ERROR_TURNOFF_BURST_CMD109_NOT_IMPL, "");
            return false;
        }

        bool withError = false;
        for (int i = 0; cmdIt != m_pResponse->pCmdWrappersList->end(); ++cmdIt)
        {
            if (((*cmdIt)->GetCmdId() == CMDID_C106_FlushDelayedResponses))
            {
                continue;
            }

            if (SaveRespErr::IsErr_109(m_pResponse->hostErr, (*cmdIt)->GetResponseCode()))
            {
                SaveRespErr(*p_rBurstConfig.pCommands, p_rBurstConfig.dbCommand).SaveErr_109(m_pResponse->hostErr, m_pResponse->whCmd.responseCode);

                if ((*cmdIt)->GetResponseCode() == RCS_E32_Busy || (*cmdIt)->GetResponseCode() == RCS_E36_DelayedResponseConflict)
                {
                    pubInfo.flushDelayedResponses = true;
                    LOG_ERROR_APP("[SetBurstConfiguration] Processing burst messages turn-off response for deviceMAC= " << dev->Mac()
                                << " - flushing delayed responses because we have received ResponseCode=" << ((int) ((*cmdIt)->GetResponseCode())));
                }
                p_rBurstConfig.RegisterBurstMessageConfigurationStatus(BURST_STATE_NOT_SET);
                withError = true;
                std::string errorMessage("109-" + SaveRespErr::GetErrText((*cmdIt)->GetResponseCode(), m_pResponse->hostErr, (*cmdIt)->GetResponseCode()) + " on turning off");
                PublisherConf().SetBurstState(p_rBurstConfig.GetPubConfigFileName().c_str(), dev->Mac(), i++, BURST_STATE_ERROR, errorMessage.c_str());
                p_rBurstConfig.pDevices->UpdatePublisherState(dev->Mac(), -1, SETPUBLISHER_STATE_TURNOFF_BURST, SETPUBLISHER_ERROR_TURNOFF_BURST,
                                                              "109-" + SaveRespErr::GetErrText((*cmdIt)->GetCmdId(), m_pResponse->hostErr, (*cmdIt)->GetResponseCode()));
                continue;
            }

            C109_BurstModeControl_Resp* pStResp = (C109_BurstModeControl_Resp*) (*cmdIt)->GetParsedData();
            if (!pStResp)
            {
                LOG_ERROR_APP("[SetBurstConfiguration]: Incorrect response received for CMD109. DeviceMAC=" << dev->Mac());
                p_rBurstConfig.pDevices->UpdatePublisherState(dev->Mac(), -1, SETPUBLISHER_STATE_TURNOFF_BURST, SETPUBLISHER_ERROR_TURNOFF_BURST, "109-InvalidResponse");
                return false;
            }
            pubInfo.burstMessagesState[pStResp->burstMessage] = BURST_STATE_OFF;
            PublisherConf().SetBurstState(p_rBurstConfig.GetPubConfigFileName().c_str(), dev->Mac(), pStResp->burstMessage, BURST_STATE_OFF, NULL);
            p_rBurstConfig.RegisterBurstMessageConfigurationStatus(BURST_STATE_OFF);
        }

        LOG_INFO_APP("[SetBurstConfiguration] Finished Processing burst messages turnoff response for device " << dev->id << " with status=" << (withError ? "FAIL" : "SUCCESS"));
        return !withError;
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[AppSetBurstConfigurationCmd_ProcessTurnOffBurstMessagesResponse]: Application error: " << e.what());
        return false;
    }
    catch (...)
    {
        LOG_ERROR_APP("[AppSetBurstConfigurationCmd_ProcessTurnOffBurstMessagesResponse]: System error!");
        return false;
    }
}

bool ResponseProcessor::AppSetBurstConfigurationCmd_ProcessGetSubDeviceIndexResponse(AppSetBurstConfigurationCmd& p_rBurstConfig)
{
    try
    {
        DevicePtr dev = p_rBurstConfig.GetDevice();
        if (dev == 0) // invalid device status
        {
            LOG_ERROR_APP("[SetBurstConfiguration]: device not initialized in burst configuration command");
            p_rBurstConfig.RegisterBurstMessageConfigurationStatus(BURST_STATE_NOT_SET);
            return false;
        }

        if (m_pResponse->whCmd.commandID == CMDID_C074_ReadIOSystemCapabilities)
        {
            if (SaveRespErr::IsErr_074(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
            {
                LOG_WARN_APP("[SetBurstConfiguration]:CMD074 - Request for deviceMAC=" << dev->Mac() << " has got a response code=" << m_pResponse->whCmd.responseCode);
                SaveRespErr(*p_rBurstConfig.pCommands, p_rBurstConfig.dbCommand).SaveErr_074(m_pResponse->hostErr, m_pResponse->whCmd.responseCode);
                p_rBurstConfig.pDevices->UpdatePublisherState(dev->Mac(), -1, SETPUBLISHER_STATE_GET_SUBDEVICEINDEX, SETPUBLISHER_ERROR_GET_SUBDEVICEINDEX,
                                                              "74-" + SaveRespErr::GetErrText(m_pResponse->whCmd.commandID, m_pResponse->hostErr, m_pResponse->whCmd.responseCode));
                return false;
            }

            C074_ReadIOSystemCapabilities_Resp *pwhResp074 = (C074_ReadIOSystemCapabilities_Resp*) (m_pResponse->whCmd.command);
            if (!pwhResp074)
            {
                LOG_ERROR_APP("[SetBurstConfiguration]: Incorrect response received for CMD074. DeviceMAC=" << dev->Mac());
                p_rBurstConfig.pDevices->UpdatePublisherState(dev->Mac(), -1, SETPUBLISHER_STATE_GET_SUBDEVICEINDEX, SETPUBLISHER_ERROR_GET_SUBDEVICEINDEX, "74-InvalidResponse");
                return false;
            }

            p_rBurstConfig.m_nSubDevicesCount = (int) pwhResp074->noOfDevicesDetected - 1;
            dev->SetMasterMode(pwhResp074->masterMode);
            LOG_DEBUG_APP("[SetBurstConfiguration]:CMD074 - RESPONSE[" << dev->Mac() << "]=(subDevicesCount=" << (int) p_rBurstConfig.m_nSubDevicesCount << ")");

            if (p_rBurstConfig.m_nSubDevicesCount <= 0)
            {
                LOG_DEBUG_APP("[SetBurstConfiguration]:CMD074 - There is no sub-devices for deviceMAC=" << dev->Mac());
                return true;
            }
            else //if (p_rBurstConfig.m_nSubDevicesCount > 0) // number of detected sub-devices
            {
                LOG_DEBUG_APP("[SetBurstConfiguration]:CMD074 - subDevicesMap.size()=" << (int) dev->subDevicesMap.size()
                            << ", subDeviceCount=" << (int) p_rBurstConfig.m_nSubDevicesCount << " - for deviceMAC=" << dev->Mac());

                p_rBurstConfig.m_nCurrentSubDeviceIndex = 1;
                Device::SubDevicesMapT::iterator it;
                bool isNotInMap = true;
                while (p_rBurstConfig.m_nCurrentSubDeviceIndex <= p_rBurstConfig.m_nSubDevicesCount)
                {
                    for (it = dev->subDevicesMap.begin(); it != dev->subDevicesMap.end(); ++it)
                    {
                        if (it->second == p_rBurstConfig.m_nCurrentSubDeviceIndex)
                        {
                            isNotInMap = false;
                            break;
                        }
                    }
                    if (isNotInMap)
                    {
                        break;
                    }
                    else
                    {
                        LOG_DEBUG_APP("[SetBurstConfiguration]:CMD074 - subDeviceIndex="
                                    << p_rBurstConfig.m_nCurrentSubDeviceIndex << " is already in the MAP: subDevicesMap["
                                    << it->first << "]=" << (int) it->second << " where deviceMAC=" << dev->Mac() << ". Go to next index: "
                                    << (int) (p_rBurstConfig.m_nCurrentSubDeviceIndex + 1));
                        p_rBurstConfig.m_nCurrentSubDeviceIndex++;
                    }
                }

                LOG_DEBUG_APP("[SetBurstConfiguration]:CMD074 - CurentSubdeviceIndex[" << dev->Mac() << "]=" << (int) p_rBurstConfig.m_nCurrentSubDeviceIndex);
                if (p_rBurstConfig.m_nCurrentSubDeviceIndex > p_rBurstConfig.m_nSubDevicesCount)
                    return true;

                if (!AppSetBurstConfigurationCmd_SendGetSubDeviceIdCmd(p_rBurstConfig, m_pResponse->appData.appCmd, m_pProcessor))
                {
                    dev->IssueSetBurstNotificationCmd = true;
                    LOG_ERROR_APP("[SetBurstConfiguration]:CMD" << m_pResponse->whCmd.commandID << " - Error on sending command 074 to deviceMAC=" << dev->Mac());
                    p_rBurstConfig.pDevices->UpdatePublisherState(dev->Mac(), -1, SETPUBLISHER_STATE_GET_SUBDEVICEINDEX, SETPUBLISHER_ERROR_GET_SUBDEVICEINDEX, "74-CannotSendCommand");
                }
                return false;
            }
        }
        else if (m_pResponse->whCmd.commandID == CMDID_C084_ReadSubDeviceIdentitySummary)
        {
            if (SaveRespErr::IsErr_084(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
            {
                LOG_WARN_APP("[SetBurstConfiguration]:CMD084 - Request for deviceMAC=" << dev->Mac() << " has got a response code="
                            << (int) m_pResponse->whCmd.responseCode);
                SaveRespErr(*p_rBurstConfig.pCommands, p_rBurstConfig.dbCommand).SaveErr_084(m_pResponse->hostErr, m_pResponse->whCmd.responseCode);
                p_rBurstConfig.pDevices->UpdatePublisherState(dev->Mac(), -1, SETPUBLISHER_STATE_GET_SUBDEVICEINDEX, SETPUBLISHER_ERROR_GET_SUBDEVICEINDEX,
                                                              "84-" + SaveRespErr::GetErrText(m_pResponse->whCmd.commandID, m_pResponse->hostErr, m_pResponse->whCmd.responseCode));
                return false;
            }

            C084_ReadSubDeviceIdentitySummary_Resp *pwhResp084 = (C084_ReadSubDeviceIdentitySummary_Resp*) (m_pResponse->whCmd.command);
            if (!pwhResp084)
            {
                LOG_ERROR_APP("[SetBurstConfiguration]: Incorrect response received for CMD084. DeviceMAC=" << dev->Mac());
                p_rBurstConfig.pDevices->UpdatePublisherState(dev->Mac(), -1, SETPUBLISHER_STATE_GET_SUBDEVICEINDEX, SETPUBLISHER_ERROR_GET_SUBDEVICEINDEX, "84-InvalidResponse");
                return false;
            }
            MAC subDeviceMAC(MakeCorrectedUniqueID(pwhResp084->expandedDeviceType, pwhResp084->deviceID, dev->GetMasterMode()).bytes);
            dev->subDevicesMap[subDeviceMAC] = p_rBurstConfig.m_nCurrentSubDeviceIndex;

            LOG_DEBUG_APP("[SetBurstConfiguration]:CMD084 - RESPONSE[" << dev->Mac() << "]=(subDeviceMAC=" << subDeviceMAC
                        << ", subDeviceIndex=" << (int) p_rBurstConfig.m_nCurrentSubDeviceIndex << ", subDeviceCont="
                        << (int) p_rBurstConfig.m_nSubDevicesCount << ")");

            if (p_rBurstConfig.m_nSubDevicesCount > p_rBurstConfig.m_nCurrentSubDeviceIndex)
            {
                p_rBurstConfig.m_nCurrentSubDeviceIndex++;
                LOG_DEBUG_APP("[SetBurstConfiguration]:CMD084 - %%%%%%%%%%% CurentSubdeviceIndex[" << dev->Mac() << "]="
                            << (int) p_rBurstConfig.m_nCurrentSubDeviceIndex);
                if (!AppSetBurstConfigurationCmd_SendGetSubDeviceIdCmd(p_rBurstConfig, m_pResponse->appData.appCmd, m_pProcessor))
                {
                    dev->IssueSetBurstNotificationCmd = true;
                    LOG_ERROR_APP("[SetBurstConfiguration]:CMD084 - EXCEPTION: Error on sending command 084 to deviceMAC=" << dev->Mac());
                    p_rBurstConfig.pDevices->UpdatePublisherState(dev->Mac(), -1, SETPUBLISHER_STATE_GET_SUBDEVICEINDEX, SETPUBLISHER_ERROR_GET_SUBDEVICEINDEX, "84-CannotSendCommand");
                }
                return false;
            }
        }

        return true;
    }
    catch (std::exception& e)
    {
        LOG_ERROR_APP("[AppSetBurstConfigurationCmd_ProcessGetSubDeviceIndexResponse]: Application error: " << e.what());
        return false;
    }
    catch (...)
    {
        LOG_ERROR_APP("[AppSetBurstConfigurationCmd_ProcessGetSubDeviceIndexResponse]: System error!");
        return false;
    }
}

bool ResponseProcessor::AppSetBurstConfigurationCmd_ProcessConfigureBurstMessagesResponse(AppSetBurstConfigurationCmd& p_rBurstConfig)
{
    try
    {
        DevicePtr dev = p_rBurstConfig.GetDevice();
        if (dev == 0)
        {
            LOG_ERROR_APP("[SetBurstConfiguration]: device not initialized in burst configuration command");
            p_rBurstConfig.RegisterBurstMessageConfigurationStatus(BURST_STATE_NOT_SET);
            return false;
        }

        uint8_t burstMessage = (uint8_t)(m_pResponse->appData.innerDataHandle);
        PublisherInfo& pubInfo = dev->GetPublisherInfo();

        if (m_pResponse->appData.cmdType != gateway::GatewayIO::AppData::CmdType_Multiple || m_pResponse->pCmdWrappersList == 0)
        {
            LOG_ERROR_APP("[SetBurstConfiguration] Processing burst message " << (int) burstMessage << " configuration response on device "
                        << dev->id << " - INTERNAL ERROR - " << " CmdTypeMultiple:" << (int) (m_pResponse->appData.cmdType
                        != gateway::GatewayIO::AppData::CmdType_Multiple) << " pCmdWrappersList:" << std::hex
                        << (int) (m_pResponse->pCmdWrappersList));

            p_rBurstConfig.RegisterBurstMessageConfigurationStatus(BURST_STATE_NOT_SET);
            p_rBurstConfig.pDevices->UpdatePublisherState(dev->Mac(), (int)burstMessage, SETPUBLISHER_STATE_CONFIGURE_BURST, SETPUBLISHER_ERROR_CONFIGBURST, "InvalidResponse");
            return false;
        }

        std::string errorMessage = "";
        BurstState state = BURST_STATE_NOT_SET;
        {
            PublisherInfo::BurstMessageStateMap::iterator stateIt = pubInfo.burstMessagesState.find(burstMessage);
            if (stateIt != pubInfo.burstMessagesState.end())
            {
                state = stateIt->second;
            }
        }

        stack::CHartCmdWrapperList::iterator cmdIt = m_pResponse->pCmdWrappersList->begin();
        for (; cmdIt != m_pResponse->pCmdWrappersList->end(); ++cmdIt)
        {
            if (((*cmdIt)->GetCmdId() == CMDID_C106_FlushDelayedResponses))
            {
                continue;
            }

            //check for not implemented => FATAL ERROR
            if ((*cmdIt)->GetResponseCode() == RCS_E64_CommandNotImplemented)
            {
                switch ((*cmdIt)->GetCmdId())
                {
                    case CMDID_C102_MapSubDeviceToBurstMessage:
                        p_rBurstConfig.pDevices->UpdatePublisherState(dev->Mac(), (int)burstMessage, SETPUBLISHER_STATE_CONFIGURE_BURST, SETPUBLISHER_ERROR_CONFIGBURST_CMD102_NOT_IMPL, "");
                        break;
                    case CMDID_C103_WriteBurstPeriod:
                        p_rBurstConfig.pDevices->UpdatePublisherState(dev->Mac(), (int)burstMessage, SETPUBLISHER_STATE_CONFIGURE_BURST, SETPUBLISHER_ERROR_CONFIGBURST_CMD103_NOT_IMPL, "");
                        break;
                    case CMDID_C104_WriteBurstTrigger:
                        p_rBurstConfig.pDevices->UpdatePublisherState(dev->Mac(), (int)burstMessage, SETPUBLISHER_STATE_CONFIGURE_BURST, SETPUBLISHER_ERROR_CONFIGBURST_CMD104_NOT_IMPL, "");
                        break;
                    case CMDID_C106_FlushDelayedResponses:
                        p_rBurstConfig.pDevices->UpdatePublisherState(dev->Mac(), (int)burstMessage, SETPUBLISHER_STATE_CONFIGURE_BURST, SETPUBLISHER_ERROR_CONFIGBURST_CMD106_NOT_IMPL, "");
                        break;
                    case CMDID_C107_WriteBurstDeviceVariables:
                        p_rBurstConfig.pDevices->UpdatePublisherState(dev->Mac(), (int)burstMessage, SETPUBLISHER_STATE_CONFIGURE_BURST, SETPUBLISHER_ERROR_CONFIGBURST_CMD107_NOT_IMPL, "");
                        break;
                    case CMDID_C108_WriteBurstModeCommandNumber:
                        p_rBurstConfig.pDevices->UpdatePublisherState(dev->Mac(), (int)burstMessage, SETPUBLISHER_STATE_CONFIGURE_BURST, SETPUBLISHER_ERROR_CONFIGBURST_CMD108_NOT_IMPL, "");
                        break;
                    case CMDID_C109_BurstModeControl:
                        p_rBurstConfig.pDevices->UpdatePublisherState(dev->Mac(), (int)burstMessage, SETPUBLISHER_STATE_CONFIGURE_BURST, SETPUBLISHER_ERROR_CONFIGBURST_CMD109_NOT_IMPL, "");
                        break;
                }

                state = BURST_STATE_ERROR;
                break; // continue;
            }

            if ((*cmdIt)->GetResponseCode() == RCS_E32_Busy || (*cmdIt)->GetResponseCode() == RCS_E36_DelayedResponseConflict)
            {
                pubInfo.flushDelayedResponses = true;
                LOG_ERROR_APP("[SetBurstConfiguration] Processing burst message " << (int) burstMessage
                            << " configuration response on device " << (int) (dev->id)
                            << " - flushing delayed responses because we have received RC " << ((int) ((*cmdIt)->GetResponseCode())));
                p_rBurstConfig.pDevices->UpdatePublisherState(dev->Mac(), (int)burstMessage, SETPUBLISHER_STATE_CONFIGURE_BURST, SETPUBLISHER_ERROR_CONFIGBURST_BUSY, "");

                state = BURST_STATE_TIMEOUT;
                break; // continue;
            }

            if (CMDID_C102_MapSubDeviceToBurstMessage == (*cmdIt)->GetCmdId()) // switch ((*cmdIt)->GetCmdId()) { case CMDID_C102_MapSubDeviceToBurstMessage:
            {
                if (SaveRespErr::IsErr_102(m_pResponse->hostErr, (*cmdIt)->GetResponseCode()))
                {
                    LOG_ERROR_APP("[SetBurstConfiguration] Processing burst message " << (int) burstMessage
                                << " configuration response on device " << (int) (dev->id) << " - cmd 102 failed : hosterr="
                                << (int) (m_pResponse->hostErr) << " resp code=" << (int) ((*cmdIt)->GetResponseCode()));
                    p_rBurstConfig.pDevices->UpdatePublisherState(dev->Mac(), (int)burstMessage, SETPUBLISHER_STATE_CONFIGURE_BURST, SETPUBLISHER_ERROR_CONFIGBURST_MAP_SUBDEVICE,
                                                                  "102-" + SaveRespErr::GetErrText((*cmdIt)->GetCmdId(), m_pResponse->hostErr, (*cmdIt)->GetResponseCode()));

                    state = BURST_STATE_ERROR;
                    break; // continue;
                }
            }
            else if (CMDID_C103_WriteBurstPeriod == (*cmdIt)->GetCmdId()) // case CMDID_C103_WriteBurstPeriod:
            {
                //if the device has adjusted the publish time, use the new publish time
                if ((*cmdIt)->GetResponseCode() == C103_W08)
                {
                    for (BurstMessageSetT::iterator bmIt = pubInfo.burstMessageList.begin(); bmIt != pubInfo.burstMessageList.end(); ++bmIt)
                    {
                        if (bmIt->burstMessage == burstMessage)
                        {
                            BurstMessage& bm = const_cast<BurstMessage&> (*bmIt); // safe because we don't modify the key

                            C103_WriteBurstPeriod_Resp* pStResp = (C103_WriteBurstPeriod_Resp*) (*cmdIt)->GetParsedData();
                            if (!pStResp)
                            {
                                LOG_ERROR_APP("[SetBurstConfiguration]: Incorrect response received for CMD103. DeviceMAC=" << dev->Mac());
                                p_rBurstConfig.pDevices->UpdatePublisherState(dev->Mac(), (int)burstMessage, SETPUBLISHER_STATE_CONFIGURE_BURST, SETPUBLISHER_ERROR_CONFIGBURST_WRITE_UPDATE_PERIOD, "103-InvalidResponse");
                                break;
                            }

                            float updatePer = pStResp->updatePeriod.u32 / 32000;
                            LOG_WARN_APP("[SetBurstConfiguration]: Update period adjusted to: " << updatePer << " for mac=" << dev->Mac());

                            if (bm.updatePeriod != updatePer)
                            {
                                bm.updatePeriod = updatePer;
                                PublisherConf().SetBurstPublishPeriod(p_rBurstConfig.GetPubConfigFileName().c_str(), dev->Mac(), burstMessage, updatePer);
                                p_rBurstConfig.pDevices->UpdateBurstMessage(dev->id, bm);
                            }
                            break;
                        }
                    }
                }
                else if (SaveRespErr::IsErr_103(m_pResponse->hostErr, (*cmdIt)->GetResponseCode()))
                {
                    LOG_ERROR_APP("[SetBurstConfiguration] Processing burst message " << (int)burstMessage
                                << " configuration response on device " << (int) (dev->id) << " - cmd 103 failed : hosterr="
                                << (int) (m_pResponse->hostErr) << " resp code=" << (int) ((*cmdIt)->GetResponseCode()));
                    p_rBurstConfig.pDevices->UpdatePublisherState(dev->Mac(), (int)burstMessage, SETPUBLISHER_STATE_CONFIGURE_BURST, SETPUBLISHER_ERROR_CONFIGBURST_WRITE_UPDATE_PERIOD,
                                                                  "103-" + SaveRespErr::GetErrText((*cmdIt)->GetCmdId(), m_pResponse->hostErr, (*cmdIt)->GetResponseCode()));

                    state = BURST_STATE_ERROR;
                    break;
                }
            }
            else if (CMDID_C104_WriteBurstTrigger == (*cmdIt)->GetCmdId()) // case CMDID_C104_WriteBurstTrigger:
            {
                if (SaveRespErr::IsErr_104(m_pResponse->hostErr, (*cmdIt)->GetResponseCode()))
                {
                    LOG_ERROR_APP("[SetBurstConfiguration] Processing burst message " << (int) burstMessage
                                << " configuration response on device " << (int) (dev->id) << " - cmd 104 failed : hosterr="
                                << (int) (m_pResponse->hostErr) << " resp code=" << (int) ((*cmdIt)->GetResponseCode()));
                    p_rBurstConfig.pDevices->UpdatePublisherState(dev->Mac(), (int)burstMessage, SETPUBLISHER_STATE_CONFIGURE_BURST, SETPUBLISHER_ERROR_CONFIGBURST_WRITE_TRIGGER,
                                                                  "104-" + SaveRespErr::GetErrText((*cmdIt)->GetCmdId(), m_pResponse->hostErr, (*cmdIt)->GetResponseCode()));

                    state = BURST_STATE_ERROR;
                    break;
                }
            }
            else if (CMDID_C107_WriteBurstDeviceVariables == (*cmdIt)->GetCmdId()) // case CMDID_C107_WriteBurstDeviceVariables:
            {
                if (SaveRespErr::IsErr_107(m_pResponse->hostErr, (*cmdIt)->GetResponseCode()))
                {
                    LOG_ERROR_APP("[SetBurstConfiguration] Processing burst message " << (int) burstMessage
                                << " configuration response on device " << (int) (dev->id) << " - cmd 107 failed : hosterr="
                                << (int) (m_pResponse->hostErr) << " resp code=" << (int) ((*cmdIt)->GetResponseCode()));
                    p_rBurstConfig.pDevices->UpdatePublisherState(dev->Mac(), (int)burstMessage, SETPUBLISHER_STATE_CONFIGURE_BURST, SETPUBLISHER_ERROR_CONFIGBURST_WRITE_VARIABLES,
                                                                  "107-" + SaveRespErr::GetErrText((*cmdIt)->GetCmdId(), m_pResponse->hostErr, (*cmdIt)->GetResponseCode()));

                    state = BURST_STATE_ERROR;
                    break;
                }
            }
            else if (CMDID_C108_WriteBurstModeCommandNumber == (*cmdIt)->GetCmdId()) // case CMDID_C108_WriteBurstModeCommandNumber:
            {
                if (SaveRespErr::IsErr_108(m_pResponse->hostErr, (*cmdIt)->GetResponseCode()))
                {
                    LOG_ERROR_APP("[SetBurstConfiguration] Processing burst message " << (int) burstMessage
                                << " configuration response on device " << (int) (dev->id) << " - cmd 108 failed : hosterr="
                                << (int) (m_pResponse->hostErr) << " resp code=" << (int) ((*cmdIt)->GetResponseCode()));
                    p_rBurstConfig.pDevices->UpdatePublisherState(dev->Mac(), (int)burstMessage, SETPUBLISHER_STATE_CONFIGURE_BURST, SETPUBLISHER_ERROR_CONFIGBURST_WRITE_COMMAND_NO,
                                                                  "108-" + SaveRespErr::GetErrText((*cmdIt)->GetCmdId(), m_pResponse->hostErr, (*cmdIt)->GetResponseCode()));

                    state = BURST_STATE_ERROR;
                    break;
                }
            }
            else if (CMDID_C109_BurstModeControl == (*cmdIt)->GetCmdId()) // case CMDID_C109_BurstModeControl:
            {
                if (SaveRespErr::IsErr_109(m_pResponse->hostErr, (*cmdIt)->GetResponseCode()))
                {
                    LOG_ERROR_APP("[SetBurstConfiguration] Processing burst message " << (int) burstMessage
                                << " configuration response on device " << (int) (dev->id) << " - cmd 109 failed : hosterr="
                                << (int) (m_pResponse->hostErr) << " resp code=" << (int) ((*cmdIt)->GetResponseCode()));
                    p_rBurstConfig.pDevices->UpdatePublisherState(dev->Mac(), (int)burstMessage, SETPUBLISHER_STATE_CONFIGURE_BURST, SETPUBLISHER_ERROR_CONFIGBURST_SET_BURST_ON,
                                                                  "109-" + SaveRespErr::GetErrText((*cmdIt)->GetCmdId(), m_pResponse->hostErr, (*cmdIt)->GetResponseCode()));

                    state = BURST_STATE_ERROR;
                    break;
                }
            }
            else // default:
            {
                LOG_DEBUG_APP("[SetBurstConfiguration] - Received invalid set burst config command " << (int) ((*cmdIt)->GetCmdId()));
                p_rBurstConfig.pDevices->UpdatePublisherState(dev->Mac(), (int)burstMessage, SETPUBLISHER_STATE_CONFIGURE_BURST, SETPUBLISHER_ERROR_CONFIGBURST, "InvalidCommand");
                break;
            } // }
        }
        bool isConfigured = (state != BURST_STATE_ERROR && state != BURST_STATE_TIMEOUT);
        if (isConfigured)
        {
            state = BURST_STATE_SET;
            p_rBurstConfig.pDevices->UpdatePublisherState(dev->Mac(), -1, SETPUBLISHER_STATE_DONE, SETPUBLISHER_ERROR_NONE, "");
            PublisherConf().SetBurstState(p_rBurstConfig.GetPubConfigFileName().c_str(), dev->Mac(), burstMessage, state, NULL);
        }
        else
        {
            std::ostringstream errmsg;
            errmsg << (int)((*cmdIt)->GetCmdId()) << "-" << SaveRespErr::GetErrText((*cmdIt)->GetResponseCode(), m_pResponse->hostErr, (*cmdIt)->GetResponseCode());
            PublisherConf().SetBurstState(p_rBurstConfig.GetPubConfigFileName().c_str(), dev->Mac(), burstMessage, state, errmsg.str().c_str());
        }
        pubInfo.burstMessagesState[burstMessage] = state;
        p_rBurstConfig.RegisterBurstMessageConfigurationStatus(state);
        LOG_INFO_APP("[SetBurstConfiguration] Finished processing burst message " << (int) burstMessage
                    << " configuration response on device " << dev->id << " - with status = " << (isConfigured ? "SUCCESS" : "FAIL"));
        return isConfigured;
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[AppSetBurstConfigurationCmd_ProcessConfigureBurstMessagesResponse]: Application error: " << e.what());
        return false;
    }
    catch (...)
    {
        LOG_ERROR_APP("[AppSetBurstConfigurationCmd_ProcessConfigureBurstMessagesResponse]: System error!");
        return false;
    }
}

//read value
void ResponseProcessor::Visit(AppReadValueCmd& p_rReadValueCmd)
{
    ////LOG_INFO_APP("[ResponseProcessor::Visit]:AppReadValueCmd. Device=");
    try
    {
        DevicePtr dev = p_rReadValueCmd.pDevices->FindDevice(p_rReadValueCmd.GetDeviceId());
        LOG_INFO_APP("[ReadValue]: RESPONSE received: commandNo=" << ((int) (p_rReadValueCmd.GetCmdNo())) << ", for deviceMAC=" << dev->Mac());

        if (ReadValueResponseError(p_rReadValueCmd))
        {
            LOG_ERROR_APP("[ReadValue]: m_pResponse reported an error");
            return;
        }

        bool succes;
        switch (p_rReadValueCmd.GetCmdNo())
        {
            case CMDID_C001_ReadPrimaryVariable:
                succes = ProcessReadValueCmd1(p_rReadValueCmd);
            break;
            case CMDID_C002_ReadLoopCurrentAndPercentOfRange:
                succes = ProcessReadValueCmd2(p_rReadValueCmd);
            break;
            case CMDID_C003_ReadDynamicVariablesAndLoopCurrent:
                succes = ProcessReadValueCmd3(p_rReadValueCmd);
            break;
            case CMDID_C009_ReadDeviceVariablesWithStatus:
                succes = ProcessReadValueCmd9(p_rReadValueCmd);
            break;
            case CMDID_C033_ReadDeviceVariables:
                succes = ProcessReadValueCmd33(p_rReadValueCmd);
            break;
            case CMDID_C178_PublishedDynamicData:
                succes = ProcessReadValueCmd178(p_rReadValueCmd);
            break;
            default:
                succes = false;
            break;
        }
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Visit::AppReadValueCmd]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Visit::AppReadValueCmd]: System error!");
    }
}

bool ResponseProcessor::ReadValueResponseError(AppReadValueCmd& p_rReadValueCmd)
{
    try
    {
        switch (p_rReadValueCmd.GetCmdNo())
        {
            case CMDID_C001_ReadPrimaryVariable:
                if (SaveRespErr::IsErr_001(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
                {
                    SaveRespErr(*p_rReadValueCmd.pCommands, p_rReadValueCmd.dbCommand).SaveErr_001(m_pResponse->hostErr, m_pResponse->whCmd.responseCode);
                    return true;
                }
                return false;
            case CMDID_C002_ReadLoopCurrentAndPercentOfRange:
                if (SaveRespErr::IsErr_002(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
                {
                    SaveRespErr(*p_rReadValueCmd.pCommands, p_rReadValueCmd.dbCommand).SaveErr_002(m_pResponse->hostErr, m_pResponse->whCmd.responseCode);
                    return true;
                }
                return false;
            case CMDID_C003_ReadDynamicVariablesAndLoopCurrent:
                if (SaveRespErr::IsErr_003(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
                {
                    SaveRespErr(*p_rReadValueCmd.pCommands, p_rReadValueCmd.dbCommand).SaveErr_003(m_pResponse->hostErr, m_pResponse->whCmd.responseCode);
                    return true;
                }
                return false;
            case CMDID_C009_ReadDeviceVariablesWithStatus:
                if (SaveRespErr::IsErr_009(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
                {
                    SaveRespErr(*p_rReadValueCmd.pCommands, p_rReadValueCmd.dbCommand).SaveErr_009(m_pResponse->hostErr, m_pResponse->whCmd.responseCode);
                    return true;
                }
                return false;
            case CMDID_C033_ReadDeviceVariables:
                if (SaveRespErr::IsErr_033(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
                {
                    SaveRespErr(*p_rReadValueCmd.pCommands, p_rReadValueCmd.dbCommand).SaveErr_033(m_pResponse->hostErr, m_pResponse->whCmd.responseCode);
                    return true;
                }
                return false;
            case CMDID_C178_PublishedDynamicData:
                if (SaveRespErr::IsErr_178(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
                {
                    SaveRespErr(*p_rReadValueCmd.pCommands, p_rReadValueCmd.dbCommand).SaveErr_178(m_pResponse->hostErr, m_pResponse->whCmd.responseCode);
                    return true;
                }
                return false;
            default:
                return true;
        }
        return true;
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[ReadValueResponseError]: Application error: " << e.what());
        return true;
    }
    catch (...)
    {
        LOG_ERROR_APP("[ReadValueResponseError]: System error!");
        return true;
    }
}

static const std::string& GetFloatValStr(volatile float val)
{
    static std::string floatValStr;
    floatValStr = "val: ";
    char szFloatVal[100];
    if (val != val)
    	strcpy(szFloatVal, "NaN");
    else
    	if (val == std::numeric_limits<float>::infinity())
    		strcpy(szFloatVal, "Infinity");
    	else
    		sprintf(szFloatVal, "%f", val);
    return floatValStr += szFloatVal;
}

static const std::string& GetFloatValStr(volatile float val, unsigned char status)
{
    static std::string floatValStr;
    floatValStr = "status: ";
    char szTempVal[100];
    sprintf(szTempVal, "0x%02X", status);
    floatValStr += szTempVal;
    floatValStr += " val: ";
    if (val != val)
        strcpy(szTempVal, "NaN");
    else
    	if (val == std::numeric_limits<float>::infinity())
    		strcpy(szTempVal, "Infinity");
    	else
    		sprintf(szTempVal, "%f", val);
    return floatValStr += szTempVal;
}

bool ResponseProcessor::ProcessReadValueCmd1(AppReadValueCmd& p_rReadValueCmd)
{
    try
    {
        struct timeval time;
        gettimeofday(&time, NULL);
        DeviceReading reading;

        PublishChannelSetT::iterator it = p_rReadValueCmd.GetChannelList().begin();

        if (it == p_rReadValueCmd.GetChannelList().end())
        {
            LOG_ERROR_APP("[ReadValue]: no channels for cmd=" << p_rReadValueCmd.GetCmdNo() << " and deviceID="
                        << p_rReadValueCmd.GetDeviceId());
            return false;
        }

        for (; it != p_rReadValueCmd.GetChannelList().end(); ++it)
        {
            if (it->cmdNo != p_rReadValueCmd.GetCmdNo())
            {
                LOG_ERROR_APP("[ReadValue]: Found bad cmdNo=" << ((int) it->cmdNo) << "in channelId=" << ((int) (it->channelID)));
                return false;
            }

            if (it->deviceVariable != DeviceVariableCodes_PrimaryVariable)
            {
                LOG_WARN_APP("[ReadValue]: Invalid device variable code " << (int) (it->deviceVariable) << " specified for cmd "
                            << (int) (p_rReadValueCmd.GetCmdNo()));
                continue;
            }

            C001_ReadPrimaryVariable_Resp *pwhResp = (C001_ReadPrimaryVariable_Resp*) (m_pResponse->whCmd.command);
            if (!pwhResp)
            {
                LOG_ERROR_APP("[ReadValue]: Incorrect response received for CMD001. DeviceID=" << p_rReadValueCmd.GetDeviceId());
                continue;
            }

            InitDeviceReading(reading, p_rReadValueCmd.GetDeviceId(), it->channelID, p_rReadValueCmd.dbCommand.commandID, time, 0,
                pwhResp->primaryVariable, false, p_rReadValueCmd.GetReadingType());
            p_rReadValueCmd.pDevices->ProcessReading(reading);

            if (p_rReadValueCmd.GetReadingType() == DeviceReading::ReadValue)
                p_rReadValueCmd.pCommands->SetCommandResponded(p_rReadValueCmd.dbCommand, nlib::CurrentUniversalTime(), GetFloatValStr(
                    pwhResp->primaryVariable), NULL);
        }

        return true;
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[ProcessReadValueCmd1]: Application error: " << e.what());
        return false;
    }
    catch (...)
    {
        LOG_ERROR_APP("[ProcessReadValueCmd1]: System error!");
        return false;
    }
}

bool ResponseProcessor::ProcessReadValueCmd2(AppReadValueCmd& p_rReadValueCmd)
{
    try
    {
        struct timeval time;
        gettimeofday(&time, NULL);
        DeviceReading reading;

        PublishChannelSetT::iterator it = p_rReadValueCmd.GetChannelList().begin();
        if (it == p_rReadValueCmd.GetChannelList().end())
        {
            LOG_ERROR_APP("[ReadValue]: no channels for cmd=" << p_rReadValueCmd.GetCmdNo() << " and deviceID="
                        << p_rReadValueCmd.GetDeviceId());
            return false;
        }

        C002_ReadLoopCurrentAndPercentOfRange_Resp *pwhResp = (C002_ReadLoopCurrentAndPercentOfRange_Resp*) (m_pResponse->whCmd.command);
        if (!pwhResp)
        {
            LOG_ERROR_APP("[ReadValue]: Incorrect response received for CMD002. DeviceID=" << p_rReadValueCmd.GetDeviceId());
            return false;
        }

        for (; it != p_rReadValueCmd.GetChannelList().end(); ++it)
        {
            if (it->cmdNo != p_rReadValueCmd.GetCmdNo())
            {
                LOG_ERROR_APP("[ReadValue]: Found bad cmdNo=" << ((int) it->cmdNo) << "in channelId=" << ((int) (it->channelID)));
                return false;
            }

            if (it->deviceVariable != DeviceVariableCodes_LoopCurrent && it->deviceVariable != DeviceVariableCodes_PercentRange)
            {
                LOG_WARN_APP("[ReadValue]: Invalid variable " << (int) (it->deviceVariable) << " specified for cmd "
                            << ((int) (p_rReadValueCmd.GetCmdNo())));
                continue;
            }

            InitDeviceReading(reading, p_rReadValueCmd.GetDeviceId(), it->channelID, p_rReadValueCmd.dbCommand.commandID, time, 0,
                (it->deviceVariable == DeviceVariableCodes_LoopCurrent) ? pwhResp->primaryVariableLoopCurrent
                    : pwhResp->primaryVariablePercentOfRange, false, p_rReadValueCmd.GetReadingType());
            p_rReadValueCmd.pDevices->ProcessReading(reading);

            if (p_rReadValueCmd.GetReadingType() == DeviceReading::ReadValue)
                p_rReadValueCmd.pCommands->SetCommandResponded(p_rReadValueCmd.dbCommand, nlib::CurrentUniversalTime(), GetFloatValStr(
                    (it->deviceVariable == DeviceVariableCodes_LoopCurrent) ? pwhResp->primaryVariableLoopCurrent
                        : pwhResp->primaryVariablePercentOfRange), NULL);

        }
        return true;
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[ProcessReadValueCmd2]: Application error: " << e.what());
        return false;
    }
    catch (...)
    {
        LOG_ERROR_APP("[ProcessReadValueCmd2]: System error!");
        return false;
    }
}

bool ResponseProcessor::ProcessReadValueCmd3(AppReadValueCmd& p_rReadValueCmd)
{
    try
    {
        struct timeval time;
        gettimeofday(&time, NULL);
        DeviceReading reading;

        PublishChannelSetT::iterator it = p_rReadValueCmd.GetChannelList().begin();
        if (it == p_rReadValueCmd.GetChannelList().end())
        {
            LOG_ERROR_APP("[ReadValue]: no channels for cmd=" << p_rReadValueCmd.GetCmdNo() << " and deviceID="
                        << p_rReadValueCmd.GetDeviceId());
            return false;
        }

        C003_ReadDynamicVariablesAndLoopCurrent_Resp *pwhResp =
                    (C003_ReadDynamicVariablesAndLoopCurrent_Resp*) (m_pResponse->whCmd.command);
        if (!pwhResp)
        {
            LOG_ERROR_APP("[ReadValue]: Incorrect response received for CMD003. DeviceID=" << p_rReadValueCmd.GetDeviceId());
            return false;
        }

        //send vars
        for (; it != p_rReadValueCmd.GetChannelList().end(); ++it)
        {
            if (it->cmdNo != p_rReadValueCmd.GetCmdNo())
            {
                LOG_ERROR_APP("[ReadValue]: Found bad cmdNo=" << ((int) it->cmdNo) << "in channelId=" << ((int) (it->channelID)));
                return false;
            }

            if (it->deviceVariable != DeviceVariableCodes_LoopCurrent && it->deviceVariable != DeviceVariableCodes_PrimaryVariable
                        && it->deviceVariable != DeviceVariableCodes_SecondaryVariable && it->deviceVariable
                        != DeviceVariableCodes_TertiaryVariable && it->deviceVariable != DeviceVariableCodes_QuaternaryVariable)
            {
                LOG_WARN_APP("[ReadValue]: Invalid variable " << (int) (it->deviceVariable) << " specified for cmd "
                            << ((int) (p_rReadValueCmd.GetCmdNo())));
                continue;
            }

            float varValue = 0;
            switch ((int) it->deviceVariable)
            {
                case DeviceVariableCodes_LoopCurrent:
                    varValue = pwhResp->primaryVariableLoopCurrent;
                break;
                case DeviceVariableCodes_PrimaryVariable:
                    varValue = pwhResp->variables[0].variable;
                break;
                case DeviceVariableCodes_SecondaryVariable:
                    varValue = pwhResp->variables[1].variable;
                break;
                case DeviceVariableCodes_TertiaryVariable:
                    varValue = pwhResp->variables[2].variable;
                break;
                case DeviceVariableCodes_QuaternaryVariable:
                    varValue = pwhResp->variables[3].variable;
                break;
                default:
                break;
            }

            InitDeviceReading(reading, p_rReadValueCmd.GetDeviceId(), it->channelID, p_rReadValueCmd.dbCommand.commandID, time, 0,
                varValue, false, p_rReadValueCmd.GetReadingType());
            p_rReadValueCmd.pDevices->ProcessReading(reading);

            if (p_rReadValueCmd.GetReadingType() == DeviceReading::ReadValue)
                p_rReadValueCmd.pCommands->SetCommandResponded(p_rReadValueCmd.dbCommand, nlib::CurrentUniversalTime(), GetFloatValStr(
                    varValue), NULL);
        }
        return true;
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[ProcessReadValueCmd3]: Application error: " << e.what());
        return false;
    }
    catch (...)
    {
        LOG_ERROR_APP("[ProcessReadValueCmd3]: System error!");
        return false;
    }
}

bool ResponseProcessor::ProcessReadValueCmd9(AppReadValueCmd& p_rReadValueCmd)
{
    try
    {
        struct timeval time;
        gettimeofday(&time, NULL);
        DeviceReading reading;

        PublishChannelSetT::iterator it = p_rReadValueCmd.GetChannelList().begin();
        if (it == p_rReadValueCmd.GetChannelList().end())
        {
            LOG_ERROR_APP("[ReadValue]: no channels for cmd=" << p_rReadValueCmd.GetCmdNo() << " and deviceID="
                        << p_rReadValueCmd.GetDeviceId());
            return false;
        }

        C009_ReadDeviceVariablesWithStatus_Resp *pwhResp = (C009_ReadDeviceVariablesWithStatus_Resp*) (m_pResponse->whCmd.command);
        if (!pwhResp)
        {
            LOG_ERROR_APP("[ReadValue]: Incorrect response received for CMD009. DeviceID=" << p_rReadValueCmd.GetDeviceId());
            return false;
        }

        //iterate by chanels; can have same device variable multiple times in the channels set (on different burst messages)
        for (; it != p_rReadValueCmd.GetChannelList().end(); ++it)
        {
            if (it->cmdNo != p_rReadValueCmd.GetCmdNo())
            {
                LOG_ERROR_APP("[ReadValue]: Found bad cmdNo=" << ((int) it->cmdNo) << "in channelId=" << (int) (it->channelID));
                return false;
            }

            //check if our resp has the data necessary to write this channel (we might not have received the necessary variable)
            const Slot* slot = FindMatchingVariableSlot(*pwhResp, *it);
            if (slot == 0)
            {
                continue;
            }

            InitDeviceReading(reading, p_rReadValueCmd.GetDeviceId(), it->channelID, p_rReadValueCmd.dbCommand.commandID, time,
                slot->deviceVariableStatus, slot->deviceVariableValue, true, p_rReadValueCmd.GetReadingType());

            p_rReadValueCmd.pDevices->ProcessReading(reading);

            if (p_rReadValueCmd.GetReadingType() == DeviceReading::ReadValue)
                p_rReadValueCmd.pCommands->SetCommandResponded(p_rReadValueCmd.dbCommand, nlib::CurrentUniversalTime(), GetFloatValStr(
                    slot->deviceVariableValue, slot->deviceVariableStatus), NULL);

        }
        return true;
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[ProcessReadValueCmd9]: Application error: " << e.what());
        return false;
    }
    catch (...)
    {
        LOG_ERROR_APP("[ProcessReadValueCmd9]: System error!");
        return false;
    }
}

bool ResponseProcessor::ProcessReadValueCmd33(AppReadValueCmd& p_rReadValueCmd)
{
    try
    {
        struct timeval time;
        gettimeofday(&time, NULL);
        DeviceReading reading;

        PublishChannelSetT::iterator it = p_rReadValueCmd.GetChannelList().begin();
        if (it == p_rReadValueCmd.GetChannelList().end())
        {
            LOG_ERROR_APP("[ReadValue]: no channels for cmd=" << p_rReadValueCmd.GetCmdNo() << " and deviceID="
                        << p_rReadValueCmd.GetDeviceId());
            return false;
        }

        C033_ReadDeviceVariables_Resp *pwhResp = (C033_ReadDeviceVariables_Resp*) (m_pResponse->whCmd.command);
        if (!pwhResp)
        {
            LOG_ERROR_APP("[ReadValue]: Incorrect response received for CMD033. DeviceID=" << p_rReadValueCmd.GetDeviceId());
            return false;
        }

        //send vars
        for (; it != p_rReadValueCmd.GetChannelList().end(); ++it)
        {
            if (it->cmdNo != p_rReadValueCmd.GetCmdNo())
            {
                LOG_ERROR_APP("[ReadValue]: Found bad cmdNo=" << ((int) it->cmdNo) << "in channelId=" << (int) (it->channelID));
                return false;
            }

            const float* varValue = FindMatchingVariableValue(*pwhResp, *it);
            if (varValue == 0)
            {
                continue;
            }

            InitDeviceReading(reading, p_rReadValueCmd.GetDeviceId(), it->channelID, p_rReadValueCmd.dbCommand.commandID, time, 0,
                *varValue, false, p_rReadValueCmd.GetReadingType());

            p_rReadValueCmd.pDevices->ProcessReading(reading);

            if (p_rReadValueCmd.GetReadingType() == DeviceReading::ReadValue)
                p_rReadValueCmd.pCommands->SetCommandResponded(p_rReadValueCmd.dbCommand, nlib::CurrentUniversalTime(), GetFloatValStr(
                    pwhResp->variables[it->deviceVariableSlot].value), NULL);
        }
        return true;
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[ProcessReadValueCmd33]: Application error: " << e.what());
        return false;
    }
    catch (...)
    {
        LOG_ERROR_APP("[ProcessReadValueCmd33]: System error!");
        return false;
    }
}

bool ResponseProcessor::ProcessReadValueCmd178(AppReadValueCmd& p_rReadValueCmd)
{
    try
    {
        struct timeval time;
        gettimeofday(&time, NULL);
        DeviceReading reading;

        PublishChannelSetT::iterator it = p_rReadValueCmd.GetChannelList().begin();
        if (it == p_rReadValueCmd.GetChannelList().end())
        {
            LOG_ERROR_APP("[ReadValue]: no channels for cmd=" << p_rReadValueCmd.GetCmdNo() << " and deviceID=" << p_rReadValueCmd.GetDeviceId());
            return false;
        }

        C178_PublishedDynamicData_Resp *pwhResp = (C178_PublishedDynamicData_Resp*) (m_pResponse->whCmd.command);
        if (!pwhResp)
        {
            LOG_ERROR_APP("[ReadValue]: Incorrect response received for CMD178. DeviceID=" << p_rReadValueCmd.GetDeviceId());
            return false;
        }

        //iterate by chanels; can have same device variable multiple times in the channels set (on different burst messages)
        for (; it != p_rReadValueCmd.GetChannelList().end(); ++it)
        {
            if (it->cmdNo != p_rReadValueCmd.GetCmdNo())
            {
                LOG_ERROR_APP("[ReadValue]: Found bad cmdNo=" << ((int) it->cmdNo) << "in channelId=" << (int) (it->channelID));
                return false;
            }
            DevicePtr device = p_rReadValueCmd.pDevices->FindDevice(p_rReadValueCmd.GetDeviceId());
            const float* varValue = FindMatchingVariableValue(*pwhResp, device,  *it);
            LOG_DEBUG_APP("[ReadValue]: DeviceID[deviceId=" << p_rReadValueCmd.GetDeviceId()
                    << "] = {cmdNo=" << ((int) it->cmdNo)
                    << ", channelName=" << it->name
                    << ", VarCodes_Primary=" << (int)pwhResp->VarCodes_Primary
                    << ", VarCodes_Secondary=" << (int)pwhResp->VarCodes_Secondary
                    << ", VarCodes_Tertiary=" << (int)pwhResp->VarCodes_Tertiary
                    << ", VarCodes_Quaternary=" << (int)pwhResp->VarCodes_Quaternary
                    << ", value=" << (varValue == 0 ? -888.88 : *varValue) << "}");
            if (varValue == 0)
            {
                continue;
            }

            InitDeviceReading(reading, p_rReadValueCmd.GetDeviceId(), it->channelID, p_rReadValueCmd.dbCommand.commandID, time, 0,
                *varValue, false, p_rReadValueCmd.GetReadingType());

            p_rReadValueCmd.pDevices->ProcessReading(reading);

            if (p_rReadValueCmd.GetReadingType() == DeviceReading::ReadValue)
                p_rReadValueCmd.pCommands->SetCommandResponded(p_rReadValueCmd.dbCommand, nlib::CurrentUniversalTime(),
                    GetFloatValStr(*varValue), NULL);
        }
        return true;

    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[ProcessReadValueCmd178]: Application error: " << e.what());
        return false;
    }
    catch (...)
    {
        LOG_ERROR_APP("[ProcessReadValueCmd178]: System error!");
        return false;
    }
}

DeviceReading* ResponseProcessor::InitDeviceReading( DeviceReading& p_rReading, /*output param*/
    int p_nDeviceID,
    int p_nChannelNo,
    int p_nCommandID,
    struct timeval& p_rTv,
    unsigned char p_nValueStatus,
    float p_flValue,
    bool p_bHasStatus,
    DeviceReading::ReadingType p_enumReadingType)
{
    p_rReading.m_deviceID = p_nDeviceID;
    p_rReading.m_channelNo = p_nChannelNo;
    p_rReading.m_nCommandID = ( (p_enumReadingType == DeviceReading::BurstValue) ? (-1) : p_nCommandID );
    p_rReading.m_tv = p_rTv;
    p_rReading.m_valueStatus = p_nValueStatus;
    p_rReading.m_value = p_flValue;
    p_rReading.m_hasStatus = p_bHasStatus;
    p_rReading.m_readingType = p_enumReadingType;

    return &p_rReading;
}

//general wh cmd
void ResponseProcessor::Visit(AppGeneralCommand& p_rGeneralCommand)
{
    //LOG_INFO_APP("[ResponseProcessor::Visit]:AppGeneralCommand. Device=");
    try
    {
        if (m_pResponse->hostErr != gateway::GatewayIO::HostSuccess)
        {
            LOG_ERROR_APP("[GeneralCommand]: hostError=" << (int) m_pResponse->hostErr);
            SaveRespErr(*p_rGeneralCommand.pCommands, p_rGeneralCommand.dbCommand).CommandFailed(
                (DBCommand::ResponseStatus) ((int) DBCommand::rsFailure_HostReceptionErrorBase + m_pResponse->hostErr));
            return;
        }

        DevicePtr fromDevice = p_rGeneralCommand.pDevices->FindDevice((int) (p_rGeneralCommand.GetDeviceID()));
        if (!fromDevice)
        {
            LOG_ERROR_APP("[GeneralCommand]: device id= " << p_rGeneralCommand.GetDeviceID() << "not found");
            SaveRespErr(*p_rGeneralCommand.pCommands, p_rGeneralCommand.dbCommand).CommandFailed(DBCommand::rsFailure_InvalidDevice);
            return;
        }

        std::string szCommand((char*) m_pResponse->whCmd.command);
        p_rGeneralCommand.pCommands->SetCommandResponded(p_rGeneralCommand.dbCommand, nlib::CurrentUniversalTime(), szCommand, NULL);
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Visit::AppGeneralCommand]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Visit::AppGeneralCommand]: System error!");
    }
}

//reports
void ResponseProcessor::Visit(AppRoutesReportCmd& p_rRoutesCmd)
{
    //LOG_INFO_APP("[ResponseProcessor::Visit]:AppRoutesReportCmd. Device=");
    try
    {
        switch (p_rRoutesCmd.m_enumState)
        {
            case AppRoutesReportCmd::GetRouteLists_State:
                LOG_DEBUG_APP("[RoutesList]: routes received for deviceID=" << m_pResponse->appData.innerDataHandle);
                ProcessRouteListReport(p_rRoutesCmd);

                //send source route requests
                if (p_rRoutesCmd.ReceivedAllRouteListReports())
                {
                    //if gw's disconnected, the command with multiple states should stop
                    if (m_pResponse->hostErr == gateway::GatewayIO::HostDisconnected)
                    {
                        if (p_rRoutesCmd.dbCommand.generatedType == DBCommand::cgtAutomatic)
                            p_rRoutesCmd.pDevices->ChangeRoutes(p_rRoutesCmd.GetRoutes(), true);
                        SaveRespErr(*p_rRoutesCmd.pCommands, p_rRoutesCmd.dbCommand).SaveErr_802(m_pResponse->hostErr, m_pResponse->whCmd.responseCode);
                        return;
                    }

                    LOG_DEBUG_APP("[RoutesList]: all routes received, so send source_routes_cmd");
                    AppRoutesReportCmd::RouteIdsPerDevice::iterator it = p_rRoutesCmd.m_oRouteIdsPerDevice.begin();
                    for (; it != p_rRoutesCmd.m_oRouteIdsPerDevice.end(); ++it)
                    {
                        //for each route ID in the list
                        AppRoutesReportCmd::RouteIdsPerDevice::mapped_type::iterator routeIt = it->second.begin();
                        for (; routeIt != it->second.end(); ++routeIt)
                        {
                            SendReadSourceRouteRequest(it->first, p_rRoutesCmd, *routeIt);
                        }
                    }
                }
            break;
            case AppRoutesReportCmd::GetSourceRoutes_State:
                LOG_DEBUG_APP("[RoutesList]: source_routes received for deviceID=" << m_pResponse->appData.innerDataHandle);
                ProcessSourceRouteReport(p_rRoutesCmd);

                if (p_rRoutesCmd.ReceivedAllSourceRouteReports())
                {
                    if (p_rRoutesCmd.dbCommand.generatedType == DBCommand::cgtAutomatic)
                    {
                        p_rRoutesCmd.pDevices->ChangeSourceRoutes(p_rRoutesCmd.GetSourceRoutes(), true);
                        p_rRoutesCmd.pDevices->ChangeRoutes(p_rRoutesCmd.GetRoutes(), true);
                    }
                    p_rRoutesCmd.pCommands->SetCommandResponded(p_rRoutesCmd.dbCommand, nlib::CurrentUniversalTime(), "success", NULL);
                }
            break;
        }
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Visit::AppRoutesReportCmd]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Visit::AppRoutesReportCmd]: System error!");
    }
}

void ResponseProcessor::SendReadSourceRouteRequest(const DevicePtr& p_rDevice, AppRoutesReportCmd& p_rRoutesCmd, int p_nRouteId)
{
    C803_ReadSourceRoute_Req whReq;
    whReq.m_ucRouteId = p_nRouteId;

    stack::WHartCommand whCmd;
    whCmd.commandID = CMDID_C803_ReadSourceRoute;

    whCmd.command = &whReq;

    gateway::GatewayIO::AppData appData;
    appData.appCmd = m_pResponse->appData.appCmd;
    appData.innerDataHandle = p_rDevice->id;

    //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C803_ReadSourceRoute. Device=" << p_rDevice->Mac());
    m_pProcessor->gateway.SendWHRequest(appData, p_rDevice->Mac().Address(), whCmd, false, true);
}

void ResponseProcessor::ProcessRouteListReport(AppRoutesReportCmd& p_rRoutesCmd)
{
    try
    {
        DevicePtr device = p_rRoutesCmd.pDevices->FindDevice(m_pResponse->appData.innerDataHandle);
        if ((device == 0) || SaveRespErr::IsErr_802(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            //there could be gathered some till this error
            if (p_rRoutesCmd.dbCommand.generatedType == DBCommand::cgtManual)
                p_rRoutesCmd.pDevices->ReplaceRoutes(m_pResponse->appData.innerDataHandle, p_rRoutesCmd.GetRoutes());
            p_rRoutesCmd.RegisterRouteListReport();
            return;
        }

        C802_ReadRouteList_Resp *pResp = (C802_ReadRouteList_Resp*) m_pResponse->whCmd.command;
        if (!pResp)
        {
            LOG_ERROR_APP("[RoutesList]: Incorrect response received for CMD802.");
            return;
        }

        //save routes in db and cache the route id
        for (int i = 0; i < pResp->m_ucNoOfEntriesRead; i++)
        {
            DevicePtr dev = p_rRoutesCmd.pDevices->FindDevice(NickName((WHartShortAddress) pResp->m_aRoutes[i].destinationNickname));
            if (!dev)
            {
                LOG_WARN_APP("[RoutesList]: unknown nickname=" << (int) pResp->m_aRoutes[i].destinationNickname);
                continue;
            }
            Route r(pResp->m_aRoutes[i].routeId, m_pResponse->appData.innerDataHandle,
            //pResp->m_aRoutes[i].destinationNickname,     //THIS SHOULD NOT BE USED
                dev->id, pResp->m_aRoutes[i].graphId, pResp->m_aRoutes[i].sourceRouteAttached);

            p_rRoutesCmd.GetRoutes().push_back(r);
            if (r.m_nSourceRoute != 0)
            {
                p_rRoutesCmd.RegisterRoute(device, pResp->m_aRoutes[i].routeId);
            }
        }

        //see if there still are routes to be read
        if (pResp->m_ucNoOfActiveRoutes - (pResp->m_ucRouteIndex + pResp->m_ucNoOfEntriesRead) != pResp->m_ucNoOfRoutesRemaining)
        {
            LOG_WARN_APP("[RoutesList] Response to 802 cmd contains invalid data (incorrect number of remaining routes)");
        }

        if (pResp->m_ucNoOfRoutesRemaining != 0)
        {
            SendRoutesListRequests(device, pResp->m_ucRouteIndex + pResp->m_ucNoOfEntriesRead, pResp->m_ucNoOfRoutesRemaining,
                m_pResponse->appData.appCmd, m_pProcessor);
        }
        else
        {
            //we finished for this device
            if (p_rRoutesCmd.dbCommand.generatedType == DBCommand::cgtManual)
                p_rRoutesCmd.pDevices->ReplaceRoutes(m_pResponse->appData.innerDataHandle, p_rRoutesCmd.GetRoutes());
            p_rRoutesCmd.RegisterRouteListReport();
        }
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[ProcessRouteListReport]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[ProcessRouteListReport]: System error!");
    }
}

void ResponseProcessor::ProcessSourceRouteReport(AppRoutesReportCmd& p_rRoutesCmd)
{
    try
    {
        p_rRoutesCmd.RegisterSourceRouteReport();
        DevicePtr device = p_rRoutesCmd.pDevices->FindDevice(m_pResponse->appData.innerDataHandle);
        if (device == 0 || SaveRespErr::IsErr_803(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            return;
        }

        C803_ReadSourceRoute_Resp *pResp = (C803_ReadSourceRoute_Resp*) m_pResponse->whCmd.command;
        if (!pResp)
        {
            LOG_ERROR_APP("[SourceRoutes]: Incorrect response received for CMD803. DeviceMAC=" << device->Mac());
            return;
        }

        /*compose devices string*/
        char* devicesList = new char[pResp->m_ucNoOfHops * 4/*nb of hexchars per hop*/+ 1/*string terminator*/];
        memset(devicesList, 0, pResp->m_ucNoOfHops * 4/*nb of hexchars per hop*/+ 1/*string terminator*/);

        for (int i = 0; i < pResp->m_ucNoOfHops; i++)
        {
            sprintf(devicesList + i * 4, "%04X", pResp->m_aHopNicknames[i]);
        }

        SourceRoute sr(device->id, pResp->m_ucRouteId, devicesList);

        p_rRoutesCmd.GetSourceRoutes().push_back(sr);

        delete[] devicesList;

        //we finished for this device
        if (p_rRoutesCmd.dbCommand.generatedType == DBCommand::cgtAutomatic)
        {
            p_rRoutesCmd.pDevices->ChangeSourceRoutes(p_rRoutesCmd.GetSourceRoutes(), true);
            p_rRoutesCmd.pDevices->ChangeRoutes(p_rRoutesCmd.GetRoutes(), true);
        }
        else
            p_rRoutesCmd.pDevices->ReplaceSourceRoutes(m_pResponse->appData.innerDataHandle, p_rRoutesCmd.GetSourceRoutes());
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[ProcessSourceRouteReport]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[ProcessSourceRouteReport]: System error!");
    }
}

void ResponseProcessor::Visit(AppServicesReportCmd& p_rServicesCmd)
{
    //LOG_INFO_APP("[ResponseProcessor::Visit]:AppServicesReportCmd. Device=");
    try
    {
        LOG_DEBUG_APP("[ServicesList]: services received for deviceID=" << m_pResponse->appData.innerDataHandle);

        if (SaveRespErr::IsErr_800(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        { //there could be gathered some till this error
            p_rServicesCmd.ReportReceived();
            if (p_rServicesCmd.dbCommand.generatedType == DBCommand::cgtManual)
                p_rServicesCmd.pDevices->ReplaceServices(m_pResponse->appData.innerDataHandle, p_rServicesCmd.GetServices());
            if (p_rServicesCmd.ReceivedAllReports())
            {
                if (p_rServicesCmd.dbCommand.generatedType == DBCommand::cgtAutomatic)
                    p_rServicesCmd.pDevices->ChangeServices(p_rServicesCmd.GetServices(), true);
                p_rServicesCmd.pCommands->SetCommandResponded(p_rServicesCmd.dbCommand, nlib::CurrentUniversalTime(), "success", NULL);
            }
            return;
        }

        C800_ReadServiceList_Resp *pResp = (C800_ReadServiceList_Resp*) m_pResponse->whCmd.command;
        if (!pResp)
        {
            LOG_ERROR_APP("[ServiceList]: Incorrect response received for CMD800. DeviceID=" << m_pResponse->appData.innerDataHandle);
            return;
        }

        //save services
        for (int i = 0; i < pResp->m_ucNoOfEntriesRead; i++)
        {
            NickName nick((WHartShortAddress) pResp->m_aServices[i].nicknameOfPeer);
            DevicePtr dev = p_rServicesCmd.pDevices->FindDevice(nick);
            if (!dev)
            {
                LOG_WARN_APP("[ServicesList]: unknown nickname=" << nick);
                continue;
            }
            Service s(pResp->m_aServices[i].serviceId, m_pResponse->appData.innerDataHandle, dev->id, (uint8_t)(
                pResp->m_aServices[i].serviceApplicationDomain), ((pResp->m_aServices[i].serviceRequestFlags
                        & ServiceRequestFlagsMask_Source) != 0),
                ((pResp->m_aServices[i].serviceRequestFlags & ServiceRequestFlagsMask_Sink) != 0),
                ((pResp->m_aServices[i].serviceRequestFlags & ServiceRequestFlagsMask_Intermittent) != 0),
                pResp->m_aServices[i].period.u32, pResp->m_aServices[i].routeId);

            p_rServicesCmd.GetServices().push_back(s);

        }

        if (pResp->m_ucNoOfActiveServices > pResp->m_ucServiceIndex + pResp->m_ucNoOfEntriesRead)
        {
            C800_ReadServiceList_Req whReq;
            whReq.m_ucServiceIndex = pResp->m_ucServiceIndex + pResp->m_ucNoOfEntriesRead;
            whReq.m_ucNoOfEntriesToRead = pResp->m_ucNoOfActiveServices - (pResp->m_ucServiceIndex + pResp->m_ucNoOfEntriesRead);

            stack::WHartCommand whCmd;
            whCmd.commandID = CMDID_C800_ReadServiceList;
            whCmd.command = &whReq;

            gateway::GatewayIO::AppData appData;
            appData.appCmd = m_pResponse->appData.appCmd;
            appData.innerDataHandle = m_pResponse->appData.innerDataHandle;

            DevicePtr device = (*p_rServicesCmd.pDevices).FindDevice(m_pResponse->appData.innerDataHandle);
            if (!device)
                p_rServicesCmd.ReportReceived();
            else
            {
                //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C803_ReadSourceRoute. Device=" << device->Mac());
                m_pProcessor->gateway.SendWHRequest(appData, device->Mac().Address(), whCmd, false, true);
            }
        }
        else
        {
            p_rServicesCmd.ReportReceived();
            //we finished for this device
            if (p_rServicesCmd.dbCommand.generatedType == DBCommand::cgtManual)
                p_rServicesCmd.pDevices->ReplaceServices(m_pResponse->appData.innerDataHandle, p_rServicesCmd.GetServices());
        }

        if (p_rServicesCmd.ReceivedAllReports())
        {
            if (p_rServicesCmd.dbCommand.generatedType == DBCommand::cgtAutomatic)
                p_rServicesCmd.pDevices->ChangeServices(p_rServicesCmd.GetServices(), true);
            p_rServicesCmd.pCommands->SetCommandResponded(p_rServicesCmd.dbCommand, nlib::CurrentUniversalTime(), "success", NULL);
        }
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Visit::AppServicesReportCmd]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Visit::AppServicesReportCmd]: System error!");
    }
}

void ResponseProcessor::Visit(AppDeviceHealthReportCmd& p_rDeviceHealthCmd)
{
    //LOG_INFO_APP("[ResponseProcessor::Visit]:AppDeviceHealthReportCmd. Device=");
    try
    {
        LOG_DEBUG_APP("[DeviceHealthReport]: recived");
        p_rDeviceHealthCmd.ReportReceived();
        if (SaveRespErr::IsErr_779(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            if (p_rDeviceHealthCmd.ReceivedAllReports())
                p_rDeviceHealthCmd.pCommands->SetCommandResponded(p_rDeviceHealthCmd.dbCommand, nlib::CurrentUniversalTime(), "success",
                    NULL);
            return;
        }

        C779_ReportDeviceHealth_Resp *pResp = (C779_ReportDeviceHealth_Resp*) m_pResponse->whCmd.command;
        if (!pResp)
        {
            LOG_ERROR_APP("[DeviceHealthReport]: Incorrect response received for CMD779. DeviceMAC="
                        << p_rDeviceHealthCmd.GetDevicesHealth()[m_pResponse->appData.innerDataHandle].m_oDeviceMac);
            return;
        }

        std::vector<DeviceHealth>& devicesHealth = p_rDeviceHealthCmd.GetDevicesHealth();
        DeviceHealth devHealth(devicesHealth[m_pResponse->appData.innerDataHandle].m_nDeviceId,
            devicesHealth[m_pResponse->appData.innerDataHandle].m_oDeviceMac, pResp->m_ucPowerStatus,
            pResp->m_unNoOfPacketsGeneratedByDevice, pResp->m_unNoOfPacketsTerminatedByDevice,
            pResp->m_ucNoOfDataLinkLayerMICFailuresDetected, pResp->m_ucNoOfDataLinkLayerMICFailuresDetected,
            pResp->m_ucNoOfCRCErrorsDetected, pResp->m_ucNoOfNonceCounterValuesNotReceived);

        devicesHealth[m_pResponse->appData.innerDataHandle] = devHealth;

        //we finished for this device
        p_rDeviceHealthCmd.pDevices->ReplaceDevicesHealthReport(p_rDeviceHealthCmd.GetDevicesHealth());

        if (p_rDeviceHealthCmd.ReceivedAllReports())
            p_rDeviceHealthCmd.pCommands->SetCommandResponded(p_rDeviceHealthCmd.dbCommand, nlib::CurrentUniversalTime(), "success", NULL);
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Visit::AppDeviceHealthReportCmd]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Visit::AppDeviceHealthReportCmd]: System error!");
    }
}

void ResponseProcessor::Visit(AppSuperframesReportCmd& p_rSuperframesCmd)
{
    //LOG_INFO_APP("[ResponseProcessor::Visit]:AppSuperframesReportCmd. Device=");
    try
    {
        LOG_DEBUG_APP("[SuperframesList]: superframes received for deviceID=" << m_pResponse->appData.innerDataHandle);

        if (SaveRespErr::IsErr_783(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            p_rSuperframesCmd.ReportReceived();
            //there could be gathered some till this error
            if (p_rSuperframesCmd.dbCommand.generatedType == DBCommand::cgtManual)
                p_rSuperframesCmd.pDevices->ReplaceSuperframes(m_pResponse->appData.innerDataHandle, p_rSuperframesCmd.GetSuperframes());
            if (p_rSuperframesCmd.ReceivedAllReports())
            {
                if (p_rSuperframesCmd.dbCommand.generatedType == DBCommand::cgtAutomatic)
                    p_rSuperframesCmd.pDevices->ChangeSuperframes(p_rSuperframesCmd.GetSuperframes(), true);
                p_rSuperframesCmd.pCommands->SetCommandResponded(p_rSuperframesCmd.dbCommand, nlib::CurrentUniversalTime(), "success", NULL);
            }
            return;
        }

        C783_ReadSuperframeList_Resp *pResp = (C783_ReadSuperframeList_Resp*) m_pResponse->whCmd.command;
        if (!pResp)
        {
            LOG_ERROR_APP("[SuperframesList]: Incorrect response received for CMD783. DeviceID=" << m_pResponse->appData.innerDataHandle);
            return;
        }

        //save Superframes
        for (int i = 0; i < pResp->m_ucNoOfEntriesRead; i++)
        {
            Superframe sFrame(pResp->m_aSuperframes[i].superframeId, m_pResponse->appData.innerDataHandle,
                pResp->m_aSuperframes[i].noOfSlotsInSuperframe, ((pResp->m_aSuperframes[i].superframeModeFlags
                            & SuperframeModeFlagsMask_Active) != 0), ((pResp->m_aSuperframes[i].superframeModeFlags
                            & SuperframeModeFlagsMask_HandheldSuperframe) != 0));
            p_rSuperframesCmd.GetSuperframes().push_back(sFrame);
        }

        if (pResp->m_ucNoOfActiveSuperframes > pResp->m_ucSuperframeIndex + pResp->m_ucNoOfEntriesRead)
        {
            C783_ReadSuperframeList_Req whReq;
            whReq.m_ucSuperframeIndex = pResp->m_ucSuperframeIndex + pResp->m_ucNoOfEntriesRead;
            whReq.m_ucNoOfEntriesToRead = pResp->m_ucNoOfActiveSuperframes - (pResp->m_ucSuperframeIndex + pResp->m_ucNoOfEntriesRead);

            stack::WHartCommand whCmd;
            whCmd.commandID = CMDID_C783_ReadSuperframeList;
            whCmd.command = &whReq;

            gateway::GatewayIO::AppData appData;
            appData.appCmd = m_pResponse->appData.appCmd;
            appData.innerDataHandle = m_pResponse->appData.innerDataHandle;

            DevicePtr device = (*p_rSuperframesCmd.pDevices).FindDevice(m_pResponse->appData.innerDataHandle);
            if (!device)
                p_rSuperframesCmd.ReportReceived();
            else
            {
                //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C783_ReadSuperframeList. Device=" << device->Mac());
                m_pProcessor->gateway.SendWHRequest(appData, device->Mac().Address(), whCmd, false, true);
            }
        }
        else
        {
            p_rSuperframesCmd.ReportReceived();
            //we finished for this device
            if (p_rSuperframesCmd.dbCommand.generatedType == DBCommand::cgtManual)
                p_rSuperframesCmd.pDevices->ReplaceSuperframes(m_pResponse->appData.innerDataHandle, p_rSuperframesCmd.GetSuperframes());
        }

        if (p_rSuperframesCmd.ReceivedAllReports())
        {
            if (p_rSuperframesCmd.dbCommand.generatedType == DBCommand::cgtAutomatic)
                p_rSuperframesCmd.pDevices->ChangeSuperframes(p_rSuperframesCmd.GetSuperframes(), true);
            p_rSuperframesCmd.pCommands->SetCommandResponded(p_rSuperframesCmd.dbCommand, nlib::CurrentUniversalTime(), "success", NULL);
        }
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Visit::AppSuperframesReportCmd]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Visit::AppSuperframesReportCmd]: System error!");
    }
}

void ResponseProcessor::Visit(AppDeviceScheduleLinksReportCmd& p_rDeviceScheduleLinksCmd)
{
    //LOG_INFO_APP("[ResponseProcessor::Visit]:AppDeviceScheduleLinksReportCmd. Device=");
    try
    {
        LOG_DEBUG_APP("[ScheduleLinks]: schedule_links received for deviceID="
                    << p_rDeviceScheduleLinksCmd.m_oDevicesLinkList[m_pResponse->appData.innerDataHandle].m_nDeviceId);

        if (SaveRespErr::IsErr_784(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            p_rDeviceScheduleLinksCmd.ReportReceived();
            if (p_rDeviceScheduleLinksCmd.ReceivedAllReports())
            {
                if (p_rDeviceScheduleLinksCmd.dbCommand.generatedType == DBCommand::cgtAutomatic)
                {
                    DevicesScheduleLinks& devicesSchedLinks = p_rDeviceScheduleLinksCmd.GetDevicesScheduleLinks();
                    for (DevicesScheduleLinks::iterator i = devicesSchedLinks.begin(); i != devicesSchedLinks.end(); ++i)
                        p_rDeviceScheduleLinksCmd.pDevices->ChangeDevicesScheduleLinks(i->m_nDeviceId, i->linkList, true);
                }
                else
                {
                    p_rDeviceScheduleLinksCmd.pDevices->ReplaceDevicesScheduleLinks(p_rDeviceScheduleLinksCmd.GetDevicesScheduleLinks());
                }
                p_rDeviceScheduleLinksCmd.pCommands->SetCommandResponded(p_rDeviceScheduleLinksCmd.dbCommand, nlib::CurrentLocalTime(),
                    "success", NULL);
            }
            return;
        }

        C784_ReadLinkList_Resp *pResp = (C784_ReadLinkList_Resp*) m_pResponse->whCmd.command;
        if (!pResp)
        {
            LOG_ERROR_APP("[ScheduleLinks]: Incorrect response received for CMD784. DeviceMAC="
                        << p_rDeviceScheduleLinksCmd.m_oDevicesLinkList[m_pResponse->appData.innerDataHandle].m_oDeviceMac);
            return;
        }

        //save DeviceScheduleLinks
        const uint16_t broadcastNN = 0xFFFF;
        int devID = -1;
        for (int i = 0; i < pResp->m_ucNoOfLinksRead; i++)
        {
            if (broadcastNN != pResp->m_aLinks[i].nicknameOfNeighborForThisLink)
            {
                DevicePtr dev = p_rDeviceScheduleLinksCmd.pDevices->FindDevice(NickName(
                    (WHartShortAddress) pResp->m_aLinks[i].nicknameOfNeighborForThisLink));
                if (!dev)
                {
                    LOG_WARN_APP("[ScheduleLinks]: unknown nickname=" << (int) pResp->m_aLinks[i].nicknameOfNeighborForThisLink);
                    continue;
                }
                devID = dev->id;
            }

            DeviceScheduleLink link(pResp->m_aLinks[i].superframeId, devID, pResp->m_aLinks[i].slotNoForThisLink,
                pResp->m_aLinks[i].channelOffsetForThisLink, ((pResp->m_aLinks[i].linkOptions & LinkOptionFlagCodesMask_Transmit) != 0),
                ((pResp->m_aLinks[i].linkOptions & LinkOptionFlagCodesMask_Receive) != 0), ((pResp->m_aLinks[i].linkOptions
                            & LinkOptionFlagCodesMask_Shared) != 0), pResp->m_aLinks[i].linkType);
            p_rDeviceScheduleLinksCmd.AddDeviceScheduleLink(m_pResponse->appData.innerDataHandle, link);
        }

        if (pResp->m_unNoOfActiveLinks > pResp->m_unLinkIndex + pResp->m_ucNoOfLinksRead)
        {
            C784_ReadLinkList_Req whReq;
            whReq.m_unLinkIndex = pResp->m_unLinkIndex + pResp->m_ucNoOfLinksRead;
            whReq.m_ucNoOfLinksToRead = pResp->m_unNoOfActiveLinks - (pResp->m_unLinkIndex + pResp->m_ucNoOfLinksRead);

            stack::WHartCommand whCmd;
            whCmd.commandID = CMDID_C784_ReadLinkList;
            whCmd.command = &whReq;

            gateway::GatewayIO::AppData appData;
            appData.appCmd = m_pResponse->appData.appCmd;
            appData.innerDataHandle = m_pResponse->appData.innerDataHandle;

            DevicePtr device = (*p_rDeviceScheduleLinksCmd.pDevices).FindDevice(m_pResponse->appData.innerDataHandle);
            if (!device)
                p_rDeviceScheduleLinksCmd.ReportReceived();
            else
            {
                //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C784_ReadLinkList. Device=" << device->Mac());
                m_pProcessor->gateway.SendWHRequest(appData, device->Mac().Address(), whCmd, false, true);
            }
        }
        else
        {
            p_rDeviceScheduleLinksCmd.ReportReceived();
        }

        if (p_rDeviceScheduleLinksCmd.ReceivedAllReports())
        {
            if (p_rDeviceScheduleLinksCmd.dbCommand.generatedType == DBCommand::cgtAutomatic)
            {
                DevicesScheduleLinks& devicesSchedLinks = p_rDeviceScheduleLinksCmd.GetDevicesScheduleLinks();
                for (DevicesScheduleLinks::iterator i = devicesSchedLinks.begin(); i != devicesSchedLinks.end(); ++i)
                    p_rDeviceScheduleLinksCmd.pDevices->ChangeDevicesScheduleLinks(i->m_nDeviceId, i->linkList, true);
            }
            else
            {
                p_rDeviceScheduleLinksCmd.pDevices->ReplaceDevicesScheduleLinks(p_rDeviceScheduleLinksCmd.GetDevicesScheduleLinks());
            }
            p_rDeviceScheduleLinksCmd.pCommands->SetCommandResponded(p_rDeviceScheduleLinksCmd.dbCommand, nlib::CurrentLocalTime(),
                "success", NULL);
        }
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Visit::AppDeviceScheduleLinksReportCmd]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Visit::AppDeviceScheduleLinksReportCmd]: System error!");
    }
}

void ResponseProcessor::Visit(AppNeighborHealthReportCmd& p_rNeighborHealthCmd)
{
    //LOG_INFO_APP("[ResponseProcessor::Visit]:AppNeighborHealthReportCmd. Device=");
    try
    {
        DeviceNeighborsHealth& rNeighborHealths = p_rNeighborHealthCmd.m_oNeighborsHealthCache[m_pResponse->appData.innerDataHandle];
        LOG_DEBUG_APP("[NeighborHealthsList]: neighborHealths received for deviceID=" << rNeighborHealths.m_nDeviceID);
        if (SaveRespErr::IsErr_780(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            p_rNeighborHealthCmd.ReportReceived();
            if (p_rNeighborHealthCmd.ReceivedAllReports())
            {
                p_rNeighborHealthCmd.pDevices->ReplaceDevicesNeighborsHealth(p_rNeighborHealthCmd.GetDevicesNeighbHealth());
                p_rNeighborHealthCmd.pCommands->SetCommandResponded(p_rNeighborHealthCmd.dbCommand, nlib::CurrentUniversalTime(),
                    "success", NULL);
                if (p_rNeighborHealthCmd.IsToBeAccessPoint()) //update in db if it's just for one device
                {
                    DevicePtr dev = p_rNeighborHealthCmd.pDevices->FindDevice(p_rNeighborHealthCmd.GetDevicesNeighbHealth()[0].m_oMac);
                    if (dev && dev->Type() != Device::dtAccessPoint)
                    {
                        dev->Type(Device::dtAccessPoint);
                        p_rNeighborHealthCmd.pDevices->UpdateDevice(dev, false, false);
                    }
                }
            }
            return;
        }

        C780_ReportNeighborHealthList_Resp *pResp = (C780_ReportNeighborHealthList_Resp*) m_pResponse->whCmd.command;
        if (!pResp)
        {
            LOG_ERROR_APP("[NeighborHealthsList]: Incorrect response received for CMD780. DeviceMAC=" << rNeighborHealths.m_oMac);
            return;
        }

        //init deviceNeighborHealths
        for (int i = 0; i < pResp->m_ucNoOfNeighborEntriesRead; i++)
        {
            NickName nick(pResp->m_aNeighbors[i].nicknameOfNeighbor);
            if (p_rNeighborHealthCmd.m_oNeighborsHealthCache.size() == 1) //if it's just for one device
            {
                //SET ACCESS_POINT TYPE
                if (p_rNeighborHealthCmd.IsDevice())
                {
                    if (nick.Address() == stack::Gateway_Nickname())
                        p_rNeighborHealthCmd.SetIsToBeAccessPoint();
                }
            }

            DevicePtr dev = p_rNeighborHealthCmd.pDevices->FindDevice(nick);
            if (!dev)
            {
                LOG_WARN_APP("[NeighborHealthsList]: not found in db the neighbor ="
                            << (NickName(pResp->m_aNeighbors[i].nicknameOfNeighbor)));
                continue;
            }
            NeighborHealth nh(pResp->m_aNeighbors[i].nicknameOfNeighbor, pResp->m_aNeighbors[i].neighborFlags,
                pResp->m_aNeighbors[i].meanRSLSinceLastReport, pResp->m_aNeighbors[i].noOfPacketsTransmittedToThisNeighbor,
                pResp->m_aNeighbors[i].noOfFailedTransmits, pResp->m_aNeighbors[i].noOfPacketsReceivedFromThisNeighbor);
            rNeighborHealths.m_oNeighborsList.push_back(nh);
        }

        if (pResp->m_ucTotalNoOfNeighbors > pResp->m_ucNeighborTableIndex + pResp->m_ucNoOfNeighborEntriesRead)
        {
            C780_ReportNeighborHealthList_Req whReq;
            whReq.m_ucNeighborTableIndex = pResp->m_ucNeighborTableIndex + pResp->m_ucNoOfNeighborEntriesRead;
            whReq.m_ucNoOfNeighborEntriesToRead = pResp->m_ucTotalNoOfNeighbors - (pResp->m_ucNeighborTableIndex
                        + pResp->m_ucNoOfNeighborEntriesRead);

            stack::WHartCommand whCmd;
            whCmd.commandID = CMDID_C780_ReportNeighborHealthList;
            whCmd.command = &whReq;

            gateway::GatewayIO::AppData appData;
            appData.appCmd = m_pResponse->appData.appCmd;
            appData.innerDataHandle = m_pResponse->appData.innerDataHandle;

            //LOG_INFO_APP("[ResponseProcessor::SendWHRequest]:CMDID_C780_ReportNeighborHealthList. Device=" << rNeighborHealths.m_oMac);
            m_pProcessor->gateway.SendWHRequest(appData, rNeighborHealths.m_oMac.Address(), whCmd, false, true);
        }
        else
        {
            p_rNeighborHealthCmd.ReportReceived();
        }

        if (p_rNeighborHealthCmd.ReceivedAllReports())
        {
            p_rNeighborHealthCmd.pDevices->ReplaceDevicesNeighborsHealth(p_rNeighborHealthCmd.GetDevicesNeighbHealth());
            p_rNeighborHealthCmd.pCommands->SetCommandResponded(p_rNeighborHealthCmd.dbCommand, nlib::CurrentUniversalTime(), "success",
                NULL);
            if (p_rNeighborHealthCmd.IsToBeAccessPoint()) //update in db if it's just for one device
            {
                DevicePtr dev = p_rNeighborHealthCmd.pDevices->FindDevice(p_rNeighborHealthCmd.GetDevicesNeighbHealth()[0].m_oMac);
                if (dev && dev->Type() != Device::dtAccessPoint)
                {
                    dev->Type(Device::dtAccessPoint);
                    p_rNeighborHealthCmd.pDevices->UpdateDevice(dev, false, false);
                }
            }
        }
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Visit::AppNeighborHealthReportCmd]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Visit::AppNeighborHealthReportCmd]: System error!");
    }
}

// autodetect burst message
void ResponseProcessor::Visit(AppDiscoveryBurstConfigCmd& p_rAppCmd)
{
    //LOG_INFO_APP("[ResponseProcessor::Visit]:AppDiscoveryBurstConfigCmd. Device=");
    try
    {
        DevicePtr device = p_rAppCmd.pDevices->FindDevice(p_rAppCmd.m_oDeviceMAC);
        LOG_INFO_APP("[DiscoveryBurstConfig]:CMDID[" << p_rAppCmd.m_oDeviceMAC << "] = " << m_pResponse->whCmd.commandID);

        if (m_pResponse->whCmd.responseCode == RCS_E64_CommandNotImplemented) // command Not Implemented
        {
            LOG_WARN_APP("[DiscoveryBurstConfig]:CMD" << m_pResponse->whCmd.commandID << " - Command not implemented for deviceMAC="
                        << p_rAppCmd.m_oDeviceMAC);
            SaveRespErr(*p_rAppCmd.pCommands, p_rAppCmd.dbCommand).CommandFailed(DBCommand::rsFailure_InvalidDevice);
            // exclude device from discovery device list
            p_rAppCmd.pDevices->UpdateStoredPublishers(p_rAppCmd.m_oDeviceMAC, p_rAppCmd.m_oPublisherInfo, AUTODETECT_NOT_IMPLEMENT_105);
            device->IssueDiscoveryBurstConfigCmd = AUTODETECT_NONE; // reset device flag
            return;
        }
        if (m_pResponse->hostErr == gateway::GatewayIO::HostTimedOut)
        {
            LOG_WARN_APP("[DiscoveryBurstConfig]:CMD" << m_pResponse->whCmd.commandID << " - Request for deviceMAC="
                        << p_rAppCmd.m_oDeviceMAC << " was delayed due to a timed-out response.");
            p_rAppCmd.pCommands->SetCommandResponded(p_rAppCmd.dbCommand, nlib::CurrentUniversalTime(), "response timed-out", NULL);
            // mark device as unavailable for <DiscoveryRequestDelayInterval> seconds
            device->lastTimeoutResponse.MarkStartTime();
            device->IssueDiscoveryBurstConfigCmd = AUTODETECT_DELAYED; // Timeout status received
            return;
        }

        if (m_pResponse->whCmd.commandID == CMDID_C105_ReadBurstModeConfiguration)
        {
            if (SaveRespErr::IsErr_105(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
            {
                LOG_WARN_APP("[DiscoveryBurstConfig]:CMD105 - Request for deviceMAC=" << p_rAppCmd.m_oDeviceMAC
                            << " has got ResponseCode=" << (int) m_pResponse->whCmd.responseCode << ", HostErrorCode="
                            << (int) m_pResponse->hostErr);
                SaveRespErr(*p_rAppCmd.pCommands, p_rAppCmd.dbCommand).SaveErr_105(m_pResponse->hostErr, m_pResponse->whCmd.responseCode);
                // auto-detect burst error. Invalid response for cmd105
                device->IssueDiscoveryBurstConfigCmd = AUTODETECT_NONE;
                return;
            }

            bool burstActive = false;
            bool res105 = AppDiscoveryBurstConfigCmd_ProcessResponse105(p_rAppCmd, burstActive);
            p_rAppCmd.m_oPublisherInfo.autodetectState = (p_rAppCmd.m_nTotalBurstCount > (p_rAppCmd.m_nCurrentBurst + 1)) ? AUTODETECT_IN_PROGRESS : AUTODETECT_DONE;

            if (!res105)
                return;

            LOG_INFO_APP("[DiscoveryBurstConfig]:CMD105 - RESPONSE[" << p_rAppCmd.m_oDeviceMAC << "]=(IsAdapter="
                        << (device->IsAdapter() ? "true" : "false") << ", burstActive=" << (burstActive ? "true" : "false")
                        << ", PublisherList.size = " << p_rAppCmd.m_oPublisherInfo.burstMessageList.size() << ")");

            if (device->IsAdapter() && burstActive)
            {
                device->IssueDiscoveryBurstConfigCmd = AUTODETECT_IN_PROGRESS;
                if (device->GetMasterMode() < 0)
                    SendRequest074(m_pResponse->appData.innerDataHandle, p_rAppCmd.m_oDeviceMAC, m_pResponse->appData.appCmd, m_pProcessor);
                else
                    SendRequest101(p_rAppCmd.m_nCurrentBurst, m_pResponse->appData.innerDataHandle, p_rAppCmd.m_oDeviceMAC, m_pResponse->appData.appCmd, m_pProcessor);
                return;
            }
            else
            {
                if (!AppDiscoveryBurstConfigCmd_ProcessNextBurst(p_rAppCmd, device))
                    return;
            }
        }
        else if (m_pResponse->whCmd.commandID == CMDID_C009_ReadDeviceVariablesWithStatus)
        {
            if (SaveRespErr::IsErr_009(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
            {
                LOG_WARN_APP("[DiscoveryBurstConfig]:CMD009 - Request for deviceMAC=" << p_rAppCmd.m_oDeviceMAC
                            << " has got ResponseCode=" << (int) m_pResponse->whCmd.responseCode << ", HostErrorCode="
                            << (int) m_pResponse->hostErr << ". Cannot get device variables classification and units.");
                SaveRespErr(*p_rAppCmd.pCommands, p_rAppCmd.dbCommand).SaveErr_009(m_pResponse->hostErr, m_pResponse->whCmd.responseCode);
                // auto-detect burst error. Invalid response for cmd9
                device->IssueDiscoveryBurstConfigCmd = AUTODETECT_NONE;
                return;
            }

            LOG_INFO_APP("[DiscoveryBurstConfig]:CMD009 - RESPONSE[" << p_rAppCmd.m_oDeviceMAC << "]=(burst=" << (int) p_rAppCmd.m_nCurrentBurst
                    << ", PublisherList.size = " << p_rAppCmd.m_oPublisherInfo.burstMessageList.size() << ")");

            if (!AppDiscoveryBurstConfigCmd_ProcessResponse009(p_rAppCmd))
                return;

            if (device->IsAdapter())
            {
                device->IssueDiscoveryBurstConfigCmd = AUTODETECT_IN_PROGRESS;
                if (device->GetMasterMode() < 0)
                    SendRequest074(m_pResponse->appData.innerDataHandle, p_rAppCmd.m_oDeviceMAC, m_pResponse->appData.appCmd, m_pProcessor);
                else
                    SendRequest101(p_rAppCmd.m_nCurrentBurst, m_pResponse->appData.innerDataHandle, p_rAppCmd.m_oDeviceMAC, m_pResponse->appData.appCmd, m_pProcessor);
                return;
            }
            else
            {
                if (!AppDiscoveryBurstConfigCmd_ProcessNextBurst(p_rAppCmd, device))
                    return;
            }
        }
        else if (m_pResponse->whCmd.commandID == CMDID_C074_ReadIOSystemCapabilities)
        {
            if (SaveRespErr::IsErr_074(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
            {
                LOG_WARN_APP("[DiscoveryBurstConfig]:CMD074 - Request for deviceMAC=" << p_rAppCmd.m_oDeviceMAC
                            << " has got ResponseCode=" << (int) m_pResponse->whCmd.responseCode << ", HostErrorCode="
                            << (int) m_pResponse->hostErr);
                SaveRespErr(*p_rAppCmd.pCommands, p_rAppCmd.dbCommand).SaveErr_074(m_pResponse->hostErr, m_pResponse->whCmd.responseCode);
                // auto-detect burst error. Invalid response for cmd074
                device->IssueDiscoveryBurstConfigCmd = AUTODETECT_NONE;
                return;
            }

            C074_ReadIOSystemCapabilities_Resp *pwhResp074 = (C074_ReadIOSystemCapabilities_Resp*) (m_pResponse->whCmd.command);
            if (!pwhResp074)
            {
                LOG_ERROR_APP("[DiscoveryBurstConfig]: Incorrect response received for CMD074. DeviceMAC=" << p_rAppCmd.m_oDeviceMAC);
                return;
            }

            device->SetMasterMode(pwhResp074->masterMode);
            LOG_INFO_APP("[DiscoveryBurstConfig]:CMD074 - RESPONSE[" << p_rAppCmd.m_oDeviceMAC << "]=(masterMode=" << device->GetMasterMode() << "), AUTO_DETECT=" <<
                         ((p_rAppCmd.m_oPublisherInfo.autodetectState == AUTODETECT_DONE) ? "DONE" :
                         ((p_rAppCmd.m_oPublisherInfo.autodetectState == AUTODETECT_IN_PROGRESS) ? "IN_PROGRESS" : "NONE")));
            SendRequest101(p_rAppCmd.m_nCurrentBurst, m_pResponse->appData.innerDataHandle, p_rAppCmd.m_oDeviceMAC, m_pResponse->appData.appCmd, m_pProcessor);
            return;
        }
        else if (m_pResponse->whCmd.commandID == CMDID_C101_ReadSubDeviceToBurstMessageMap)
        {
            if (SaveRespErr::IsErr_101(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
            {
                LOG_WARN_APP("[DiscoveryBurstConfig]:CMD101 - Request for deviceMAC=" << p_rAppCmd.m_oDeviceMAC
                            << " has got ResponseCode=" << (int) m_pResponse->whCmd.responseCode << ", HostErrorCode="
                            << (int) m_pResponse->hostErr);
                SaveRespErr(*p_rAppCmd.pCommands, p_rAppCmd.dbCommand).SaveErr_101(m_pResponse->hostErr, m_pResponse->whCmd.responseCode);
                // auto-detect burst error. Invalid response for cmd101.
                device->IssueDiscoveryBurstConfigCmd = AUTODETECT_NONE;
                return;
            }

            C101_ReadSubDeviceToBurstMessageMap_Resp *pwhResp101 = (C101_ReadSubDeviceToBurstMessageMap_Resp*) (m_pResponse->whCmd.command);
            if (!pwhResp101)
            {
                LOG_ERROR_APP("[DiscoveryBurstConfig]: Incorrect response received for CMD101. DeviceMAC=" << p_rAppCmd.m_oDeviceMAC);
                return;
            }

            LOG_INFO_APP("[DiscoveryBurstConfig]:CMD101 - RESPONSE[" << p_rAppCmd.m_oDeviceMAC << "]=(burst=" << (int)p_rAppCmd.m_nCurrentBurst << ", subDeviceIndex=" << (int) pwhResp101->subDeviceIndex << ")");

            if (pwhResp101->subDeviceIndex > 0 && pwhResp101->subDeviceIndex < 65535)
            {
                device->IssueDiscoveryBurstConfigCmd = AUTODETECT_IN_PROGRESS;
                SendRequest084(pwhResp101->subDeviceIndex, m_pResponse->appData.innerDataHandle, p_rAppCmd.m_oDeviceMAC, m_pResponse->appData.appCmd, m_pProcessor);
                return;
            }
            else
            {
                BurstMessage burst;
                burst.cmdNo = p_rAppCmd.m_nCurrentCmdNo;
                burst.burstMessage = p_rAppCmd.m_nCurrentBurst;
                burst.updatePeriod = p_rAppCmd.m_nCurrentUpdatePeriod;
                burst.maxUpdatePeriod = p_rAppCmd.m_nCurrentMaxUpdatePeriod;
                hostapp::MAC emptyMAC(0);
                burst.subDeviceMAC = emptyMAC;

                p_rAppCmd.m_oPublisherInfo.burstMessageList.erase(burst);
                p_rAppCmd.m_oPublisherInfo.burstMessageList.insert(burst);

                LOG_INFO_APP("[DiscoveryBurstConfig]:CMD101 - RESPONSE[" << p_rAppCmd.m_oDeviceMAC << "]=(subDeviceIndex=" << (int) pwhResp101->subDeviceIndex << "), AUTO_DETECT="
                        << ((p_rAppCmd.m_oPublisherInfo.autodetectState == AUTODETECT_DONE) ? "DONE" :
                           ((p_rAppCmd.m_oPublisherInfo.autodetectState = AUTODETECT_IN_PROGRESS) ? "IN_PROGRESS" : "NONE")));

                if (!AppDiscoveryBurstConfigCmd_ProcessNextBurst(p_rAppCmd, device))
                    return;
            }
        }
        else if (m_pResponse->whCmd.commandID == CMDID_C084_ReadSubDeviceIdentitySummary)
        {
            if (SaveRespErr::IsErr_084(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
            {
                LOG_WARN_APP("[DiscoveryBurstConfig]:CMD084 - Request for deviceMAC=" << p_rAppCmd.m_oDeviceMAC
                            << " has got ResponseCode=" << (int) m_pResponse->whCmd.responseCode << ", HostErrorCode="
                            << (int) m_pResponse->hostErr);
                SaveRespErr(*p_rAppCmd.pCommands, p_rAppCmd.dbCommand).SaveErr_084(m_pResponse->hostErr, m_pResponse->whCmd.responseCode);
                // auto-detect burst error. Invalid response for cmd084
                device->IssueDiscoveryBurstConfigCmd = AUTODETECT_NONE;
                return;
            }

            if (!AppDiscoveryBurstConfigCmd_ProcessResponse084(p_rAppCmd, device))
                return;

            if (!AppDiscoveryBurstConfigCmd_ProcessNextBurst(p_rAppCmd, device))
                return;
        }
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Visit::AppDiscoveryBurstConfigCmd]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Visit::AppDiscoveryBurstConfigCmd]: System error!");
    }
}

bool ResponseProcessor::AppDiscoveryBurstConfigCmd_ProcessResponse105(AppDiscoveryBurstConfigCmd& p_rAppCmd, bool burstActive)
{
	try
	{
		C105_ReadBurstModeConfiguration_Resp *pwhResp105 = (C105_ReadBurstModeConfiguration_Resp*) (m_pResponse->whCmd.command);
        if (!pwhResp105)
        {
            LOG_ERROR_APP("[DiscoveryBurstConfig]: Incorrect response received for CMD105. DeviceMAC=" << p_rAppCmd.m_oDeviceMAC);
            return false;
        }

		BurstModeControlCodes burstMode = pwhResp105->burstMode;
		p_rAppCmd.m_nCurrentCmdNo = (pwhResp105->commandNo != 31) ? pwhResp105->commandNo : pwhResp105->extendedCommandNumber;
		p_rAppCmd.m_nCurrentBurst = pwhResp105->burstMessage;
		p_rAppCmd.m_nTotalBurstCount = pwhResp105->totalBurstMessagesCount;

		burstActive = (burstMode == BurstModeControlCodes_EnableBurstOnTDMADataLinkLayer ||
				   	   burstMode == BurstModeControlCodes_EnableBurstOnTDMAandTokenPassingDataLinkLayers);
		if (burstActive)
		{
			// set burst
			BurstMessage burstMessage;
			burstMessage.cmdNo = p_rAppCmd.m_nCurrentCmdNo;
			burstMessage.burstMessage = p_rAppCmd.m_nCurrentBurst;
			p_rAppCmd.m_nCurrentUpdatePeriod = burstMessage.updatePeriod = pwhResp105->updateTime.u32 / 32000;
			p_rAppCmd.m_nCurrentMaxUpdatePeriod = burstMessage.maxUpdatePeriod = pwhResp105->maxUpdateTime.u32 / 32000;
			hostapp::MAC emptyMAC(0);
			burstMessage.subDeviceMAC = emptyMAC;
			p_rAppCmd.m_oPublisherInfo.burstMessageList.insert(burstMessage);
			// set trigger
			Trigger trigger;
			trigger.cmdNo = p_rAppCmd.m_nCurrentCmdNo;
			trigger.burstMessage = p_rAppCmd.m_nCurrentBurst;
			trigger.modeSelection = pwhResp105->burstTriggerMode;
			trigger.classification = pwhResp105->classificationCode;
			trigger.unitCode = pwhResp105->unitsCode;
			trigger.triggerLevel = pwhResp105->triggerValue;
			p_rAppCmd.m_oPublisherInfo.triggersList.insert(trigger);
			p_rAppCmd.m_oPublisherInfo.burstMessagesState[p_rAppCmd.m_nCurrentBurst] = BURST_STATE_SET;
			p_rAppCmd.m_oPublisherInfo.burstNoTotalCmd105 = p_rAppCmd.m_nTotalBurstCount;

			bool hasVars = false;
			for (int i = 0; i <  MaxNoOfDeviceVariables; ++i)
			{
				if (pwhResp105->deviceVariableCode[i] != DeviceVariableCodes_None)
				{
					PublishChannel publishChannel;
					publishChannel.cmdNo = p_rAppCmd.m_nCurrentCmdNo;
					publishChannel.burstMessage = p_rAppCmd.m_nCurrentBurst;
					publishChannel.deviceVariable = pwhResp105->deviceVariableCode[i];
					publishChannel.deviceVariableSlot = (unsigned char)i;
					publishChannel.name = GetDeviceVariableName((int)pwhResp105->deviceVariableCode[i]);
					p_rAppCmd.m_oPublisherInfo.channelList.insert(publishChannel);
					hasVars = true;
				}
			}
			if (hasVars)
			{
				SendRequest009(pwhResp105->deviceVariableCode, MaxNoOfDeviceVariables,
							   m_pResponse->appData.innerDataHandle, p_rAppCmd.m_oDeviceMAC,
							   m_pResponse->appData.appCmd, m_pProcessor);
				return false;
			}
		}
	}
    catch(std::exception ex)
    {
    	LOG_ERROR_APP("[AppDiscoveryBurstConfigCmd_ProcessResponse105]:CMD105 - Error on processing cmd105 response for deviceMAC=" << p_rAppCmd.m_oDeviceMAC << ". EXCEPTION: " << ex.what());
        return false;
    }
    catch (...)
    {
        LOG_ERROR_APP("[AppDiscoveryBurstConfigCmd_ProcessResponse105]: System error!");
        return false;
    }
	return true;
}

bool ResponseProcessor::AppDiscoveryBurstConfigCmd_ProcessResponse009(AppDiscoveryBurstConfigCmd& p_rAppCmd)
{
	try
	{
		C009_ReadDeviceVariablesWithStatus_Resp *pwhResp009 = (C009_ReadDeviceVariablesWithStatus_Resp*)(m_pResponse->whCmd.command);
        if (!pwhResp009)
        {
            LOG_ERROR_APP("[DiscoveryBurstConfig]: Incorrect response received for CMD009. DeviceMAC=" << p_rAppCmd.m_oDeviceMAC);
            return false;
        }

//		std::ostringstream oss;
//		oss << "[";
//		for (int i = 0; i < MaxNoOfDeviceVariables; ++i) {
//			oss << "{slot:" << i
//				<< ", status:" << (int)pwhResp009->slots[i].deviceVariableStatus
//				<< ", varCode:" << (int)pwhResp009->slots[i].deviceVariableCode
//				<< ", value:" << pwhResp009->slots[i].deviceVariableValue
//				<< ", class:" << (int)pwhResp009->slots[i].deviceVariableClassificationCode
//				<< ", unit:" << (int)pwhResp009->slots[i].unitCode
//				<< ((i<7)? "}, ":"}");
//		}
//
//		LOG_INFO_APP("[DiscoveryBurstConfig]:CMD009 - RESPONSE[" << p_rAppCmd.m_oDeviceMAC << "] = ("
//				<< "extendedFieldDeviceStatus=" << (int)pwhResp009->extendedFieldDeviceStatus
//				<< ", slot0TimeStamp=" << (int)pwhResp009->slot0TimeStamp.u32
//				<< ", variablesSize=" << (int)pwhResp009->variablesSize
//				<< ", slots=" << oss.str()
//				<< ")");

		PublishChannelSetT channelList;
		for (PublishChannelSetT::iterator it = p_rAppCmd.m_oPublisherInfo.channelList.begin(); it != p_rAppCmd.m_oPublisherInfo.channelList.end() ; ++it)
		{
			if (it->burstMessage == p_rAppCmd.m_nCurrentBurst && it->cmdNo == p_rAppCmd.m_nCurrentCmdNo)
			{
				const Slot* slot = FindMatchingVariableSlot(*pwhResp009, *it);
				PublishChannel channel;
				if (slot)
				{
				    if (slot->deviceVariableStatus == 0)
				    {
		                channel.burstMessage = it->burstMessage;
		                channel.cmdNo = it->cmdNo;
		                channel.deviceVariable = it->deviceVariable;
		                channel.name = it->name;
				        channel.deviceVariableSlot = it->deviceVariableSlot;
				        channel.classification = slot->deviceVariableClassificationCode;
				        channel.unitCode = slot->unitCode;
				        channelList.insert(channel);
				    }
				    else
				    {
                        LOG_WARN_APP("[DiscoveryBurstConfig]:CMD009 - Invalid status (0x" << std::hex << (int)slot->deviceVariableStatus
                                << ") for channel [burst=" << (int)it->burstMessage << ", cmdNo=" << (int)it->cmdNo << ", deviceVariable="
                                << (int)it->deviceVariable <<"]. DeviceMAC=" << p_rAppCmd.m_oDeviceMAC << ")");
				    }
				}
				else
				{
                    LOG_WARN_APP("[DiscoveryBurstConfig]:CMD009 - Cannot identify slot for channel [burst="
                            << (int)it->burstMessage << ", cmdNo=" << (int)it->cmdNo << ", deviceVariable="
                            << (int)it->deviceVariable <<"]. DeviceMAC=" << p_rAppCmd.m_oDeviceMAC << ")");
				}
			}
		}

		for (PublishChannelSetT::iterator it = channelList.begin(); it != channelList.end() ; ++it)
		{
			p_rAppCmd.m_oPublisherInfo.channelList.erase(*it);
			if (it->deviceVariableSlot >= 0)
			{
				p_rAppCmd.m_oPublisherInfo.channelList.insert(*it);
				LOG_INFO_APP("[DiscoveryBurstConfig]:CMD009 - Set VARIABLE[burst="
								<< (int)it->burstMessage << ", cmd=" << (int)it->cmdNo << ", varCode=" << (int)it->deviceVariable << "]=(classification="
								<< (int)it->classification << ", unit=" << (int)it->unitCode << ") for deviceMAC=" << p_rAppCmd.m_oDeviceMAC << ".");
			}
			else
			{
				LOG_INFO_APP("[DiscoveryBurstConfig]:CMD009 - Ignore VARIABLE[burst="
								<< (int)it->burstMessage << ", cmd=" << (int)it->cmdNo << ", varCode=" << (int)it->deviceVariable << "]=(classification="
								<< (int)it->classification << ", unit=" << (int)it->unitCode << ") for deviceMAC=" << p_rAppCmd.m_oDeviceMAC << ".");
			}
		}
	}
	catch (std::exception ex)
	{
    	LOG_ERROR_APP("[AppDiscoveryBurstConfigCmd_ProcessResponse009]:CMD009 - Error on processing cmd009 response for deviceMAC=" << p_rAppCmd.m_oDeviceMAC << ". EXCEPTION: " << ex.what());
    	return false;
	}
    catch (...)
    {
        LOG_ERROR_APP("[AppDiscoveryBurstConfigCmd_ProcessResponse009]: System error!");
        return false;
    }
	return true;
}

bool ResponseProcessor::AppDiscoveryBurstConfigCmd_ProcessResponse084(AppDiscoveryBurstConfigCmd& p_rAppCmd, DevicePtr device)
{
	try
	{
		C084_ReadSubDeviceIdentitySummary_Resp *pwhResp084 = (C084_ReadSubDeviceIdentitySummary_Resp*) (m_pResponse->whCmd.command);
        if (!pwhResp084)
        {
            LOG_ERROR_APP("[DiscoveryBurstConfig]: Incorrect response received for CMD084. DeviceMAC=" << p_rAppCmd.m_oDeviceMAC);
            return false;
        }

        BurstMessage burstMsg;
		burstMsg.cmdNo = p_rAppCmd.m_nCurrentCmdNo;
		burstMsg.burstMessage = p_rAppCmd.m_nCurrentBurst;
		burstMsg.updatePeriod = p_rAppCmd.m_nCurrentUpdatePeriod;
		burstMsg.maxUpdatePeriod = p_rAppCmd.m_nCurrentMaxUpdatePeriod;
		hostapp::MAC subDevMAC(MakeCorrectedUniqueID(pwhResp084->expandedDeviceType, pwhResp084->deviceID, device->GetMasterMode()).bytes);
		burstMsg.subDeviceMAC = subDevMAC;

		p_rAppCmd.m_oPublisherInfo.burstMessageList.erase(burstMsg);
		p_rAppCmd.m_oPublisherInfo.burstMessageList.insert(burstMsg);

		device->subDevicesMap[burstMsg.subDeviceMAC] = pwhResp084->subDeviceIndex;

//		for (BurstMessageSetT::iterator itBurst = p_rAppCmd.m_oPublisherInfo.burstMessageList.begin(); itBurst != p_rAppCmd.m_oPublisherInfo.burstMessageList.end() ; ++itBurst) {
//			LOG_INFO_APP("[DiscoveryBurstConfig]:CMD084 - BURSTS[" << itBurst->cmdNo << ", " << (int)itBurst->burstMessage << "]=[" << itBurst->cmdNo << ", " << (int)itBurst->burstMessage << ", " << itBurst->updatePeriod << ", " << itBurst->maxUpdatePeriod << ", " << itBurst->subDeviceMAC.ToString() << "]");
//		}

		LOG_INFO_APP("[DiscoveryBurstConfig]:CMD084 - RESPONSE[" << p_rAppCmd.m_oDeviceMAC << "]=(burst=" << (int)burstMsg.burstMessage << ", subDevice=" << burstMsg.subDeviceMAC.ToString() << "), AUTO_DETECT=" <<
					 ((p_rAppCmd.m_oPublisherInfo.autodetectState == AUTODETECT_DONE) ? "DONE" :
					 ((p_rAppCmd.m_oPublisherInfo.autodetectState == AUTODETECT_IN_PROGRESS) ? "IN_PROGRESS" : "NONE")));
	}
	catch (std::exception ex)
	{
		LOG_ERROR_APP("[AppDiscoveryBurstConfigCmd_ProcessResponse084]:CMD084 - Error on processing cmd084 response for deviceMAC=" << p_rAppCmd.m_oDeviceMAC << ". EXCEPTION: " << ex.what());
		return false;
	}
    catch (...)
    {
        LOG_ERROR_APP("[AppDiscoveryBurstConfigCmd_ProcessResponse084]: System error!");
        return false;
    }
    return true;
}

bool ResponseProcessor::AppDiscoveryBurstConfigCmd_ProcessNextBurst(AppDiscoveryBurstConfigCmd& p_rAppCmd, DevicePtr device)
{
    switch (p_rAppCmd.m_oPublisherInfo.autodetectState)
    {
        case AUTODETECT_NONE:
            return true;
        case AUTODETECT_IN_PROGRESS: // send a new request to get next burst
            p_rAppCmd.m_nCurrentBurst++;
            SendRequest105(p_rAppCmd.m_nCurrentBurst, m_pResponse->appData.innerDataHandle, p_rAppCmd.m_oDeviceMAC, m_pResponse->appData.appCmd, m_pProcessor);
            return false;
        case AUTODETECT_DONE: // save discovered bursts in Publishers' cache and file
            // auto-detect burst done
            p_rAppCmd.pDevices->UpdateStoredPublishers(p_rAppCmd.m_oDeviceMAC, p_rAppCmd.m_oPublisherInfo, AUTODETECT_DONE);
            p_rAppCmd.pCommands->SetCommandResponded(p_rAppCmd.dbCommand, nlib::CurrentUniversalTime(), "success", NULL);
            device->IssueDiscoveryBurstConfigCmd = AUTODETECT_DONE;
            LOG_INFO_APP("[DiscoveryBurstConfig]: TotalBurstMessagesCount[" << p_rAppCmd.m_oDeviceMAC << "]=" << (int)p_rAppCmd.m_nTotalBurstCount);
        case AUTODETECT_DELAYED: ;
        case AUTODETECT_NOT_IMPLEMENT_105: ;
        default: ;
    }
    return true;
}

void ResponseProcessor::Visit(AppFirmwareTransfersCmd& p_rAppCmd)
{
    //LOG_INFO_APP("[ResponseProcessor::Visit]:AppFirmwareTransfersCmd. Device=" << p_rAppCmd.m_deviceMAC);

    try
    {
        if (SaveRespErr::IsErr_112(m_pResponse->hostErr, m_pResponse->whCmd.responseCode))
        {
            LOG_WARN_APP("[DiscoveryBurstConfig]:CMD112 - Request for deviceMAC=" << p_rAppCmd.m_deviceMAC << " has got ResponseCode="
                        << (int) m_pResponse->whCmd.responseCode << ", HostErrorCode=" << (int) m_pResponse->hostErr);
            SaveRespErr(*p_rAppCmd.pCommands, p_rAppCmd.dbCommand).SaveErr_112(m_pResponse->hostErr, m_pResponse->whCmd.responseCode);
            return;
        }

        AppFirmwareTransfersCmd::TransferStates nextState;
        unsigned int nextCurrentBlockSize = 0;
        unsigned int nextCurrentBlockCount = 0;
        // in BulkEnd we do not have a valid File
        unsigned int datasize = (p_rAppCmd.m_file != 0) ? p_rAppCmd.m_file->getMapSize() : 0;
        unsigned int amountToSend = (p_rAppCmd.m_file != 0) ? (p_rAppCmd.m_file->getMapSize() - p_rAppCmd.m_currentOffset) : 0;

        switch (p_rAppCmd.m_currentTransferState)
        {
            case AppFirmwareTransfersCmd::TransferOpen:
            {
                p_rAppCmd.m_maxBlockCount = (datasize % p_rAppCmd.m_maxBlockSize) == 0 ? datasize / p_rAppCmd.m_maxBlockSize - 1 : datasize
                            / p_rAppCmd.m_maxBlockSize;
                nextCurrentBlockSize = p_rAppCmd.m_maxBlockSize < amountToSend ? p_rAppCmd.m_maxBlockSize : amountToSend;
                nextCurrentBlockCount = 0;
                nextState = AppFirmwareTransfersCmd::TransferInProgress;

                //FirmwareDownloadStatus
                CMicroSec now;
                p_rAppCmd.m_transferTime = now;
                break;
            }
            case AppFirmwareTransfersCmd::TransferInProgress:
            {
                //FirmwareDownloadStatus
                int nTime = p_rAppCmd.m_transferTime.GetElapsedMSec();
                if (!nTime)
                    nTime = 1; /// SIGFPE protection

                nextCurrentBlockCount = p_rAppCmd.m_currentBlockCount + 1;
                if (nextCurrentBlockCount > p_rAppCmd.m_maxBlockCount)
                {
                    nextState = AppFirmwareTransfersCmd::TransferEnd;
                    break;
                }
                else
                {
                    nextState = AppFirmwareTransfersCmd::TransferInProgress;
                    nextCurrentBlockSize = p_rAppCmd.m_maxBlockSize < amountToSend ? p_rAppCmd.m_maxBlockSize : amountToSend;
                }
                break;
            }
            case AppFirmwareTransfersCmd::TransferEnd:
            {
                int cmdID = 0;
                if (cmdID != p_rAppCmd.dbCommand.commandID)
                {
                    LOG_ERROR_APP(" bulk -> command ID inconsistent " << cmdID << " != " << p_rAppCmd.dbCommand.commandID);
                    return;
                }

                if (p_rAppCmd.m_maxBlockCount < 0)// ERROR
                {
                    LOG_ERROR_APP(" bulk -> error on BulkEnd: " << p_rAppCmd.m_maxBlockCount);
                    //FirmwareDownloadStatus
                    SaveRespErr(*p_rAppCmd.pCommands, p_rAppCmd.dbCommand).CommandFailed(DBCommand::rsFailure_InternalError);
                }
                else
                {
                    p_rAppCmd.pCommands->SetCommandResponded(p_rAppCmd.dbCommand, nlib::CurrentUniversalTime(), "success", NULL);
                }

                break;
            }
            default:
                ;
        }
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[Visit::AppFirmwareTransfersCmd]: Application error: " << e.what());
    }
    catch (...)
    {
        LOG_ERROR_APP("[Visit::AppFirmwareTransfersCmd]: System error!");
    }
}

} //namespace hostapp
} //namespace hart7

