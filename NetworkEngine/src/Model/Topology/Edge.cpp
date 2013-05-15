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

#include "Edge.h"
#include "Model/NetworkEngine.h"
#include <limits.h>

using namespace NE::Model::Topology;

Edge::Edge(Address32 _source, Address32 _destination) :
    source(_source), destination(_destination) {

    LOG_TRACE("create Edge: source=" << ToStr(source) << ", dest=" << ToStr(destination));

    status = Status::ACTIVE;

    oldRsl = 125;
    rsl = 125;

    sent = 1;
    received = 0;
    failed = 0;

    oldCostFactorFail = 100;
    costFactorFail = 100;

    traffic = 0;
    costFactorBattery = 1;
    costFactorRetry = 1;

    diagCount = 0;

    isEdgeFailing = false;
    saveFromDelete = false;

    lastFailed = 0;
    lastReceived = 0;
    lastRsl = 0;
    lastSent = 0;

    totalSent = 0;
    totalReceived = 0;
    totalFailed = 0;

    perFactor = 1;
    cqiFactor = 1;

    edgePER = 0;
    edgeCQI = 0;
    evalEdgeCost = 0;

    maxDiagCount = NE::Model::NetworkEngine::instance().getSettingsLogic().forceReevaluate;
}

Edge::~Edge() {
}

Address32 Edge::getSource() {
    return source;
}

Address32 Edge::getDestination() {
    return destination;
}

Status::StatusEnum Edge::getStatus() const {
    return status;
}

void Edge::setStatus(Status::StatusEnum status_) {
    status = status_;
}

Uint16 Edge::getTraffic() const {
    return traffic;
}

Uint8 Edge::getRsl() {
    return rsl;
}

Uint16 Edge::getSent() {
    return sent;
}

Uint16 Edge::getReceived() {
    return received;
}

Uint16 Edge::getFailedSent() {
    return failed;
}

Uint16 Edge::getCostFactorBattery() {
    return costFactorBattery;
}

void Edge::setCostFactorBattery(Uint16 costFactorBattery) {
    this->costFactorBattery = costFactorBattery;
}

Uint16 Edge::getCostFactorRetry() {
    return costFactorRetry;
}

void Edge::setCostFactorRetry(Uint16 costFactorRetry) {
    this->costFactorRetry = costFactorRetry;
}

Uint16 Edge::getSearchId() {
    return searchId;
}

void Edge::setSearchId(Uint16 searchId) {
    this->searchId = searchId;
}

void Edge::setGraphNeighborStatus(Uint16 graphId, Status::StatusEnum status) {
    getGraphOnEdge(graphId).status = status;
}

Status::StatusEnum Edge::getGraphNeighborStatus(Uint16 graphId) {
    return getGraphOnEdge(graphId).status;
}

bool Edge::existsGraphNeighbor(Uint16 graphId) {
    GraphNeighborMap::iterator it = graphs.find(graphId);
    if (it != graphs.end()) {
        return it->second.status != Status::DELETED && it->second.status != Status::REMOVED;
    } else {
        return false;
    }
}

bool Edge::existsActiveGraphNeighbor(Uint16 graphId) {
    GraphNeighborMap::iterator it = graphs.find(graphId);
    if (it != graphs.end()) {
        return it->second.status == Status::ACTIVE;
    } else {
        return false;
    }
}

bool Edge::existsDeletedClockSource(Uint16 graphId) {
    GraphNeighborMap::iterator it = graphs.find(graphId);
    if (it != graphs.end()) {
        return it->second.status == Status::DELETED && it->second.preffered;
    } else {
        return false;
    }
}

GraphNeighborMap& Edge::getGraphs() {
    return graphs;
}

GraphNeighbor& Edge::getGraphOnEdge(Uint16 graphId) {
    GraphNeighborMap::iterator it = graphs.find(graphId);
    if (it != graphs.end()) {
        return it->second;
    } else {
        std::ostringstream stream;
        stream << "The edge " << *this << " has not graph: " << ToStr(graphId);
        LOG_ERROR(stream.str());
        throw NEException(stream.str());
    }
}

