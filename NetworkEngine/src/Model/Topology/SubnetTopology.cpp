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

#include "SubnetTopology.h"

#include "Common/NETypes.h"
#include "Model/NetworkEngine.h"
#include "Model/Topology/DijkstraSearch.h"
#include "Model/Topology/PathIdGenerator.h"
#include "Model/Topology/Algorithms/SelectBestPeer.h"
#include "Model/Topology/Algorithms/SelectMinPath.h"
#include "SMState/SMStateLog.h"
#include <boost/shared_ptr.hpp>

using namespace NE::Model;
using namespace NE::Model::Topology;
using namespace NE::Model::Topology::Algorithms;
using namespace NE::Model::Operations;

SubnetTopology::SubnetTopology() {

}

void SubnetTopology::init(Uint16 _subnetId) {
    subnetId = _subnetId;
    searchId = 0;
    gatewayAdded = false;

    if (NetworkEngine::instance().getSettingsLogic().RoutingType == RoutingTypes::GRAPH_ROUTING) {
        graphRoutingAlgorithm.reset(new SelectBestPeer());
    }
    else if (NetworkEngine::instance().getSettingsLogic().RoutingType == RoutingTypes::GRAPH_ROUTING_WITH_MIN_PATH_SELECTION){
        graphRoutingAlgorithm.reset(new SelectMinPath());
    }

    Node managerNode(MANAGER_ADDRESS, MANAGER_ADDRESS, MANAGER_ADDRESS, NodeType::MANAGER);

    managerNode.setMetaDataAttributes(MetaDataAttributesPointer(new MetaDataAttributes()));

    managerNode.getMetaDataAttributes()->setTotalServices(0xFFFF);
    managerNode.getMetaDataAttributes()->setTotalRoutes(0xFFFF);
    managerNode.getMetaDataAttributes()->setTotalGraphs(0xFFFF);
    managerNode.getMetaDataAttributes()->setTotalGraphNeighbors(0xFFFF);
    managerNode.getMetaDataAttributes()->setTotalSuperframes(0xFFFF);
    managerNode.getMetaDataAttributes()->setTotalLinks(0xFFFF);
    managerNode.getMetaDataAttributes()->setTotalDiagnostics(0xFFFF);

    nodes[MANAGER_ADDRESS] = managerNode;
}

SubnetTopology::~SubnetTopology() {
}

Uint16 SubnetTopology::getSubnetId() {
    return subnetId;
}

bool SubnetTopology::isGatewayAdded() {
    return gatewayAdded;
}

NodeMap& SubnetTopology::getSubnetNodes() {
    return nodes;
}

Uint16 SubnetTopology::getSearchId() {
    return searchId;
}

Uint16 SubnetTopology::setNextSearchId() {
    return ++searchId;
}

PathMap& SubnetTopology::getPaths() {
    return paths;
}

bool SubnetTopology::existsNode(Address32 nodeAddress) {
    return nodes.find(nodeAddress) != nodes.end();
}

bool SubnetTopology::existsActiveNode(Address32 nodeAddress) {
    NodeMap::iterator it = nodes.find(nodeAddress);
    if (it == nodes.end()) {
        return false;
    }

    return it->second.getStatus() != Status::REMOVED && it->second.getStatus() != Status::DELETED;
}


bool SubnetTopology::existsPath(Uint16 graphId) {
    return paths.find(graphId) != paths.end();
}

bool SubnetTopology::existsActivePath(Uint16 graphId) {
    PathMap::iterator it = paths.find(graphId);
    if (it == paths.end()) {
        return false;
    }

    return it->second.getStatus() != Status::REMOVED && it->second.getStatus() != Status::DELETED;
}

Uint16 SubnetTopology::getNextPathId() {
    return PathIdGenerator::generatePathId();
}

Path& SubnetTopology::getPath(Uint16 graphId) {
    PathMap::iterator it = paths.find(graphId);
    if (it != paths.end()) {
        return it->second;
    } else {
        std::ostringstream stream;
        stream << "In subnet " << (int) subnetId;
        stream << " the following path does not exist (---): ";
        stream << ToStr(graphId);
        LOG_ERROR(stream.str());
        throw NE::Common::NEException(stream.str());
    }
}

Node& SubnetTopology::getNode(Address32 address) {
    NodeMap::iterator it = nodes.find(address);
    if (it != nodes.end()) {
        return it->second;
    }

    std::ostringstream stream;
    stream << "The node does not exist: ";
    stream << ToStr(address);
    LOG_ERROR(stream.str());
    throw NE::Common::NEException(stream.str());
}

void SubnetTopology::getListOfClockSources(Address32 address, std::vector<Address32>& list) {
}

void SubnetTopology::setEdgeFailing(Address32 src, Address32 dest, bool value) {
    Node& source = getNode(src);

    if (source.hasNeighbor(dest)) {
        Edge& edgeSource = source.getEdge(dest);
        edgeSource.IsEdgeFailing(value);
    }

    Node& destination = getNode(dest);

    if (destination.hasNeighbor(src)) {
        Edge& edgeDest = destination.getEdge(src);
        edgeDest.IsEdgeFailing(value);
    }
}

