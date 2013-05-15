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
 * SelectMinPath.cpp
 *
 *  Created on: Dec 29, 2010
 *      Author: Mihai.Stef
 */

#include "SelectMinPath.h"
#include "Common/NEAddress.h"

#include "Model/Topology/Edge.h"
#include "Model/Topology/Path.h"
#include "Model/Topology/Node.h"
#include "Model/NetworkEngine.h"

using namespace NE::Model::Topology::Algorithms;


SelectMinPath::SelectMinPath() {
    // TODO Auto-generated constructor stub
    settingsFactor = NE::Model::NetworkEngine::instance().getSettingsLogic().settingsFactor;
    topologyFactor = NE::Model::NetworkEngine::instance().getSettingsLogic().topologyFactor;
    hopsFactor = NE::Model::NetworkEngine::instance().getSettingsLogic().hopsFactor;
    maxHops = NE::Model::NetworkEngine::instance().getSettingsLogic().maxHops;
    maxNeighbors = NE::Model::NetworkEngine::instance().getSettingsLogic().maxNeighbors;
    rerouteOnlyOnFail = (bool) NE::Model::NetworkEngine::instance().getSettingsLogic().rerouteOnlyOnFail;

}

SelectMinPath::~SelectMinPath() {
    // TODO Auto-generated destructor stub
}





bool SelectMinPath::evaluateGraphPath(EngineOperations& engineOperations, Path& path) {
    bool result;

    LOG_INFO("evaluateGraphPath - START for graph =" << path);

    source = path.getSource();
    destination = path.getDestination();
    graphId = path.getGraphId();
    traffic = path.getTraffic();

    if (path.getSource() == MANAGER_ADDRESS || path.getSource() == GATEWAY_ADDRESS) {
        result = evaluateOutboundGraphPath(engineOperations, path);
    } else {
        result = evaluateInboundGraphPath(engineOperations);
    }
    LOG_INFO("evaluateGraphPath - START for graph =" << path);
    //TODO: test only
    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();
    Node& node = subnetTopology.getNode(path.getSource());

    if (source == MANAGER_ADDRESS || source == GATEWAY_ADDRESS) {
        node.setOutboundGraphId(path.getGraphId());
    } else {
        node.setInboundGraphId(path.getGraphId());
    }

    cost = evaluateCost(path.getSource(), subnetTopology.setNextSearchId(), false);
    //end test

    path.setCost(cost);

    LOG_DEBUG("evaluateGraphPath - END with " << (int) result);
    LOG_INFO("evaluateGraphPath - result graph =" << path);

    return result;
}


