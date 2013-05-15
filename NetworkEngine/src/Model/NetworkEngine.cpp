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

/**
 * NetworkEngine.cpp
 *
 */

#include "NetworkEngine.h"
#include "Model/Operations/ResolveOperationsVisitor.h"
#include "SMState/SMStateLog.h"
#include "Model/Operations/Join/WriteKeyOperation.h"
#include "Model/Operations/Join/WriteNicknameOperation.h"
#include "Model/Operations/Join/WriteSessionOperation.h"
#include "Model/Operations/Join/ReadWirelessDeviceCapabilitiesOperation.h"
#include <ApplicationLayer/Model/CommonTables.h>

using namespace NE::Model::Operations;
using namespace NE::Model::Topology;
using namespace NE::Model;

NetworkEngine::NetworkEngine() : discoveryCount(0), evalCount(0){

}

NetworkEngine::~NetworkEngine() {
    LOG_DEBUG("NetworkEngine destroyed");
    startTime = time(NULL);
}

NetworkEngine& NetworkEngine::instance() {
    static NetworkEngine instance;
    return instance;
}

Uint32 NetworkEngine::getStartTime() {
    return startTime;
}

SettingsLogic& NetworkEngine::getSettingsLogic() {
    return settingsLogic;
}

void NetworkEngine::setSettingsLogic(const NE::Common::SettingsLogic& _settingsLogic) {

    try {
        settingsLogic = _settingsLogic;

        subnetId = settingsLogic.networkId;

        devicesTable.LoadDeviceNicknames (settingsLogic);

        devicesTable.addManager(settingsLogic.networkId, settingsLogic.managerAddress64,
                    settingsLogic.networkManagerTag);
        Device& managerDevice = devicesTable.getDevice(MANAGER_ADDRESS);

        managerDevice.setAction(DeviceAction::ROUTER_SUCCESS);

        subnetTopology.init(settingsLogic.networkId);
        subnetServices.init();
        subnetTdma.init(MANAGER_ADDRESS);
        subnetTdma.setChannelMap(settingsLogic.channelMap);

        Node& managerNode = subnetTopology.getNode(MANAGER_ADDRESS);
        managerNode.setMetaDataAttributes(managerDevice.getMetaDataAttributes());

        SMState::SMStateLog::logAllInfo("NetworkEngine (re)started.");
    } catch (NEException& e) {
        LOG_ERROR("setSettingsLogic:" << e.what());
        throw e;
    }
}

NE::Model::Services::SubnetServices& NetworkEngine::getSubnetServices() {
    return subnetServices;
}

NE::Model::DevicesTable& NetworkEngine::getDevicesTable() {
    return devicesTable;
}

SubnetTdma& NetworkEngine::getSubnetTdma() {
    return subnetTdma;
}

SubnetTopology& NetworkEngine::getSubnetTopology() {
    return subnetTopology;
}

Address32 NetworkEngine::createAddress32(Address64 address64) {
    return devicesTable.createAddress32(address64);
}

Device& NetworkEngine::getDevice(Address32 address32) {
    return devicesTable.getDevice(address32);
}

bool NetworkEngine::existsDevice(Address32 address32) {
    return devicesTable.existsDevice(address32);
}

bool NetworkEngine::existsDevice(const Address64& address64) {
    return devicesTable.existsDevice(address64);
}

Address64 NetworkEngine::getAddress64(Address32 address32) {
    return devicesTable.getAddress64(address32);
}

Address32 NetworkEngine::getAddress32(const Address64& address64) {
    return devicesTable.getAddress32(address64);
}

Uint16 NetworkEngine::getSubnetId() {
    return subnetId;
}

void NetworkEngine::checkParentConstraints(Model::Topology::SubnetTopology& subnetTopology, Address32 deviceAddress,
            Address32 parentAddress, Capabilities& capabilities) {

    if (subnetId != capabilities.dllSubnetId) {
        const Address64& address64 = capabilities.euidAddress;
        std::ostringstream stream;
        stream << "Illegal join for device " << address64.toString() << " to parent " << ToStr(parentAddress)
                    << ". The subnetId of joining device doesn't match to provisioned subnetId! ("
                    << (int) capabilities.dllSubnetId << " != " << (int) subnetId << ").";
        LOG_ERROR(stream.str());
        throw NEException(stream.str());
    }

    if (!devicesTable.existsConfirmedDevice(parentAddress)) {
        const Address64& address64 = capabilities.euidAddress;
        std::ostringstream stream;
        stream << "Illegal join for device " << address64.toString() << " to parent " << ToStr(parentAddress)
                    << ". Parent not joined yet.";
        LOG_ERROR(stream.str());
        throw NEException(stream.str());
    }
}

bool NetworkEngine::getDeviceSessionKey(Address32 address) {
    return devicesWithSessionKeys.find(address) != devicesWithSessionKeys.end();
}

void NetworkEngine::setDeviceSessionKey(Address32 address, bool hasKey) {
    if (hasKey) {
        devicesWithSessionKeys.insert(address);
    } else {
        devicesWithSessionKeys.erase(address);
    }
}

bool NetworkEngine::initJoinDevice(NE::Model::Operations::EngineOperations& operationsList,
            NE::Model::Capabilities& capabilities, Address32 address32, WHartUniqueID uniqueID,
            Address32 parentAddress32, const SecurityKey& networkKey, const SecurityKey& key) {
    LOG_INFO("InitJoinDevice " << capabilities.euidAddress.toString() << ", subnetId="
                << (int) capabilities.dllSubnetId << ", parent=" << ToStr(parentAddress32) << ", role="
                << capabilities.getRoleAsString());

    if (address32 == GATEWAY_ADDRESS) {
        operationsList.setRequesterAddress32(address32);
        operationsList.setNetworkEngineEventType(NetworkEngineEventType::NONE);
        operationsList.setRequesterAddress64(capabilities.euidAddress);
    } else {
        operationsList.setProxyAddress32(parentAddress32, false);
        operationsList.setRequesterAddress32(address32);
        operationsList.setNetworkEngineEventType(NetworkEngineEventType::NONE);
        operationsList.setRequesterAddress64(capabilities.euidAddress);
    }

    WriteKeyOperation *keyOp = new WriteKeyOperation(address32, networkKey);
    WriteNicknameOperation *nickOp = new WriteNicknameOperation(address32);
    WriteSessionOperation *sessionOp = new WriteSessionOperation(address32, 0, MANAGER_ADDRESS,
                hart7::stack::NetworkManager_UniqueID(), key, hart7::stack::WHartSessionKey::sessionKeyed);

    WHartUniqueID deviceUID;
    memcpy(deviceUID.bytes, capabilities.euidAddress.value + 3, 5);

    NE::Model::Operations::IEngineOperationPointer keyOpPtr(keyOp);
    NE::Model::Operations::IEngineOperationPointer nickOpPtr(nickOp);
    NE::Model::Operations::IEngineOperationPointer sessionOpPtr(sessionOp);

    keyOpPtr->setDependency(WaveDependency::FIRST);
    nickOpPtr->setDependency(WaveDependency::FIRST);
    sessionOpPtr->setDependency(WaveDependency::FIRST);

    operationsList.addOperation(keyOpPtr);
    operationsList.addOperation(nickOpPtr);
    operationsList.addOperation(sessionOpPtr);

    return true;
}

