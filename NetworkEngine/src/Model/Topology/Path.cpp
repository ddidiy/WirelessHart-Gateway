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

#include "Path.h"
#include "Model/NetworkEngine.h"
#include "Common/NETypes.h"
#include <assert.h>

using namespace NE::Model::Topology;

Path::Path(Uint16 graphId_, Address32 source_, Address32 destination_, RoutingTypes::RoutingTypes_Enum type_,
            Uint16 traffic_) :
    graphId(graphId_), source(source_), destination(destination_), type(type_), traffic(traffic_) {

    reevaluate = false;

    onEvaluation = false;

    if (source == MANAGER_ADDRESS || source == GATEWAY_ADDRESS) {
        evaluatePathPriority = EvaluatePathPriority::OutboundAttach;
    } else {
        evaluatePathPriority = EvaluatePathPriority::InboundAttach;
    }

    status = Status::NEW;

    //TODO: ivp
    maxHops = 64;

    allocationHandler = graphId;

    useRetryOnPreffered = (bool) NE::Model::NetworkEngine::instance().getSettingsLogic().useRetryOnPreffered;

    lastEvalTime = time(NULL);

    // LOG_DEBUG("create Path: graphId=" << (int)graphId);
}

Path::~Path() {
    // LOG_DEBUG("destroy Path: graphId=" << (int)graphId);
}

RoutingTypes::RoutingTypes_Enum Path::getType() {
    return type;
}

bool Path::isSourcePath() {
    return (type == RoutingTypes::SOURCE_ROUTING);
}

bool Path::isReevaluate() {
    return reevaluate;
}

bool Path::isOnEvaluation() {
    return onEvaluation;
}

void Path::updateLastEvalTime() {
    bool found = NetworkEngine::instance().deleteOldEntryInPriorityPathSet(this);
    lastEvalTime = time(NULL);
    if (found) {
        NetworkEngine::instance().insertNewEntryInPriorityPathSet(this);
    }
}

time_t Path::getLastEvalTime() {
    return lastEvalTime;
}

void Path::setOnEvaluation(bool onEvaluation_) {
    LOG_INFO("setOnEvaluation - graphId= " << ToStr(graphId) << ", old onEvaluation=" << (int) onEvaluation
                << ", new onEvaluation=" << (int) onEvaluation_);
    onEvaluation = onEvaluation_;
}

EvaluatePathPriority::EvaluatePathPriority Path::getEvaluatePathPriority() {
    return evaluatePathPriority;
}

void Path::setReevaluate(bool _reevaluate, EvaluatePathPriority::EvaluatePathPriority evaluatePathPriority_) {
    LOG_INFO("setReevaluate - current reevaluate = " << reevaluate << ", new reevaluate=" << _reevaluate
                << ", onEvaluation=" << (int) onEvaluation << ", current priority=" << EvaluatePathPriority::toString(
                            evaluatePathPriority) << ", new priority=" << EvaluatePathPriority::toString(evaluatePathPriority_)
                << ", route=" << *this);

    //if the old reevaluate is not set, set the new reevaluate
    //if the old reevaluate is set, accept the new one if it is more prioritary
    if (evaluatePathPriority > evaluatePathPriority_ && reevaluate) {
        LOG_INFO("setReevaluate - currently onEvaluation, SKIP = ");
        return;
    }

    bool insert = (!reevaluate && _reevaluate);
    bool del = (reevaluate && !_reevaluate);
//    bool insert = true;
//    bool del = true;

    reevaluate = _reevaluate;

    bool found = false;
    if (evaluatePathPriority != evaluatePathPriority_ || del) {
        found = NetworkEngine::instance().deleteOldEntryInPriorityPathSet(this);
    }

    evaluatePathPriority = evaluatePathPriority_;

    if ((found && !del) || insert) {
        NetworkEngine::instance().insertNewEntryInPriorityPathSet(this);
    }
}

Uint32 Path::getCost() {
    return cost;
}

void Path::setCost(Uint32 cost_) {
    cost = cost_;
}

std::string Path::getTypeDescription() {
    if (RoutingTypes::GRAPH_ROUTING == type) {
        return "GraphRouting";
    } else if (RoutingTypes::SOURCE_ROUTING == type) {
        return "SourceRouting";
    } else if (RoutingTypes::BROADCAST_ROUTING == type) {
        return "BrodcastRouting";
    }

    return "Unknown routing!";
}

