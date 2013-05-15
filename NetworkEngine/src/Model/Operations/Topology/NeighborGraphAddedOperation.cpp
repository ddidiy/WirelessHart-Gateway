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
 * NeighborGraphAddedOperation.cpp
 *
 *  Created on: 30.01.2009
 *      Author: radu.pop
 */

#include "NeighborGraphAddedOperation.h"
#include "Model/Operations/IEngineOperationsVisitor.h"
#include "Model/Operations/Topology/NeighborGraphRemovedOperation.h"

using namespace NE::Model::Operations;

NeighborGraphAddedOperation::NeighborGraphAddedOperation(Address32 owner_, Uint16 graphId_, Address32 neighbor_) :
    graphId(graphId_), neighbor(neighbor_) {
    owner = owner_;
    setResponseCode(0);
}

NeighborGraphAddedOperation::~NeighborGraphAddedOperation() {
}

EngineOperationType::EngineOperationTypeEnum NeighborGraphAddedOperation::getOperationType() {
    return EngineOperationType::ADD_NEIGHBOR_GRAPH;
}

void NeighborGraphAddedOperation::toStringInternal(std::ostringstream &stream) {
    //    toStringCommonOperationState(stream);
    stream << ", graphId=" << std::hex << (int) graphId << std::dec;
    stream << ", neighbor=" << ToStr(neighbor);
    stream << ", rc:" << std::dec << (int) responseCode;
    stream << "}";
}

bool NeighborGraphAddedOperation::accept(NE::Model::Operations::IEngineOperationsVisitor& visitor) {
    return visitor.visitNeighborGraphAddedOperation(*this);
}

void NeighborGraphAddedOperation::setResponseCode(Uint8 responseCode_) {
    switch (responseCode_) {
        case C969_NOO:
        case C969_E05:
        case C969_E16:
        case C969_E65:
        case C969_E66:
        case C969_E67:
            responseCode = (C969_WriteGraphNeighbourPair_RespCodes) responseCode_;
        break;
        default:
            LOG_WARN("Error code: " << (int) responseCode_ << " not treated for command C969.");
        break;
    }
}

Uint8 NeighborGraphAddedOperation::getResponseCode() {
    return responseCode;
}