bool NetworkEngine::joinDevice(EngineOperations& engineOperations, Capabilities& capabilities, Address32 address,
            Address32 parentAddress, Uint16 parentJoinsInProgressNo) {

    LOG_INFO("JoinDevice=" << capabilities.euidAddress.toString() << ", subnetId=" << (int) capabilities.dllSubnetId
                << ", parent=" << ToStr(parentAddress) << ", role=" << capabilities.getRoleAsString());

    checkParentConstraints(subnetTopology, address, parentAddress, capabilities);

    try {
        engineOperations.setProxyAddress32(parentAddress, settingsLogic.enableShortProxy);
        engineOperations.setRequesterAddress32(address);
        engineOperations.setNetworkEngineEventType(NetworkEngineEventType::JOIN_REQUEST);
        engineOperations.setRequesterAddress64(capabilities.euidAddress);

        if (address != GATEWAY_ADDRESS && devicesTable.existsDevice(address)) {
            LOG_ERROR(
                        "joinDevice - the device should be deleted before and the previous allocated resources should be deleted");
            return false;
        }

        Device& device = devicesTable.addDevice(capabilities, address, parentAddress);
        device.setAction(DeviceAction::JOIN_REQUEST);
        device.setOnEvaluation(false);
        device.getMetaDataAttributes()->resetAttributes();

        LOG_DEBUG("new device:" << device);

        NodeType::NodeTypeEnum nodeType;

        if (capabilities.isGateway()) {
            nodeType = NodeType::GATEWAY;
        } else if (capabilities.isBackbone()) {
            nodeType = NodeType::BACKBONE;
        } else {
            nodeType = NodeType::NODE;
        }

        Uint16 inboundGraphId;
        Uint16 outboundGraphId;

        //period is of time data type; the time is 1/32 from a millisecond
        Uint32 managementPeriod = 32 * 1000 * settingsLogic.managementBandwidth;

        //on graph allocate bandwidth for both NM and GW
        PublishPeriod::PublishPeriodEnum managementPublishPeriod =
                    (PublishPeriod::PublishPeriodEnum) ((Uint32) PublishPeriod::P_1_S
                                / settingsLogic.managementBandwidth);

        PublishPeriod::PublishPeriodEnum gatewayPublishPeriod =
                    (PublishPeriod::PublishPeriodEnum) ((Uint32) PublishPeriod::P_1_S / settingsLogic.gatewayBandwidth);

        PublishPeriod::PublishPeriodEnum joinPublishPeriod =
                    (PublishPeriod::PublishPeriodEnum) ((Uint32) PublishPeriod::P_1_S / settingsLogic.joinBandwidth);

        Uint16 proxyGraphId = getSuperframeId(joinPublishPeriod);

        subnetTopology.addNode(engineOperations, address, parentAddress, nodeType, (Uint16) (managementPublishPeriod
                    + gatewayPublishPeriod), proxyGraphId, inboundGraphId, outboundGraphId);

        Uint32 inboundServiceId;
        Uint32 outboundServiceId;

        //allocate attach bandwidth for NM only
        subnetServices.addNode(engineOperations, parentAddress, address, nodeType, inboundGraphId, outboundGraphId,
                    managementPeriod, inboundServiceId, outboundServiceId);

        if (address == GATEWAY_ADDRESS) {
            {
                IEngineOperationPointer operationF(new ChangeNotificationOperation((uint32_t) GATEWAY_ADDRESS,
                            settingsLogic.managerAddress64, 0));
                operationF->setDependency(WaveDependency::TWELFTH);
                engineOperations.addOperation(operationF);
            }
            {
                IEngineOperationPointer operationF(new ChangeNotificationOperation((uint32_t) GATEWAY_ADDRESS,
                            settingsLogic.managerAddress64, 832));
                operationF->setDependency(WaveDependency::ELEVENTH);
                engineOperations.addOperation(operationF);
            }
        }

        {
            IEngineOperationPointer operationF(new ChangeNotificationOperation((uint32_t) GATEWAY_ADDRESS,
                        devicesTable.getAddress64(address), 0));
            operationF->setDependency((address == GATEWAY_ADDRESS) ? WaveDependency::TWELFTH : WaveDependency::SECOND);
            engineOperations.addOperation(operationF);
        }
        {
            IEngineOperationPointer operationF(new ChangeNotificationOperation((uint32_t) GATEWAY_ADDRESS,
                        devicesTable.getAddress64(address), 769));
            operationF->setDependency((address == GATEWAY_ADDRESS) ? WaveDependency::TWELFTH : WaveDependency::FIRST);
            engineOperations.addOperation(operationF);
        }
        {
            IEngineOperationPointer operationF(new ChangeNotificationOperation((uint32_t) GATEWAY_ADDRESS,
                        devicesTable.getAddress64(address), 832));
            operationF->setDependency((address == GATEWAY_ADDRESS) ? WaveDependency::ELEVENTH : WaveDependency::FIRST);
            engineOperations.addOperation(operationF);
        }

        if (address != GATEWAY_ADDRESS) {
            // notify GW about the device has nick, session and key (JoinProcessStatusMask_Authenticated)
            IEngineOperationPointer operationAuth(new ChangeNotificationOperation((uint32_t) GATEWAY_ADDRESS,
                        devicesTable.getAddress64(address), 769));
            operationAuth->setDependency(WaveDependency::SECOND);
            engineOperations.addOperation(operationAuth);

            // notify GW about the device has normal superframes and links (JoinProcessStatusMask_NetworkJoined)
            IEngineOperationPointer operation(new ChangeNotificationOperation((uint32_t) GATEWAY_ADDRESS,
                        devicesTable.getAddress64(address), 769));
            operation->setDependency(WaveDependency::FOURTH);
            engineOperations.addOperation(operation);

        }

        if (!capabilities.isGateway()) {
            subnetTdma.addNode(engineOperations, inboundServiceId, outboundServiceId, parentAddress, address, nodeType,
                        managementPublishPeriod, joinPublishPeriod);
        } else {
            // the device is joined
            IEngineOperationPointer operation(new ChangeNotificationOperation((uint32_t) GATEWAY_ADDRESS,
                        devicesTable.getAddress64(MANAGER_ADDRESS), 769));
            operation->setDependency(WaveDependency::TWELFTH);
            engineOperations.addOperation(operation);
        }
        //      }

        //For TR, set reports period
        if ((nodeType == NodeType::BACKBONE)) {
            generateHealthReportPeriod(engineOperations, address);
        }

        if (nodeType == NodeType::NODE) {
            NE::Model::Operations::NeighborHealthReportOperation * operation =
                        new NE::Model::Operations::NeighborHealthReportOperation(GATEWAY_ADDRESS,
                                    capabilities.euidAddress);

            NE::Model::Operations::IEngineOperationPointer op(operation);
            op->setDependency(WaveDependency::TWELFTH);
            engineOperations.addOperation(op);
        }

        Node& node = subnetTopology.getNode(address);
        node.setMetaDataAttributes(device.getMetaDataAttributes());

        //test
        if (nodeType == NodeType::NODE) {
            engineOperations.setJoinDependencies(address);
        } else if (nodeType == NodeType::BACKBONE) {
            IEngineOperationPointer operation(new ChangePriorityEngineOperation(address, 10));
            operation->setDependency(WaveDependency::TWELFTH);
            engineOperations.addOperation(operation);
        }
        //end test

        // parentAddress32 - just accepted a join request from a device and in order to fulfill this request it'll allocate
        // some of its resources. During this join process the parentAddress32 could accept other join requests. In order to
        // avoid the situation in which a very big number of devices joins through a single device we have to update the join priority
        // of a the device in one of the first operations sent to the parentAddress32.
        Device& parentDevice = getDevicesTable().getDevice(parentAddress);
        if (parentAddress != Gateway_Nickname() && parentAddress != NetworkManager_Nickname()) {
            uint8_t parentJp = parentDevice.getMetaDataAttributes()->getJoinInProgressPriority(parentJoinsInProgressNo
                        + 1);
            IEngineOperationPointer operation(new ChangePriorityEngineOperation(parentAddress, parentJp));
            operation->setDependency(WaveDependency::FIRST);
            engineOperations.addOperation(operation);
        }

        if (nodeType == NodeType::BACKBONE) {
            IEngineOperationPointer operation(new ReadWirelessDeviceCapabilitiesOperation(address));
            operation->setDependency(WaveDependency::ELEVENTH);
            engineOperations.addOperation(operation);
        }


        std::ostringstream reason;
        reason << " DeviceJoin: " << device << ", Parent=";
        reason << ToStr(parentAddress) << ", role=" << capabilities.getRoleAsString();
        engineOperations.reasonOfOperations = reason.str();

        SMState::SMStateLog::logOperations(reason.str(), engineOperations);
        SMState::SMStateLog::logAllInfo(reason.str());

    } catch (NEException& e) {
        LOG_ERROR(e.what());
        throw e;
    }

    return true;
}

void NetworkEngine::removeDefaultRoute(NE::Model::Operations::EngineOperations& operations, Address32 deviceAddress) {
    static Uint32 defaultRouteId = 0;

    Node& node = subnetTopology.getNode(deviceAddress);
    if (node.getNodeType() != NodeType::NODE) {
        return;
    }

    NE::Model::Operations::RouteRemovedOperation *routeRemove = new NE::Model::Operations::RouteRemovedOperation(
                deviceAddress, defaultRouteId, 0);
    routeRemove->setDependency(WaveDependency::NINETH);

    NE::Model::Operations::IEngineOperationPointer op(routeRemove);
    operations.addOperation(op);
}