bool SubnetTopology::isEdgeFailing(Address32 src, Address32 dest) {
    bool failingSource = false;
    bool failingDestination = false;

    Node& source = getNode(src);
    Node& destination = getNode(dest);

    if (source.hasNeighbor(dest)) {
        failingSource = source.getEdge(dest).IsEdgeFailing();
    }

    if (destination.hasNeighbor(src)) {
        failingDestination = destination.getEdge(src).IsEdgeFailing();
    }

    return failingSource || failingDestination;
}

void SubnetTopology::reevaluateRoutesContainingEdge(Address32 src, Address32 dest) {
    for (PathMap::iterator it = paths.begin(); it != paths.end(); ++it) {
        Path& path = it->second;
        if (path.containsEdge(src, dest)) {
            if (path.getSource() == MANAGER_ADDRESS || path.getSource() == GATEWAY_ADDRESS) {
                path.setReevaluate(true, EvaluatePathPriority::OutboundAffected);
            } else {
                path.setReevaluate(true, EvaluatePathPriority::InboundAffected);
            }
        }
    }
}

void SubnetTopology::addNode(EngineOperations& engineOperations, Address32 newAddress, Address32 parentAddress,
            NodeType::NodeTypeEnum nodeType, Uint16 traffic, Uint16 proxyGraphId, Uint16& inboundGraphId,
            Uint16& outboundGraphId) {

    LOG_DEBUG("joinNode : parentAddress = " << ToStr(parentAddress) << ", newAddress = " << ToStr(newAddress)
                << ", subnetId = " << std::hex << (int) subnetId);

    // Only BACKBONE, GATEWAY and NODE can join
    if (nodeType != NodeType::BACKBONE && nodeType != NodeType::GATEWAY && nodeType != NodeType::NODE) {
        std::ostringstream stream;
        stream << "joinNode : Invalid nodeType! Only BACKBONE, GATEWAY and NODE can join the network!";
        LOG_ERROR(stream.str());
        throw NEException(stream.str());
    }

    NodeMap::iterator it = nodes.find(parentAddress);

    // ensure that the node to join through exists
    if (it == nodes.end()) {
        std::ostringstream stream;
        stream << "SubnetTopology with id " << subnetId << " does not contain a node with the address: ";
        stream << ToStr(parentAddress);
        LOG_ERROR(stream.str());
        throw NEException(stream.str());
    }

    Node& parent = it->second;

    // ensure that a NODE is joined only through another NODE or BACKBONE
    if (nodeType == NodeType::NODE) {
        if (parent.getNodeType() != NodeType::NODE && parent.getNodeType() != NodeType::BACKBONE) {
            std::ostringstream stream;
            stream << "The join of a node can be made only through another node or backbone!";
            stream << " Invalid joining address : " << ToStr(parentAddress);
            LOG_ERROR(stream.str());
            throw NEException(stream.str());
        }
    }

    Address32 backboneAddress = parent.getBackboneAddress();
    if (parentAddress == MANAGER_ADDRESS || parentAddress == GATEWAY_ADDRESS) {
        backboneAddress = newAddress;
    }

    if (nodeType == NodeType::GATEWAY) {
        gatewayAdded = true;
    }

    Node tmpNode(newAddress, parentAddress, backboneAddress, nodeType);
    nodes[newAddress] = tmpNode;

    Node& manager = getNode(MANAGER_ADDRESS);
    Node& newNode = getNode(newAddress);

    parent.addEdge(newAddress);
    newNode.addEdge(parentAddress);

    if (nodeType == NodeType::GATEWAY) {
        inboundGraphId = INBOUND_GW_GRAPH_ID;
        outboundGraphId = OUTBOUND_GW_GRAPH_ID;
    } else if (nodeType == NodeType::BACKBONE) {
        inboundGraphId = getNextPathId();
        outboundGraphId = getNextPathId();

        newNode.setRouter(true);
    } else {
        inboundGraphId = getNextPathId();
        outboundGraphId = getNextPathId();
    }

    newNode.setInboundGraphId(inboundGraphId);
    newNode.setOutboundGraphId(outboundGraphId);

    //if unknown routing algorithm is provided in config, GRAPH_ROUTING is selected (Mihai)
    SettingsLogic& settings = NetworkEngine::instance().getSettingsLogic();
    RoutingTypes::RoutingTypes_Enum type = RoutingTypes::GRAPH_ROUTING;
    if (settings.RoutingType == RoutingTypes::GRAPH_ROUTING_WITH_MIN_PATH_SELECTION) {
        type = RoutingTypes::GRAPH_ROUTING_WITH_MIN_PATH_SELECTION;
    }
    Path inboundPath(inboundGraphId, newAddress, MANAGER_ADDRESS, type, traffic);
    Path outboundPath(outboundGraphId, MANAGER_ADDRESS, newAddress, type, traffic);

    //the GW and AP are already evaluated so are added all required settings
    if (nodeType == NodeType::BACKBONE) {
        manager.addGraphNeighbor(engineOperations, outboundGraphId, ETHERNET_TRAFFIC, GATEWAY_ADDRESS, true);
        parent.addGraphNeighbor(engineOperations, outboundGraphId, ETHERNET_TRAFFIC, newAddress, true);
        parent.addGraphNeighbor(engineOperations, inboundGraphId, ETHERNET_TRAFFIC, MANAGER_ADDRESS, true);
        newNode.addGraphNeighbor(engineOperations, inboundGraphId, ETHERNET_TRAFFIC, GATEWAY_ADDRESS, true, true, true);

        //neighbors on proxy graph
        manager.addGraphNeighbor(engineOperations, proxyGraphId, ETHERNET_TRAFFIC, GATEWAY_ADDRESS, true);
        parent.addGraphNeighbor(engineOperations, proxyGraphId, ETHERNET_TRAFFIC, newAddress, true);
        // TODO should we also add path for these proxy graphs ?
        // ...
    } else if (nodeType == NodeType::GATEWAY) {
        manager.addGraphNeighbor(engineOperations, outboundGraphId, ETHERNET_TRAFFIC, GATEWAY_ADDRESS, true);
        newNode.addGraphNeighbor(engineOperations, inboundGraphId, ETHERNET_TRAFFIC, MANAGER_ADDRESS, true);
    }

    paths[inboundGraphId] = inboundPath;
    paths[outboundGraphId] = outboundPath;

    //the node should be reevaluated, no need to add other settings, for services send the parent graphId
    if (nodeType == NodeType::NODE) {
        paths[outboundGraphId].setReevaluate(true, EvaluatePathPriority::OutboundAttach);
        paths[inboundGraphId].setReevaluate(true, EvaluatePathPriority::InboundAttach);
    }



    if (nodeType == NodeType::NODE) {
        inboundGraphId = parent.getInboundGraphId();

        //test
        outboundGraphId = parent.getOutboundGraphId();

        SetClockSourceOperation *op = new SetClockSourceOperation(newAddress, parentAddress, true);
        IEngineOperationPointer engineOp(op);
        engineOp->setDependency(WaveDependency::SECOND);
        engineOperations.addOperation(engineOp);

        //test
        newNode.addGraphNeighbor(engineOperations, inboundGraphId, traffic, parentAddress, true);
    }
}

