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
 * TimeslotAllocation.cpp
 *
 *  Created on: Mar 17, 2009
 *      Author: ioanpocol
 */

#include "TimeslotAllocation.h"

#include <iomanip>
#include "Model/Tdma/AllocationBitmap.h"

using namespace NE::Model::Tdma;
using namespace NE::Model::Operations;

TimeslotAllocation::TimeslotAllocation(Uint32 allocationHandler_, Address32 peerAddress_, Uint8 channel_,
            Uint8 timeslotIndex_, PublishPeriod::PublishPeriodEnum publishPeriod_, bool transmission_, bool reception_,
            bool shared_, LinkTypes::LinkTypesEnum linkType_) :

    allocationHandler(allocationHandler_), peerAddress(peerAddress_), channel(channel_), timeslotIndex(timeslotIndex_),
                publishPeriod(publishPeriod_), transmission(transmission_), reception(reception_), shared(shared_),
                linkType(linkType_) {

    status = Status::NEW;
}

TimeslotAllocation::~TimeslotAllocation() {
}

Uint32 TimeslotAllocation::getAllocationHandler() {
    return allocationHandler;
}

Address32 TimeslotAllocation::getPeerAddress() {
    return peerAddress;
}

LinkTypes::LinkTypesEnum TimeslotAllocation::getLinkType() {
    return linkType;
}

Status::StatusEnum TimeslotAllocation::getStatus() {
    return status;
}

void TimeslotAllocation::setStatus(Status::StatusEnum status_) {
    status = status_;
}

Uint8 TimeslotAllocation::getChannel() {
    return channel;
}

Uint8 TimeslotAllocation::getTimeslotIndex() {
    return timeslotIndex;
}

bool TimeslotAllocation::isTransmission() {
    return transmission;
}

bool TimeslotAllocation::isReception() {
    return reception;
}

PublishPeriod::PublishPeriodEnum TimeslotAllocation::getPublishPeriod() {
    return publishPeriod;
}

void TimeslotAllocation::reallocate() {
    if (status == Status::REMOVED) {
        status = Status::RECOVERED;
    } else {
        status = Status::NEW;
    }
}

void TimeslotAllocation::deallocate() {

    if (status != Status::DELETED) {
        status = Status::REMOVED;
    }
}

bool TimeslotAllocation::matchAllocation(Uint32 allocationHandler_, Address32 peerAddress_) {
    return allocationHandler == allocationHandler_ && peerAddress == peerAddress_;
}

const Uint32 countMask =   0x0000FF00;
const Uint32 reverseMask = 0xFFFF00FF;

bool TimeslotAllocation::matchAllocationWithOlderHandler(Uint32 allocationHandler_, Address32 peerAddress_) {
    if (peerAddress != peerAddress_)
        return false;

    return ((allocationHandler_ & countMask) != (allocationHandler & countMask)) &&      // the count inside the handler is different
                ((allocationHandler_ & reverseMask) == (allocationHandler & reverseMask));  // the rest is equal
}

bool TimeslotAllocation::matchAllocation(Uint32 allocationHandler_, Uint8 timeslotIndex_,
            PublishPeriod::PublishPeriodEnum publishPeriod_) {

    return allocationHandler == allocationHandler_ && timeslotIndex == timeslotIndex_ && publishPeriod
                == publishPeriod_;

}

bool TimeslotAllocation::matchAllocation(Uint32 allocationHandler_, Address32 peerAddress_, Uint8 channel_,
            Uint8 timeslotIndex_, PublishPeriod::PublishPeriodEnum publishPeriod_) {

    return allocationHandler == allocationHandler_ && peerAddress == peerAddress_ && channel == channel_
                && timeslotIndex == timeslotIndex_ && publishPeriod == publishPeriod_;

}

bool TimeslotAllocation::matchAllocation(LinkTypes::LinkTypesEnum linkType_) {
    return linkType == linkType_;
}

void TimeslotAllocation::commitChanges() {

    if (status == Status::NEW) {
        status = Status::PENDING;
    } else if (status == Status::REMOVED) {
        status = Status::DELETED;
    } else if (status == Status::RECOVERED) {
        status = Status::ACTIVE;
    }
}

void TimeslotAllocation::generateOperations(EngineOperations& engineOperations, Address32 address, Uint8 starIndex,
            bool useSingleReception) {

    if (useSingleReception && reception && linkType == LinkTypes::NORMAL) {
        if (status == Status::NEW) {
            status = Status::ACTIVE;
        }

        return;
    }

    Uint8 superframeId = getSuperframeId(publishPeriod);

    if (status == Status::NEW) {
        IEngineOperationPointer operation(new LinkAddedOperation(address, allocationHandler, superframeId,
                    publishPeriod, starIndex, timeslotIndex, channel, peerAddress, transmission, reception, shared,
                    linkType));

        // caches the operation
        LOG_DEBUG("generateOperations() generated LinkAddedOperation");
        this->operationLinkAdded = operation;

        if (!transmission) {
            operation->setDependency(WaveDependency::SECOND);
        } else {
            operation->setDependency(WaveDependency::THIRD);
        }

        engineOperations.addOperation(operation);

    } else if (status == Status::REMOVED) {

        IEngineOperationPointer operation(new LinkRemovedOperation(address, allocationHandler, superframeId,
                    publishPeriod, starIndex, timeslotIndex, channel, peerAddress, transmission, reception, shared,
                    linkType));

        if (this->operationLinkAdded != NULL) {
            //operation->setOperationDependency(this->operationLinkAdded);
            LOG_DEBUG("It should be set the AddLinkOperation dependency for the delete operation.");
        }

        if (!transmission) {
            operation->setDependency(WaveDependency::ELEVENTH);
        } else {
            operation->setDependency(WaveDependency::TENTH);
        }

        engineOperations.addOperation(operation);

    }
}

void TimeslotAllocation::confirmAddLinkOperation() {
    LOG_DEBUG("confirmAddLinkOperation()");
    this->operationLinkAdded.reset();
}

void TimeslotAllocation::toString(std::ostringstream& stream, Address32 address, int starIndex) {
    stream << ToStr(address, 8);
    stream << " " << ToStr(peerAddress, 8);
    stream << std::hex << std::setw(10) << (int) allocationHandler << std::dec;
    stream << std::setw(5) << (int) getSuperframeId(publishPeriod);
    stream << std::setw(5) << (int) publishPeriod;
    stream << std::setw(5) << (int) starIndex;
    stream << std::setw(5) << (int) timeslotIndex;
    stream << std::setw(5) << (int) channel;
    stream << std::setw(5) << (int) linkType;
    stream << std::setw(5) << (int) transmission;
    stream << std::setw(5) << (int) reception;
    stream << std::setw(5) << (int) shared;
    stream << std::setw(10) << (int) status;
    stream << std::endl;
}