void NetworkEngine::generateHealthReportPeriod(NE::Model::Operations::EngineOperations& operations,
            Address32 deviceAddress) {

    bool isAp = subnetTopology.getNode(deviceAddress).getNodeType() == NodeType::BACKBONE;

    {
        Uint32 reportingPeriod = settingsLogic.healthReportPeriod * 60 * 1000; // convert to msec

        if (isAp) {
            reportingPeriod = 60 * 1000; // convert to msec
        }

        NE::Model::Operations::WriteTimerIntervalOperation * operation =
                    new NE::Model::Operations::WriteTimerIntervalOperation(deviceAddress, reportingPeriod,
                                WirelessTimerCode_HealthReport);

        NE::Model::Operations::IEngineOperationPointer op(operation);

        op->setDependency(WaveDependency::FIRST);

        operations.addOperation(op);
    }

    {
        Uint32 keepAlivePeriod = settingsLogic.keepAlivePeriod * 1000; // convert to msecs

        NE::Model::Operations::WriteTimerIntervalOperation * op2 =
                    new NE::Model::Operations::WriteTimerIntervalOperation(deviceAddress, keepAlivePeriod,
                                WirelessTimerCode_Keep_Alive);
        NE::Model::Operations::IEngineOperationPointer op2ptr(op2);
        op2ptr->setDependency(WaveDependency::FIRST);

        operations.addOperation(op2ptr);
    }

    {
        Uint32 discoveryAlivePeriod = settingsLogic.discoveryReportPeriod * 60 * 1000; // convert to msecs

        if (isAp) {
            discoveryAlivePeriod = 60 * 1000; // convert to msec
        }

        NE::Model::Operations::WriteTimerIntervalOperation * op3 =
                    new NE::Model::Operations::WriteTimerIntervalOperation(deviceAddress, discoveryAlivePeriod,
                                WirelessTimerCode_Discovery);
        NE::Model::Operations::IEngineOperationPointer op3ptr(op3);
        op3ptr->setDependency(WaveDependency::FIRST);

        operations.addOperation(op3ptr);
    }
}

Service& NetworkEngine::findManagementService(const Address32& destination, bool isProxyDestination) {
    return subnetServices.findManagementService(destination, isProxyDestination);
}

bool NetworkEngine::createService(Service& serviceRequest, bool sendToRequester) {
    LOG_DEBUG("createService - request" << serviceRequest);

    bool serviceCreated = false;

    Address32 address = serviceRequest.getAddress();

    if (address == MANAGER_ADDRESS || address == GATEWAY_ADDRESS) {
        address = serviceRequest.getPeerAddress();
    }
    serviceRequest.serviceId = serviceRequest.requestServiceId;

    Node& node = subnetTopology.getNode(address);

    if (serviceRequest.source) {

        //allocate from address to peer
        if (serviceRequest.getAddress() == MANAGER_ADDRESS || serviceRequest.getAddress() == GATEWAY_ADDRESS) {
            serviceCreated = subnetServices.allocateService(serviceRequest, node.getOutboundGraphId(), sendToRequester);
            if (serviceCreated) {
                subnetTopology.setReevaluateGraph(address, EvaluatePathPriority::OutboundService);
            }
        } else {
            serviceCreated = subnetServices.allocateService(serviceRequest, node.getInboundGraphId(), sendToRequester);
            if (serviceCreated) {
                subnetTopology.setReevaluateGraph(address, EvaluatePathPriority::InboundService);
            }
        }
    } else {
        serviceRequest.setRequestPending(false);
    }

    if (serviceRequest.sink) {
        // allocate from peer to address

        Service peerServiceRequest(serviceRequest.getPeerAddress(), serviceRequest.requestServiceId, 0,
                    serviceRequest.isSink(), serviceRequest.isSource(), serviceRequest.isIntermittent(),
                    serviceRequest.getApplicationDomain(), serviceRequest.getAddress(), serviceRequest.getPeriod(), 0,
                    false);

        if (serviceRequest.getAddress() == MANAGER_ADDRESS || serviceRequest.getAddress() == GATEWAY_ADDRESS) {
            serviceCreated = subnetServices.allocateService(peerServiceRequest, node.getInboundGraphId(), true);
            if (serviceCreated) {
                subnetTopology.setReevaluateGraph(address, EvaluatePathPriority::InboundService);
            }
        } else {
            serviceCreated = subnetServices.allocateService(peerServiceRequest, node.getOutboundGraphId(), true);
            if (serviceCreated) {
                subnetTopology.setReevaluateGraph(address, EvaluatePathPriority::OutboundService);
            }
        }

        if (peerServiceRequest.isRequestPending()) {
            serviceRequest.setRequestPending(true);
        }
    }

    std::ostringstream reason;
    reason << "CreateGatewayService for device:" << serviceRequest;
    SMState::SMStateLog::logDeviceTable("createService");

    return true;
}

bool NetworkEngine::terminateService(Address32 address, ServiceId serviceId) {
    bool inbound = false;
    bool outbound = false;
    Address32 nodeAddress = 0;

    //TODO: add delete service on peer service if the peer service exists
    bool result = subnetServices.terminateService(address, serviceId, inbound, outbound, nodeAddress);

    if (nodeAddress != 0) {
        if (inbound) {
            subnetTopology.setReevaluateGraph(nodeAddress, EvaluatePathPriority::InboundService);
        }

        if (outbound) {
            subnetTopology.setReevaluateGraph(nodeAddress, EvaluatePathPriority::OutboundService);
        }
    } else {
        LOG_WARN("Could not found peer address for service: " << std::hex << (long) serviceId << " on device: "
                    << ToStr(address));
    }

    return result;
}

void NetworkEngine::addVisible(Address32 existingAddress32, Address32 visibleAddress32, Uint8 rsl) {
    try {
        LOG_DEBUG("addVisible - no operations - existingAddress32=" << ToStr(existingAddress32)
                    << ", visibleAddress32=" << ToStr(visibleAddress32) << ", rsl=" << (int) rsl);

        if (existsDevice(existingAddress32) && existsDevice(visibleAddress32)) {
            subnetTopology.addVisibleNode(existingAddress32, visibleAddress32, rsl);
            return;
        }

        LOG_DEBUG("addVisible : visible REFUSED : existingAddress32=" << ToStr(existingAddress32)
                    << ", visibleAddress32=" << ToStr(visibleAddress32));

    } catch (NEException& e) {
        LOG_ERROR(e.what());
        throw e;
    }
}

void NetworkEngine::addDiagnostics(Address32 nodeAddress32, Address32 neighborAddress32, Uint8 rsl, Uint16 sent,
            Uint16 received, Uint16 failed) {

    try {
        LOG_INFO("addDiagnostics: nodeAddress=" << ToStr(nodeAddress32) << ", neighborAddress=" << ToStr(
                    neighborAddress32) << ", rsl=" << (int) rsl - 128 << ", sent=" << (int) sent << ", received="
                    << (int) received << ", failed=" << (int) failed);

        try
        {
            Device& device = getDevice(neighborAddress32);
            device.updateDiagnostics(sent, received, failed, 0);
        }
        catch (...)
        {
            LOG_WARN("addDiagnostics: nodeAddress=" << ToStr(nodeAddress32) << ", Device " << ToStr(neighborAddress32)
                        << " not found!");
            return;
        }

        subnetTopology.addDiagnostics(nodeAddress32, neighborAddress32, rsl, sent, received, failed);

    } catch (NEException& e) {
        LOG_ERROR(e.what());
        throw e;
    }
}