Uint16 Path::getGraphId() {
    return graphId;
}

Address32 Path::getSource() {
    return source;
}

Address32 Path::getDestination() {
    return destination;
}

Uint16 Path::getTraffic() {
    return traffic;
}

void Path::setTraffic(Uint16 traffic_) {
    this->traffic = traffic_;
    LOG_DEBUG("setTraffic - " << this->traffic << ", route:" << *this);
}

Uint8 Path::getRedundantFactor() {
    return redundantFactor;
}

void Path::setRedundantFactor(Uint8 redundantFactor) {
    this->redundantFactor = redundantFactor;
}

Uint8 Path::getMaxHops() {
    return maxHops;
}

void Path::setMaxHops(Uint8 _maxHops) {

    //TODO: ivp
    maxHops = 64;
    // this->maxHops = maxHops;
}

AddressList& Path::getSourcePath() {
    return sourcePath;
}

Status::StatusEnum Path::getStatus() {
    return status;
}

void Path::setStatus(Status::StatusEnum status_) {
    status = status_;
}

void Path::setSourcePath(AddressList& addressList) {
    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();

    AddressList::iterator itDel = sourcePath.begin();
    for (AddressList::iterator it = ++sourcePath.begin(); it != sourcePath.end(); ++it) {
        Node& crtNode = subnetTopology.getNode(*itDel);
        crtNode.removeSourceNeighbor(graphId, *it);
        itDel = it;
    }

    sourcePath = addressList;

    AddressList::iterator itAdd = addressList.begin();
    for (AddressList::iterator it = ++addressList.begin(); it != sourcePath.end(); ++it) {
        Node& crtNode = subnetTopology.getNode(*itAdd);
        crtNode.addSourceNeighbor(graphId, traffic, *it);
        itAdd = it;
    }
}

bool Path::checkGraphPath(EngineOperations& engineOperations, Uint16 searchId) {
    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();
    Node& node = subnetTopology.getNode(source);
    bool isGood = false;
    if (node.getStatus() != Status::REMOVED) {
        bool interrupted = !markReachableOnPath(source, searchId);
        isGood = checkGraphPathSearch(engineOperations, source, true, searchId, interrupted);

        LOG_DEBUG("interrupted= "<<interrupted<<"; isGood= "<<isGood);
    }

    LOG_DEBUG("checkGraphPath : graphId=" << ToStr(graphId) << " check result=" << (int) isGood);

    return isGood;
}

//check if nodes are reachable in path - ONLY UNREACHABLE NODES NEED TO BE REMOVED FROM GRAPH
//if nodes are reachable set current searchId
//returns true if destination is reachable (not interrupted graph)
bool Path::markReachableOnPath(Address32 address, Uint16 searchId) {
    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();
    Node& node = subnetTopology.getNode(address);
    bool result = false;

    if (node.getStatus() == Status::REMOVED || node.getSearchId() == searchId) {
        return result;
    }
    else if (address == destination) {
        LOG_DEBUG("markReachableOnGraph - mark node: " << ToStr(address));
        node.setSearchId(searchId);
        return true;
    }
    else {
        LOG_DEBUG("markReachableOnGraph - mark node: " << ToStr(address));
        node.setSearchId(searchId);
    }

    EdgeList& edges = node.getOutBoundEdges();
    for (EdgeList::iterator it = edges.begin(); it != edges.end(); ++it) {
        if (it->getStatus() == Status::DELETED) {
            continue;
        }
        if (it->existsGraphNeighbor(graphId)) {
            result = result || markReachableOnPath(it->getDestination(), searchId);
        }
    }
    return result;
}

