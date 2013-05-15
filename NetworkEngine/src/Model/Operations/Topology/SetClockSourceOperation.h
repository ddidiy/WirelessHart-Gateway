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

//C971_WriteNeighbourPropertyFlag
/*
 * SetClockSourceOperation.h
 *
 *  Created on: Oct 26, 2009
 *      Author: Andy
 */

#ifndef SETCLOCKSOURCEOPERATION_H_
#define SETCLOCKSOURCEOPERATION_H_

#include "Model/Operations/EngineOperation.h"
#include "Common/NETypes.h"

#include <ApplicationLayer/Model/WirelessNetworkManagerCommands.h>

namespace NE {
namespace Model {
namespace Operations {

using namespace NE::Common;

class SetClockSourceOperation: public NE::Model::Operations::EngineOperation {

    public:

        Address32 neighborAddress;

        Uint8 flags;

        C971_WriteNeighbourPropertyFlag_RespCodes responseCode;

    public:

        SetClockSourceOperation(Address32 owner, Address32 neighborAddress, Uint8 flags);

        virtual ~SetClockSourceOperation();

        void toStringInternal(std::ostringstream &stream);

        std::string getName() {
            return "Set Clock Source";
        }

        NE::Model::Operations::EngineOperationType::EngineOperationTypeEnum getOperationType();

        bool accept(NE::Model::Operations::IEngineOperationsVisitor& visitor);

        virtual void setResponseCode(Uint8 responseCode);

        virtual Uint8 getResponseCode();

};

}
}
}

#endif /* SETCLOCKSOURCEOPERATION_H_ */