void SubnetTopology::removeJoinGraphNeighbor(EngineOperations& engineOperations, Address32 address) {

    Node& node = getNode(address);
    Node& parent = getNode(node.getParentAddress());

    if (node.getNodeType() == NodeType::NODE) {
        node.setRemoveGraphNeighbor(engineOperations, parent.getInboundGraphId(), node.getParentAddress());
    }
}

void SubnetTopology::regenerateGatewayOperations(NE::Model::Operations::EngineOperations& engineOperations) {

    IEngineOperationPointer operation(new NeighborGraphAddedOperation(GATEWAY_ADDRESS, INBOUND_GW_GRAPH_ID,
                MANAGER_ADDRESS));

    operation->setDependency(WaveDependency::FOURTH);
    engineOperations.addOperation(operation);
}

void SubnetTopology::markReachableNodes(Address32 address, Uint16 searchId) {
    //LOG_DEBUG("markReachableNodes - node=" << ToStr(address));
    Node& node = NetworkEngine::instance().getSubnetTopology().getNode(address);

    if (node.getStatus() == Status::REMOVED) {
        LOG_DEBUG("markReachableNodes - SKIP");
        // skips the node to be removed
        return;
    }

    if (node.getSearchId() == searchId) {
        //LOG_DEBUG("markReachableNodes - ALREADY evaluated");
        return;
    } else {
        node.setSearchId(searchId);
    }

    EdgeList & edges = node.getOutBoundEdges();
    for (EdgeList::iterator it = edges.begin(); it != edges.end(); ++it) {
        if (it->getStatus() == Status::DELETED || it->getStatus() == Status::CANDIDATE) {
            continue;
        }

        if (!NetworkEngine::instance().getDevicesTable().existsDevice(it->getDestination()))
        {
            continue;
        }

        //TODO: ivp - check if the device uses source route
        if (!it->existsGraphNeighbor()) {
            Node& peerNode = NetworkEngine::instance().getSubnetTopology().getNode(it->getDestination());

            if (peerNode.getParentAddress() != address) {
                continue;
            }

            Path& path = getPath(peerNode.getOutboundGraphId());

            if (path.getEvaluatePathPriority() != EvaluatePathPriority::OutboundAttach) {
                continue;
            }
        }

        markReachableNodes(it->getDestination(), searchId);
    }
}

void SubnetTopology::getRemovedDevices(AddressSet& addressSet)
{
    markReachableNodes(MANAGER_ADDRESS, setNextSearchId());

    for (NodeMap::iterator itNode = nodes.begin(); itNode != nodes.end(); ++itNode) {
        if (itNode->second.getSearchId() != searchId) {
            addressSet.insert(itNode->first);
        }
    }
}


