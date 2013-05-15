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
 * ReadWirelessDeviceCapabilitiesOperation.cpp
 *
 *  Created on: Jan 18, 2011
 *      Author: Mihai.Stef
 */

#include "ReadWirelessDeviceCapabilitiesOperation.h"
#include "Model/Operations/IEngineOperationsVisitor.h"

namespace NE {

namespace Model {

namespace Operations {


ReadWirelessDeviceCapabilitiesOperation::ReadWirelessDeviceCapabilitiesOperation(Address32 _owner) {
    owner = _owner;
    setResponseCode(0);
}

ReadWirelessDeviceCapabilitiesOperation::~ReadWirelessDeviceCapabilitiesOperation() {
}

EngineOperationType::EngineOperationTypeEnum ReadWirelessDeviceCapabilitiesOperation::getOperationType() {
    return EngineOperationType::READ_WIRELESS_DEVICE_CAPABILITIES;
}

void ReadWirelessDeviceCapabilitiesOperation::toStringInternal(std::ostringstream &stream)
{
    stream << ", rc:" << std::dec << (int) responseCode;
    stream << "}";

}


bool ReadWirelessDeviceCapabilitiesOperation::accept(NE::Model::Operations::IEngineOperationsVisitor& visitor) {
    return visitor.visit(*this);
}

void ReadWirelessDeviceCapabilitiesOperation::setResponseCode(Uint8 responseCode_) {
    responseCode = responseCode_;
}

Uint8 ReadWirelessDeviceCapabilitiesOperation::getResponseCode() {
    return responseCode;
}

}
}
}