bool Edge::isLazyParent(Uint16 graphId) {
    GraphNeighborMap::iterator it = graphs.find(graphId);
    if (it != graphs.end()) {
        return it->second.lazy;
    }

    return false;
}

bool Edge::operator==(Edge& edge) const {
    if (source == edge.getSource()) {
        if (destination == edge.getDestination()) {
            return true;
        }
    }

    return false;
}

bool Edge::existsGraphNeighbor() {
    return !graphs.empty();
}

void Edge::addGraphNeighbor(Uint16 graphId, Uint16 pathTraffic, bool preffered) {
    GraphNeighborMap::iterator it = graphs.find(graphId);
    if (it != graphs.end()) {
        traffic -= it->second.traffic;
    }

    traffic += pathTraffic;
    GraphNeighbor graphNeighbor(pathTraffic, preffered);
    graphs[graphId] = graphNeighbor;
}

void Edge::removeGraphNeighbor(Uint16 graphId) {
    GraphNeighborMap::iterator it = graphs.find(graphId);
    if (it != graphs.end()) {
        if (it->second.status == Status::DELETED || it->second.status == Status::NEW) {
            LOG_DEBUG("removeGraphNeighbor for- " << *this << ", delete the neighbor for graphId=" << ToStr(graphId));

            traffic -= it->second.traffic;
            graphs.erase(it);
        } else {
            LOG_WARN("removeGraphNeighbor for- " << *this << ", the neighbor status for graphId=" << ToStr(graphId)
                        << " is:" << (int) it->second.status);
        }

    } else {
        LOG_WARN("removeGraphNeighbor for- " << *this << ", there is no graphId=" << ToStr(graphId));
    }
}

void Edge::setRemoveGraphNeighbor(Uint16 graphId) {
    GraphNeighborMap::iterator it = graphs.find(graphId);
    if (it != graphs.end()) {
        if (it->second.status != Status::DELETED) {
            it->second.status = Status::DELETED;
        } else {
            LOG_WARN("setRemoveGraphNeighbor for- " << *this << ", the neighbor status for graphId=" << ToStr(graphId)
                        << " is:" << (int) it->second.status);
        }
    } else {
        LOG_WARN("setRemoveGraphNeighbor for- " << *this << ", there is no graphId=" << ToStr(graphId));
    }
}


Uint16 Edge::evaluateEdgeCost(Uint16 graphId, Uint16 evalGraphTraffic, Node& sourceNode, Node& destNode, Path& path) {
    if (isEdgeFailing) {
        LOG_DEBUG("Edge is marked as failing. Cost=0xFFFF");
        return 0xFFFF;
    }

    //TODO: ivp - to be replaced traffic with a function of traffic
    std::ostringstream stream;
    stream << "{ " << ToStr(source) << " -> " << ToStr(destination) << " }";

    if (source != MANAGER_ADDRESS && source != GATEWAY_ADDRESS && destination != MANAGER_ADDRESS && destination
                != GATEWAY_ADDRESS) {

        Uint16 pathTraffic = 0;
        GraphNeighborMap::iterator it = graphs.find(graphId);
        if (it != graphs.end()) {
            pathTraffic = it->second.traffic;
        }


        Uint8 active = 1;


        if (sourceNode.getNodeType() == NodeType::BACKBONE && destination == path.getDestination()
                    && path.getSource() == MANAGER_ADDRESS) {
            active = 0;
        }

        if (destNode.getNodeType() == NodeType::BACKBONE && source == path.getSource()
                    && path.getDestination() == MANAGER_ADDRESS) {
            active = 0;
        }

        if (costFactorFail != 0xFFFF) {
            evalEdgeCost = (costFactorBattery * costFactorRetry * (active*(traffic + evalGraphTraffic - pathTraffic)+(1-active))
                        * costFactorFail) / 1000;
        }
        else {
            evalEdgeCost = 0xFFFF;
        }

        if (status == Status::CANDIDATE) {
            if (sourceNode.maxNeighborsNoReached() || destNode.maxNeighborsNoReached()) {
                evalEdgeCost = 0xFFFF;
            }
        }


        LOG_DEBUG("evaluateEdgeCost : edge=" << stream.str() << ", costFactorFail=" << costFactorFail
                    << ", costFactorBattery=" << costFactorBattery << ", costFactorRetry=" << costFactorRetry
                    << ", traffic=" << traffic << ", evalGraphTraffic=" << evalGraphTraffic << ", pathTraffic="
                    << pathTraffic);
    } else {
        evalEdgeCost = 0;
    }

    LOG_DEBUG("evaluateEdgeCost : edge=" << stream.str() << "  cost=" << evalEdgeCost);

    return evalEdgeCost;

}

