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
 * ReadLinksOperation.cpp
 *
 *  Created on: May 17, 2010
 *      Author: andrei.petrut
 */

#include "ReadLinksOperation.h"
#include "Model/Operations/IEngineOperationsVisitor.h"

using namespace NE::Model;
using namespace NE::Model::Tdma;
using namespace NE::Model::Operations;

ReadLinksOperation::ReadLinksOperation(Address32 owner, int startIndex, int count)
{
    this->owner = owner;
    this->startIndex = startIndex;
    this->count = count;
}

ReadLinksOperation::~ReadLinksOperation() {}

void ReadLinksOperation::toStringInternal(std::ostringstream &stream)
{
    stream << ", stIx:" << startIndex << ", cnt:" << count;
    stream << ", rc:" << std::dec << (int) responseCode;
    stream << "}";

}

EngineOperationType::EngineOperationTypeEnum ReadLinksOperation::getOperationType()
{
    return EngineOperationType::READ_LINKLIST;
}

bool ReadLinksOperation::accept(NE::Model::Operations::IEngineOperationsVisitor& visitor)
{
    return visitor.visitReadLinksOperation(*this);
}

void ReadLinksOperation::setResponseCode(Uint8 responseCode_)
{
   responseCode =  (C784_ReadLinkList_RespCodes)responseCode_;
}

Uint8 ReadLinksOperation::getResponseCode()
{
    return responseCode;
}
