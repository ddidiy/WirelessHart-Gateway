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
 * JoinFlowHandler.cpp
 *
 *  Created on: May 25, 2009
 *      Author: andrei.petrut
 */

#include "JoinFlowHandler.h"
#include "../AllNetworkManagerCommands.h"
#include "../DevicesManager.h"
#include "../../util/ManagerUtils.h"
#include "../operations/WHOperation.h"
#include "../operations/WHEngineOperationsVisitor.h"
#include "Model/Operations/Join/ChangePriorityEngineOperation.h"
#include "Misc/Convert/Convert.h"
#include "SMState/SMStateLog.h"
#include "../../util/NMLog.h"

#include "UnboundResolveOperations.h"

#include <boost/bind.hpp>

namespace hart7 {
namespace nmanager {

JoinFlowHandler::JoinFlowHandler(DevicesManager& devicesManager_, IRequestSend& requestSend_, CommonData& commonData_,
                                 operations::WHOperationQueue& operationsQueue_) :
    FlowHandler(requestSend_, commonData_, operationsQueue_), devicesManager(devicesManager_)
{
    haveError = false;
    stopRequested = false;
    isActive = true;
}

JoinFlowHandler::~JoinFlowHandler()
{
}

void JoinFlowHandler::StopFlow()
{
    LOG_DEBUG("Stopping JoinFlowHandler, currentState=" << (int) currentState);
    haveError = true;
    stopRequested = true;
    if (currentState == KeyNickSessionSent)
    {
        operationsQueue.CancelOperations(keyNickSessionHandle);
    }
    else if (currentState == ExistingDeviceRemoved)
    {
    }
    else if (currentState == OperationsSending)
    {
        operationsQueue.CancelOperations(joinOperationsEvent);
    }

    isActive = false;
}

void JoinFlowHandler::ProcessSuccessGWConfirm()
{
    LOG_DEBUG("Confirm received with success");

    if (currentState == KeyNickSessionSent)
    {
        if (haveError)
        {
            OnJoinFinished(false);
            return;
        }

        // the operations from the first state (KeyNickSessionSent) are only into NM => so we have to log them here
        SMState::SMStateLog::logAllInfo(keyNickSessionOperations->reasonOfOperations);

        // for GW rejoin
        RemoveExistingDevice();

        commonData.stack.initGWService();
        currentState = OperationsSending;
        JoinDevice();
    }
    else if (currentState == OperationsSending)
    {
        currentState = OperationsResolved;
        ResolveOperations(joinOperations, joinOperations, true);
        if (!haveError)
        {
            commonData.networkEngine.confirmDeviceQuarantine(joinedDeviceAddress32);
        }

        OnJoinFinished(!haveError);
    }
    else
    {
        LOG_WARN("Unexpected confirm.");
    }
}

void JoinFlowHandler::ProcessSuccessDeviceConfirm()
{
    if (currentState == KeyNickSessionSent)
    {
        if (haveError || stopRequested)
        {
            OnJoinFinished(false);
            return;
        }

        SMState::SMStateLog::logAllInfo(keyNickSessionOperations->reasonOfOperations);
        SMState::SMStateLog::logOperations(keyNickSessionOperations->reasonOfOperations, *keyNickSessionOperations);

        currentState = OperationsSending;
        JoinDevice();
    }
    else if (currentState == OperationsSending)
    {
        if (haveError || stopRequested)
        {
            operationsQueue.CancelOperations(joinOperationsEvent);  // just in case other operations will be called later.
            return;
        }

        currentState = OperationsResolved;
        if (!haveError || !stopRequested)
        {
            commonData.networkEngine.confirmDeviceQuarantine(joinedDeviceAddress32);
        }
        OnJoinFinished(!haveError && !stopRequested);
    }
    else
    {
        LOG_WARN("Unexpected confirm.");
    }
}

void JoinFlowHandler::ProcessConfirmedOperations(bool errorOccured)
{
    if (errorOccured)
    {
        haveError = true;
    }

    { // separated from here to make join flows more readable
        if (device.isGateway)
        {
            ProcessSuccessGWConfirm();
        }
        else
        {
            ProcessSuccessDeviceConfirm();
        }
    }

}

void JoinFlowHandler::OnJoinFinished(bool status)
{
    static long topId = 0;
    if (status)
    {
        LOG_INFO("Device " << joiningDevice << " has successfully joined.");
        std::ostringstream strfilename;
        strfilename << "top_";
        strfilename << std::setw(10) << std::setfill('0') << ++topId;
        hart7::util::writeTopologyToFile(commonData.networkEngine.getSubnetTopology(), strfilename.str());
    }
    else
    {
        LOG_INFO("Device " << joiningDevice << " has failed to join.");
    }

    isActive = false;

    if (JoinFinished)
    {
        JoinFinished(device, status);
    }
}

bool JoinFlowHandler::RemoveExistingDevice()
{
    try
    {
        if (commonData.networkEngine.existsDevice(longAddressDevice))
        {
            commonData.networkEngine.setRemoveStatus(joinedDeviceAddress32);
            CheckRemoveStatus(false);
        }
        else
        {
            LOG_DEBUG("Finished removing existing devices.");
        }

        return true;
    }
    catch (std::exception& ex)
    {
        LOG_INFO("Join failed for device " << longAddressDevice.toString() << ". Reason='" << ex.what() << "'");
        OnJoinFinished(false);

        return false;
    }
}

void JoinFlowHandler::ProcessJoinRequest(const stack::WHartAddress& src, const stack::WHartCommandList& commands, bool overrideMaxChildren)
{
    currentState = Initial;
    joiningDevice = src;

    device.longAddress = src.address.uniqueID;

    if (src == Gateway_Nickname() || src == Gateway_UniqueID())
    {
        device.isGateway = true;
        device.isBackbone = false;
    }

    C000_ReadUniqueIdentifier_Resp* readUniqueId = (C000_ReadUniqueIdentifier_Resp*) commands.list[0].command;
    C020_ReadLongTag_Resp* longTag = (C020_ReadLongTag_Resp*) commands.list[1].command;
    C787_ReportNeighborSignalLevels_Resp* neighbors = (C787_ReportNeighborSignalLevels_Resp*) commands.list[2].command;

    uint8_t joinReason[4];
    memcpy(&joinReason[0], &readUniqueId->manufacturerIDCode, 2);
#ifdef IS_MACHINE_LITTLE_ENDIAN
    uint8_t tmp1 = joinReason[0];
    joinReason[0] = joinReason[1];
    joinReason[1] = tmp1;
#endif

    memcpy(&joinReason[2], &readUniqueId->privateLabelDistributorCode, 2);
#ifdef IS_MACHINE_LITTLE_ENDIAN
    uint8_t tmp2 = joinReason[2];
    joinReason[2] = joinReason[3];
    joinReason[3] = tmp2;
#endif

    LOG_INFO("Device addr=" << src << " join attempt Reason=" << std::hex << std::setw(2) << std::setfill('0') <<
             (int)joinReason[0] << " " << std::setw(2) << (int)joinReason[1] << " " << std::setw(2) << (int)joinReason[2]
             << " " << std::setw(2)<< (int)joinReason[3]);


    hart7::util::NMLog::logCommandResponse(commands.list[0].commandID, 0, commands.list[0].command, src);
    hart7::util::NMLog::logCommandResponse(commands.list[1].commandID, 0, commands.list[1].command, src);
    hart7::util::NMLog::logCommandResponse(commands.list[2].commandID, 0, commands.list[2].command, src);

    deviceCapabilities = hart7::util::getCapabilities(src, readUniqueId, longTag, neighbors, 0, commonData.settings,
                                                      commonData.networkEngine.getDevicesTable());

    longAddressDevice = deviceCapabilities.euidAddress;
    parentAddress32 = device.isGateway ? NetworkManager_Nickname() : neighbors->m_aNeighbors[0].nickname;

#ifdef JOIN_REASON
    commonData.networkEngine.getDevicesTable().addDeviceJoinReason(longAddressDevice, joinReason);
#endif

    if (deviceCapabilities.deviceType == DeviceType::BACKBONE)
    {
        device.isBackbone = true;
        commonData.securityManager.ChangeNetworkKey();
    }

    uint16_t nick = (uint16_t) commonData.networkEngine.createAddress32(longAddressDevice);
    commonData.nodeVisibleVerifier.removeVisibleNode(nick);

    LOG_DEBUG("ProcessJoinRequest() for device " << longAddressDevice.toString() << " to parent : "
                << ToStr(parentAddress32));

    if (!NE::Model::NetworkEngine::instance().existsDevice(parentAddress32))
    {
        LOG_DEBUG("Cannot join device " << longAddressDevice.toString() << " on parent " << parentAddress32 << " because parent does not exist or is deleted.");

        OnJoinFinished(false);
        return;
    }

    try
    {
        joinedDeviceAddress32 = commonData.networkEngine.createAddress32(longAddressDevice);
        commonData.networkEngine.setDeviceSessionKey(joinedDeviceAddress32, false);
        device.nickname = joinedDeviceAddress32 & 0xFFFF;

        LOG_INFO("Join Device Reason: addr=" << ToStr(device.nickname) << ", reason=" <<
                    std::setfill('0') << std::hex << std::setw(2) << (int)joinReason[0] << " " << std::setw(2) << (int)joinReason[1] << " "
                    << std::setw(2) << (int)joinReason[2] << " " << std::setw(2) << (int)joinReason[3]);
        // deletes the sequence number as slave
        commonData.stack.transport.RemoveTableEntry(stack::WHartAddress(device.nickname), false);
    }
    catch (std::exception& ex)
    {
        LOG_INFO("Join failed for device " << longAddressDevice.toString() << ". Reason='" << ex.what() << "'");
        OnJoinFinished(false);
        return;
    }

    if (!RemoveExistingDevice())
        return;

    if (!deviceCapabilities.isBackbone() && commonData.networkEngine.getDevicesTable().existsDevice(parentAddress32))
    {
        Device& parent = commonData.networkEngine.getDevicesTable().getDevice(parentAddress32);

        std::set<Address32> dummySet;

        // check the resources on parent
        NE::Model::MetaDataAttributesPointer metaDataAttributes;
        metaDataAttributes
                    = parent.getMetaDataAttributes();
        uint
                    parentJpInProgress =
                                metaDataAttributes->getJoinInProgressPriority(
                                                                              devicesManager.getJoinsInProgressNo(
                                                                                                                  joinedDeviceAddress32,
                                                                                                                  parentAddress32, dummySet));
        if (parentJpInProgress >= commonData.settings.maxJoinPriorityInProgressOnParent)
        {
            LOG_WARN("Join refused for device " << longAddressDevice.toString() << ". Not enough resources on parent : "
                        << ToStr(parentAddress32) << ", parent join priority in progress is : " << (int) parentJpInProgress);

            OnJoinFinished(false);

            return;
        }


        uint8_t maxJoinPriority = commonData.networkEngine.getMaxJoinPriority(parentAddress32);
        if (maxJoinPriority >= commonData.settings.maxJoinPriorityInProgressOnParent)
        {
            LOG_WARN("Join refused for device " << longAddressDevice.toString() << ". Not enough resources on parent or children: "
                        << ToStr(parentAddress32) << ", parent max join priority in progress is : " << (int) maxJoinPriority);

            OnJoinFinished(false);

            return;
        }


        // check the number of join IN PROGRESS on parent
        if (!commonData.networkEngine.getDevicesTable().getDevice(parentAddress32).capabilities.isBackbone())
        {
            uint16_t maxJoinsInProgress = commonData.settings.maxJoinsInProgressPerParent;
            if (devicesManager.getJoinsInProgressNo(joinedDeviceAddress32, parentAddress32, dummySet) > maxJoinsInProgress)
            {
                LOG_WARN("Join refused for device " << longAddressDevice.toString() << ". Too many devices in join on parent : "
                            << ToStr(parentAddress32));
                //            RemoveExistingDevice();
                OnJoinFinished(false);

                return;
            }
        }

        // check the number of TOTAL joins on parent (done and in progress)
        uint16_t maxJoins = commonData.settings.maxJoinsPerDevice;
        if (commonData.networkEngine.getDevicesTable().getDevice(parentAddress32).capabilities.isBackbone())
        {
            maxJoins = commonData.settings.maxJoinsPerAP;
        }

        std::set<Address32> addresses;

        int children = devicesManager.getJoinsInProgressNo(joinedDeviceAddress32, parentAddress32, addresses)
                    + commonData.networkEngine.getDevicesTable().getNumberOfJoinInProgress(parentAddress32, addresses)
                    + commonData.networkEngine.getDevicesTable().getNumberOfChildren(parentAddress32, addresses);

        LOG_DEBUG("Children count for join on parent=" << ToStr(parentAddress32) << " is " << children);

        if (addresses.size() >= maxJoins)
        {
            bool canHaveMoreChildren = true;

            LOG_WARN("Join refused for device " << longAddressDevice.toString() << ". Too many devices joined and in join ("
                        << (int) maxJoins << ") on parent : " << ToStr(parentAddress32) << ", canHaveChildren=" << (canHaveMoreChildren ? "true" : "false"));
            //            RemoveExistingDevice();
            OnJoinFinished(false);

            return;

        }
    }

    joinEuidAddress = deviceCapabilities.euidAddress;

    NE::Model::SecurityKey key = commonData.securityManager.GetSessionKey(deviceCapabilities.euidAddress);
    NE::Model::SecurityKey networkKey = commonData.securityManager.GetNetworkKey();

    // send with proxy the first neighbor
    proxyToDevice = device.isGateway ? stack::WHartAddress(Gateway_Nickname())
                : stack::WHartAddress(neighbors->m_aNeighbors[0].nickname, src.address.uniqueID);
    currentState = KeyNickSessionSent;
    keyNickSessionHandle = WriteKeyNickSession(deviceCapabilities, joinedDeviceAddress32, src.address.uniqueID,
                                               parentAddress32, networkKey, key);
}

void JoinFlowHandler::JoinDevice()
{
    try
    {
        std::set<Address32> dummySet;

        commonData.networkEngine.setDeviceSessionKey(joinedDeviceAddress32, true);
        joinOperations.reset(new EngineOperations());
        commonData.networkEngine.joinDevice(*joinOperations, deviceCapabilities, joinedDeviceAddress32,
                                            parentAddress32, devicesManager.getJoinsInProgressNo(joinedDeviceAddress32,
                                                                                                 parentAddress32, dummySet));


        NE::Model::Device& nmDevice = commonData.networkEngine.getDevice(joinedDeviceAddress32);
        NE::Model::MetaDataAttributes metaData = util::getMetadataAttributes(device);
        nmDevice.setMetaDataAttributes(metaData);


        hart7::nmanager::operations::OperationDependencyType::OperationDependency operDep = hart7::nmanager::operations::OperationDependencyType::None;
        if (commonData.settings.deviceJoinAfterAllOperationsConfirms == true) {
            operDep = hart7::nmanager::operations::OperationDependencyType::DependOnCheckRemoved;
        }

        SendOperations(joinOperations, joinOperationsEvent, true, operDep);

        LOG_DEBUG("Added operations.");
    }
    catch (std::exception& ex)
    {
        LOG_INFO("Join failed for device " << longAddressDevice.toString() << ". Reason='" << ex.what() << "'");
        OnJoinFinished(false);
    }
}

uint32_t JoinFlowHandler::WriteKeyNickSession(NE::Model::Capabilities& capabilities, Address32 address32,
                                              WHartUniqueID uniqueID, Address32 parentAddress32, const NE::Model::SecurityKey& networkKey,
                                              const NE::Model::SecurityKey& key)
{
    C963_WriteSession_Req writeSessionLocal;
    { // write the session locally
        writeSessionLocal.m_ulPeerNonceCounterValue = 0;
        writeSessionLocal.m_unPeerNickname = (address32 & 0xFFFF);
        memcpy(writeSessionLocal.m_aPeerUniqueID, uniqueID.bytes, 5); //TODO check if it is ok
        memcpy(writeSessionLocal.m_aKeyValue, key.value, key.LENGTH);
        writeSessionLocal.m_eSessionType = WHartSessionKey::sessionKeyed;
        writeSessionLocal.m_ucReserved = 0;
        memset(writeSessionLocal.m_tExecutionTime, 0, 5);
    }

    C2009_NM_WriteUniqueIdNickname_Req localAddressNick;
    {
        localAddressNick.nickname = (uint16_t) address32 & 0xFFFF;
        localAddressNick.uniqueId = uniqueID;
        localAddressNick.isTR = device.isBackbone;
    }

    WHartCommand arrayLocal[] = { { CMDID_C963_WriteSession, 0, &writeSessionLocal }, {
            CMDID_C2009_NM_WriteUniqueIdNickname, 0, &localAddressNick } };
    WHartCommandList listReqLocal = { sizeof(arrayLocal) / sizeof(arrayLocal[0]), arrayLocal };

    //COULD trigger TransmitConfirm if there is a package in the stack for the peer address.
    TransmitRequest(NetworkManager_Nickname(), listReqLocal, WHartSessionKey::joinKeyed);

    try
    {
        keyNickSessionOperations.reset(new NE::Model::Operations::EngineOperations());
        commonData.networkEngine.initJoinDevice(*keyNickSessionOperations, capabilities, address32, uniqueID,
                                                parentAddress32, networkKey, key);


        std::ostringstream stream;
        stream << "Write session " << ToStr(address32) << ", parent=" << ToStr(parentAddress32);
        keyNickSessionOperations->reasonOfOperations = stream.str();

        SMState::SMStateLog::logOperations(keyNickSessionOperations->reasonOfOperations, *keyNickSessionOperations);

        SendOperations(keyNickSessionOperations, keyNickSessionHandle, true);
    }
    catch (std::exception& ex)
    {
        LOG_ERROR("Error:" << ex.what());
    }

    return keyNickSessionHandle;
}

WHartHandle JoinFlowHandler::TransmitRequest(const WHartAddress& to, const WHartCommandList& commands,
                                             WHartSessionKey::SessionKeyCode sessionCode)
{
    try
    {
        NE::Model::Services::Service& service = commonData.utils.GetServiceTo(to);
        return requestSend.TransmitRequest(to, stack::whartpCommand, (stack::WHartServiceID) service.getServiceId(),
                                           wharttRequestUnicast, commands, sessionCode, *this);
    }
    catch (...)
    {
        LOG_WARN("Could not find service to destination=" << to);
        return (WHartHandle) 0xFFFF;
    }
}

}
}
