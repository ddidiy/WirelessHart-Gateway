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
 * SubnetTdma.cpp
 *
 *  Created on: Mar 17, 2009
 *      Author: ioanpocol
 */

#include <cstdlib>

#include "SubnetTdma.h"
#include "Model/NetworkEngine.h"
#include "Model/Operations/Tdma/LinkAddedOperation.h"
#include "Model/Operations/Tdma/SuperframeAddedOperation.h"
#include "Model/Operations/Tdma/SetChannelsBlacklistOperation.h"

using namespace NE::Model::Tdma;
using namespace NE::Model::Operations;
using namespace NE::Model::Topology;
using namespace NE::Model::Services;

const Uint32 REMOVE_JOIN_LINKS_HANDLER = 0xFFFFFFFF;

SubnetTdma::SubnetTdma() {
}

bool SubnetTdma::existsNode(Address32 address) {
    return nodes.find(address) != nodes.end();
}

void SubnetTdma::init(Address32 nmAddress) {
    NodeTdma node(*this, nmAddress, NodeType::MANAGER);
    nodes.insert(std::make_pair(nmAddress, node));

    apChannel = 0;
}

void SubnetTdma::setChannelMap(Uint16 channelMap) {
    maxChannels = 0;
    Uint16 mask = 1;
    for (int i = 0; i < 16; i++) {
        if (channelMap & mask) {
            maxChannels++;
        }
        mask = mask << 1;
    }

    LOG_DEBUG("Computed MAX CHANNELS:" << (int) maxChannels);
    if (maxChannels >= 16) {
        LOG_DEBUG("MAX CHANNELS clamped to 15.");
        maxChannels = 15;
    }

    apChannel = (Uint8) (rand() % (int) maxChannels);
    LOG_DEBUG("setChannelMap index =" << (int) apChannel);

    mask = 1;
    for (int i = 0; i < 16; i++) {
        LOG_DEBUG("setChannelMap index i=" << (int) i);
        if (channelMap & mask) {
            if (apChannel == 0) {
                apChannel = i;
                LOG_DEBUG("setChannelMap - apChannel=" << (int) apChannel);
                return;
            }
            apChannel--;
        }
        mask = mask << 1;
    }
}

bool SubnetTdma::addNode(EngineOperations& engineOperations, Uint32 inboundAllocationHandler,
            Uint32 outboundAllocationHandler, Address32 parentAddress, Address32 address,
            NodeType::NodeTypeEnum nodeType, PublishPeriod::PublishPeriodEnum managementPeriod,
            PublishPeriod::PublishPeriodEnum joinPeriod) {

    NodeTdma node(*this, address, nodeType);
    if (nodes.find(address) != nodes.end()) {
        nodes.erase(address);
    }
    nodes.insert(std::make_pair(address, node));

    // If TR, set the channels map before adding any SF/Link
    if (nodeType == NodeType::BACKBONE) {
        IEngineOperationPointer operationSuperframe(new SuperframeAddedOperation(address, 0,
                    SuperframeLength::SLENGTH_250_MS, 1));

        operationSuperframe->setDependency(WaveDependency::FIRST);
        engineOperations.addOperation(operationSuperframe);

        getNodeTdma(address).useSuperframe(engineOperations, joinPeriod);

        SetChannelsBlacklistOperation *channelMapOperation = new SetChannelsBlacklistOperation(address);
        channelMapOperation->AvailableChannels = NetworkEngine::instance().getSettingsLogic().channelMap;

        IEngineOperationPointer operation(channelMapOperation);
        operation->setDependency(WaveDependency::FIRST);
        //TODO: ivp - check why Bullet and Emerson are not joining
        engineOperations.addOperation(operation);

        discoveryAllocation(engineOperations, outboundAllocationHandler, address, nodeType);

        if (NE::Model::NetworkEngine::instance().getSettingsLogic().useSingleReception) {
            receptionLinkAllocation(engineOperations, outboundAllocationHandler, address);
        }

        broadcastAllocation(engineOperations, outboundAllocationHandler, address, PublishPeriod::P_250_MS, true, true,
                    LinkTypes::JOIN);

        broadcastAllocation(engineOperations, outboundAllocationHandler, address, PublishPeriod::P_250_MS, false,
                    false, LinkTypes::JOIN);

        {
            Uint32 advOutboundAllocationHandler =
                        NetworkEngine::instance().getSubnetServices().getNodeServiceMap()[address].getCheckHandler();
            Uint8 counter = (advOutboundAllocationHandler << 16) >> 24;
            for (int i = 0; i < NetworkEngine::instance().getSettingsLogic().transceiverAdvertiseLinksNo; i++) {
                counter++;
                advOutboundAllocationHandler = (advOutboundAllocationHandler & 0xFFFF00FF) + (((Uint16) counter) << 8);
                broadcastAllocation(engineOperations, advOutboundAllocationHandler, address,
                            NE::Model::NetworkEngine::instance().getSettingsLogic().advPublishPeriod, true, false,
                            LinkTypes::NORMAL);
            }

            NE::Model::Operations::WriteTimerIntervalOperation * operation =
                        new NE::Model::Operations::WriteTimerIntervalOperation(address, 250,
                                    WirelessTimerCode_Advertisment);

            NE::Model::Operations::IEngineOperationPointer op(operation);
            op->setDependency(WaveDependency::FIRST);
            engineOperations.addOperation(op);
        }

    } else {
        attachAllocation(engineOperations, inboundAllocationHandler, outboundAllocationHandler, parentAddress, address,
                    managementPeriod);
    }

    return true;
}