bool SelectMinPath::evaluateInboundGraphPath(EngineOperations& engineOperations) {

    //LOG_INFO("evaluateInboundGraphPath");

    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();

    Address32 primaryPeer = 0;
    Address32 secondaryPeer = 0;
    Address32 candidatePeer = 0;

    boost::shared_ptr<AddressList> candidatePath (new AddressList);
    boost::shared_ptr<AddressList> minCandidatePath(new AddressList);
    boost::shared_ptr<AddressList> primaryCheckPath(new AddressList);
    boost::shared_ptr<AddressList> secondaryCheckPath(new AddressList);

    Uint32 candidateMinCost = MAX_COST_32;
    Uint32 primaryCost = MAX_COST_32;
    Uint32 secondaryCost = MAX_COST_32;

    bool newGraph = false;

    Node& sourceNode = subnetTopology.getNode(source);
    Node *secondaryNode = NULL;
    Node *primaryNode = NULL;
    EdgeList& sourceEdges = sourceNode.getOutBoundEdges();

    primaryPeer = sourceNode.getPrimaryInbound();
    secondaryPeer = sourceNode.getSecondaryInbound();

    if (primaryPeer == 0 && secondaryPeer == 0) {
        LOG_INFO("evaluateInboundGraphPath - not found primary and secondary");
        primaryPeer = sourceNode.getParentAddress();
        newGraph = true;
    }

    if (primaryPeer > 0) {
        if (!subnetTopology.existsNode(primaryPeer)) {
            LOG_ERROR("evaluateInboundGraphPath - primaryPeer: node not found=" << ToStr(primaryPeer));
            primaryPeer = 0;
        } else {
            Node& tmpNode = subnetTopology.getNode(primaryPeer);
            Uint16 tmpGraphId = tmpNode.getInboundGraphId();
            if (tmpNode.getStatus() == Status::DELETED) {
                LOG_ERROR("evaluateInboundGraphPath - primaryPeer: node deleted=" << ToStr(primaryPeer));
                primaryPeer = 0;
            } else if (!subnetTopology.existsActivePath(tmpGraphId)) {
                LOG_ERROR("evaluateInboundGraphPath - primaryPeer: no active graph=" << ToStr(tmpGraphId));
                primaryPeer = 0;
            } else if (!newGraph && !sourceNode.hasNeighborOnGraph(primaryPeer, sourceNode.getInboundGraphId())) {
                LOG_ERROR("evaluateInboundGraphPath - primaryPeer: on graph=" << ToStr(sourceNode.getInboundGraphId()) << " no neighbor=" << ToStr(primaryPeer));
                primaryPeer = 0;
            }
        }
    }

    if (secondaryPeer > 0) {
        if (!subnetTopology.existsNode(secondaryPeer)) {
            LOG_ERROR("evaluateInboundGraphPath - secondaryPeer: node not found=" << ToStr(secondaryPeer));
            secondaryPeer = 0;
        } else {
            Node& tmpNode = subnetTopology.getNode(secondaryPeer);
            Uint16 tmpGraphId = tmpNode.getInboundGraphId();
            if (tmpNode.getStatus() == Status::DELETED) {
                LOG_ERROR("evaluateInboundGraphPath - secondaryPeer: node deleted=" << ToStr(secondaryPeer));
                secondaryPeer = 0;
            } else if (!subnetTopology.existsActivePath(tmpGraphId)) {
                LOG_ERROR("evaluateInboundGraphPath - secondaryPeer: no active graph=" << ToStr(tmpGraphId));
                secondaryPeer = 0;
            } else if (!newGraph && !sourceNode.hasNeighborOnGraph(secondaryPeer, sourceNode.getInboundGraphId())) {
                LOG_ERROR("evaluateInboundGraphPath - secondaryPeer: on graph=" << ToStr(sourceNode.getInboundGraphId()) << " no neighbor=" << ToStr(secondaryPeer));
                secondaryPeer = 0;
            }
        }
    }

    if (primaryPeer == 0 && secondaryPeer != 0) {
        primaryPeer = secondaryPeer;
        secondaryPeer = 0;
    }

    LOG_DEBUG("evaluateInboundGraphPath: primaryPeer=" << ToStr(primaryPeer) << ", secondaryPeer=" << ToStr(
                secondaryPeer));

    std::multimap<Uint32, Candidate> candidates;

    bool inboundAttach = subnetTopology.getPath(graphId).getEvaluatePathPriority() == EvaluatePathPriority::InboundAttach;

    //CHECK PRIMARY PEER
    //replace secondary with a new one and keep the primary unchanged
    //if rerouteOnFail, start the evaluation only if the seccondPeer == 0
    Candidate prim;
    if (primaryPeer > 0 && (!rerouteOnlyOnFail || secondaryPeer == 0)) {
        Node& node = subnetTopology.getNode(primaryPeer);
        primaryNode = &node;
        Uint16 checkGraphId = node.getInboundGraphId();
        Path& path = subnetTopology.getPath(checkGraphId);
        (*primaryCheckPath).clear();
        primaryCost = path.getMinCostPath(*primaryCheckPath, false, source);
        if (primaryCost != MAX_COST_32) {
            try {
                Uint16 edgeCost = sourceNode.getEdge(primaryPeer).evaluateEdgeCost(graphId, traffic);
                if (edgeCost != MAX_COST_16) {
                    primaryCost += edgeCost;
                } else {
                    primaryCost = MAX_COST_32;
                }
            }
            catch (...) {
                primaryCost = MAX_COST_32;
            }
        }
        prim.peer = primaryPeer;
        prim.pathAddrs = *primaryCheckPath;
        //insert primaryInbound in case there is no other solution

        if (primaryCost != MAX_COST_32
                    || inboundAttach) {
            candidates.insert(std::pair<Uint32, Candidate>(primaryCost, prim));
        }

    }

    //CHECK SECONDARY PEER
    //replace primary with secondary and secondary with a new one
    //if rerouteOnFail, start the evaluation only if the primaryPeer == 0
    Candidate sec;
    if (secondaryPeer > 0 && (!rerouteOnlyOnFail || primaryPeer == 0)) {
        Node& node = subnetTopology.getNode(secondaryPeer);
        secondaryNode = &node;
        Uint16 checkGraphId = node.getInboundGraphId();
        Path& path = subnetTopology.getPath(checkGraphId);
        (*secondaryCheckPath).clear();
        secondaryCost = path.getMinCostPath(*secondaryCheckPath, false, source);
        if (secondaryCost != MAX_COST_32) {
            try {
                Uint16 edgeCost = sourceNode.getEdge(secondaryPeer).evaluateEdgeCost(graphId, traffic);
                if (edgeCost != MAX_COST_16) {
                    secondaryCost += edgeCost;
                } else {
                    secondaryCost = MAX_COST_32;
                }
            }
            catch (...) {
                secondaryCost = MAX_COST_32;
            }
        }
        sec.peer = secondaryPeer;
        sec.pathAddrs = *secondaryCheckPath;
        if (secondaryCost != MAX_COST_32) {
            candidates.insert(std::pair<Uint32, Candidate>(secondaryCost, sec));
            if (secondaryCost < primaryCost) {
//                winningNode = &node;
//                winningCost = secondaryCost;
            }
        }
    }

    Uint16 neighborCount = 0;

    //find the best candidate
    EdgeList::iterator it_edge = sourceEdges.begin();
    for (; it_edge != sourceEdges.end(); ++it_edge) {

        Node& candidateNode = subnetTopology.getNode(it_edge->getDestination());
        Path& path = subnetTopology.getPath(candidateNode.getInboundGraphId());

        if (path.getEvaluatePathPriority() == EvaluatePathPriority::InboundAttach) {
            continue;
        }

        if (it_edge->getDestination() == primaryPeer || it_edge->getDestination() == secondaryPeer) {
            continue;
        }

        (*candidatePath).clear();

        uint32_t candidateCost = path.getMinCostPath(*candidatePath, false, source);

        if (candidateCost != MAX_COST_32) {
            try {
                //TODO: use it_edge here
                Uint16 edgeCost = sourceNode.getEdge(candidateNode.getNodeAddress()).evaluateEdgeCost(graphId, traffic);
                if (edgeCost != MAX_COST_16) {
                    candidateCost += edgeCost;
                } else {
                    candidateCost = MAX_COST_32;
                }
            }
            catch (...) {
                candidateCost = MAX_COST_32;
            }
        }

        if (candidateCost != MAX_COST_32){
            std::multimap<Uint32, Candidate>::iterator candPos = candidates.insert(std::make_pair(candidateCost, Candidate()));
            candPos->second.peer = candidateNode.getNodeAddress();
            candPos->second.pathAddrs = *candidatePath;
        }


        if (neighborCount == maxNeighbors) {
            break;
        }
        else {
            neighborCount++;
        }
    }

    if (candidates.empty())
    {
        LOG_INFO("Empty candindates.");
        return false;
    }

    std::multimap<Uint32, Candidate>::iterator it_candidates = candidates.begin();
    std::multimap<Uint32, Candidate>::iterator it_primary;
    std::multimap<Uint32, Candidate>::iterator it_secondary;

    bool primFound = false;
    bool secFound = false;

    if (inboundAttach) {
        for (it_candidates = candidates.begin(); it_candidates != candidates.end(); ++it_candidates) {
            if (it_candidates->second.peer == primaryPeer) {
                it_primary = it_candidates;
                primFound = true;
                break;
            }
        }
    }

    for (it_candidates = candidates.begin(); it_candidates != candidates.end(); ++it_candidates) {
        if (it_candidates->first != MAX_COST_32 && !primFound) {
            it_primary = it_candidates;
            primFound = true;
            continue;
        }
        if (it_candidates->first != MAX_COST_32 && primFound) {
            if (it_candidates->second.peer != it_primary->second.peer) {
                it_secondary = it_candidates;
                secFound = true;
                break;
            }
        }
    }

    if (secFound) {
        if (it_primary->second.peer == primaryPeer && it_secondary->second.peer == secondaryPeer) {
            //nothing changed
            LOG_DEBUG("evaluateInbound - 1");
            return false;
        }
        else {
            setInbound(engineOperations, it_primary->second.peer, it_secondary->second.peer,
                        (it_primary->second.pathAddrs), (it_secondary->second.pathAddrs));
            LOG_DEBUG("evaluateInbound - 2");
        }
    }
    else if (primFound) {
            setInbound(engineOperations, it_primary->second.peer, 0,
                        (it_primary->second.pathAddrs), (it_primary->second.pathAddrs));
            LOG_DEBUG("evaluateInbound - 3");
    }

    //if new primary found reset clok src for old primary
    if (primFound) {
        if (primaryPeer != it_primary->second.peer) {
            SetClockSourceOperation *op = new SetClockSourceOperation(source, primaryPeer, false);
            IEngineOperationPointer engineOp(op);
            engineOp->setDependency(WaveDependency::ELEVENTH);
            engineOperations.addOperation(engineOp);
        }
    }

    return true;
}

