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
 * SuperframeAddedOperation.cpp
 *
 *  Created on: Sep 17, 2009
 *      Author: ioan.pocol
 */

#include "Model/Operations/IEngineOperationsVisitor.h"
#include "SuperframeAddedOperation.h"

using namespace NE::Misc::Marshall;
using namespace NE::Model;

using namespace NE::Model::Operations;

SuperframeAddedOperation::SuperframeAddedOperation(Address32 owner_, Uint8 superframeId_, Uint16 superframeLength_,
            Uint8 superframeMode_) :
    superframeId(superframeId_), superframeLength(superframeLength_), superframeMode(superframeMode_) {
    owner = owner_;
    setResponseCode(0);
}

SuperframeAddedOperation::~SuperframeAddedOperation() {
}

void SuperframeAddedOperation::toStringInternal(std::ostringstream &stream) {
    stream << ", sfID=" << (int) superframeId;
    stream << ", sfLen=" << (int) superframeLength;
    stream << ", mode=" << (int) superframeMode;
    stream << ", rc:" << std::dec << (int) responseCode;
    stream << "}";
}

NE::Model::Operations::EngineOperationType::EngineOperationTypeEnum SuperframeAddedOperation::getOperationType() {
    return NE::Model::Operations::EngineOperationType::ADD_SUPERFRAME;
}

bool SuperframeAddedOperation::accept(NE::Model::Operations::IEngineOperationsVisitor& visitor) {
    return visitor.visitSuperframeAddedOperation(*this);
}

void SuperframeAddedOperation::setResponseCode(Uint8 responseCode_) {
    switch (responseCode_) {
        case C965_NOO:
        case C965_E05:
        case C965_E16:
        case C965_E65:
        case C965_E66:
        case C965_E67:
        case C965_E68:
            responseCode = (C965_WriteSuperframe_RespCodes) responseCode_;
        break;
        default:
            LOG_WARN("Error code: " << (int) responseCode_ << " not treated for command C965.");
        break;
    }
}

Uint8 SuperframeAddedOperation::getResponseCode() {
    return responseCode;
}

