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
 * NeighborHealthReportOperation.cpp
 *
 *  Created on: Jun 29, 2010
 *      Author: radu
 */

#include "NeighborHealthReportOperation.h"
#include "Model/Operations/IEngineOperationsVisitor.h"

namespace NE {

namespace Model {

namespace Operations {

NeighborHealthReportOperation::NeighborHealthReportOperation(Address32 owner_, NE::Common::Address64 deviceAddress_) {
    owner = owner_;
    deviceAddress = deviceAddress_;
    responseCode = 0;
}

NeighborHealthReportOperation::~NeighborHealthReportOperation() {
}

EngineOperationType::EngineOperationTypeEnum NeighborHealthReportOperation::getOperationType() {
    return EngineOperationType::NEIGHBOR_HEALTH_REPORT;
}

void NeighborHealthReportOperation::toStringInternal(std::ostringstream &stream) {
    stream << ", deviceAddress: " << deviceAddress.toString();
    stream << ", rc:" << std::dec << (int) responseCode;
    stream << "}";
}

void NeighborHealthReportOperation::setResponseCode(Uint8 responseCode_) {
    responseCode = responseCode_;
}

Uint8 NeighborHealthReportOperation::getResponseCode() {
    return responseCode;
}

bool NeighborHealthReportOperation::accept(NE::Model::Operations::IEngineOperationsVisitor& visitor) {
    return visitor.visit(*this);
}

}

}

}
