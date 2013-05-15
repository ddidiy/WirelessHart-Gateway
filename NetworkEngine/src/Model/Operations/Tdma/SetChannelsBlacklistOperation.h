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
 * SetChannelsBlacklistOperation.h
 *
 *  Created on: Jun 26, 2009
 *      Author: Andy
 */

#ifndef SETCHANNELSBLACKLISTOPERATION_H_
#define SETCHANNELSBLACKLISTOPERATION_H_

#include "Model/Operations/EngineOperation.h"
#include <ApplicationLayer/Model/NetworkLayerCommands.h>
namespace NE {
namespace Model {
namespace Operations {

class SetChannelsBlacklistOperation: public EngineOperation {

    public:

        typedef boost::shared_ptr<SetChannelsBlacklistOperation> Ptr;

        /**
         * The list of available channels. A channel is blacklisted if it is not present in this list.
         */
        Uint16 AvailableChannels;

        C818_WriteChannelBlacklist_RespCodes responseCode;

    public:

        SetChannelsBlacklistOperation(Address32 owner_) {
            this->owner = owner_;
            responseCode = C818_NOO;
        }

        virtual std::string getName() {
            return "SetChannelsBlacklist";
        }

        virtual EngineOperationType::EngineOperationTypeEnum getOperationType() {
            return EngineOperationType::SET_BLACKLIST_CHANNELS;
        }

        virtual bool accept(IEngineOperationsVisitor& visitor);

        void toStringInternal(std::ostringstream &str);

        virtual void setResponseCode(Uint8 responseCode);

        virtual Uint8 getResponseCode();

};

}
}
}

#endif /* SETCHANNELSBLACKLISTOPERATION_H_ */
