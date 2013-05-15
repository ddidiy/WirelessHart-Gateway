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
 * LinkAddedOperation.h
 *
 *  Created on: Sep 17, 2009
 *      Author: ioan.pocol
 */

#ifndef LINKADDEDOPERATION_H_
#define LINKADDEDOPERATION_H_

#include "Common/logging.h"
#include "Model/Operations/EngineOperation.h"
#include "Model/Tdma/TdmaTypes.h"

#include <ApplicationLayer/Model/WirelessNetworkManagerCommands.h>

namespace NE {
namespace Model {
namespace Operations {

class LinkAddedOperation: public NE::Model::Operations::EngineOperation {
    LOG_DEF("I.M.O.LinkAddedOperation");

    public:

        Uint32 allocationHandler;

        Uint16 superframeId;

        NE::Model::Tdma::PublishPeriod::PublishPeriodEnum publishPeriod;

        Uint8 starIndex;

        Uint8 timeslotIndex;

        Uint8 channel;

        Address32 peerAddress;

        bool transmission;

        bool reception;

        bool shared;

        NE::Model::Tdma::LinkTypes::LinkTypesEnum linkType;

        C967_WriteLink_RespCodes responseCode;

    public:

        LinkAddedOperation() {
        }

        LinkAddedOperation(Address32 owner, Uint32 allocationHandler, Uint16 superframeId,
                    NE::Model::Tdma::PublishPeriod::PublishPeriodEnum publishPeriod, Uint8 starIndex, Uint8 timeslotIndex,
                    Uint8 channel, Address32 peerAddress, bool transmission, bool reception, bool shared,
                    NE::Model::Tdma::LinkTypes::LinkTypesEnum linkType);

        virtual ~LinkAddedOperation();

        std::string getName() {
            return "Link Add";
        }

        NE::Model::Operations::EngineOperationType::EngineOperationTypeEnum getOperationType() {
            return EngineOperationType::ADD_LINK;
        }

        void toStringInternal(std::ostringstream& stream);

        uint16_t GetSlotNo() {
            return (uint16_t)starIndex + (uint16_t)timeslotIndex * MAX_STAR_INDEX;
        }

        uint8_t GetChannelOffset() {
            return channel - 1; //(GetSlotNo() + MAX_FREQUENCY - channel) % MAX_FREQUENCY;
        }

        bool accept(NE::Model::Operations::IEngineOperationsVisitor& visitor);

        virtual void setResponseCode(Uint8 responseCode);

        virtual Uint8 getResponseCode();
};

}
}
}

#endif /* LINKADDEDOPERATION_H_ */
