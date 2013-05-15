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

#ifndef SUBNETTOPOLOGY_H_
#define SUBNETTOPOLOGY_H_

#include "Common/NEAddress.h"
#include "Common/logging.h"

#include "Model/Topology/Edge.h"
#include "Model/Topology/Path.h"
#include "Model/Topology/Node.h"
#include "Model/Services/Service.h"
#include "Model/Operations/EngineOperations.h"
#include "Model/DevicesTable.h"
#include "Model/Operations/IEngineOperationsVisitor.h"
#include "Model/Topology/GraphRoutingAlgorithmInterface.h"

using namespace NE::Model;
using namespace NE::Common;

namespace NE {

namespace Model {

namespace Topology {

/**
 * For each subnet there is an instance of this class that keeps the topology, the
 * routing graphs and source routing paths. A source routing path is kept as a graph routing
 * with the type set to source routing (see Path class).
 * A source routing is a like a graph routing with only one path.
 * There are several events that change the status of the topology:
 * <ul>
 * <li>1. A device sends a join request. (joinNode() function)</li>
 * <li>2. A device is removed from the subnet. (removeNode() function)</li>
 * <li>3. A device reports that can view another device ( addVisibleNeighbor() function).</li>
 * <li>4. From time to time a routing graph evalution event is fired. (evaluatePath() function)</li>
 * </ul>
 *
 * @author Radu Pop, Ioan-Vasile Pocol
 * @version 1.1
 */
class SubnetTopology {

    LOG_DEF("I.M.R.SubnetTopology");

    private:

        /**
         * The id of the subnet for which the data is saved.
         */
        Uint16 subnetId;

        /**
         * True if the gateway is joined
         */
        bool gatewayAdded;

        /**
         * Used on graph search to indicate if the node has already been touched.
         */
        Uint16 searchId;

        /**
         * The list of nodes from the graph.
         */
        NodeMap nodes;

        /**
         * The list with all the routing graphs (including source routing) from the
         * current subnet topology.
         */
        PathMap paths;

        /**
         * This is the graph that will be called to optimize an existing route.
         */
        boost::shared_ptr<NE::Model::Topology::GraphRoutingAlgorithmInterface> graphRoutingAlgorithm;

        /**
         * The list of nodes that needs a reevaluation (e.g. secondary clk source)
         */
        std::queue<Address32> onEvaluationNodesQueue;

        std::multimap<int, Uint16> graphsToRefresh;

    public:

        /**
         *
         */
        SubnetTopology();

        /**
         *
         */
        virtual ~SubnetTopology();

        /**
         *
         */
        void init(Uint16 subnetId);

        /**
         *
         */
        Uint16 getSubnetId();

        /**
         *
         */
        bool isGatewayAdded();

        /**
         *
         */
        NodeMap& getSubnetNodes();

        /**
         *
         */
        Uint16 getSearchId();

        /**
         *
         */
        Uint16 setNextSearchId();

        /**
         *
         */
        PathMap& getPaths();

        /**
         * Returns true if there is a node with the given address.
         */
        bool existsNode(Address32 address);

        /**
         * Return true if the path exists and the status != REMOVED and != DELETED
         * @param nodeAddress
         * @return
         */
        bool existsActiveNode(Address32 nodeAddress);


        /**
         * Returns true if there is a routing graph with the given graphId.
         */
        bool existsPath(Uint16 routeId);

        /**
         *  Return true if the path exists and the status != REMOVED and != DELETED
         */
        bool existsActivePath(Uint16 graphId);

        /**
         * Sets the edge between src and dest as failing.
         */
        void setEdgeFailing(Address32 src, Address32 dest, bool value);

        /**
         *  Return true if the edge exists and was marked as failing already.
         */
        bool isEdgeFailing(Address32 src, Address32 dest);

        /**
         *  Requests reevaluation of all routes that contain the specified edge.
         */
        void reevaluateRoutesContainingEdge(Address32 src, Address32 dest);

        /**
         * Returns the route with the given routeId
         */
        Path& getPath(Uint16 graphId);

        /**
         * Generates the next available graph id
         */
        Uint16 getNextPathId();

