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
 * NodeTdma.h
 *
 *  Created on: Mar 17, 2009
 *      Author: ioanpocol
 */

#ifndef NODETDMA_H_
#define NODETDMA_H_

#include <map>

#include "Model/Operations/EngineOperations.h"
#include "Model/Tdma/TimeslotAllocation.h"
#include "Model/Tdma/LinkEdge.h"
#include "Model/Tdma/AllocationBitmap.h"
#include "Model/Tdma/TdmaTypes.h"

#include "Model/Topology/TopologyTypes.h"

namespace NE {

namespace Model {

namespace Tdma {

class NodeTdma;
typedef std::map<Address32, NodeTdma> NodeTdmaMap;

class SubnetTdma;

class NodeTdma {
    LOG_DEF("I.M.G.NodeTdma");

    public:

        TimeslotAllocations timeslotAllocationsArray[MAX_STAR_INDEX];
        AllocationBitmap allocationBitmapes[MAX_STAR_INDEX];

    private:

        SubnetTdma& subnetTdma;
        Address32 address;
        std::map<Uint64, Uint8> handlerIndex;
        std::map<Uint64, Uint8> retryHandlerIndex;

        //temporary data
        Uint8 latency;
        Uint32 changed;

        bool useSingleReception;

        bool useBestFitAllocation;

        NE::Model::Topology::NodeType::NodeTypeEnum nodeType;
        Status::StatusEnum status;

        /* the allocated superframes as bits set, the most significant bit is for the supeframe with id 1 the second for superframe with id 2 and so on*/
        Uint16 superframes;

    public:

//        NodeTdma();

        NodeTdma(SubnetTdma& subnetTdma, Address32 address, NE::Model::Topology::NodeType::NodeTypeEnum nodeType);

        /**
         *
         */
        virtual ~NodeTdma();

        /**
         *
         */
        Address32 getAddress();

        /**
         *
         */
        void setAddress(Address32 address);

        /**
         *
         */
        NE::Model::Topology::NodeType::NodeTypeEnum getNodeType();

        /**
         *
         */
        Status::StatusEnum& getStatus();

        /**
         *
         */
        void setStatus(Status::StatusEnum status);

        /**
         *
         */
        Uint8 getLatency();

        /**
         *
         */
        void setLatency(Uint8 latency);

        /**
         *
         */
        void resetLatency();

        /**
         *
         */
        bool isChanged();

        /**
         *
         */
        bool isFreePosition(Uint8 starIndex, const AllocationBitmap& allocationBitmap);


        void useSuperframe(NE::Model::Operations::EngineOperations& engineOperations, PublishPeriod::PublishPeriodEnum publishPeriod);

        /**
         *
         */
        void allocate(NE::Model::Operations::EngineOperations& engineOperations, Uint32 allocationHandler, Address32 peerAddress,
                    bool isRetry, Uint8 channel, Uint8 starIndex, AllocationBitmap& allocationBitmap, bool transmission,
                    bool reception, bool shared, LinkTypes::LinkTypesEnum linkType);

        /**
         *
         */
        bool deallocate(NE::Model::Operations::EngineOperations& engineOperations, Uint32 newAllocationHandler, Uint32 allocationHandler,
                    Address32 peerAddress, bool isRetry, Uint8& starIndex, bool deallocateOlder = true, LinkTypes::LinkTypesEnum linkType = LinkTypes::ANY);

        /**
         *
         */
        bool isUseSingleReception();

        /**
         *
         */
        bool isUseBestFitAllocation();

        /**
         * Less than operator.
         */
        bool operator<(NodeTdma &nodeTdma) const;

        /**
         *
         */
        bool resolveOperation(NE::Model::Operations::SuperframeAddedOperation& operation);

        /**
         *
         */
        bool resolveOperation(NE::Model::Operations::LinkAddedOperation& operation, bool& deallocate);

        /**
         *
         */
        bool resolveOperation(NE::Model::Operations::LinkRemovedOperation& operation);

        /**
         *
         */
        void releaseRemoved(NE::Model::Operations::EngineOperations& engineOperations);

        /**
         *
         */
        void generateRemoveJoinLinks(NE::Model::Operations::EngineOperations& engineOperations, Address32 childAddress);

        /**
         * Returns the number of links used by this device.
         * If it has single reception enabled then ignores all the reception links and consider only one.
         */
        uint16_t getTimeslotAllocationsArraySize();

        /**
         * Returns the number of neighbors with links.
         */
        uint16_t getLinkedNeighborsNo();

        /**
         *
         */
        void toString(std::ostringstream& stream);
};

}
}
}
#endif /* NODETDMA_H_ */
