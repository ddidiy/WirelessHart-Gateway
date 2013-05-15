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
 * SubnetTdma.h
 *
 *  Created on: Mar 17, 2009
 *      Author: ioanpocol
 */

#ifndef SUBNETTDMA_H_
#define SUBNETTDMA_H_

#include "Model/Operations/EngineOperations.h"
#include "Model/Tdma/NodeTdma.h"
#include "Model/Tdma/LinkEdge.h"
#include "Model/Tdma/ChannelTimeslotAllocation.h"
#include "Model/Tdma/TdmaTypes.h"
#include "Model/Operations/Tdma/SuperframeAddedOperation.h"
#include "Model/Operations/Tdma/LinkAddedOperation.h"
#include "Model/Services/SubnetServices.h"
#include "Model/Topology/TopologyTypes.h"

namespace NE {

namespace Model {

namespace Tdma {

namespace AllocationDirection
{
    enum Direction
    {
        Outbound,
        Inbound
    };
}


class SubnetTdma {
    LOG_DEF("I.M.G.SubnetTdma");

    private:

        NodeTdmaMap nodes;

        /** contains the structure of the allocation for the given channel timeslot */
        ChannelTimeslotAllocation channelTimeslotAllocations[MAX_STAR_INDEX][16];

        Uint8 maxChannels;

        Uint8 apChannel;

    public:

        /**
         *
         */
        SubnetTdma();

        /**
         *
         */
        void init(Address32 nmAddress);

        /**
         * Return true if the node already has set a star superframe.
         */
        bool existsNode(Address32 address32);

        /**
         * Set the channel map.
         */
        void setChannelMap(Uint16 channelMap);

        /**
         *
         */
        bool addNode(NE::Model::Operations::EngineOperations& engineOperations, Uint32 inboundAllocationHandler,
                    Uint32 outboundAllocationHandler, Address32 parentAddress, Address32 address,
                    NE::Model::Topology::NodeType::NodeTypeEnum nodeType,
                    PublishPeriod::PublishPeriodEnum managementPeriod, PublishPeriod::PublishPeriodEnum joinPeriod);
        /**
         *
         */
        NodeTdma& getNodeTdma(Address32 address);

        /**
         * Allocated the first next free adv link.
         */
        bool broadcastAllocateNextLink(NE::Model::Operations::EngineOperations& engineOperations, Address32 address);

        /**
         * Allocate a link to check the visibility between the nodes.
         */
        bool allocateCheckLink(NE::Model::Operations::EngineOperations& engineOperations, Uint32 allocationHandler,
                    Address32 address, Address32 peerAddress);

        /**
         * Deallocate a link used to check the visibility between the nodes.
         */
        bool deallocateCheckLink(NE::Model::Operations::EngineOperations& engineOperations, Uint32 allocationHandler,
                    Address32 address, Address32 peerAddress);

        /**
         * Allocate unique reception link
         */
        void receptionLinkAllocation(NE::Model::Operations::EngineOperations& engineOperations,
                    Uint32 outboundAllocationHandler, Address32 address);

        /**
         *
         */
        bool broadcastAllocation(NE::Model::Operations::EngineOperations& engineOperations, Uint32 allocationHandler,
                    Address32 address, PublishPeriod::PublishPeriodEnum publishPeriod, bool transmit, bool shared,
                    LinkTypes::LinkTypesEnum linkType, int startSlot = 0);

        /**
         *
         */
        bool broadcastDeallocation(NE::Model::Operations::EngineOperations& engineOperations, Uint32 allocationHandler,
                    Address32 address, LinkTypes::LinkTypesEnum linkType);

        /**
         *
         */
        bool allocatePublishTraffic(NE::Model::Operations::EngineOperations& engineOperations,
                    Uint32 allocationHandler, LinkEdgesList& edges, PublishPeriod::PublishPeriodEnum publishPeriod,
                    Uint8 phase, Uint8 latency, AllocationDirection::Direction direction);

        /**
         *
         */
        void deallocatePublishTraffic(NE::Model::Operations::EngineOperations& engineOperations,
                    Uint32 newAllocationHandler, Uint32 allocationHandler, LinkEdgesList& edges);

        /**
         *
         */
        bool resolveOperation(NE::Model::Operations::SuperframeAddedOperation& operation);

        /**
         *
         */
        bool resolveOperation(NE::Model::Operations::LinkAddedOperation& addLinkOperation);

        /**
         *
         */
        bool resolveOperation(NE::Model::Operations::LinkRemovedOperation& removeLinkOperation);

        /**
         * Return true if the status of the node is removed
         */
        bool isRemovedStatus(Address32 address);

        /**
         * Set the status of the nod as REMOVED
         */
        void setRemovedStatus(Address32 address);

        /**
         *
         */
        void releaseRemoved(NE::Model::Operations::EngineOperations& engineOperations);

        /**
         *
         */
        void deallocateChannel(Uint8 starIndex, Uint8 channelIndex, AllocationBitmap& allocationBitmap);

        /**
         *
         */
        bool
                    allocateJoinLinks(NE::Model::Operations::EngineOperations& engineOperations,
                                Uint32 outboundAllocationHandler, Address32 address,
                                PublishPeriod::PublishPeriodEnum publishPeriod);

        /**
         *
         */
        void discoveryAllocation(NE::Model::Operations::EngineOperations& engineOperations, Uint32 allocationHandler,
                    Address32 address, NE::Model::Topology::NodeType::NodeTypeEnum nodeType);

        /**
         *
         */
        void removeJoinLinks(NE::Model::Operations::EngineOperations& engineOperations, Address32 address,
                    Address32 parent);

        /**
         *
         */
        void toString(std::ostringstream& stream);

        /**
         * Execute Compact Links flow
         * @param engineOperations
         */
        void compactLinks(NE::Model::Operations::EngineOperations& engineOperations);

    private:

        /**
         *
         */
        bool attachAllocation(NE::Model::Operations::EngineOperations& engineOperations,
                    Uint32 inboundAllocationHandler, Uint32 outboundServiceId, Address32 parentAddress,
                    Address32 address, PublishPeriod::PublishPeriodEnum publishPeriod);

        /**
         *
         */
        bool allocateLink(NE::Model::Operations::EngineOperations& engineOperations, Uint32 allocationHandler,
                    Address32 source, Address32 destination, Uint8& starIndex, AllocationBitmap& allocationBitmap,
                    LinkTypes::LinkTypesEnum linkType);

        /**
         *
         */
        void initIndexes(PublishPeriod::PublishPeriodEnum publishPeriod, Uint8 phase, Uint8 latency, Uint8& starIndex,
                    Uint8& timeslotIndex);

        /**
         *
         */
        bool existsChannelAllocation(Uint8 starIndex, const AllocationBitmap& allocationBitmap, bool useSingleReception);

        /**
         *
         */
        bool isSlotOk(Uint8 starIndex, const AllocationBitmap& allocationBitmap, bool useSingleReception);

        /**
         *
         */
        Uint8 allocateChannel(Uint8 starIndex, const AllocationBitmap& allocationBitmap, bool useSingleReception);

        /**
         *
         */
        void checkAllLinks();

};

}
}
}
#endif /* SUBNETTDMA_H_ */
