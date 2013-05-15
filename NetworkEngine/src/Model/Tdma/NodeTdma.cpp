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
 * NodeTdma.cpp
 *
 *  Created on: Mar 17, 2009
 *      Author: ioanpocol
 */

#include "NodeTdma.h"
#include "Model/NetworkEngine.h"
#include "Model/Tdma/SubnetTdma.h"

using namespace NE::Model;
using namespace NE::Model::Tdma;
using namespace NE::Model::Operations;

NodeTdma::NodeTdma(SubnetTdma& subnetTdma_, Address32 address_, NE::Model::Topology::NodeType::NodeTypeEnum nodeType_) :
    subnetTdma(subnetTdma_), address(address_), nodeType(nodeType_), superframes(0) {
    changed = 0;
    status = Status::NEW;

    if (NE::Model::NetworkEngine::instance().getSettingsLogic().enableApBestFitAllocation == true) {
        useBestFitAllocation = (nodeType == NodeType::BACKBONE);
    } else {
        useBestFitAllocation = false;
    }

    if (NE::Model::NetworkEngine::instance().getSettingsLogic().useSingleReception == true) {
        useSingleReception = (nodeType == NodeType::BACKBONE);
    } else {
        useSingleReception = false;
    }
}

NodeTdma::~NodeTdma() {
    // deallocate all links
    NE::Model::Tdma::SubnetTdma& subnetTdma = NE::Model::NetworkEngine::instance().getSubnetTdma();
    for (int i = 0; i < MAX_STAR_INDEX; i++) {
        TimeslotAllocations& timeslotAllocations = timeslotAllocationsArray[i];
        for (TimeslotAllocations::iterator it = timeslotAllocations.begin(); it != timeslotAllocations.end(); ++it) {
            try
            {
                NE::Model::Tdma::NodeTdma& peer = subnetTdma.getNodeTdma(it->getPeerAddress());
                if (peer.isUseSingleReception() && it->transmission && it->linkType == LinkTypes::NORMAL)
                {
                    LinkRemovedOperation operationPeer(peer.getAddress(), it->allocationHandler,
                                getSuperframeId(it->publishPeriod), it->publishPeriod, i,
                                it->timeslotIndex, it->channel, address, it->reception,
                                it->transmission, it->shared, it->linkType);

                    operationPeer.setResponseCode(0);
                    operationPeer.setErrorCode(ErrorCode::SUCCESS);
                    operationPeer.setState(OperationState::CONFIRMED);

                    peer.resolveOperation(operationPeer);
                }
            }
            catch (...) {}


            AllocationBitmap allocationBitmap;
            allocationBitmap.initAllocationBitmap(it->getTimeslotIndex(), it->getPublishPeriod());
            // only deallocate channel for discovery when Backbone
            if (it->linkType != LinkTypes::DISCOVERY || getNodeType() == Topology::NodeType::BACKBONE) {
                LOG_DEBUG("~NodeTdma - deallocate timeslot node=" << ToStr(address) << ", peer=" << ToStr(
                            it->getPeerAddress()) << ", handler=" << ToStr(it->getAllocationHandler()));
                subnetTdma.deallocateChannel(i, it->getChannel(), allocationBitmap);
            }
        }
    }
}

Address32 NodeTdma::getAddress() {
    return this->address;
}

Status::StatusEnum& NodeTdma::getStatus() {
    return status;
}

void NodeTdma::setStatus(Status::StatusEnum status_) {
    status = status_;
}

NE::Model::Topology::NodeType::NodeTypeEnum NodeTdma::getNodeType() {
    return nodeType;
}

Uint8 NodeTdma::getLatency() {
    return latency;
}

void NodeTdma::setLatency(Uint8 latency_) {
    if (latency < latency_) {
        latency = latency_;
    }
}

void NodeTdma::resetLatency() {
    latency = 0;
}

bool NodeTdma::isChanged() {
    return changed != 0;
}

bool NodeTdma::isFreePosition(Uint8 starIndex, const AllocationBitmap& allocationBitmap) {
    return allocationBitmapes[starIndex].isFreePosition(allocationBitmap);
}

