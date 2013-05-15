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
 * ServiceAddedOperation.cpp
 *
 *  Created on: Sep 18, 2009
 *      Author: ioanpocol
 */

#include "ServiceAddedOperation.h"

#include "Model/Operations/IEngineOperationsVisitor.h"
#include "Model/Operations/IEngineOperation.h"

using namespace NE::Model;
using namespace NE::Model::Operations;
using namespace NE::Model::Services;

ServiceAddedOperation::ServiceAddedOperation(Address32 owner_, ServiceId serviceId_, bool source_, bool sink_,
            bool intermittent_, ApplicationDomain::ApplicationDomainEnum applicationDomain_, Address32 peerAddress32_,
            Uint32 period_, Uint8 routeId_) :

    serviceId(serviceId_), source(source_), sink(sink_), intermittent(intermittent_), applicationDomain(applicationDomain_),
                peerAddress32(peerAddress32_), period(period_), routeId(routeId_) {

    owner = owner_;
    setResponseCode(0);
}

ServiceAddedOperation::~ServiceAddedOperation() {
}

ServiceId ServiceAddedOperation::getServiceId() {
    return serviceId;
}

EngineOperationType::EngineOperationTypeEnum ServiceAddedOperation::getOperationType() {
    return EngineOperationType::ADD_SERVICE;
}

void ServiceAddedOperation::toStringInternal(std::ostringstream& stream) {

    stream << ", serviceId:" << std::hex << serviceId << std::dec;
    stream << ", source:" << (int) source;
    stream << ", sink:" << (int) sink;
    stream << ", int:" << (int) intermittent;
    stream << ", appD:" << (int) applicationDomain;
    stream << ", peer:" << ToStr(peerAddress32);
    stream << ", period:" << period;
    stream << ", routeId:" << (int) routeId;
    stream << ", rc:" << std::dec << (int) writeServiceResponsecode;
    stream << "}";

}

bool ServiceAddedOperation::accept(NE::Model::Operations::IEngineOperationsVisitor& visitor) {
    return visitor.visitServiceAddedOperation(*this);
}

void ServiceAddedOperation::setResponseCode(Uint8 responseCode) {
    switch (responseCode) {
        case C973_NOO:
        case C973_E05:
        case C973_E16:
        case C973_E65:
        case C973_E66:
        case C973_E67:
        case C973_E68:
        case C973_E69:
        case C973_E70:
        case C973_E71:
            writeServiceResponsecode = (C973_WriteService_RespCodes) responseCode;
        break;
        default:
            LOG_WARN("Error code: " << (int) responseCode << " not treated for command C973.");
        break;
    }
}

Uint8 ServiceAddedOperation::getResponseCode() {
    return writeServiceResponsecode;
}
