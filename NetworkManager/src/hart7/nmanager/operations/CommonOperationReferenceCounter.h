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
 * CommonOperationReferenceCounter.h
 *
 *  Created on: Aug 12, 2009
 *      Author: Andy
 */

#ifndef COMMONOPERATIONREFERENCECOUNTER_H_
#define COMMONOPERATIONREFERENCECOUNTER_H_

#include <Model/Operations/EngineOperations.h>

using namespace hart7::stack;
using namespace NE::Model::Operations;

namespace hart7 {
namespace nmanager {
namespace operations {
//DEPRECATED
/**
 * Used when more WHartOperations are generated from an EngineOperation. Sets the engine operation state to Confirmed when
 * all WHartOperations are confirmed.
 */
class CommonOperationReferenceCounter
{
    public:

        /**
         * The engine operation corresponding to the WH operations.
         */
        IEngineOperationPointer engineOperationPointer;

    private:

        int referenceCount;

    public:

        void setOperation(IEngineOperationPointer engineOperationPointer_)
        {
            engineOperationPointer = engineOperationPointer_;
            referenceCount = 0;
        }

        void setOperationStatus(OperationState::OperationStateEnum operationStatus)
        {
            if (operationStatus == OperationState::SENT)
            {
                referenceCount++;
                if (engineOperationPointer->getState() != OperationState::CONFIRMED)
                {
                    engineOperationPointer->setState(OperationState::SENT);
                }
            }
            else if (operationStatus == OperationState::CONFIRMED)
            {
                referenceCount--;
                if (referenceCount == 0)
                {
                    engineOperationPointer->setState(OperationState::CONFIRMED);
                }
            }
        }

};

}
}
}

#endif /* COMMONOPERATIONREFERENCECOUNTER_H_ */
