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
 * CheckDevicePresenceFlow.cpp
 *
 *  Created on: Mar 24, 2010
 *      Author: radu pop, anrei petrut, mihai stef
 */

#include "CheckDevicePresenceFlow.h"
#include "../../util/ManagerUtils.h"
#include "../FlowHandlers/UnboundResolveOperations.h"
#include "Model/Operations/Join/ChangePriorityEngineOperation.h"
#include "SMState/SMStateLog.h"
#include <boost/bind.hpp>

namespace hart7 {

namespace nmanager {

CheckDevicePresenceFlow::CheckDevicePresenceFlow(CommonData& commonData_,
                                                 operations::WHOperationQueue& operationsQueue_, bool reEvaluateEdge) :
      reEvaluateFailingEdge(reEvaluateEdge), commonData(commonData_), operationsQueue(operationsQueue_)
{
    LOG_TRACE("CheckDevicePresenceFlow()");

    haveError = false;
    stopRequested = false;
}

CheckDevicePresenceFlow::~CheckDevicePresenceFlow()
{
}

void CheckDevicePresenceFlow::checkDevice(uint16_t deviceAddress, uint16_t reportingPeer, uint8_t joinPriority)
{
    LOG_TRACE("checkDevice() for (" << ToStr(deviceAddress) << ")");

    this->deviceAddress = deviceAddress;
    this->reportingPeer = reportingPeer;

    Device& dev = commonData.networkEngine.getDevice(deviceAddress);
//    dev.setAction(NE::Model::DeviceAction::CHECK_PRESENCE);
    dev.setOnEvaluation(true);	// prevent other eval flows

    longAddressDevice = commonData.networkEngine.getAddress64(deviceAddress);

    linkOperations.reset(new EngineOperations());
    std::ostringstream stream;
    stream << "Check device " << ToStr(deviceAddress) << " presence";
    linkOperations->reasonOfOperations = stream.str();
    ///currentFlowState = VisibleEdgeFlowState::Initial;

    // set the device priority
    IEngineOperationPointer operation(new ChangePriorityEngineOperation((uint32_t) deviceAddress, joinPriority));
    operation->setDependency(WaveDependency::FIRST);
    linkOperations->addOperation(operation);

    operations::WHEngineOperationsVisitor visitor(linkOperations, commonData);
    operationsQueue.generateWHOperations(linkOperations, linkWhOperations, visitor);

    // the operations are only into NM => so we have to log them here
    SMState::SMStateLog::logAllInfo(linkOperations->reasonOfOperations);
    SMState::SMStateLog::logOperations(linkOperations->reasonOfOperations, *linkOperations);

    if (operationsQueue.addOperations(linkOperations, linkWhOperations, linkOperationsEvent, linkOperations->reasonOfOperations,
                                      boost::bind(&CheckDevicePresenceFlow::ProcessConfirmedOperations, this, _1), NULL))
    {
        operationsQueue.sendOperations();
    }

    //currentFlowState = VisibleEdgeFlowState::LinkSent;
    //    LOG_TRACE("checkDevice() for (" << ToStr(deviceAddress) << ") <= "
    //                << VisibleEdgeFlowState::getDescription(currentFlowState));
}

void CheckDevicePresenceFlow::DeleteDevice(uint16_t deviceAdress_)
{
    deviceAddress = deviceAdress_;
    RemoveExistingDevice();
}

void CheckDevicePresenceFlow::RemoveExistingDevice()
{
    try
    {
        if (commonData.networkEngine.existsDevice(deviceAddress))
        {
            commonData.networkEngine.setRemoveStatus(deviceAddress);
            CheckRemoveStatus(false);
        }
        else
        {
            LOG_DEBUG("Finished removing existing devices.");
        }
    }
    catch (std::exception& ex)
    {
        LOG_INFO("Join failed for device " << longAddressDevice.toString() << ". Reason='" << ex.what() << "'");
        //OnJoinFinished(false);
    }
}

void CheckDevicePresenceFlow::ProcessConfirmedOperations(bool errorOccured)
{
    LOG_TRACE("ProcessConfirm() for (" << ToStr(deviceAddress) << ") err=" << (errorOccured ? "1" : "0"));

    // the operations are only into NM => so we have to log them here
    SMState::SMStateLog::logAllInfo(linkOperations->reasonOfOperations);
    SMState::SMStateLog::logOperations(linkOperations->reasonOfOperations, *linkOperations);

    if (errorOccured)
    {
        LOG_DEBUG("ProcessConfirm() received with error for device " << ToStr(deviceAddress));
        haveError = true;

        Device& dev = commonData.networkEngine.getDevice(deviceAddress);
//        dev.setAction(NE::Model::DeviceAction::CHECK_PRESENCE_FAIL);
        dev.setOnEvaluation(false);

        RemoveExistingDevice();
    }
    else
    {
        LOG_DEBUG("ProcessConfirm() received with success for " << ToStr(deviceAddress));
        haveError = false;

        Device& dev = commonData.networkEngine.getDevice(deviceAddress);
//        dev.setAction(NE::Model::DeviceAction::CHECK_PRESENCE_SUCCESS);
        dev.setOnEvaluation(false);

        if (reEvaluateFailingEdge)
        {
        	commonData.networkEngine.markFailedEdge(reportingPeer, deviceAddress);
        }
    }

    if (checkDevicePresenceFlowFinishedHandler)
    {
        checkDevicePresenceFlowFinishedHandler(deviceAddress, haveError);
    }
}

void CheckDevicePresenceFlow::CheckRemoveStatus(bool callConfirm)
{
    LOG_DEBUG("Checking device remove status...");

    deleteDeviceOperations.reset(new EngineOperations());
    deleteDeviceOperations->reasonOfOperations = "Checking device remove status...";

    AddressSet removedDevices;
    commonData.networkEngine.getRemovedDevices(removedDevices);
    operationsQueue.markRemovedDevices(removedDevices);

    commonData.networkEngine.checkRemovedDevices(*deleteDeviceOperations, removedDevices);


     if (deleteDeviceOperations->getSize() > 0)
     {
         operations::WHOperationQueue::OperationsCompletedHandler handler;
         std::vector<operations::WHOperationPointer> whOperations;
         operations::WHEngineOperationsVisitor visitor(deleteDeviceOperations, commonData);

         operationsQueue.generateWHOperations(deleteDeviceOperations, whOperations, visitor);
         if (callConfirm)
         {
             handler = boost::bind(&CheckDevicePresenceFlow::ResolveOperationsDelete, this, deleteDeviceOperations,
                                   callConfirm);
         }
         else
         {
             handler = boost::bind(&UnboundResolveOperations::ResolveOperations,
                                   UnboundResolveOperations(commonData, operationsQueue), deleteDeviceOperations, deleteDeviceOperations);
         }

         if (operationsQueue.addOperations(deleteDeviceOperations, whOperations, deleteDeviceOperationEvent, deleteDeviceOperations->reasonOfOperations,
                                           NULL, handler, false))
         {
             operationsQueue.sendOperations();
         }
     }
    else
    {
        ResolveOperations(deleteDeviceOperations, callConfirm);
    }
}

void CheckDevicePresenceFlow::ResolveOperations(NE::Model::Operations::EngineOperationsListPointer operations,
                                                bool callConfirm)
{
    if (commonData.networkEngine.resolveOperations(*operations, *operations, false) && !stopRequested)
    {
        CheckRemoveStatus(callConfirm);
    }
}

void CheckDevicePresenceFlow::ResolveOperationsDelete(NE::Model::Operations::EngineOperationsListPointer operations,
                                                bool callConfirm)
{
    if ((commonData.networkEngine.resolveOperations(*operations, *operations, true) && !stopRequested))
    {
        CheckRemoveStatus(callConfirm);
    }
}


void CheckDevicePresenceFlow::StopFlow()
{
    haveError = true;
    stopRequested = true;

    checkDevicePresenceFlowFinishedHandler.clear();
}

}

}