NodeTdma& SubnetTdma::getNodeTdma(Address32 address) {
    NodeTdmaMap::iterator it = nodes.find(address);

    if (it != nodes.end()) {
        return it->second;
    }

    std::ostringstream stream;
    stream << "Not found NodeTdma: " << ToStr(address);
    LOG_ERROR(stream.str());
    throw NEException(stream.str());
}

bool SubnetTdma::allocateCheckLink(NE::Model::Operations::EngineOperations& engineOperations, Uint32 allocationHandler,
            Address32 address, Address32 peerAddress) {
    LOG_DEBUG("allocateCheckLink() (" << ToStr(address) << ", " << ToStr(peerAddress) << "), allocationHandler="
                << std::dec << (int) allocationHandler);
    AllocationBitmap allocationBitmap;
    Uint8 starIndex = 0;
    Uint8 crtStarIndex;
    Uint8 channel;

    PublishPeriod::PublishPeriodEnum publishPeriod = PublishPeriod::P_8_S;
    allocationBitmap.initAllocationBitmap(0, publishPeriod);

    NodeTdma& node = getNodeTdma(address);
    NodeTdma& peerNode = getNodeTdma(peerAddress);

    crtStarIndex = starIndex;

    while (!(node.isFreePosition(crtStarIndex, allocationBitmap) && peerNode.isFreePosition(crtStarIndex,
                allocationBitmap) && existsChannelAllocation(crtStarIndex, allocationBitmap,
                node.isUseSingleReception()) && isSlotOk(crtStarIndex, allocationBitmap, node.isUseSingleReception()))) {

        crtStarIndex = (crtStarIndex + 1) % MAX_STAR_INDEX;

        LOG_DEBUG("allocateCheckLink - crtStarIndex=" << (int) crtStarIndex << "  starIndex=" << (int) starIndex);

        if (crtStarIndex == starIndex) {
            bool existsNewPosition = allocationBitmap.selectNextPosition();
            if (!existsNewPosition) {

                LOG_ERROR("allocateCheckLink - STOP - crtStarIndex: " << (int) crtStarIndex << " == starIndex:"
                            << (int) starIndex);
                return false;
            }
        }
    }

    starIndex = crtStarIndex;
    channel = allocateChannel(starIndex, allocationBitmap, node.isUseSingleReception());

    node.allocate(engineOperations, allocationHandler, peerAddress, false, channel, starIndex, allocationBitmap, true,
                false, false, LinkTypes::NORMAL); //LinkTypes::DISCOVERY);

    peerNode.allocate(engineOperations, allocationHandler, address, false, channel, starIndex, allocationBitmap, false,
                true, false, LinkTypes::NORMAL); // LinkTypes::DISCOVERY);

    //create dependencies between the last two operations
    EngineOperationsVector& ops = engineOperations.getEngineOperations();
    if ( ops.size() == 2) {
        EngineOperationsVector::iterator it_rx = ops.end() - 1;
        EngineOperationsVector::iterator it_tx = ops.end() - 2;
        (*it_tx)->setOperationDependency(*it_rx);
    }


    return true;
}

bool SubnetTdma::broadcastDeallocation(NE::Model::Operations::EngineOperations& engineOperations,
            Uint32 allocationHandler, Address32 address, LinkTypes::LinkTypesEnum linkType) {
    uint32_t peerAddress = 0xFFFF; // broadcast

    LOG_DEBUG("broadcastDeallocation() (" << ToStr(address) << ", " << ToStr(peerAddress) << "), allocationHandler="
                << std::dec << (long) allocationHandler << ", link type: " << linkType);

    Uint8 starIndexSource;

    NodeTdma& nodeSource = getNodeTdma(address);

    nodeSource.resetLatency();

    bool resultSource = nodeSource.deallocate(engineOperations, allocationHandler, allocationHandler, peerAddress, false, starIndexSource, false, linkType);

    if (!resultSource) {
        LOG_ERROR("broadcastDeallocation - no allocation for source=" << ToStr(address));

    }

    return true;
}

bool SubnetTdma::deallocateCheckLink(NE::Model::Operations::EngineOperations& engineOperations,
            Uint32 allocationHandler, Address32 address, Address32 peerAddress) {
    LOG_DEBUG("deallocateCheckLink() (" << ToStr(address) << ", " << ToStr(peerAddress) << "), allocationHandler="
                << std::dec << (long) allocationHandler);

    Uint8 starIndexSource;
    Uint8 starIndexDestination;

    NodeTdma& nodeSource = getNodeTdma(address);
    NodeTdma& nodeDestination = getNodeTdma(peerAddress);

    nodeSource.resetLatency();
    nodeDestination.resetLatency();

    bool resultSource = nodeSource.deallocate(engineOperations, allocationHandler, allocationHandler, peerAddress, false, starIndexSource, false);

    bool resultDestination = nodeDestination.deallocate(engineOperations, allocationHandler, allocationHandler, address, false,
                starIndexDestination, false);

    if ( ( resultSource ) && ( resultDestination ) ) {
        //create dependencies between the last two operations
        EngineOperationsVector& ops = engineOperations.getEngineOperations();
        EngineOperationsVector::iterator it_rx = ops.end() - 1;
        EngineOperationsVector::iterator it_tx = ops.end() - 2;
    }

    if (!resultSource && !resultDestination) {
        LOG_ERROR("deallocateCheckLink - no allocation for source=" << ToStr(address) << ", destination=" << ToStr(
                    peerAddress));

    }

    if (starIndexSource != starIndexDestination) {
        LOG_ERROR("deallocateCheckLink - deallocation error - idxS=" << (int) starIndexSource << " idxD="
                    << (int) starIndexDestination);
    }

    return true;
}

