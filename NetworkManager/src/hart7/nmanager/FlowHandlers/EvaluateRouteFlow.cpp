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
 * EvaluateRouteFlow.cpp
 *
 *  Created on: Sep 2, 2009
 *      Author: Andrei Petrut
 */
#include "EvaluateRouteFlow.h"
#include <boost/bind.hpp>
#include <Common/NETypes.h>
#include <set>

namespace hart7 {
namespace nmanager {

EvaluateRouteFlow::EvaluateRouteFlow(CommonData& commonData_, operations::WHOperationQueue& operationsQueue_) :
    commonData(commonData_), operationsQueue(operationsQueue_), isEvaluating(false)
{
    engineOperations.reset(new NE::Model::Operations::EngineOperations());
    confirmedCount = -1;
}

void EvaluateRouteFlow::EvaluateRoutes()
{
    for (int i = 0; i < commonData.settings.maxEvaluationsStartedAtOnce; i++)
    {
        engineOperations.reset(new NE::Model::Operations::EngineOperations());
        if (commonData.networkEngine.evaluateNextPath(*engineOperations))
        {
            LOG_TRACE("Evaluated route. Sending operations...");
            SendOperations(engineOperations);
        }
        else
        {
            break;  // no need trying 50 times if there is no route to be evaluated.
        }
    }
}

void EvaluateRouteFlow::SendOperations(NE::Model::Operations::EngineOperationsListPointer operations)
{
    operations::WHEngineOperationsVisitor visitor(operations, commonData);
    std::vector<operations::WHOperationPointer> whartOperations;

    operationsQueue.generateWHOperations(operations, whartOperations, visitor);

    uint32_t eventOperation = 0;
    if (operationsQueue.addOperations(operations, whartOperations, eventOperation, operations->reasonOfOperations,
                                      NULL,
                                      boost::bind(&EvaluateRouteFlow::ProcessTimeouts, this, operations, _1, false)))
    {
        operationsQueue.sendOperations();
    }
}

void EvaluateRouteFlow::CheckRemoveStatus()
{
    LOG_DEBUG("Checking device remove status...");
    deleteDeviceOperations.reset(new NE::Model::Operations::EngineOperations());

    confirmedCount = 0;
    AddressSet removedDevices;
    commonData.networkEngine.getRemovedDevices(removedDevices);
    operationsQueue.markRemovedDevices(removedDevices);

    commonData.networkEngine.checkRemovedDevices(*deleteDeviceOperations, removedDevices);


     if (deleteDeviceOperations->getSize() > 0)
     {
         std::vector<operations::WHOperationPointer> whOperations;
         operations::WHEngineOperationsVisitor visitor(deleteDeviceOperations, commonData);

         operationsQueue.generateWHOperations(deleteDeviceOperations, whOperations, visitor);
         if (operationsQueue.addOperations(deleteDeviceOperations, whOperations, deleteDeviceOperationEvent, deleteDeviceOperations->reasonOfOperations,
                                           boost::bind(&EvaluateRouteFlow::ResolveOperationsDeleteEvent, this, deleteDeviceOperations, _1),
                                           boost::bind(&EvaluateRouteFlow::ResolveOperationsDelete, this,
                                                       deleteDeviceOperations, _1, false),
                                           false))
         {
             operationsQueue.sendOperations();
         }
     }
    else
    {
        ResolveOperations(deleteDeviceOperations, deleteDeviceOperations, true);
    }
}

void EvaluateRouteFlow::ResolveOperationsDeleteEvent(NE::Model::Operations::EngineOperationsListPointer operations, bool hasError)
{
}

void EvaluateRouteFlow::ResolveOperationsDelete(NE::Model::Operations::EngineOperationsListPointer allOps,
                                                NE::Model::Operations::EngineOperationsListPointer operations, bool hasError)
{
    if (commonData.networkEngine.resolveOperations(*allOps, *operations, true))
    {
        CheckRemoveStatus();
    }
    else
    {
        // will be done on confirmEvent
    }
}


void EvaluateRouteFlow::ResolveOperations(NE::Model::Operations::EngineOperationsListPointer allOps,
                                          NE::Model::Operations::EngineOperationsListPointer operations, bool hasError)
{
    if (commonData.networkEngine.resolveOperations(*allOps, *operations, true))
    {
        CheckRemoveStatus();
    }
    else
    {
        bool isLast = true;
        for (EngineOperationsVector::iterator it = allOps->getEngineOperations().begin();
                    it != allOps->getEngineOperations().end(); ++it)
        {
            if  ((*it)->getState() != NE::Model::Operations::OperationState::CONFIRMED &&
                        (*it)->getState() != NE::Model::Operations::OperationState::SENT_IGNORED)
            {
                isLast = false;
                break;
            }
        }

        if (isLast)
        {
            LOG_TRACE("Route finished evaluating.");
            if (RouteEvaluated)
            {
                RouteEvaluated();
            }
        }
    }
}

void EvaluateRouteFlow::ProcessTimeouts(NE::Model::Operations::EngineOperationsListPointer allOps,
                                        NE::Model::Operations::EngineOperationsListPointer operations, bool hasError)
{
    timeoutAddresses.clear();
    for (NE::Model::Operations::EngineOperationsVector::iterator it = operations->operations.begin(); it
                != operations->operations.end(); ++it)
    {
        if ((*it)->getErrorCode() == NE::Model::Operations::ErrorCode::TIMEOUT)
        {
            timeoutAddresses.insert((*it)->getOwner());
            LOG_DEBUG("Timeout Detected for device " << ToStr((*it)->getOwner()));
        }
    }

    ResolveOperations(allOps, operations, hasError);
}

void EvaluateRouteFlow::CheckDevicePresences()
{
    LOG_ERROR("k7t: devicePresenceFlow.size()= " << (int)devicePresenceFlows.size());

    std::set<uint32_t> checkDevPreFloAddresses;
    for (CheckDevicePresencesList::iterator it = devicePresenceFlows.begin(); it != devicePresenceFlows.end(); ++it)
    {
        checkDevPreFloAddresses.insert((*it)->deviceAddress);
    }

    //HACK:[andy] - keep a copy as apparently the timeoutAddresses can be modified while iterating on it.
    std::set<uint32_t> internalTimeoutAddresses = timeoutAddresses;
    for (std::set<uint32_t>::iterator it = internalTimeoutAddresses.begin(); it != internalTimeoutAddresses.end(); ++it)
    {
        if (checkDevPreFloAddresses.find(*it) != checkDevPreFloAddresses.end()) {
            // there is already a flow for this address
            continue;
        }

        LOG_DEBUG("Checking device presence for address=" << ToStr(*it));
        CheckDevicePresenceFlowPointer flow(new CheckDevicePresenceFlow(commonData, operationsQueue, false));
        flow->checkDevicePresenceFlowFinishedHandler = boost::bind(&EvaluateRouteFlow::DevicePresenceFlowFinished,
                                                                   this, _1, _2);
        devicePresenceFlows.push_back(flow);
        flow->checkDevice(*it, //  device address
                          0, // reporting peer, not used
                          15); // join priority, set to max in this case
    }
}

void EvaluateRouteFlow::DevicePresenceFlowFinished(uint16_t address, bool hasError)
{
    LOG_DEBUG("Finished Check Device Presence flow for address=" << ToStr(address));
    for (CheckDevicePresencesList::iterator it = devicePresenceFlows.begin(); it != devicePresenceFlows.end(); ++it)
    {
        if ((*it)->deviceAddress == address)
        {
            devicePresenceFlows.erase(it);
            break;
        }
    }

    if (devicePresenceFlows.empty()) // finished
    {
        LOG_DEBUG("Finished Check Device Presence Flows...");
        if (RouteEvaluated)
        {
            RouteEvaluated();
        }
    }
}

}
}
