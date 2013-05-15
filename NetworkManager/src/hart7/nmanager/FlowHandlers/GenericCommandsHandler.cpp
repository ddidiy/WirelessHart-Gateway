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

/*
 * GenericCommandsHandler.cpp
 *
 *  Created on: Sep 10, 2009
 *      Author: andrei.petrut
 */

#include "GenericCommandsHandler.h"
#include "../AllNetworkManagerCommands.h"
#include "TerminateServiceFlowHandler.h"
#include "hart7/util/NMLog.h"
#include <WHartStack/WHartStack.h>
#include <boost/bind.hpp>

namespace hart7 {
namespace nmanager {

void GenericCommandsHandler::HandleRequests(WHartHandle handle, const WHartAddress& source,
                                            const WHartCommandList& commandsList)
{
    if (source == Gateway_Nickname() || source == Gateway_UniqueID())
    {
        HandleGWRequests(handle, source, commandsList);
    }
    else
    {
        HandleDeviceRequests(handle, source, commandsList);
    }
}

void GenericCommandsHandler::HandleResponses(WHartHandle handle, const WHartAddress& source,
                                             const WHartCommandList& commandsList)
{
    for (int i = 0; i < commandsList.count; i++)
    {
        WHartCommand *command = commandsList.list + i;
        LOG_WARN("Unsolicited response received with commandID=" << command->commandID);
    }
}

void GenericCommandsHandler::HandlePublishes(WHartHandle handle, const WHartAddress& source,
                                             const WHartCommandList& commandsList)
{
    for (int i = 0; i < commandsList.count; i++)
    {
        WHartCommand *command = commandsList.list + i;

        LOG_DEBUG("HandlePublishes() command->commandID : " << (int) command->commandID);
        hart7::util::NMLog::logCommandResponse((int) command->commandID, 0, command->command, source);

        switch (command->commandID)
        {
            case CMDID_C779_ReportDeviceHealth:
            {
                reportsHandler.Handle(source, *((C779_ReportDeviceHealth_Resp*) command->command));
                break;
            }
            case CMDID_C780_ReportNeighborHealthList:
            {
                reportsHandler.Handle(source, *((C780_ReportNeighborHealthList_Resp*) command->command));
                break;
            }
            case CMDID_C787_ReportNeighborSignalLevels:
            {
                reportsHandler.Handle(source, *((C787_ReportNeighborSignalLevels_Resp*) command->command));
                break;
            }
            case CMDID_C788_AlarmPathDown:
            {
                LOG_INFO("C788_AlarmPathDown received from " << source << " with neighbor="
                            << WHartAddress(((C788_AlarmPathDown_Resp*) command->command)->Nickname));

                if (source.type == source.whartaNickname)
                {
                    commonData.alarmDispatcher.dispatchAlarm788(source.address.nickname,
                                                                *((C788_AlarmPathDown_Resp*) command->command));
                }

                break;
            }
            case CMDID_C789_AlarmSourceRouteFailed:
            {
                LOG_INFO("789_AlarmSourceRouteFailed received from " << source << " with neighbor="
                            << WHartAddress(((C789_AlarmSourceRouteFailed_Resp*) command->command)->m_unNicknameOfUnreachableNeighbor));

                if (source.type == source.whartaNickname)
                {
                    commonData.alarmDispatcher.dispatchAlarm789(source.address.nickname,
                                                                *((C789_AlarmSourceRouteFailed_Resp*) command->command));
                }

                break;
            }
            case CMDID_C790_AlarmGraphRouteFailed:
            {
                LOG_INFO("CMDID_C790_AlarmGraphRouteFailed received from " << source << " with m_unGraphIdOfFailedRoute="
                            << std::hex << ((C790_AlarmGraphRouteFailed_Resp*) command->command)->m_unGraphIdOfFailedRoute);

                if (source.type == source.whartaNickname)
                {
                    commonData.alarmDispatcher.dispatchAlarm790(source.address.nickname,
                                                                *((C790_AlarmGraphRouteFailed_Resp*) command->command));
                }

                break;
            }
            case CMDID_C791_AlarmTransportLayerFailed:
            {
                LOG_INFO(
                            "C791_AlarmTransportLayerFailed received from " << source << " with neighbor="
                            << WHartAddress(
                                        ((C791_AlarmTransportLayerFailed_Resp*) command->command)->m_unNicknameOfUnreachablePeer));
                if (source.type == source.whartaNickname)
                {
                    commonData.alarmDispatcher.dispatchAlarm791(
                                                                source.address.nickname,
                                                                *((C791_AlarmTransportLayerFailed_Resp*) command->command));
                }
                break;
            }
            default:
            {
                LOG_INFO("Untreated publish received from " << source << ".");
                break;
            }
        }
    }
}

void GenericCommandsHandler::HandleGWRequests(WHartHandle handle, const WHartAddress& source,
                                              const WHartCommandList& commandsList)
{
    std::deque<WHartCommandWrapper> responses;

    LOG_DEBUG("HandleGWRequests() commandsList.count : " << (int) commandsList.count);
    WHartCommand cmds[commandsList.count];

    for (int i = 0; i < commandsList.count; i++)
    {
        WHartCommand *command = commandsList.list + i;

        LOG_DEBUG("HandleGWRequests() command->commandID : " << (int) command->commandID);
        if (command->command)
        {
            hart7::util::NMLog::logCommand((int) command->commandID, command->command, source);
        }

        responses.push_back(WHartCommandWrapper());
        WHartCommandWrapper& cmd = responses.back();
        if (HandleDefaults(Gateway_Nickname(), *command, cmd))
        {
            LOG_DEBUG("Default command handler responded to command ID=" << command->commandID);
            cmds[i].command = cmd.command.command;
            cmds[i].responseCode = cmd.command.responseCode;
            cmds[i].commandID = cmd.command.commandID;
        }
        else
        {
            cmds[i].commandID = command->commandID;
            cmds[i].command = NULL;

            switch (command->commandID)
            {
                case CMDID_C000_ReadUniqueIdentifier:
                {
                    C000_ReadUniqueIdentifier_Resp *resp;

                    resp = (C000_ReadUniqueIdentifier_Resp *) cmd.commandBuffer.get();
                    cmds[i].responseCode
                                = gatewayRequestsHandler.getGatewayWrappedRequestsHandler().Handle(
                                                                                                   handle,
                                                                                                   NetworkManager_UniqueID().bytes,
                                                                                                   *((C000_ReadUniqueIdentifier_Req*) command->command),
                                                                                                   *resp);
                    cmds[i].command = resp;
                    break;
                }
                case CMDID_C013_ReadTagDescriptorDate:
                {
                    C013_ReadTagDescriptorDate_Resp *resp;
                    resp = (C013_ReadTagDescriptorDate_Resp *) cmd.commandBuffer.get();
                    cmds[i].responseCode
                                = gatewayRequestsHandler.getGatewayWrappedRequestsHandler().Handle(
                                                                                                   handle,
                                                                                                   NetworkManager_UniqueID().bytes,
                                                                                                   *((C013_ReadTagDescriptorDate_Req*) command->command),
                                                                                                   *resp);
                    cmds[i].command = resp;
                    break;
                }
                case CMDID_C020_ReadLongTag:
                {
                    C020_ReadLongTag_Resp *resp;

                    resp = (C020_ReadLongTag_Resp *) cmd.commandBuffer.get();
                    cmds[i].responseCode
                                = gatewayRequestsHandler.getGatewayWrappedRequestsHandler().Handle(
                                                                                                   handle,
                                                                                                   NetworkManager_UniqueID().bytes,
                                                                                                   *((C020_ReadLongTag_Req*) command->command),
                                                                                                   *resp);
                    cmds[i].command = resp;
                    break;
                }
                case CMDID_C769_ReadJoinStatus:
                {
                    C769_ReadJoinStatus_Resp *resp;

                    resp = (C769_ReadJoinStatus_Resp *) cmd.commandBuffer.get();
                    cmds[i].responseCode
                                = gatewayRequestsHandler.getGatewayWrappedRequestsHandler().Handle(
                                                                                                   handle,
                                                                                                   NetworkManager_UniqueID().bytes,
                                                                                                   *((C769_ReadJoinStatus_Req*) command->command),
                                                                                                   *resp);
                    cmds[i].command = resp;
                    break;
                }
                case CMDID_C773_WriteNetworkId:
                {
                    C773_WriteNetworkId_Resp *resp;

                    resp = (C773_WriteNetworkId_Resp *) cmd.commandBuffer.get();
                    cmds[i].responseCode
                                = gatewayRequestsHandler.getGatewayWrappedRequestsHandler().Handle(
                                                                                                   handle,
                                                                                                   NetworkManager_UniqueID().bytes,
                                                                                                   *((C773_WriteNetworkId_Req*) command->command),
                                                                                                   *resp);
                    cmds[i].command = resp;
                    break;
                }
                case CMDID_C774_ReadNetworkId:
                {
                    C774_ReadNetworkId_Resp *resp;

                    resp = (C774_ReadNetworkId_Resp *) cmd.commandBuffer.get();
                    cmds[i].responseCode
                                = gatewayRequestsHandler.getGatewayWrappedRequestsHandler().Handle(
                                                                                                   handle,
                                                                                                   NetworkManager_UniqueID().bytes,
                                                                                                   *((C774_ReadNetworkId_Req*) command->command),
                                                                                                   *resp);
                    cmds[i].command = resp;
                    break;
                }
                case CMDID_C775_WriteNetworkTag:
                {
                    C775_WriteNetworkTag_Resp *resp;

                    resp = (C775_WriteNetworkTag_Resp *) cmd.commandBuffer.get();
                    cmds[i].responseCode
                                = gatewayRequestsHandler.getGatewayWrappedRequestsHandler().Handle(
                                                                                                   handle,
                                                                                                   NetworkManager_UniqueID().bytes,
                                                                                                   *((C775_WriteNetworkTag_Req*) command->command),
                                                                                                   *resp);
                    cmds[i].command = resp;
                    break;
                }
                case CMDID_C776_ReadNetworkTag:
                {
                    C776_ReadNetworkTag_Resp *resp;

                    resp = (C776_ReadNetworkTag_Resp *) cmd.commandBuffer.get();
                    cmds[i].responseCode
                                = gatewayRequestsHandler.getGatewayWrappedRequestsHandler().Handle(
                                                                                                   handle,
                                                                                                   NetworkManager_UniqueID().bytes,
                                                                                                   *((C776_ReadNetworkTag_Req*) command->command),
                                                                                                   *resp);
                    cmds[i].command = resp;
                    break;
                }
                case CMDID_C778_ReadBatteryLife:
                {
                    C778_ReadBatteryLife_Resp *resp;

                    resp = (C778_ReadBatteryLife_Resp *) cmd.commandBuffer.get();
                    cmds[i].responseCode
                                = gatewayRequestsHandler.getGatewayWrappedRequestsHandler().Handle(
                                                                                                   handle,
                                                                                                   NetworkManager_UniqueID().bytes,
                                                                                                   *((C778_ReadBatteryLife_Req*) command->command),
                                                                                                   *resp);

                    cmds[i].command = resp;
                    break;
                }
                case CMDID_C779_ReportDeviceHealth:
                {
                    C779_ReportDeviceHealth_Resp *resp;

                    resp = (C779_ReportDeviceHealth_Resp *) cmd.commandBuffer.get();
                    cmds[i].responseCode
                                = gatewayRequestsHandler.getGatewayWrappedRequestsHandler().Handle(
                                                                                                   handle,
                                                                                                   NetworkManager_UniqueID().bytes,
                                                                                                   *((C779_ReportDeviceHealth_Req*) command->command),
                                                                                                   *resp);
                    cmds[i].command = resp;
                    break;
                }
                case CMDID_C780_ReportNeighborHealthList:
                {
                    C780_ReportNeighborHealthList_Resp *resp;

                    resp = (C780_ReportNeighborHealthList_Resp *) cmd.commandBuffer.get();
                    cmds[i].responseCode
                                = gatewayRequestsHandler.getGatewayWrappedRequestsHandler().Handle(
                                                                                                   handle,
                                                                                                   NetworkManager_UniqueID().bytes,
                                                                                                   *((C780_ReportNeighborHealthList_Req*) command->command),
                                                                                                   *resp);
                    cmds[i].command = resp;
                    break;
                }
                case CMDID_C781_ReadDeviceNicknameAddress:
                {
                    C781_ReadDeviceNicknameAddress_Resp *resp;

                    resp = (C781_ReadDeviceNicknameAddress_Resp *) cmd.commandBuffer.get();
                    cmds[i].responseCode
                                = gatewayRequestsHandler.getGatewayWrappedRequestsHandler().Handle(
                                                                                                   handle,
                                                                                                   NetworkManager_UniqueID().bytes,
                                                                                                   *((C781_ReadDeviceNicknameAddress_Req*) command->command),
                                                                                                   *resp);
                    cmds[i].command = resp;
                    break;
                }
                case CMDID_C782_ReadSessionEntries:
                {
                    C782_ReadSessionEntries_Resp *resp;

                    resp = (C782_ReadSessionEntries_Resp *) cmd.commandBuffer.get();
                    cmds[i].responseCode
                                = gatewayRequestsHandler.getGatewayWrappedRequestsHandler().Handle(
                                                                                                   handle,
                                                                                                   NetworkManager_UniqueID().bytes,
                                                                                                   *((C782_ReadSessionEntries_Req*) command->command),
                                                                                                   *resp);
                    cmds[i].command = resp;
                    break;
                }
                case CMDID_C783_ReadSuperframeList:
                {
                    C783_ReadSuperframeList_Resp *resp;

                    resp = (C783_ReadSuperframeList_Resp *) cmd.commandBuffer.get();
                    cmds[i].responseCode
                                = gatewayRequestsHandler.getGatewayWrappedRequestsHandler().Handle(
                                                                                                   handle,
                                                                                                   NetworkManager_UniqueID().bytes,
                                                                                                   *((C783_ReadSuperframeList_Req*) command->command),
                                                                                                   *resp);

                    cmds[i].command = resp;
                    break;
                }
                case CMDID_C784_ReadLinkList:
                {
                    C784_ReadLinkList_Resp *resp;

                    resp = (C784_ReadLinkList_Resp *) cmd.commandBuffer.get();
                    cmds[i].responseCode
                                = gatewayRequestsHandler.getGatewayWrappedRequestsHandler().Handle(
                                                                                                   handle,
                                                                                                   NetworkManager_UniqueID().bytes,
                                                                                                   *((C784_ReadLinkList_Req*) command->command),
                                                                                                   *resp);
                    cmds[i].command = resp;
                    break;
                }
                case CMDID_C785_ReadGraphList:
                {
                    C785_ReadGraphList_Resp *resp;

                    resp = (C785_ReadGraphList_Resp *) cmd.commandBuffer.get();
                    cmds[i].responseCode
                                = gatewayRequestsHandler.getGatewayWrappedRequestsHandler().Handle(
                                                                                                   handle,
                                                                                                   NetworkManager_UniqueID().bytes,
                                                                                                   *((C785_ReadGraphList_Req*) command->command),
                                                                                                   *resp);
                    cmds[i].command = resp;
                    break;
                }
                case CMDID_C786_ReadNeighborPropertyFlag:
                {
                    C786_ReadNeighborPropertyFlag_Resp *resp;

                    resp = (C786_ReadNeighborPropertyFlag_Resp *) cmd.commandBuffer.get();
                    cmds[i].responseCode
                                = gatewayRequestsHandler.getGatewayWrappedRequestsHandler().Handle(
                                                                                                   handle,
                                                                                                   NetworkManager_UniqueID().bytes,
                                                                                                   *((C786_ReadNeighborPropertyFlag_Req*) command->command),
                                                                                                   *resp);
                    cmds[i].command = resp;
                    break;
                }
                case CMDID_C787_ReportNeighborSignalLevels:
                {
                    C787_ReportNeighborSignalLevels_Resp *resp;
                    resp = (C787_ReportNeighborSignalLevels_Resp *) cmd.commandBuffer.get();

                    cmds[i].responseCode
                                = gatewayRequestsHandler.getGatewayWrappedRequestsHandler().Handle(
                                                                                                   handle,
                                                                                                   NetworkManager_UniqueID().bytes,
                                                                                                   *((C787_ReportNeighborSignalLevels_Req*) command->command),
                                                                                                   *resp);
                    cmds[i].command = resp;
                    break;
                }
                case CMDID_C794_ReadUTCTime:
                {
                    C794_ReadUTCTime_Resp *resp;
                    resp = (C794_ReadUTCTime_Resp *) cmd.commandBuffer.get();

                    cmds[i].responseCode
                                = gatewayRequestsHandler.getGatewayWrappedRequestsHandler().Handle(
                                                                                                   handle,
                                                                                                   NetworkManager_UniqueID().bytes,
                                                                                                   *((C794_ReadUTCTime_Req*) command->command),
                                                                                                   *resp);
                    cmds[i].command = resp;
                    break;
                }
                case CMDID_C799_RequestService:
                {
                    C799_RequestService_Req *req = (C799_RequestService_Req *) command->command;
                    serviceRequests.push_back(
                                              RequestServiceFlowHandler::Ptr(
                                                                             new RequestServiceFlowHandler(
                                                                                                           commonData,
                                                                                                           boost::bind(
                                                                                                                       &GenericCommandsHandler::RegisterDefaultHandler,
                                                                                                                       this,
                                                                                                                       _1))));
                    serviceRequests.back()->ProcessRequest(source, *req, cmd);

                    cmds[i].command = cmd.command.command;
                    cmds[i].responseCode = cmd.command.responseCode;

                    break;
                }
                case CMDID_C800_ReadServiceList:
                {
                    C800_ReadServiceList_Resp *resp;
                    resp = (C800_ReadServiceList_Resp *) cmd.commandBuffer.get();

                    cmds[i].responseCode
                                = gatewayRequestsHandler.getGatewayWrappedRequestsHandler().Handle(
                                                                                                   handle,
                                                                                                   NetworkManager_UniqueID().bytes,
                                                                                                   *((C800_ReadServiceList_Req*) command->command),
                                                                                                   *resp);
                    cmds[i].command = resp;
                    break;
                }
                case CMDID_C802_ReadRouteList:
                {
                    C802_ReadRouteList_Resp *resp;
                    resp = (C802_ReadRouteList_Resp *) cmd.commandBuffer.get();

                    cmds[i].responseCode
                                = gatewayRequestsHandler.getGatewayWrappedRequestsHandler().Handle(
                                                                                                   handle,
                                                                                                   NetworkManager_UniqueID().bytes,
                                                                                                   *((C802_ReadRouteList_Req*) command->command),
                                                                                                   *resp);
                    cmds[i].command = resp;
                    break;
                }
                case CMDID_C803_ReadSourceRoute:
                {
                    C803_ReadSourceRoute_Resp *resp;
                    resp = (C803_ReadSourceRoute_Resp *) cmd.commandBuffer.get();

                    cmds[i].responseCode
                                = gatewayRequestsHandler.getGatewayWrappedRequestsHandler().Handle(
                                                                                                   handle,
                                                                                                   NetworkManager_UniqueID().bytes,
                                                                                                   *((C803_ReadSourceRoute_Req*) command->command),
                                                                                                   *resp);
                    cmds[i].command = resp;
                    break;
                }
                case CMDID_C814_ReadDeviceListEntries:
                {
                    C814_ReadDeviceListEntries_Resp *resp;
                    resp = (C814_ReadDeviceListEntries_Resp *) cmd.commandBuffer.get();
                    cmds[i].responseCode
                                = gatewayRequestsHandler.getGatewayWrappedRequestsHandler().Handle(
                                                                                                   handle,
                                                                                                   NetworkManager_UniqueID().bytes,
                                                                                                   *((C814_ReadDeviceListEntries_Req*) command->command),
                                                                                                   *resp);
                    cmds[i].command = resp;
                    break;
                }
                case CMDID_C817_ReadChannelBlacklist:
                {
                    C817_ReadChannelBlacklist_Resp *resp;
                    resp = (C817_ReadChannelBlacklist_Resp *) cmd.commandBuffer.get();
                    cmds[i].responseCode
                                = gatewayRequestsHandler.getGatewayWrappedRequestsHandler().Handle(
                                                                                                   handle,
                                                                                                   NetworkManager_UniqueID().bytes,
                                                                                                   *((C817_ReadChannelBlacklist_Req*) command->command),
                                                                                                   *resp);
                    cmds[i].command = resp;
                    break;
                }
                case CMDID_C818_WriteChannelBlacklist:
                {
                    C818_WriteChannelBlacklist_Resp *resp;
                    resp = (C818_WriteChannelBlacklist_Resp *) cmd.commandBuffer.get();
                    cmds[i].responseCode
                                = gatewayRequestsHandler.getGatewayWrappedRequestsHandler().Handle(
                                                                                                   handle,
                                                                                                   NetworkManager_UniqueID().bytes,
                                                                                                   *((C818_WriteChannelBlacklist_Req*) command->command),
                                                                                                   *resp);
                    cmds[i].command = resp;
                    break;
                }
                case CMDID_C821_WriteNetworkAccessMode:
                {
                    C821_WriteNetworkAccessMode_Resp *resp;
                    resp = (C821_WriteNetworkAccessMode_Resp *) cmd.commandBuffer.get();
                    cmds[i].responseCode
                                = gatewayRequestsHandler.getGatewayWrappedRequestsHandler().Handle(
                                                                                                   handle,
                                                                                                   NetworkManager_UniqueID().bytes,
                                                                                                   *((C821_WriteNetworkAccessMode_Req*) command->command),
                                                                                                   *resp);
                    cmds[i].command = resp;
                    break;
                }
                case CMDID_C822_ReadNetworkAccessMode:
                {
                    C822_ReadNetworkAccessMode_Resp *resp;
                    resp = (C822_ReadNetworkAccessMode_Resp *) cmd.commandBuffer.get();
                    cmds[i].responseCode
                                = gatewayRequestsHandler.getGatewayWrappedRequestsHandler().Handle(
                                                                                                   handle,
                                                                                                   NetworkManager_UniqueID().bytes,
                                                                                                   *((C822_ReadNetworkAccessMode_Req*) command->command),
                                                                                                   *resp);
                    cmds[i].command = resp;
                    break;
                }
                case CMDID_C832_ReadNetworkDeviceIdentity:
                {
                    C832_ReadNetworkDeviceIdentity_Resp *resp;
                    resp = (C832_ReadNetworkDeviceIdentity_Resp *) cmd.commandBuffer.get();
                    cmds[i].responseCode
                                = gatewayRequestsHandler.getGatewayWrappedRequestsHandler().Handle(
                                                                                                   handle,
                                                                                                   NetworkManager_UniqueID().bytes,
                                                                                                   *((C832_ReadNetworkDeviceIdentity_Req*) command->command),
                                                                                                   *resp);
                    cmds[i].command = resp;
                    break;
                }
                case CMDID_C833_ReadNetworkDeviceNeighbourHealth:
                {
                    C833_ReadNetworkDeviceNeighbourHealth_Resp *resp;
                    resp = (C833_ReadNetworkDeviceNeighbourHealth_Resp *) cmd.commandBuffer.get();
                    cmds[i].responseCode
                                = gatewayRequestsHandler.getGatewayWrappedRequestsHandler().Handle(
                                                                                                   handle,
                                                                                                   NetworkManager_UniqueID().bytes,
                                                                                                   *((C833_ReadNetworkDeviceNeighbourHealth_Req*) command->command),
                                                                                                   *resp);
                    cmds[i].command = resp;
                    break;
                }
                case CMDID_C834_ReadNetworkTopologyInformation:
                {
                    C834_ReadNetworkTopologyInformation_Resp *resp;
                    resp = (C834_ReadNetworkTopologyInformation_Resp *) cmd.commandBuffer.get();
                    cmds[i].responseCode
                                = gatewayRequestsHandler.getGatewayWrappedRequestsHandler().Handle(
                                                                                                   handle,
                                                                                                   NetworkManager_UniqueID().bytes,
                                                                                                   *((C834_ReadNetworkTopologyInformation_Req*) command->command),
                                                                                                   *resp);
                    cmds[i].command = resp;
                    break;
                }
                case CMDID_C840_ReadDeviceStatistics:
                {
                    C840_ReadDeviceStatistics_Resp *resp;
                    resp = (C840_ReadDeviceStatistics_Resp *) cmd.commandBuffer.get();
                    cmds[i].responseCode
                                = gatewayRequestsHandler.getGatewayWrappedRequestsHandler().Handle(
                                                                                                   handle,
                                                                                                   NetworkManager_UniqueID().bytes,
                                                                                                   *((C840_ReadDeviceStatistics_Req*) command->command),
                                                                                                   *resp);
                    cmds[i].command = resp;
                    break;
                }
                case CMDID_C842_WriteDeviceSchedulingFlags:
                {
                    C842_WriteDeviceSchedulingFlags_Resp *resp;
                    resp = (C842_WriteDeviceSchedulingFlags_Resp *) cmd.commandBuffer.get();
                    cmds[i].responseCode
                                = gatewayRequestsHandler.getGatewayWrappedRequestsHandler().Handle(
                                                                                                   handle,
                                                                                                   NetworkManager_UniqueID().bytes,
                                                                                                   *((C842_WriteDeviceSchedulingFlags_Req*) command->command),
                                                                                                   *resp);
                    cmds[i].command = resp;
                    break;
                }
                case CMDID_C843_ReadDeviceSchedulingFlags:
                {
                    C843_ReadDeviceSchedulingFlags_Resp *resp;
                    resp = (C843_ReadDeviceSchedulingFlags_Resp *) cmd.commandBuffer.get();
                    cmds[i].responseCode
                                = gatewayRequestsHandler.getGatewayWrappedRequestsHandler().Handle(
                                                                                                   handle,
                                                                                                   NetworkManager_UniqueID().bytes,
                                                                                                   *((C843_ReadDeviceSchedulingFlags_Req*) command->command),
                                                                                                   *resp);
                    cmds[i].command = resp;
                    break;
                }
                case CMDID_C844_ReadNetworkConstraints:
                {
                    C844_ReadNetworkConstraints_Resp *resp;
                    resp = (C844_ReadNetworkConstraints_Resp *) cmd.commandBuffer.get();
                    cmds[i].responseCode
                                = gatewayRequestsHandler.getGatewayWrappedRequestsHandler().Handle(
                                                                                                   handle,
                                                                                                   NetworkManager_UniqueID().bytes,
                                                                                                   *((C844_ReadNetworkConstraints_Req*) command->command),
                                                                                                   *resp);
                    cmds[i].command = resp;
                    break;
                }
                case CMDID_C845_WriteNetworkConstraints:
                {
                    C845_WriteNetworkConstraints_Resp *resp;
                    resp = (C845_WriteNetworkConstraints_Resp *) cmd.commandBuffer.get();
                    cmds[i].responseCode
                                = gatewayRequestsHandler.getGatewayWrappedRequestsHandler().Handle(
                                                                                                   handle,
                                                                                                   NetworkManager_UniqueID().bytes,
                                                                                                   *((C845_WriteNetworkConstraints_Req*) command->command),
                                                                                                   *resp);
                    cmds[i].command = resp;
                    break;
                }
                case CMDID_C64765_NivisMetaCommand:
                {
                    C64765_NivisMetaCommand_Resp *resp;
                    resp = (C64765_NivisMetaCommand_Resp *) cmd.commandBuffer.get();
                    cmds[i].responseCode
                                = gatewayRequestsHandler.getGatewayWrappedRequestsHandler().Handle(
                                                                                                   handle,
                                                                                                   *((C64765_NivisMetaCommand_Req*) command->command),
                                                                                                   resp);
                    cmds[i].command = resp;
                    break;
                }
                default:
                {
                    LOG_WARN("HandleGWRequests() invalid command->commandID : " << (int) command->commandID);
                    break;
                }
            }
        }

        if (cmds[i].command)
        {
            hart7::util::NMLog::logCommandResponse((int) cmds[i].commandID, cmds[i].responseCode, cmds[i].command,
                                                   source);
        }
        else
        {
            return;
        }
    }

    try
    {
        NE::Model::Services::Service& service = commonData.utils.GetServiceTo(Gateway_Nickname());
        WHartCommandList commandsRespList = { commandsList.count, cmds };
        requestSend.TransmitResponse(handle, (stack::WHartServiceID) service.getServiceId(), commandsRespList,
                                     hart7::stack::WHartSessionKey::sessionKeyed);
    }
    catch (std::exception& ex)
    {
        LOG_ERROR(ex.what());
    }

}

void GenericCommandsHandler::HandleDeviceRequests(WHartHandle handle, const WHartAddress& source,
                                                  const WHartCommandList& commandsList)
{
    LOG_DEBUG("HandleDeviceRequests()");
    std::vector<WHartCommandWrapper> responses;

    LOG_DEBUG("HandleDeviceRequests() commandsList.count : " << (int) commandsList.count);
    WHartCommand cmds[commandsList.count];

    WHartCommand gwNotResps[1];

    for (int i = 0; i < commandsList.count; i++)
    {
        WHartCommand *command = commandsList.list + i;

        LOG_DEBUG("HandleDeviceRequests() command->commandID : " << (int) command->commandID);
        if (command->command)
        {
            hart7::util::NMLog::logCommand((int) command->commandID, command->command, source);
        }

        responses.push_back(WHartCommandWrapper());
        WHartCommandWrapper& cmd = responses.back();

        if (HandleDefaults(source, *command, cmd))
        {
            LOG_DEBUG("Default command handler responded to command ID=" << command->commandID);

            cmds[i].command = cmd.command.command;
            cmds[i].responseCode = cmd.command.responseCode;
            cmds[i].commandID = command->commandID;
        }
        else
        {
            cmds[i].commandID = command->commandID;

            switch (command->commandID)
            {
                case CMDID_C799_RequestService:
                {
                    C799_RequestService_Req *req = (C799_RequestService_Req *) command->command;
                    serviceRequests.push_back(
                                              RequestServiceFlowHandler::Ptr(
                                                                             new RequestServiceFlowHandler(
                                                                                                           commonData,
                                                                                                           boost::bind(
                                                                                                                       &GenericCommandsHandler::RegisterDefaultHandler,
                                                                                                                       this,
                                                                                                                       _1))));
                    serviceRequests.back()->ProcessRequest(source, *req, cmd);

                    cmds[i].command = cmd.command.command;
                    cmds[i].responseCode = cmd.command.responseCode;

                    break;
                }
                case CMDID_C801_DeleteService:
                {
                    C801_DeleteService_Req *req = (C801_DeleteService_Req *) command->command;
                    TerminateServiceFlowHandler terminateService(commonData);

                    terminateService.ProcessRequest(source, *req, cmd);

                    cmds[i].command = cmd.command.command;
                    cmds[i].responseCode = cmd.command.responseCode;

                    break;
                }

                default:
                {
                    LOG_WARN("HandleDeviceRequests() invalid command->commandID : " << (int) command->commandID);
                    break;
                }
            }
        }

        if (command->command)
        {
            hart7::util::NMLog::logCommandResponse((int) cmds[i].commandID, cmds[i].responseCode, cmds[i].command,
                                                   source);
        }
        else
        {
            return;
        }

        //send the notification to the GW
        //NM notify the GW with a 973 command response (because the GW do not parse the 799 command responses)
        if (cmds[i].commandID == CMDID_C799_RequestService)
        {
            //notify gw only when the request is responsed with Success
            if (cmds[i].responseCode == 0)
            {
                C64765_NivisMetaCommand_Resp *response;
                WHartCommandWrapper cmd = WHartCommandWrapper();
                response = (C64765_NivisMetaCommand_Resp *)cmd.commandBuffer.get();

                if (generateGWNotification(response, cmds[i], source))
                {
                    gwNotResps[0].commandID = CMDID_C64765_NivisMetaCommand;
                    gwNotResps[0].command = response;
                    gwNotResps[0].responseCode = 0;

                    try
                    {
                        LOG_DEBUG("sending notifications to GW : ");
                        NE::Model::Services::Service& serviceToGW = commonData.utils.GetServiceTo(GATEWAY_ADDRESS);
                        LOG_DEBUG("sending notifications with Service : " << serviceToGW);
                        WHartCommandList commandsGwRespList;
                        commandsGwRespList.count = 1;
                        commandsGwRespList.list = gwNotResps;
                        requestSend.TransmitRequest(GATEWAY_ADDRESS, stack::whartpCommand,
                                                    (stack::WHartServiceID) serviceToGW.getServiceId(),
                                                    stack::wharttPublishNotify, commandsGwRespList,
                                                    commonData.utils.GetSessionType(GATEWAY_ADDRESS), *this);
                    }
                    catch (std::exception& ex)
                    {
                        LOG_ERROR(ex.what());
                    }
                }
            }
        }
    }

    try
    {
        LOG_DEBUG("sending response with source : " << source);
        NE::Model::Services::Service& service = commonData.utils.GetServiceTo(source);
        LOG_DEBUG("sending response with Service : " << service);
        WHartCommandList commandsRespList = { commandsList.count, cmds };
        requestSend.TransmitResponse(handle, (stack::WHartServiceID) service.getServiceId(), commandsRespList,
                                     hart7::stack::WHartSessionKey::sessionKeyed);
    }
    catch (std::exception& ex)
    {
        LOG_ERROR(ex.what());
    }
}

//generates a 973 response in order to be correctly processed by the gw
//(in the future, the gw needs to parse 799 responses)
bool GenericCommandsHandler::generateGWNotification(C64765_NivisMetaCommand_Resp* resp, WHartCommand& command, WHartAddress address)
{
    resp->Nickname = GATEWAY_ADDRESS;
    std::memcpy(
                resp->DeviceUniqueId,
                hart7::util::getUniqueIdFromAddress64(
                                                      hart7::util::getAddress64(
                                                                                commonData.networkEngine.getDevicesTable(),
                                                                                address)).bytes, 5);
     uint16_t buffSize = 256;
     uint8_t commandBuff[buffSize];
     uint16_t writtenBytes;

     //constructs 973 response
     WHartCommandWrapper writeServiceCmd = WHartCommandWrapper();
     C973_WriteService_Resp* writeServiceResp = (C973_WriteService_Resp*)writeServiceCmd.commandBuffer.get();

     writeServiceResp->m_ucServiceID = ((C799_RequestService_Resp*)command.command)->m_ucServiceId;
     writeServiceResp->m_ucRouteID = ((C799_RequestService_Resp*)command.command)->m_ucRouteId;
     writeServiceResp->m_unPeerNickname = ((C799_RequestService_Resp*)command.command)->m_unNicknameOfPeer;
     writeServiceResp->m_ucRequestFlags = ((C799_RequestService_Resp*)command.command)->m_ucServiceRequestFlags;
     writeServiceResp->m_tPeriod.u32 = ((C799_RequestService_Resp*)command.command)->m_tPeriod.u32;
     writeServiceResp->m_eApplicationDomain = ((C799_RequestService_Resp*)command.command)->m_ucServiceApplicationDomain;

     Uint16 noOfUsedServices = commonData.networkEngine.getDevice(writeServiceResp->m_unPeerNickname).getMetaDataAttributes()->getUsedServices();
     Uint16 TotalNoOfServices = commonData.networkEngine.getDevice(writeServiceResp->m_unPeerNickname).getMetaDataAttributes()->getTotalServices();
     writeServiceResp->m_ucRemainingServicesNo = TotalNoOfServices - noOfUsedServices - 1;
     commonData.networkEngine.getDevice(writeServiceResp->m_unPeerNickname).getMetaDataAttributes()->setUsedServices(noOfUsedServices+1);

     writeServiceCmd.command.commandID = CMDID_C973_WriteService;
     writeServiceCmd.command.responseCode = 0;
     writeServiceCmd.command.command = writeServiceCmd.commandBuffer.get();

     //writeServiceCmd.command
     if (commonData.serializeResponse((uint16_t) writeServiceCmd.command.commandID, commandBuff, buffSize, writeServiceCmd.command.command, writtenBytes))
     {
         resp->CommandSize = writtenBytes;// + 4;
         memcpy(resp->Command, commandBuff, writtenBytes);
     }
     else
     {
         LOG_ERROR("Can not serialize " << (int) writeServiceCmd.command.commandID);
         return false;
     }

     return true;

}

void GenericCommandsHandler::RegisterDefaultHandler(IDefaultCommandHandler::Ptr defaultHandler)
{
    defaultHandlers.push_back(defaultHandler);
}

struct DefaultHandlerFinished
{
        bool operator()(IDefaultCommandHandler::Ptr& ptr)
        {
            return !ptr || ptr->CommandHandlerFinished();
        }
};

bool GenericCommandsHandler::HandleDefaults(const WHartAddress& src, const WHartCommand& command,
                                            WHartCommandWrapper& response)
{
    bool commandHandled = false;
    for (std::vector<IDefaultCommandHandler::Ptr>::iterator it = defaultHandlers.begin(); it != defaultHandlers.end(); ++it)
    {
        if ((*it)->HandleCommand(src, command, response))
        {
            commandHandled = true;
            break;
        }
    }

    defaultHandlers.erase(std::remove_if(defaultHandlers.begin(), defaultHandlers.end(), DefaultHandlerFinished()),
                          defaultHandlers.end());

    return commandHandled;
}

}
}

void GenericCommandsHandler::ProcessConfirm(stack::WHartHandle requestHandle, const stack::WHartLocalStatus& localStatus,
                                     const stack::WHartCommandList& list)
{
    //do nothing
}

void GenericCommandsHandler::ProcessIndicate(stack::WHartHandle handle, const stack::WHartAddress& src,
                                      stack::WHartPriority priority, stack::WHartTransportType transportType,
                                      const stack::WHartCommandList& list)
{
    // do nothing
}
