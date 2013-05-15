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
 * NivisCustom64765Operation.cpp
 *
 *  Created on: Apr 30, 2010
 *      Author: radu
 */

#include "NivisCustom64765Operation.h"
#include "Model/Operations/IEngineOperationsVisitor.h"

namespace NE {

namespace Model {

namespace Operations {

NivisCustom64765Operation::NivisCustom64765Operation(Address32 owner_, uint16_t CommandId_,uint16_t Nickname_,
                                                     _device_address_t DeviceUniqueId_, uint8_t CommandSize_,
                                                     uint8_t* Command_)
{
    owner = owner_;
    this->CommandId = CommandId_;
    this->Nickname = Nickname_;
    std::memcpy(this->DeviceUniqueId, DeviceUniqueId_, 5);
    this->CommandSize = CommandSize_;
    std::memcpy(this->Command, Command_, CommandSize_);

    setResponseCode(0);
}

NivisCustom64765Operation::~NivisCustom64765Operation()
{
}

NE::Model::Operations::EngineOperationType::EngineOperationTypeEnum NivisCustom64765Operation::getOperationType()
{
    return NE::Model::Operations::EngineOperationType::NIVIS_CUSTOM_64765;
}

void NivisCustom64765Operation::toStringInternal(std::ostringstream &stream)
{
    stream << ", rc= " << std::dec << (int) responseCode;
    stream << ", cmdId= " << (int) CommandId;
    stream << ", cmdSize= " << (int) CommandSize;
    stream << ", cmd= " << NE::Misc::Convert::array2string(Command, CommandSize);

    stream << "}";
}

bool NivisCustom64765Operation::accept(NE::Model::Operations::IEngineOperationsVisitor& visitor)
{
    return visitor.visit(*this);
}

void NivisCustom64765Operation::setResponseCode(Uint8 responseCode_)
{
    responseCode = responseCode_;
}

Uint8 NivisCustom64765Operation::getResponseCode()
{
    return responseCode;
}

}

}

}
