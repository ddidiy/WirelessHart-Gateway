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
 * WriteNicknameOperation.h
 *
 *  Created on: Oct 30, 2009
 *      Author: Andy
 */

#ifndef WRITENICKNAMEOPERATION_H_
#define WRITENICKNAMEOPERATION_H_

#include "Model/Operations/EngineOperation.h"
#include "Common/logging.h"

namespace NE {
namespace Model {
namespace Operations {

class WriteNicknameOperation: public EngineOperation {

    public:

        WriteNicknameOperation(Address32 owner);

        virtual ~WriteNicknameOperation() {
        }

        EngineOperationType::EngineOperationTypeEnum getOperationType();

        std::string getName() {
            return "Write Nick";
        }

        void toStringInternal(std::ostringstream &stream);

        virtual void setResponseCode(Uint8 responseCode);

        virtual Uint8 getResponseCode();

        bool accept(NE::Model::Operations::IEngineOperationsVisitor& visitor);
};

}
}
}
#endif /* WRITENICKNAMEOPERATION_H_ */