bool SelectMinPath::createsClockSourceCycle(Address32 secondaryNode, bool checkSecondary) {
    std::set<Address32> touchedAddresses;
    std::queue<Address32> remainingAddresses;

    touchedAddresses.insert(source);
    remainingAddresses.push(secondaryNode);

    while (!remainingAddresses.empty()) {
        Address32 peer = remainingAddresses.front();
        remainingAddresses.pop();
        try {
            Node& peerNode = NetworkEngine::instance().getSubnetTopology().getNode(peer);

            Address32 primary = peerNode.getPrimaryInbound();
            Address32 secondary = peerNode.getSecondaryClkSource();

            if (touchedAddresses.find(primary) != touchedAddresses.end()) {
                LOG_DEBUG("Avoiding secondary clock source cycle on " << peer << " with primary CS " << primary);
                return true;
            }
            if (checkSecondary && touchedAddresses.find(secondary) != touchedAddresses.end()) {
                LOG_DEBUG("Avoiding secondary clock source cycle on " << peer << " with secondary CS " << secondary);
                return true;
            }
            // add as touched if exists and not TR
            if (primary != 0 && primary != peerNode.getBackboneAddress()) {
                touchedAddresses.insert(primary);
                remainingAddresses.push(primary);
            }
            if (checkSecondary && secondary != 0 && secondary != peerNode.getBackboneAddress()) {
                touchedAddresses.insert(secondary);
                remainingAddresses.push(secondary);
            }
        } catch (...) {
            // node does not exist
        }
    }

    return false;
}