bool SubnetTopology::releaseRemoved(EngineOperations& engineOperations, AddressSet& addressSet) {
    bool isGood = false;
    bool repeat = false;

    SubnetTdma& subnetTdma = NetworkEngine::instance().getSubnetTdma();
    SubnetServices& subnetServices = NetworkEngine::instance().getSubnetServices();
    DevicesTable& devicesTable = NetworkEngine::instance().getDevicesTable();

    markReachableNodes(MANAGER_ADDRESS, setNextSearchId());

    // steps through every node and see if the node has been touched (node.searchId has the value of
    // the current searchId)
    std::set<Uint16> affectedGraphs;
    for (NodeMap::iterator itNode = nodes.begin(); itNode != nodes.end(); ++itNode) {
        if (itNode->second.getSearchId() != searchId || itNode->second.getStatus() == Status::CANDIDATE_FOR_REMOVAL ) {
            LOG_INFO("Delete Node: " << ToStr(itNode->second.getNodeAddress()));

            affectedGraphs.insert( itNode->second.getOutboundGraphId() );
            affectedGraphs.insert( itNode->second.getInboundGraphId() );

            itNode->second.setStatus(Status::REMOVED);
            {
                Device& device = devicesTable.getDevice(itNode->first);
                if (device.capabilities.isGateway()) {
                    device.deviceRequested832 = true;
                } else {
                    device.deviceRequested832 = false;
                }

                // notify GW about the device has been removed (if the GW is not deleted also)
                if (devicesTable.existsDevice(GATEWAY_ADDRESS)) {
                    IEngineOperationPointer operation(new ChangeNotificationOperation((uint32_t) GATEWAY_ADDRESS,
                                device.address64, 769));
                    operation->setDependency(WaveDependency::TENTH);
                    engineOperations.addOperation(operation);
                }
            }

            subnetTdma.setRemovedStatus(itNode->first);
            subnetServices.setRemovedStatus(itNode->first);
            devicesTable.setRemoveStatus(itNode->first);

            EdgeList& edges = itNode->second.getOutBoundEdges();
            for (EdgeList::iterator itEdge = edges.begin(); itEdge != edges.end(); ++itEdge) {

                if (itEdge->getStatus() == Status::DELETED) {
                    continue;
                }

                GraphNeighborMap& neighbors = itEdge->getGraphs();
                for (GraphNeighborMap::iterator it = neighbors.begin(); it != neighbors.end(); ++it) {
                    if (it->second.status == Status::DELETED) {
                        continue;
                    }

                    if (!existsPath(it->first)) {
                        //TODO: ivp - !!! workaround
                        it->second.status = Status::DELETED;
                        LOG_ERROR("Deleted graph: " << ToStr(it->first) << " exists on node: " << itNode->second);
                    } else {
                        affectedGraphs.insert(it->first);
                        LOG_INFO("Affected graph(1): " << ToStr(it->first));
                    }
                }
            }

        } else {
            EdgeList& edges = itNode->second.getOutBoundEdges();
            for (EdgeList::iterator itEdge = edges.begin(); itEdge != edges.end();) {
                if (itEdge->getStatus() == Status::DELETED) {
                    ++itEdge;
                    continue;
                }

                //if peer node is not found mark the edge as deleted
                NodeMap::iterator it_peerNode = nodes.find(itEdge->getDestination());
                if (it_peerNode == nodes.end()) {
                    LOG_DEBUG("Mark deleted edge: its peer is down.");
                    itEdge->setStatus(Status::DELETED);
                    ++itEdge;
                    continue;
                }

                Node& peerNode = it_peerNode->second;

                //Node& peerNode = getNode(itEdge->getDestination());

                if (peerNode.getSearchId() != searchId) {
                    bool hasGraphNeighbors = false;

                    //generate delete GraphNeighbor operation
                    GraphNeighborMap& neighbors = itEdge->getGraphs();
                    for (GraphNeighborMap::iterator it = neighbors.begin(); it != neighbors.end(); ++it) {
                        if (it->second.status == Status::DELETED) {
                            continue;
                        }

                        if (!existsPath(it->first)) {
                            //TODO: ivp - !!! workaround
                            it->second.status = Status::DELETED;
                            LOG_ERROR("Deleted graph: " << ToStr(it->first) << " exists on node: " << itNode->second);
                        } else {
                            affectedGraphs.insert(it->first);
                            LOG_INFO("Affected graph(2 ): " << ToStr(it->first));
                        }

                        hasGraphNeighbors = true;

                        it->second.status = Status::DELETED;

                        if (it->first == OUTBOUND_GW_GRAPH_ID //
                                    && itEdge->getSource() == MANAGER_ADDRESS //
                                    && itEdge->getDestination() == GATEWAY_ADDRESS) {
                            continue;
                        }

                        IEngineOperationPointer operation(new NeighborGraphRemovedOperation(itEdge->getSource(),
                                    it->first, itEdge->getDestination()));

                        operation->setDependency(WaveDependency::NINETH);
                        engineOperations.addOperation(operation);
                    }

                    if (hasGraphNeighbors) {
                        LOG_DEBUG("releaseRemoved - set DELETED edge " << *itEdge);
                        itEdge->setStatus(Status::DELETED);

                        if ((itNode->second.getNodeType() == NodeType::NODE) && (itNode->second.getSecondaryClkSource()
                                    == peerNode.getNodeAddress())) {

                            itNode->second.resetSecondaryClkSource();
                            SetClockSourceOperation *op = new SetClockSourceOperation(itNode->second.getNodeAddress(),
                                        peerNode.getNodeAddress(), 0);
                            IEngineOperationPointer engineOp(op);
                            engineOp->setDependency(WaveDependency::SECOND);
                            engineOperations.addOperation(engineOp);
                        }

                        itEdge++;
                    } else {
                        LOG_DEBUG("releaseRemoved - delete edge" << *itEdge);

                        if ((itNode->second.getNodeType() == NodeType::NODE) && (itNode->second.getSecondaryClkSource()
                                    == peerNode.getNodeAddress())) {
                            itNode->second.resetSecondaryClkSource();
                            SetClockSourceOperation *op = new SetClockSourceOperation(itNode->second.getNodeAddress(),
                                        peerNode.getNodeAddress(), 0);
                            IEngineOperationPointer engineOp(op);
                            engineOp->setDependency(WaveDependency::SECOND);
                            engineOperations.addOperation(engineOp);
                        }

                        //edges.erase(itEdge++);
                        itNode->second.removeEdge(itEdge);
                    }
                } else {
                    ++itEdge;
                }
            }
        }
    }

    // checks the affected routing graphs
    //if the source or destination of the graph is deleted, check if the graph is used by other route,
    //delete or change the source/destination of the graph
    for (std::set<Uint16>::iterator it = affectedGraphs.begin(); it != affectedGraphs.end(); ++it) {
        Path& path = getPath(*it);
        if (path.getType() == RoutingTypes::SOURCE_ROUTING) {
            isGood = path.checkSourcePath();
        } else if (path.getType() == RoutingTypes::GRAPH_ROUTING || path.getType() == RoutingTypes::GRAPH_ROUTING_WITH_MIN_PATH_SELECTION) {
            isGood = path.checkGraphPath(engineOperations, setNextSearchId());
        }

        LOG_DEBUG("releaseRemoved - isGood=" << (int) isGood << " path: " << path);

        //TODO: temporary until a graph is used by multiple devices - in the case
        // that a graph is used by multiple routes, update the source/destination of the graph
        if (!isGood) {
            Address32 address = path.getSource();
            if (address == MANAGER_ADDRESS) {
                address = path.getDestination();
            }

            NodeMap::iterator nodeIt = nodes.find(address);

            if (nodeIt != nodes.end()) {
                if (nodeIt->second.getStatus() != Status::REMOVED) {
                    nodeIt->second.setStatus(Status::CANDIDATE_FOR_REMOVAL);
                    repeat = true;
                }
            }

            if (nodeIt == nodes.end() || nodeIt->second.getStatus() == Status::REMOVED) {
                LOG_DEBUG("releaseRemoved - delete path: " << path);

                PathMap::iterator itPath = paths.find(*it);
                if (itPath != paths.end()) {
                    //paths.erase(itPath);
                    deletePath(itPath);
                }
            }
        }
        else {
            graphsToRefresh.insert(std::pair<int, Uint16>(engineOperations.getOperationsSetIndex(), *it));
            PathMap::iterator itPath = paths.find(*it);
            if (itPath != paths.end()) {
                itPath->second.createTopologicalOrder();
            }
        }
    }

    for (PathMap::iterator it = paths.begin(); it != paths.end();) {
        if (it->second.getEvaluatePathPriority() != EvaluatePathPriority::InboundAttach
                    && it->second.getEvaluatePathPriority() != EvaluatePathPriority::OutboundAttach
                    && devicesTable.getDeviceStatus(it->second.getSource()) != DeviceStatus::DELETED
                    && devicesTable.getDeviceStatus(it->second.getDestination()) != DeviceStatus::DELETED
                    ) {
            ++it;
            continue;
        }

        Address32 address = it->second.getSource();

        if (address == MANAGER_ADDRESS || address == GATEWAY_ADDRESS) {
            address = it->second.getDestination();
        }

        NodeMap::iterator itNode = nodes.find(address);

        if (itNode == nodes.end() || itNode->second.getStatus() == Status::REMOVED) {
            //paths.erase(it++);
            deletePath(it);
        } else {
            ++it;
        }
    }

    for (NodeMap::iterator it = nodes.begin(); it != nodes.end();) {
        if (it->second.getStatus() == Status::REMOVED) {
            subnetServices.setRemovedStatus(it->first);
            subnetTdma.setRemovedStatus(it->first);
            devicesTable.setRemoveStatus(it->first);

            addressSet.insert(it->first);

            if ((it->second.getNodeType() == NodeType::NODE) //
                        && (it->second.hasNeighbor(it->second.getBackboneAddress())) //
                        && (devicesTable.existsDevice(it->second.getBackboneAddress()))) {
                SetClockSourceOperation *op = new SetClockSourceOperation(it->second.getBackboneAddress(), it->first,
                            0x2);
                IEngineOperationPointer engineOp(op);
                engineOp->setDependency(WaveDependency::FIFTH);
                engineOperations.addOperation(engineOp);
            }

            nodes.erase(it++);
        } else {
            ++it;
        }
    }
    if (repeat) {
        LOG_DEBUG("Repeat");
    }
    return repeat;
}