void SubnetTdma::receptionLinkAllocation(EngineOperations& engineOperations, Uint32 outboundAllocationHandler,
            Address32 address) {

    IEngineOperationPointer operationSuperframe(new SuperframeAddedOperation(address, SuperframeId::SID_ALL,
                SuperframeLength::SLENGTH_ALL, 1));

    operationSuperframe->setDependency(WaveDependency::FIRST);
    engineOperations.addOperation(operationSuperframe);

    IEngineOperationPointer operationLinkAdd(new LinkAddedOperation(address, outboundAllocationHandler,
                SuperframeId::SID_ALL, PublishPeriod::P_ALL, 0, 0, apChannel + 1, BROADCAST_ADDRESS, 0, 1, 0,
                LinkTypes::NORMAL));

    operationLinkAdd->setDependency(WaveDependency::SECOND);
    engineOperations.addOperation(operationLinkAdd);

}

bool SubnetTdma::broadcastAllocation(EngineOperations& engineOperations, Uint32 allocationHandler, Address32 address,
            PublishPeriod::PublishPeriodEnum publishPeriod, bool transmit, bool shared,
            LinkTypes::LinkTypesEnum linkType, int startSlot) {

    AllocationBitmap allocationBitmap;
    Uint8 starIndex = startSlot % MAX_STAR_INDEX;
    Uint8 crtStarIndex;
    Uint8 channel;

    allocationBitmap.initAllocationBitmap(0, publishPeriod);

    NodeTdma& node = getNodeTdma(address);

    crtStarIndex = starIndex;

    while (!(node.isFreePosition(crtStarIndex, allocationBitmap) && existsChannelAllocation(crtStarIndex,
                allocationBitmap, node.isUseSingleReception()) && isSlotOk(crtStarIndex, allocationBitmap, node.isUseSingleReception()))) {

        bool existsNewPosition = allocationBitmap.selectNextPosition();

        if (!existsNewPosition) {
            crtStarIndex = (crtStarIndex + 7) % MAX_STAR_INDEX;
            LOG_DEBUG("broadcastAllocation - crtStarIndex=" << (int) crtStarIndex << "  starIndex=" << (int) starIndex);
            allocationBitmap.initAllocationBitmap(0, publishPeriod);

            if (crtStarIndex == starIndex) {

                LOG_ERROR("broadcastAllocation - STOP - crtStarIndex: " << (int) crtStarIndex << " == starIndex:"
                            << (int) starIndex);
                return false;
            }
        }
    }

    starIndex = crtStarIndex;
    channel = allocateChannel(starIndex, allocationBitmap, node.isUseSingleReception());

    node.allocate(engineOperations, allocationHandler, BROADCAST_ADDRESS, false, channel, starIndex, allocationBitmap,
                transmit, !transmit, shared, linkType);

    return true;
}

bool SubnetTdma::broadcastAllocateNextLink(NE::Model::Operations::EngineOperations& engineOperations, Address32 address) {
    NodeTdma& nodeTdma = getNodeTdma(address);

    uint32_t outboundAllocationHandler =
                NetworkEngine::instance().getSubnetServices().getNodeServiceMap()[address].getCheckHandler();

    std::set<uint32_t> handlers; // possible handlers
    Uint8 counter = (outboundAllocationHandler << 16) >> 24;
    for (int i = 0; i < NetworkEngine::instance().getSettingsLogic().transceiverAdvertiseLinksNo; i++) {
        counter++;
        outboundAllocationHandler = (outboundAllocationHandler & 0xFFFF00FF) + (((Uint16) counter) << 8);
        handlers.insert(outboundAllocationHandler);
    }

    for (int i = 0; i < MAX_STAR_INDEX; ++i) {
        NE::Model::Tdma::TimeslotAllocations tas = nodeTdma.timeslotAllocationsArray[i];
        NE::Model::Tdma::TimeslotAllocations::iterator itTa = tas.begin();
        for (; itTa != tas.end(); ++itTa) {
            handlers.erase(itTa->allocationHandler);
        }
    }

    if (handlers.size() > 0) {
        LOG_TRACE("k3t: " << std::hex << *(handlers.begin()));
        broadcastAllocation(engineOperations, *(handlers.begin()), address,
                    NE::Model::NetworkEngine::instance().getSettingsLogic().advPublishPeriod, true, false,
                    LinkTypes::NORMAL);
        return true;
    }

    LOG_WARN("k3t There is no allocation handler available to delete.");

    return false;
}

bool SubnetTdma::attachAllocation(EngineOperations& engineOperations, Uint32 inboundAllocationHandler,
            Uint32 outboundAllocationHandler, Address32 parentAddress, Address32 address,
            PublishPeriod::PublishPeriodEnum publishPeriod) {

    Uint8 starIndex = rand() % MAX_STAR_INDEX;
    AllocationBitmap allocationBitmap;

    uint16_t maxTix = allocationBitmap.GetMaxTix(publishPeriod);
    allocationBitmap.initAllocationBitmap(rand() % maxTix, publishPeriod);

    allocateLink(engineOperations, outboundAllocationHandler, parentAddress, address, starIndex, allocationBitmap,
                LinkTypes::NORMAL);

    starIndex = (starIndex + 3) % MAX_STAR_INDEX;

    allocateLink(engineOperations, inboundAllocationHandler, address, parentAddress, starIndex, allocationBitmap,
                LinkTypes::NORMAL);

    return true;
}

