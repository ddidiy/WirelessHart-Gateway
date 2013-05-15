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

#ifndef ROUTINGGRAPH_H_
#define ROUTINGGRAPH_H_

#include "Common/logging.h"
#include "TopologyTypes.h"
#include "Edge.h"
#include "Node.h"
#include "Model/Tdma/LinkEdge.h"
#include "Model/Operations/EngineOperations.h"

namespace NE {

namespace Model {

namespace Topology {

class Path;
struct EdgePointer;

typedef std::map<Uint16, Path> PathMap;
typedef std::vector<EdgePointer> EdgePointerVector;
typedef std::map< Address32, EdgePointerVector > ConnectionsMap;


typedef struct NodePointer {
        NodePointer() {
            node = NULL;
            address = 0;
        }

        NodePointer(Node &node_) {
            address = node_.getNodeAddress();
            node = &node_;
        }

        Address32 address;
        Node *node;
}NodePointer;

typedef struct EdgePointer{
        EdgePointer() {
            dest = 0;
            edge = NULL;
            peerNode = NULL;
        }

        EdgePointer(Edge &edge_, Node &peerNode_) {
            dest = edge_.getDestination();
            edge = &edge_;
            peerNode = &peerNode_;
        }

        Address32 dest;
        Edge *edge;
        Node *peerNode;
}EdgePointer;


/**
 * Keeps information about a routing graph or a source routing.
 * To determine the routing graph or the source routing path we have steps.
 *
 * @author Radu Pop, Ioan-Vasile Pocol
 */
class Path {

    LOG_DEF("I.M.G.Path");

    private:

        /**
         * The id of the route.
         */
        Uint16 graphId;

        /**
         * The pointer to the source of the graph.
         */
        Address32 source;

        /**
         * The pointer to the destination of the graph.
         */
        Address32 destination;

        /**
         * The type of the edge
         */
        RoutingTypes::RoutingTypes_Enum type;

        /**
         * The path for source route
         */
        AddressList sourcePath;

        /**
         * The status of the route
         */
        Status::StatusEnum status;

        /**
         * The current graph traffic.
         */
        Uint16 traffic;

        /**
         * Values 0, 1, 2 - 0 lower redundancy
         */
        Uint8 redundantFactor;

        /**
         * Use retry on preffered
         */
        bool useRetryOnPreffered;

        /**
         * The maximum numbers of hops allowed
         */
        Uint8 maxHops;

        /**
         * If the route should be reevaluated
         */
        bool reevaluate;

        /**
         *
         */
        bool onEvaluation;

        /**
         * The current cost of the graph
         */
        Uint32 cost;

        /**
         * The handler used for allocation
         */
        Uint32 allocationHandler;

        /**
         * The priority for evaluating.
         */
        EvaluatePathPriority::EvaluatePathPriority evaluatePathPriority;

        /**
         * The time of the last path evaluation.
         */
        time_t lastEvalTime;

        ConnectionsMap incomingConnections;
        ConnectionsMap outcomingConnections;
        std::list<Address32> pathNodesList;
        std::vector<NodePointer> orderedNodes;
        //std::map< Address32, Uint16> levelMap;

    public:

        /**
         *
         */
        Path() {
        }

        /**
         *
         */
        Path(Uint16 graphId, Address32 source, Address32 destination, RoutingTypes::RoutingTypes_Enum type,
                    Uint16 traffic);

        /**
         */
        virtual ~Path();

        /**
         *
         */
        RoutingTypes::RoutingTypes_Enum getType();

        /**
         *
         */
        bool isSourcePath();

        /**
         *
         */
        bool isReevaluate();

        /**
         *
         */
        bool isOnEvaluation();

        /**
         *
         */
        void setOnEvaluation(bool onEvaluation);

        /**
         *
         */
        EvaluatePathPriority::EvaluatePathPriority getEvaluatePathPriority();

        /**
         *
         */
        void setReevaluate(bool reevaluate, EvaluatePathPriority::EvaluatePathPriority evaluatePathPriority);

        /**
         *
         */
        std::string getTypeDescription();