void NodeTdma::useSuperframe(NE::Model::Operations::EngineOperations& engineOperations,
            PublishPeriod::PublishPeriodEnum publishPeriod) {
    Uint8 superframeId = getSuperframeId(publishPeriod);
    Uint32 superframeLength = SuperframeLength::SLENGTH_250_MS;

    Uint16 mask = (0x8000 >> (superframeId - SuperframeId::SID_250_MS));

    if ((superframes & mask) == 0) {
        superframes |= mask;

        for (int i = SuperframeId::SID_250_MS; i <= SuperframeId::SID_ALL; i++) {
            if (i == superframeId) {
                IEngineOperationPointer operation(new SuperframeAddedOperation(address, i, superframeLength, 1));

                operation->setDependency(WaveDependency::FIRST);
                engineOperations.addOperation(operation);

                break;
            }

            superframeLength *= 2;
        }
    }

}

void NodeTdma::allocate(EngineOperations& engineOperations, Uint32 allocationHandler, Address32 peerAddress,
            bool isRetry, Uint8 channel, Uint8 starIndex, AllocationBitmap& allocationBitmap, bool transmission,
            bool reception, bool shared, LinkTypes::LinkTypesEnum linkType) {

    Uint8 timeslotIndex = allocationBitmap.getTimeslotIndex();
    PublishPeriod::PublishPeriodEnum publishPeriod = allocationBitmap.getPublishPeriod();

    Uint64 key = (((Uint64) allocationHandler) << 32) + peerAddress;

    changed |= (0x8000 >> starIndex);

    if (isRetry) {
        retryHandlerIndex[key] = starIndex;
    } else {
        handlerIndex[key] = starIndex;
    }

    LOG_DEBUG("allocate - handler= " << ToStr(allocationHandler) << ", address=" << ToStr(address) << ", peerAddress="
                << ToStr(peerAddress) << ", key=" << std::hex << key << std::dec << ", isRetry=" << (int) isRetry
                << ", starIndex=" << (int) starIndex << ", timeslotIndex=" << (int) timeslotIndex << ", publishPeriod="
                << (int) publishPeriod);

    allocationBitmapes[starIndex] + allocationBitmap;

    useSuperframe(engineOperations, publishPeriod);

    TimeslotAllocations& timeslotAllocations = timeslotAllocationsArray[starIndex];

    TimeslotAllocation timeslotAllocation(allocationHandler, peerAddress, channel, timeslotIndex, publishPeriod,
                transmission, reception, shared, linkType);

    timeslotAllocation.generateOperations(engineOperations, address, starIndex, useSingleReception);
    timeslotAllocation.commitChanges();

    timeslotAllocations.push_back(timeslotAllocation);

}

