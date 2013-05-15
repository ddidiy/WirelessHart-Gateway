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
 * WHOperation.cpp
 *
 *  Created on: Dec 16, 2008
 *      Author: Radu Pop
 */

#include "WHOperation.h"

namespace hart7 {

namespace nmanager {

namespace operations {

WHOperation::WHOperation()
{
    wHartCommand.responseCode = 0;
    responseCode = 0;
}

WHOperation::WHOperation(Uint16 subnetId, const Uint16 address16_, Uint16 engineOperationId_)
{
    responseCode = 0;
    operationState = OperationState::GENERATED;
}

WHOperation::WHOperation(WHartAddress destinationAddress_, uint16_t commandID_, void* command_) :
    destinationAddress(destinationAddress_)
{
    wHartCommand.commandID = commandID_;
    wHartCommand.responseCode = 0;
    wHartCommand.command = command_;

    responseCode = 0;

    operationState = OperationState::GENERATED;

}

WHOperation::~WHOperation()
{
}

std::string WHOperation::toString()
{
    std::ostringstream stream;
    stream << "WHOperation(";
    if (engineOperationPointer)
    {
        engineOperationPointer->toString(stream);
    }
    stream << ")";

    return stream.str();
}

}

}

}