        /**
         *
         */
        Uint16 getGraphId();

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
        Uint16 getTraffic();

        /**
         *
         */
        void setTraffic(Uint16 traffic);

        /**
         *
         */
        Uint8 getRedundantFactor();

        /**
         *
         */
        void setRedundantFactor(Uint8 redundantFactor);

        /**
         *
         */
        Uint8 getMaxHops();

        /**
         *
         */
        void setMaxHops(Uint8 _maxHops);

        /**
         *
         */
        AddressList& getSourcePath();

        /**
         *
         */
        void setSourcePath(AddressList& addressList);

        /**
         *
         */
        Status::StatusEnum getStatus();

        /**
         *
         */
        void setStatus(Status::StatusEnum status);

        /**
         *
         */
        Uint32 getCost();

        /**
         *
         */
        void setCost(Uint32 cost);

        /**
         *
         */
        bool containsEdge(Uint16 src, Uint16 dest);

    private:

        /**
         * creates incoming and outgoing connections maps of the graph (structures are needed by the
         * topological sorting algorithm)
         */
        void populateConnectionsMap(Address32 address, Uint16 searchId);

        /**
         * Traverses a graph and marks the reached nodes with the current <code>subnetId</code>.
         */
        bool checkGraphPathSearch(NE::Model::Operations::EngineOperations& engineOperations,  Address32 address, bool active, Uint16 searchId, bool interrupted);

        /**
         * Mark reachable nodes on graph
         */
        bool markReachableOnPath(Address32 address, Uint16 searchId);

        /**
         *
         */
        void populateLinkEdgesSearch(NE::Model::Tdma::LinkEdgesList& linkEdges);

        /**
         *
         */
        void populateEvalLinkEdgesSearch(NE::Model::Tdma::LinkEdgesList& linkEdges);

        /**
         * Set all resources allocated to route as removed
         */
        //void removePathSearch(Address32 address, Uint16 routeId);

        /**
         * Determines recursively the edges that compose the current graph.
         */
        void determineGraphsEdges(Address32 address, std::ostream& stream, Uint16 searchId) const;

        /**
         *
         * @param crtAddress
         * @param endAddress
         * @param destinations
         * @param searchId
         */
        void initDestinationsSearch(Address32 crtAddress, Address32 endAddress, AddressList& destinations, Uint16 searchId);

    public:

        /**
         * Checks the routing graphs affected by the node removal.
         * After some of the nodes have been marked for removal we check all the graphs that passes
         * through the given node. Each graph is traversed from source to destination ignoring the nodes
         * that have been marked for removal. The nodes that are reached are marked as reached (subnetId).
         * <code>markReachableGraphNeighbors()</code> is the method that does the above mention action.
         */
        bool checkGraphPath(NE::Model::Operations::EngineOperations& engineOperations, Uint16 searchId);

        /**
         *
         */
        bool checkSourcePath();

        /**
         * Populates the list of paths with the paths from route
         */
        void populateLinkEdges(NE::Model::Tdma::LinkEdgesList& linkEdges, bool isEval);

        /**
         *
         */
        void populateJoinLinkEdge(NE::Model::Tdma::LinkEdgesList& linkEdges, bool isInbound);

        /**
         *
         */
        void setSourcePath(std::vector<Address32>& sourcePath);

        /**
         * Sorts the graph nodes in topological order.
         */
        void createTopologicalOrder();

        /**
         *  Searches for the shortest path in graph (alg. needs the nodes in topological order)
         */
        Uint32 getMinCostPath(AddressList& shortPathAddr, bool createTopOrder, Address32 testAddress = 0);



        /**
         *
         * @param address
         * @param destinations
         */
        void initDestinations(Address32 address, AddressList& destinations);

        /**
         *
         */
        void updateLastEvalTime();

        /**
         *
         */
        time_t getLastEvalTime();

        /**
         * Returns a string representation of this route.
         */
        friend std::ostream& operator<<(std::ostream&, Path&);
};

}

}

}

#endif /* ROUTINGGRAPH_H_ */
