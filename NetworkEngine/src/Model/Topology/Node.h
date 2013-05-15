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

#ifndef ROUTINGNODE_H_
#define ROUTINGNODE_H_

#include <vector>
#include <set>

#include "Common/NEAddress.h"
#include "Common/logging.h"
#include "Model/MetaDataAttributes.h"
#include "Model/Topology/TopologyTypes.h"
#include "Model/Topology/Edge.h"
#include "Model/Operations/EngineOperations.h"
#include "Model/Operations/Topology/NeighborGraphAddedOperation.h"
#include "Model/Operations/Topology/NeighborGraphRemovedOperation.h"

namespace NE {

namespace Model {

namespace Topology {

class Node;
typedef std::map<Address32, Node> NodeMap;

typedef std::map<Address32, Edge*> EdgeMap;

/**
 *
 * @author Radu Pop, Ioan-Vasile Pocol
 * @version 1.0
 */
class Node {
    LOG_DEF("I.M.G.Node");

    private:

        /**
         * The address of the current node.
         */
        Address32 nodeAddress;

        /**
         * The address of the parent.
         */
        Address32 parentAddress;

        /**
         * The backbone address32.
         */
        Address32 backboneAddress;

        /**
         * In bound management route
         */
        Uint16 inboundGraphId;

        /**
         *
         */
        Address32 primaryOutbound;

        /**
         *
         */
        Address32 secondaryOutbound;

        /**
         *
         */
        Address32 primaryInbound;

        /**
         *
         */
        Address32 secondaryInbound;

        /**
         *
         */
        Address32 secondaryClkSource;

        /**
         * Out bound management route
         */
        Uint16 outboundGraphId;

        /**
         * Represents the type of the device : System/Network Manager, Backbone, GW, etc.
         */
        NodeType::NodeTypeEnum nodeType;

        /**
         * If the node is router
         */
        bool router;

        /**
         * The status of the available and allocated resources on device.
         */
        MetaDataAttributesPointer metaDataAttributes;

        /**
         * The list of edges (existing and visible edges) that exits from this node.
         */
        EdgeList outBoundEdges;
        EdgeMap outBoundEdgesMap;

        /**
         * When a node is removed from the subnet, all the nodes that are affected
         * by this removal are marked for remove.
         */
        Status::StatusEnum status;

        /**
         * The current set search ID.
         */
        Uint16 searchId;

        /**
         * The current set search ID for the evaluation of the lazy parent.
         */
        Uint16 evalSearchId;

        /**
         * Represents the cost of the necessary settings that have to be done on the devices.
         */
        Uint16 settingsCost;

        /**
         * The list of edges for the current evaluated graph path.
         */
        std::map<Address32, bool> evalGraphNeighbors;

        /**
         * The last parent on the eval graph
         */
        Address32 evalLazyParent;

        Uint16 activeEdgesNo;

        /**
         *
         */
        Uint8 deep;

        /**
         * Caches the neighbor added operation for each added neighbor.
         * On confirmed we should delete reset the operation.
         */
        std::map<std::pair<Uint16, Address32>, NE::Model::Operations::IEngineOperationPointer>
                    neighborGraphAddedOperations;

        Address32 oldPInbound;
        Address32 oldSInbound;
        Address32 oldPOutbound;
        Address32 oldSOutbound;

        Uint8 linkedNeighborsCount;

    public:

        /**
         *
         */
        Node() {
        }

        /**
         *
         */
        Node(Address32 nodeAddress, Address32 parentAddress, Address32 backboneAddress,
                    NodeType::NodeTypeEnum deviceType);

        /**
         *
         */
        virtual ~Node();

        /**
         *
         */
        Address32 getNodeAddress();

        /**
         *
         */
        Address32 getParentAddress();

        /**
         *
         */
        Address32 getBackboneAddress();

        /**
         *
         */
        MetaDataAttributesPointer getMetaDataAttributes();