void SubnetTdma::removeJoinLinks(EngineOperations& engineOperations, Address32 address, Address32 parentAddress) {
    NodeTdma& node = getNodeTdma(address);
    if (node.getNodeType() != NodeType::NODE) {
        return;
    }

    NodeTdma& parent = getNodeTdma(parentAddress);
    parent.generateRemoveJoinLinks(engineOperations, address);
}

bool SubnetTdma::allocateLink(EngineOperations& engineOperations, Uint32 allocationHandler, Address32 source,
            Address32 destination, Uint8& starIndex, AllocationBitmap& allocationBitmap,
            LinkTypes::LinkTypesEnum linkType) {

    Uint8 crtStarIndex;
    Uint8 channel;

    NodeTdma& nodeSource = getNodeTdma(source);
    NodeTdma& nodeDestination = getNodeTdma(destination);

    bool isSingleReception = nodeSource.isUseSingleReception() || nodeDestination.isUseSingleReception();

    crtStarIndex = starIndex;

    while (!(nodeSource.isFreePosition(crtStarIndex, allocationBitmap) && nodeDestination.isFreePosition(crtStarIndex,
                allocationBitmap) && existsChannelAllocation(crtStarIndex, allocationBitmap, isSingleReception)
                && isSlotOk(crtStarIndex, allocationBitmap, isSingleReception))) {

        crtStarIndex = (crtStarIndex + 1) % MAX_STAR_INDEX;

        LOG_DEBUG("allocateLink - crtStarIndex=" << (int) crtStarIndex << "  starIndex=" << (int) starIndex);

        if (crtStarIndex == starIndex) {
            bool existsNewPosition = allocationBitmap.selectNextPosition();
            if (!existsNewPosition) {

                LOG_ERROR("allocateLink - STOP - crtStarIndex: " << (int) crtStarIndex << " == starIndex:"
                            << (int) starIndex);
                return false;
            }
        }
    }

    starIndex = crtStarIndex;
    channel = allocateChannel(starIndex, allocationBitmap, isSingleReception);

    nodeSource.allocate(engineOperations, allocationHandler, nodeDestination.getAddress(), false, channel, starIndex,
                allocationBitmap, true, false, false, linkType);

    nodeDestination.allocate(engineOperations, allocationHandler, nodeSource.getAddress(), false, channel, starIndex,
                allocationBitmap, false, true, false, linkType);

    return true;
}

void SubnetTdma::discoveryAllocation(EngineOperations& engineOperations, Uint32 allocationHandler, Address32 address,
            NodeType::NodeTypeEnum nodeType) {

    NodeTdma& nodeTdma = getNodeTdma(address);

    AllocationBitmap allocationBitmap;
    allocationBitmap.initAllocationBitmap(0, PublishPeriod::P_4_S);
    nodeTdma.allocate(engineOperations, 0xFFFFFFFF/*allocationHandler*/, BROADCAST_ADDRESS, false, maxChannels, MAX_STAR_INDEX - 1,
                allocationBitmap, true, true, true, LinkTypes::DISCOVERY);

    if (nodeType == NodeType::BACKBONE) {
        AllocationBitmap allocationBitmap;
        allocationBitmap.initAllocationBitmap(0, PublishPeriod::P_4_S);

        channelTimeslotAllocations[MAX_STAR_INDEX - 1][maxChannels - 1].allocateDiscovery(allocationBitmap);
    }
}

bool SubnetTdma::isSlotOk(Uint8 starIndex, const AllocationBitmap& allocationBitmap, bool useSingleReception)
{
    if (!NE::Model::NetworkEngine::instance().getSettingsLogic().useSlotZero && starIndex == 0)
        return false;

    return true;
}

