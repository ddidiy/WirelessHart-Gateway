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
 * WHEngineOperationsVisitor.cpp
 *
 *  Created on: Jan 5, 2009
 *      Author: ioanpocol
 */

#include "WHEngineOperationsVisitor.h"
#include "../../util/ManagerUtils.h"
#include <ApplicationLayer/Model/WirelessNetworkManagerCommands.h>
#include <ApplicationLayer/Model/DataLinkLayerCommands.h>
#include <ApplicationLayer/Model/GatewayCommands.h>
#include <ApplicationLayer/Model/CommonTables.h>
#include <ApplicationLayer/Model/NivisSpecificCommands.h>

#define MAX_COMMAND_SIZE 256

using namespace hart7::nmanager::operations;
using namespace NE::Model::Operations;

WHEngineOperationsVisitor::WHEngineOperationsVisitor(EngineOperationsListPointer operationEventPointer_,
                                                     hart7::nmanager::CommonData& commonData_) :
    operationEventPointer(operationEventPointer_), commonData(commonData_)
{
}

WHEngineOperationsVisitor::~WHEngineOperationsVisitor()
{
}

void WHEngineOperationsVisitor::setASN(Uint32 taiCutover)
{
    asn.hi = 0;
    asn.u32 = taiCutover;
}

std::vector<WHOperationPointer> WHEngineOperationsVisitor::getWHOperations()
{
    return whOperations;
}

void WHEngineOperationsVisitor::resetWHOperations()
{
    whOperations.clear();
}

WHOperationPointer WHEngineOperationsVisitor::initWHOperation(IEngineOperation& operation)
{
    WHOperationPointer whOperation(new WHOperation(operation.getSubnetId(), operation.getOwner(),
                                                   operation.getOperationId()));

    if (operationEventPointer->isProxyAddress() && operation.getOwner()
                == operationEventPointer->getRequesterAddress32() && operation.getOwner() != Gateway_Nickname())
    {

        if (operationEventPointer->isShortAddress())
        {
            whOperation->getDestinationAddress().type = WHartAddress::whartaProxyShort;
            whOperation->getDestinationAddress().address.proxyShort.nickname
                        = operationEventPointer->getProxyAddress32();
            whOperation->getDestinationAddress().address.proxyShort.destNickname
                        = operationEventPointer->getRequesterAddress32();
        }
        else
        {
            whOperation->getDestinationAddress().type = WHartAddress::whartaProxy;
            whOperation->getDestinationAddress().address.proxy.nickname = operationEventPointer->getProxyAddress32();
            whOperation->getDestinationAddress().address.proxy.uniqueID
                        = hart7::util::getUniqueIdFromAddress64(operationEventPointer->getRequesterAddress64());
        }
    }
    else
    {
        whOperation->getDestinationAddress().type = WHartAddress::whartaNickname;
        whOperation->getDestinationAddress().address.nickname = operation.getOwner();
    }

    whOperations.push_back(whOperation);
    return whOperation;
}

bool WHEngineOperationsVisitor::visitServiceAddedOperation(ServiceAddedOperation& operation)
{
    WHOperationPointer whOperation = initWHOperation(operation);

    whOperation->commandData.reset(new uint8_t[MAX_COMMAND_SIZE]);
    C973_WriteService_Req* writeServiceReq = (C973_WriteService_Req*) (whOperation->commandData.get());

    writeServiceReq->m_ucServiceID = (uint8_t) operation.serviceId;
    writeServiceReq->m_ucRouteID = (uint8_t) operation.routeId;
    writeServiceReq->m_unPeerNickname = operation.peerAddress32 & 0xFFFF;
    writeServiceReq->m_ucRequestFlags = 0;

    writeServiceReq->m_ucRequestFlags |= (operation.source) ? ServiceRequestFlagsMask_Source : 0;
    writeServiceReq->m_ucRequestFlags |= (operation.sink) ? ServiceRequestFlagsMask_Sink : 0;
    writeServiceReq->m_ucRequestFlags |= (operation.intermittent) ? ServiceRequestFlagsMask_Intermittent : 0;

    writeServiceReq->m_tPeriod.u32 = operation.period;
    writeServiceReq->m_eApplicationDomain = operation.applicationDomain;

    whOperation->getWHartCommand().commandID = CMDID_C973_WriteService;
    whOperation->getWHartCommand().command = writeServiceReq;

    return true;
}