bool Path::checkGraphPathSearch(EngineOperations& engineOperations, Address32 address, bool active, Uint16 searchId, bool interrupted) {
    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();
    Node& node = subnetTopology.getNode(address);
    node.setSearchId(searchId);

    bool result = false;
    bool edgeResult = false;

    active = (node.getStatus() != Status::REMOVED) && active;

    if (address == destination) {
        return active;
    }

    if (!node.existsGraphNeighbor(graphId)) {
        return result;
    }

    EdgeList& edges = node.getOutBoundEdges();

    for (EdgeList::iterator it = edges.begin(); it != edges.end(); ++it) {
        if (it->getStatus() == Status::DELETED) {
            continue;
        }

        //LOG_DEBUG("checkGraphPathSearch - edge: " << *it);

        if (it->existsGraphNeighbor(graphId)) {
            edgeResult = checkGraphPathSearch(engineOperations, it->getDestination(), active, searchId, interrupted);
            result = result || edgeResult;

            //check graph neighbor status to protect against duplicated operations
            if (!edgeResult && it->getGraphNeighborStatus(graphId) != Status::DELETED) {
                //TODO: ivp - update link status, generate link delete operations?

                //removed neighbor on graph if node is unreachable on graph or if its neighbor is removed
                Node& peerNode = subnetTopology.getNode(it->getDestination());
                if (node.getStatus() != Status::REMOVED && (node.getSearchId() != searchId
                            || peerNode.getStatus() == Status::REMOVED || interrupted)) {

                    it->setRemoveGraphNeighbor(graphId);

                    IEngineOperationPointer operation(
                                new NeighborGraphRemovedOperation(it->getSource(), graphId, it->getDestination()));

                    operation->setDependency(WaveDependency::NINETH);
                    engineOperations.addOperation(operation);

                    if (node.getPrimaryInbound() == it->getDestination() || node.getSecondaryClkSource() == it->getDestination()) {
                        SetClockSourceOperation *op = new SetClockSourceOperation(it->getSource(), it->getDestination(), 0);
                        IEngineOperationPointer engineOp(op);
                        engineOp->setDependency(WaveDependency::ELEVENTH);
                        engineOperations.addOperation(engineOp);

                        if (node.getSecondaryClkSource() == it->getDestination()) {
                            node.resetSecondaryClkSource();
                        }
                    }
                }
            }
        }
    }

    return result;
}

bool Path::checkSourcePath() {
    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();

    bool isBroken;

    isBroken = false;

    for (AddressList::iterator it = sourcePath.begin(); it != sourcePath.end(); ++it) {
        Node& crtNode = subnetTopology.getNode(*it);
        if (crtNode.getStatus() == Status::REMOVED) {
            isBroken = true;
            break;
        }
    }

    if (isBroken && sourcePath.size() > 1) {
        AddressList::iterator itNext = ++sourcePath.begin();
        for (AddressList::iterator it = sourcePath.begin(); itNext != sourcePath.end(); ++it, ++itNext) {
            Node& crtNode = subnetTopology.getNode(*it);

            crtNode.removeSourceNeighbor(graphId, *itNext);
        }
    }

    LOG_DEBUG("checkSourcePath : graphId=" << (int) graphId << " check result=" << (int) !isBroken);

    return !isBroken;
}

bool Path::containsEdge(Uint16 src, Uint16 dest) {
    NE::Model::Tdma::LinkEdgesList linkEdges;
    populateLinkEdges(linkEdges, false); //TODO check if isEval is ok on false

    for (NE::Model::Tdma::LinkEdgesList::iterator it = linkEdges.begin(); it != linkEdges.end(); ++it) {
        if ((it->getSource() == src && it->getDestination() == dest) || (it->getSource() == dest
                    && it->getDestination() == src))

            return true;
    }

    return false;
}

void Path::populateLinkEdges(NE::Model::Tdma::LinkEdgesList& linkEdges, bool isEval) {
    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();

    if (subnetTopology.getNode(source).getNodeType() != NodeType::NODE
                && subnetTopology.getNode(destination).getNodeType() != NodeType::NODE) {
        return;
    }

    if (isSourcePath()) {
        AddressList::iterator itNext = ++sourcePath.begin();
        for (AddressList::iterator it = sourcePath.begin(); itNext != sourcePath.end(); ++it, ++itNext) {
            if (*it != GATEWAY_ADDRESS && *it != MANAGER_ADDRESS && *itNext != GATEWAY_ADDRESS && *itNext
                        != MANAGER_ADDRESS) {
                NE::Model::Tdma::LinkEdge linkEdge(*it, *itNext, useRetryOnPreffered);
                linkEdges.push_back(linkEdge);
            }
        }
    } else {
        if (isEval) {
            populateEvalLinkEdgesSearch(linkEdges);
        } else {
            populateLinkEdgesSearch(linkEdges);
        }
    }

}

