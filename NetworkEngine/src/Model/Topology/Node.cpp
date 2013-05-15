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

#include "Node.h"
#include "Common/NETypes.h"
#include "Model/MetaDataAttributes.h"
#include "Model/NetworkEngine.h"
#include "Model/Operations/Topology/SetClockSourceOperation.h"

using namespace NE::Model::Topology;
using namespace NE::Model::Operations;
using namespace NE::Model;

Node::Node(Address32 _nodeAddress, Address32 _parentAddress, Address32 _backboneAddress,
            NodeType::NodeTypeEnum _nodeType) :
    nodeAddress(_nodeAddress), parentAddress(_parentAddress), backboneAddress(_backboneAddress), nodeType(_nodeType) {

    LOG_DEBUG("create Node: " << ToStr(nodeAddress) << ", backboneAddress: " << ToStr(_backboneAddress));

    inboundGraphId = 0;
    primaryOutbound = 0;
    secondaryOutbound = 0;

    primaryInbound = 0;
    secondaryInbound = 0;
    outboundGraphId = 0;

    router = false;

    status = Status::NEW;
    searchId = 0;
    evalSearchId = 0;
    settingsCost = 0;
    secondaryClkSource = 0x00000000;

    evalLazyParent = _nodeAddress;
    deep = 0;
    resetInboundOldies();
    resetOutboundOldies();
    activeEdgesNo = 0;
}

Node::~Node() {
}

Address32 Node::getNodeAddress() {
    return nodeAddress;
}

Address32 Node::getParentAddress() {
    return parentAddress;
}

Address32 Node::getBackboneAddress() {
    return backboneAddress;
}

MetaDataAttributesPointer Node::getMetaDataAttributes() {
    return metaDataAttributes;
}

void Node::setMetaDataAttributes(MetaDataAttributesPointer _metaDataAttributes) {
    this->metaDataAttributes = _metaDataAttributes;
}

Uint16 Node::getInboundGraphId() {
    return inboundGraphId;
}

void Node::setInboundGraphId(Uint16 inboundGraphId_) {
    inboundGraphId = inboundGraphId_;

    LOG_DEBUG("setInboundGraphId  node=" << ToStr(nodeAddress) << ", graphId=" << ToStr(inboundGraphId));
}

Uint16 Node::getOutboundGraphId() {
    return outboundGraphId;
}

void Node::setOutboundGraphId(Uint16 outboundGraphId_) {
    outboundGraphId = outboundGraphId_;

    LOG_DEBUG("setOutboundGraphId  node=" << ToStr(nodeAddress) << ", graphId=" << ToStr(outboundGraphId));
}

Address32 Node::getPrimaryOutbound() {
    return primaryOutbound;
}

void Node::setPrimaryOutbound(Address32 primaryOutbound_) {
    oldPOutbound = primaryOutbound;
    primaryOutbound = primaryOutbound_;
}

Address32 Node::getSecondaryOutbound() {
    return secondaryOutbound;
}

void Node::setSecondaryOutbound(Address32 secondaryOutbound_) {
    oldSOutbound = secondaryOutbound;
    secondaryOutbound = secondaryOutbound_;
}

Address32 Node::getPrimaryInbound() {
    return primaryInbound;
}

void Node::setPrimaryInbound(Address32 primaryInbound_) {
    oldPInbound = primaryInbound;
    primaryInbound = primaryInbound_;
}

Address32 Node::getSecondaryInbound() {
    return secondaryInbound;
}

void Node::setSecondaryInbound(Address32 secondaryInbound_) {
    oldSInbound = secondaryInbound;
    secondaryInbound = secondaryInbound_;
}

void Node::goBackToOldInbound(Address32 address) {

    if (address == primaryInbound) {
        primaryInbound = oldPInbound;
    }
    if (address == secondaryOutbound) {
        secondaryInbound = oldSInbound;
    }
}

void Node::goBackToOldOutbound(Address32 address) {
    if (address == primaryOutbound) {
        primaryOutbound = oldPOutbound;
    }
    if (address == secondaryOutbound) {
        secondaryOutbound = oldSOutbound;
    }
}

void Node::resetOutboundOldies() {
    oldPOutbound = 0;
    oldSOutbound = 0;
}
void Node::resetInboundOldies() {
    oldPInbound = 0;
    oldSInbound = 0;
}

EdgeList& Node::getOutBoundEdges() {
    return outBoundEdges;
}

Uint16 Node::getSearchId() {
    return searchId;
}

void Node::setSearchId(Uint16 searchId) {
    this->searchId = searchId;
}

Status::StatusEnum Node::getStatus() {
    return status;
}

void Node::setStatus(Status::StatusEnum status_) {
    status = status_;
}