bool WHEngineOperationsVisitor::visitServiceRemovedOperation(ServiceRemovedOperation& operation)
{

    WHOperationPointer whOperation = initWHOperation(operation);

    whOperation->commandData.reset(new uint8_t[MAX_COMMAND_SIZE]);
    C801_DeleteService_Req* deleteServiceReq = (C801_DeleteService_Req*) (whOperation->commandData.get());

    deleteServiceReq->m_ucServiceId = (uint8_t) operation.serviceId;

    deleteServiceReq->m_ucReason = (uint8_t) operation.deleteReason;

    if (whOperation->getDestinationAddress() == Gateway_Nickname() || whOperation->getDestinationAddress()
                == NetworkManager_Nickname())
    {
        deleteServiceReq->m_ucReason = 0xFF;
        deleteServiceReq->m_peerNickname = (uint16_t) (operation.peerAddress);
    }

    whOperation->getWHartCommand().commandID = CMDID_C801_DeleteService;
    whOperation->getWHartCommand().command = deleteServiceReq;

    return true;
}

bool WHEngineOperationsVisitor::visitRouteAddedOperation(RouteAddedOperation& operation)
{
    WHOperationPointer whOperation = initWHOperation(operation);
    //TODO: the graphID is on the device and has 256 range

    //todo change size to the actual size of the command
    whOperation->commandData.reset(new uint8_t[MAX_COMMAND_SIZE]);

    C974_WriteRoute_Req* c974_WriteRoute_Req = (C974_WriteRoute_Req*) (whOperation->commandData.get());

    c974_WriteRoute_Req->m_unPeerNickname = (uint16_t) operation.peerAddress;

    c974_WriteRoute_Req->m_unGraphID = operation.graphId;
    c974_WriteRoute_Req->m_ucRouteID = (Uint8) operation.routeId;

    whOperation->getWHartCommand().commandID = CMDID_C974_WriteRoute;
    whOperation->getWHartCommand().command = c974_WriteRoute_Req;

    return true;
}

bool WHEngineOperationsVisitor::visitRouteRemovedOperation(RouteRemovedOperation& operation)
{
    WHOperationPointer whOperation = initWHOperation(operation);

    //todo change size to the actual size of the command
    whOperation->commandData.reset(new uint8_t[MAX_COMMAND_SIZE]);

    C975_DeleteRoute_Req* c975_DeleteRoute_Req = (C975_DeleteRoute_Req*) (whOperation->commandData.get());

    c975_DeleteRoute_Req->m_ucRouteID = (Uint8) operation.routeId;

    whOperation->getWHartCommand().commandID = CMDID_C975_DeleteRoute;
    whOperation->getWHartCommand().command = c975_DeleteRoute_Req;

    return true;
}

bool WHEngineOperationsVisitor::visitSourceRouteAddedOperation(
                                                               NE::Model::Operations::SourceRouteAddedOperation& operation)
{

    WHOperationPointer childWHOperation = initWHOperation(operation);

    childWHOperation->commandData.reset(new uint8_t[MAX_COMMAND_SIZE]);
    C976_WriteSourceRoute_Req* c976_WriteSourceRoute_Req =
                (C976_WriteSourceRoute_Req*) (childWHOperation->commandData.get());

    c976_WriteSourceRoute_Req->m_ucRouteID = (Uint8) operation.routeId;

    int index = 0;
    for (std::list<Address32>::iterator it = operation.destinations.begin(); it != operation.destinations.end(); ++it)
    {
        c976_WriteSourceRoute_Req->m_aNicknameHopEntries[index++] = *it; // TODO Address should be removed
    }

    c976_WriteSourceRoute_Req->m_ucHopsNo = operation.destinations.size();

    LOG_INFO("Generated commandID=" << CMDID_C976_WriteSourceRoute << " with size="
                << (int) c976_WriteSourceRoute_Req->m_ucHopsNo);

    childWHOperation->getWHartCommand().commandID = CMDID_C976_WriteSourceRoute;
    childWHOperation->getWHartCommand().command = c976_WriteSourceRoute_Req;

    return true;
}

