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
 * ReadWirelessDeviceCapabilitiesOperation.h
 *
 *  Created on: Jan 18, 2011
 *      Author: Mihai.Stef
 */

#ifndef READWIRELESSDEVICECAPABILITIESOPERATION_H_
#define READWIRELESSDEVICECAPABILITIESOPERATION_H_

#include "Model/Operations/EngineOperation.h"

namespace NE {
namespace Model {
namespace Operations {

class ReadWirelessDeviceCapabilitiesOperation;

typedef boost::shared_ptr<ReadWirelessDeviceCapabilitiesOperation> ReadWirelessDeviceCapabilitiesOperationPointer;

class ReadWirelessDeviceCapabilitiesOperation: public NE::Model::Operations::EngineOperation {

    private:
        Uint8 responseCode;

    public:
        ReadWirelessDeviceCapabilitiesOperation(Address32 _owner);
        virtual ~ReadWirelessDeviceCapabilitiesOperation();

        EngineOperationType::EngineOperationTypeEnum getOperationType();

        void toStringInternal(std::ostringstream &stream);

        bool accept(IEngineOperationsVisitor & visitor);

        virtual void setResponseCode(Uint8 responseCode);

        virtual Uint8 getResponseCode();

        std::string getName() {
            return "ReadWirelessDevCap";
        }

};

}
}
}
#endif /* READWIRELESSDEVICECAPABILITIESOPERATION_H_ */