NodeType::NodeTypeEnum Node::getNodeType() {
    return nodeType;
}

bool Node::isRouter() {
    return router;
}

void Node::setRouter(bool router_) {
    router = router_;
}

bool Node::operator<(const Node &compare) {

    return nodeAddress < compare.nodeAddress;
}

bool Node::operator==(const Node &node) {

    return nodeAddress == node.nodeAddress;
}

bool Node::isSourceClock(Address32 destination) {
    LOG_DEBUG("isSourceClock  crt=" << ToStr(nodeAddress) << ", peer=" << ToStr(destination) << ", graphId=" << ToStr(
                inboundGraphId));

    if (parentAddress == destination) {
        LOG_DEBUG("isSourceClock1 - true");
        return true;
    }

    LOG_DEBUG("isSourceClock - false");
    return false;
}

bool Node::hasSecondaryClkSource() {
    LOG_DEBUG("hasSecondaryClkSource() secondaryClkSource: " << ToStr(secondaryClkSource) << ": "//
                << (!(0x0000 == secondaryClkSource)));
    return !(0x00000000 == secondaryClkSource);
}

void Node::setSecondaryClkSource(Address32 addr) {
    LOG_DEBUG("setSecondaryClkSource - SET " << ToStr(nodeAddress) << " <-- " << ToStr(addr));
    secondaryClkSource = addr;
}

void Node::resetSecondaryClkSource() {
    LOG_DEBUG("setSecondaryClkSource - RESET " << ToStr(nodeAddress) << " <-- " << ToStr(secondaryClkSource));
    secondaryClkSource = 0x00000000;
}

Address32 Node::getSecondaryClkSource() {
    return secondaryClkSource;
}

void Node::getListOfClockSources(std::vector<Address32>& list) {
}

//return true if the edge already exist and false other way
//insert_position = iterator of the existing edge (if true)
//insert_position = position where to insert (if false)
// !!! if edge is mark as deleted or peer node is removed return false and insert_position is set to the end of the list
bool Node::updateEdgesList(Address32 peerAddress, Uint8 rsl, EdgeList::iterator& insert_position){

    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();
    insert_position = outBoundEdges.end();

    //test if exists TR in the list; if true set secondElement
    EdgeList::iterator secondElement = outBoundEdges.begin();
    if (subnetTopology.existsActiveNode(secondElement->getDestination())) {
        Node& peerNode = subnetTopology.getNode(secondElement->getDestination());
        if (peerNode.getNodeType() == NodeType::BACKBONE) {
               secondElement++;
        }
    }

    //for all edges test if edge exist
    for (EdgeList::iterator it = outBoundEdges.begin(); it != outBoundEdges.end(); ++it) {
        //if edge exist test if new rsl value modifies the order of edges in list
        if (it->getDestination() == peerAddress) {

            if ((it->getStatus() == Status::DELETED) || (it->getStatus() == Status::REMOVED)) {
                //insert_position = outBoundEdges.end();
                return false;
            }

            //if peer is backbone set the insert_position (in order to update rsl) and return true
            if (subnetTopology.existsActiveNode(peerAddress)) {
                Node& peerNode = subnetTopology.getNode(peerAddress);
                if (peerNode.getNodeType() == NodeType::BACKBONE) {
                    insert_position = outBoundEdges.begin();
                    return true;
                }
            }
            else {
                insert_position = outBoundEdges.end();
                return false;
            }

            //if rsl of an edge is changed (which is not very probable in the case of static nodes) maintain the list sorted
            EdgeList::iterator it_find = it;
            EdgeList::iterator before = it;
            EdgeList::iterator after = it;
            EdgeList::iterator last = outBoundEdges.end();
            last--;

            bool goLeft = false;
            bool goRight = false;

            //test first, second and last elements
            if (it == outBoundEdges.begin() || it == secondElement) {
                goRight = true;
                //test if tr exist in list, if true start for the second element
                if (outBoundEdges.begin() != secondElement && it == outBoundEdges.begin()) {
                    continue;
                }
                after++;
            }
            else if (it == last) {
                goLeft = true;
                before--;
            }
            else {
                goLeft = true;
                before--;
                goRight = true;
                after++;
            }

            //go left
            if (goLeft && rsl > before->getRsl()) {
                //find new position before before
                it_find = before;
                bool erased = false;
                do {
                    if (it_find->getRsl() > rsl) {
                        erased = true;
                        break;
                    }
                    it_find--;
                } while (it_find != secondElement);

                //increment it_find if element is moved or if it_find is secondElement and tr exist
                if (erased) {
                    it_find++;
                } else if (secondElement != outBoundEdges.begin()) {
                    it_find++;
                }
                outBoundEdges.splice(it_find, outBoundEdges, it);
                //decrement the iterator in order to point to the moved element
                it_find--;
            }
            //go right
            else if (goRight && rsl < after->getRsl()) {
                //find new position after after
                it_find = after;
                for (; it_find != outBoundEdges.end(); ++it_find) {
                    if (it_find->getRsl() < rsl) {
                        break;
                    }
                }
                outBoundEdges.splice(it_find, outBoundEdges, it);
                //decrement the iterator in order to point to the moved element
                it_find--;
            }
            insert_position = it_find;
            return true;
        }
        //in the case that the edge is new, insert_position needs to indicate the position where to insert the new element
        else {
            //if tr exists do not evaluate its position
            if (it == outBoundEdges.begin() && outBoundEdges.begin() != secondElement) {
                continue;
            }
            //first "good" (which maintain the order in the list) position is saved in insert_position for new elements
            if (it->getRsl() < rsl && insert_position == outBoundEdges.end()) {
                insert_position = it;
            }
        }
    }
    return false;
}