bool NodeTdma::deallocate(NE::Model::Operations::EngineOperations& engineOperations, Uint32 newAllocationHandler, Uint32 allocationHandler,
            Address32 peerAddress, bool isRetry, Uint8& starIndex, bool deallocateOlder, LinkTypes::LinkTypesEnum linkType) {

    bool returnValue = true;

    LOG_DEBUG("deallocate - handler= " << ToStr(allocationHandler) << ", address=" << ToStr(address)
                << ", peerAddress=" << ToStr(peerAddress) << ", is Retry=" << (int) isRetry);

    Uint64 key = (((Uint64) allocationHandler) << 32) + peerAddress;

    if (isRetry) {
        if (retryHandlerIndex.find(key) == retryHandlerIndex.end()) {
            LOG_ERROR("deallocate - no value for retry key " << std::hex << key << std::dec);
            returnValue = false;
        }

        starIndex = retryHandlerIndex[key];
    } else {
        if (handlerIndex.find(key) == handlerIndex.end()) {
            LOG_ERROR("deallocate - no value for first key " << std::hex << key << std::dec);
            returnValue = false;
        }

        starIndex = handlerIndex[key];
    }

    LOG_DEBUG("deallocate - handler= " << ToStr(allocationHandler) << ", peerAddress=" << ToStr(peerAddress)
                << ", key=" << std::hex << key << std::dec << ", isRetry=" << (int) isRetry << ", starIndex="
                << (int) starIndex);

    changed |= (0x8000 >> starIndex);

    for (int six = 0; six < MAX_STAR_INDEX; six++)
    {
        TimeslotAllocations& timeslotAllocations = timeslotAllocationsArray[six];
        for (TimeslotAllocations::iterator it = timeslotAllocations.begin(); it != timeslotAllocations.end(); ++it) {

            // LOG_DEBUG("Attempting to match allocationHandler=" << std::hex << it->allocationHandler << ", peerAddress=" << it->peerAddress);

            if (it->matchAllocation(allocationHandler, peerAddress)) {

                // LOG_DEBUG("Matched allocationhandler=" << std::hex << allocationHandler);

                if (linkType == LinkTypes::ANY || it->linkType == linkType) {

                    it->deallocate();
                    it->generateOperations(engineOperations, address, six, useSingleReception);
                    it->commitChanges();

                    if (isRetry) {
                        retryHandlerIndex.erase(key);
                    } else {
                        handlerIndex.erase(key);
                    }
                }
            }

            if (deallocateOlder && it->matchAllocationWithOlderHandler(newAllocationHandler, peerAddress))
            {
                // cleanup in case older handlers exist that have not been deallocated
                // LOG_DEBUG("Matched olderAllocationHandler than =" << std::hex << allocationHandler);

                if (linkType == LinkTypes::ANY || it->linkType == linkType) {
                    it->deallocate();
                    it->generateOperations(engineOperations, address, six, useSingleReception);
                    it->commitChanges();
                }
            }
        }
    }

    return returnValue;
}

bool NodeTdma::isUseSingleReception() {
    return useSingleReception;
}

bool NodeTdma::isUseBestFitAllocation() {
    return useBestFitAllocation;
}

bool NodeTdma::operator<(NodeTdma &NodeTdma) const {
    return this->address < NodeTdma.address;
}

bool NodeTdma::resolveOperation(SuperframeAddedOperation& operation) {
    if (operation.getErrorCode() != ErrorCode::SUCCESS) {
        Uint16 mask = (0x8000 >> (operation.superframeId - SuperframeId::SID_250_MS));

        superframes &= ~mask;
    }

    return true;
}

bool NodeTdma::resolveOperation(LinkAddedOperation& operation, bool& deallocate) {
    deallocate = false;
    TimeslotAllocations& timeslotAllocations = timeslotAllocationsArray[operation.starIndex];

    for (TimeslotAllocations::iterator it = timeslotAllocations.begin(); it != timeslotAllocations.end(); ++it) {
        if (it->matchAllocation(operation.allocationHandler, operation.peerAddress)) {

            if (operation.getErrorCode() == ErrorCode::SUCCESS) {
                if (it->getStatus() == Status::PENDING) {
                    it->setStatus(Status::ACTIVE);
                    it->confirmAddLinkOperation();
                    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();
                    Node& node = subnetTopology.getNode(address);
                    node.setLinkedNeighborsCount(getLinkedNeighborsNo());
                    return true;
                } else {
                    //TODO: nothing?
                }
            } else if (operation.getErrorCode() == ErrorCode::TIMEOUT) {
                return false;
            } else if (operation.getErrorCode() == ErrorCode::ERROR) {
                //TODO: ivp - ivp - depending by the error set ACTIVE or delete it
                if (operation.getResponseCode() == C967_E66) { //TODO change this to not use 2 streams
                    std::ostringstream str;
                    str << "Link already exists! ";
                    operation.toString(str);
                    LOG_WARN(str.str());
                    return true;
                } else if (operation.getState() == OperationState::SENT_IGNORED) {
                    AllocationBitmap allocationBitmap;
                    allocationBitmap.initAllocationBitmap(it->getTimeslotIndex(), it->getPublishPeriod());
                    allocationBitmapes[operation.starIndex] - allocationBitmap;
                    deallocate = true;

                    timeslotAllocations.erase(it);
                    return true;
                }

                return false;
            }

            //TODO: ivp - is this ok?
            return true;
        }
    }

    return true;
}

