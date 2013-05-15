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
 * WriteTimerIntervalOperation.cpp
 *
 *  Created on: Jun 8, 2010
 *      Author: radu
 */

#include "WriteTimerIntervalOperation.h"
#include "Model/Operations/IEngineOperationsVisitor.h"

namespace NE {
namespace Model {
namespace Operations {

WriteTimerIntervalOperation::WriteTimerIntervalOperation(Address32 owner_, Uint32 timeIntervalMsecs_,
            WirelessTimerCode timerCode_) :
    timeIntervalMsecs(timeIntervalMsecs_), timerCode(timerCode_) {
    owner = owner_;
    setResponseCode(0);
}

void WriteTimerIntervalOperation::toStringInternal(std::ostringstream &stream) {
    stream << ", tI=" << std::dec << (int) (timeIntervalMsecs / 1000) << "s";

    switch (timerCode) {
        case WirelessTimerCode_Discovery:
            stream << ", WirelessTimerCode_Discovery";
        break;
        case WirelessTimerCode_Advertisment:
            stream << ", WirelessTimerCode_Advertisment";
        break;
        case WirelessTimerCode_Keep_Alive:
            stream << ", WirelessTimerCode_Keep_Alive";
        break;
        case WirelessTimerCode_PathFailure:
            stream << ", WirelessTimerCode_PathFailure";
        break;
        case WirelessTimerCode_HealthReport:
            stream << ", WirelessTimerCode_HealthReport";
        break;
        case WirelessTimerCode_BroadcastReply:
            stream << ", WirelessTimerCode_BroadcastReply";
        break;
        case WirelessTimerCode_MaximumPDUAge:
            stream << ", WirelessTimerCode_MaximumPDUAge";
        case WirelessTimerCode_NoOfEntries:
            stream << ", WirelessTimerCode_NoOfEntries";
        break;
        default:
            stream << ", Unknown WirelessTimerCode";
    }

    stream << ", rc:" << std::dec << (int) responseCode;
    stream << "}";
}

NE::Model::Operations::EngineOperationType::EngineOperationTypeEnum WriteTimerIntervalOperation::getOperationType() {
    return NE::Model::Operations::EngineOperationType::SET_KEEP_ALIVE_PERIOD;
}

bool WriteTimerIntervalOperation::accept(NE::Model::Operations::IEngineOperationsVisitor& visitor) {
    return visitor.visitWriteTimerIntervalOperation(*this);
}

void WriteTimerIntervalOperation::setResponseCode(Uint8 responseCode_) {
    switch (responseCode_) {
        case C795_NOO:
        case C795_EO3:
        case C795_EO4:
        case C795_E05:
        case C795_E06:
        case C795_E07:
        case C795_W08:
        case C795_E16:
        case C795_E32:
        case C795_E33:
        case C795_E34:
        case C795_E35:
        case C795_E36:
        case C795_E65:
            responseCode = (C795_WriteTimerInterval_RespCodes) responseCode_;
        break;
        default:
            LOG_WARN("Error code: " << (int) responseCode_ << " not treated for command C975.");
        break;
    }
}

Uint8 WriteTimerIntervalOperation::getResponseCode() {
    return responseCode;
}

}
}
}