bool Node::addVisibleNeighbor(Address32 peerAddress, Uint8 rsl) {
    // the list of  visible neighbors is kept in the list of edges with the type set to visible.

    EdgeList::iterator insert_position = outBoundEdges.end();

    //test if neighbor exist in list and re-sort the list if needed
    if (updateEdgesList(peerAddress, rsl, insert_position)) {
        return insert_position->addDiagnostics(rsl, false);
    }

    //if edge is mark as deleted return false
    if (insert_position->getStatus() == Status::DELETED || insert_position->getStatus() == Status::REMOVED) {
        return false;
    }

    //if peer node is backbone insert the edge at the beginin of the list, if peer node is marked as removed return false
    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();
    if (subnetTopology.existsActiveNode(peerAddress)) {
        Node& peerNode = subnetTopology.getNode(peerAddress);
        if (peerNode.getNodeType() == NodeType::BACKBONE) {
            insert_position = outBoundEdges.begin();
        }
    }
    else {
        return false;
    }

    //insert new edge
    Edge edge(nodeAddress, peerAddress);
    edge.setStatus(Status::CANDIDATE);
    bool result = edge.addDiagnostics(rsl);
    EdgeList::iterator new_edge = outBoundEdges.insert(insert_position, edge);
    outBoundEdgesMap.insert(std::pair<Address32, Edge*>(edge.getDestination(), &(*new_edge)));

    return result;
}

bool Node::addDiagnostics(Address32 neighborAddress, Uint8 rsl, Uint16 sent, Uint16 received, Uint16 failed) {
    for (EdgeList::iterator it = outBoundEdges.begin(); it != outBoundEdges.end(); ++it) {
        if (it->getDestination() == neighborAddress) {
            if (it->getStatus() == Status::DELETED || it->getStatus() == Status::REMOVED) {
                continue;
            }

            return it->addDiagnostics(rsl, sent, received, failed);
        }
    }

    LOG_INFO("addDiagnostics - on node:" << ToStr(nodeAddress) << " ,neighbor not found: " << ToStr(neighborAddress));
    return false;
}

Edge& Node::getEdge(Address32 address32) {

    EdgeMap::iterator it_edge = outBoundEdgesMap.find(address32);
    if (it_edge != outBoundEdgesMap.end()) {
        return *(it_edge->second);
    }

    std::ostringstream stream;
    stream << "For node: " << ToStr(nodeAddress);
    stream << " the following neighbor does not exist: " << ToStr(address32);
    LOG_ERROR(stream.str());
    throw NEException(stream.str());
}

bool Node::addEdge(Address32 peerAddress) {
    // the list of  visible neighbors is kept in the list of edges with the type set to visible.

    for (EdgeList::iterator it = outBoundEdges.begin(); it != outBoundEdges.end(); ++it) {
        if (it->getDestination() == peerAddress) {

            if (it->getStatus() == Status::DELETED || it->getStatus() == Status::REMOVED) {
                it->setStatus(Status::ACTIVE);
                // save from deletion, flag should be reset when the corresponding Remove gets confirmed.
                it->SaveFromDelete(true);
            }

            return false;
        }
    }

    EdgeList::iterator insert_position = outBoundEdges.end();
    bool insert_new = false;

    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();
    if (subnetTopology.existsActiveNode(peerAddress)) {
        Node& peerNode = subnetTopology.getNode(peerAddress);
        if (peerNode.getNodeType() == NodeType::BACKBONE) {
            insert_new = true;
            insert_position = outBoundEdges.begin();
        }
        else if (!updateEdgesList(peerAddress, 125, insert_position)) {
            insert_new = true;
        }

    }
    else {
        return false;
    }

    if (insert_new) {
        Edge edge(nodeAddress, peerAddress);
        EdgeList::iterator new_edge = outBoundEdges.insert(insert_position, edge);
        outBoundEdgesMap.insert(std::pair<Address32, Edge*>(edge.getDestination(), &(*new_edge)));
        activeEdgesNo++;
        return true;
    }

    return false;
}

