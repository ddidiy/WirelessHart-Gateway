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
 * WriteNetworkIDOperation.cpp
 *
 *  Created on: Jan 10, 2011
 *      Author: andy
 */

#include "WriteNetworkIDOperation.h"
#include "Model/Operations/IEngineOperationsVisitor.h"

namespace NE {

namespace Model {

namespace Operations {

WriteNetworkIDOperation::WriteNetworkIDOperation(Address32 owner_, uint16_t networkID_) {
    owner = owner_;
    networkID = networkID_;
    setResponseCode(0);
}

WriteNetworkIDOperation::~WriteNetworkIDOperation() {
}

EngineOperationType::EngineOperationTypeEnum WriteNetworkIDOperation::getOperationType() {
    return EngineOperationType::WRITE_NETWORKID;
}

void WriteNetworkIDOperation::toStringInternal(std::ostringstream &stream) {
    stream << ", networkID: " << std::hex << networkID;
    stream << ", rc:" << std::dec << (int) responseCode;

    stream << "}";
}

bool WriteNetworkIDOperation::accept(NE::Model::Operations::IEngineOperationsVisitor& visitor) {
    return visitor.visit(*this);
}

void WriteNetworkIDOperation::setResponseCode(Uint8 responseCode_) {
    responseCode = responseCode_;
}

Uint8 WriteNetworkIDOperation::getResponseCode() {
    return responseCode;
}

}

}

}