bool SelectMinPath::evaluateOutboundGraphPath(EngineOperations& engineOperations, Path& path) {
    //LOG_INFO("evaluateOutboundGraphPath");

    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();

    Address32 primaryPeer = 0;
    Address32 primaryMinPeer = 0;
    Address32 secondaryPeer = 0;
    Address32 secondaryMinPeer = 0;

    Uint32 cost = 0;
    Uint32 primaryMinCost = MAX_COST_32;
    Uint32 secondaryMinCost = MAX_COST_32;

    bool newGraph = false;
    AddressList minCandidatePath;
    AddressList minCheckPath;
    AddressList primaryCandidatePath;
    AddressList primaryCheckPath;
    AddressList secondaryCandidatePath;
    AddressList secondaryCheckPath;

    Node& destinationNode = subnetTopology.getNode(destination);

    primaryPeer = destinationNode.getPrimaryOutbound();
    secondaryPeer = destinationNode.getSecondaryOutbound();

    if (primaryPeer == 0 && secondaryPeer == 0) {
        LOG_INFO("evaluateOutboundGraphPath - not found primary and secondary");
        primaryPeer = destinationNode.getParentAddress();
        Node& parentNode = subnetTopology.getNode(primaryPeer);
        Path& parentPath = subnetTopology.getPath(parentNode.getOutboundGraphId());
        parentPath.getMinCostPath(primaryCheckPath, false);
        newGraph = true;
    }

    if (primaryPeer > 0) {
        if (!subnetTopology.existsNode(primaryPeer)) {
            LOG_ERROR("evaluateOutboundGraphPath - primaryPeer: node not found=" << ToStr(primaryPeer));
            primaryPeer = 0;
        } else {
            Node& tmpNode = subnetTopology.getNode(primaryPeer);
            Uint16 tmpGraphId = tmpNode.getOutboundGraphId();

            if (!subnetTopology.existsActivePath(tmpGraphId)) {
                LOG_ERROR("evaluateOutboundGraphPath - primaryPeer: no active graph=" << ToStr(tmpGraphId));
                primaryPeer = 0;
            } else if (!newGraph && !tmpNode.hasNeighborOnGraph(destination, destinationNode.getOutboundGraphId())) {
                LOG_ERROR("evaluateOutboundGraphPath - primaryPeer: on graph=" << ToStr(destinationNode.getOutboundGraphId()) << " no destination for neighbor=" << ToStr(primaryPeer));
                primaryPeer = 0;
            }
        }
    }

    if (secondaryPeer > 0) {
        if (!subnetTopology.existsNode(secondaryPeer)) {
            LOG_ERROR("evaluateOutboundGraphPath - secondaryPeer: node not found=" << ToStr(secondaryPeer));
            secondaryPeer = 0;
        } else {
            Node& tmpNode = subnetTopology.getNode(secondaryPeer);
            Uint16 tmpGraphId = tmpNode.getOutboundGraphId();

            if (!subnetTopology.existsActivePath(tmpGraphId)) {
                LOG_ERROR("evaluateOutboundGraphPath - secondaryPeer: no active graph=" << ToStr(tmpGraphId));
                secondaryPeer = 0;
            } else if (!newGraph && !tmpNode.hasNeighborOnGraph(destination, destinationNode.getOutboundGraphId())) {
                LOG_ERROR("evaluateOutboundGraphPath - secondaryPeer: on graph=" << ToStr(destinationNode.getOutboundGraphId()) << " no destination for neighbor=" << ToStr(secondaryPeer));
                secondaryPeer = 0;
            }
        }
    }

    if (primaryPeer == 0 && secondaryPeer != 0) {
        primaryPeer = secondaryPeer;
        secondaryPeer = 0;
    }

    LOG_DEBUG("evaluateOutboundGraphPath: primaryPeer=" << ToStr(primaryPeer) << ", secondaryPeer=" << ToStr(
                secondaryPeer));

    Uint16 neighborCount = 0;

    //TODO: save cost in path; take the first x(read from settings) ordered ascending by cost
    //replace secondary with a new one and keep the primary unchanged
    //if rerouteOnFail, start the evaluation only if the secondaryPeer == 0
    if (primaryPeer > 0 && (!rerouteOnlyOnFail || secondaryPeer == 0)) {
        Node& primaryPeerNode = subnetTopology.getNode(primaryPeer);
        Uint16 checkGraphId = primaryPeerNode.getOutboundGraphId();

        EdgeList& destinationEdges = primaryPeerNode.getOutBoundEdges();
        neighborCount = 0;

        Path& checkPath = subnetTopology.getPath(checkGraphId);
        minCheckPath.clear();
        uint32_t checkCost = checkPath.getMinCostPath(minCheckPath, false, destination);

        //test primary peer neighbors
        for (EdgeList::iterator it = destinationEdges.begin(); it != destinationEdges.end(); ++it) {
            //LOG_DEBUG("evaluateOutboundGraphPath - primaryPeer edge =" << *it);

            if (it->getStatus() == Status::DELETED || it->getDestination() == destination) {
                //LOG_DEBUG("evaluateOutboundGraphPath continue if primaryPeer 1");
                continue;
            }

            if (!subnetTopology.existsNode(it->getDestination()))
            {
                continue;
            }

            Node& peerNode = subnetTopology.getNode(it->getDestination());
            if (!peerNode.isRouter()) {
                //LOG_DEBUG("evaluateOutboundGraphPath continue if primaryPeer 2");
                continue;
            }

            if (destinationNode.existsGraphNeighbor(peerNode.getOutboundGraphId())) {
                LOG_DEBUG("evaluateOutboundGraphPath continue: the graph exists on destination");
//                continue;
            }

            if (neighborCount == maxNeighbors) {
                break;
            }
            else {
                neighborCount++;
            }

            cost = checkOutbound(primaryPeer, it->getDestination(), checkGraphId);
            if (cost != MAX_COST_32) {
                //TODO: cost max value is on 32 and on 16
                // add the cost of the peer's graph
                uint32_t graphCost = MAX_COST_32;
                Node& candidateNode = subnetTopology.getNode(it->getDestination());
                Path& candidatePath = subnetTopology.getPath(candidateNode.getOutboundGraphId());

                minCandidatePath.clear();

                uint32_t candidateCost = candidatePath.getMinCostPath(minCandidatePath, false, destination);

                if (candidateCost != MAX_COST_32 && checkCost != MAX_COST_32) {
                    graphCost = candidateCost+checkCost;
                } else {
                    graphCost = MAX_COST_32;
                }

                if (graphCost != MAX_COST_32) {
                    cost += graphCost;
                } else {
                    cost = MAX_COST_32;
                }
            }

            LOG_DEBUG("evaluateOutboundGraphPath - cost =" << ToStr(cost));

            if (cost < primaryMinCost) {
                LOG_DEBUG("evaluateOutboundGraphPath -primaryPeer found min for " << ToStr(it->getDestination())
                            << ", min=" << (int) cost);

                primaryMinPeer = it->getDestination();
                primaryMinCost = cost;
                primaryCandidatePath = minCandidatePath;
                primaryCheckPath = minCheckPath;
            }
        }
    }

    //TODO: save cost in path; take the first x(read from settings) ordered ascending by cost
    //replace primary with secondary and secondary with a new one
    //if rerouteOnFail, start the evaluation only if the primaryPeer == 0
    if (secondaryPeer > 0 && (!rerouteOnlyOnFail || primaryPeer == 0)) {
        Uint32 tmpCost = evaluateCost(secondaryPeer, subnetTopology.setNextSearchId());

        Node& secondaryPeerNode = subnetTopology.getNode(secondaryPeer);
        Uint16 checkGraphId = secondaryPeerNode.getOutboundGraphId();

        //TODO: read from settings, percents cost cheaper to switch to secondary
        Uint8 factor = 20;
        neighborCount = 0;
        //if primaryMinCost < tmpCost no reason to switch to secondary
        if (primaryMinCost == MAX_COST_32 || (primaryMinCost * (100 + factor)) / 100 > tmpCost) {
            EdgeList& destinationEdges = secondaryPeerNode.getOutBoundEdges();

            Path& checkPath = subnetTopology.getPath(checkGraphId);
            minCheckPath.clear();
            uint32_t checkCost = checkPath.getMinCostPath(minCheckPath, false, destination);

            for (EdgeList::iterator it = destinationEdges.begin(); it != destinationEdges.end(); ++it) {
                //LOG_DEBUG("evaluateOutboundGraphPath - secondaryPeer edge =" << *it);

                if (it->getStatus() == Status::DELETED || it->getDestination() == destination) {
                    //LOG_DEBUG("evaluateOutboundGraphPath continue if secondaryPeer 1");
                    continue;
                }

                if (!subnetTopology.existsNode(it->getDestination()))
                {
                    continue;
                }

                Node& peerNode = subnetTopology.getNode(it->getDestination());
                if (!peerNode.isRouter()) {
                    //LOG_DEBUG("evaluateOutboundGraphPath continue if secondaryPeer 2");
                    continue;
                }

                if (destinationNode.existsGraphNeighbor(peerNode.getOutboundGraphId())) {
                    LOG_DEBUG("evaluateOutboundGraphPath continue: the graph exists on destination");
                    //continue;
                }
                if (neighborCount == maxNeighbors) {
                    break;
                }
                else {
                    neighborCount++;
                }

                cost = checkOutbound(secondaryPeer, it->getDestination(), checkGraphId);

                if (cost != MAX_COST_32) {
                    //TODO: check eval edge cost, it is on other graph

                    uint32_t graphCost = MAX_COST_32;

                    Node& candidateNode = subnetTopology.getNode(it->getDestination());
                    Path& candidatePath = subnetTopology.getPath(candidateNode.getOutboundGraphId());

                    minCandidatePath.clear();

                    uint32_t candidateCost = candidatePath.getMinCostPath(minCandidatePath, false, destination);

                    if (candidateCost != MAX_COST_32 && checkCost != MAX_COST_32) {
                        graphCost = candidateCost+checkCost;
                    } else {
                        graphCost = MAX_COST_32;
                    }

                    if (graphCost != MAX_COST_32) {
                        cost += graphCost;
                    } else {
                        cost = MAX_COST_32;
                    }
                }

                if (cost < secondaryMinCost) {
                    LOG_DEBUG("evaluateOutboundGraphPath -secondaryPeer found min for " << ToStr(it->getDestination())
                                << ", min=" << (int) cost);

                    secondaryMinPeer = it->getDestination();
                    secondaryMinCost = cost;
                    secondaryCandidatePath = minCandidatePath;
                    secondaryCheckPath = minCheckPath;
                }
            }

        }

    }

    if (secondaryMinCost < primaryMinCost) {
        LOG_DEBUG("evaluateOutboundGraphPath - secondaryPeer is minim");
        cost = secondaryMinCost;

        setOutbound(engineOperations, secondaryPeer, secondaryMinPeer, subnetTopology.setNextSearchId(),
                    secondaryCheckPath, secondaryCandidatePath);

        destinationNode.setPrimaryOutbound(secondaryPeer);
        destinationNode.setSecondaryOutbound(secondaryMinPeer);
    } else if (primaryMinCost != MAX_COST_32 || newGraph) {
        LOG_DEBUG("evaluateOutboundGraphPath - primaryPeer is minim");
        cost = primaryMinCost;

        if (secondaryPeer != primaryMinPeer || newGraph) {
            setOutbound(engineOperations, primaryPeer, primaryMinPeer, subnetTopology.setNextSearchId(),
                        primaryCheckPath, primaryCandidatePath);

            destinationNode.setPrimaryOutbound(primaryPeer);
            destinationNode.setSecondaryOutbound(primaryMinPeer);
        } else {
            LOG_DEBUG("evaluateOutboundGraphPath - primaryPeer is minim, but no changes");
            return false;
        }
    }

    return true;
}