void Node::resetPrefferdNeighbor(Uint16 graphId, Address32 neighborAddress) {
    for (EdgeList::iterator it = outBoundEdges.begin(); it != outBoundEdges.end(); ++it) {
        if (it->getDestination() == neighborAddress) {
            it->getGraphOnEdge(graphId).preffered = false;
        }
    }
}

void Node::setPrefferdNeighbor(Uint16 graphId, Address32 neighborAddress) {
    for (EdgeList::iterator it = outBoundEdges.begin(); it != outBoundEdges.end(); ++it) {
        if (it->getDestination() == neighborAddress) {
            it->getGraphOnEdge(graphId).preffered = true;
        }
    }
}

void Node::addGraphNeighbor(EngineOperations& engineOperations, Uint16 graphId, Uint16 traffic, Address32 address,
            bool preffered, bool addAlways, bool isBackboneException) {

    LOG_INFO("addGraphEdge: on node=" << ToStr(nodeAddress) << ", add graph=" << ToStr(graphId) << " for neighbor:"
                << ToStr(address));

    for (EdgeList::iterator it = outBoundEdges.begin(); it != outBoundEdges.end(); ++it) {
        if (it->getDestination() == address) {
            //LOG_INFO("addGraphEdge: add graph=" << ToStr(graphId) << " on edge:" << *it);

            if (it->getStatus() != Status::ACTIVE) {
                it->setStatus(Status::ACTIVE);
                activeEdgesNo++;
            }

            if (address == backboneAddress && !it->existsGraphNeighbor()) {
                SetClockSourceOperation *op = new SetClockSourceOperation(backboneAddress, nodeAddress, 0x4);
                IEngineOperationPointer engineOp(op);
                engineOp->setDependency(WaveDependency::FIFTH);
                engineOperations.addOperation(engineOp);
            }

            //TODO: ivp - set preferred edge
            it->addGraphNeighbor(graphId, traffic, preffered);

            if ((nodeAddress != MANAGER_ADDRESS) && (address == GATEWAY_ADDRESS) && !addAlways) {
                LOG_INFO("addGraphEdge: on node=" << ToStr(nodeAddress) << ", DO NOT ADD graph=" << ToStr(graphId)
                            << " for neighbor:" << ToStr(address));
                it->setGraphNeighborStatus(graphId, Status::ACTIVE);

            } else {
                if (nodeType == NodeType::BACKBONE && address == GATEWAY_ADDRESS && !isBackboneException) {
                    EdgeList::iterator it = outBoundEdges.begin();
                    for (; it != outBoundEdges.end(); ++it) {
                        if (it->getDestination() == GATEWAY_ADDRESS) {
                            it->setGraphNeighborStatus(graphId, Status::ACTIVE);
                            break;
                        }
                    }
                }
                else {
                    IEngineOperationPointer operation(new NeighborGraphAddedOperation(nodeAddress, graphId, address));
                    operation->setDependency(WaveDependency::FOURTH);
                    engineOperations.addOperation(operation);

                    LOG_DEBUG("addGraphNeighbor() : caches NeighborGraphAddedOperation for graphID : " << graphId
                            << " and device : " << ToStr(address));
                    neighborGraphAddedOperations.insert(std::make_pair(std::make_pair(graphId, address), operation));
                }
            }

            return;
        }
    }

    LOG_ERROR("addGraphEdge:  NOT FOUND EDGE:" << ToStr(nodeAddress) << " -> " << ToStr(address));
}