bool WHEngineOperationsVisitor::visitSourceRouteRemovedOperation(
                                                                 NE::Model::Operations::SourceRouteRemovedOperation& operation)
{
    WHOperationPointer childWHOperation = initWHOperation(operation);

    childWHOperation->commandData.reset(new uint8_t[MAX_COMMAND_SIZE]);
    C977_DeleteSourceRoute_Req* c977_DeleteSourceRoute_Req =
                (C977_DeleteSourceRoute_Req*) (childWHOperation->commandData.get());
    c977_DeleteSourceRoute_Req->m_ucRouteID = (Uint8) operation.routeId;

    childWHOperation->getWHartCommand().commandID = CMDID_C977_DeleteSourceRoute;
    childWHOperation->getWHartCommand().command = c977_DeleteSourceRoute_Req;

    return true;
}

bool WHEngineOperationsVisitor::visitSuperframeAddedOperation(SuperframeAddedOperation& operation)
{
    WHOperationPointer whOperation = initWHOperation(operation);

    //todo change size to the actual size of the command
    whOperation->commandData.reset(new uint8_t[MAX_COMMAND_SIZE]);

    C965_WriteSuperframe_Req* c965_WriteSuperframe_Req = (C965_WriteSuperframe_Req*) (whOperation->commandData.get());

    c965_WriteSuperframe_Req->m_unSuperframeSlotsNo = operation.superframeLength;//getSuperframeLength();
    c965_WriteSuperframe_Req->m_ucSuperframeID = operation.superframeId;//getSuperframeID();
    c965_WriteSuperframe_Req->m_ucSuperframeMode = operation.superframeMode;//1;//operation.getIsIdle() ? 0x00 : 0x01;
    c965_WriteSuperframe_Req->m_ucReserved = 0;
    memset(c965_WriteSuperframe_Req->m_tExecutionTime, 0, 5); //TODO check if this is ok

    whOperation->getWHartCommand().commandID = CMDID_C965_WriteSuperframe;
    whOperation->getWHartCommand().command = c965_WriteSuperframe_Req;

    return true;
}

bool WHEngineOperationsVisitor::visitNeighborGraphAddedOperation(NeighborGraphAddedOperation& operation)
{
    WHOperationPointer whOperation = initWHOperation(operation);

    // TODO change size to the actual size of the command
    whOperation->commandData.reset(new uint8_t[MAX_COMMAND_SIZE]);

    C969_WriteGraphNeighbourPair_Req* c969_WriteGraphNeighbourPair_Req =
                (C969_WriteGraphNeighbourPair_Req*) (whOperation->commandData.get());

    c969_WriteGraphNeighbourPair_Req->m_unGraphID = operation.getGraphId();
    c969_WriteGraphNeighbourPair_Req->m_unNeighborNickname = operation.getNeighbor();

    whOperation->getWHartCommand().commandID = CMDID_C969_WriteGraphNeighbourPair;
    whOperation->getWHartCommand().command = c969_WriteGraphNeighbourPair_Req;

    return true;
}

bool WHEngineOperationsVisitor::visitNeighborGraphRemovedOperation(NeighborGraphRemovedOperation& operation)
{
    WHOperationPointer whOperation = initWHOperation(operation);

    // TODO change size to the actual size of the command
    whOperation->commandData.reset(new uint8_t[MAX_COMMAND_SIZE]);

    C970_DeleteGraphConnection_Req* c970_DeleteGraphConnection_Req =
                (C970_DeleteGraphConnection_Req*) (whOperation->commandData.get());

    c970_DeleteGraphConnection_Req->m_unGraphID = operation.getGraphId();
    c970_DeleteGraphConnection_Req->m_unNeighborNickname = operation.getNeighbor();

    whOperation->getWHartCommand().commandID = CMDID_C970_DeleteGraphConnection;
    whOperation->getWHartCommand().command = c970_DeleteGraphConnection_Req;
    return true;
}