Uint32 SelectMinPath::checkOutbound(Address32 primaryPeer, Address32 candidatePeer, Uint16 checkGraphId) {
    LOG_DEBUG("checkOutbound - candidatePeer=" << ToStr(candidatePeer) << ", checkGraphId=" << ToStr(checkGraphId));

    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();

    Uint32 crtCost = 0;
    Uint32 minCost = MAX_COST_32;

    //TODO:add link from primary secondary to secondary

    if (candidatePeer == 0) {
        return minCost;
    }

    Node& candidateNode = subnetTopology.getNode(candidatePeer);
    EdgeList& candidateEdges = candidateNode.getOutBoundEdges();

    if (candidateNode.hasNeighborOnGraph(primaryPeer, checkGraphId) && candidateNode.getNodeType() == NodeType::NODE) {
        //LOG_DEBUG("checkOutbound - SKIP reverse neighbor between primaryPeer to secondaryPeer");
        return minCost;
    }

    for (EdgeList::iterator it = candidateEdges.begin(); it != candidateEdges.end(); ++it) {
        //LOG_DEBUG("checkOutbound - edge" << *it);

        if (it->getDestination() != destination || it->getStatus() == Status::DELETED) {
            continue;
        }

        if (!subnetTopology.existsNode(it->getDestination()))
        {
            continue;
        }

        Node& destinationNode = subnetTopology.getNode(it->getDestination());

        if (destinationNode.hasNeighborOnGraph(candidatePeer, checkGraphId)) {
            //LOG_DEBUG("checkOutbound - SKIP reverse neighbor");
            return minCost;
        }

        crtCost = it->evaluateEdgeCost(checkGraphId, traffic);

        //LOG_DEBUG("checkOutbound - crtCost=" << ToStr(crtCost));

        if (crtCost < minCost && crtCost != MAX_COST_16) {
            Uint16 settingsCost = candidateNode.getSettingsCost();
            if (settingsCost != MAX_COST_16) {
                minCost = crtCost + settingsCost;
            }
        }

        return minCost;
    }

    return minCost;
}

Uint32 SelectMinPath::evaluateCost(Address32 address, Uint16 searchId, bool testCycle) {

    bool completeGraph = false;
    Uint16 costGraphId;

    cost = 0;
    pathCost = 0;
    nodes = 0;
    redundantNodes = 0;
    activeEdges = 0;
    settingsCost = 0;
    hopsCost = 0;

    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();

    Node& node = subnetTopology.getNode(address);
    if (source == MANAGER_ADDRESS || source == GATEWAY_ADDRESS) {
        costGraphId = node.getOutboundGraphId();
    } else {
        costGraphId = node.getInboundGraphId();
    }

    Path& path = subnetTopology.getPath(costGraphId);
    evalSource = path.getSource();
    evalDestination = path.getDestination();

    LOG_DEBUG("evaluateCost - address=" << ToStr(address) << path);

    completeGraph = evaluateCostSearch(evalSource, costGraphId, searchId, 0, testCycle);

    if (!completeGraph) {
        LOG_ERROR("evaluateCostSearch : interrupted graph: " << int (costGraphId));
        return MAX_COST_32;
    }

    if (activeEdges == 0) {
        activeEdges = 1;
        LOG_DEBUG("evaluateCostSearch - activeEdges is 0 for: " << ToStr(evalSource));
    }

    if (nodes == 0) {
        nodes = 1;
        LOG_ERROR("evaluateCostSearch - nodes is 0");
    }

    if ( pathCost == MAX_COST_32 || settingsCost == MAX_COST_16 || hopsCost == MAX_COST_16 )
    {
        cost = MAX_COST_32;
    }
    else
    {
        hopsCost = nodes * nodes;

        if ( path.getEvaluatePathPriority() == EvaluatePathPriority::InboundDiscovery
                    || path.getEvaluatePathPriority() == EvaluatePathPriority::OutboundDiscovery ) {
            cost = topologyFactor * pathCost + settingsFactor * settingsCost + hopsFactor * hopsCost;
        }
        else {
            cost = topologyFactor * pathCost + settingsFactor * settingsCost;
        }
    }

    //SMState::WebLogger::Instance().PublishOverTCPIP("PathCost", pathCost, "SettingsCost", settingsCost, "HopsCost", hopsCost);

    LOG_DEBUG("evaluateCostSearch : cost = " << cost << ", pathCost=" << pathCost << ", topologyFactor="
                << topologyFactor << " nodes=" << nodes << " edges=" << activeEdges << " redundantNodes="
                << redundantNodes << " settingCost=" << settingsCost << ", settingsFactor=" << settingsFactor
                << " hopsCost=" << hopsCost << " hopsFactor=" << (int)hopsFactor);

    return cost;
}

