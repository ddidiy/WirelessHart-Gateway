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
 * SelectMinPath.h
 *
 *  Created on: Dec 29, 2010
 *      Author: Mihai.Stef
 *
 *
 */

#ifndef SELECTMINPATH_H_
#define SELECTMINPATH_H_

#define MAX_COST_32 0xFFFFFFFF
#define MAX_COST_16 0xFFFF

#include "../GraphRoutingAlgorithmInterface.h"
#include "../SubnetTopology.h"
#include "Common/logging.h"

namespace NE {

namespace Model {

namespace Topology {

namespace Algorithms {

using namespace NE::Model::Topology;

typedef struct {
        Address32 peer;
        AddressList pathAddrs;
}Candidate;

class SelectMinPath: public NE::Model::Topology::GraphRoutingAlgorithmInterface{
    private:

        Address32 source;
        Address32 destination;
        Uint16 traffic;
        Uint16 graphId;

        Uint32 cost;
        Uint32 pathCost;
        Uint16 hopsCost;
        Uint16 nodes;
        Uint16 redundantNodes;
        Uint16 activeEdges;
        Uint16 settingsCost;

        Address32 evalSource;
        Address32 evalDestination;
        Uint16 evalGraphId;
        Address32 evalPrimary;
        Address32 evalPrimarySecondary;
        Address32 evalSecondary;
        Address32 evalSecondaryPrimary;
        Address32 evalSecondarySecondary;


        Uint16 settingsFactor;
        Uint16 topologyFactor;
        Uint8 hopsFactor;
        Uint8 maxHops;
        Uint16 maxNeighbors;
        bool rerouteOnlyOnFail;

        std::multimap<Address32, Address32> minCostPath;

        LOG_DEF("I.M.R.A.SelectMinPath");

        bool evaluateInboundGraphPath(NE::Model::Operations::EngineOperations& engineOperations);

        Uint32 checkOutbound(Address32 primaryPeer, Address32 candidatePeer, Uint16 checkGraphId);

        bool evaluateOutboundGraphPath(NE::Model::Operations::EngineOperations& engineOperations, Path& path);

        /**
         * Evaluate the cost if in source node only the firstDestination and secondDestination edges are used
         */
        Uint32 evaluateCost(Address32 address, Uint16 searchId,  bool testCycle = true);

        /**
         * Search the graph and evaluate the costs, on source skip other edges than first and second
         */
        bool evaluateCostSearch(Address32 address, Uint16 costGraphId, Uint16 searchId, Uint16 lastHopCount, bool testCycle);

        void setMinimumInboundPath(NE::Model::Operations::EngineOperations& engineOperations, Address32 primaryPeer, Address32 secondaryPeer
                    , AddressList& primaryPath, AddressList& secondaryPath);

        /**
         * Set the current selected edges as part of the graph
         */
        void setInbound(NE::Model::Operations::EngineOperations& engineOperations, Address32 primaryPeer, Address32 secondaryPeer,
                    AddressList& primaryPath, AddressList& secondaryPath);

        void setMinimumOutboundPath(NE::Model::Operations::EngineOperations& engineOperations, Address32 primaryPeer, Address32 secondaryPeer
                    , AddressList& primaryPath, AddressList& secondaryPath);

        void checkGraph(NE::Model::Operations::EngineOperations& engineOperations, Address32 address, Uint16 searchId);

        /**
         * Set the current selected edges as part of the graph
         */
        void setOutbound(NE::Model::Operations::EngineOperations& engineOperations, Address32 primaryPeer,
                    Address32 secondaryPeer, Uint16 searchId, AddressList& primaryPath, AddressList& secondaryPath);


        bool breakCSCycles(NE::Model::Operations::EngineOperations& engineOperations, Address32 newPrimary, std::set<Address32>& nodesSoFar);

        bool createsClockSourceCycle(Address32 secondary, bool checkSecondary);

        bool testForCycles(AddressList& primaryPath, AddressList& secPath);

        Uint16 checkEdge(Node& src, Node& dest);

    public:

        SelectMinPath();
        virtual ~SelectMinPath();

        bool evaluateGraphPath(NE::Model::Operations::EngineOperations& engineOperations, Path& path);


};

}

}

}

}

#endif /* SELECTMINPATH_H_ */