bool NetworkEngine::resolveOperations(EngineOperations& engineOperations, NE::Model::Operations::EngineOperations& confirmedOperationsList, bool removeOnTimeout) {
    bool checkRemovedDevices = false;
    bool timeout = false;

    try {
        ResolveOperationsVisitor resolveOperationsVisitor(*this);

        for (EngineOperationsVector::iterator it = confirmedOperationsList.getEngineOperations().begin(); it
                    != confirmedOperationsList.getEngineOperations().end(); ++it) {

            if ((*it)->getErrorCode() == ErrorCode::TIMEOUT) {
                timeout = true;
                if (removeOnTimeout && subnetTopology.existsNode((*it)->getOwner())) {

                    //check device status
                    if ( subnetTopology.getNode((*it)->getOwner()).getStatus() != Status::REMOVED ) {

                        subnetTopology.setRemoveStatus((*it)->getOwner());
                        checkRemovedDevices = true;

                        std::ostringstream stream;
                        stream << "resolveOperations - set check remove-> timeout for: ";
                        (*it)->toString(stream);

                        LOG_ERROR(stream.str());
                    }
                }
            } else {
                if (!(*it)->accept(resolveOperationsVisitor)) {
                    if (subnetTopology.markDeleted((*it)->getOwner())) {
                        checkRemovedDevices = true;
                    }

                    std::ostringstream stream;
                    stream << "resolveOperations - set check remove-> error for: ";
                    (*it)->toString(stream);

                    LOG_ERROR(stream.str());
                }
            }
        }

        bool isFinalResolve = true;
        for (EngineOperationsVector::iterator it = engineOperations.getEngineOperations().begin(); it
                    != engineOperations.getEngineOperations().end(); ++it) {
            if ((*it)->getState() != OperationState::CONFIRMED && (*it)->getState() != OperationState::SENT_IGNORED)
            {
                isFinalResolve = false;
                break;
            }
        }


        if (isFinalResolve)
        {
            if (engineOperations.getNetworkEngineEventType() != NetworkEngineEventType::NONE)
            {
                LOG_DEBUG("Final resolve of events.");
                Device& device = devicesTable.getDevice(engineOperations.getRequesterAddress32());
                NE::Model::DeviceAction::DeviceAction lastDevAction = device.getAction();
                device.updateAction(engineOperations.getNetworkEngineEventType(), checkRemovedDevices);

                if (engineOperations.getNetworkEngineEventType() == NetworkEngineEventType::EVALUATE_NEXT_ROUTE) {
                    device.setOnEvaluation(false);

                    if (subnetTopology.existsActivePath(engineOperations.getGraphId())) {
                        Path& path = subnetTopology.getPath(engineOperations.getGraphId());

                        if (path.getEvaluatePathPriority()
                                    == EvaluatePathPriority::InboundDiscovery || path.getEvaluatePathPriority()
                                    == EvaluatePathPriority::OutboundDiscovery) {
                            discoveryCount--;
                        }
                        evalCount--;

                        if (path.getType() == RoutingTypes::GRAPH_ROUTING_WITH_MIN_PATH_SELECTION) {
                            path.createTopologicalOrder();
                        }

                        Address32 address = (path.getSource() == MANAGER_ADDRESS) ? path.getDestination() : path.getSource();

                        if (subnetTopology.getNode(address).getNodeType() == NodeType::NODE) {
                            subnetTopology.pushOnEvaluationNodes(address);
                        }

                        path.setOnEvaluation(false);
                        path.setReevaluate(false, path.getEvaluatePathPriority());

                        LOG_DEBUG("Finishing evaluation of route.");
                        //TODO: ivp - if evaluation is with error?
                        subnetServices.finishEvaluation(engineOperations.getRequesterAddress32(),
                                    engineOperations.getGraphId(), path.getSource() == MANAGER_ADDRESS || path.getSource()
                                                == GATEWAY_ADDRESS);

                        if (timeout) {
                            path.setReevaluate(true, path.getEvaluatePathPriority());
                        }

                    } else {
                        LOG_DEBUG("resolveOperations - graph already deleted:" << ToStr(engineOperations.getGraphId()));
                        if (lastDevAction == DeviceAction::DISCOVERY_IN || lastDevAction == DeviceAction::DISCOVERY_OUT) {
                            discoveryCount--;
                        }
                        evalCount--;
                    }
                }

                //reoreder affected graph here
                if (engineOperations.getNetworkEngineEventType() == NetworkEngineEventType::REMOVE_DEVICES) {

                    if (settingsLogic.RoutingType == RoutingTypes::GRAPH_ROUTING_WITH_MIN_PATH_SELECTION) {

                        std::pair<std::multimap<int, Uint16>::iterator, std::multimap<int, Uint16>::iterator> rez;

                        subnetTopology.getGraphsToRefresh(engineOperations.getOperationsSetIndex(), rez);

                        subnetTopology.removeGraphsFromRefreshing(rez);
                    }
                }

                if (checkRemovedDevices) {
                    //for manager the noJoin is the number of check remove operation
                    devicesTable.incrementJoinNumber(MANAGER_ADDRESS);
                }

                std::ostringstream reason;
                reason << "Resolve operations: ";
                reason << engineOperations.reasonOfOperations;

                SMState::SMStateLog::logOperations(reason.str(), engineOperations, false);
                SMState::SMStateLog::logDeviceTable("resolveOperations");
                SMState::SMStateLog::logSubnetServices("resolveOperations");
            }
        }
        return checkRemovedDevices;

    } catch (NEException& e) {

        LOG_ERROR(e.what());

        throw e;
    }
}

void NetworkEngine::confirmDeviceQuarantine(Address32 address) {
    devicesTable.confirmJoinedDevice(address);

    SMState::SMStateLog::logDeviceTable("confirmDeviceQuarantine");
}

void NetworkEngine::setRemoveStatus(Address32 address) {
    LOG_DEBUG("setRemoveStatus - for address: " << ToStr(address));
    subnetTopology.setRemoveStatus(address);
    devicesTable.setRemoveStatus(address);
}

void NetworkEngine::getRemovedDevices(AddressSet& addressSet) {
    subnetTopology.getRemovedDevices(addressSet);
}


void NetworkEngine::checkRemovedDevices(EngineOperations& engineOperations, AddressSet& addressSet) {

    LOG_INFO("checkRemovedDevices - START");

    try {
        engineOperations.setRequesterAddress32(MANAGER_ADDRESS);
        engineOperations.setNetworkEngineEventType(NetworkEngineEventType::REMOVE_DEVICES);
        Device& device = devicesTable.getDevice(MANAGER_ADDRESS);
        device.setAction(DeviceAction::REMOVE);

        bool repeat;
        do {
            repeat = subnetTopology.releaseRemoved(engineOperations, addressSet);
        } while (repeat);
        subnetServices.releaseRemoved(engineOperations);
        subnetTdma.releaseRemoved(engineOperations);

        std::ostringstream reason;
        reason << "Check removed devices: ";

        for (AddressSet::iterator it = addressSet.begin(); it != addressSet.end(); ++it) {
            reason << ToStr(*it) << " ";
        }


        engineOperations.reasonOfOperations = reason.str();

        SMState::SMStateLog::logOperations(reason.str(), engineOperations);
        SMState::SMStateLog::logAllInfo("checkRemovedDevices");
    } catch (NEException& e) {
        LOG_ERROR("CheckRemovedDevices:" << e.what());
    }

    LOG_INFO("checkRemovedDevices - END");
}

EvaluatePathPriority::EvaluatePathPriority GetComparePriority(EvaluatePathPriority::EvaluatePathPriority priority) {
    // used to ensure that onbound and outbound service have equal chances to evaluation
    if (priority == EvaluatePathPriority::InboundDiscovery) {
        return EvaluatePathPriority::OutboundDiscovery;
    }

    return priority;
}

void NetworkEngine::triggerTopologyEvaluations() {

}

