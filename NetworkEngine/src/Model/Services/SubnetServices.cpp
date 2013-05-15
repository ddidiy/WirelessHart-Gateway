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

#include <boost/lexical_cast.hpp>
#include "Common/SettingsLogic.h"
#include "SubnetServices.h"
#include "Model/DevicesTable.h"
#include "Model/NetworkEngine.h"
#include "Model/Operations/Services/ServiceAddedOperation.h"
#include "Model/Operations/Services/ServiceRemovedOperation.h"
#include "Misc/Marshall/NetworkOrderBytesWriter.h"
#include "SMState/SMStateLog.h"

using namespace NE::Common;
using namespace NE::Model::Operations;
using namespace NE::Model::Services;

SubnetServices::SubnetServices() {
}

SubnetServices::~SubnetServices() {
}

void SubnetServices::init() {
    nodeServices[MANAGER_ADDRESS] = NodeService(MANAGER_ADDRESS, MANAGER_ADDRESS, NodeType::MANAGER);
    managerService = nodeServices.find(MANAGER_ADDRESS);
}

void SubnetServices::addNode(EngineOperations& engineOperations, Address32 parentAddress, Address32 address,
            NodeType::NodeTypeEnum nodeType, Uint16 inboundGraphId, Uint16 outboundGraphId, Uint32 period,
            Uint32& inboundServiceId, Uint32& outboundServiceId) {

    //TODO: ivp - if node already exists?
    nodeServices[address] = NodeService(address, parentAddress, nodeType);
    NodeService& nodeService = nodeServices[address];
    if (address == GATEWAY_ADDRESS) {
        gatewayService = nodeServices.find(address);
    }

    inboundServiceId = nodeService.addInboundManagementService(engineOperations, inboundGraphId, period);
    outboundServiceId = managerService->second.addOutboundManagementService(engineOperations, address, nodeType,
                parentAddress, outboundGraphId, period);
}

bool SubnetServices::allocateService(Service& serviceRequest, Uint32 graphId, bool generateServiceOperation) {

    NodeServiceMap::iterator it = nodeServices.find(serviceRequest.address);

    if (it != nodeServices.end()) {
        return it->second.addService(serviceRequest, graphId, generateServiceOperation);
    } else {
        LOG_ERROR("allocatePublishService - node not found" << ToStr(serviceRequest.address));
        return false;
    }
}

bool SubnetServices::terminateService(Address32 address, ServiceId serviceId, bool& inbound, bool& outbound,
            Address32& peerAddress) {
    NodeServiceMap::iterator it = nodeServices.find(address);

    if (it == nodeServices.end()) {
        LOG_ERROR("allocatePublishService - node not found" << ToStr(address));
        return false;
    }

    ServiceId requestServiceId;

    bool result = it->second.terminateService(serviceId, peerAddress, requestServiceId);

    if (!result) {
        return result;
    }

    if (address == MANAGER_ADDRESS || address == GATEWAY_ADDRESS) {
        outbound = true;
    } else {
        inbound = true;
    }

    it = nodeServices.find(peerAddress);

    if (it == nodeServices.end()) {
        LOG_ERROR("allocatePublishService - peer node not found" << ToStr(peerAddress));
        return false;
    }

    if (address == MANAGER_ADDRESS || address == GATEWAY_ADDRESS) {
        inbound = true;
    } else {
        outbound = true;
    }

    return it->second.terminateService(requestServiceId);
}

bool SubnetServices::finishEvaluation(Address32 address, Uint32 graphId, bool isOutbound) {
    if (isOutbound) {
        return managerService->second.finishEvaluation(graphId)
                    | gatewayService->second.finishEvaluation(graphId);
    } else {
        NodeServiceMap::iterator it = nodeServices.find(address);

        if (it == nodeServices.end()) {
            LOG_ERROR("finishEvaluation - node not found" << ToStr(address));
            return false;
        }

        return it->second.finishEvaluation(graphId);
    }
}

Service& SubnetServices::SubnetServices::findManagementService(const Address32& destination, bool isProxyDestination) {
    return managerService->second.findManagementService(destination, isProxyDestination);
}

void SubnetServices::regenerateGatewayOperations(EngineOperations& engineOperations) {
    gatewayService->second.renewServices(engineOperations);
}

void SubnetServices::generateProxyService(EngineOperations& engineOperations, Address32 address,
            AddressList& destinations, Uint16 graphId, Uint32 period) {

    NodeServiceMap::iterator it = nodeServices.find(address);
    if (it != nodeServices.end()) {
        if (!it->second.isNodeType()) {
            return;
        }
    } else {
        LOG_ERROR("generateProxyService - no nodeService found for address: " << ToStr(address));
        return;
    }

    managerService->second.generateProxyService(engineOperations, address, destinations, graphId, period);
}