void Node::setRemoveGraphNeighbor(EngineOperations& engineOperations, Uint16 graphId, Address32 address) {
    LOG_INFO("setRemoveGraphNeighbor: on node=" << ToStr(nodeAddress) << ", remove graph=" << ToStr(graphId)
                << " for neighbor:" << ToStr(address));

    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();

    for (EdgeList::iterator it = outBoundEdges.begin(); it != outBoundEdges.end(); ++it) {
        if (it->getDestination() == address) {
            LOG_INFO("deleteGraphNeighbor:  SELECT EDGE:" << (*it));

            it->setRemoveGraphNeighbor(graphId);

            if ((it->getActiveTraffic() == 0) && (secondaryClkSource == address)) {
                subnetTopology.pushOnEvaluationNodes(nodeAddress);
            }

            IEngineOperationPointer operation(new NeighborGraphRemovedOperation(nodeAddress, graphId, address));

            operation->setDependency(WaveDependency::NINETH);

            if (neighborGraphAddedOperations.find(std::make_pair(graphId, address))
                        != neighborGraphAddedOperations.end()) {
                //operation->setOperationDependency(neighborGraphAddedOperations[std::make_pair< graphId, address>]);
                LOG_DEBUG(
                            "It should be set the NeighborGraphAddedOperation dependency for the delete operation for graphId : "
                                        << graphId << " and address: " << ToStr(address));
            }

            engineOperations.addOperation(operation);
            return;
        }
    }
    LOG_ERROR("deleteGraphNeighbor:  NOT FOUND EDGE:" << ToStr(nodeAddress) << " -> " << ToStr(address));
}

void Node::addSourceNeighbor(Uint16 graphId, Uint16 traffic, Address32 address) {
    for (EdgeList::iterator it = outBoundEdges.begin(); it != outBoundEdges.end(); ++it) {
        if (it->getDestination() == address) {
            std::ostringstream stream;
            it->toShortString(stream);
            LOG_INFO("addGraphEdge:  SELECT EDGE:" << stream.str());

            it->setStatus(Status::ACTIVE);

            //TODO: ivp - set preferred edge
            it->addGraphNeighbor(graphId, traffic, true);

            return;
        }
    }
    LOG_ERROR("addGraphEdge:  NOT FOUND EDGE:" << ToStr(nodeAddress) << " -> " << ToStr(address));
}

void Node::removeSourceNeighbor(Uint16 graphId, Address32 address) {
    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();

    for (EdgeList::iterator it = outBoundEdges.begin(); it != outBoundEdges.end(); ++it) {
        if (it->getDestination() == address) {
            std::ostringstream stream;
            it->toShortString(stream);
            LOG_INFO("deleteGraphNeighbor:  SELECT EDGE:" << stream.str());

            it->removeGraphNeighbor(graphId);

            if (!it->existsGraphNeighbor() || !subnetTopology.existsNode(it->getDestination())) {
                removeEdge(it);
            }

            return;
        }
    }
    LOG_ERROR("deleteGraphNeighbor:  NOT FOUND EDGE:" << ToStr(nodeAddress) << " -> " << ToStr(address));
}

Address32 Node::getEvalLazyParent() {
    return evalLazyParent;
}

void Node::setEvalLazyParent(Address32 evalLazyParent_, Uint8 deep_, Uint16 evalSearchId_) {
    LOG_DEBUG("setEvalLazyParent - node=" << ToStr(nodeAddress) << " old deep=" << (int) deep << " setLazyParent="
                << ToStr(evalLazyParent_) << " deep=" << (int) deep_);
    if (evalSearchId != evalSearchId_) {
        LOG_DEBUG("setEvalLazyParent - reset");
        deep = 0;
        evalSearchId = evalSearchId_;
    }

    if (deep_ > deep) {
        evalLazyParent = evalLazyParent_;
        deep = deep_;
        LOG_DEBUG("setEvalLazyParent - set");
    }
}

void Node::setDeep(Uint8 deep_) {
    deep = deep_;
}

void Node::setEvalLazyParent(Address32 evalLazyParent_) {
    evalLazyParent = evalLazyParent_;
}

GraphNeighbor& Node::updateLazyParent(Address32 peer, Address32 peerLazyParent, Uint16 graphId) {
    LOG_DEBUG("updateLazyParent - nodeAddress: " << ToStr(nodeAddress) << " peer=" << ToStr(peer)
                << ", peerLazyParent=" << ToStr(peerLazyParent));

    for (EdgeList::iterator it = outBoundEdges.begin(); it != outBoundEdges.end(); ++it) {
        if (it->getStatus() == Status::DELETED) {
            LOG_DEBUG("updateLazyParent - SKIP");
            continue;
        }

        if (it->getDestination() == peer) {
            LOG_DEBUG("updateLazyParent - found for edge: " << *it);

            GraphNeighbor& graphNeighbor = it->getGraphOnEdge(graphId);
            if (nodeAddress == peerLazyParent) {
                graphNeighbor.lazy = true;
                LOG_DEBUG("updateLazyParent - set");
            } else {
                graphNeighbor.lazy = false;
                LOG_DEBUG("updateLazyParent - not set");
            }

            return graphNeighbor;
        }
    }

    std::ostringstream stream;
    stream << "For node: " << ToStr(nodeAddress);
    stream << " the following neighbor does not exist: " << ToStr(peer);
    LOG_ERROR(stream.str());
    throw NEException(stream.str());
}