bool WHEngineOperationsVisitor::visitLinkAddedOperation(NE::Model::Operations::LinkAddedOperation& operation)
{
    WHOperationPointer whOperation = initWHOperation(operation);

    //TODO change size to the actual size of the command
    whOperation->commandData.reset(new uint8_t[256]);

    C967_WriteLink_Req* c967_WriteLink_Req = (C967_WriteLink_Req*) (whOperation->commandData.get());

    c967_WriteLink_Req->m_unSlotNumber = operation.GetSlotNo();
    c967_WriteLink_Req->m_unNeighborNickname = (uint16_t) operation.peerAddress;
    c967_WriteLink_Req->m_ucSuperframeID = operation.superframeId;
    c967_WriteLink_Req->m_ucChannelOffset = operation.GetChannelOffset();

    c967_WriteLink_Req->m_eLinkType = operation.linkType;
    c967_WriteLink_Req->m_ucLinkOptions = (operation.transmission ? 0x01 : 0) | (operation.reception ? 0x02 : 0)
                | (operation.shared ? 0x04 : 0);

    //uint8_t m_ucLinkOptions: 0x01- Transmit, 0x02- Receive, 0x04- Shared
    //uint8_t m_eLinkType: 0- Normal, 1- Discovery, 2- Broadcast, 3- Join

    whOperation->getWHartCommand().commandID = CMDID_C967_WriteLink;
    whOperation->getWHartCommand().command = c967_WriteLink_Req;

    return true;
}

bool WHEngineOperationsVisitor::visitLinkRemovedOperation(NE::Model::Operations::LinkRemovedOperation& operation)
{
    //TODO see when the operation will contain the <SfID, SlotNo, NeighborNickname> unique identifier

    WHOperationPointer whOperation = initWHOperation(operation);
    //todo change size to the actual size of the command
    whOperation->commandData.reset(new uint8_t[256]);

    C968_DeleteLink_Req* c968_DeleteLink_Req = (C968_DeleteLink_Req*) (whOperation->commandData.get());

    c968_DeleteLink_Req->m_unSlotNumber = operation.GetSlotNo();
    c968_DeleteLink_Req->m_unNeighborNickname = (uint16_t) operation.peerAddress;
    c968_DeleteLink_Req->m_ucSuperframeID = operation.superframeId;

    whOperation->getWHartCommand().commandID = CMDID_C968_DeleteLink;
    whOperation->getWHartCommand().command = c968_DeleteLink_Req;

    return true;
}

bool WHEngineOperationsVisitor::visitReadLinksOperation(NE::Model::Operations::ReadLinksOperation& operation)
{
    //TODO see when the operation will contain the <SfID, SlotNo, NeighborNickname> unique identifier

    WHOperationPointer whOperation = initWHOperation(operation);
    //todo change size to the actual size of the command
    whOperation->commandData.reset(new uint8_t[256]);

    C784_ReadLinkList_Req* c784_ReadLinkList_Req = (C784_ReadLinkList_Req*) (whOperation->commandData.get());

    c784_ReadLinkList_Req->m_unLinkIndex = operation.startIndex;

    c784_ReadLinkList_Req->m_ucNoOfLinksToRead = (uint16_t) operation.count;

    whOperation->getWHartCommand().commandID = CMDID_C784_ReadLinkList;
    whOperation->getWHartCommand().command = c784_ReadLinkList_Req;

    return true;
}