bool SubnetServices::resolveOperation(NE::Model::Operations::ServiceAddedOperation& operation) {
    NodeServiceMap::iterator it = nodeServices.find(operation.getOwner());
    if (it != nodeServices.end()) {
        return it->second.resolveOperation(operation);
    } else {
        std::ostringstream stream;
        stream << "resolveOperation - no nodeService found for operation: ";
        operation.toString(stream);

        LOG_ERROR(stream.str());
        return true;
    }
}

bool SubnetServices::resolveOperation(NE::Model::Operations::ServiceRemovedOperation& operation) {
    NodeServiceMap::iterator it = nodeServices.find(operation.getOwner());
    if (it != nodeServices.end()) {
        return it->second.resolveOperation(operation);
    } else {
        std::ostringstream stream;
        stream << "resolveOperation - no nodeService found for operation: ";
        operation.toString(stream);

        LOG_ERROR(stream.str());
        return true;
    }
}

bool SubnetServices::resolveOperation(NE::Model::Operations::RouteAddedOperation& operation) {
    NodeServiceMap::iterator it = nodeServices.find(operation.getOwner());
    if (it != nodeServices.end()) {
        return it->second.resolveOperation(operation);
    } else {
        std::ostringstream stream;
        stream << "resolveOperation - no nodeService found for operation: ";
        operation.toString(stream);

        LOG_ERROR(stream.str());

        return true;
    }
}

bool SubnetServices::resolveOperation(NE::Model::Operations::SourceRouteAddedOperation& operation) {
    NodeServiceMap::iterator it = nodeServices.find(operation.getOwner());
    if (it != nodeServices.end()) {
        return it->second.resolveOperation(operation);
    } else {
        std::ostringstream stream;
        stream << "resolveOperation - no nodeService found for operation: ";
        operation.toString(stream);

        LOG_ERROR(stream.str());

        return true;
    }
}

bool SubnetServices::resolveOperation(NE::Model::Operations::RouteRemovedOperation& operation) {
    NodeServiceMap::iterator it = nodeServices.find(operation.getOwner());
    if (it != nodeServices.end()) {
        return it->second.resolveOperation(operation);
    } else {
        std::ostringstream stream;
        stream << "resolveOperation - no nodeService found for operation: ";
        operation.toString(stream);

        LOG_ERROR(stream.str());
        return true;
    }
}

bool SubnetServices::resolveOperation(NE::Model::Operations::SourceRouteRemovedOperation& operation) {
    NodeServiceMap::iterator it = nodeServices.find(operation.getOwner());
    if (it != nodeServices.end()) {
        return it->second.resolveOperation(operation);
    } else {
        std::ostringstream stream;
        stream << "resolveOperation - no nodeService found for operation: ";
        operation.toString(stream);

        LOG_ERROR(stream.str());
        return true;
    }
}

bool SubnetServices::isRemovedStatus(Address32 address) {
    NodeServiceMap::iterator it = nodeServices.find(address);

    if (it != nodeServices.end()) {
        return it->second.getStatus() == Status::REMOVED;
    } else {
        //error
    }

    return false;
}

void SubnetServices::setRemovedStatus(Address32 address) {
    NodeServiceMap::iterator it = nodeServices.find(address);

    if (it != nodeServices.end()) {
        it->second.setStatus(Status::REMOVED);
    } else {
        //error
    }
}

void SubnetServices::releaseRemoved(NE::Model::Operations::EngineOperations& engineOperations) {
    managerService->second.releaseRemoved(engineOperations);

    NodeServiceMap::iterator it = nodeServices.find(GATEWAY_ADDRESS);
    if (it != nodeServices.end() && it->second.getStatus() != Status::REMOVED) {
        it->second.releaseRemoved(engineOperations);
    }

    for (NodeServiceMap::iterator it = nodeServices.begin(); it != nodeServices.end();) {
        if (it->first == MANAGER_ADDRESS) {
            ++it;
            continue;
        }

        if (it->second.getStatus() == Status::REMOVED) {
            nodeServices.erase(it++);
        } else {
            ++it;
        }
    }

}

bool SubnetServices::existsService(Uint32 serviceID) {
    for (NodeServiceMap::iterator it = nodeServices.begin(); it != nodeServices.end(); it++) {
        if (it->second.existsService(serviceID)) {
            return true;
        }
    }

    return false;
}