bool SubnetTdma::allocatePublishTraffic(EngineOperations& engineOperations, Uint32 allocationHandler,
            LinkEdgesList& edges, PublishPeriod::PublishPeriodEnum publishPeriod, Uint8 phase, Uint8 latency, AllocationDirection::Direction direction) {

    LOG_DEBUG("allocatePublishTraffic - allocationHandler=" << ToStr(allocationHandler) << ", publishPeriod="
                << (int) publishPeriod << ", phase=" << (int) phase << ", latency=" << (int) latency << ", direction=" << (int)direction);

    AllocationBitmap allocationBitmap;
    Uint8 starIndex;
    Uint8 crtStarIndex;
    Uint8 timeslotIndex;
    Uint8 channel;
    Uint8 crtLatency;


    initIndexes(publishPeriod, phase, latency, starIndex, timeslotIndex);
    crtStarIndex = starIndex;

    allocationBitmap.initAllocationBitmap(timeslotIndex, publishPeriod);

    int factorDir = (direction == AllocationDirection::Inbound) ? -1 : 1;

    if (direction == AllocationDirection::Inbound)
        edges.reverse();

    for (LinkEdgesList::iterator it = edges.begin(); it != edges.end(); ++it) {
        NodeTdma& nodeSource = getNodeTdma(it->getSource());
        NodeTdma& nodeDestination = getNodeTdma(it->getDestination());

        nodeSource.resetLatency();
        nodeDestination.resetLatency();
    }

    bool allocationStarted = false;

    //the list of edges are on the required allocation order
    for (LinkEdgesList::iterator it = edges.begin(); it != edges.end(); ++it) {

        LOG_DEBUG("allocatePublishTraffic - edge=(" << ToStr(it->getSource()) << ", " << ToStr(it->getDestination())
                    << ")");

        NodeTdma& nodeSource = getNodeTdma(it->getSource());
        NodeTdma& nodeDestination = getNodeTdma(it->getDestination());
        bool isSingleReception = nodeSource.isUseSingleReception() || nodeDestination.isUseSingleReception();
        bool isBestFitAllocation = nodeSource.isUseBestFitAllocation() || nodeDestination.isUseBestFitAllocation();

        Uint8 maxCount = it->isRetry() ? 2 : 1;
        Uint8 count = 0;

        while (count < maxCount) {
            if (!isBestFitAllocation)
            {
                while (!(nodeSource.isFreePosition(crtStarIndex, allocationBitmap) && nodeDestination.isFreePosition(
                            crtStarIndex, allocationBitmap) && isSlotOk(crtStarIndex, allocationBitmap, isSingleReception)
                            && existsChannelAllocation(crtStarIndex, allocationBitmap, isSingleReception))) {

                    if (!allocationStarted)
                    {
                        crtStarIndex = (crtStarIndex + 7 * factorDir + MAX_STAR_INDEX) % MAX_STAR_INDEX;    // select next sIx
                    }
                    else
                    {
                        crtStarIndex = (crtStarIndex + factorDir + MAX_STAR_INDEX) % MAX_STAR_INDEX;    // select next sIx
                    }

                    LOG_DEBUG("allocatePublishTraffic - crtStarIndex=" << (int) crtStarIndex << "  starIndex="
                                << (int) starIndex);

                    if (crtStarIndex == starIndex)  // if we have done a complete scan of sIx
                    {   // move to the next tIx
                        bool existsNewPosition = false;
                        if (factorDir == -1)
                            existsNewPosition = allocationBitmap.selectPreviousPosition();
                        else
                            existsNewPosition = allocationBitmap.selectNextPosition();

                        if (!existsNewPosition)
                        {   // reset tIx to other end
                            if (factorDir == -1)
                                allocationBitmap.initMax(publishPeriod);
                            else
                                allocationBitmap.initAllocationBitmap(0, publishPeriod);
                        }
                        if (timeslotIndex == allocationBitmap.getTimeslotIndex())   // we have done a complete scan of the tdma space
                        {
                            LOG_ERROR("allocatePublishTraffic - STOP - crtStarIndex: " << (int) crtStarIndex
                                        << " == starIndex:" << (int) starIndex);
                            return false;
                        }
                    }   // if
                } //while
                timeslotIndex = allocationBitmap.getTimeslotIndex();

            }   // if (!isBestFitAllocation)
            else
            {
                //compute the number of allocated slots
                int ones[MAX_STAR_INDEX];

                for (Uint8 six = 0; six < MAX_STAR_INDEX; six++) {
                    ones[six] = 0;
                    if ( nodeSource.isUseBestFitAllocation() ) {
                        ones[six] += nodeSource.allocationBitmapes[six].GetNumberOfAllocatedSlots();
                    }
                    if ( nodeDestination.isUseBestFitAllocation() ) {
                        ones[six] += nodeDestination.allocationBitmapes[six].GetNumberOfAllocatedSlots();
                    }
                }

                //search for the best allocation pattern
                Uint8 bestSix = 0xFF;
                Uint8 bestTix = 0xFF;
                int maxOnes =-1;
                int maxTix = allocationBitmap.GetMaxTix(publishPeriod);

                for (Uint16 tix = 0; tix < maxTix; tix++) {
                    allocationBitmap.initAllocationBitmap(tix, publishPeriod);
                    for (Uint8 six = 0; six < MAX_STAR_INDEX; six++) {
                        if (nodeSource.isFreePosition(six, allocationBitmap) && nodeDestination.isFreePosition(
                            six, allocationBitmap) && existsChannelAllocation(six, allocationBitmap,
                            isSingleReception) && isSlotOk(six, allocationBitmap, true)) {
                            if (ones[six] > maxOnes) {
                                bestSix = six;
                                bestTix = tix;
                                maxOnes = ones[six];
                            }
                        }
                    }
                }

                if (bestSix == 0xFF)
                {
                    LOG_ERROR("allocatePublishTraffic - STOP on useBestFitAllocation- bestSix = 0xFF");
                    return false;
                }

                crtStarIndex = bestSix;
                allocationBitmap.initAllocationBitmap(bestTix, publishPeriod);
            }

            LOG_DEBUG("allocatePublishTraffic - crtStarIndex=" << (int) crtStarIndex << ", count: " << (int) count
                        << ", crtLatency=" << (int) crtLatency);
            starIndex = crtStarIndex;
            channel = allocateChannel(starIndex, allocationBitmap, isSingleReception);
            allocationStarted = true;
            LOG_DEBUG("allocatePublishTraffic - channel: " << (int) channel);

            nodeSource.allocate(engineOperations, allocationHandler, nodeDestination.getAddress(), count == 1, channel,
                        starIndex, allocationBitmap, true, false, false, LinkTypes::NORMAL);

            nodeDestination.allocate(engineOperations, allocationHandler, nodeSource.getAddress(), count == 1, channel,
                        starIndex, allocationBitmap, false, true, false, LinkTypes::NORMAL);

            nodeDestination.setLatency(crtLatency);

            count++;

            //[andy] - avoid allocating green light in exactly the next slot, because a node might receive some commands and miss
            // the transmission in the next slot
            crtStarIndex = (crtStarIndex + 2 * factorDir + MAX_STAR_INDEX) % MAX_STAR_INDEX;

        }   // while (count < maxCount)
    }

    return true;
}