bool NodeTdma::resolveOperation(LinkRemovedOperation& operation) {
    AllocationBitmap allocationBitmap;
    TimeslotAllocations& timeslotAllocations = timeslotAllocationsArray[operation.starIndex];

    for (TimeslotAllocations::iterator it = timeslotAllocations.begin(); it != timeslotAllocations.end(); ++it) {
        if (it->matchAllocation(operation.allocationHandler, operation.timeslotIndex, operation.publishPeriod)) {

            if (operation.getErrorCode() == ErrorCode::SUCCESS) {
                if (it->getStatus() != Status::DELETED) {
                    LOG_WARN("On node " << std::hex << address << " removing a link that is not deleted.");
                }

                    if (it->getTimeslotIndex() != operation.timeslotIndex || it->getPublishPeriod()
                                != operation.publishPeriod) {
                        LOG_ERROR(" resolveOperation - crtTx=" << (int) it->getTimeslotIndex() << ", crtPp="
                                    << (int) it->getPublishPeriod());
                    }

                    allocationBitmap.initAllocationBitmap(it->getTimeslotIndex(), it->getPublishPeriod());
                    allocationBitmapes[operation.starIndex] - allocationBitmap;

                    timeslotAllocations.erase(it);

                    return true;
            } else if (operation.getErrorCode() == ErrorCode::TIMEOUT) {
                return false;
            } else if (operation.getErrorCode() == ErrorCode::ERROR) {
                //TODO: ivp - depending by the error delete it or not
                if (operation.getState() == OperationState::SENT_IGNORED) {
                    it->setStatus(Status::ACTIVE);
                    return false;   // do not deallocate channel
                }
            }
            return true;
        }
    }

    return true;
}

void NodeTdma::generateRemoveJoinLinks(EngineOperations& engineOperations, Address32 childAddress) {
    for (int starIndex = 0; starIndex < 25; starIndex++) {
        TimeslotAllocations& timeslotAllocations = timeslotAllocationsArray[starIndex];

        for (TimeslotAllocations::iterator it = timeslotAllocations.begin(); it != timeslotAllocations.end(); ++it) {
            if (it->matchAllocation(LinkTypes::JOIN)) {
                LinkRemovedOperation *removeLink = new LinkRemovedOperation(childAddress, 0xFFFFFFFF,           // special handle
                            getSuperframeId(it->getPublishPeriod()), it->getPublishPeriod(), starIndex,
                            it->getTimeslotIndex(), it->getChannel(), 0xFFFF, it->isTransmission(), it->isReception(),
                            true, LinkTypes::JOIN);

                removeLink->setDependency(WaveDependency::EIGTH); //TODO check

                IEngineOperationPointer opLink(removeLink);
                engineOperations.addOperation(opLink);
            }
        }
    }
}

