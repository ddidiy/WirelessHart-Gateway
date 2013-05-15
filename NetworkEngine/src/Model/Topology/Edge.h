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

#ifndef EDGE_H_
#define EDGE_H_

#include <list>
#include "Model/Topology/TopologyTypes.h"
#include "Model/Topology/GraphNeighbor.h"
#include "Common/NEAddress.h"
#include "Common/logging.h"

namespace NE {

namespace Model {

namespace Topology {

class Edge;
class Node;
class Path;
typedef std::list<Edge> EdgeList;

/**
 * Keeps information about an edge used in a graph associated to a subnet topology.
 * It is a physical link between two nodes that have allocated resources to communicate with
 * each other.
 * This edge can be part of one or more routing graphs and this is kept in <code>routeGraphsStatus</code>.
 * An edge is used in three contexts :
 * 1. Represents the physical link between two nodes.
 * 2. Part of one or more routing graphs.
 * 3. Used to determined a new routing graph.
 *
 * @author Radu Pop, Ioan-Vasile Pocol
 */
class Edge {

    LOG_DEF("I.M.G.Edge");

    private:

        /**
         * The pointer to the source of the edge.
         */
        Address32 source;

        /**
         * The pointer to the destination of the edge.
         */
        Address32 destination;

        Uint32 totalSent;
        Uint32 totalReceived;
        Uint32 totalFailed;

        /**
        * Packet Error Rate statistic [%]
        */
        unsigned int edgePER;

        /**
         * Channel Quality Indicator [%]
         */
        unsigned int edgeCQI;

        unsigned int perFactor;

        unsigned int cqiFactor;

        /**
         * The status of the edge.
         */
        Status::StatusEnum status;

        /**
         * The current traffic of the edge.
         */
        Uint16 traffic;

        /**
         * The average sent packages.
         */
        Uint16 sent;

        /**
         * The average received packages.
         */
        Uint16 received;

        /**
         * The average sent packages.
         */
        Uint16 failed;

        Uint16 lastSent;
        Uint16 lastReceived;
        Uint16 lastFailed;

        /**
         * The fail factor, given by packet loss.
         */
        Uint16 oldCostFactorFail;

        /**
         * The fail factor, given by packet loss.
         */
        Uint16 costFactorFail;

        /**
         * This is a factor related to the battery of the node.
         */
        Uint16 costFactorBattery;

        /**
         * A retry factor.
         */
        Uint16 costFactorRetry;

        /**
         * The current set search Id.
         */
        Uint16 searchId;

        /**
         * The last evaluated cost for the edge.
         */
        Uint16 evalEdgeCost;

        /**
         *
         */
        Uint16 diagCount;

        /**
         *
         */
        Uint16 maxDiagCount;

        /**
         * The average RSL.
         */
        Uint8 rsl;

        /**
         * The average RSL.
         */
        Uint8 oldRsl;

        Int8 lastRsl;

        /**
         * TODO : to be set by someone
         * True if the destination of the edge is clock source for the source of the edge.
         */
        bool destinationClockSource;

        /**
         * True if the edge was reported as failing by devices.
         */
        bool isEdgeFailing;

        /**
         * If an edge is to be deleted, but it should be reused, this will be true
         */
        bool saveFromDelete;

        /**
         * Edge information for every graph that goes through the edge.
         */
        GraphNeighborMap graphs;

    public:

        inline bool SaveFromDelete() {
            return saveFromDelete;
        }

        inline void SaveFromDelete(bool value) {
            saveFromDelete = value;
        }

        inline bool IsEdgeFailing() {
            return isEdgeFailing;
        }

        inline void IsEdgeFailing(bool value) {
            isEdgeFailing = value;
        }

        /**
         * Equal operator.
         * @param the Edge to compare with
         */
        bool operator==(Edge& edge) const;

        /**
         *
         */
        Edge(Address32 source, Address32 destination);

        /**
         *
         */
        virtual ~Edge();

        /**
         *
         */
        bool existsGraphNeighbor();

        /**
         *
         */
        bool existsGraphNeighbor(Uint16 graphId);

        bool existsActiveGraphNeighbor(Uint16 graphId);

        /**
         *
         */
        bool existsDeletedClockSource(Uint16 graphId);

        /**
         *
         */
        GraphNeighbor& getGraphOnEdge(Uint16 graphId);

        /**
         *
         */
        GraphNeighborMap& getGraphs();

        /**
         *
         */
        Address32 getSource();

        /**
         *
         */
        Address32 getDestination();

        /**
         *
         */
        Status::StatusEnum getStatus() const;

        /**
         *
         */
        void setStatus(Status::StatusEnum status);