bool WHEngineOperationsVisitor::visitSetChannelsBlacklistOperation(
                                                                   NE::Model::Operations::SetChannelsBlacklistOperation& operation)
{

    WHOperationPointer whOperation = initWHOperation(operation);
    //todo change size to the actual size of the command
    whOperation->commandData.reset(new uint8_t[256]);

    C818_WriteChannelBlacklist_Req* c818_WriteChannelBlacklist_Req =
                (C818_WriteChannelBlacklist_Req*) (whOperation->commandData.get());

    c818_WriteChannelBlacklist_Req->m_unPendingChannelMapArray = operation.AvailableChannels;
    c818_WriteChannelBlacklist_Req->m_ucNoOfBitsInNewChannelMapArray = 15;

    whOperation->getWHartCommand().commandID = CMDID_C818_WriteChannelBlacklist;
    whOperation->getWHartCommand().command = c818_WriteChannelBlacklist_Req;

    return true;
}

bool WHEngineOperationsVisitor::visitWriteTimerIntervalOperation(
                                                                 NE::Model::Operations::WriteTimerIntervalOperation& operation)
{
    WHOperationPointer whOperation = initWHOperation(operation);
    //todo change size to the actual size of the command
    whOperation->commandData.reset(new uint8_t[256]);

    C795_WriteTimerInterval_Req* c795_WriteTimerInterval_Req =
                (C795_WriteTimerInterval_Req*) (whOperation->commandData.get());

    c795_WriteTimerInterval_Req->m_ulTimerInterval = operation.timeIntervalMsecs;
    c795_WriteTimerInterval_Req->m_ucTimerType = (uint8_t) operation.timerCode;

    whOperation->getWHartCommand().commandID = CMDID_C795_WriteTimerInterval;
    whOperation->getWHartCommand().command = c795_WriteTimerInterval_Req;

    return true;
}

bool WHEngineOperationsVisitor::visitSetClockSourceOperation(NE::Model::Operations::SetClockSourceOperation& operation)
{
    WHOperationPointer whOperation = initWHOperation(operation);
    //todo change size to the actual size of the command
    whOperation->commandData.reset(new uint8_t[256]);

    C971_WriteNeighbourPropertyFlag_Req* req = (C971_WriteNeighbourPropertyFlag_Req*) (whOperation->commandData.get());

    req->m_unNeighborNickname = (uint16_t) operation.neighborAddress;
    req->m_ucNeighborFlags = operation.flags;

    whOperation->getWHartCommand().commandID = CMDID_C971_WriteNeighbourPropertyFlag;
    whOperation->getWHartCommand().command = req;

    return true;
}

bool WHEngineOperationsVisitor::visit(NE::Model::Operations::ChangePriorityEngineOperation& operation)
{
    WHOperationPointer whOperation = initWHOperation(operation);
    //todo change size to the actual size of the command
    whOperation->commandData.reset(new uint8_t[256]);

    C811_WriteJoinPriority_Req* req = (C811_WriteJoinPriority_Req*) (whOperation->commandData.get());
    memset(req, 0, sizeof(C811_WriteJoinPriority_Req));
    req->JoinPriority = operation.getJoinPriority();

    whOperation->getWHartCommand().commandID = CMDID_C811_WriteJoinPriority;
    whOperation->getWHartCommand().command = req;

    return true;
}

