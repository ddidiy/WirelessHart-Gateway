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

#include "Misc/Marshall/NetworkOrderStream.h"
#include "Model/Operations/IEngineOperationsVisitor.h"
#include "RouteAddedOperation.h"

using namespace NE::Common;
using namespace NE::Model;
using namespace NE::Model::Topology;
using namespace NE::Model::Operations;

RouteAddedOperation::RouteAddedOperation(Address32 owner_, Uint32 routeId_, Uint16 graphId_, Address32 peerAddress_) :
    routeId(routeId_), graphId(graphId_), peerAddress(peerAddress_) {

    owner = owner_;
    writeRouteResponseCode = C974_NOO;
}

RouteAddedOperation::~RouteAddedOperation() {
}

Uint32 RouteAddedOperation::getRouteId() {
    return routeId;
}

EngineOperationType::EngineOperationTypeEnum RouteAddedOperation::getOperationType() {
    return EngineOperationType::ADD_ROUTE;
}

void RouteAddedOperation::toStringInternal(std::ostringstream &stream) {

    stream << ", routeId=" << std::hex << (int) routeId;
    stream << ", graphId=" << (int) graphId;
    stream << ", peerAddress=" << peerAddress << std::dec;
    stream << ", rc:" << std::dec << (int) writeRouteResponseCode;
    stream << "}";
}

bool RouteAddedOperation::accept(NE::Model::Operations::IEngineOperationsVisitor& visitor) {
    return visitor.visitRouteAddedOperation(*this);
}

void RouteAddedOperation::setResponseCode(Uint8 responseCode) {
    if (responseCode == C974_NOO || responseCode == C974_E05 || responseCode == C974_E16 || responseCode == C974_E65
                || responseCode == C974_E66 || responseCode == C974_E67) {
        writeRouteResponseCode = (C974_WriteRoute_RespCodes) responseCode;
    }
}

Uint8 RouteAddedOperation::getResponseCode() {
    return writeRouteResponseCode;
}