void Node::addEvalGraphNeighbor(Address32 address) {
    evalGraphNeighbors[address] = true;
}

std::map<Address32, bool>& Node::getEvalGraphNeighbors() {
    return evalGraphNeighbors;
}

void Node::resetEvalGraphNeighbors() {
    evalGraphNeighbors.clear();
}

bool Node::maxNeighborsNoReached() {
    try {

        int maxNeighbors = metaDataAttributes->getMaxNeighbors();
        int maxConfig = 99;
        if (getNodeType() == NodeType::BACKBONE)
        {
            maxConfig = NE::Model::NetworkEngine::instance().getSettingsLogic().maxTRNeighbors;
        }
        else
        {
            maxConfig = NE::Model::NetworkEngine::instance().getSettingsLogic().maxDeviceNeighbors;
        }
        maxNeighbors = maxConfig < maxNeighbors ? maxConfig : maxNeighbors;


        return linkedNeighborsCount >= maxNeighbors;
    }
    catch (...) {
        return true;
    }
}

Uint16 Node::evaluateSettingsCost() {

    if (nodeType == NodeType::MANAGER || nodeType == NodeType::GATEWAY) {
        settingsCost = 0;
        return settingsCost;
    }

    if (nodeType == NodeType::BACKBONE) {
        settingsCost = 0;
        return settingsCost;
    }

    settingsCost = 0;

    Uint8 index = (metaDataAttributes->getUsedGraphNeighbors() * 20) / (metaDataAttributes->getTotalGraphNeighbors() - 2);
    settingsCost += distribution[index];

    index = (metaDataAttributes->getUsedLinks() * 20) / (metaDataAttributes->getTotalLinks() - 2);
    if (settingsCost != 0xFFFF && distribution[index] != 0xFFFF) {
        settingsCost += distribution[index];
    }
    else {
        settingsCost = 0xFFFF;
    }

    LOG_DEBUG("Node=" << ToStr(nodeAddress) << ", Cost Settings=" << settingsCost);
    LOG_DEBUG("Settings: " << metaDataAttributes->toString());

    return settingsCost;
}

Uint16 Node::getSettingsCost() {
    return settingsCost;
}

Uint16 Node::evaluateEdgeCost(Address32 address, Uint16 graphId, Uint16 traffic) {
    for (EdgeList::iterator it = outBoundEdges.begin(); it != outBoundEdges.end(); ++it) {

        if (it->getStatus() == Status::DELETED) {
            continue;
        }

        if (it->getDestination() == address) {
            return it->evaluateEdgeCost(graphId, traffic);
        }
    }

    return 0;
}

Uint16 Node::getEvalEdgeCost(Address32 address) {
    for (EdgeList::iterator it = outBoundEdges.begin(); it != outBoundEdges.end(); ++it) {
        if (it->getStatus() == Status::DELETED) {
            continue;
        }

        if (it->getDestination() == address) {
            return it->getEvalEdgeCost();
        }
    }

    return 0;
}

bool Node::existsGraphNeighbor(Uint16 graphId) {
    for (EdgeList::iterator it = outBoundEdges.begin(); it != outBoundEdges.end(); ++it) {
        if (it->getStatus() == Status::DELETED) {
            continue;
        }

        if (it->existsGraphNeighbor(graphId)) {
            return true;
        }
    }

    return false;
}

bool Node::hasNeighborOnGraph(Address32 address, Uint16 graphId) {
    for (EdgeList::iterator it = outBoundEdges.begin(); it != outBoundEdges.end(); ++it) {
        if (it->getStatus() == Status::DELETED) {
            continue;
        }

        if (it->existsGraphNeighbor(graphId) && it->getDestination() == address) {
            return true;
        }
    }

    return false;
}

bool Node::hasNeighbor(Address32 address) {
    for (EdgeList::iterator it = outBoundEdges.begin(); it != outBoundEdges.end(); ++it) {
        if (it->getStatus() == Status::DELETED) {
            continue;
        }

        if (it->getDestination() == address) {
            return true;
        }
    }

    return false;
}

bool Node::hasOtherOutboundNeighborOnGraph(Address32 address, Uint16 graphId) {
    for (EdgeList::iterator it = outBoundEdges.begin(); it != outBoundEdges.end(); ++it) {
        if (it->getStatus() == Status::DELETED) {
            continue;
        }

        if (it->existsGraphNeighbor(graphId) && it->getDestination() != address) {
            return true;
        }
    }

    return false;
}