bool SelectMinPath::evaluateCostSearch(Address32 address, Uint16 costGraphId, Uint16 searchId, Uint16 lastHopCount, bool testCycle) {

    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();

    Node& node = subnetTopology.getNode(address);

    bool completeGraph = false;
    Uint16 count = 0;
    Uint16 tempHop = lastHopCount;

    LOG_DEBUG("evaluateCostSearch : node=" << ToStr(address) << ",  pathCost=" << pathCost);

    if (node.getSearchId() == searchId) {
        //LOG_DEBUG("evaluateCostSearch : return for node=" << ToStr(address));
        return false;
    } else {
        if (address != MANAGER_ADDRESS && address != GATEWAY_ADDRESS) {
            nodes++;
            tempHop++;
        }
        node.setSearchId(searchId);
    }

    LOG_DEBUG("evaluateCostSearch : node=" << ToStr(address) << ", cost=" << node.getSettingsCost());

    //TODO: - init the cost
    settingsCost += node.evaluateSettingsCost();

    if ( tempHop > maxHops ) {
        hopsCost = MAX_COST_16;
    }

    LOG_DEBUG("evaluateCostSearch : hops=" << (int)tempHop << " hopsCost=" << (int)hopsCost << " maxHops=" << (int)maxHops);

    if (address == evalDestination) {
//        LOG_DEBUG("evaluateCostSearch - set completeGraph = true");
        return true;
    }

    EdgeList& edges = node.getOutBoundEdges();

    for (EdgeList::iterator it = edges.begin(); it != edges.end(); ++it) {
//        LOG_DEBUG("evaluateCostSearch - edge =" << *it);

        if (it->getStatus() == Status::DELETED) {
//            LOG_DEBUG("evaluateCostSearch - SKIP DELETED");
            continue;
        }

        if (!it->existsGraphNeighbor(costGraphId)) {
//            LOG_DEBUG("evaluateCostSearch - no graph on edge");
            continue;
        }

        count++;

        if ((node.getNodeType() == NodeType::NODE) || ((it->getDestination() != MANAGER_ADDRESS)
                    && (it->getDestination() != GATEWAY_ADDRESS))) {
            activeEdges++;
        }

        if ( testCycle ) {
            if ( (source == address && source != MANAGER_ADDRESS ) ||
                        ((destination == address) && (source == MANAGER_ADDRESS || source == GATEWAY_ADDRESS)) ) {
                pathCost = MAX_COST_32;
            }
        }

        //protect pathCost against overflow
        if ( pathCost != MAX_COST_32 ) {
            pathCost += it->evaluateEdgeCost(costGraphId, traffic);
        }

//        LOG_DEBUG("evaluateCostSearch : pathCost=" << pathCost << " for " << "(" << ToStr(address) << ", " << ToStr(
//        it->getDestination()) << ")");


        bool result = evaluateCostSearch(it->getDestination(), costGraphId, searchId, tempHop, testCycle);

        completeGraph = completeGraph || result;

    }

    if (count > 1) {
        redundantNodes++;
    }

   //LOG_DEBUG("evaluateCostSearch : finished=" << ToStr(currentHop) << " hopsCost=" << maxHops);

    return completeGraph;
}

//used only by min path routing alg
void SelectMinPath::setMinimumInboundPath(EngineOperations& engineOperations, Address32 primaryPeer, Address32 secondaryPeer
            , AddressList& primaryPath, AddressList& secondaryPath) {

    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();

    bool secondaryPeerIsBackbone = false;
    minCostPath.clear();

    bool primFoundInSecPath = false;

    if (secondaryPeer > 0) {

        if (secondaryPath.size() > 1) {
            AddressList::iterator it_neigh = secondaryPath.begin();
            AddressList::iterator it_node = secondaryPath.begin();
            it_node++;

            for (;;) {
                Node& node = subnetTopology.getNode(*it_node);
                if (!node.hasNeighborOnGraph(*it_neigh, graphId)) {
                    node.addGraphNeighbor(engineOperations, graphId, traffic, *it_neigh, false);
                } else {
                    node.resetPrefferdNeighbor(graphId, *it_neigh);
                }

                if (*it_node == primaryPeer) {
                    primFoundInSecPath = true;
                }


                if (node.getNodeType() == NodeType::NODE) {
                    Node& peerNode = subnetTopology.getNode(*it_neigh);
                    peerNode.setEvalLazyParent(*it_node);
                }

                minCostPath.insert(pair<Address32, Address32>(*it_node, *it_neigh));
                it_node++;
                it_neigh++;
                if (it_node == secondaryPath.end()) {
                    break;
                }
            }
            Node& src = subnetTopology.getNode(source);
            if (!src.hasNeighborOnGraph(*it_neigh, graphId)) {
                src.addGraphNeighbor(engineOperations, graphId, traffic, *it_neigh, false);
            } else {
                src.resetPrefferdNeighbor(graphId, *it_neigh);
            }
            Node& parent = subnetTopology.getNode(*it_neigh);
            parent.setEvalLazyParent(src.getNodeAddress());
            if (parent.getNodeType() == NodeType::BACKBONE) {
                secondaryPeerIsBackbone = true;
            }

            minCostPath.insert(pair<Address32, Address32>(source, *it_neigh));

        }
    }


    if (primaryPeer > 0) {

        if (primaryPath.size() > 1) {

            AddressList::iterator it_neigh = primaryPath.begin();
            AddressList::iterator it_node = primaryPath.begin();
            it_node++;
            for (;;) {
                Node& node = subnetTopology.getNode(*it_node);
                if (!node.hasNeighborOnGraph(*it_neigh, graphId)) {
                    node.addGraphNeighbor(engineOperations, graphId, traffic, *it_neigh, false, true);
                }
                if (node.getNodeType() == NodeType::NODE) {
                    Node& peerNode = subnetTopology.getNode(*it_neigh);
                    peerNode.setEvalLazyParent(*it_node);
                }

                minCostPath.insert(pair<Address32, Address32>(*it_node, *it_neigh));
                it_node++;
                it_neigh++;
                if (it_node == primaryPath.end()) {
                    break;
                }
            }
            Node& src = subnetTopology.getNode(source);
            if (!src.hasNeighborOnGraph(*it_neigh, graphId)) {
                src.addGraphNeighbor(engineOperations, graphId, traffic, *it_neigh, true, true);
            }
            else {
                src.setPrefferdNeighbor(graphId, *it_neigh);
            }
            Node& parent = subnetTopology.getNode(*it_neigh);
            parent.setEvalLazyParent(src.getNodeAddress());
            minCostPath.insert(pair<Address32, Address32>(source, *it_neigh));

        }

    }

    std::map<Address32, Address32>::iterator it = minCostPath.begin();
    for (; it != minCostPath.end(); ++it) {
        LOG_DEBUG("Part of min: " << std::hex << it->first << "-" << it->second);
    }

}