        /**
         *
         */
        Uint16 getTraffic() const;

        /**
         *
         */
        Uint8 getRsl();

        /**
         *
         */
        Uint16 getSent();

        /**
         *
         */
        Uint16 getReceived();

        /**
         *
         */
        Uint16 getFailedSent();

        /**
         *
         */
        Uint16 getCostFactorBattery();

        /**
         *
         */
        void setCostFactorBattery(Uint16 costFactorBattery);

        /**
         *
         */
        Uint16 getCostFactorRetry();

        /**
         *
         */
        void setCostFactorRetry(Uint16 costFactorRetry);

        /**
         *
         */
        Uint16 getSearchId();

        /**
         *
         */
        void setSearchId(Uint16 searchId);

        /**
         *
         */
        Status::StatusEnum getGraphNeighborStatus(Uint16 graphId);

        /**
         *
         */
        void setGraphNeighborStatus(Uint16 graphId, Status::StatusEnum status);

        /**
         * Add the neighbor from the graph
         */
        void addGraphNeighbor(Uint16 graphId, Uint16 traffic, bool preffered);

        /**
         * Remove the neighbor from the graph
         */
        void removeGraphNeighbor(Uint16 graphId);

        /**
         * Set the status to removed
         */
        void setRemoveGraphNeighbor(Uint16 graphId);

        /**
         * Evaluate the cost on the edge.
         */
        Uint16 evaluateEdgeCost(Uint16 graphId, Uint16 evalGraphTraffic, Node& sourceNode, Node& destNode, Path& path);
        Uint16 evaluateEdgeCost(Uint16 graphId, Uint16 evalGraphTraffic);

        /**
         * Return the last evaluated edge cost
         */
        Uint16 getEvalEdgeCost();

        /**
         * Return the traffic for non deleted graph edges.
         */
        Uint16 getActiveTraffic();

        /**
         * Update the edge link diagnostics
         */
        bool addDiagnostics(Uint8 rsl, bool updateEdges = true);

        /**
         * Update the edge link diagnostics
         */
        bool addDiagnostics(Uint8 rsl, Uint16 sent, Uint16 received, Uint16 failed, bool updateEdges = true);

        /**
         * Compute the traffic and the edge cost by taken in account no traffic on the current graph route.
         */
        void deleteTraffic(Uint16 graphId, Uint16 graphPathTraffic);

        /**
         * Set the traffic and cost by taken in account the currently set graph route
         */
        void updateTraffic(Uint16 graphId, Uint16 graphPathTraffic);

        /**
         * Return true if the traffic is allocated with retry on this edge
         */
        bool isRetryOnGraph(Uint16 graphId);

        /**
         * Return true if the edge is preferred neighbor on graph
         */
        bool isPrefferedOnGraph(Uint16 graphId);

        /**
         * Return true if the source is the lazy parent for destination on the given graph
         */
        bool isLazyParent(Uint16 graphId);

        /**
         * Returns a string description of the status of the edge in the graph.
         * @return a string description of the status of the edge in the graph
         */
        std::string getGraphEdgeDescription(Uint16 graphId);

        /**
         * Delete source/graph route from the edge
         */
        void deletePath(Uint16 graphId, Uint16 routeTraffic);

        /**
         * Add source route to the edge
         */
        void addSourcePath(Uint16 graphId, Uint16 routeTraffic);

        /**
         * Returns true if the destination of the edge is clock source for the source of the edge.
         */
        bool isDestinationClockSource() {
            return destinationClockSource;
        }

        void setDestinationClockSource(bool flag) {
            destinationClockSource = flag;
        }

        Uint8 getLastRsl() {
            return lastRsl;
        }

        Uint16 getLastSent() {
            return lastSent;
        }

        Uint16 getLastReceived() {
            return lastReceived;
        }

        Uint16 getLastFailed() {
            return lastFailed;
        }

        Uint16 getTotalSent() {
            return totalSent;
        }

        Uint16 getTotalReceived() {
            return totalReceived;
        }

        Uint16 getTotalFailed() {
            return totalFailed;
        }

        /**
         * Returns a short string representation for the object (a pair with the address
         * of the source and the address of the destination).
         * @return a short string representation for the object
         */
        void toShortString(std::ostream& stream);

        /**
         *
         */
        void toShortString(Uint16 graphId, std::ostream &stream);

        /**
         *
         */
        void toShortStatusString(std::ostream& stream) const;

        /**
         * Returns a string representation of this Edge.
         */
        friend std::ostream& operator<<(std::ostream&, const Edge&);

};

}
}
}

#endif /* EDGE_H_ */