bool Node::hasOtherInboundNeighborOnGraph(Address32 address, Uint16 graphId) {
    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();

    for (EdgeList::iterator it = outBoundEdges.begin(); it != outBoundEdges.end(); ++it) {
        if (it->getStatus() == Status::DELETED) {
            continue;
        }

        if (it->getDestination() != address) {
            if (!subnetTopology.existsNode(it->getDestination()))
                return false;

            Node& peer = subnetTopology.getNode(it->getDestination());

            if (peer.hasNeighborOnGraph(nodeAddress, graphId)) {
                return true;
            }
        }
    }

    return false;
}

bool Node::resolveOperation(NeighborGraphAddedOperation& operation) {
    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();

    for (EdgeList::iterator it = outBoundEdges.begin(); it != outBoundEdges.end(); ++it) {
        if (it->getDestination() == operation.getNeighbor()) {

            //the status can be confirmed or generated
            if (operation.getState() == OperationState::CONFIRMED) {
                //TODO: check error code

                if (!subnetTopology.existsPath(operation.getGraphId())) {
                    LOG_DEBUG("resolveOperation - NeighborGraphAddedOperation - graph not exists:" << ToStr(
                                operation.getGraphId()));
                    return true;
                }

                // if (!it->existsGraphNeighbor() && !subnetTopology.existsNode(it->getDestination())) {
                if (!it->existsGraphNeighbor() || !subnetTopology.existsNode(it->getDestination())) {
                    removeEdge(it);
                    return true;                }

                if (operation.getResponseCode() == C969_E66) {
                    removeEdge(it);
                    return true;
                }

                try {
                    it->setGraphNeighborStatus(operation.getGraphId(), Status::ACTIVE);
                } catch (NEException e) {
                    LOG_DEBUG("resolveOperation - NeighborGraphAddedOperation - the edge:" << *it << "has not graph: "
                                << operation.getGraphId());
                }

                confirmAddGraphNeighborOperation(operation.getGraphId(), it->getDestination());
                return true;
            } else {
                it->removeGraphNeighbor(operation.getGraphId());

                if (!subnetTopology.existsNode(it->getDestination())) {
                    removeEdge(it);
                    return true;
                }

                if (!it->existsGraphNeighbor()) {
                    it->setStatus(Status::CANDIDATE);
                }

                return true;
            }
        }
    }

    std::ostringstream stream;
    stream << "resolveOperation - no edge found for operation: ";
    operation.toString(stream);

    LOG_ERROR(stream.str());
    return true;
}

bool Node::resolveOperation(NeighborGraphRemovedOperation& operation) {
    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();

    if (subnetTopology.existsActivePath(operation.getGraphId())) {
        Path& path = subnetTopology.getPath(operation.getGraphId());

        //if problems occur and the operation is SI restore the old primary/secondary Inbound/Outbound in the model
        if (operation.getState() == OperationState::SENT_IGNORED) {
            //if outbound
            if (path.getSource() == MANAGER_ADDRESS
                        && path.getDestination() == operation.getNeighbor()) {
                Node& destNode =  subnetTopology.getNode(operation.getNeighbor());
                destNode.goBackToOldOutbound(nodeAddress);
            }
            //if inbound
            if (path.getDestination() == MANAGER_ADDRESS
                        && path.getSource() == nodeAddress) {
                goBackToOldInbound(operation.getNeighbor());
            }
        }
    }

    for (EdgeList::iterator it = outBoundEdges.begin(); it != outBoundEdges.end(); ++it) {
        if (it->getDestination() == operation.getNeighbor()) {
            if (operation.getState() == OperationState::CONFIRMED) {
                //TODO: check error code
                it->removeGraphNeighbor(operation.getGraphId());

                if (!it->existsGraphNeighbor() || !subnetTopology.existsNode(it->getDestination())) {
                    if (it->SaveFromDelete())
                    {
                     it->SaveFromDelete(false);
                    }
                    else
                    {
                        removeEdge(it);
                    }
                }

                return true;
            } else {
                if (!subnetTopology.existsPath(operation.getGraphId())) {
                    LOG_DEBUG("resolveOperation - NeighborGraphRemovedOperation - graph not exists:" << ToStr(
                                operation.getGraphId()));

                    //if (!it->existsGraphNeighbor() && !subnetTopology.existsNode(it->getDestination())) {
                    if (!it->existsGraphNeighbor() || !subnetTopology.existsNode(it->getDestination())) {
                        removeEdge(it);
                    }

                    return true;
                }

                try {
                    it->setGraphNeighborStatus(operation.getGraphId(), Status::ACTIVE);
                } catch (NEException e) {
                    LOG_DEBUG("resolveOperation - NeighborGraphRemovedOperation - the edge:" << *it
                                << "has not graph: " << operation.getGraphId());
                }

                return true;
            }
        }
    }

    std::ostringstream stream;
    stream << "resolveOperation - no edge found for operation: ";
    operation.toString(stream);

    LOG_ERROR(stream.str());
    //TODO: ivp - is this ok?
    return true;
}