bool NetworkEngine::evaluateNextPath(EngineOperations& engineOperations) {
    LOG_DEBUG("evaluateNextPath - START");

    {
        for (std::set<Path*>::iterator it = priorityPathSet.begin(); it != priorityPathSet.end(); ++it) {
            LOG_DEBUG("PriorityQueue path=" << **it);
        }
    }

    LOG_DEBUG("evaluateNextPath - currently there are " << evalCount << " evaluations (maxEvaluations="
                        << settingsLogic.maxEvaluations << ") and " << discoveryCount
                        << " discovery evaluations(maxDiscoveryEvaluations=" << settingsLogic.maxDiscoveryEvaluations << ")");

    if (evalCount >= settingsLogic.maxEvaluations)
    {
//        LOG_DEBUG("evaluateNextPath - currently there are " << evalCount << " evaluations (maxEvaluations="
//                    << settingsLogic.maxEvaluations << ") and " << discoveryCount
//                    << " discovery evaluations(maxDiscoveryEvaluations=" << settingsLogic.maxDiscoveryEvaluations << ")");

        return false;
    }

    triggerTopologyEvaluations();

    bool result = false;

    bool makeOpsSI = false;

    Uint16 graphId = 0;

    Address32 address = 0;
    Address32 parent = 0;

    std::set<Path*>::iterator it = priorityPathSet.begin();
    for ( ; it != priorityPathSet.end(); ++it) {

        if ((*it)->isOnEvaluation()) {
            if ((*it)->getEvaluatePathPriority() == EvaluatePathPriority::InboundDiscovery
                        || (*it)->getEvaluatePathPriority() == EvaluatePathPriority::OutboundDiscovery) {

            }
        }
        else if ((*it)->isReevaluate()) {
            address = (*it)->getSource();
            if (address == MANAGER_ADDRESS || address == GATEWAY_ADDRESS) {
                address = (*it)->getDestination();
            }

            if (devicesTable.notExistsConfirmedDeviceOrOnEvaluation(address)) {
                continue;
            }

            break;
        }
    }

    if (it == priorityPathSet.end()) {
        return false;
    }

    Path& path = **it;
    try {
        graphId = path.getGraphId();

        if ((evalCount >= settingsLogic.maxDiscoveryEvaluations && (path.getEvaluatePathPriority()
                    == EvaluatePathPriority::InboundDiscovery || path.getEvaluatePathPriority()
                    == EvaluatePathPriority::OutboundDiscovery))) {

            LOG_DEBUG("evaluateNextPath - currently there are " << evalCount << " evaluations (maxEvaluations="
                        << settingsLogic.maxEvaluations << ") and " << discoveryCount
                        << " discovery evaluations(maxDiscoveryEvaluations=" << settingsLogic.maxDiscoveryEvaluations << ")");

            return false;
        }


        LOG_DEBUG("evaluateNextPath - selected path: " << path);

        if (path.getEvaluatePathPriority()
                    == EvaluatePathPriority::InboundDiscovery || path.getEvaluatePathPriority()
                    == EvaluatePathPriority::OutboundDiscovery) {
            discoveryCount++;
        }
        evalCount++;

        LinkEdgesList deallocateEdges;
        if (path.getEvaluatePathPriority() == EvaluatePathPriority::InboundAttach) {
            path.populateJoinLinkEdge(deallocateEdges, true);
            subnetTopology.removeJoinGraphNeighbor(engineOperations, address);
        } else if (path.getEvaluatePathPriority() == EvaluatePathPriority::OutboundAttach) {
            path.populateJoinLinkEdge(deallocateEdges, false);
            parent = subnetTopology.getNode(address).getParentAddress();
            subnetTdma.removeJoinLinks(engineOperations, address, parent);
        } else {
            path.populateLinkEdges(deallocateEdges, false);
        }

        DeviceAction::DeviceAction action = Device::getAction(path.getEvaluatePathPriority());

        bool allocate = false;

        if (path.getEvaluatePathPriority() == EvaluatePathPriority::OutboundAttach) {
                IEngineOperationPointer operation(new ReadWirelessDeviceCapabilitiesOperation(path.getDestination()));
                operation->setDependency(WaveDependency::ELEVENTH);
                engineOperations.addOperation(operation);
        }

        result = subnetTopology.evaluateNextPath(engineOperations, path);

        for (Operations::EngineOperationsVector::iterator it = engineOperations.getEngineOperations().begin(); it
                    != engineOperations.getEngineOperations().end(); ++it) {
            if ((*it)->getOperationType() == EngineOperationType::ADD_NEIGHBOR_GRAPH || (*it)->getOperationType()
                        == EngineOperationType::REMOVE_NEIGHBOR_GRAPH) {
                allocate = true;
                break;
            }
        }

        Uint16 newTraffic = 0;
        Uint16 oldTraffic = path.getTraffic();
        std::multiset<Uint16> compactPublishPeriods;

        AllocationDirection::Direction allocDirection = AllocationDirection::Outbound;
        if ((path.getEvaluatePathPriority() == EvaluatePathPriority::InboundAffected)
                    || (path.getEvaluatePathPriority() == EvaluatePathPriority::InboundAttach)
                    || (path.getEvaluatePathPriority() == EvaluatePathPriority::InboundDiscovery)
                    || (path.getEvaluatePathPriority() == EvaluatePathPriority::InboundService)) {
                        allocDirection = AllocationDirection::Inbound;
        }

        if (allocDirection == AllocationDirection::Outbound) {
            newTraffic = subnetServices.computeOutboundTraffic(path.getGraphId(), path.getSource(), path.getDestination());
        } else {
            newTraffic = subnetServices.computeInboundTraffic(path.getGraphId(), path.getSource(), path.getDestination(), compactPublishPeriods);
        }

        //on attach allocate the bandwidth for GW service; the total bandwidth is set in oldTraffic
        if (path.getEvaluatePathPriority() == EvaluatePathPriority::InboundAttach || path.getEvaluatePathPriority()
                    == EvaluatePathPriority::OutboundAttach) {
            newTraffic = oldTraffic;
        }

        if ((oldTraffic < newTraffic) || (oldTraffic / 2 >= newTraffic)) {
            allocate = true;
            LOG_DEBUG("evaluateNextPath - set allocate");
        }

        LinkEdgesList allocateEdges;

        if (allocate) {
            path.populateLinkEdges(allocateEdges, result);
        }

        Uint32 oldHandler = 0;
        Uint32 newHandler = 0;
        result = subnetServices.allocateResources(engineOperations, path.getGraphId(), path.getSource(),
                    path.getDestination(), allocate, oldHandler, newHandler);

        if (!result) {
            LOG_ERROR("Could not allocate resources on Path: " << path);
            return false;
        }

        if (allocate) {
            LOG_DEBUG("evaluateNextPath - oldHandler=" << ToStr(oldHandler) << ",  newHandler=" << ToStr(newHandler));

            //TODO: ivp - init phase and latency - for inbound service the phase should be set
            // according to the peer outbound service in order to have a green light on client server traffic
            Uint8 phase = rand() % 100;
            Uint8 latency = 255;
            bool resAllocate = true;

            if (allocateEdges.size() > 0) {

                if (allocDirection == AllocationDirection::Outbound) {

                    //TODO: ivp - change the generation of publishPeriod if publish > 250 ms
                    PublishPeriod::PublishPeriodEnum publishPeriod = toPublishPeriod(newTraffic);

                    LOG_DEBUG("Allocating traffic (outbound) - oldTraffic: " << (int) oldTraffic << ", newTraffic=" << (int) newTraffic
                                                << ", publishPeriod=" << (int) publishPeriod);

                    resAllocate = subnetTdma.allocatePublishTraffic(engineOperations, newHandler, allocateEdges,
                                publishPeriod, phase, latency, allocDirection);

                } else {

                    LOG_DEBUG("Allocating traffic (compacted, inbound) as a total of " << compactPublishPeriods.size()
                            << " links. Min traffic: " << *compactPublishPeriods.begin());

                    for (std::multiset<Uint16>::const_iterator cit = compactPublishPeriods.begin(); cit != compactPublishPeriods.end (); ++cit) {

                        resAllocate = resAllocate && subnetTdma.allocatePublishTraffic(engineOperations, newHandler,
                                    allocateEdges, (PublishPeriod::PublishPeriodEnum)(*cit), phase, latency, allocDirection);

                    }
                }

                if (path.getEvaluatePathPriority() == EvaluatePathPriority::OutboundAttach) {
                    Node& node = subnetTopology.getNode(path.getDestination());
                    if (node.getNodeType() == NodeType::NODE) {
                        subnetTdma.discoveryAllocation(engineOperations, newHandler, path.getDestination(),
                                    NodeType::NODE);
                    }
                }

                if (!resAllocate) {
                    LOG_ERROR("Could not allocate -> tries a roll back");
                    //put operations generated in SI state
                    makeOpsSI = true;
                }
            } else {
                Address32 tmpAddress = path.getSource();
                if (path.getSource() == MANAGER_ADDRESS || path.getSource() == GATEWAY_ADDRESS) {
                    tmpAddress = path.getDestination();
                }

                Node& tmpNode = subnetTopology.getNode(tmpAddress);
                if (tmpNode.getNodeType() == NodeType::NODE) {
                    LOG_ERROR("evaluateNextPath - the allocatedEdges is empty");
                }
            }

            if (deallocateEdges.size() > 0 && !makeOpsSI) {
                subnetTdma.deallocatePublishTraffic(engineOperations, newHandler, oldHandler, deallocateEdges);
            }

            LOG_DEBUG("allocateResources - after allocation");
        }

        if (path.getSource() == MANAGER_ADDRESS || path.getSource() == GATEWAY_ADDRESS) {
            engineOperations.setRequesterAddress32(path.getDestination());
            Device& device = devicesTable.getDevice(path.getDestination());
            device.setAction(action);
            device.setOnEvaluation(true);
        } else {
            engineOperations.setRequesterAddress32(path.getSource());
            Device& device = devicesTable.getDevice(path.getSource());
            device.setAction(action);
            device.setOnEvaluation(true);
        }

        if (allocate) {
            path.setOnEvaluation(true);
        } else {
            path.setReevaluate(false, path.getEvaluatePathPriority());
            path.setOnEvaluation(false);
        }

        engineOperations.setGraphId(graphId);
        engineOperations.setNetworkEngineEventType(NetworkEngineEventType::EVALUATE_NEXT_ROUTE);

        path.setTraffic(newTraffic);

        if (path.getEvaluatePathPriority() == EvaluatePathPriority::InboundService) {
            // only notify when the device is in quarantine and gets its inbound service with the GW, thus GW can communicate with it,
            // and only do it once, not for every service later on.
            if (getDevicesTable().getDevice(path.getSource()).status == DeviceStatus::QUARANTINE) {
                // notify GW about the device is in normal operation (JoinProcessStatusMask_NormalOperationCommencing)
                IEngineOperationPointer operationNormal(new ChangeNotificationOperation((uint32_t) GATEWAY_ADDRESS,
                            devicesTable.getAddress64(address), 769));
                operationNormal->setDependency(WaveDependency::TWELFTH);

                // add every operation as dependency, so that this operation is sent last.
                for(EngineOperationsVector::iterator opIt = engineOperations.getEngineOperations().begin();
                            opIt != engineOperations.getEngineOperations().end(); ++opIt)
                {
                    operationNormal->setOperationDependency(*opIt);
                }

                engineOperations.addOperation(operationNormal);
            }
        }

        if (settingsLogic.addCheckOperation && !makeOpsSI) {
            Address32 dest = path.getSource();
            if (path.getSource() == MANAGER_ADDRESS || path.getSource() == GATEWAY_ADDRESS) {
                dest = path.getDestination();
            }

            IEngineOperationPointer operation(new ChangePriorityEngineOperation(dest, 1));
            operation->setDependency(WaveDependency::TWELFTH);
            engineOperations.addOperation(operation);
        }

        // TODO radu : do we need it? anyway 769 does not match the
        // notify the GW that something changed with the path's source
        //        IEngineOperationPointer operation(new ChangeNotificationOperation((uint32_t) GATEWAY_ADDRESS, getAddress64(
        //                    path.getSource()), 769));
        //        operation->setDependency(WaveDependency::TWELFTH);
        //        engineOperations.addOperation(operation);

        if (path.getEvaluatePathPriority() == EvaluatePathPriority::OutboundAttach) {
            engineOperations.setAttachOutDependencies(address, parent);
        }

        if (path.getEvaluatePathPriority() == EvaluatePathPriority::InboundAttach) {
            engineOperations.setAttachInDependencies(address, parent);
        }

        //test if engine operations list is empty in order to reset requestPending flag if no operation is generated
        //also, resets device onevaluation flag
        if (engineOperations.getSize() == 0) {
            LOG_DEBUG("Operations list is empty");
            subnetServices.finishEvaluation(engineOperations.getRequesterAddress32(), engineOperations.getGraphId(),
                        path.getSource() == MANAGER_ADDRESS || path.getSource() == GATEWAY_ADDRESS);
            Device& device = devicesTable.getDevice(engineOperations.getRequesterAddress32());
            device.setOnEvaluation(false);
            device.updateAction(engineOperations.getNetworkEngineEventType(), false);   // set to Success
            evalCount--;
        }


    } catch (NEException& e) {
        LOG_ERROR("EvaluateNextPath:" << e.what());

        if (path.getEvaluatePathPriority()
                    == EvaluatePathPriority::InboundDiscovery || path.getEvaluatePathPriority()
                    == EvaluatePathPriority::OutboundDiscovery) {
            discoveryCount--;
        }
        evalCount--;
        makeOpsSI = true;
    }

    std::ostringstream reason;
    reason << "Evaluate route: " << path;
    engineOperations.reasonOfOperations = reason.str();

    //When error occurs change all operations state to SI and make a roll-back by resolving the operations
    if ( makeOpsSI ) {
        EngineOperationsVector::iterator it_ops = engineOperations.operations.begin();
        for ( ; it_ops != engineOperations.operations.end(); ++it_ops ) {
            (*it_ops)->setState(OperationState::SENT_IGNORED);

            (*it_ops)->setErrorCode(ErrorCode::ERROR);
        }

        resolveOperations(engineOperations, engineOperations, false);
    }
    // logs operation
    SMState::SMStateLog::logDeviceTable("evaluateNextPath");

    SMState::SMStateLog::logOperations(reason.str(), engineOperations);
    SMState::SMStateLog::logAllInfo(reason.str());

    LOG_DEBUG("evaluateNextPath - STOP");

    return result;
}

