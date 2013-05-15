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
 * UnboundResolveOperations.h
 *
 *  Created on: Nov 9, 2009
 *      Author: andrei.petrut
 */

#ifndef UNBOUNDRESOLVEOPERATIONS_H_
#define UNBOUNDRESOLVEOPERATIONS_H_

#include <Model/Operations/EngineOperations.h>
#include <Model/NetworkEngine.h>
#include <Common/NETypes.h>

#include <boost/bind.hpp>

#include "../CommonData.h"
#include "../operations/WHOperationQueue.h"

namespace hart7 {
namespace nmanager {

/**
 * Handler for confirm of operations from the queue. Handles timeouts or errors.
 * Use this default handler, that does nothing on confirm, in case you are not interested on the responses
 * (for instance if the flow is finished upon confirmation of the operations).
 */
class UnboundResolveOperations
{

    public:

        UnboundResolveOperations(CommonData& commonData_, operations::WHOperationQueue& operationsQueue_) :
            commonData(commonData_), operationsQueue(operationsQueue_)
        {

        }

        UnboundResolveOperations(const UnboundResolveOperations& other) :
            commonData(other.commonData), operationsQueue(other.operationsQueue)
        {
        }

        void ResolveOperations(NE::Model::Operations::EngineOperationsListPointer allOperations,
                               NE::Model::Operations::EngineOperationsListPointer operations)
        {
            if (commonData.networkEngine.resolveOperations(*allOperations, *operations, true))
            {
                CheckRemoveStatus();
            }
            else
            {
                //do nothing
            }
        }

        void CheckRemoveStatus()
        {   // keep copies in case object is destroyed.
            CommonData& commonData_(commonData);
            operations::WHOperationQueue& operationsQueue_(operationsQueue);
            NE::Model::Operations::EngineOperationsListPointer deleteDeviceOperations;

            deleteDeviceOperations.reset(new EngineOperations());
            deleteDeviceOperations->reasonOfOperations = "CheckRemoveStatus()";
            AddressSet removedDevices;
            commonData_.networkEngine.getRemovedDevices(removedDevices);
            operationsQueue_.markRemovedDevices(removedDevices);

            commonData_.networkEngine.checkRemovedDevices(*deleteDeviceOperations, removedDevices);

            if (deleteDeviceOperations->getSize() > 0)
            {
                std::vector<operations::WHOperationPointer> whOperations;
                operations::WHEngineOperationsVisitor visitor(deleteDeviceOperations, commonData_);

                operationsQueue_.generateWHOperations(deleteDeviceOperations, whOperations, visitor);
                uint32_t operationEvent;
                if (operationsQueue_.addOperations(deleteDeviceOperations, whOperations, operationEvent, deleteDeviceOperations->reasonOfOperations,
                                                  NULL,
                                                  boost::bind(&UnboundResolveOperations::ResolveOperations, UnboundResolveOperations(commonData_, operationsQueue_),
                                                              deleteDeviceOperations, _1), false))
                {
                    operationsQueue_.sendOperations();
                }
            }
        }

        void ProcessConfirmedCommand(NE::Model::Operations::EngineOperationsListPointer allOperations, NE::Model::Operations::EngineOperationsListPointer& operations, bool callConfirm)
        {
            ResolveOperations(allOperations, operations);
        }


    private:

        CommonData& commonData;

        operations::WHOperationQueue& operationsQueue;

};

}
}

#endif /* UNBOUNDRESOLVEOPERATIONS_H_ */
