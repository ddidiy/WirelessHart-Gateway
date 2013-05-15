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
 * WriteSessionOperation.cpp
 *
 *  Created on: Oct 30, 2009
 *      Author: Andy
 */
#include "WriteSessionOperation.h"
#include "Model/Operations/IEngineOperationsVisitor.h"

namespace NE {
namespace Model {
namespace Operations {

using namespace hart7::stack;

WriteSessionOperation::WriteSessionOperation(Address32 owner_, Uint32 peerNonceCounterValue_, Address32 peerNickname_,
            WHartUniqueID peerUniqueID_, const NE::Model::SecurityKey& key_, WHartSessionKey::SessionKeyCode sessionType_) :
    peerNonceCounterValue(peerNonceCounterValue_), peerNickname(peerNickname_), peerUniqueID(peerUniqueID_), key(key_),
                sessionType(sessionType_) {
    owner = owner_;
}

EngineOperationType::EngineOperationTypeEnum WriteSessionOperation::getOperationType() {
    return EngineOperationType::WRITE_SESSION;
}

void WriteSessionOperation::toStringInternal(std::ostringstream &stream) {
    stream << ", key: " << key.toString();
    stream << "}";
}

void WriteSessionOperation::setResponseCode(Uint8 responseCode) {

}

Uint8 WriteSessionOperation::getResponseCode() {
    return 0;
}

bool WriteSessionOperation::accept(NE::Model::Operations::IEngineOperationsVisitor& visitor) {
    return visitor.visit(*this);
}

}
}
}
