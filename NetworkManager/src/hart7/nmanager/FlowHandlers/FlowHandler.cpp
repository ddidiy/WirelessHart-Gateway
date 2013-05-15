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
 * FlowHandler.cpp
 *
 *  Created on: Apr 14, 2010
 *      Author: andrei.petrut
 */
#include "FlowHandler.h"
#include "UnboundResolveOperations.h"

namespace hart7 {
namespace nmanager {


FlowHandler::FlowHandler(IRequestSend& requestSend_, CommonData& commonData_, operations::WHOperationQueue& operationsQueue_)
	: requestSend(requestSend_), commonData(commonData_), operationsQueue(operationsQueue_)
{
	stopRequested = false;
	callInnerConfirm = true;
	confirmCalled = false;
	confirmedCount = -1;
}


void FlowHandler::ProcessConfirm(stack::WHartHandle requestHandle, const stack::WHartLocalStatus& localStatus,
                                     const stack::WHartCommandList& list)
{
    //do nothing
}

void FlowHandler::ProcessIndicate(stack::WHartHandle handle, const stack::WHartAddress& src,
                                      stack::WHartPriority priority, stack::WHartTransportType transportType,
                                      const stack::WHartCommandList& list)
{
    // do nothing
}

void FlowHandler::SendOperations(const NE::Model::Operations::EngineOperationsListPointer& operations, uint32_t& handle, bool stopOnError, operations::OperationDependencyType::OperationDependency dependency)
{
    std::vector<operations::WHOperationPointer> whOperations;
    operations::WHEngineOperationsVisitor visitor(operations, commonData);

    operationsQueue.generateWHOperations(operations, whOperations, visitor);
    if (operationsQueue.addOperations(operations, whOperations, handle,
                                      operations->reasonOfOperations,
                                      boost::bind(&FlowHandler::CallbackConfirmOperations, this, _1),
                                      boost::bind(&FlowHandler::ProcessConfirmedCommand, this, operations, _1, true),
                                      stopOnError, dependency))
    {
        operationsQueue.sendOperations();
    }
}

void FlowHandler::CallbackConfirmOperations(bool withError)
{
    if (callInnerConfirm)
          ProcessConfirmedOperations(withError);
    else
    {
        confirmCalled = true;
    }
}

void FlowHandler::ProcessConfirmedCommand(NE::Model::Operations::EngineOperationsListPointer allOperations, NE::Model::Operations::EngineOperationsListPointer& operations, bool callConfirm)
{
    ResolveOperations(allOperations, operations, callConfirm);
}

void FlowHandler::CheckRemoveStatus(bool callConfirm)
{
    LOG_DEBUG("Checking device remove status...");

    confirmedCount = 0;

    NE::Model::Operations::EngineOperationsListPointer deleteDeviceOperations;
    std::vector<operations::WHOperationPointer> deleteDeviceWhOperations;

    deleteDeviceOperations.reset(new EngineOperations());
    deleteDeviceOperations->reasonOfOperations = "Checking device remove status...";
    AddressSet removedDevices;
    commonData.networkEngine.getRemovedDevices(removedDevices);

    callInnerConfirm = false; confirmCalled = false;

    operationsQueue.markRemovedDevices(removedDevices);
    commonData.networkEngine.checkRemovedDevices(*deleteDeviceOperations, removedDevices);

    callInnerConfirm = true;

     if (deleteDeviceOperations->getSize() > 0)
     {
         operations::WHOperationQueue::OperationsCompletedHandler handler;
         operations::WHOperationQueue::OperationsConfirmedHandler confHandler;
         std::vector<operations::WHOperationPointer> whOperations;
         operations::WHEngineOperationsVisitor visitor(deleteDeviceOperations, commonData);

         if (callConfirm)
         {
             handler = boost::bind(&FlowHandler::ResolveOperationsDelete, this, deleteDeviceOperations, deleteDeviceOperations,
                                   callConfirm);
             confHandler = boost::bind(&FlowHandler::ProcessConfirmedCommand, this, deleteDeviceOperations, _1, callConfirm);
         }
         else
         {
             handler = NULL;
             confHandler = boost::bind(&UnboundResolveOperations::ProcessConfirmedCommand, UnboundResolveOperations(commonData, operationsQueue),
                                       deleteDeviceOperations, _1, callConfirm);
         }

         operationsQueue.generateWHOperations(deleteDeviceOperations, whOperations, visitor);
         if (operationsQueue.addOperations(deleteDeviceOperations, whOperations, deleteDeviceOperationEvent, deleteDeviceOperations->reasonOfOperations,
                                           handler, confHandler,
                                           false))
         {
             operationsQueue.sendOperations();
         }

         if (confirmCalled)
             CallbackConfirmOperations(true);

    }
}

void FlowHandler::ResolveOperations(NE::Model::Operations::EngineOperationsListPointer allOperations, NE::Model::Operations::EngineOperationsListPointer operations, bool callConfirm)
{
    if (commonData.networkEngine.resolveOperations(*allOperations, *operations, true) && !stopRequested)
    {
        CheckRemoveStatus(false);
    }
}

void FlowHandler::ResolveOperationsDelete(NE::Model::Operations::EngineOperationsListPointer allOperations, NE::Model::Operations::EngineOperationsListPointer operations, bool callConfirm)
{
    if ((commonData.networkEngine.resolveOperations(*allOperations, *operations, true) && !stopRequested))
    {
        CheckRemoveStatus(false);
    }
}


}
}