void NetworkEngine::evaluateClockSource(EngineOperations& engineOperations, Address32 address) {
    //find secondary clock source for address
    if (subnetTopology.getNode(address).getNodeType() == NodeType::NODE) {
        LOG_DEBUG("evaluateSecondaryClkSrc - for node: " << ToStr(address));
        if (evaluateSecondaryClkSrc(engineOperations, address, 254)) {
            LOG_DEBUG("evaluateNextPath - secondary clock source NOT SET for node " << ToStr(address));
            //subnetTopology.pushOnEvaluationNodes(address);
        }
    }
    std::ostringstream reason;
    reason << "Evaluate secondary clock source for node: " << ToStr(address);
    SMState::SMStateLog::logOperations(reason.str(), engineOperations);
    engineOperations.reasonOfOperations = reason.str();

}

bool NetworkEngine::evaluateSecondaryClkSrc(EngineOperations& engineOperations, Address32 addr, Uint16 maxHops) {
    //construct priority list (priority: from first to last)
    std::vector<Address32> candidateList;
    NodeMap& nodes = subnetTopology.getSubnetNodes();
    NodeMap::iterator it_node = nodes.find(addr);

    if (it_node == nodes.end()) {
        LOG_DEBUG("evaluateSecondaryClkSrc - node with address " << ToStr(addr) << " does not exist ");
        return false;
    }

    EdgeList edges = it_node->second.getOutBoundEdges();
    EdgeList::iterator it_edges;

    if (it_node->second.hasSecondaryClkSource()) {
        LOG_DEBUG("evaluateSecondaryClkSrc - node with address " << ToStr(addr) << " has secondary clock source");

        bool ClkSrcOK = true;
        bool foundEdge = false;

        //check if the edge is active
        for (it_edges = edges.begin(); it_edges != edges.end(); it_edges++) {
            if (it_edges->getDestination() == it_node->second.getSecondaryClkSource()) {
                foundEdge = true;
                if (it_edges->getActiveTraffic() > 0) {
                    LOG_DEBUG("evaluateSecondaryClkSrc - node with address " << ToStr(addr)
                                << " has an active secondary clock source");
                    return true;
                } else {
                    LOG_DEBUG("evaluateSecondaryClkSrc - node with address " << ToStr(addr)
                                << " has an inactive secondary clock source");
                    ClkSrcOK = false;
                    break;
                }
            }
        }

        if ((foundEdge && !ClkSrcOK) || !foundEdge) {
            //secondary clock source needs to be reevaluated
            SetClockSourceOperation *op = new SetClockSourceOperation(it_node->second.getNodeAddress(),
                        it_node->second.getSecondaryClkSource(), false);
            IEngineOperationPointer engineOp(op);
            engineOp->setDependency(WaveDependency::SIXTH);
            engineOperations.addOperation(engineOp);
            it_node->second.resetSecondaryClkSource();
        }

    }

    if (it_node->second.getPrimaryInbound() == 0x0000) {
        LOG_DEBUG("evaluateSecondaryClkSrc - node with address " << ToStr(addr) << " doesn't have primaryInbound set");
        return false;
    }

    Address32 secInbound = it_node->second.getSecondaryInbound();
    for (it_edges = edges.begin(); it_edges != edges.end(); it_edges++) {

        if ( it_edges->getDestination() == secInbound ) {
            if ( it_edges->getActiveTraffic() > 0 ){
                std::vector<Address32>::iterator it = candidateList.begin();
                candidateList.insert(it, secInbound);
            }
            continue;
        }

        if (it_edges->getDestination() == it_node->second.getPrimaryInbound()) {
            continue;
        }

        if (it_edges->getActiveTraffic() > 0) {
            candidateList.push_back(it_edges->getDestination());
        }
    }

    //select secondary clock source for node
    LOG_DEBUG("evaluateSecondaryClkSrc - evaluate candidates");

    bool found = false;

    Int16 nodeLevel = getPrimaryInboundLevel(addr, 0, maxHops);
    LOG_DEBUG("PrimaryInboundLevel for node (" << ToStr(addr) << ")=" << nodeLevel);

    if (nodeLevel < 0) {
        return false;
    }

    std::vector<Address32>::iterator it_candidates = candidateList.begin();
    for (; it_candidates != candidateList.end(); it_candidates++) {
        LOG_DEBUG("evaluateSecondaryClkSrc - evaluate candidate: " << ToStr(*it_candidates));
        found = false;
        NodeMap::iterator it_candidateNode = nodes.find(*it_candidates);

        if (it_candidateNode == nodes.end()) {
            LOG_DEBUG("evaluateSecondaryClkSrc - node " << ToStr(*it_candidates) << " not found");
            continue;
        }

        Int16 candidateLevel = getPrimaryInboundLevel(*it_candidates, 0, maxHops);
        LOG_DEBUG("PrimaryInboundLevel for candidate(" << ToStr(*it_candidates) << ")=" << candidateLevel);

        if (candidateLevel < 0) {
            continue;
        }

        if (nodeLevel > candidateLevel) {
            //candidate selected
            LOG_DEBUG("evaluateSecondaryClkSrc - candidate selected " << ToStr(addr) << " candidateLevel:"
                        << candidateLevel << " nodeLevel:" << nodeLevel);
            found = true;
        } else if (nodeLevel == candidateLevel) {
            //test if the secondary clock source candidate forms a cycle free graph
            if (testSecondaryClkSrc(addr, *it_candidates, 0, 254, nodeLevel)) {
                LOG_DEBUG("evaluateSecondaryClkSrc - candidate selected " << ToStr(addr) << " candidateLevel:"
                            << candidateLevel << " nodeLevel:" << nodeLevel);

                found = true;
            }
        }

        if (found) {
            if (existsCSCycle(it_node->second, *it_candidates)) {   // test for CS cycles
                continue;
            }

            it_node->second.getEdge(*it_candidates).setDestinationClockSource(true);
            it_node->second.setSecondaryClkSource(*it_candidates);

            //generate operation for the selected node
            LOG_DEBUG("evaluateSecondaryClkSrc - generate operation for " << ToStr(addr) << " with neighbor " << ToStr(
                        *it_candidates));
            SetClockSourceOperation *op = new SetClockSourceOperation(it_node->second.getNodeAddress(),
                        it_candidateNode->second.getNodeAddress(), true);
            IEngineOperationPointer engineOp(op);
            engineOp->setDependency(WaveDependency::FIFTH);
            engineOperations.addOperation(engineOp);

            return true;
        }
    }

    return false;
}