void SelectMinPath::setInbound(EngineOperations& engineOperations, Address32 primaryPeer, Address32 secondaryPeer,
            AddressList& primaryPath, AddressList& secondaryPath) {

    LOG_DEBUG("setInbound - primaryPeer = " << ToStr(primaryPeer) << ",  secondaryPeer = " << ToStr(secondaryPeer));
    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();

    bool cycleTest = testForCycles(primaryPath, secondaryPath);

    if (cycleTest) {
        secondaryPeer = 0;
        LOG_INFO("Cycle detected");
    }

    setMinimumInboundPath(engineOperations, primaryPeer, secondaryPeer, primaryPath, secondaryPath);
    Path& path = subnetTopology.getPath(graphId);
    if (path.getEvaluatePathPriority() != EvaluatePathPriority::InboundAttach) {
        checkGraph(engineOperations, source, subnetTopology.setNextSearchId());
    }

    Node& src = subnetTopology.getNode(source);
    src.setPrimaryInbound(primaryPeer);
    src.setSecondaryInbound(secondaryPeer);
    if (destination == MANAGER_ADDRESS) {
        //generate clk src ops
        std::set<Address32> pathSoFar;
        pathSoFar.insert(source);
        if (!breakCSCycles(engineOperations, primaryPeer, pathSoFar)) {
            LOG_INFO("A CS Cycle that could not be broken has been found.");
        }
        src.generateClockSources(engineOperations, graphId);
    }
    LOG_DEBUG("setMinInbound - END");
}

bool SelectMinPath::breakCSCycles(EngineOperations& engineOperations, Address32 newPrimary, std::set<Address32>& nodesSoFar) {

    if (nodesSoFar.find(newPrimary) != nodesSoFar.end()) {
        return false;   // we have cycle
    }
    bool result = true;
    nodesSoFar.insert(newPrimary);
    try {
        Node& primary = NetworkEngine::instance().getSubnetTopology().getNode(newPrimary);

        if (primary.getPrimaryInbound() != 0 && primary.getPrimaryInbound() != primary.getBackboneAddress()) {
            if (!breakCSCycles(engineOperations, primary.getPrimaryInbound(), nodesSoFar)) {
                //CS cycle, but on primary, so return false until a secondary CS is reached back, and break that.
                return false;
            }
        }

        if (primary.getSecondaryClkSource() != 0 && primary.getSecondaryClkSource() != primary.getBackboneAddress()) {
            if (!breakCSCycles(engineOperations, primary.getSecondaryClkSource(), nodesSoFar)) {
                //CYCLE, remove this secondary
                LOG_INFO("Secondary clock source cycle detected: Removing Secondary CS from " << primary.getNodeAddress() << " to " << primary.getSecondaryClkSource());
                SetClockSourceOperation *op = new SetClockSourceOperation(primary.getNodeAddress(), primary.getSecondaryClkSource(), false);
                IEngineOperationPointer engineOp(op);
                engineOp->setDependency(WaveDependency::ELEVENTH);
                engineOperations.addOperation(engineOp);

                primary.resetSecondaryClkSource();
            }
        }
    } catch (...) {
        result = false;
    }

    nodesSoFar.erase(newPrimary);

    return result;
}

