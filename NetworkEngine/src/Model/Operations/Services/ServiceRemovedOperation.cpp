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
 * ServiceRemovedOperation.cpp
 *
 *  Created on: Sep 18, 2009
 *      Author: ioanpocol
 */

#include "ServiceRemovedOperation.h"

#include "Model/Operations/IEngineOperationsVisitor.h"
#include "Model/Operations/IEngineOperation.h"

using namespace NE::Model;
using namespace NE::Model::Operations;
using namespace NE::Model::Services;

ServiceRemovedOperation::ServiceRemovedOperation(Address32 owner_, Address32 peerAddress_, Uint32 serviceId_, Uint32 routeId_,
            DeleteReason::DeleteReasonEnum deleteReason_) :
    peerAddress(peerAddress_), serviceId(serviceId_), routeId(routeId_), deleteReason(deleteReason_) {

    owner = owner_;
    setResponseCode(0);
}

ServiceRemovedOperation::~ServiceRemovedOperation() {
}

ServiceId ServiceRemovedOperation::getServiceId() {
    return serviceId;
}

EngineOperationType::EngineOperationTypeEnum ServiceRemovedOperation::getOperationType() {
    return EngineOperationType::REMOVE_SERVICE;
}

void ServiceRemovedOperation::toStringInternal(std::ostringstream& stream) {

    stream << ", serviceId:" << std::hex << (int) serviceId << std::dec;
    stream << ", deleteReason:" << (int) deleteReason;
    stream << ", rc:" << std::dec << (int) responseCode;
    stream << "}";

}

bool ServiceRemovedOperation::accept(NE::Model::Operations::IEngineOperationsVisitor& visitor) {
    return visitor.visitServiceRemovedOperation(*this);
}

void ServiceRemovedOperation::setResponseCode(Uint8 responseCode_) {
    switch (responseCode_) {
        case C801_NOO:
        case C801_E05:
        case C801_E16:
        case C801_E65:
        case C801_E66:
        case C801_E67:
        case C801_E68:
            responseCode = (C801_DeleteService_RespCodes) responseCode_;
        break;
        default:
            LOG_WARN("Error code: " << (int) responseCode_ << " not treated for command C801.");
        break;
    }
}

Uint8 ServiceRemovedOperation::getResponseCode() {
    return responseCode;
}