bool WHEngineOperationsVisitor::visit(NE::Model::Operations::ChangeNotificationOperation& operation)
{
    if (commonData.settings.sendNotificationsOnJoinFlow == true || (operation.getChangeNotification() != 0
                && operation.getChangeNotification() != 20 && operation.getChangeNotification() != 769
                && operation.getChangeNotification() != 832))
    {
        WHOperationPointer whOperation = initWHOperation(operation);
        //todo change size to the actual size of the command
        whOperation->commandData.reset(new uint8_t[256]);

        C839_ChangeNotification_Resp* resp = (C839_ChangeNotification_Resp*) (whOperation->commandData.get());
        memset(resp, 0, sizeof(C839_ChangeNotification_Resp));
        memcpy(resp->DeviceAddress, hart7::util::getUniqueIdFromAddress64(operation.getDeviceAddress()).bytes, 5);

        resp->ChangeNotifications[0] = operation.getChangeNotification();

        whOperation->getWHartCommand().commandID = CMDID_C839_ChangeNotification;
        whOperation->getWHartCommand().command = resp;
    }
    else
    {
        WHOperationPointer whOperation = initWHOperation(operation);
        whOperation->commandData.reset(new uint8_t[sizeof(C64765_NivisMetaCommand_Resp)]);

        C64765_NivisMetaCommand_Resp* resp = (C64765_NivisMetaCommand_Resp*) (whOperation->commandData.get());
        memset(resp, 0, sizeof(C64765_NivisMetaCommand_Resp));
        resp->Nickname = GATEWAY_ADDRESS;
        memcpy(resp->DeviceUniqueId, hart7::util::getUniqueIdFromAddress64(operation.getDeviceAddress()).bytes, 5);

        Address32 address32 = commonData.networkEngine.getAddress32(operation.getDeviceAddress());
        Device& device = commonData.networkEngine.getDevice(address32);

        uint16_t buffSize = 256;
        uint8_t command[buffSize];
        switch (operation.getChangeNotification())
        {
            case CMDID_C000_ReadUniqueIdentifier:
            {
                uint16_t writtenBytes;
                if (commonData.serializeResponse((uint16_t) CMDID_C000_ReadUniqueIdentifier, command, buffSize,
                                                 device.capabilities.readUniqueIdentifier, writtenBytes))
                {
                    resp->CommandSize = writtenBytes;
                    memcpy(resp->Command, command, resp->CommandSize);
                }
                else
                {
                    LOG_ERROR("Can not serialize CMDID_C000_ReadUniqueIdentifier");
                }

                break;
            }
            case CMDID_C020_ReadLongTag:
            {
                C020_ReadLongTag_Resp readLongTag_Resp;
                std::memcpy(readLongTag_Resp.longTag, device.capabilities.longTag, 32);
                uint16_t writtenBytes;
                if (commonData.serializeResponse((uint16_t) CMDID_C020_ReadLongTag, command, buffSize,
                                                 readLongTag_Resp, writtenBytes))
                {
                    resp->CommandSize = writtenBytes;
                    memcpy(resp->Command, command, resp->CommandSize);
                }
                else
                {
                    LOG_ERROR("Can not serialize CMDID_C020_ReadLongTag");
                }

                break;
            }
            case CMDID_C769_ReadJoinStatus:
            {
                C769_ReadJoinStatus_Resp readJoinStatusResp;
                if (device.capabilities.isManager() || device.capabilities.isGateway())
                {
                    readJoinStatusResp.joinStatus = JoinProcessStatusMask_NormalOperationCommencing;
                }
                else
                {
                    readJoinStatusResp.joinStatus
                                = hart7::util::getJoinProcessStatus(device, operation.getDependency());
                }

                // !!! this is a hack used only to inform the GW about the status of a device join flow;
                // only the join status is filled, the rest of the members will be 0;
                readJoinStatusResp.wirelessMode = 0;
                readJoinStatusResp.noOfAvailableNeighbors = 0;
                readJoinStatusResp.noOfAdvertisingPacketsReceived = 0;
                readJoinStatusResp.noOfJoinAttempts = 0;
                readJoinStatusResp.joinRetryTimer.u32 = 0;
                readJoinStatusResp.networkSearchTimer.u32 = 0;

                uint16_t writtenBytes;
                if (commonData.serializeResponse((uint16_t) CMDID_C769_ReadJoinStatus, command, buffSize,
                                                 readJoinStatusResp, writtenBytes))
                {
                    resp->CommandSize = writtenBytes;
                    memcpy(resp->Command, command, resp->CommandSize);
                }
                else
                {
                    LOG_ERROR("Can not serialize CMDID_C769_ReadJoinStatus");
                }

                break;
            }
            case CMDID_C832_ReadNetworkDeviceIdentity:
            {
                C832_ReadNetworkDeviceIdentity_Resp readNetworkDeviceIdentityResp;
                memcpy(readNetworkDeviceIdentityResp.DeviceUniqueID,
                       hart7::util::getUniqueIdFromAddress64(operation.getDeviceAddress()).bytes, 5);
                device.deviceRequested832 = true;
                readNetworkDeviceIdentityResp.Nickname = (uint16_t) address32;
                memcpy(readNetworkDeviceIdentityResp.LongTag, device.capabilities.longTag, 32);

                uint16_t writtenBytes;
                if (commonData.serializeResponse((uint16_t) CMDID_C832_ReadNetworkDeviceIdentity, command, buffSize,
                                                 readNetworkDeviceIdentityResp, writtenBytes))
                {
                    resp->CommandSize = writtenBytes;
                    memcpy(resp->Command, command, resp->CommandSize);
                }
                else
                {
                    LOG_ERROR("Can not serialize CMDID_C832_ReadNetworkDeviceIdentity");
                }

                break;
            }
            default:
            {
                LOG_ERROR("Could not process notification: " << (int) operation.getChangeNotification());
            }
        }

        whOperation->getWHartCommand().commandID = CMDID_C64765_NivisMetaCommand;
        whOperation->getWHartCommand().responseCode = 0;
        whOperation->getWHartCommand().command = resp;
    }

    return true;
}

