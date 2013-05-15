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
 * ChangePriorityEngineOperation.cpp
 *
 *  Created on: Dec 16, 2009
 *      Author: Radu Pop
 */

#include "ChangePriorityEngineOperation.h"
#include "Model/Operations/IEngineOperationsVisitor.h"

namespace NE {
namespace Model {
namespace Operations {

ChangePriorityEngineOperation::ChangePriorityEngineOperation(Address32 owner_, uint8_t joinPriority_) :
    joinPriority(joinPriority_) {
    owner = owner_;
    setResponseCode(0);
}

ChangePriorityEngineOperation::~ChangePriorityEngineOperation() {
}

EngineOperationType::EngineOperationTypeEnum ChangePriorityEngineOperation::getOperationType() {
    return EngineOperationType::CHANGE_JOIN_PRIORITY;
}

void ChangePriorityEngineOperation::toStringInternal(std::ostringstream &stream) {
    stream << ", joinPriority=" << (int) joinPriority;
    stream << ", rc:" << std::dec << (int) responseCode;
    stream << "}";
}

bool ChangePriorityEngineOperation::accept(NE::Model::Operations::IEngineOperationsVisitor& visitor) {
    return visitor.visit(*this);
}

void ChangePriorityEngineOperation::setResponseCode(Uint8 responseCode_) {
    switch (responseCode_) {
        case C811_NOO:
        case C811_E05:
        case C811_E16:
            responseCode = (C811_WriteJoinPriority_RespCodes) responseCode_;
            break;
        default:
            LOG_WARN("Error code: " << (int) responseCode_ << " not treated for command C811.");
        break;
    }
}

Uint8 ChangePriorityEngineOperation::getResponseCode() {
    return responseCode;
}

}
}
}
