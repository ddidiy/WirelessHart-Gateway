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

#ifndef LINKREMOVEDOPERATION_H_
#define LINKREMOVEDOPERATION_H_

#include "Model/Operations/EngineOperation.h"
#include "Model/Tdma/TdmaTypes.h"

#include <ApplicationLayer/Model/WirelessNetworkManagerCommands.h>

namespace NE {
namespace Model {
namespace Operations {

/**
 * A LinkRemovedOperation is generated when a slot (and implicitly one or two links) is removed from superframe
 */
class LinkRemovedOperation: public NE::Model::Operations::EngineOperation {

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

        C968_DeleteLink_RespCodes responseCode;

    public:

        LinkRemovedOperation(Address32 owner, Uint32 allocationHandler, Uint16 superframeId,
                    NE::Model::Tdma::PublishPeriod::PublishPeriodEnum publishPeriod, Uint8 starIndex,
                    Uint8 timeslotIndex, Uint8 channel, Address32 peerAddress, bool transmission, bool reception,
                    bool shared, NE::Model::Tdma::LinkTypes::LinkTypesEnum linkType);

        virtual ~LinkRemovedOperation();

        std::string getName() {
            return "Link Removed";
        }

        Address32 getPeerAddress();


        bool isTransmission();

        bool isReception();

        bool isNormalLink();

        void toStringInternal(std::ostringstream &stream);

        EngineOperationType::EngineOperationTypeEnum getOperationType();

        bool accept(NE::Model::Operations::IEngineOperationsVisitor& visitor);

        virtual void setResponseCode(Uint8 responseCode);

        virtual Uint8 getResponseCode();

        uint16_t GetSlotNo() {
            return (uint16_t)starIndex + (uint16_t)timeslotIndex * MAX_STAR_INDEX;
        }

};

}
}
}

#endif /*LINKREMOVEDSUPERFRAMEOPERATION_H_*/
