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

#include "LinkRemovedOperation.h"
#include "Misc/Marshall/NetworkOrderStream.h"
#include "Model/Operations/IEngineOperationsVisitor.h"

using namespace NE::Model;
using namespace NE::Model::Tdma;
using namespace NE::Model::Operations;

LinkRemovedOperation::LinkRemovedOperation(Address32 owner_, Uint32 allocationHandler_, Uint16 superframeId_,
            NE::Model::Tdma::PublishPeriod::PublishPeriodEnum publishPeriod_, Uint8 starIndex_, Uint8 timeslotIndex_,
            Uint8 channel_, Address32 peerAddress_, bool transmission_, bool reception_, bool shared_,
            LinkTypes::LinkTypesEnum linkType_) :

    allocationHandler(allocationHandler_), superframeId(superframeId_), publishPeriod(publishPeriod_), starIndex(
                starIndex_), timeslotIndex(timeslotIndex_), channel(channel_), peerAddress(peerAddress_), transmission(
                transmission_), reception(reception_), shared(shared_), linkType(linkType_) {

    owner = owner_;
    setResponseCode(0);
}

Address32 LinkRemovedOperation::getPeerAddress() {
    return peerAddress;
}

bool LinkRemovedOperation::isTransmission() {
    return transmission;
}

bool LinkRemovedOperation::isReception() {
    return reception;
}

bool LinkRemovedOperation::isNormalLink() {
    return linkType == LinkTypes::NORMAL;
}

LinkRemovedOperation::~LinkRemovedOperation() {
}

void LinkRemovedOperation::toStringInternal(std::ostringstream &stream) {
    //    toStringCommonOperationState(stream);
    stream << ", handler=" << std::hex << std::setw(8) << (int) allocationHandler << std::dec;
    stream << ", sfID:" << std::setw(2) << (int) superframeId;
    stream << ", pP:" << std::setw(3) << (int) publishPeriod;
    stream << ", sIx:" << std::setw(2) << (int) starIndex;
    stream << ", tIx:" << std::setw(3) << (int) timeslotIndex;
    stream << ", ch:" << std::setw(2) << (int) channel;
    stream << ", peer:" << std::setw(4) << ToStr(peerAddress);
    stream << ", type:" << (int) linkType;
    stream << ", Tx:" << (int) transmission;
    stream << ", Rx:" << (int) reception;
    stream << ", Sh:" << (int) shared;
    stream << ", rc:" << std::dec << (int) responseCode;
    stream << "}";
}

EngineOperationType::EngineOperationTypeEnum LinkRemovedOperation::getOperationType() {
    return EngineOperationType::REMOVE_LINK;
}

bool LinkRemovedOperation::accept(NE::Model::Operations::IEngineOperationsVisitor& visitor) {
    return visitor.visitLinkRemovedOperation(*this);
}

void LinkRemovedOperation::setResponseCode(Uint8 responseCode_) {
    switch (responseCode_) {
        case C968_NOO:
        case C968_E05:
        case C968_E16:
        case C968_E65:
            responseCode = (C968_DeleteLink_RespCodes) responseCode_;
        break;
        default:
            LOG_WARN("Error code: " << (int) responseCode_ << " not treated for command C968.");
        break;
    }
}

Uint8 LinkRemovedOperation::getResponseCode() {
    return responseCode;
}
