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
 * NivisCustom64765Operation.h
 *
 *  Created on: Apr 30, 2010
 *      Author: radu
 */

#ifndef NIVISCUSTOM64765OPERATION_H_
#define NIVISCUSTOM64765OPERATION_H_

#include "Model/Operations/EngineOperation.h"
#include <WHartStack/WHartStack.h>

namespace NE {

namespace Model {

namespace Operations {

class NivisCustom64765Operation;
typedef boost::shared_ptr<NivisCustom64765Operation> NivisCustom64765OperationPointer;

class NivisCustom64765Operation: public NE::Model::Operations::EngineOperation {
    public:

        uint16_t CommandId;
        uint16_t Nickname; // F980 for NM, F981 for GW
        _device_address_t DeviceUniqueId;
        uint8_t CommandSize; // !!! this will no be serializable
        uint8_t Command[256];

    private:

        uint8_t responseCode;

    public:

        NivisCustom64765Operation(Address32 owner, uint16_t CommandId, uint16_t Nickname,
                    _device_address_t DeviceUniqueId, uint8_t CommandSize, uint8_t* Command);

        virtual ~NivisCustom64765Operation();

        void toStringInternal(std::ostringstream &stream);

        NE::Model::Operations::EngineOperationType::EngineOperationTypeEnum getOperationType();

        bool accept(NE::Model::Operations::IEngineOperationsVisitor & visitor);

        virtual void setResponseCode(Uint8 responseCode);

        virtual Uint8 getResponseCode();

        std::string getName() {
            return "NivisCustom64765Operation";
        }
};

}

}

}

#endif /* NIVISCUSTOM64765OPERATION_H_ */