void SubnetTdma::deallocatePublishTraffic(EngineOperations& engineOperations, Uint32 newAllocationHandler, Uint32 allocationHandler,
            LinkEdgesList& edges) {

    Uint8 starIndexSource = 0xFF;
    Uint8 starIndexDestination = 0xFE;
    AllocationBitmap allocationBitmapSource;
    AllocationBitmap allocationBitmapDestination;

    for (LinkEdgesList::iterator it = edges.begin(); it != edges.end(); ++it) {
        LOG_DEBUG("deallocatePublishTraffic - source=" << ToStr(it->getSource()) << ", destination=" << ToStr(
                    it->getDestination()) << ", isRetry=" << (int) it->isRetry());

        NodeTdma& nodeSource = getNodeTdma(it->getSource());
        NodeTdma& nodeDestination = getNodeTdma(it->getDestination());

        nodeSource.resetLatency();
        nodeDestination.resetLatency();

        Uint8 maxCount = it->isRetry() ? 2 : 1;
        Uint8 count = 0;

        while (count < maxCount) {
            bool resultSource = nodeSource.deallocate(engineOperations, newAllocationHandler, allocationHandler,
                        nodeDestination.getAddress(), count == 1, starIndexSource);

            bool resultDestination = nodeDestination.deallocate(engineOperations, newAllocationHandler, allocationHandler,
                        nodeSource.getAddress(), count == 1, starIndexDestination);

            if (!resultSource || !resultDestination) {
                LOG_ERROR("deallocatePublishTraffic - no allocation for source=" << ToStr(it->getSource())
                            << ", destination=" << ToStr(it->getDestination()) << ", isRetry=" << (int) it->isRetry()
                            << ", crtRetryLoop=" << (int) (count == 1));

                count++;
                continue;
            }

            if (starIndexSource != starIndexDestination) {
                LOG_ERROR("deallocatePublishTraffic - deallocation error - idxS=" << (int) starIndexSource << " idxD="
                            << (int) starIndexDestination);
            }

            count++;
        }
    }
}

void SubnetTdma::initIndexes(PublishPeriod::PublishPeriodEnum publishPeriod, Uint8 phase, Uint8 latency,
            Uint8& starIndex, Uint8& timeslotIndex) {

    Uint8 realIndex = (phase * publishPeriod) / 100;

    timeslotIndex = realIndex / MAX_STAR_INDEX;
    starIndex = realIndex % MAX_STAR_INDEX;
}

bool SubnetTdma::existsChannelAllocation(Uint8 starIndex, const AllocationBitmap& allocationBitmap,
            bool useSingleReception) {
    if (useSingleReception) {
        if (channelTimeslotAllocations[starIndex][apChannel].isFreePosition(allocationBitmap)) {
            return true;
        } else {
            return false;
        }
    }

    for (int i = 0; i < maxChannels - 1; i++) {
        if (i == apChannel && NE::Model::NetworkEngine::instance().getSettingsLogic().useSingleReception) {
            continue;
        }

        if (channelTimeslotAllocations[starIndex][i].isFreePosition(allocationBitmap)) {
            return true;
        }
    }

    return false;
}

Uint8 SubnetTdma::allocateChannel(Uint8 starIndex, const AllocationBitmap& allocationBitmap, bool useSingleReception) {
    if (useSingleReception) {
        if (channelTimeslotAllocations[starIndex][apChannel].isFreePosition(allocationBitmap)) {
            channelTimeslotAllocations[starIndex][apChannel].allocate(allocationBitmap);
            return apChannel + 1;
        } else {
            LOG_ERROR("allocateChannel - the apChannel " << (int) apChannel << " is not free for starIndex="
                        << (int) starIndex);
            return apChannel + 1;
        }
    }
    // attempt to allocate the middle of the longest unallocated interval
    int startChannel = apChannel; bool haveStartChannel = false;
    if (!NE::Model::NetworkEngine::instance().getSettingsLogic().useSingleReception) {
        LOG_TRACE("AllocateChannel: not using single reception, searching for the first used channel...");
        for (int i = 0; i < maxChannels; i++) {
            if (!channelTimeslotAllocations[starIndex][i].isFreePosition(allocationBitmap)) {
                startChannel = i;
                haveStartChannel = true;
                LOG_DEBUG("AllocateChannel: first channel found at " << (int)i);
            }
        }
        if (!haveStartChannel)
        {// just return the first channel
            LOG_DEBUG("AllocateChannel: no allocated channel found, returning 1.");
            channelTimeslotAllocations[starIndex][0].allocate(allocationBitmap);
            return 1;
        }
    }

    int bestOffset = -1;
    int offset = -1;
    int maxLength = 0;
    for (int i = 0; i < maxChannels ; i++) {
        int crtChannel = (startChannel + i + 1) % maxChannels;
        if (!channelTimeslotAllocations[starIndex][crtChannel].isFreePosition(allocationBitmap) || i == (maxChannels - 1)) {
            int length = i - offset;
            LOG_DEBUG("Allocated channel found at:" << crtChannel <<", len=" << length);
            if (length > maxLength) {
                maxLength = length;
                bestOffset = i;
            }
            offset = i;
        }
    }

    if (maxLength == 0) {   // just one allocated channel found
        LOG_DEBUG("Just one allocated channel found at " << (int)startChannel << ".");
        maxLength = maxChannels;
        bestOffset = -1;
    }

    if (maxLength > 1) {
        int crtChannel = (startChannel + bestOffset + 1 - maxLength / 2 + maxChannels) % maxChannels;
        LOG_DEBUG("allocateChannel: allocating channel " << crtChannel << " for starIndex " << starIndex);
        if (channelTimeslotAllocations[starIndex][crtChannel].isFreePosition(allocationBitmap)) {
            channelTimeslotAllocations[starIndex][crtChannel].allocate(allocationBitmap);
            return crtChannel + 1;
        }
    }

    LOG_ERROR("allocateChannel - no channel available for starIndex= " << (int) starIndex);

    return 0;
}