void SubnetTopology::addVisibleNode(Address32 nodeAddress, Address32 visibleAddress, Uint8 rsl) {
    DevicesTable& devicesTable = NetworkEngine::instance().getDevicesTable();

    NodeMap::iterator it = nodes.find(nodeAddress);
    if (it == nodes.end()) {
        LOG_WARN("addVisibleNode to " << ToStr(visibleAddress) << " : The address does not exist : " << ToStr(
                    nodeAddress));
        return;
    }
    Node& node = it->second;

    it = nodes.find(visibleAddress);
    if (it == nodes.end()) {
        // a device reports that can hear another device that is not yet join
        LOG_INFO("addVisibleNode : For node " << ToStr(nodeAddress) << " The visible address does not exist : "
                    << ToStr(visibleAddress));
        return;
    }
    Node& visible = it->second;

    bool existingHasVisible = node.hasNeighbor(visibleAddress);
    bool visibleHasExisting = visible.hasNeighbor(nodeAddress);

    bool changedExisting = node.addVisibleNeighbor(visibleAddress, rsl);
    bool changedVisible = visible.addVisibleNeighbor(nodeAddress, rsl);

    bool fastEvaluate = false;

    if (node.getNodeType() == NodeType::BACKBONE && !visibleHasExisting) {
        changedVisible = true;
        fastEvaluate = false;
    }

    if (node.getSecondaryInbound() == 0 || node.getSecondaryOutbound() == 0) {
        changedVisible = true;
        fastEvaluate = false;
    }

    if (visible.getNodeType() == NodeType::BACKBONE && !existingHasVisible) {
        changedExisting = true;
        fastEvaluate = false;
    }

    if (visible.getSecondaryInbound() == 0 || visible.getSecondaryOutbound() == 0) {
        changedExisting = true;
        fastEvaluate = false;
    }

    if (!changedExisting && !changedVisible) {
        return;
    }

    if (node.getNodeType() == NodeType::NODE) {
        if (devicesTable.getDevice(nodeAddress).isOperational()) {
            updateReevaluatePaths(nodeAddress, MANAGER_ADDRESS, fastEvaluate);
            if (gatewayAdded) {
                updateReevaluatePaths(nodeAddress, GATEWAY_ADDRESS, fastEvaluate);
            }
        }
    }

    if (visible.getNodeType() == NodeType::NODE) {
        if (devicesTable.getDevice(visibleAddress).isOperational()) {
            updateReevaluatePaths(MANAGER_ADDRESS, visibleAddress, fastEvaluate);
            if (gatewayAdded) {
                updateReevaluatePaths(GATEWAY_ADDRESS, visibleAddress, fastEvaluate);
            }
        }
    }

}

