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
 * SetClockSourceOperation.cpp
 *
 *  Created on: Oct 26, 2009
 *      Author: Andy
 */

#include "Model/Operations/IEngineOperationsVisitor.h"
#include "SetClockSourceOperation.h"

using namespace NE::Model;

using namespace NE::Model::Operations;

SetClockSourceOperation::SetClockSourceOperation(Address32 owner_, Address32 neighborAddress_, Uint8 flags_) :
    neighborAddress(neighborAddress_), flags(flags_) {
    owner = owner_;
    setResponseCode(0);
}

SetClockSourceOperation::~SetClockSourceOperation() {
}

void SetClockSourceOperation::toStringInternal(std::ostringstream &stream) {
    stream << ", ngh:" << ToStr(neighborAddress);
    stream << ", flags:" << (int) flags;
    stream << ", rc:" << std::dec << (int) responseCode;
    stream << "}";
}

NE::Model::Operations::EngineOperationType::EngineOperationTypeEnum SetClockSourceOperation::getOperationType() {
    return NE::Model::Operations::EngineOperationType::SET_CLOCK_SOURCE;
}

bool SetClockSourceOperation::accept(NE::Model::Operations::IEngineOperationsVisitor& visitor) {
    return visitor.visitSetClockSourceOperation(*this);
}

void SetClockSourceOperation::setResponseCode(Uint8 responseCode_) {
    switch (responseCode_) {
        case C971_NOO:
        case C971_E05:
        case C971_E16:
        case RCS_E64_CommandNotImplemented:
        case C971_E65:
        case C971_E66:
            responseCode = (C971_WriteNeighbourPropertyFlag_RespCodes) responseCode_;
        break;
        default:
            LOG_WARN("Error code: " << (int) responseCode_ << " not treated for command C971.");
        break;
    }
}

Uint8 SetClockSourceOperation::getResponseCode() {
    return responseCode;
}

