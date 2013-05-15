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
 * SelectBestPeer.cpp
 *
 *  Created on: Jan 27, 2010
 *      Author: ioanpocol
 */

#include "SelectBestPeer.h"

#include "Common/NEAddress.h"

#include "Model/Topology/Edge.h"
#include "Model/Topology/Path.h"
#include "Model/Topology/Node.h"
#include "Model/NetworkEngine.h"

using namespace NE::Model::Topology::Algorithms;

SelectBestPeer::SelectBestPeer() {
    settingsFactor = NE::Model::NetworkEngine::instance().getSettingsLogic().settingsFactor;
    topologyFactor = NE::Model::NetworkEngine::instance().getSettingsLogic().topologyFactor;
    hopsFactor = NE::Model::NetworkEngine::instance().getSettingsLogic().hopsFactor;
    maxHops = NE::Model::NetworkEngine::instance().getSettingsLogic().maxHops;
    maxNeighbors = NE::Model::NetworkEngine::instance().getSettingsLogic().maxNeighbors;
    rerouteOnlyOnFail = (bool) NE::Model::NetworkEngine::instance().getSettingsLogic().rerouteOnlyOnFail;

}

SelectBestPeer::~SelectBestPeer() {
}

bool SelectBestPeer::evaluateGraphPath(EngineOperations& engineOperations, Path& path) {
    bool result;

    LOG_INFO("evaluateGraphPath - START for graph =" << path);

    source = path.getSource();
    destination = path.getDestination();
    graphId = path.getGraphId();
    traffic = path.getTraffic();

    if (path.getSource() == MANAGER_ADDRESS || path.getSource() == GATEWAY_ADDRESS) {
        result = evaluateOutboundGraphPath(engineOperations);
    } else {
        result = evaluateInboundGraphPath(engineOperations);
    }

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

bool SelectBestPeer::evaluateInboundGraphPath(EngineOperations& engineOperations) {

    //LOG_INFO("evaluateInboundGraphPath");

    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();

    Address32 primaryPeer = 0;
    Address32 primaryPrimaryPeer = 0;
    Address32 primarySecondaryPeer = 0;
    Address32 primaryMinPeer = 0;

    Address32 secondaryPeer = 0;
    Address32 secondaryPrimaryPeer = 0;
    Address32 secondarySecondaryPeer = 0;
    Address32 secondaryMinPeer = 0;

    Uint32 cost = MAX_COST_32;
    Uint32 primaryMinCost = MAX_COST_32;
    Uint32 secondaryMinCost = MAX_COST_32;
    bool newGraph = false;

    Node& sourceNode = subnetTopology.getNode(source);
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

            if (!subnetTopology.existsActivePath(tmpGraphId)) {
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

            if (!subnetTopology.existsActivePath(tmpGraphId)) {
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

    Uint16 neighborCount = 0;

    //TODO: select best x neighbors
    //replace secondary with a new one and keep the primary unchanged
    //if rerouteOnFail, start the evaluation only if the seccondPeer == 0
    if (primaryPeer > 0 && (!rerouteOnlyOnFail || secondaryPeer == 0)) {
        Address32 checkPrimaryPeer = 0;
        Address32 checkSecondaryPeer = 0;

        Uint16 checkGraphId = subnetTopology.getNode(primaryPeer).getInboundGraphId();
        neighborCount = 0;

        for (EdgeList::iterator it = sourceEdges.begin(); it != sourceEdges.end(); ++it) {

            checkPrimaryPeer = 0;
            checkSecondaryPeer = 0;

            LOG_DEBUG("evaluateInboundGraphPath - primaryPeer edge =" << *it);

            if (it->getStatus() == Status::DELETED || it->getDestination() == primaryPeer) {
                LOG_DEBUG("evaluateInboundGraphPath continue: is deleted or primaryPeer");
                continue;
            }

            if (!subnetTopology.existsNode(it->getDestination()))
            {
                LOG_DEBUG("evaluateInboundGraphPath - edge destination does not exist!");
                continue;
            }

            Node& peerNode = subnetTopology.getNode(it->getDestination());
            if (!peerNode.isRouter()) {
                LOG_DEBUG("evaluateInboundGraphPath continue: is no router");
                continue;
            }

            if (sourceNode.existsGraphNeighbor(peerNode.getInboundGraphId())) {
                LOG_DEBUG("evaluateInboundGraphPath continue: the graph exists on source");
                continue;
            }

            if (neighborCount == maxNeighbors) {
                break;
            }
            else {
                neighborCount++;
            }

            cost = checkInbound(primaryPeer, it->getDestination(), checkGraphId, checkPrimaryPeer,
                            checkSecondaryPeer);

            LOG_DEBUG("evaluateInboundGraphPath - cost =" << ToStr(cost));

            if (cost != MAX_COST_32) {
                try
                {
                    uint32_t graphCost = evaluateCost(it->getDestination(), subnetTopology.setNextSearchId());
                    if (graphCost != MAX_COST_32) {
                        cost += graphCost;
                    } else {
                        cost = MAX_COST_32;
                    }
                }
                catch (...) {
                    cost = MAX_COST_32;
                }

                if (cost < primaryMinCost * (100.0 - NetworkEngine::instance().getSettingsLogic().percentCostLower) / 100) {
                    LOG_DEBUG("evaluateInboundGraphPath -primaryPeer found min for " << ToStr(it->getDestination())
                                << ", min=" << (int) cost << ", primaryPrimaryPeer=" << ToStr(checkPrimaryPeer)
                                << ", primarySecondaryPeer=" << ToStr(checkSecondaryPeer));

                    primaryMinPeer = it->getDestination();
                    primaryMinCost = cost;

                    primaryPrimaryPeer = checkPrimaryPeer;
                    primarySecondaryPeer = checkSecondaryPeer;
                }
            }
        }

    }

    //replace primary with secondary and secondary with a new one
    //if rerouteOnFail, start the evaluation only if the primaryPeer == 0
    if (secondaryPeer > 0 && (!rerouteOnlyOnFail || primaryPeer == 0) &&
                !createsClockSourceCycle(secondaryPeer, true)) {
        Address32 checkPrimaryPeer = 0;
        Address32 checkSecondaryPeer = 0;

        Uint32 tmpCost = evaluateCost(secondaryPeer, subnetTopology.setNextSearchId());
        Uint16 checkGraphId = subnetTopology.getNode(secondaryPeer).getInboundGraphId();

        //TODO: read from settings, percents cost cheaper to switch to secondary
        Uint8 factor = 20;

        neighborCount = 0;

        if ((primaryMinCost * (100 + factor)) / 100 > tmpCost) {

            for (EdgeList::iterator it = sourceEdges.begin(); it != sourceEdges.end(); ++it) {
                checkPrimaryPeer = 0;
                checkSecondaryPeer = 0;

                //LOG_DEBUG("evaluateInboundGraphPath - secondaryPeer edge =" << *it);

                if (it->getStatus() == Status::DELETED || it->getDestination() == secondaryPeer) {
                    //LOG_DEBUG("evaluateInboundGraphPath continue if secondaryPeer 1");
                    continue;
                }

                if (!subnetTopology.existsNode(it->getDestination()))
                {
                    //LOG_DEBUG("evaluateInboundGraphPath continue - edge has deleted destination");
                    continue;
                }

                Node& peerNode = subnetTopology.getNode(it->getDestination());
                if (!peerNode.isRouter()) {
                    //LOG_DEBUG("evaluateInboundGraphPath continue if primaryPeer 2");
                    continue;
                }

                if (sourceNode.existsGraphNeighbor(peerNode.getInboundGraphId())) {
                    LOG_DEBUG("evaluateInboundGraphPath continue: the graph exists on source");
                    continue;
                }

                if (neighborCount == maxNeighbors) {
                    break;
                }
                else {
                    neighborCount++;
                }

                cost = checkInbound(secondaryPeer, it->getDestination(), checkGraphId, checkPrimaryPeer,
                                checkSecondaryPeer);

                LOG_DEBUG("evaluateInboundGraphPath - cost =" << ToStr(cost));

                //TODO: check eval edge cost; it is on other graph
                if (cost != MAX_COST_32) {
                    //cost += it->evaluateEdgeCost(checkGraphId, traffic);

                    if (cost < secondaryMinCost * (100.0 - NetworkEngine::instance().getSettingsLogic().percentCostLower) / 100) {
                        LOG_DEBUG("evaluateInboundGraphPath -primaryPeer found min=" << (int) cost << ", for " << ToStr(
                                    it->getDestination()) << ", secondaryPrimaryPeer=" << ToStr(checkPrimaryPeer)
                                    << ", secondarySecondaryPeer=" << ToStr(checkSecondaryPeer));

                        secondaryMinPeer = it->getDestination();
                        secondaryMinCost = cost;

                        secondaryPrimaryPeer = checkPrimaryPeer;
                        secondarySecondaryPeer = checkSecondaryPeer;
                    }
                }
            }
        }

        if (secondaryMinPeer != 0) {
            secondaryMinCost += tmpCost;
        }
    }

    if (secondaryMinCost < primaryMinCost) {
        LOG_DEBUG("evaluateInboundGraphPath - secondaryPeer is minim");
        cost = secondaryMinCost;

        setInbound(engineOperations, secondaryPeer, secondaryMinPeer, secondaryPrimaryPeer, secondarySecondaryPeer,
                    subnetTopology.setNextSearchId());

        sourceNode.setPrimaryInbound(secondaryPeer);
        sourceNode.setSecondaryInbound(secondaryMinPeer);

    } else {
        LOG_DEBUG("evaluateInboundGraphPath - primaryPeer is minim");
        cost = primaryMinCost;

        if (secondaryPeer != primaryMinPeer || newGraph) {
            setInbound(engineOperations, primaryPeer, primaryMinPeer, primaryPrimaryPeer, primarySecondaryPeer,
                        subnetTopology.setNextSearchId());

            sourceNode.setPrimaryInbound(primaryPeer);
            sourceNode.setSecondaryInbound(primaryMinPeer);
        } else {
            LOG_DEBUG("evaluateInboundGraphPath - primaryPeer is minim, but no changes");
            return false;
        }
    }


    return true;
}

bool SelectBestPeer::createsClockSourceCycle(Address32 secondaryNode, bool checkSecondary) {
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

bool SelectBestPeer::evaluateOutboundGraphPath(EngineOperations& engineOperations) {
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

    Node& destinationNode = subnetTopology.getNode(destination);

    primaryPeer = destinationNode.getPrimaryOutbound();
    secondaryPeer = destinationNode.getSecondaryOutbound();

    if (primaryPeer == 0 && secondaryPeer == 0) {
        LOG_INFO("evaluateOutboundGraphPath - not found primary and secondary");
        primaryPeer = destinationNode.getParentAddress();
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
                continue;
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

                graphCost = evaluateCost(it->getDestination(), subnetTopology.setNextSearchId());

                //TODO andy review here, something's fishy
                if (graphCost != MAX_COST_32) {
                    cost += graphCost;
                } else {
                    cost = MAX_COST_32;
                }
                //                cost += it->evaluateEdgeCost(destinationNode.getOutboundGraphId(), traffic);
            }

            LOG_DEBUG("evaluateOutboundGraphPath - cost =" << ToStr(cost));

            if (cost < primaryMinCost) {
                LOG_DEBUG("evaluateOutboundGraphPath -primaryPeer found min for " << ToStr(it->getDestination())
                            << ", min=" << (int) cost);

                primaryMinPeer = it->getDestination();
                primaryMinCost = cost;
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
                    continue;
                }
                if (neighborCount == maxNeighbors) {
                    break;
                }
                else {
                    neighborCount++;
                }

                cost = checkOutbound(secondaryPeer, it->getDestination(), checkGraphId);

                if (cost != MAX_COST_32) {
                    uint32_t evalCost = evaluateCost(it->getDestination(), subnetTopology.setNextSearchId());
                    if (evalCost != MAX_COST_32)
                    {
                        cost += evalCost;
                    }
                    else
                    {
                        cost = MAX_COST_32;
                    }
                }

                if (cost < secondaryMinCost) {
                    LOG_DEBUG("evaluateOutboundGraphPath -secondaryPeer found min for " << ToStr(it->getDestination())
                                << ", min=" << (int) cost);

                    secondaryMinPeer = it->getDestination();
                    secondaryMinCost = cost;
                }
            }

        }
    }

    if (secondaryMinCost < primaryMinCost) {
        LOG_DEBUG("evaluateOutboundGraphPath - secondaryPeer is minim");
        cost = secondaryMinCost;

        setOutbound(engineOperations, secondaryPeer, secondaryMinPeer, subnetTopology.setNextSearchId());

        destinationNode.setPrimaryOutbound(secondaryPeer);
        destinationNode.setSecondaryOutbound(secondaryMinPeer);
    } else {
        LOG_DEBUG("evaluateOutboundGraphPath - primaryPeer is minim");
        cost = primaryMinCost;

        if (secondaryPeer != primaryMinPeer || newGraph) {
            setOutbound(engineOperations, primaryPeer, primaryMinPeer, subnetTopology.setNextSearchId());

            destinationNode.setPrimaryOutbound(primaryPeer);
            destinationNode.setSecondaryOutbound(primaryMinPeer);
        } else {
            LOG_DEBUG("evaluateOutboundGraphPath - primaryPeer is minim, but no changes");
            return false;
        }
    }

    return true;
}

Uint32 SelectBestPeer::checkInbound(Address32 primaryPeer, Address32 candidatePeer, Uint16 checkGraphId,
            Address32& primaryCandidatePeer, Address32& secondaryCandidatePeer) {

    LOG_DEBUG("checkInbound - primaryPeer=" << ToStr(primaryPeer) << ", candidatePeer=" << ToStr(candidatePeer)
                << ", checkGraphId=" << ToStr(checkGraphId));

    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();

    Uint32 cost = MAX_COST_32;

    Uint32 primaryCandidateCost = MAX_COST_32;
    Uint32 secondaryCandidateCost = MAX_COST_32;

    primaryCandidatePeer = 0;
    secondaryCandidatePeer = 0;

    if (candidatePeer == 0) {
        return cost;
    }

    //TODO: a cost to choose between a direct link, other node with single or redundant path
    Node& candidateNode = subnetTopology.getNode(candidatePeer);

    if (candidateNode.hasNeighborOnGraph(source, checkGraphId)) {
        return cost;
    }

    if (candidateNode.existsGraphNeighbor(checkGraphId)) {
        return 0;
    }

    EdgeList& candidateEdges = candidateNode.getOutBoundEdges();

    for (EdgeList::iterator it = candidateEdges.begin(); it != candidateEdges.end(); ++it) {
        //LOG_DEBUG("checkInbound - edge:  " << *it);
        if (it->getStatus() == Status::DELETED || it->getDestination() == source) {
            //LOG_DEBUG("checkInbound - SKIP deleted");
            continue;
        }

        if (!subnetTopology.existsNode(it->getDestination()))
        {
            continue;
        }

        Node& peerNode = subnetTopology.getNode(it->getDestination());

        if (peerNode.hasNeighborOnGraph(it->getSource(), checkGraphId)) {
            //LOG_DEBUG("checkInbound - SKIP reverse neighbor");
            continue;
        }

        if (!peerNode.existsGraphNeighbor(checkGraphId)) {
            //LOG_DEBUG("checkInbound - SKIP no neighbor on graph");
            continue;
        }

        //TODO: evaluate the cost including the peer graph cost
        Uint32 edgeCost = it->evaluateEdgeCost(checkGraphId, traffic);

        if (edgeCost < primaryCandidateCost) {
            secondaryCandidatePeer = primaryCandidatePeer;
            secondaryCandidateCost = primaryCandidateCost;
            primaryCandidatePeer = it->getDestination();
            primaryCandidateCost = edgeCost;

            LOG_DEBUG("checkInbound - primaryCandidatePeer=" << ToStr(primaryCandidatePeer));
        } else if (edgeCost < secondaryCandidateCost) {
            secondaryCandidatePeer = it->getDestination();
            secondaryCandidateCost = edgeCost;

            LOG_DEBUG("checkInbound - secondaryCandidatePeer=" << ToStr(secondaryCandidatePeer));
        }
    }

    if (primaryCandidatePeer != 0) {
        cost = candidateNode.getSettingsCost() + primaryCandidateCost;
    }

    if (secondaryCandidatePeer != 0) {
        cost += secondaryCandidateCost;
    }

    return cost;
}

Uint32 SelectBestPeer::checkOutbound(Address32 primaryPeer, Address32 candidatePeer, Uint16 checkGraphId) {
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

        if (crtCost < minCost) {
            minCost = crtCost + candidateNode.getSettingsCost();
        }

        return minCost;
    }

    return minCost;
}

