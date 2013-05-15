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
 * SourceRouteAddedOperation.cpp
 *
 *  Created on: Oct 30, 2009
 *      Author: Andy
 */

#include "Misc/Marshall/NetworkOrderStream.h"
#include "Model/Operations/IEngineOperationsVisitor.h"
#include "RouteAddedOperation.h"

using namespace NE::Common;
using namespace NE::Model;
using namespace NE::Model::Topology;
using namespace NE::Model::Operations;

SourceRouteAddedOperation::SourceRouteAddedOperation(Address32 owner_, Uint32 routeId_, Uint16 graphId_, Address32 peerAddress_) :
    routeId(routeId_), graphId(graphId_), peerAddress(peerAddress_) {

    owner = owner_;
    writeSourceRouteResponseCode = C976_NOO;
}

SourceRouteAddedOperation::~SourceRouteAddedOperation() {
}

Uint32 SourceRouteAddedOperation::getRouteId() {
    return routeId;
}

EngineOperationType::EngineOperationTypeEnum SourceRouteAddedOperation::getOperationType() {
    return EngineOperationType::ADD_SOURCEROUTE;
}

void SourceRouteAddedOperation::toStringInternal(std::ostringstream &stream) {

    stream << ", routeId=" << std::hex << (int) routeId;
    stream << ", graphId=" << (int) graphId;
    stream << ", peerAddress=" << ToStr(peerAddress) << std::dec;
    stream << ", hopsNo=" << (int) destinations.size();

    stream << ", destinations=";
    for (std::list<Address32>::iterator it = destinations.begin(); it != destinations.end(); ++it) {
        stream << ToStr(*it) << ", ";
    }

    stream << ", rc:" << std::dec << (int) writeSourceRouteResponseCode;
    stream << "}";

}

bool SourceRouteAddedOperation::accept(NE::Model::Operations::IEngineOperationsVisitor& visitor) {
    return visitor.visitSourceRouteAddedOperation(*this);
}

void SourceRouteAddedOperation::setResponseCode(Uint8 responseCode) {
    if (responseCode == C976_NOO || responseCode == C976_E05 || responseCode == C976_W08 || responseCode == C976_E16
                || responseCode == C976_E65 || responseCode == C976_E66 || responseCode == C976_E67 || responseCode == C976_E68) {
        writeSourceRouteResponseCode = (C976_WriteSourceRoute_RespCodes) responseCode;
    }
}

Uint8 SourceRouteAddedOperation::getResponseCode() {
    return writeSourceRouteResponseCode;
}