void Path::populateLinkEdgesSearch(NE::Model::Tdma::LinkEdgesList& linkEdges) {
    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();
    Address32 address;
    AddressQueue queue;
    queue.push(source);

    std::set<Address32> nodes;

    LOG_DEBUG("populateLinkEdgesSearch - start - graphId=" << ToStr(graphId));

    while (!queue.empty()) {
        address = queue.front();
        queue.pop();

        if (nodes.find(address) != nodes.end())
        {
            LOG_INFO("populateLinkEdgesSearch: avoiding cycle.");
            continue;
        }
        nodes.insert(address);

        Node& node = subnetTopology.getNode(address);


        //LOG_DEBUG("populateLinkEdgesSearch - start - node=" << ToStr(address));

        EdgeList& edges = node.getOutBoundEdges();
        for (EdgeList::iterator it = edges.begin(); it != edges.end(); ++it) {
            if (it->getStatus() == Status::DELETED) {
                continue;
            }

            //            LOG_DEBUG("populateLinkEdgesSearch - check edge (" << ToStr(it->getSource()) << ", " << ToStr(
            //                        it->getDestination()) << ")");

            if (!it->existsGraphNeighbor(graphId)) {
                //LOG_DEBUG("populateLinkEdgesSearch - doesn't have graph");
                continue;
            }

            Node& peer = subnetTopology.getNode(it->getDestination());

            if (node.getNodeType() == NodeType::NODE || peer.getNodeType() == NodeType::NODE) {

                if (it->isPrefferedOnGraph(graphId)) {
                    //bool isRetry = it->isPrefferedOnGraph(graphId) && useRetryOnPreffered;
                    bool isRetry = it->isRetryOnGraph(graphId); //|| !node.hasOtherOutboundNeighborOnGraph(it->getDestination(), graphId);

                    LinkEdge linkEdge(it->getSource(), it->getDestination(), isRetry);
                    linkEdges.push_back(linkEdge);
                }
                //                LOG_DEBUG("populateLinkEdgesSearch - add edge (" << ToStr(it->getSource()) << ", " << ToStr(
                //                            it->getDestination()) << ")");
            }

            if (it->isLazyParent(graphId)) {
                //                LOG_DEBUG("populateLinkEdgesSearch - put node in queue: " << ToStr(it->getDestination()));
                queue.push(it->getDestination());
            }
        }

        //        LOG_DEBUG("populateLinkEdgesSearch - end - node=" << ToStr(address));

        for (EdgeList::iterator it = edges.begin(); it != edges.end(); ++it) {
            if (it->getStatus() == Status::DELETED) {
                continue;
            }

            if (!it->existsGraphNeighbor(graphId)) {
                continue;
            }

            Node& peer = subnetTopology.getNode(it->getDestination());

            if (node.getNodeType() == NodeType::NODE || peer.getNodeType() == NodeType::NODE) {
                if (!it->isPrefferedOnGraph(graphId)) {
                    LinkEdge linkEdge(it->getSource(), it->getDestination(), false);
                    linkEdges.push_back(linkEdge);
                }
            }
        }

    }
}

