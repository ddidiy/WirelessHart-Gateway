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
 * WriteKeyOperation.h
 *
 *  Created on: Oct 30, 2009
 *      Author: Andy
 */

#ifndef WRITEKEYOPERATION_H_
#define WRITEKEYOPERATION_H_

#include "Model/Operations/EngineOperation.h"
#include "Model/SecurityKey.h"
#include "Common/logging.h"

namespace NE {
namespace Model {
namespace Operations {

class WriteKeyOperation: public EngineOperation {

    public:

        NE::Model::SecurityKey key;

        WriteKeyOperation(Address32 owner, const NE::Model::SecurityKey& key);

        virtual ~WriteKeyOperation() {
        }

        EngineOperationType::EngineOperationTypeEnum getOperationType();

        std::string getName() {
            return "Write Key";
        }

        void toStringInternal(std::ostringstream &stream);

        virtual void setResponseCode(Uint8 responseCode);

        virtual Uint8 getResponseCode();

        bool accept(NE::Model::Operations::IEngineOperationsVisitor& visitor);

};

}
}
}
#endif /* WRITEKEYOPERATION_H_ */