bool NetworkEngine::existsCSCycle(Node& node, Address32 peer) {
    std::set<Address32> processedAddresses;
    std::queue<Address32> remainingNodes;

    processedAddresses.insert(node.getNodeAddress());
    remainingNodes.push(peer);
    int steps = 500;
    while (!remainingNodes.empty()) {
        Address32 tmp = remainingNodes.front();
        remainingNodes.pop();

        if (tmp == node.getNodeAddress())
            return true;

        if (tmp == 0)
            continue;

        if (processedAddresses.find(tmp) != processedAddresses.end())
            continue;

        processedAddresses.insert(tmp);

        try
        {
            Node& tmpNode = subnetTopology.getNode(tmp);
            if (tmpNode.getBackboneAddress() != tmp) {
                remainingNodes.push(tmpNode.getPrimaryInbound());
                remainingNodes.push(tmpNode.getSecondaryClkSource());
            }
        }
        catch (...)
        {
            continue;
        }

        if (--steps <= 0) {
            LOG_INFO("Potential cycle in existsCSCycle.");
            return true;
        }
    }

    return false;
}

bool NetworkEngine::testSecondaryClkSrc(Address32 nodeAddr, Address32 crtAddr, Uint16 currentCount, Uint16 maxCounts,
            Uint16 level) {
    LOG_DEBUG("Test node " << ToStr(crtAddr) << " for " << ToStr(nodeAddr) << "(" << currentCount << ")");

    if (nodeAddr == crtAddr) {
        LOG_DEBUG("Cycle found " << ToStr(nodeAddr) << " = " << ToStr(crtAddr));
        return false;
    }

    if (currentCount == maxCounts) {
        LOG_DEBUG("Counter reaches his maximum value" << ToStr(nodeAddr) << " ; " << ToStr(crtAddr));
        return false;
    }

    Int16 crtNodeLevel = getPrimaryInboundLevel(crtAddr, 0, 254);
    LOG_DEBUG("SecondaryInboundLevel = " << crtNodeLevel);

    if (crtNodeLevel > level) { // do not assign secondary clock source if secondary path leads
                                // to primary path on greater level
        return false;
    }

    if (crtNodeLevel < level) {
        return true;
    }

    if (crtAddr == 0x0000) {
        return true;
    }

    NodeMap::iterator it_node = subnetTopology.getSubnetNodes().find(crtAddr);
    if (it_node == subnetTopology.getSubnetNodes().end()) {
        LOG_DEBUG("Node " << ToStr(crtAddr) << " missing");
        return false;
    }

    return testSecondaryClkSrc(nodeAddr, it_node->second.getSecondaryClkSource(), currentCount + 1, maxCounts, level);
}

Int16 NetworkEngine::getPrimaryInboundLevel(Address32 addr, Uint16 currentLevel, Uint16 maxLevel) {
    NodeMap& nodes = subnetTopology.getSubnetNodes();
    NodeMap::iterator it_node = nodes.find(addr);

    if (it_node == nodes.end()) {
        LOG_DEBUG("getPrimaryInboundLevel: node " << ToStr(addr) << " not found");
        return -maxLevel - 1;
    }

    Node node = it_node->second;

    if (node.getNodeType() == NodeType::BACKBONE) {
        LOG_DEBUG("getPrimaryInboundLevel: BACKBONE; level: " << currentLevel);
        return 0;
    } else if (currentLevel == maxLevel) {
        LOG_DEBUG("getPrimaryInboundLevel: MAX LEVEL " << currentLevel);
        return -maxLevel - 1;
    } else
        return 1 + getPrimaryInboundLevel(node.getPrimaryInbound(), currentLevel + 1, maxLevel);
}

PublishPeriod::PublishPeriodEnum NetworkEngine::toPublishPeriod(Uint16 traffic) {

    if (traffic > (int) PublishPeriod::P_250_MS) {
        return PublishPeriod::P_250_MS;
    } else if (traffic > (int) PublishPeriod::P_500_MS) {
        return PublishPeriod::P_250_MS;
    } else if (traffic > (int) PublishPeriod::P_1_S) {
        return PublishPeriod::P_500_MS;
    } else if (traffic > (int) PublishPeriod::P_2_S) {
        return PublishPeriod::P_1_S;
    } else if (traffic > (int) PublishPeriod::P_4_S) {
        return PublishPeriod::P_2_S;
    } else if (traffic > (int) PublishPeriod::P_8_S) {
        return PublishPeriod::P_4_S;
    } else if (traffic > (int) PublishPeriod::P_16_S) {
        return PublishPeriod::P_8_S;
    } else if (traffic > (int) PublishPeriod::P_32_S) {
        return PublishPeriod::P_16_S;
    } else if (traffic > (int) PublishPeriod::P_64_S) {
        return PublishPeriod::P_32_S;
    } else {
        return PublishPeriod::P_64_S;
    }
}