void Path::populateEvalLinkEdgesSearch(NE::Model::Tdma::LinkEdgesList& linkEdges) {
    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();
    Uint16 searchId = subnetTopology.setNextSearchId();
    Address32 address;
    AddressQueue queue;
    queue.push(source);

    std::set<Address32> nodes;

    LOG_DEBUG("populateEvalLinkEdgesSearch - start - graphId=" << ToStr(graphId));

    while (!queue.empty()) {
        address = queue.front();
        queue.pop();

        if (nodes.find(address) != nodes.end())
        {
            LOG_INFO("populateEvalLinkEdgesSearch: avoiding cycle.");
            continue;
        }

        nodes.insert(address);

        Node& node = subnetTopology.getNode(address);

        EdgeList& edges = node.getOutBoundEdges();
        for (EdgeList::iterator it = edges.begin(); it != edges.end(); ++it) {
            if (it->getStatus() == Status::DELETED) {
                continue;
            }

            LOG_DEBUG("populateEvalLinkEdgesSearch - check edge (" << ToStr(it->getSource()) << ", " << ToStr(
                                    it->getDestination()) << ")");

            LOG_DEBUG("populateEvalLinkEdgesSearch - first edge:" << *it);

            if (!it->existsGraphNeighbor(graphId)) {
                LOG_DEBUG("populateEvalLinkEdgesSearch - doesn't have graph");
                continue;
            }

            Node& peer = subnetTopology.getNode(it->getDestination());

            GraphNeighbor& graphNeighbor = node.updateLazyParent(peer.getNodeAddress(), peer.getEvalLazyParent(),
                        graphId);

            if (node.getNodeType() == NodeType::NODE || peer.getNodeType() == NodeType::NODE) {

                if (graphNeighbor.preffered) {
                    bool isRetry = useRetryOnPreffered;

                    graphNeighbor.retry = isRetry;

                    LinkEdge linkEdge(address, peer.getNodeAddress(), isRetry);
                    linkEdges.push_back(linkEdge);

                    LOG_DEBUG("populateEvalLinkEdgesSearch - add edge (" << ToStr(address) << ", " << ToStr(
                                            peer.getNodeAddress()) << ")");
                }
            }

            if (graphNeighbor.lazy) {
                LOG_DEBUG("populateEvalLinkEdgesSearch - put node in queue: " << ToStr(peer.getNodeAddress()));

                if (peer.getSearchId() == searchId) {
                    LOG_ERROR("populateEvalLinkEdgesSearch - put duplicate node in queue: " << ToStr(
                                            peer.getNodeAddress()));
                } else {
                    queue.push(peer.getNodeAddress());
                    peer.setSearchId(searchId);
                }
            }
        }

        for (EdgeList::iterator it = edges.begin(); it != edges.end(); ++it) {
            if (it->getStatus() == Status::DELETED) {
                continue;
            }

            LOG_DEBUG("populateEvalLinkEdgesSearch - second edge:" << *it);

            if (!it->existsGraphNeighbor(graphId)) {
                LOG_DEBUG("populateEvalLinkEdgesSearch - doesn't have graph");
                continue;
            }

            Node& peer = subnetTopology.getNode(it->getDestination());

            if (node.getNodeType() == NodeType::NODE || peer.getNodeType() == NodeType::NODE) {
                if (!it->isPrefferedOnGraph(graphId)) {
                    LinkEdge linkEdge(address, peer.getNodeAddress(), false);
                    linkEdges.push_back(linkEdge);

                    LOG_DEBUG("populateEvalLinkEdgesSearch - add edge (" << ToStr(address) << ", " << ToStr(
                                            peer.getNodeAddress()) << ")");
                }
            }
        }
    }
}

void Path::populateJoinLinkEdge(NE::Model::Tdma::LinkEdgesList& linkEdges, bool isInbound) {
    LOG_DEBUG("populateJoinLinkEdge - source=" << ToStr(source) << " , destination=" << ToStr(destination)
                << ", isInbound=" << (int) isInbound);

    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();

    Address32 nodeAddress = source;
    Address32 parentAddress;

    if (source == MANAGER_ADDRESS || source == GATEWAY_ADDRESS) {
        nodeAddress = destination;
    }

    Node& node = subnetTopology.getNode(nodeAddress);
    if (node.getNodeType() != NodeType::NODE) {
        return;
    }

    parentAddress = node.getParentAddress();

    if (node.getNodeType() != NodeType::NODE) {
        return;
    }

    if (!isInbound) {
        parentAddress = nodeAddress;
        nodeAddress = node.getParentAddress();
    }

    LinkEdge linkEdge(nodeAddress, parentAddress, false);

    linkEdges.push_back(linkEdge);
}

void Path::determineGraphsEdges(Address32 address, std::ostream& stream, Uint16 searchId) const {
    if (destination == address) {
        return;
    }

    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();

    if (!subnetTopology.existsNode(address)) {
        LOG_ERROR("determineGraphsEdges - in graphId=" << ToStr(graphId) << ", the address doesn't exist: " << ToStr(
                                address));
        return;
    }

    Node& node = subnetTopology.getNode(address);

    if (node.getSearchId() == searchId) {
        return;
    } else {
        node.setSearchId(searchId);
    }

    EdgeList& edges = node.getOutBoundEdges();
    for (EdgeList::iterator it = edges.begin(); it != edges.end(); ++it) {
        if (it->getStatus() == Status::DELETED) {
            continue;
        }

        if (it->existsGraphNeighbor(graphId)) {

            if (it->getSource() != GATEWAY_ADDRESS && it->getSource() != MANAGER_ADDRESS && it->getDestination()
                        != GATEWAY_ADDRESS && it->getDestination() != MANAGER_ADDRESS) {

                it->toShortString(graphId, stream);
                stream << " ";
            }

            determineGraphsEdges(it->getDestination(), stream, searchId);
        }

    }
}

