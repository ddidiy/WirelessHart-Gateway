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
 * ChangePriorityEngineOperation.h
 *
 *  Created on: Dec 16, 2009
 *      Author: Radu Pop
 */

#ifndef CHANGEPRIORITYENGINEOPERATION_H_
#define CHANGEPRIORITYENGINEOPERATION_H_

#include "Model/Operations/EngineOperation.h"
#include <ApplicationLayer/Model/DataLinkLayerCommands.h>

namespace NE {
namespace Model {
namespace Operations {

class ChangePriorityEngineOperation;
typedef boost::shared_ptr<ChangePriorityEngineOperation> ChangePriorityEngineOperationPointer;

class ChangePriorityEngineOperation: public NE::Model::Operations::EngineOperation
{
    private:

        uint8_t joinPriority;

        C811_WriteJoinPriority_RespCodes responseCode;

    public:

        ChangePriorityEngineOperation(Address32 owner_, uint8_t joinPriority_);

        virtual ~ChangePriorityEngineOperation();

        void toStringInternal(std::ostringstream &stream);

        EngineOperationType::EngineOperationTypeEnum getOperationType();

        bool accept(IEngineOperationsVisitor & visitor);

        virtual void setResponseCode(Uint8 responseCode);

        virtual Uint8 getResponseCode();

        std::string getName()
        {
            return "ChangePriority";
        }

        uint8_t getJoinPriority() const
        {
            return joinPriority;
        }

        void setJoinPriority(uint8_t joinPriority_)
        {
            joinPriority = joinPriority_;
        }
};

}
}
}

#endif /* CHANGEPRIORITYENGINEOPERATION_H_ */
