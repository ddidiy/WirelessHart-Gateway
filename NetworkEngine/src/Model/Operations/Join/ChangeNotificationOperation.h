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
 * ChangeNotification.h
 *
 *  Created on: Apr 27, 2010
 *      Author: radu
 */

#ifndef CHANGENOTIFICATION_H_
#define CHANGENOTIFICATION_H_

#include "Model/Operations/EngineOperation.h"
#include <ApplicationLayer/Model/DataLinkLayerCommands.h>

namespace NE {
namespace Model {
namespace Operations {

class ChangeNotificationOperation;
typedef boost::shared_ptr<ChangeNotificationOperation> ChangeNotificationOperationPointer;

class ChangeNotificationOperation: public NE::Model::Operations::EngineOperation {

    private:

        NE::Common::Address64 deviceAddress;

        uint16_t changeNotification;

        uint8_t responseCode;

    public:

        ChangeNotificationOperation(Address32 owner_, NE::Common::Address64 deviceAddress,
                                uint16_t notification);

        virtual ~ChangeNotificationOperation();

        void toStringInternal(std::ostringstream &stream);

        EngineOperationType::EngineOperationTypeEnum getOperationType();

        bool accept(IEngineOperationsVisitor & visitor);

        virtual void setResponseCode(Uint8 responseCode);

        virtual Uint8 getResponseCode();

        std::string getName() {
            return "ChangeNotification";
        }

        NE::Common::Address64 getDeviceAddress() {
            return deviceAddress;
        }

        uint16_t getChangeNotification() {
            return changeNotification;
        }
};

}

}

}

#endif /* CHANGENOTIFICATION_H_ */
