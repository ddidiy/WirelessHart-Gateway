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
 * TimeslotAllocation.h
 *
 *  Created on: Mar 17, 2009
 *      Author: ioanpocol
 */

#ifndef TIMESLOTALLOCATION_H_
#define TIMESLOTALLOCATION_H_

#include <list>

#include "Common/logging.h"
#include "Model/Tdma/TdmaTypes.h"
#include "Common/NETypes.h"
#include "Model/Operations/EngineOperations.h"

#include "Model/Operations/Tdma/LinkAddedOperation.h"
#include "Model/Operations/Tdma/LinkRemovedOperation.h"
#include "Model/Operations/Tdma/SuperframeAddedOperation.h"

namespace NE {

namespace Model {

namespace Tdma {

class TimeslotAllocation;
typedef std::list<TimeslotAllocation> TimeslotAllocations;

class TimeslotAllocation {

        LOG_DEF("I.M.G.TimeslotAllocation");

    public:

        Uint32 allocationHandler;

        Address32 peerAddress;

        Uint8 channel;

        Uint8 timeslotIndex;

        PublishPeriod::PublishPeriodEnum publishPeriod;

        bool transmission;

        bool reception;

        bool shared;

        LinkTypes::LinkTypesEnum linkType;

        Status::StatusEnum status;

        /**
         * When LinkAddedOperation is generated will be cached until it is confirmed.
         * On confirmed we should delete reset the operation.
         * If a delete operation is generated before the link added is confirmed, then it will
         * be put in the list of operations dependencies for the DeleteLinkOperation.
         */
        NE::Model::Operations::IEngineOperationPointer operationLinkAdded;

    public:

        /**
         *
         */
        TimeslotAllocation(Uint32 allocationHandler, Address32 peerAddress, Uint8 channel, Uint8 timeslotIndex,
                    PublishPeriod::PublishPeriodEnum publishPeriod, bool transmission, bool reception, bool shared,
                    LinkTypes::LinkTypesEnum linkType);

        /**
         *
         */
        virtual ~TimeslotAllocation();

        /**
         *
         */
        ServiceId getAllocationHandler();

        /**
         *
         */
        Status::StatusEnum getStatus();

        /**
         *
         */
        LinkTypes::LinkTypesEnum getLinkType();

        /**
         *
         */
        Address32 getPeerAddress();

        /**
         *
         */
        void setStatus(Status::StatusEnum status);

        /**
         *
         */
        Uint8 getChannel();

        /**
         *
         */
        Uint8 getTimeslotIndex();

        /**
         *
         */
        PublishPeriod::PublishPeriodEnum getPublishPeriod();

        /**
         *
         */
        bool isTransmission();

        /**
         *
         */
        bool isReception();

        /**
         *
         */
        void reallocate();

        /**
         *
         */
        void deallocate();

        /**
         *
         */
        bool matchAllocation(ServiceId serviceId, Address32 peerAddress);

        bool matchAllocationWithOlderHandler(Uint32 allocationHandler, Address32 peerAddress);
        /**
         *
         */
        bool matchAllocation(Uint32 allocationHandler, Uint8 timeslotIndex,
                    PublishPeriod::PublishPeriodEnum publishPeriod);

        /**
         *
         */
        bool matchAllocation(Uint32 allocationHandler, Address32 peerAddress, Uint8 channel, Uint8 timeslotIndex,
                    PublishPeriod::PublishPeriodEnum publishPeriod);

        /**
         *
         */
        bool matchAllocation(LinkTypes::LinkTypesEnum linkType);

        /**
         *
         */
        void commitChanges();

        /**
         *
         */
        void generateOperations(NE::Model::Operations::EngineOperations& engineOperations, Address32 address,
                    Uint8 starIndex, bool useSingleReception);

        /**
         * Called when the link added operation has been confirmed.
         */
        void confirmAddLinkOperation();

        /**
         *
         */
        void toString(std::ostringstream& stream, Address32 address, int starIndex);

};

}
}
}
#endif /* TIMESLOTALLOCATION_H_ */
