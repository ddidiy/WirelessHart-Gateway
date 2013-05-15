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
 * SourceRouteRemovedOperation.cpp
 *
 *  Created on: Oct 30, 2009
 *      Author: Andy
 */

#include "Misc/Marshall/NetworkOrderStream.h"
#include "Model/Operations/IEngineOperationsVisitor.h"
#include "SourceRouteRemovedOperation.h"

using namespace NE::Model::Operations;

SourceRouteRemovedOperation::SourceRouteRemovedOperation(Address32 owner_, Uint32 routeId_) :
    routeId(routeId_) {

    owner = owner_;
    deleteSourceRouteResponseCode = C977_NOO;
}

SourceRouteRemovedOperation::~SourceRouteRemovedOperation() {
}

Uint32 SourceRouteRemovedOperation::getRouteId() {
    return routeId;
}

void SourceRouteRemovedOperation::toStringInternal(std::ostringstream& stream) {
    stream << ", routeId=" << std::hex << (int) routeId << std::dec;
    stream << ", rc:" << std::dec << (int) deleteSourceRouteResponseCode;
    stream << "}";
}

EngineOperationType::EngineOperationTypeEnum SourceRouteRemovedOperation::getOperationType() {
    return EngineOperationType::REMOVE_SOURCEROUTE;
}

bool SourceRouteRemovedOperation::accept(NE::Model::Operations::IEngineOperationsVisitor& visitor) {
    return visitor.visitSourceRouteRemovedOperation(*this);
}

void SourceRouteRemovedOperation::setResponseCode(Uint8 responseCode) {
    if (responseCode == C977_NOO || responseCode == C977_E05 || responseCode == C977_E16 || responseCode == C977_E66) {
        deleteSourceRouteResponseCode = (C977_DeleteSourceRoute_RespCodes) responseCode;
    }
}

Uint8 SourceRouteRemovedOperation::getResponseCode() {
    return deleteSourceRouteResponseCode;
}