        /**
         *
         */
        void setMetaDataAttributes(MetaDataAttributesPointer _metaDataAttributes);

        /**
         *
         */
        Uint16 getInboundGraphId();

        /**
         *
         */
        void setInboundGraphId(Uint16 inboundGraphId);

        /**
         *
         */
        Uint16 getOutboundGraphId();

        /**
         *
         */
        void setOutboundGraphId(Uint16 outboundGraphId);

        /**
         *
         */
        Address32 getPrimaryOutbound();

        /**
         *
         */
        void setPrimaryOutbound(Address32 primaryOutbound_);

        /**
         *
         */
        Address32 getSecondaryOutbound();

        /**
         *
         */
        void setSecondaryOutbound(Address32 secondaryOutbound_);

        /**
         *
         */
        Address32 getPrimaryInbound();

        /**
         *
         */
        void setPrimaryInbound(Address32 primaryInbound_);

        /**
         *
         */
        Address32 getSecondaryInbound();

        /**
         *
         */
        void setSecondaryInbound(Address32 secondaryInbound_);

        /**
         *
         */
        EdgeList& getOutBoundEdges();

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
        bool existsGraphNeighbor(Uint16 graphId);

        /**
         *
         */
        bool hasNeighbor(Address32 address);

        /**
         *
         */
        bool hasNeighborOnGraph(Address32 address, Uint16 graphId);

        /**
         * Return true if the node has other neighbor on graph excepting the one received as parameter.
         */
        bool hasOtherOutboundNeighborOnGraph(Address32 address, Uint16 graphId);

        /**
         * Return true if the node has other in neighbor on graph excepting the one received as parameter.
         */
        bool hasOtherInboundNeighborOnGraph(Address32 address, Uint16 graphId);

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
        NodeType::NodeTypeEnum getNodeType();

        /**
         *
         */
        bool isRouter();

        /**
         *
         */
        void setRouter(bool router_);

        /**
         * Equal operator.
         * @param the Node to compare with
         */
        bool operator==(const Node& node);

        /**
         * Less operator.
         * @param compare the Node to compare with
         */
        bool operator<(const Node& node);

        /**
         * Return true if the neighbor is part of device -> sm management graph
         */
        bool isSourceClock(Address32 destination);

        /**
         * Populate the list of addresses with the addresses of neighbors
         * on in-bound management route
         */
        void getListOfClockSources(std::vector<Address32>& list);

        /**
         * Adds the specified node as a visible neighbor to the current node.
         * Returns <code>true</code> if the the node was not already added as a visible neighbor or as a neighbor.
         * Returns <code>false</code> if the node was already added to the list of visible neighbor or as a neighbor.
         */
        bool addVisibleNeighbor(Address32 peerAddress, Uint8 rsl);

        /**
         *
         */
        bool addDiagnostics(Address32 neighborAddress, Uint8 rsl, Uint16 sent, Uint16 received, Uint16 failed);

        /**
         * Evaluate the settings cost
         */
        Uint16 evaluateSettingsCost();

        /**
         *  Return the settings cost
         */
        Uint16 getSettingsCost();

        /**
         * Returns the edge to the given neighbor
         */
        Edge& getEdge(Address32 address32);

        /**
         * Checks to see if there is an outbound edge with the given address.
         * If there is returns false, otherwise create a new Edge, adds it to
         * the list of outbound edges and returns true.
         */
        bool addEdge(Address32 address);

        /**
         * Add the existing edge between two nodes to be part of the graph
         */
        void addSourceNeighbor(Uint16 graphId, Uint16 traffic, Address32 address);

        /**
         * Delete the neighbor on graph
         */
        void removeSourceNeighbor(Uint16 graphId, Address32 address);

        void resetPrefferdNeighbor(Uint16 graphId, Address32 neighborAddress);

        void setPrefferdNeighbor(Uint16 graphId, Address32 neighborAddress);