void NodeTdma::releaseRemoved(NE::Model::Operations::EngineOperations& engineOperations) {
    SubnetTdma & subnetTdma = NetworkEngine::instance().getSubnetTdma();
    SubnetServices& subnetServices = NetworkEngine::instance().getSubnetServices();
    AllocationBitmap allocationBitmap;

    for (int i = 0; i < MAX_STAR_INDEX; i++) {
        TimeslotAllocations& timeslotAllocations = timeslotAllocationsArray[i];

        for (TimeslotAllocations::iterator it = timeslotAllocations.begin(); it != timeslotAllocations.end();) {
            Uint32 actualServiceId = it->getAllocationHandler() & 0xFFFF00FF; // remove the session handler ID from the serviceID

            if ((it->getStatus() == Status::DELETED)) {
                it++;
                continue;
            } else if (subnetServices.existsService(actualServiceId) || actualServiceId == 0xFFFF00FF) {
                if (it->getPeerAddress() == BROADCAST_ADDRESS) {
                    it++;
                    continue;
                }

                if (!subnetTdma.isRemovedStatus(it->getPeerAddress())) {
                    it++;
                    continue;
                }
            }

            LOG_DEBUG("releaseRemoved - the service doesn't exist: " << ToStr(actualServiceId, 8) << " for node: "
                        << ToStr(address));

            //HACK[andy] - send Link Removed for deleted links too if the peer does not exist or is marked for delete.

            it->setStatus(Status::REMOVED);

            if (getStatus() != Status::REMOVED) {
                it->generateOperations(engineOperations, address, i, useSingleReception);

                //delete unique reception links if the peer is deleted
                if ( it->getLinkType() == LinkTypes::NORMAL  && it->isReception() && useSingleReception ) {
                    if (subnetTdma.isRemovedStatus(it->getPeerAddress())) {
                        allocationBitmap.initAllocationBitmap(it->getTimeslotIndex(), it->getPublishPeriod());
                        allocationBitmapes[i] - allocationBitmap;
                        timeslotAllocations.erase(it++);
                        continue;
                    }
                 }
            } else {
                bool deallocate = false;

                if (it->getLinkType() == LinkTypes::NORMAL) {
                    if (it->isReception() && !useSingleReception) {
                        deallocate = true;
                    } else if (it->isTransmission()) {
                        if (subnetTdma.existsNode(it->getPeerAddress())) {
                            NodeTdma& peerNode = subnetTdma.getNodeTdma(it->getPeerAddress());
                            if (peerNode.isUseSingleReception()) {
                                deallocate = true;
                            }
                        } else if (it->getPeerAddress() == BROADCAST_ADDRESS) {
                            deallocate = true;
                        } else {
                            LOG_ERROR("releaseRemoved - peer node doesn't exist: " << ToStr(it->getPeerAddress()));
                        }
                    }
                } else {
                    deallocate = true;
                }

                if (deallocate) {
                    //the delete operation is not generated so deallocate the channel for RX
                    allocationBitmap.initAllocationBitmap(it->getTimeslotIndex(), it->getPublishPeriod());
                    //allocationBitmapes[ i ] - allocationBitmap;
                    LOG_DEBUG("releaseRemoved - deallocate timeslot node=" << ToStr(address) << ", peer=" << ToStr(
                                it->getPeerAddress()) << ", handler=" << ToStr(it->getAllocationHandler()));

                    if (it->linkType != LinkTypes::DISCOVERY || nodeType == NodeType::BACKBONE) {
                        subnetTdma.deallocateChannel(i, it->getChannel(), allocationBitmap);
                    }
                }
            }

            it->commitChanges();
            it++;

        }
    }
}

uint16_t NodeTdma::getLinkedNeighborsNo() {
    uint16_t neighborsCount = 0;

    std::set<Address32> checked;

    for (int i = 0; i < MAX_STAR_INDEX; ++i) {
        TimeslotAllocations::iterator itTA = timeslotAllocationsArray[i].begin();
        for (; itTA != timeslotAllocationsArray[i].end(); ++itTA) {

            if (itTA->getPeerAddress() == BROADCAST_ADDRESS) {
                //ignore
                continue;
            }
            checked.insert(itTA->getPeerAddress());
        }
    }

    return checked.size();
}


uint16_t NodeTdma::getTimeslotAllocationsArraySize() {
    uint16_t linksNo = 0;

    if (this->useSingleReception) {
        // if it is single reception enabled then consider a single reception link and ignores all the reception links ...
        ++linksNo;
    }

    for (int i = 0; i < MAX_STAR_INDEX; ++i) {
        TimeslotAllocations::iterator itTA = timeslotAllocationsArray[i].begin();
        for (; itTA != timeslotAllocationsArray[i].end(); ++itTA) {

            if (this->useSingleReception) {
                if (itTA->reception && !itTA->shared && !itTA->transmission) {
                    if (itTA->linkType != LinkTypes::JOIN) {
                        // ... these links are ignored
                        continue;
                    }
                }
            }

            ++linksNo;
        }
    }

    return linksNo;
}

void NodeTdma::toString(std::ostringstream& stream) {
    for (int i = 0; i < MAX_STAR_INDEX; i++) {
        TimeslotAllocations& timeslotAllocations = timeslotAllocationsArray[i];
        for (TimeslotAllocations::iterator it = timeslotAllocations.begin(); it != timeslotAllocations.end(); ++it) {
            it->toString(stream, address, i);
        }
    }
}

