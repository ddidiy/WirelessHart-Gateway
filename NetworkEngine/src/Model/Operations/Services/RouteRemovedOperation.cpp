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
#include "RouteRemovedOperation.h"

using namespace NE::Model::Operations;

RouteRemovedOperation::RouteRemovedOperation(Address32 owner_, Uint32 routeId_, Uint16 graphId_) :
    routeId(routeId_), graphId(graphId_) {
    owner = owner_;
    deleteRouteResponseCode = C975_NOO;
}

RouteRemovedOperation::~RouteRemovedOperation() {
}

Uint32 RouteRemovedOperation::getRouteId() {
    return routeId;
}

void RouteRemovedOperation::toStringInternal(std::ostringstream& stream) {
    stream << ", routeId=" << std::hex << (int) routeId << std::dec;
    stream << ", rc:" << std::dec << (int) deleteRouteResponseCode;
    stream << "}";
}

EngineOperationType::EngineOperationTypeEnum RouteRemovedOperation::getOperationType() {
    return EngineOperationType::REMOVE_ROUTE;
}

bool RouteRemovedOperation::accept(NE::Model::Operations::IEngineOperationsVisitor& visitor) {
    return visitor.visitRouteRemovedOperation(*this);
}

void RouteRemovedOperation::setResponseCode(Uint8 responseCode) {
    if (responseCode == C975_NOO || responseCode == C975_E05 || responseCode == C975_E16 || responseCode == C975_E65) {
        deleteRouteResponseCode = (C975_DeleteRoute_RespCodes) responseCode;
    }
}

Uint8 RouteRemovedOperation::getResponseCode() {
    return deleteRouteResponseCode;
}