void SubnetTopology::addDiagnostics(Address32 nodeAddress, Address32 neighborAddress, Uint8 rsl, Uint16 sent,
            Uint16 received, Uint16 failed) {

    DevicesTable& devicesTable = NetworkEngine::instance().getDevicesTable();

    NodeMap::iterator it = nodes.find(nodeAddress);
    if (it == nodes.end()) {
        LOG_WARN("addDiagnostics : The address does not exist : " << ToStr(nodeAddress));
        return;
    }
    Node& node = it->second;

    it = nodes.find(neighborAddress);
    if (it == nodes.end()) {
        LOG_WARN("addDiagnostics : The peer address does not exist : " << ToStr(neighborAddress));
        return;
    }
    Node& neighbor = it->second;

    bool changed = node.addDiagnostics(neighborAddress, rsl, sent, received, failed);

    if (!changed) {
        return;
    }

        if (devicesTable.getDevice(nodeAddress).isOperational()) {
            updateReevaluatePaths(nodeAddress, MANAGER_ADDRESS, false);
            if (gatewayAdded) {
                updateReevaluatePaths(nodeAddress, GATEWAY_ADDRESS, false);
            }
        }

        if (devicesTable.getDevice(neighborAddress).isOperational()) {
            updateReevaluatePaths(MANAGER_ADDRESS, neighborAddress, false);
            if (gatewayAdded) {
                updateReevaluatePaths(GATEWAY_ADDRESS, neighborAddress, false);
            }
        }
}

void SubnetTopology::setRemoveStatus(Address32 address) {
    LOG_DEBUG("setRemoveStatus - node=" << ToStr(address));

    NodeMap::iterator it = nodes.find(address);

    if (it != nodes.end()) {
        it->second.setStatus(Status::REMOVED);
    } else {
        LOG_WARN("setRemoveStatus - node don't exists: " << ToStr(address));
    }
}

bool SubnetTopology::markDeleted(Address32 address) {
    LOG_DEBUG("markDeleted - node=" << ToStr(address));

    NodeMap::iterator it = nodes.find(address);

    if (it != nodes.end()) {
        if (it->second.getNodeType() == NodeType::NODE) {
            it->second.setStatus(Status::REMOVED);
            return true;
        } else {
            LOG_WARN("markDeleted - SKIP set removed on operation fail for: " << ToStr(address));
            return false;
        }
    } else {
        LOG_WARN("setNodeStatus - node don't exists: " << ToStr(address));
        return false;
    }
}

