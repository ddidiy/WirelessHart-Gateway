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
 * ChangeNotificationOperation.cpp
 *
 *  Created on: Apr 27, 2010
 *      Author: radu
 */

#include "ChangeNotificationOperation.h"
#include "Model/Operations/IEngineOperationsVisitor.h"

namespace NE {

namespace Model {

namespace Operations {

ChangeNotificationOperation::ChangeNotificationOperation(Address32 owner_, NE::Common::Address64 deviceAddress_,
            uint16_t notification_) {
    owner = owner_;
    deviceAddress = deviceAddress_;
    changeNotification = notification_;
    setResponseCode(0);
}

ChangeNotificationOperation::~ChangeNotificationOperation() {
}

EngineOperationType::EngineOperationTypeEnum ChangeNotificationOperation::getOperationType() {
    return EngineOperationType::CHANGE_NOTIFICATION;
}

void ChangeNotificationOperation::toStringInternal(std::ostringstream &stream) {
    stream << ", deviceAddress: " << deviceAddress.toString();
    stream << ", changeNotification: " << (int) changeNotification;
    stream << ", rc:" << std::dec << (int) responseCode;

    stream << "}";
}

bool ChangeNotificationOperation::accept(NE::Model::Operations::IEngineOperationsVisitor& visitor) {
    return visitor.visit(*this);
}

void ChangeNotificationOperation::setResponseCode(Uint8 responseCode_) {
    responseCode = responseCode_;
}

Uint8 ChangeNotificationOperation::getResponseCode() {
    return responseCode;
}

}

}

}