Uint16 Edge::evaluateEdgeCost(Uint16 graphId, Uint16 evalGraphTraffic) {

    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();
    Node& sourceNode = subnetTopology.getNode(source);
    Node& destNode = subnetTopology.getNode(destination);
    Path& path = subnetTopology.getPath(graphId);

    return evaluateEdgeCost(graphId, evalGraphTraffic, sourceNode, destNode, path);
}

Uint16 Edge::getEvalEdgeCost() {
    return evalEdgeCost;
}

bool Edge::addDiagnostics(Uint8 rsl_, bool updateEdges) {
    LOG_TRACE("addDiagnostics - Cost: old rsl:" << (int) rsl - 128 << ", new rsl=" << (int) rsl_);

    if (isEdgeFailing) {
        isEdgeFailing = false;
        LOG_DEBUG("RSL received for edge marked as failing. Removing marker.");
    }

    rsl = rsl_;
    if (updateEdges) {
        SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();
        Node& sourceNode = subnetTopology.getNode(source);
        EdgeList::iterator pos;
        sourceNode.updateEdgesList(destination, rsl, pos);
    }

    if (sent == 0) {
        sent = 1;
    }

    if ( totalSent > 0 ) {
        edgePER = ( unsigned int ) totalFailed * 100 / totalSent;
    }
    else {
        edgePER = 0;
    }

    edgeCQI = (unsigned int) rsl * 100 / 255;

    costFactorFail =  (Uint16) floor( perFactor * edgePER + cqiFactor * (100 - edgeCQI) );


    LOG_DEBUG("addDiagnostics - costFactorFail=" << (int) costFactorFail);

    Uint8 gap = (oldRsl > rsl ? (oldRsl - rsl) : (rsl - oldRsl));

    if (((100 * gap) / 255) > 20) {
        oldRsl = rsl;
        diagCount = 0;
        LOG_DEBUG("addDiagnostics - gap=" << (int) gap << ", new rsl=" << (int) rsl);
        return true;
    }

    if (++diagCount > maxDiagCount) {
        LOG_DEBUG("addDiagnostics - force true");
        diagCount = 0;
        return true;
    }

    return false;
}