void SubnetTopology::setReevaluateGraph(Address32 address,
            EvaluatePathPriority::EvaluatePathPriority evaluatePathPriority) {

    Uint32 graphId;

    NodeMap::iterator itNode = nodes.find(address);

    if (itNode == nodes.end()) {
        LOG_ERROR("setReevaluateGraph - node not found " << ToStr(address));
        return;
    }

    if (evaluatePathPriority == EvaluatePathPriority::InboundService) {
        graphId = itNode->second.getInboundGraphId();
    } else {
        graphId = itNode->second.getOutboundGraphId();
    }

    PathMap::iterator it = paths.find(graphId);
    if (it == paths.end()) {
        LOG_ERROR("setReevaluateGraph - graph not found" << ToStr(graphId));
        return;
    }

    it->second.setReevaluate(true, evaluatePathPriority);
}

Uint16 SubnetTopology::createSourcePath(Address32 source, Address32 destination, Uint16 traffic, bool managementPath,
            Uint8 maxHops) {

    LOG_DEBUG("createSourcePath(" << ToStr(source) << ", " << ToStr(destination) << ", traffic = " << traffic
                << ", maxHops = " << (int) maxHops << ")");

    AddressList addressList;

    DijkstraSearch::searchDijkstra(source, destination, nodes, addressList);

    if (addressList.size() == 0) {
        LOG_DEBUG("createPath: There is no path between the " << ToStr(source) << " and " << ToStr(destination));
    } else {
        //LOG_DEBUG("createPath() : the path is : " << NE::Model::Topology::nodesToString(sourcePath));
    }

    Path path(getNextPathId(), source, destination, RoutingTypes::SOURCE_ROUTING, traffic);

    path.setSourcePath(addressList);

    paths[path.getGraphId()] = path;

    LOG_TRACE("createSourcePath end");

    return path.getGraphId();
}

Uint16 SubnetTopology::createGraphPath(Address32 source, Address32 destination, Uint16 traffic, bool managementPath,
            bool broadcastAllocation, Uint8 redundantFactor, Uint8 maxHops) {

    LOG_DEBUG("createGraphRouting: source=" << ToStr(source) << ", destination=" << ToStr(destination) << ", traffic="
                << (int) traffic << ", broadcastAllocation=" << broadcastAllocation << ", redundantFactor="
                << (int) redundantFactor << ", maxHops=" << (int) maxHops);

    Path path(getNextPathId(), source, destination, RoutingTypes::GRAPH_ROUTING, traffic);

    path.setRedundantFactor(redundantFactor);
    path.setMaxHops(maxHops);

    paths[path.getGraphId()] = path;

    LOG_TRACE("createGraphRouting end");
    return path.getGraphId();
}

bool SubnetTopology::evaluateNextPath(EngineOperations& engineOperations, Path& path) {
    LOG_DEBUG("evaluateNextPath - " << path);

    if (getNode(path.getSource()).getNodeType() != NodeType::NODE && getNode(path.getDestination()).getNodeType()
                != NodeType::NODE) {
        return true;
    }

    path.updateLastEvalTime();

    if (path.getType() == RoutingTypes::SOURCE_ROUTING) {
        AddressList addressList;
        DijkstraSearch::searchDijkstra(path.getSource(), path.getDestination(), nodes, addressList);

        if (addressList.size() == 0) {
            LOG_DEBUG("recreatePath: There is no path between the " << ToStr(path.getSource()) << " and " << ToStr(
                        path.getDestination()));
            return false;
        } else if (path.getMaxHops() <= addressList.size()) {
            LOG_DEBUG("recreatePath: There path is too long for (" << ToStr(path.getSource()) << " -> " << ToStr(
                        path.getDestination()) << " ), hops: " << (int) addressList.size() << " > "
                        << (int) path.getMaxHops());
        } else {
            path.setSourcePath(addressList);
            return true;
        }
    } else if (path.getType() == RoutingTypes::GRAPH_ROUTING || path.getType() == RoutingTypes::GRAPH_ROUTING_WITH_MIN_PATH_SELECTION) {
        return graphRoutingAlgorithm->evaluateGraphPath(engineOperations, path);
    }

    return false;
}

void SubnetTopology::updateReevaluatePaths(Address32 source, Address32 destination, bool fastEvaluate) {

    LOG_DEBUG("updateReevaluatePaths for : " << ToStr(source) << " -> " << ToStr(destination));

    for (PathMap::iterator it = paths.begin(); it != paths.end(); ++it) {
        if ((it->second.getSource() == source && it->second.getDestination() == destination) || (it->second.getSource()
                    == destination && it->second.getDestination() == source)) {

            if (it->second.getStatus() == Status::REMOVED || it->second.getStatus() == Status::DELETED) {
                LOG_DEBUG("skip route " << (int) it->first);
                continue;
            }
            LOG_DEBUG("updateReevaluatePaths set reevaluate for route : " << it->second);

            if (it->second.getSource() == MANAGER_ADDRESS || it->second.getSource() == GATEWAY_ADDRESS) {
                it->second.setReevaluate(true, fastEvaluate ? EvaluatePathPriority::OutboundAffected
                            : EvaluatePathPriority::OutboundDiscovery);
            } else {
                it->second.setReevaluate(true, fastEvaluate ? EvaluatePathPriority::InboundAffected
                            : EvaluatePathPriority::InboundDiscovery);
            }
        }
    }
}