//used only by min path routing alg
void SelectMinPath::setMinimumOutboundPath(EngineOperations& engineOperations, Address32 primaryPeer, Address32 secondaryPeer
            , AddressList& primaryPath, AddressList& secondaryPath) {

    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();

    Node *primNode_p = NULL;
    bool PrimToSec = false;

    minCostPath.clear();


    if (primaryPeer > 0) {

        if (primaryPath.size() > 1) {

            AddressList::iterator it_neigh = primaryPath.begin();
            AddressList::iterator it_node = primaryPath.begin();
            it_node++;
            Node& primNode = subnetTopology.getNode(*it_neigh);
            primNode_p = &primNode;
            if (!primNode.hasNeighborOnGraph(destination, graphId)) {
                primNode.addGraphNeighbor(engineOperations, graphId, traffic, destination, true);
            }

            Node& dest = subnetTopology.getNode(destination);
            dest.setEvalLazyParent(*it_neigh);


            minCostPath.insert(pair<Address32, Address32>(primaryPeer, destination));

            for (;;) {
                Node& node = subnetTopology.getNode(*it_node);
                if (!node.hasNeighborOnGraph(*it_neigh, graphId)) {
                    node.addGraphNeighbor(engineOperations, graphId, traffic, *it_neigh, true);
                }
                Node& peerNode = subnetTopology.getNode(*it_neigh);
                peerNode.setEvalLazyParent(*it_node);

                minCostPath.insert(pair<Address32, Address32>(*it_node, *it_neigh));
                it_node++;
                it_neigh++;
                if (it_node == primaryPath.end()) {
                    break;
                }
            }

        }
    }


    if (secondaryPeer > 0) {

        if (secondaryPath.size() > 1) {
            AddressList::iterator it_neigh = secondaryPath.begin();
            AddressList::iterator it_node = secondaryPath.begin();
            it_node++;

            Node& node = subnetTopology.getNode(*it_neigh);

            if (!node.hasNeighborOnGraph(destination, graphId)) {
                node.addGraphNeighbor(engineOperations, graphId, traffic, destination, false);
            }
            Node& dest = subnetTopology.getNode(destination);
            dest.setEvalLazyParent(*it_neigh);
            bool primFoundInSecPath = false;
            minCostPath.insert(pair<Address32, Address32>(secondaryPeer, destination));
            for (;;) {
                Node& node = subnetTopology.getNode(*it_node);
                if (!node.hasNeighborOnGraph(*it_neigh, graphId)) {
                    node.addGraphNeighbor(engineOperations, graphId, traffic, *it_neigh, false);
                }
                Node& peerNode = subnetTopology.getNode(*it_neigh);
                peerNode.setEvalLazyParent(*it_node);

                if (*it_node == primaryPeer) {
                    primFoundInSecPath = true;
                }

                minCostPath.insert(pair<Address32, Address32>(*it_node, *it_neigh));
                it_node++;
                it_neigh++;
                if (it_node == secondaryPath.end()) {
                    break;
                }
            }
            if (PrimToSec && !primFoundInSecPath) {
                primNode_p->addGraphNeighbor(engineOperations, graphId, traffic, secondaryPeer, false);
                minCostPath.insert(pair<Address32, Address32>(primaryPeer, secondaryPeer));
            }
        }
    }

    std::map<Address32, Address32>::iterator it = minCostPath.begin();
    for (; it != minCostPath.end(); ++it) {
        LOG_DEBUG("Part of min: " << std::hex << it->first << "-" << it->second);
    }

}


//checks graph in order to remove old nodes and edges
void SelectMinPath::checkGraph(EngineOperations& engineOperations, Address32 address, Uint16 searchId) {
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

            //test if edge is in minPath map
            pair<multimap<Address32,Address32>::iterator,multimap<Address32,Address32>::iterator> ret;
            ret = minCostPath.equal_range(it->getSource());
            std::multimap<Address32, Address32>::iterator it_find = ret.first;
            bool foundEdge = false;
            for (; it_find != ret.second; ++it_find) {
                if (it_find->second == it->getDestination()) {
                    foundEdge = true;
                    break;
                }
            }
            if ( !foundEdge ) {
                node.setRemoveGraphNeighbor(engineOperations, graphId, it->getDestination());
            }

            checkGraph(engineOperations, it->getDestination(), searchId);
        }

    }
}

void SelectMinPath::setOutbound(EngineOperations& engineOperations, Address32 primaryPeer, Address32 secondaryPeer,
            Uint16 searchId, AddressList& primaryPath, AddressList& secondaryPath) {

    //LOG_DEBUG("setOutbound - primaryPeer = " << ToStr(primaryPeer) << ",  secondaryPeer = " << ToStr(secondaryPeer));

    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();

    Node& primaryNode = subnetTopology.getNode(primaryPeer);

    evalGraphId = primaryNode.getOutboundGraphId();

    LOG_DEBUG("setOutbound - primaryPeer=" << ToStr(primaryPeer) << ", secondartPeer=" << ToStr(secondaryPeer)
                << ", evalGraphId = " << subnetTopology.getPath(evalGraphId));

    bool cycleTest = testForCycles(primaryPath, secondaryPath);

    if (cycleTest) {
        secondaryPeer = 0;
        LOG_INFO("Cycle detected");
    }

    evalPrimary = primaryPeer;
    evalSecondary = secondaryPeer;

    evalPrimarySecondary = primaryNode.getSecondaryOutbound();

    Path& path = subnetTopology.getPath(graphId);

    setMinimumOutboundPath(engineOperations, primaryPeer, secondaryPeer, primaryPath, secondaryPath);

    if (path.getEvaluatePathPriority() != EvaluatePathPriority::OutboundAttach) {
        checkGraph(engineOperations, source, subnetTopology.setNextSearchId());
    }
}

bool SelectMinPath::testForCycles(AddressList& primaryPath, AddressList& secPath) {

    if (secPath.size() < 2) {
        return false;
    }

    AddressList::iterator it_s = secPath.end();
    it_s--;
    AddressList::iterator it_d = it_s;
    it_d--;

    Uint8 primLevel;
    Uint8 secLevel = 1;
    do {
        //do not test manager and gateway (common in all paths)
        if (*it_d != MANAGER_ADDRESS && *it_d != GATEWAY_ADDRESS) {
            //search if edge dest is part of primary path
            primLevel = 0;
            AddressList::iterator it_last = primaryPath.end();
            it_last--;
            for (AddressList::iterator it = it_last; it != primaryPath.begin(); --it) {
                //test if node exist in primary path
                if (*it_d == *it) {
                    //test if level is lower (secPath edge has dest on a lower level in primPath - probable cycle)
                    if (primLevel <= secLevel) {
                        //test if its dest returns on a lower level in secPath
                        AddressList::iterator it_p_d = it;
                        it_p_d--;
                        AddressList::iterator its = secPath.end();
                        its--;
                        AddressList::iterator it_stop = it_d;
                        it_stop--;
                        for (;its != it_stop; its--) {
                            if (*its == *it_p_d) {
                                return true;
                            }
                        }
                    }

                }
                primLevel++;
            }
        }
        it_d--;
        secLevel++;
    } while (it_d != secPath.begin());

    return false;
}


Uint16 SelectMinPath::checkEdge(Node& src, Node& dest) {
    try {
        Edge& edge = src.getEdge(dest.getNodeAddress());
        Uint16 edgeCost = edge.evaluateEdgeCost(graphId, traffic);
        return edgeCost;
    } catch (...) {
        return MAX_COST_16;
    }

}