void Node::generateClockSources(EngineOperations& operations, Uint16 graphId) {
    if (nodeType != NodeType::NODE) {
        return;
    }

    for (EdgeList::iterator it = outBoundEdges.begin(); it != outBoundEdges.end(); ++it) {
        if (it->existsDeletedClockSource(graphId)) {
            if (this->secondaryClkSource == it->getDestination()) {
                // secondary clock source node was removed
                resetSecondaryClkSource();
            }

            SetClockSourceOperation *op = new SetClockSourceOperation(it->getSource(), it->getDestination(), false);
            IEngineOperationPointer engineOp(op);
            engineOp->setDependency(WaveDependency::ELEVENTH);
            operations.addOperation(engineOp);
        } else if (it->existsGraphNeighbor(graphId)) {
            if (it->getGraphOnEdge(graphId).preffered) {
                if (this->secondaryClkSource == it->getDestination()) {
                    // primary clk source was moved into existing secondary clk source -> reset secondary clk source
                    resetSecondaryClkSource();
                }

                SetClockSourceOperation *op = new SetClockSourceOperation(it->getSource(), it->getDestination(),
                                true);
                IEngineOperationPointer engineOp(op);
                engineOp->setDependency(WaveDependency::FOURTH);
                operations.addOperation(engineOp);

            }
        }
    }
}

void Node::removeEdge(EdgeList::iterator& it_edge) {
    if (it_edge->getStatus() == Status::ACTIVE) {
        activeEdgesNo--;
    }

    EdgeMap::iterator it_edge_map = outBoundEdgesMap.find(it_edge->getDestination());
    if (it_edge_map != outBoundEdgesMap.end()) {
        outBoundEdgesMap.erase(it_edge_map);
    }

    it_edge = outBoundEdges.erase(it_edge);
}

std::ostream& NE::Model::Topology::operator<<(std::ostream& stream, const Node& node) {
    stream << "Node { nodeAddress=" << ToStr(node.nodeAddress);
    stream << ", parentAddress32=" << ToStr(node.parentAddress);
    stream << ", backboneAddress32=" << ToStr(node.backboneAddress);
    stream << ", secondaryClkSrc=" << ToStr(node.secondaryClkSource);
    stream << ", activeEdgesNo=" << ToStr(node.activeEdgesNo);
    stream << ", status=" << getStatusDescription(node.status) << ", outBoundEdges=";
    for (EdgeList::const_iterator it = node.outBoundEdges.begin(); it != node.outBoundEdges.end(); ++it) {
        it->toShortStatusString(stream);
    }
    stream << "}";

    return stream;
}

std::ostream& NE::Model::Topology::operator<<(std::ostream& stream, const Node::IndentString& indentString) {

    const Node &node = indentString.node;

    std::string addressString;
    stream << "Node {" << std::endl;
    stream << "        nodeAddress=" << ToStr(node.nodeAddress);
    stream << ", parentAddress=" << ToStr(node.parentAddress);
    stream << ", backboneAddress=" << ToStr(node.backboneAddress);
    stream << ", inGraphId=" << ToStr(node.inboundGraphId);
    stream << ", outGraphId=" << ToStr(node.outboundGraphId);

    stream << ", inP=" << ToStr(node.primaryInbound);
    stream << ", inS=" << ToStr(node.secondaryInbound);
    stream << ", outP=" << ToStr(node.primaryOutbound);
    stream << ", outS=" << ToStr(node.secondaryOutbound);

    stream << ", secondaryClkSrc=" << ToStr(node.secondaryClkSource);
    stream << ", activeEdgesNo=" << ToStr(node.activeEdgesNo);
    stream << ", status=" << Status::getStatusDescription(node.status) << std::endl << "        outBoundEdges=";
    for (EdgeList::const_iterator it = node.outBoundEdges.begin(); it != node.outBoundEdges.end(); ++it) {
        stream << std::endl;
        it->toShortStatusString(stream);
    }
    stream << std::endl << "    }";

    return stream;
}

void Node::confirmAddGraphNeighborOperation(Uint16 graphId, Address32 address) {
    LOG_DEBUG("confirmAddGraphNeighborOperation() for graphId : " << graphId << ", address : " << ToStr(address));

    std::map<std::pair<Uint16, Address32>, NE::Model::Operations::IEngineOperationPointer>::iterator it;
    it = neighborGraphAddedOperations.find(std::make_pair(graphId, address));

    if (it != neighborGraphAddedOperations.end()) {
        this->neighborGraphAddedOperations.erase(it);
    }
}

