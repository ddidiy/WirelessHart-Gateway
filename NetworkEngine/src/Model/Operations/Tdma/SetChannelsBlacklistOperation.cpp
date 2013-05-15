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
 * SetChannelsBlacklistOperation.cpp
 *
 *  Created on: Jun 30, 2009
 *      Author: Andy
 */

#include "SetChannelsBlacklistOperation.h"
#include "Model/Operations/IEngineOperationsVisitor.h"

using namespace NE::Model::Operations;

bool SetChannelsBlacklistOperation::accept(IEngineOperationsVisitor& visitor) {
    return visitor.visitSetChannelsBlacklistOperation(*this);
}

void SetChannelsBlacklistOperation::setResponseCode(Uint8 responseCode) {
    this->responseCode = (C818_WriteChannelBlacklist_RespCodes)responseCode;
}

Uint8 SetChannelsBlacklistOperation::getResponseCode() {
    return responseCode;
}


void SetChannelsBlacklistOperation::toStringInternal(std::ostringstream &str) {
    toStringCommonOperationState(str);
    str << ", channels=" << std::hex << AvailableChannels;
    str << ", rc:" << std::dec << (int) responseCode;
    str << "}";
}