void SubnetTopology::initOutboundDestinations(Address32 address, AddressList& destinations) {
    NodeMap::iterator it = nodes.find(address);
    if (it != nodes.end()) {
        Uint16 graphId = it->second.getOutboundGraphId();

        if (existsPath(graphId)) {
            getPath(graphId).initDestinations(address, destinations);
        } else {
            LOG_ERROR("initOutboundDestinations - node found: " << ToStr(address) << ", path not found: " << ToStr(
                        graphId));
        }
    } else {
        LOG_ERROR("initOutboundDestinations - node not found: " << ToStr(address));

    }
}

Address32 SubnetTopology::popOnEvaluationNodes() {
    if (onEvaluationNodesQueue.empty()) {
        return 0;
    } else {
        Address32 result = onEvaluationNodesQueue.front();
        onEvaluationNodesQueue.pop();
        return result;
    }
}

void SubnetTopology::pushOnEvaluationNodes(Address32 address) {
    onEvaluationNodesQueue.push(address);
}

bool SubnetTopology::resolveOperation(NeighborGraphAddedOperation& operation) {
    NodeMap::iterator it = nodes.find(operation.getOwner());
    if (it != nodes.end()) {
        return it->second.resolveOperation(operation);
    } else {
        std::ostringstream stream;
        stream << "resolveOperation - no owner found for operation: ";
        operation.toString(stream);

        LOG_ERROR(stream.str());
        //TODO: ivp - it is ok?
        return true;
    }
}

void SubnetTopology::deletePath(PathMap::iterator it) {
    //delete from priority set
    NetworkEngine::instance().deletePathFromPrioritySet(&(it->second));
    //delete from path map
    paths.erase(it);
}

bool SubnetTopology::resolveOperation(NeighborGraphRemovedOperation& operation) {
    NodeMap::iterator it = nodes.find(operation.getOwner());
    if (it != nodes.end()) {
        return it->second.resolveOperation(operation);
    } else {
        std::ostringstream stream;
        stream << "resolveOperation - no owner found for operation: ";
        operation.toString(stream);

        LOG_ERROR(stream.str());
        //TODO: ivp - it is ok?
        return true;
    }
}

void SubnetTopology::getGraphsToRefresh(int operationsIndex,
            std::pair<std::multimap<int, Uint16>::iterator, std::multimap<int, Uint16>::iterator>& rez) {
    rez = graphsToRefresh.equal_range(operationsIndex);
}

void SubnetTopology::removeGraphsFromRefreshing(std::pair<std::multimap<int, Uint16>::iterator, std::multimap<int, Uint16>::iterator>& rez) {
    graphsToRefresh.erase(rez.first, rez.second);
}


std::string SubnetTopology::toDotString() {
    std::ostringstream stream;
    stream << "digraph finite_state_machine {";
    stream << std::endl;

    stream << " size=\"25,22\"";
    stream << std::endl;

    stream << "\tnode [shape = egg, style=filled,  fillcolor=brown]; \"f980\";";
    stream << std::endl;

    stream << "\tnode [shape = egg, style=filled,  fillcolor=brown]; \"f981\";";
    stream << std::endl;

    stream << "\tnode [shape = circle, style=solid];";
    stream << std::endl;

    for (NodeMap::iterator it = nodes.begin(); it != nodes.end(); ++it) {
        EdgeList& edges = (*it).second.getOutBoundEdges();
        for (EdgeList::iterator itEdge = edges.begin(); itEdge != edges.end(); ++itEdge) {
            if ((*itEdge).getStatus() == Status::ACTIVE) {
                stream << "\t" << "\"" << std::hex << (*itEdge).getSource() << "\"";
                stream << " -> ";
                stream << "\"" << std::hex << (*itEdge).getDestination() << "\"";
                stream << " [ color=";

                if ((*itEdge).getStatus() == Status::ACTIVE) {
                    stream << "\"black\"";
                } else if ((*itEdge).getStatus() == Status::DELETED) {
                    stream << "\"red\"";
                } else if ((*itEdge).getStatus() == Status::NEW) {
                    stream << "\"green\"";
                } else {
                    LOG_ERROR("Not known edge type!");
                }

                stream << " ]; ";
                stream << std::endl;
            }
        }
    }

    stream << "}";
    stream << std::endl;

    return stream.str();
}

void SubnetTopology::toIndentString(std::ostringstream& stream) {
    stream << "SubnetTopology {";
    stream << "subnetId=" << subnetId;
    stream << ", " << std::endl;
    stream << "  subnetNodes={";
    for (NodeMap::iterator it = nodes.begin(); it != nodes.end(); ++it) {
        stream << std::endl << "    " << Node::IndentString(it->second);
    }
    stream << std::endl;
    stream << "  }";
    stream << std::endl;
    stream << "  Paths={";
    for (PathMap::iterator it = paths.begin(); it != paths.end(); ++it) {
        stream << std::endl;
        stream << "    " << it->second;
    }
    stream << std::endl;
    stream << "  }";
    stream << std::endl;
    stream << "}";
}

void SubnetTopology::toString(std::ostringstream &stream) {
    stream << "SubnetTopology {";
    stream << "subnetId=" << subnetId;
    stream << ", subnetNodes={";
    for (NodeMap::iterator it = nodes.begin(); it != nodes.end(); ++it) {
        stream << "    " << it->second;
    }
    stream << "}";
    stream << ", routes=";
    stream << "}";

}
