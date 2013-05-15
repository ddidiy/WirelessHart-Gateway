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
 * WriteKeyOperation.cpp
 *
 *  Created on: Oct 30, 2009
 *      Author: Andy
 */
#include "WriteKeyOperation.h"
#include "Model/Operations/IEngineOperationsVisitor.h"

namespace NE {
namespace Model {
namespace Operations {

WriteKeyOperation::WriteKeyOperation(Address32 owner_, const NE::Model::SecurityKey& key_) :
    key(key_) {
    owner = owner_;
}

EngineOperationType::EngineOperationTypeEnum WriteKeyOperation::getOperationType() {
    return EngineOperationType::WRITE_KEY;
}

void WriteKeyOperation::toStringInternal(std::ostringstream &stream) {
    stream << "}";
}

void WriteKeyOperation::setResponseCode(Uint8 responseCode_) {
}

Uint8 WriteKeyOperation::getResponseCode() {
    return 0;
}

bool WriteKeyOperation::accept(NE::Model::Operations::IEngineOperationsVisitor& visitor) {
    return visitor.visit(*this);
}

}
}
}