void SubnetTdma::deallocateChannel(Uint8 starIndex, Uint8 channel, AllocationBitmap& allocationBitmap) {
    int tix = allocationBitmap.getTimeslotIndex();
    LOG_DEBUG("DeallocateChannel: starIndex=" << (int) starIndex << ", tIx=" << tix << ", channel=" << (int) channel);

    if (starIndex >= 25 || channel > 15) {
        LOG_ERROR("deallocateChannel - Invalid channel deallocation (ch=" << (int) channel << ", sIx="
                    << (int) starIndex << ").");
        return;
    }

    channelTimeslotAllocations[starIndex][channel - 1].deallocate(allocationBitmap);

    LOG_DEBUG("DeallocateChannel: end");

}

bool SubnetTdma::allocateJoinLinks(EngineOperations& engineOperations, Uint32 outboundAllocationHandler,
            Address32 address, PublishPeriod::PublishPeriodEnum publishPeriod) {

    IEngineOperationPointer operationSuperframe(new SuperframeAddedOperation(address, 0,
                SuperframeLength::SLENGTH_250_MS, 1));

    operationSuperframe->setDependency(WaveDependency::FIRST);
    engineOperations.addOperation(operationSuperframe);

    int startIndex = rand() % MAX_STAR_INDEX;

    bool first = broadcastAllocation(engineOperations, outboundAllocationHandler, address, publishPeriod, true, true,
                LinkTypes::JOIN, startIndex);

    bool second = broadcastAllocation(engineOperations, outboundAllocationHandler, address, publishPeriod, false,
                false, LinkTypes::JOIN, startIndex + 2);

    return first & second;
}

bool SubnetTdma::resolveOperation(SuperframeAddedOperation& operation) {
    NodeTdmaMap::iterator it = nodes.find(operation.getOwner());

    if (it != nodes.end()) {
        return it->second.resolveOperation(operation);
    } else {
        std::ostringstream stream;
        stream << "resolveOperation - no node found for operation: ";
        operation.toString(stream);

        LOG_ERROR(stream.str());
        //TODO: ivp - is this ok?
        return true;
    }
}

bool SubnetTdma::resolveOperation(LinkAddedOperation& operation) {
    NodeTdmaMap::iterator it = nodes.find(operation.getOwner());

    if (it != nodes.end()) {
        bool deallocate = false;
        bool retVal = it->second.resolveOperation(operation, deallocate);

        if (deallocate)
        {
            LOG_DEBUG("LinkAdd - deallocate resolve operation: " << (int) operation.getOperationId());
            AllocationBitmap allocationBitmap;
            allocationBitmap.initAllocationBitmap(operation.timeslotIndex, operation.publishPeriod);
            deallocateChannel(operation.starIndex, operation.channel, allocationBitmap);
        }

        if (LOG_DEBUG_ENABLED())
        {
            checkAllLinks();
        }

        return retVal;
    } else {
        std::ostringstream stream;
        stream << "resolveOperation - no node found for operation: ";
        operation.toString(stream);

        LOG_ERROR(stream.str());
        //TODO: ivp - is this ok?
        return true;
    }
}

bool SubnetTdma::resolveOperation(LinkRemovedOperation& operation) {
    NodeTdmaMap::iterator it = nodes.find(operation.getOwner());
    if (it != nodes.end()) {
        bool result;
        bool deallocate = false;

        if (operation.isTransmission() && operation.isNormalLink()) {
            NodeTdmaMap::iterator itPeer = nodes.find(operation.getPeerAddress());
            if (itPeer != nodes.end() && itPeer->second.isUseSingleReception()) {
                //generate peer reception operation
                LinkRemovedOperation operationPeer(operation.peerAddress, operation.allocationHandler,
                            operation.superframeId, operation.publishPeriod, operation.starIndex,
                            operation.timeslotIndex, operation.channel, operation.getOwner(), operation.reception,
                            operation.transmission, operation.shared, operation.linkType);

                operationPeer.responseCode = operation.responseCode;
                operationPeer.setErrorCode(operation.getErrorCode());
                operationPeer.setState(operation.getState());
                LOG_DEBUG("isUseSingleReception: deallocate");
                deallocate = itPeer->second.resolveOperation(operationPeer);
                //what about channel map?
            }
        }

        result = it->second.resolveOperation(operation);

        //release the channel when BBR advertise links are removed
        if (operation.getPeerAddress() == BROADCAST_ADDRESS && operation.isNormalLink()) {
            deallocate = true;
        }

        if (result && operation.allocationHandler != 0xFFFFFFFF) {
            deallocate = true;
        }

        if (deallocate) {
            AllocationBitmap allocationBitmap;
            allocationBitmap.initAllocationBitmap(operation.timeslotIndex, operation.publishPeriod);

            LOG_DEBUG("releaseRemoved - deallocate resolve operation: " << (int) operation.getOperationId());
            deallocateChannel(operation.starIndex, operation.channel, allocationBitmap);
        }

        if (LOG_DEBUG_ENABLED())
        {
            checkAllLinks();
        }

        return true;
    } else {
        std::ostringstream stream;
        stream << "resolveOperation - no node found for operation: ";
        operation.toString(stream);

        stream << "; deallocate channel";

        AllocationBitmap allocationBitmap;
        allocationBitmap.initAllocationBitmap(operation.timeslotIndex, operation.publishPeriod);

        LOG_DEBUG("releaseRemoved - deallocate resolve operation: " << (int) operation.getOperationId());
        deallocateChannel(operation.starIndex, operation.channel, allocationBitmap);

        LOG_WARN(stream.str());

        if (LOG_DEBUG_ENABLED())
        {
            checkAllLinks();
        }

        //TODO: ivp - is this ok?
        return true;
    }
}