void Path::initDestinations(Address32 address, AddressList& destinations) {
    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();
    Uint16 searchId = subnetTopology.setNextSearchId();

    initDestinationsSearch(source, address, destinations, searchId);
}

void Path::initDestinationsSearch(Address32 crtAddress, Address32 endAddress, AddressList& destinations,
            Uint16 searchId) {
    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();

    LOG_DEBUG("initDestinationsSearch - address " << ToStr(crtAddress));

    if (!subnetTopology.existsNode(crtAddress)) {
        LOG_ERROR("initDestinationsSearch - in graphId=" << ToStr(graphId) << ", the address doesn't exist: " << ToStr(
                                crtAddress));
        return;
    }

    Node& node = subnetTopology.getNode(crtAddress);

    if (node.getSearchId() == searchId) {
        return;
    } else {
        node.setSearchId(searchId);
    }

    EdgeList& edges = node.getOutBoundEdges();
    for (EdgeList::iterator it = edges.begin(); it != edges.end(); ++it) {
        if (it->getStatus() == Status::DELETED) {
            continue;
        }

        if (it->existsGraphNeighbor(graphId)) {
            GraphNeighbor& graphNeighbor = it->getGraphOnEdge(graphId);

            if (graphNeighbor.preffered) {
                if (it->getDestination() != MANAGER_ADDRESS && it->getDestination() != GATEWAY_ADDRESS) {
                    destinations.push_back(it->getDestination());

                    LOG_DEBUG("initDestinationsSearch - added " << ToStr(it->getDestination()));
                }

                if (it->getDestination() != endAddress) {
                    initDestinationsSearch(it->getDestination(), endAddress, destinations, searchId);
                }
            }
        }
    }
}

//updates connections map - depends only on the topology of the graph. Need to be called any time graph is modified.
void Path::populateConnectionsMap(Address32 address, Uint16 searchId) {

    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();
    Node& node = subnetTopology.getNode(address);
    EdgeList& edges = node.getOutBoundEdges();
    EdgeList::iterator it = edges.begin();

    if (node.getSearchId() != searchId) {
        pathNodesList.insert(pathNodesList.end(), address);
        outcomingConnections.insert (std::pair< Address32, EdgePointerVector >(address, EdgePointerVector()));
        node.setSearchId(searchId);
    } else {
        return;
    }

    if ( address == destination) {
        return;
    }

    for (; it != edges.end(); ++it) {
        if ( it->existsGraphNeighbor(graphId) ) {
            Address32 peerAddress = it->getDestination();
            ConnectionsMap::iterator it_in = incomingConnections.find(peerAddress);
            LOG_DEBUG ("Found incoming for " << std::hex << peerAddress << ": " << address);
            if (it_in == incomingConnections.end()) {
                std::pair<ConnectionsMap::iterator, bool> rez =
                    incomingConnections.insert (std::pair< Address32, EdgePointerVector >(peerAddress, EdgePointerVector()));

                //rez.first->second.push_back(address);
                rez.first->second.push_back(EdgePointer(*it, node));
            }
            else {
                //it_in->second.push_back(address);
                it_in->second.push_back(EdgePointer(*it, node));
            }
            ConnectionsMap::iterator it_out = outcomingConnections.find(address);
            //it_out->second.push_back(peerAddress);
            Node& peerNode = subnetTopology.getNode(peerAddress);
            it_out->second.push_back(EdgePointer(*it, peerNode));
            populateConnectionsMap(peerAddress, searchId);
        }
    }
}

