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
 * WriteSessionOperation.h
 *
 *  Created on: Oct 30, 2009
 *      Author: Andy
 */

#ifndef WRITESESSIONOPERATION_H_
#define WRITESESSIONOPERATION_H_

#include <WHartStack/WHartTypes.h>
#include <WHartStack/WHartStack.h>
#include "Model/Operations/EngineOperation.h"
#include "Model/SecurityKey.h"
#include "Common/logging.h"

namespace NE {
namespace Model {
namespace Operations {

using namespace hart7::stack;

class WriteSessionOperation: public EngineOperation {

    public:

        Uint32 peerNonceCounterValue;

        Address32 peerNickname;

        WHartUniqueID peerUniqueID;

        NE::Model::SecurityKey key;

        WHartSessionKey::SessionKeyCode sessionType;

        WriteSessionOperation(Address32 owner, Uint32 peerNonceCounterValue, Address32 peerNickname, WHartUniqueID peerUniqueID,
                    const NE::Model::SecurityKey& key, WHartSessionKey::SessionKeyCode sessionType);

        virtual ~WriteSessionOperation() {
        }

        EngineOperationType::EngineOperationTypeEnum getOperationType();

        std::string getName() {
            return "Write Session";
        }

        void toStringInternal(std::ostringstream &stream);

        virtual void setResponseCode(Uint8 responseCode);

        virtual Uint8 getResponseCode();

        bool accept(NE::Model::Operations::IEngineOperationsVisitor& visitor);
};

}
}
}
#endif /* WRITESESSIONOPERATION_H_ */