bool SubnetServices::getManagementServiceIds(Address32 address, ServiceId& inboundServiceId,
            ServiceId& outboundServiceId) {
    bool result = true;
    NodeServiceMap::iterator it = nodeServices.find(MANAGER_ADDRESS); // find manager node service for outbound management services.

    if (it != nodeServices.end()) {
        result = it->second.getManagementService(outboundServiceId, address);
    } else {
        return false;
    }

    it = nodeServices.find(address); // find node service for inbound management services.
    if (it != nodeServices.end()) {
        result &= it->second.getManagementService(inboundServiceId, MANAGER_ADDRESS);
    } else {
        return false;
    }

    return result;
}

bool SubnetServices::canSendAdvertise(Address32 address) {
    NodeServiceMap::iterator it = nodeServices.find(address);

    if (it != nodeServices.end()) {
        return it->second.canSendAdvertise(MANAGER_ADDRESS) && managerService->second.canSendAdvertise(address);
    } else {
        LOG_ERROR("canSendAdvertise - not found node: " << ToStr(address));
    }

    return false;
}

bool SubnetServices::allocateResources(EngineOperations& engineOperations, Uint16 graphId, Address32 source,
            Address32 destination, bool changeHandler, Uint32& oldHandler, Uint32& newHandler) {

    if (source == MANAGER_ADDRESS || source == GATEWAY_ADDRESS) {
        bool resultManager = managerService->second.allocateResources(engineOperations, graphId, destination,
                    changeHandler, oldHandler, newHandler);
        bool resultGateway = gatewayService->second.allocateResources(engineOperations, graphId, destination,
                    changeHandler, oldHandler, newHandler);
        return resultManager || resultGateway;
    }

    NodeServiceMap::iterator it = nodeServices.find(source);
    if (it != nodeServices.end()) {
        return it->second.allocateResources(engineOperations, graphId, destination, changeHandler, oldHandler,
                    newHandler);
    } else {
        return false;
    }

    return true;
}

Uint32 SubnetServices::getCheckHandler() {
    return gatewayService->second.getCheckHandler();
}

Uint16 SubnetServices::computeOutboundTraffic(Uint16 graphId, Address32 source, Address32 destination) {

    Uint16 traffic = 0;

    if (source == MANAGER_ADDRESS || source == GATEWAY_ADDRESS) {
        traffic = managerService->second.computeOutboundTraffic(graphId, destination);
        traffic += gatewayService->second.computeOutboundTraffic(graphId, destination);
    }

    return traffic;
}

Uint16 SubnetServices::computeInboundTraffic(Uint16 graphId, Address32 source, Address32 destination, std::multiset<Uint16>& compactPublishPeriods) {

    Uint16 traffic = 0;

    if (source != MANAGER_ADDRESS && source != GATEWAY_ADDRESS) {

        NodeServiceMap::iterator it = nodeServices.find(source);

        if (it != nodeServices.end()) {
            traffic = it->second.computeInboundTraffic(graphId, destination, compactPublishPeriods);
        }
    }

    return traffic;
}

void SubnetServices::toString(std::ostringstream& stream) {

    stream << "Service Table {" << "size=" << nodeServices.size() << ", " << std::endl;
    stream << std::setw(9) << "ServiceId";
    stream << std::setw(9) << "Addr";
    stream << std::setw(9) << "PeerAdd";
    stream << std::setw(9) << "Handler";
    stream << std::setw(6) << "Src";
    stream << std::setw(6) << "Sink";
    stream << std::setw(6) << "Int";
    stream << std::setw(10) << "AppDomain";
    stream << std::setw(9) << "RouteId";
    stream << std::setw(7) << "Period";
    stream << std::setw(12) << "OldPer";
    stream << std::setw(6) << "rqPnd";
    stream << std::setw(6) << "Mngt";
    stream << std::setw(11) << "Status";
    stream << std::endl;

    for (NodeServiceMap::iterator it = nodeServices.begin(); it != nodeServices.end(); it++) {
        it->second.servicesToString(stream);
    }

    stream << "}";
    stream << std::endl;
    stream << std::endl;

    stream << "Route Table {" << "size=" << nodeServices.size() << ", " << std::endl;
    stream << std::setw(9) << "RouteId";
    stream << std::setw(9) << "Address";
    stream << std::setw(9) << "PeerAdd";
    stream << std::setw(9) << "GraphId";
    stream << std::setw(6) << "SrcRt";
    stream << std::setw(9) << "OldSrcRt";
    stream << std::setw(6) << "Pndg";
    stream << std::setw(6) << "Mngt";
    stream << std::setw(11) << "Status";
    stream << std::endl;

    for (NodeServiceMap::iterator it = nodeServices.begin(); it != nodeServices.end(); it++) {
        it->second.routesToString(stream);
    }

    stream << "}";
    stream << std::endl;
    stream << std::endl;
}