//Ordered the nodes in topological order and depends only on the topology of the graph. Need to be called any time graph is modified.
void Path::createTopologicalOrder() {

    std::ostringstream stream, nodes_stream;
    stream << "Topological order of " << std::hex << graphId << ": ";
    nodes_stream << "Nodes of " << std::hex << graphId << ": ";


    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();

    //populate incoming/outocming connections of the graph
    incomingConnections.clear();
    outcomingConnections.clear();
    pathNodesList.clear();
    orderedNodes.clear();
    populateConnectionsMap(source, subnetTopology.setNextSearchId());
    incomingConnections.insert (std::pair< Address32, EdgePointerVector >(source, EdgePointerVector()));

    //init incoming connections counters
    std::map<Address32, int> inCounter;
    std::map<Address32, int>::iterator it_counter;
    ConnectionsMap::iterator it_in = incomingConnections.begin();
    for (; it_in != incomingConnections.end(); ++it_in) {
        inCounter.insert(std::pair<Address32, Uint8>(it_in->first, it_in->second.size()));
        nodes_stream << it_in->first << ":" << it_in->second.size() << "      ";
    }

    //until all nodes are in topological order (if cycle crash)
    while ( !pathNodesList.empty() ) {

        std::list<Address32>::iterator it = pathNodesList.begin();

        //search for a node without incoming connections
        for (; it != pathNodesList.end(); ++it) {

            //get node incoming connections counter
            it_counter = inCounter.find(*it);

            //check if counter reaches zero
            if ( it_counter->second == 0 ) {

                //add node in ordered nodes
                orderedNodes.push_back(NodePointer(subnetTopology.getNode(*it)));
                stream << *it << "     ";

                //update incoming connections counters
                ConnectionsMap::iterator it_out = outcomingConnections.find(*it);
                EdgePointerVector::iterator it_neighbors = it_out->second.begin();

                for (; it_neighbors != it_out->second.end(); ++it_neighbors) {
                    //it_counter = inCounter.find(*it_neighbors);
                    it_counter = inCounter.find(it_neighbors->dest);
                    it_counter->second--;
                }

                //remove node from the unordered list
                pathNodesList.erase(it);

                break;
            }
        }
    }
    LOG_DEBUG(nodes_stream.str());
    LOG_DEBUG(stream.str());

}

