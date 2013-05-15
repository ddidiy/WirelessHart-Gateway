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
 * SuperframeAddedOperation.h
 *
 *  Created on: Sep 17, 2009
 *      Author: ioan.pocol
 */

#ifndef SUPERFRAMEADDEDOPERATION_H_
#define SUPERFRAMEADDEDOPERATION_H_

#include "Model/Operations/EngineOperation.h"
#include "Common/NETypes.h"

#include <ApplicationLayer/Model/WirelessNetworkManagerCommands.h>

namespace NE {
namespace Model {
namespace Operations {

using namespace NE::Common;

class SuperframeAddedOperation: public NE::Model::Operations::EngineOperation {

    public:

        Uint8 superframeId;

        Uint16 superframeLength;

        Uint8 superframeMode;

        C965_WriteSuperframe_RespCodes responseCode;

    public:

        SuperframeAddedOperation(Address32 owner, Uint8 superframeId, Uint16 superframeLength, Uint8 superframeMode);

        virtual ~SuperframeAddedOperation();

        void toStringInternal(std::ostringstream &stream);

        std::string getName() {
            return "Superframe Add";
        }

        NE::Model::Operations::EngineOperationType::EngineOperationTypeEnum getOperationType();

        bool accept(NE::Model::Operations::IEngineOperationsVisitor& visitor);

        virtual void setResponseCode(Uint8 responseCode);

        virtual Uint8 getResponseCode();

};

}
}
}

#endif /* SUPERFRAMEADDEDOPERATION_H_ */