bool Edge::addDiagnostics(Uint8 rsl_, Uint16 sent_, Uint16 received_, Uint16 failed_, bool updateEdges) {

    LOG_TRACE("addDiagnostics - Cost: rsl=" << (int) rsl_ - 128 << ", sent=" << (int) sent_ << ", received="
                << (int) received_ << ", failed=" << (int) failed_);

    lastRsl = (Int8) ((Int16) rsl_ - 128);

    if (sent_ >= lastSent) {
        totalSent += sent_ - lastSent;
    } else {
        // from 65000 gets 200 => the counter reached the max limit and reinitialized from 0
        totalSent += 0x10000 - (lastSent - sent_);
    }

    if (received_ >= lastReceived) {
        totalReceived += received_ - lastReceived;
    } else {
        // from 65000 gets 200 => the counter reached the max limit and reinitialized from 0
        totalReceived += 0x10000 - (lastReceived - received_);
    }

    if (failed_ >= lastFailed) {
        totalFailed += failed_ - lastFailed;
    } else {
        // from 65000 gets 200 => the counter reached the max limit and reinitialized from 0
        totalFailed += 0x10000 - (lastFailed - failed_);
    }

    lastSent = sent_;
    lastReceived = received_;
    lastFailed = failed_;

    rsl = rsl_;

    if (updateEdges) {
        SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();
        Node& sourceNode = subnetTopology.getNode(source);
        EdgeList::iterator pos;
        sourceNode.updateEdgesList(destination, rsl, pos);
    }

    sent = (sent + sent_) / 2;

    received = (received + received_) / 2;

    failed = (failed + failed_) / 2;

    if (sent == 0) {
        sent = 1;
    }


    if ( totalSent > 10 ) { //if measurements are reliable
        edgePER = ( unsigned int ) totalFailed * 100 / totalSent;
    }
    else {
        edgePER = 0;
    }

    edgeCQI = (unsigned int) rsl * 100 / 255;

    if (edgePER <= NE::Model::NetworkEngine::instance().getSettingsLogic().perThreshold) {
        costFactorFail =  (Uint16) floor( perFactor * edgePER + cqiFactor * (100 - edgeCQI) );
    }
    else {
        costFactorFail = 0xFFFF;
    }

    Uint16 gap = oldCostFactorFail < costFactorFail ? costFactorFail - oldCostFactorFail : oldCostFactorFail
                - costFactorFail;

    LOG_DEBUG("addDiagnostics: oldCost=" << oldCostFactorFail << ", newCost=" << costFactorFail);

    if (oldCostFactorFail == 0) {
        oldCostFactorFail = 1;
    }

    if (((gap * 100) / oldCostFactorFail) > 30) {
        oldCostFactorFail = costFactorFail;

        diagCount = 0;
        LOG_DEBUG("addDiagnostics - gap=" << (int) gap);
        return true;
    }

    if (++diagCount > maxDiagCount) {
        LOG_DEBUG("addDiagnostics - force true");
        diagCount = 0;
        return true;
    }

    return false;
}

Uint16 Edge::getActiveTraffic() {
    Uint16 result = 0;
    for (GraphNeighborMap::iterator it = graphs.begin(); it != graphs.end(); ++it) {
        result += (it->second.status != Status::ACTIVE) ? 0 : it->second.traffic;
    }

    return result;
}

bool Edge::isPrefferedOnGraph(Uint16 graphId) {
    return getGraphOnEdge(graphId).preffered;
}

bool Edge::isRetryOnGraph(Uint16 graphId) {
    return getGraphOnEdge(graphId).retry;
}

void Edge::toShortString(std::ostream& stream) {
    stream << "(" << ToStr(source) << ", " << ToStr(destination) << ")";

}

void Edge::toShortStatusString(std::ostream& stream) const {
    stream << "          {";
    stream << ToStr(source);
    stream << ", " << ToStr(destination);
    stream << ", T=" << getTraffic();
    stream << ", RSL=" << (int)(rsl - 128);
    stream << ", S=" << Status::getStatusDescription(status);
    stream << ", F=" << (isEdgeFailing ? "1" : "0");
    stream << ", edgePaths= { ";

    for (GraphNeighborMap::const_iterator it = graphs.begin(); it != graphs.end(); ++it) {
        stream << std::endl << "            {" << ToStr(it->first) << ", " << it->second << "}";
    }
    stream << std::endl << "          } }";
}

void Edge::toShortString(Uint16 graphId, std::ostream &stream) {
    stream << "(" << ToStr(source) << ", " << ToStr(destination) << ":" << (int) getGraphNeighborStatus(graphId) << ")";
}

std::ostream& NE::Model::Topology::operator<<(std::ostream& stream, const Edge& edge) {

    stream << "Edge {" << "(" << ToStr(edge.source) << ", " << ToStr(edge.destination) << "), " << ", traffic="
                << edge.traffic << ", status=" << Status::getStatusDescription(edge.status) << ", isFailing="
                << (edge.isEdgeFailing ? "true" : "false") << ", costFactorBattery=" << edge.costFactorBattery
                << ", costFactorRetry=" << edge.costFactorRetry << ", RSL=" << (int)(edge.rsl - 128) << "}";
    return stream;
}