bool WHEngineOperationsVisitor::visit(NE::Model::Operations::WriteKeyOperation& operation)
{
    WHOperationPointer whOperation = initWHOperation(operation);
    //todo change size to the actual size of the command
    whOperation->commandData.reset(new uint8_t[256]);

    C961_WriteNetworkKey_Req* req = (C961_WriteNetworkKey_Req*) (whOperation->commandData.get());
    memset(req, 0, sizeof(C961_WriteNetworkKey_Req));
    memcpy(req->m_aKeyValue, operation.key.value, 16);

    whOperation->getWHartCommand().commandID = CMDID_C961_WriteNetworkKey;
    whOperation->getWHartCommand().command = req;

    return true;
}

bool WHEngineOperationsVisitor::visit(NE::Model::Operations::WriteNicknameOperation& operation)
{
    WHOperationPointer whOperation = initWHOperation(operation);
    //todo change size to the actual size of the command
    whOperation->commandData.reset(new uint8_t[256]);

    C962_WriteDeviceNicknameAddress_Req* req = (C962_WriteDeviceNicknameAddress_Req*) (whOperation->commandData.get());

    req->m_unNickname = (uint16_t) operation.getOwner();

    whOperation->getWHartCommand().commandID = CMDID_C962_WriteDeviceNicknameAddress;
    whOperation->getWHartCommand().command = req;

    return true;
}

bool WHEngineOperationsVisitor::visit(NE::Model::Operations::WriteSessionOperation& operation)
{
    WHOperationPointer whOperation = initWHOperation(operation);
    //todo change size to the actual size of the command
    whOperation->commandData.reset(new uint8_t[256]);

    C963_WriteSession_Req* req = (C963_WriteSession_Req*) (whOperation->commandData.get());
    memset(req, 0, sizeof(C963_WriteSession_Req));
    memcpy(req->m_aKeyValue, operation.key.value, 16);
    memcpy(req->m_aPeerUniqueID, operation.peerUniqueID.bytes, 5);
    req->m_eSessionType = operation.sessionType;
    req->m_ucReserved = 0;
    req->m_ucTruncated = 0;
    req->m_ulPeerNonceCounterValue = operation.peerNonceCounterValue;
    req->m_unPeerNickname = (uint16_t) operation.peerNickname;

    whOperation->getWHartCommand().commandID = CMDID_C963_WriteSession;
    whOperation->getWHartCommand().command = req;

    return true;
}