Uint32 SelectBestPeer::evaluateCost(Address32 address, Uint16 searchId, bool testCycle) {

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

    //LOG_DEBUG("evaluateCost - address=" << ToStr(address) << path);

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

    LOG_DEBUG("evaluateCostSearch : cost = " << cost << ", pathCost=" << pathCost << ", topologyFactor="
                << topologyFactor << " nodes=" << nodes << " edges=" << activeEdges << " redundantNodes="
                << redundantNodes << " settingCost=" << settingsCost << ", settingsFactor=" << settingsFactor
                << " hopsCost=" << hopsCost << " hopsFactor=" << (int)hopsFactor);

    return cost;
}

bool SelectBestPeer::evaluateCostSearch(Address32 address, Uint16 costGraphId, Uint16 searchId, Uint16 lastHopCount, bool testCycle) {

    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();

    Node& node = subnetTopology.getNode(address);

    bool completeGraph = false;
    Uint16 count = 0;
    Uint16 tempHop = lastHopCount;

    //LOG_DEBUG("evaluateCostSearch : node=" << ToStr(address) << ",  pathCost=" << pathCost);

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

//    LOG_DEBUG("evaluateCostSearch : node=" << ToStr(address) << ", cost=" << node.getSettingsCost());

    //TODO: - init the cost
    //settingsCost += crtNode.getSettingsCost();
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

        //TODO: init the cost
        //pathCost += it->getEvalEdgeCost();
        //TODO: check eval edge cost, if it is on secondary costGraphId != graphId
        //mihai: set maximum value for the pathCost if source/destination node (source/destination) exists in
        //the evaluated graph (costGraphId)
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

void SelectBestPeer::setInbound(EngineOperations& engineOperations, Address32 primaryPeer, Address32 secondaryPeer,
            Address32 secondaryPrimaryPeer, Address32 secondarySecondaryPeer, Uint16 searchId) {

    LOG_DEBUG("setInbound - primaryPeer = " << ToStr(primaryPeer) << ",  secondaryPeer = " << ToStr(secondaryPeer)
                << ", secondaryPrimaryPeer=" << ToStr(secondaryPrimaryPeer) << ", secondarySecondaryPeer=" << ToStr(
                secondarySecondaryPeer));

    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();

    Node& primaryNode = subnetTopology.getNode(primaryPeer);

    evalGraphId = primaryNode.getInboundGraphId();

    LOG_DEBUG("setInbound - evalGraphId = " << subnetTopology.getPath(evalGraphId));

    evalPrimary = primaryPeer;
    evalSecondary = secondaryPeer;
    evalSecondaryPrimary = secondaryPrimaryPeer;
    evalSecondarySecondary = secondaryPrimaryPeer;

    setInboundSearch(engineOperations, source, source, true, 0, searchId);

    if (destination == MANAGER_ADDRESS) {
        // inbound route, add clock source
        std::set<Address32> pathSoFar;
        pathSoFar.insert(source);
        if (!breakCSCycles(engineOperations, primaryPeer, pathSoFar)) {
            LOG_INFO("A CS Cycle that could not be broken has been found.");
        }

        Node& node = NetworkEngine::instance().getSubnetTopology().getNode(source);
        node.generateClockSources(engineOperations, graphId);
    }
}

bool SelectBestPeer::breakCSCycles(EngineOperations& engineOperations, Address32 newPrimary, std::set<Address32>& nodesSoFar) {

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



void SelectBestPeer::setOutbound(EngineOperations& engineOperations, Address32 primaryPeer, Address32 secondaryPeer,
            Uint16 searchId) {

    //LOG_DEBUG("setOutbound - primaryPeer = " << ToStr(primaryPeer) << ",  secondaryPeer = " << ToStr(secondaryPeer));

    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();

    Node& primaryNode = subnetTopology.getNode(primaryPeer);

    evalGraphId = primaryNode.getOutboundGraphId();

    LOG_DEBUG("setOutbound - primaryPeer=" << ToStr(primaryPeer) << ", secondartPeer=" << ToStr(secondaryPeer)
                << ", evalGraphId = " << subnetTopology.getPath(evalGraphId));

    evalPrimary = primaryPeer;
    evalSecondary = secondaryPeer;

    evalPrimarySecondary = primaryNode.getSecondaryOutbound();

    setOutboundSearch(engineOperations, source, source, true, 0, searchId);
}

void SelectBestPeer::setInboundSearch(EngineOperations& engineOperations, Address32 address, Address32 parent,
            bool setLazy, Uint8 deep, Uint16 searchId) {

    //LOG_DEBUG("setInboundSearch - address=" << ToStr(address) << ", deep=" << (int) deep << ", searchId=" << searchId);

    if (address == destination) {
        return;
    }

    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();
    Node& node = subnetTopology.getNode(address);

    if (setLazy) {
        node.setEvalLazyParent(parent, deep, searchId);
    }

    if (node.getSearchId() == searchId) {
        return;
    } else {
        node.setSearchId(searchId);
    }

    EdgeList& edges = node.getOutBoundEdges();
    for (EdgeList::iterator it = edges.begin(); it != edges.end(); ++it) {
        //LOG_DEBUG("setInboundSearch - edge =" << *it);

        if (it->getStatus() == Status::DELETED) {
            //LOG_DEBUG("setInboundSearch -SKIP DELETED");
            continue;
        }

        setLazy = true;
        if (!it->existsGraphNeighbor(evalGraphId)) {
            if ((it->getSource() == source && it->getDestination() == evalPrimary) || (it->getSource() == source
                        && it->getDestination() == evalSecondary) || (it->getSource() == evalSecondary
                        && it->getDestination() == evalSecondaryPrimary) || (it->getSource() == evalSecondary
                        && it->getDestination() == evalSecondarySecondary)) {

                //source -> primary, source->secondary, secondary->secondaryPrimary, secondary->secondarySecondary are added

                if ((it->getSource() == source && it->getDestination() == evalSecondary) || (it->getSource()
                            == evalSecondary && it->getDestination() == evalSecondaryPrimary)) {
                    Node& secondaryNode = subnetTopology.getNode(evalSecondary);
                    if (secondaryNode.hasOtherInboundNeighborOnGraph(evalPrimary, evalGraphId)) {
                        continue;
                    }
                }

            } else {
                if (!it->existsGraphNeighbor(graphId)) {
                    continue;
                } else {
                    setLazy = false;
                }
            }
        }

        setInboundSearch(engineOperations, it->getDestination(), it->getSource(), setLazy, deep + 1, searchId);
    }

    for (EdgeList::iterator it = edges.begin(); it != edges.end(); ++it) {
        LOG_DEBUG("setInboundSearch - eval edge:" << *it);

        if (it->getStatus() == Status::DELETED) {
            //LOG_DEBUG("setInboundSearch -SKIP DELETED");
            continue;
        }

        bool primaryPreffered = (it->getSource() == source && it->getDestination() == evalPrimary);

        if (it->existsGraphNeighbor(evalGraphId)) {

            if (!it->existsGraphNeighbor(graphId)) {
                //LOG_DEBUG("setInboundSearch -ADD 1");
                node.addGraphNeighbor(engineOperations, graphId, traffic, it->getDestination(), it->getGraphOnEdge(
                            evalGraphId).preffered || primaryPreffered);
            } else {
                //LOG_DEBUG("setInboundSearch -Update");
                it->getGraphOnEdge(graphId).preffered = it->getGraphOnEdge(evalGraphId).preffered || primaryPreffered;
            }

        } else {
            bool add = false;
            bool preffered = true;

            if ((it->getSource() == source && it->getDestination() == evalPrimary) || (it->getSource() == source
                        && it->getDestination() == evalSecondary) || (it->getSource() == evalSecondary
                        && it->getDestination() == evalSecondaryPrimary) || (it->getSource() == evalSecondary
                        && it->getDestination() == evalSecondarySecondary)) {

                //source -> primary, source->secondary, secondary->secondaryPrimary, secondary->secondarySecondary are added

                if ((it->getSource() == source && it->getDestination() == evalSecondary) || (it->getSource()
                            == evalSecondary && it->getDestination() == evalSecondaryPrimary)) {
                    preffered = false;

                    Node& secondaryNode = subnetTopology.getNode(evalSecondary);
                    if (secondaryNode.hasOtherInboundNeighborOnGraph(evalPrimary, evalGraphId)) {
                        add = false;
                    }
                }

                add = true;
            } else {
                add = false;
            }

            if (add) {
                //LOG_DEBUG("setInboundSearch -ADD 2");
                if (it->existsGraphNeighbor(graphId)) {
                    it->getGraphOnEdge(graphId).preffered = it->getGraphOnEdge(graphId).preffered || primaryPreffered;
                } else {
                    node.addGraphNeighbor(engineOperations, graphId, traffic, it->getDestination(), preffered
                                || primaryPreffered);
                }
            } else if (!add && it->existsGraphNeighbor(graphId)) {
                //LOG_DEBUG("setInboundSearch -Remove");
                node.setRemoveGraphNeighbor(engineOperations, graphId, it->getDestination());
            }

        }
    }
}

void SelectBestPeer::setOutboundSearch(EngineOperations& engineOperations, Address32 address, Address32 parent,
            bool setLazy, Uint8 deep, Uint16 searchId) {

    //LOG_DEBUG("setOutboundSearch - address=" << ToStr(address) << ", deep=" << (int) deep << ", searchId=" << searchId);

    if (address == destination) {
        return;
    }

    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();
    Node& node = subnetTopology.getNode(address);

    if (setLazy) {
        node.setEvalLazyParent(parent, deep, searchId);
    }

    if (node.getSearchId() == searchId) {
        return;
    } else {
        node.setSearchId(searchId);
    }

    EdgeList& edges = node.getOutBoundEdges();
    for (EdgeList::iterator it = edges.begin(); it != edges.end(); ++it) {
        //LOG_DEBUG("setOutboundSearch - edge =" << *it);

        if (it->getStatus() == Status::DELETED) {
            //LOG_DEBUG("setOutboundSearch -SKIP DELETED");
            continue;
        }

        setLazy = true;
        if (!it->existsGraphNeighbor(evalGraphId)) {
            //LOG_DEBUG("setOutboundSearch -evalGraphId not found");
            if ((it->getSource() == evalPrimary && it->getDestination() == evalSecondary)
                        || (it->getSource() == evalPrimary && it->getDestination() == destination)
                        || (it->getSource() == evalSecondary && it->getDestination() == destination)
                        || (it->getSource() == evalPrimarySecondary && it->getDestination() == evalSecondary)) {

                //primary -> secondary, primary->destination, secondary->destination, primarySecondary->secondary are added

                if (it->getSource() == evalPrimary && it->getDestination() == evalSecondary) {
                    Node& secondaryNode = subnetTopology.getNode(evalSecondary);

                    if (secondaryNode.hasOtherOutboundNeighborOnGraph(destination, evalGraphId)) {
                        continue;
                    }
                }

                if (it->getSource() == evalPrimarySecondary && it->getDestination() == evalSecondary) {
                    Node& secondaryNode = subnetTopology.getNode(evalSecondary);

                    if (secondaryNode.hasOtherOutboundNeighborOnGraph(destination, evalGraphId) || secondaryNode.hasNeighborOnGraph(evalPrimarySecondary, evalGraphId)) {
                        continue;
                    }
                }

            } else {
                //LOG_DEBUG("setOutboundSearch -SKIP 1");
                if (!it->existsGraphNeighbor(graphId)) {
                    continue;
                } else {
                    setLazy = false;
                }
            }
        }

        setOutboundSearch(engineOperations, it->getDestination(), it->getSource(), setLazy, deep + 1, searchId);

    }

    for (EdgeList::iterator it = edges.begin(); it != edges.end(); ++it) {
        //LOG_DEBUG("setOutboundSearch - eval edge:" << *it);

        if (it->getStatus() == Status::DELETED) {
            //LOG_DEBUG("setOutboundSearch -SKIP DELETED");
            continue;
        }

        if (it->existsGraphNeighbor(evalGraphId)) {

            if (!it->existsGraphNeighbor(graphId)) {
                node.addGraphNeighbor(engineOperations, graphId, traffic, it->getDestination(), it->getGraphOnEdge(
                            evalGraphId).preffered);
            } else {
                it->getGraphOnEdge(graphId).preffered = it->getGraphOnEdge(evalGraphId).preffered;
            }

        } else {
            bool add = false;
            bool preffered = true;

            if ((it->getSource() == evalPrimary && it->getDestination() == evalSecondary) || (it->getSource()
                        == evalPrimary && it->getDestination() == destination) || (it->getSource() == evalSecondary
                        && it->getDestination() == destination) || (it->getSource() == evalPrimarySecondary && it->getDestination() == evalSecondary)) {

                //primary -> secondary, primary->destination, secondary->destination, primarySecondary->secondary are added
                add = true;
                if (it->getSource() == evalPrimary && it->getDestination() == evalSecondary) {
                    preffered = false;

                    Node& secondaryNode = subnetTopology.getNode(evalSecondary);
                    if (secondaryNode.hasOtherOutboundNeighborOnGraph(destination, evalGraphId)) {
                        add = false;
                    }
                }

                if (it->getSource() == evalPrimarySecondary && it->getDestination() == evalSecondary) {
                    preffered = false;

                    Node& secondaryNode = subnetTopology.getNode(evalSecondary);

                    if (secondaryNode.hasOtherOutboundNeighborOnGraph(destination, evalGraphId) || secondaryNode.hasNeighborOnGraph(evalPrimarySecondary, evalGraphId)) {
                        add = false;
                    }
                }

            } else {
                add = false;
            }

            if (add && !it->existsGraphNeighbor(graphId)) {
                node.addGraphNeighbor(engineOperations, graphId, traffic, it->getDestination(), preffered);
            } else if (!add && it->existsGraphNeighbor(graphId)) {
                node.setRemoveGraphNeighbor(engineOperations, graphId, it->getDestination());
            }

        }
    }
}