bool NetworkEngine::canSendAdvertise(Address32 address) {
    return subnetServices.canSendAdvertise(address);
}

/**
 * Returns true if the node should make advertise taking into account its level and type.
 */
bool isJoinLayerSupportingAdvertise(Node& node, Node& parent, uint16_t joinLayersNo) {
    if (joinLayersNo == 2) {
        if (parent.getNodeType() == NodeType::BACKBONE) {
            return true;
        }
    }

    if (joinLayersNo > 2) {
        return true;
    }

    return false;
}

bool NetworkEngine::allocateAdvertise(EngineOperations& engineOperations, Address32 address) {
    ServiceId inboundServiceId;
    ServiceId outboundServiceId;

    engineOperations.setRequesterAddress32(address);
    engineOperations.setNetworkEngineEventType(NetworkEngineEventType::MAKE_ROUTER);

    //    Uint16 joinLayers = settingsLogic.joinLayersNo;

    Device& device = devicesTable.getDevice(address);
    device.setAction(DeviceAction::ROUTER);

    Node& node = subnetTopology.getNode(address);
    Node& parent = subnetTopology.getNode(node.getParentAddress());

    if (!subnetServices.getManagementServiceIds(address, inboundServiceId, outboundServiceId)) {
        return false;
    }

    PublishPeriod::PublishPeriodEnum publishPeriod = (PublishPeriod::PublishPeriodEnum) ((Uint32) PublishPeriod::P_1_S
                / settingsLogic.joinBandwidth);

    bool result = false;
    //TODO: ivp - add config setting
    if (isJoinLayerSupportingAdvertise(node, parent, settingsLogic.joinLayersNo)) {
        result = subnetTdma.allocateJoinLinks(engineOperations, outboundServiceId, address, publishPeriod);
    }

    LOG_DEBUG("allocateAdvertise - for " << ToStr(address) << " result: " << (int) result);

    if (result) {
        Device & device = devicesTable.getDevice(engineOperations.getRequesterAddress32());
        device.setAction(DeviceAction::ROUTER);

        node.setRouter(true);

        //test - generate service and route for proxy address
        Uint16 graphId = getSuperframeId(publishPeriod);
        Uint32 joinPeriod = settingsLogic.joinBandwidth * 1000 * 32;

        AddressList destinations;

        subnetTopology.initOutboundDestinations(address, destinations);

        subnetServices.generateProxyService(engineOperations, address, destinations, graphId, joinPeriod);
    }

    if (!isJoinLayerSupportingAdvertise(node, parent, settingsLogic.joinLayersNo)) {
        result = true;
    }

    generateHealthReportPeriod(engineOperations, address);

    engineOperations.setAdvertiseDependencies();

    SMState::SMStateLog::logDeviceTable("allocateAdvertise");

    std::ostringstream reason;
    reason << "AllocateAdvertise for device: " << ToStr(address);
    SMState::SMStateLog::logOperations(reason.str(), engineOperations);
    engineOperations.reasonOfOperations = reason.str();

    return result;
}

void NetworkEngine::newAlarm788(Uint16 source, Uint16 destination) {
    std::ostringstream stream;
    stream << "new alarm 788 from source : " << ToStr(source) << " to destination : " << ToStr(destination);
    LOG_DEBUG(stream.str());
}

void NetworkEngine::resetEngine() {
    //this method is called only from tests to reset all previous states of the NetworkEngine
    //all members of the NetworkEngine are recreated/cleared.
    devicesTable.clearTable();
}

void NetworkEngine::toIndentString(std::ostringstream &stream) {

    stream << "DevicesTable: " << std::endl;
    devicesTable.toIndentString(stream);
    stream << std::endl;

    stream << "SubnetServices: " << std::endl;
    stream << std::endl;

    stream << "SubnetTopology " << std::endl << (int) subnetId << ": ";
    subnetTopology.toIndentString(stream);
    stream << std::endl;

}

void NetworkEngine::writeStateToStream(std::ostringstream& stream, EngineComponents::EngineComponentsEnum component) {
    switch (component) {
        case EngineComponents::ALL: {
            //TODO implement for all components
            return;
        }
        case EngineComponents::SubnetServices: {
            subnetServices.toString(stream);
            return;
        }
        case EngineComponents::DeviceTable: {
            getDevicesTable().toIndentString(stream);
            return;
        }
        case EngineComponents::Topology: {
            stream << "NetworkEngine {";
            stream << std::endl;

            subnetTopology.toIndentString(stream);

            stream << "}";
            stream << std::endl;
            return;
        }
        case EngineComponents::LinkEngine: {
            subnetTdma.toString(stream);
            return;
        }
        default: {
            LOG_ERROR("Invalid component id:" << component);
            return;
        }

    }
}

bool NetworkEngine::allocateCheckLink(NE::Model::Operations::EngineOperations& engineOperations, Address32 address,
            Address32 peerAddress) {
    Uint32 allocationHandler = subnetServices.getCheckHandler();

    return subnetTdma.allocateCheckLink(engineOperations, allocationHandler, address, peerAddress);
}

bool NetworkEngine::deallocateCheckLink(NE::Model::Operations::EngineOperations& engineOperations, Address32 address,
            Address32 peerAddress) {
    Uint32 allocationHandler = subnetServices.getCheckHandler();

    return subnetTdma.deallocateCheckLink(engineOperations, allocationHandler, address, peerAddress);
}

void NetworkEngine::markFailedEdge(Uint16 source, Uint16 destination) {
    SubnetTopology& topology = getSubnetTopology();
    topology.setEdgeFailing(source, destination, true);

    topology.reevaluateRoutesContainingEdge(source, destination);
}

bool NetworkEngine::deletePathFromPrioritySet(Path *path) {
    LOG_DEBUG("Deleting path from set:" << *path);
    std::set<Path*>::iterator it_p = priorityPathSet.find(path);
    if (it_p == priorityPathSet.end()) {
        return false;
    }
    LOG_DEBUG("Deleting path=" << **it_p);
    priorityPathSet.erase(it_p);
    return true;
}

bool NetworkEngine::deleteOldEntryInPriorityPathSet(Path* path) {
    LOG_DEBUG("Deleting old path from set:" << *path);

    std::set<Path*>::iterator it_path = priorityPathSet.find(path);
    if (it_path != priorityPathSet.end()) {
        LOG_DEBUG("Deleting path=" << **it_path);
        priorityPathSet.erase(it_path);
        return true;
    }
    return false;
}


void NetworkEngine::insertNewEntryInPriorityPathSet(Path* path) {
    LOG_DEBUG("Add path to set:" << *path);
    priorityPathSet.insert(path);
}

void NetworkEngine::registerMarkAsChangedCallback(MarkAsChangedCallback markAsChanged_) {
    this->markAsChanged = markAsChanged_;
}

void NetworkEngine::forwardMarkAsChanged(IEngineOperation& operation, uint32_t peerAddress) {
    if (markAsChanged) {
        markAsChanged(operation, peerAddress);
    }
}


uint8_t NetworkEngine::getMaxJoinPriority(Address32 address)
{
    uint8_t maxJP = 0;
    try
    {
        Node& topoNode = subnetTopology.getNode(address);

        Path& path = subnetTopology.getPath(topoNode.getInboundGraphId());

        NE::Model::Tdma::LinkEdgesList linkEdges;
        path.populateLinkEdges(linkEdges, false); //TODO check if isEval is ok on false

        for (NE::Model::Tdma::LinkEdgesList::iterator it = linkEdges.begin(); it != linkEdges.end(); ++it) {
            Device& source = devicesTable.getDevice(it->getSource());
            LOG_TRACE("getMaxJP checking address=" << ToStr(it->getSource()) << ", JP=" << source.getMetaDataAttributes()->getJoinPriority());

            if (maxJP < source.getMetaDataAttributes()->getJoinPriority())
            {
                maxJP = source.getMetaDataAttributes()->getJoinPriority();
            }
        }
    }
    catch (std::exception& ex)
    {
        LOG_WARN("Error occured while computing maxJP, ex=" << ex.what());
    }

    return maxJP;
}