//determine the minimum cost simple path in graph. Need to be call for every evaluation of the graph because the cost
//can be changed.
//if createTopOrder is set the nodes of the path are topologically sorted first
//Algorithm: Shortest Path in DAGs with topological ordering
Uint32 Path::getMinCostPath(AddressList& shortPathAddr, bool createTopOrder, Address32 testAddress) {

    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();

    if (createTopOrder /*|| onEvaluation*/) {
        createTopologicalOrder();
    }

    //test for primary inbound cycle
    if (destination == MANAGER_ADDRESS && testAddress == source) {
        LOG_DEBUG("Clk source cycle found");
        return 0xFFFFFFFF;
    }

    //init cost vector and positions
    Uint32 costs[orderedNodes.size()];
    std::map<Address32, Uint8> positions;
    Uint16 predecesors[orderedNodes.size()];
    std::vector<NodePointer>::iterator it = orderedNodes.begin();
    positions.insert( std::pair<Address32, Uint16>(it->address, 0) );
    it++;
    Uint16 count = 0;
    costs[count++] = 0;

    for (; it != orderedNodes.end(); ++it) {
        positions.insert( std::pair<Address32, Uint16>(it->address, count) );
        predecesors[count] = 0;
        costs[count++] = 0xFFFFFFFF;
    }
    Uint16 nodePos = 0;

    //for node in topological ordered
    std::vector<NodePointer>::iterator it_end = orderedNodes.end();
    it_end--;
    for (it = orderedNodes.begin(); it != it_end; ++it) {

        //TODO: cache pointer to node in orderNodes vector
        Node& node = *(it->node);

        EdgeList& edges = node.getOutBoundEdges();
        EdgeList::iterator it_edge = edges.begin();

        bool edgeFound = false;
        //for all node neighbors
        ConnectionsMap::iterator it_node = outcomingConnections.find(it->address);

        for (EdgePointerVector::iterator it_neigh = it_node->second.begin();
                    it_neigh != it_node->second.end(); ++it_neigh) {
            try {
                    Edge &edge = node.getEdge(it_neigh->dest);//*(it_neigh->edge);//

                    Uint16 peerNodeCost = (it_neigh->peerNode)->getSettingsCost();//subnetTopology.getNode(edge.getDestination()).getSettingsCost();
                    Uint16 edgeCost = edge.evaluateEdgeCost(graphId, traffic, node, *(it_neigh->peerNode), *this);

                    //relax here
                    std::map<Address32, Uint8>::iterator it_pos = positions.find(edge.getDestination());
                    if (it_pos == positions.end()) {
                        LOG_ERROR("Neighbor not found.");
                        continue;
                    }

                    Uint16 neighborPos = it_pos->second;
                    LOG_DEBUG("Pos: " << ToStr(nodePos) << " - " << ToStr(neighborPos));

                    if (edgeCost == 0xffff
                                || peerNodeCost == 0xffff
                                        || costs[nodePos] == 0xffffffff) {
                        continue;
                    }
                    edgeFound = true;

                    if ( costs[neighborPos] > costs[nodePos] + edgeCost + peerNodeCost ){
                        costs[neighborPos] = costs[nodePos] + edgeCost + peerNodeCost;
                        predecesors[neighborPos] = nodePos;
                    }
                } catch (std::exception& ex) {
                    LOG_WARN("getMinCostPath: edge from " << ToStr(node.getNodeAddress()) <<
                                " to " << ToStr(it_neigh->dest) << " not found. ex=" << ex.what());
                }
            }


        //if no edge found, it is possible that the path is under evaluation or other problem
        //so, skip it
        if (!edgeFound) {
            LOG_ERROR("Path inconsistency");
            return  0xFFFFFFFF;
        }

        nodePos++;
    }

    std::ostringstream stream;
    std::ostringstream pstream;
    std::ostringstream cstream;

    pstream << "Predecesors: " << std::dec;
    cstream << "Costs: " << std::dec;
    for (unsigned int i=0; i<orderedNodes.size(); i++) {
        pstream << "    " << predecesors[i];
        cstream << "    " << costs[i];
    }

    LOG_DEBUG(pstream.str());
    LOG_DEBUG(cstream.str());

    stream << "Shortest path found: " << std::hex;

    //Construct the minimum cost path
    bool cycleFound = false;
    Uint32 minCost = 0;
    Uint8 pos = positions.find(destination)->second;
    Address32 addr = destination;
    shortPathAddr.push_back(addr);
    stream << addr << "     ";
    do {

        if (minCost != 0xFFFFFFFF && costs[pos] != 0xFFFFFFFF) {
            minCost += costs[pos];
        }

        pos = predecesors[pos];
        addr = orderedNodes[pos].address;

        if (addr == testAddress) {
            cycleFound = true;
        }

        shortPathAddr.push_back(addr);
        stream << addr << "     ";
    } while (addr != source);

    if (cycleFound) {
        LOG_DEBUG("Cycle found: testAddress=" << std::hex << testAddress);
        return 0xFFFFFFFF;
    }

    if (minCost>0xFFFF) {
        minCost = 0xFFFFFFFF;
    }

    stream << "Cost: " << std::dec << minCost;
    LOG_DEBUG (stream.str());


    return minCost;
}

std::ostream& NE::Model::Topology::operator<<(std::ostream& stream, Path& path) {
    std::string addressString;

    stream << "Path {" << "graphId=" << ToStr(path.graphId) << ", " << ToStr(path.source);
    stream << " => " << ToStr(path.destination);
    stream << ", traffic=" << path.traffic;
    stream << ", " << RoutingTypes::toString(path.type);
    stream << ", " << Status::getStatusDescription(path.status);
    stream << ", reevaluate=" << path.reevaluate;
    stream << ", onEvaluation=" << path.onEvaluation;
    stream << ", priority=" << EvaluatePathPriority::toString(path.evaluatePathPriority);

    if (path.type == RoutingTypes::SOURCE_ROUTING) {
        stream << ", sourcePath=";
        for (AddressList::iterator it = path.sourcePath.begin(); it != path.sourcePath.end(); ++it) {
            stream << ToStr(*it);
            stream << ", ";
        }
    } else {
        stream << ", graph=";
        SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();

        path.determineGraphsEdges(path.source, stream, subnetTopology.setNextSearchId());
    }
    stream << "}";

    return stream;
}