bool WHEngineOperationsVisitor::visit(NE::Model::Operations::NeighborHealthReportOperation& operation)
{
    WHOperationPointer whOperation = initWHOperation(operation);
    whOperation->commandData.reset(new uint8_t[sizeof(C64765_NivisMetaCommand_Resp)]);

    C64765_NivisMetaCommand_Resp* resp = (C64765_NivisMetaCommand_Resp*) (whOperation->commandData.get());
    memset(resp, 0, sizeof(C64765_NivisMetaCommand_Resp));
    resp->Nickname = GATEWAY_ADDRESS;
    memcpy(resp->DeviceUniqueId, hart7::util::getUniqueIdFromAddress64(operation.deviceAddress).bytes, 5);

    uint16_t buffSize = 256;
    uint8_t command[buffSize];

    C780_ReportNeighborHealthList_Resp reportNeighborHealthListResp;
    reportNeighborHealthListResp.m_ucNeighborTableIndex = 0;
    reportNeighborHealthListResp.m_ucNoOfNeighborEntriesRead = 0;
    reportNeighborHealthListResp.m_ucTotalNoOfNeighbors = 0;

    uint16_t writtenBytes;
    if (commonData.serializeResponse((uint16_t) CMDID_C780_ReportNeighborHealthList, command, buffSize,
                                     reportNeighborHealthListResp, writtenBytes))
    {
        resp->CommandSize = writtenBytes;// + 4;
        memcpy(resp->Command, command, resp->CommandSize);
    }
    else
    {
        LOG_ERROR("Can not serialize CMDID_C780_ReportNeighborHealthList");
    }

    whOperation->getWHartCommand().commandID = CMDID_C64765_NivisMetaCommand;
    whOperation->getWHartCommand().responseCode = 0;
    whOperation->getWHartCommand().command = resp;

    return true;
}

bool WHEngineOperationsVisitor::visit(NE::Model::Operations::WriteNetworkIDOperation& operation)
{
    WHOperationPointer whOperation = initWHOperation(operation);
    whOperation->commandData.reset(new uint8_t[sizeof(C773_WriteNetworkId_Req)]);

    C773_WriteNetworkId_Req* req = (C773_WriteNetworkId_Req*) (whOperation->commandData.get());
    memset(req, 0, sizeof(C773_WriteNetworkId_Req));
    req->m_unNetworkId = operation.getNetworkID();


    whOperation->getWHartCommand().commandID = CMDID_C773_WriteNetworkId;
    whOperation->getWHartCommand().command = req;
    whOperation->getWHartCommand().responseCode = 0;

    return true;
}

bool WHEngineOperationsVisitor::visit(NE::Model::Operations::ReadWirelessDeviceCapabilitiesOperation& operation) {
    WHOperationPointer whOperation = initWHOperation(operation);
    whOperation->commandData.reset(new uint8_t[sizeof(C777_ReadWirelessDeviceInformation_Req)]);

    C777_ReadWirelessDeviceInformation_Req* req = (C777_ReadWirelessDeviceInformation_Req*) (whOperation->commandData.get());
    memset(req, 0, sizeof(C777_ReadWirelessDeviceInformation_Req));

    whOperation->getWHartCommand().commandID = CMDID_C777_ReadWirelessDeviceInformation;
    whOperation->getWHartCommand().command = req;
    whOperation->getWHartCommand().responseCode = 0;

    return true;
}

bool WHEngineOperationsVisitor::visit(NE::Model::Operations::NivisCustom64765Operation & operation)
{
    WHOperationPointer whOperation = initWHOperation(operation);
    //todo change size to the actual size of the command
    whOperation->commandData.reset(new uint8_t[sizeof(C64765_NivisMetaCommand_Resp)]);

    C64765_NivisMetaCommand_Resp* resp = (C64765_NivisMetaCommand_Resp*) (whOperation->commandData.get());
    memset(resp, 0, sizeof(C64765_NivisMetaCommand_Resp));
    resp->Nickname = operation.Nickname;
    memcpy(resp->DeviceUniqueId, operation.DeviceUniqueId, 5);
    resp->CommandSize = operation.CommandSize;
    memcpy(resp->Command, operation.Command, operation.CommandSize);

    whOperation->getWHartCommand().commandID = CMDID_C64765_NivisMetaCommand;
    whOperation->getWHartCommand().command = resp;
    whOperation->getWHartCommand().responseCode = 0;

    return true;
}