bool SubnetTdma::isRemovedStatus(Address32 address) {
    NodeTdmaMap::iterator it = nodes.find(address);
    if (it != nodes.end()) {
        return it->second.getStatus() == Status::REMOVED || it->second.getStatus() == Status::DELETED;
    } else {
        LOG_ERROR("isRemovedStatus - node not found: " << ToStr(address));
        return false;
    }

    return true;
}

void SubnetTdma::setRemovedStatus(Address32 address) {
    NodeTdmaMap::iterator it = nodes.find(address);
    if (it != nodes.end()) {
        it->second.setStatus(Status::REMOVED);
    } else {
        LOG_ERROR("setRemovedStatus - node not found: " << ToStr(address));
    }
}

void SubnetTdma::releaseRemoved(EngineOperations& engineOperations) {
    for (NodeTdmaMap::iterator it = nodes.begin(); it != nodes.end(); ++it) {
        if (it->second.getStatus() == Status::REMOVED)
        {
            continue;
        }

        it->second.releaseRemoved(engineOperations);
    }

    for (NodeTdmaMap::iterator it = nodes.begin(); it != nodes.end();) {
        if (it->second.getStatus() == Status::REMOVED) {
            nodes.erase(it++);
        } else {
            ++it;
        }
    }
}


void SubnetTdma::compactLinks(NE::Model::Operations::EngineOperations& engineOperations) {

}

void SubnetTdma::toString(std::ostringstream& stream) {

    // TODO just for test START
    for (int i = 0; i < maxChannels; i++) {
        stream << "Channel " << i << std::endl;
        for (int j = 0; j < MAX_STAR_INDEX; j++) {
            channelTimeslotAllocations[j][i].toString(stream);
        }
    }
    // TODO just for test END

    stream << std::setfill(' ') << std::endl << std::endl;

    stream << std::setw(8) << "Address";
    stream << std::setw(9) << "Peer";
    stream << std::hex << std::setw(10) << "Handler";
    stream << std::setw(5) << "SId";
    stream << std::setw(5) << "Pp";
    stream << std::setw(5) << "Six";
    stream << std::setw(5) << "Tix";
    stream << std::setw(5) << "Ch";
    stream << std::setw(5) << "Lt";
    stream << std::setw(5) << "Tx";
    stream << std::setw(5) << "Rx";
    stream << std::setw(5) << "Sh";
    stream << std::setw(10) << "Status";
    stream << std::endl;

    for (NodeTdmaMap::iterator it = nodes.begin(); it != nodes.end(); ++it) {
        it->second.toString(stream);
    }
}


void SubnetTdma::checkAllLinks()
{
    return;

    ChannelTimeslotAllocation bkpChannels[MAX_STAR_INDEX][16];

    LOG_DEBUG("checkAllLinks()");

    for (NodeTdmaMap::iterator it = nodes.begin(); it != nodes.end(); ++it)
    {
        for (int i = 0; i < MAX_STAR_INDEX; i++)
        {
            for (TimeslotAllocations::iterator itTimeslot = it->second.timeslotAllocationsArray[i].begin();
                        itTimeslot != it->second.timeslotAllocationsArray[i].end(); ++itTimeslot)
            {
                AllocationBitmap bmp;
                bmp.initAllocationBitmap(itTimeslot->timeslotIndex, itTimeslot->publishPeriod);
                if (!bmp.isFreePosition(bkpChannels[i][itTimeslot->channel - 1].getAllocationBitmap()))
                {
//                    LOG_WARN("checkAllLinks: Overlapping links detected in TimeslotAllocations for node=" << std::hex << it->first);
                }
                bkpChannels[i][itTimeslot->channel - 1].allocate(bmp);
            }
        }
    }

    bool haveDifference = false;
    for (int channel = 0; channel < 16; channel++)
    {
        for (int six = 0; six < MAX_STAR_INDEX; six++)
        {
            if (!(bkpChannels[six][channel].getAllocationBitmap() == channelTimeslotAllocations[six][channel].getAllocationBitmap()))
            {
                haveDifference = true;
            }
        }
    }

    if (haveDifference)
    {
        LOG_WARN("checkAllLinks: Difference detected between channelTimeslotAllocations and links.");
        std::ostringstream str;
        for (int channel = 0; channel < 16; channel++)
        {
            str << "Channel " << channel << std::endl;
            for (int six = 0; six < MAX_STAR_INDEX; six++)
            {
                bkpChannels[six][channel].getAllocationBitmap().toString(str, false) ;
                str << "    -    " ;
                channelTimeslotAllocations[six][channel].getAllocationBitmap().toString(str, false);

                if (!(bkpChannels[six][channel].getAllocationBitmap() == channelTimeslotAllocations[six][channel].getAllocationBitmap()))
                {
                    str << " *";
                }
                str << std::endl;
            }
        }

        LOG_DEBUG(str.str());
    }
}