        /**
         * Returns the node with the given address or null.
         */
        Node& getNode(Address32 address);


        /**
         *
         */
        void getListOfClockSources(Address32 address, std::vector<Address32>& list);

        /**
         * Adds a new node to an existing node in response to a device join request.
         */
        void addNode(NE::Model::Operations::EngineOperations& engineOperations, Address32 parentAddress,
                    Address32 nodeAddress, NodeType::NodeTypeEnum nodeType, Uint16 traffic, Uint16 proxyGraphId,
                    Uint16& inboundGraphId, Uint16& outboundGraphId);

        /**
         *
         */
        void regenerateGatewayOperations(NE::Model::Operations::EngineOperations& engineOperations);

        /**
         * Adds the visible node with the address visibleAddress to the existingAddress.
         */
        void addVisibleNode(Address32 existingAddress, Address32 visibleAddress, Uint8 rsl);

        /**
         * Update the diagnostics
         */
        void addDiagnostics(Address32 nodeAddress, Address32 neighborAddress, Uint8 rsl, Uint16 sent, Uint16 received,
                    Uint16 failed);

        /**
         * If the node exists set status to REMOVED
         */
        void setRemoveStatus(Address32 address);

        /**
         *
         */
        bool markDeleted(Address32 address);

        /**
         *
         */
        void setReevaluateGraph(Address32 address, EvaluatePathPriority::EvaluatePathPriority evaluatePathPriority);

        /**
         *
         */
        Uint16 createSourcePath(Address32 sourceAddress, Address32 destinationAddress, Uint16 bandwidth,
                    bool managementPath, Uint8 maxHops);

        /**
         *
         */
        Uint16 createGraphPath(Address32 sourceAddress, Address32 destinationAddress, Uint16 bandwidth,
                    bool managementPath, bool broadcastAllocation, Uint8 redundantFactor, Uint8 maxHops);

        /**
         * Reevaluate one route and return true if found one
         */
        bool evaluateNextPath(NE::Model::Operations::EngineOperations& engineOperations, Path& path);

        /**
         * Update the result of the operation setting
         */
        bool resolveOperation(NE::Model::Operations::NeighborGraphAddedOperation& operation);

        /**
         * Update the result of the operation setting
         */
        bool resolveOperation(NE::Model::Operations::NeighborGraphRemovedOperation& operation);

        /**
         *
         */

        bool releaseRemoved(NE::Model::Operations::EngineOperations& engineOperations, AddressSet& addressSet);

        /**
         *
         */
        void initOutboundDestinations(Address32 address, AddressList& destinations);

        /**
         * Remove the GN on parent on parent's graph
         */
        void removeJoinGraphNeighbor(NE::Model::Operations::EngineOperations& engineOperations, Address32 address);

        /**
         *
         */
        Address32 popOnEvaluationNodes();

        /**
         *
         */
        void pushOnEvaluationNodes(Address32 address);

        /**
         * Returns the list of all devices that will be removed on CheckRemove.
         * @param addressSet
         */
        void getRemovedDevices(AddressSet& addressSet);

        void getGraphsToRefresh(int operationsIndex,
                    std::pair<std::multimap<int, Uint16>::iterator, std::multimap<int, Uint16>::iterator>& rez);

        void removeGraphsFromRefreshing(std::pair<std::multimap<int, Uint16>::iterator, std::multimap<int, Uint16>::iterator>& rez);

        void deletePath(PathMap::iterator it);


    private:

        /**
         * Set the reevaluate status
         */
        void updateReevaluatePaths(Address32 source, Address32 destination, bool fastEvaluate);

        /**
         * Marks nodes as reachable.
         */
        void markReachableNodes(Address32 address, Uint16 searchId);



    public:

        /**
         *
         */
        std::string toDotString();

        /**
         * Returns a user friendly string representation for the object, indented to facilitate the reading.
         */
        void toIndentString(std::ostringstream& stream);

        /**
         * Returns a string representation for the object.
         */
        void toString(std::ostringstream &stream);
};

}

}

}

#endif /* SUBNETTOPOLOGY_H_ */