        /**
         * Add the existing edge between two nodes to be part of the graph
         */
        void addGraphNeighbor(NE::Model::Operations::EngineOperations& engineOperations, Uint16 graphId,
                    Uint16 traffic, Address32 address, bool preffered, bool addAlways = false,
                    bool isBackboneException = false);

        /**
         * Delete the neighbor on graph
         */
        void setRemoveGraphNeighbor(NE::Model::Operations::EngineOperations& engineOperations, Uint16 graphId,
                    Address32 address);

        /**
         * Returns the last parent that touched the node in the current eval graph
         */
        Address32 getEvalLazyParent();

        /**
         * Set the last parent that touched the node in the current eval graph
         */
        void setEvalLazyParent(Address32 lazyParent, Uint8 deep, Uint16 evalSearchId);

        void setDeep(Uint8 deep_);

        /**
         * Set the current node as the last one in the current graph that will visit the peer one
         */
        GraphNeighbor& updateLazyParent(Address32 peer, Address32 peerLazyParent, Uint16 graphId);

        /**
         *
         */
        void addEvalGraphNeighbor(Address32 address);

        /**
         * Return the selected graph neighbors
         */
        std::map<Address32, bool>& getEvalGraphNeighbors();

        /**
         * Reset the eval graph neighbor
         */
        void resetEvalGraphNeighbors();

        /**
         *
         */
        Uint16 evaluateEdgeCost(Address32 address, Uint16 graphId, Uint16 traffic);

        /**
         *
         */
        Uint16 getEvalEdgeCost(Address32 address);

        /**
         * Update the result of the operation setting
         */
        bool resolveOperation(NE::Model::Operations::NeighborGraphAddedOperation& operation);

        /**
         * Update the result of the operation setting
         */
        bool resolveOperation(NE::Model::Operations::NeighborGraphRemovedOperation& operation);

        /**
         * Generates clock sources for the node.
         */
        void generateClockSources(NE::Model::Operations::EngineOperations& operations, Uint16 graphId);

        /**
         * Called when the graph added operation has been confirmed.
         */
        void confirmAddGraphNeighborOperation(Uint16 graphId, Address32 address);

        /**
         * Returns true if the node has a secondary clock source set.
         */
        bool hasSecondaryClkSource();

        /**
         * Sets the secondary source clock address for the node.
         * @param addr - sec clk src address
         */
        void setSecondaryClkSource(Address32 addr);

        /**
         * Returns the address of the secondary clock source.
         */
        Address32 getSecondaryClkSource();

        /**
         * Resets the secondary clock source address (set the address to 0x0000).
         */
        void resetSecondaryClkSource();

        void removeEdge(EdgeList::iterator& it_edge);

        bool maxNeighborsNoReached();

        void setLinkedNeighborsCount(Uint8 linkedNeighbors) {
            linkedNeighborsCount = linkedNeighbors;
        }

        Uint8 getLinkedNeighborsCount() {
            return linkedNeighborsCount;
        }

        /**
         * Re-sort edges if reported rsl is changed and provides the insertion position for new edges
         */
        bool updateEdgesList(Address32 peerAddress, Uint8 rsl, EdgeList::iterator& insert_position);


        void goBackToOldInbound(Address32 address);
        void goBackToOldOutbound(Address32 address);
        void resetOutboundOldies();
        void resetInboundOldies();

        void setEvalLazyParent(Address32 evalLazyParent_);

        /**
         *
         */
        class IndentString {
            public:

                const Node& node;

                IndentString(const Node& node_) :
                    node(node_) {
                }
        };

        /**
         * Returns a string representation of this node.
         */
        friend std::ostream& operator<<(std::ostream&, const Node&);

        /**
         * Returns a friendly string representation of this node.
         */
        friend std::ostream& operator<<(std::ostream&, const Node::IndentString&);
};

}

}

}

#endif /* ROUTINGNODE_H_ */
