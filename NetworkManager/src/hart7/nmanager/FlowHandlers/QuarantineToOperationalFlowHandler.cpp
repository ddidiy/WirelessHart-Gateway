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
 * QuarantineToOperationalFlowHandler.cpp
 *
 *  Created on: Sep 3, 2009
 *      Author: andrei.petrut
 */

#include "QuarantineToOperationalFlowHandler.h"
#include "../AllNetworkManagerCommands.h"
#include "../../util/ManagerUtils.h"
#include "../operations/WHOperation.h"
#include "../operations/WHEngineOperationsVisitor.h"
#include "Model/Operations/Join/WriteSessionOperation.h"
#include "SMState/SMStateLog.h"
#include "UnboundResolveOperations.h"

#include <boost/bind.hpp>

namespace hart7 {
namespace nmanager {

QuarantineToOperationalFlowHandler::QuarantineToOperationalFlowHandler(const HartDevice& device_,
                                                                       CommonData& commonData_,
                                                                       operations::WHOperationQueue& operationsQueue_) :
    device(device_), commonData(commonData_), operationsQueue(operationsQueue_)
{
    flowState = Initial;
    status = true;
    error = false;
    confirmedCount = -1;
}

void QuarantineToOperationalFlowHandler::Start()
{
    LOG_INFO("Attempting to put device " << WHartAddress(device.longAddress) << " to OPERATIONAL");
    flowState = Initial;
    ContinueFlow(false);
}

void QuarantineToOperationalFlowHandler::ContinueFlow(bool hasError)
{
    if (!hasError && !error)
    {
        if (flowState == Initial && !device.isBackbone)
        {
            LOG_DEBUG("Flow=SendAdvertiseLinks ");
            flowState = SentAdvertiseLinks;
            SendAdvertiseLinks();
        }
        else if ((flowState == SentAdvertiseLinks) || ((flowState == Initial) && device.isBackbone))
        {
            LOG_DEBUG("Flow=SentSession");
            flowState = SentSession;
            SendDeviceSessionWithGW();
        }
        else if (flowState == SentSession)// && !device.isBackbone)
        {
            LOG_DEBUG("Flow=SentService");

            flowState = SentService;
            SendDeviceServiceWithGW();
        }
        else if (flowState == SentService)// || (flowState == SentSession && device.isBackbone))
        {
            LOG_DEBUG("Flow=Finished");

            //Finished
            if (OnFinished)
            {
                OnFinished(device, status);
            }
        }
        else
        {
            LOG_ERROR("Unexpected flow state in Q2OFlowHandler. value=" << (int) flowState);
        }
    }
    else
    {
        // just in case there are other operations that will confirm after this
        LOG_ERROR("An error occurred when sending device to OPERATIONAL.");
        if (flowState == SentAdvertiseLinks)
        {
            operationsQueue.CancelOperations(advLinksHandle);
        }

        if (OnFinished)
            OnFinished(device, false);
    }
}

void QuarantineToOperationalFlowHandler::CheckRemoveStatus()
{
    LOG_DEBUG("Checking device remove status...");
    deleteDeviceOperations.reset(new NE::Model::Operations::EngineOperations());
    AddressSet removedDevices;
    commonData.networkEngine.getRemovedDevices(removedDevices);
    operationsQueue.markRemovedDevices(removedDevices);

    commonData.networkEngine.checkRemovedDevices(*deleteDeviceOperations, removedDevices);
    confirmedCount = 0;
    bool callConfirm = false;

    if (deleteDeviceOperations->getSize() > 0)
    {
        std::vector<operations::WHOperationPointer> whOperations;
        operations::WHEngineOperationsVisitor visitor(deleteDeviceOperations, commonData);
        operations::WHOperationQueue::OperationsCompletedHandler handler;
        operations::WHOperationQueue::OperationsConfirmedHandler confHandler;

        if (callConfirm)
        {
            handler = boost::bind(&QuarantineToOperationalFlowHandler::ResolveOperationsDelete, this,
                                  deleteDeviceOperations, _1);
            confHandler = boost::bind(&QuarantineToOperationalFlowHandler::ResolveOperationConfirmed, this, deleteDeviceOperations, _1);
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

    }
    else
    {
        ResolveOperations(deleteDeviceOperations, false);
    }
}

void QuarantineToOperationalFlowHandler::ResolveOperations(
                                                           NE::Model::Operations::EngineOperationsListPointer operations,
                                                           bool hasError)
{
    if (hasError)
    {
        status = false;
    }
    else
    {
        ContinueFlow(!status);
    }
}

void QuarantineToOperationalFlowHandler::ResolveOperationConfirmed(NE::Model::Operations::EngineOperationsListPointer allOperations,
                                                                   NE::Model::Operations::EngineOperationsListPointer operations)
{
    if (commonData.networkEngine.resolveOperations(*allOperations, *operations, true))
    {
        status = false;
        CheckRemoveStatus();
    }
}

void QuarantineToOperationalFlowHandler::ResolveOperationsDelete(
                                                           NE::Model::Operations::EngineOperationsListPointer operations,
                                                           bool hasError)
{
    if (hasError)
    {
        status = false;
    }

    if (commonData.networkEngine.resolveOperations(*operations, *operations, true))
    {
         CheckRemoveStatus();
    }
    ContinueFlow(!status);

}

void QuarantineToOperationalFlowHandler::SendAdvertiseLinks()
{
    advertiseOperations.reset(new NE::Model::Operations::EngineOperations());
    if (commonData.networkEngine.allocateAdvertise(*advertiseOperations, device.nickname))
    {
        std::vector<operations::WHOperationPointer> whOperations;
        operations::WHEngineOperationsVisitor visitor(advertiseOperations, commonData);

        operationsQueue.generateWHOperations(advertiseOperations, whOperations, visitor);
        if (operationsQueue.addOperations(advertiseOperations, whOperations, advLinksHandle, advertiseOperations->reasonOfOperations,
                                          boost::bind(&QuarantineToOperationalFlowHandler::ResolveOperations, this,
                                                      advertiseOperations, _1),
                                          boost::bind(&QuarantineToOperationalFlowHandler::ResolveOperationConfirmed, this, advertiseOperations, _1)))
        {
            operationsQueue.sendOperations();
        }
    }
}

void QuarantineToOperationalFlowHandler::SendDeviceSessionWithGW()
{
    NE::Model::SecurityKey
                key =
                            commonData.securityManager.GetSessionKey(
                                                                     hart7::util::getAddress64(
                                                                                               commonData.networkEngine.getDevicesTable(),
                                                                                               device.longAddress), true);
    LOG_DEBUG("key : " << key.toString());


    NE::Model::Operations::EngineOperationsListPointer operations(new NE::Model::Operations::EngineOperations());
    std::ostringstream stream;
    stream << "sendDeviceSessionWithGW for ";
    stream << hart7::util::getAddress64(commonData.networkEngine.getDevicesTable(), device.longAddress).toString();
    operations->reasonOfOperations = stream.str();

    NE::Model::Operations::WriteSessionOperation *sessionDevice =
                new NE::Model::Operations::WriteSessionOperation((Address32) device.nickname, 0,
                                                                 (Address32) hart7::stack::Gateway_Nickname(),
                                                                 hart7::stack::Gateway_UniqueID(), key,
                                                                 WHartSessionKey::sessionKeyed);

    NE::Model::Operations::WriteSessionOperation *sessionGW =
                new NE::Model::Operations::WriteSessionOperation((Address32) hart7::stack::Gateway_Nickname(), 0,
                                                                 (Address32) device.nickname, device.longAddress, key,
                                                                 WHartSessionKey::sessionKeyed);

    NE::Model::Operations::IEngineOperationPointer ssDPtr(sessionDevice);
    NE::Model::Operations::IEngineOperationPointer ssGPtr(sessionGW);
    ssDPtr->setDependency(WaveDependency::FIRST);
    ssGPtr->setDependency(WaveDependency::FIRST);

    operations->addOperation(ssDPtr);
    operations->addOperation(ssGPtr);

    // because the operations are generated here and not in NE, we have to log them here
    SMState::SMStateLog::logOperations(operations->reasonOfOperations, *operations);
    //    SMState::SMStateLog::logAllInfo(operations->reasonOfOperations);

    std::vector<operations::WHOperationPointer> whOperations;
    operations::WHEngineOperationsVisitor visitor(operations, commonData);

    operationsQueue.generateWHOperations(operations, whOperations, visitor);
    if (operationsQueue.addOperations(operations, whOperations, sessionHandle, operations->reasonOfOperations,
                                      boost::bind(&QuarantineToOperationalFlowHandler::SessionWithGWSent, this, _1), NULL))
    {
        operationsQueue.sendOperations();
    }
}

void QuarantineToOperationalFlowHandler::SessionWithGWSent(bool withError)
{
    if (withError)
    {
        NE::Model::NetworkEngine::instance().setRemoveStatus(device.nickname);
        CheckRemoveStatus();

        ContinueFlow(true);
    }
    else
    {
        ContinueFlow(false);
    }
}

void QuarantineToOperationalFlowHandler::SendDeviceServiceWithGW()
{
    NE::Model::Services::Service service;

    service.serviceId = 0;
    service.requestServiceId = (device.nickname << 16) + MANAGEMENT_SERVICE + 1;
    service.address = device.nickname;
    service.peerAddress = Gateway_Nickname();
    service.period = 32 * 1000 * commonData.settings.gatewayBandwidth;
    service.source = true;
    service.sink = true;
    service.intermittent = false;
    service.applicationDomain = ApplicationDomain::MAINTENANCE;

    serviceOperations.reset(new NE::Model::Operations::EngineOperations());

    commonData.networkEngine.createService(service, true);

    LOG_INFO("Sending service between device and GW...");

    ContinueFlow(false);
}

void QuarantineToOperationalFlowHandler::ProcessConfirm(WHartHandle requestHandle, const WHartLocalStatus& localStatus,
                                                        const WHartCommandList& list)
{
    if (commonData.utils.IsFailedResponse(localStatus, list))
    {
        status = false;
    }

    expectedConfirmCount--;
    if (expectedConfirmCount <= 0)
    {
        expectedConfirmCount = 0;
        ContinueFlow(false);
    }
}

void QuarantineToOperationalFlowHandler::ProcessIndicate(WHartHandle handle, const WHartAddress& src,
                                                         WHartPriority priority, WHartTransportType transportType,
                                                         const WHartCommandList& list)
{
    LOG_WARN("Unexpected indicate received in Q2OFlowHandler. from=" << src << " handle=" << handle);
}

void QuarantineToOperationalFlowHandler::StopFlow()
{
    LOG_DEBUG("Stopping Q2A flow for device=" << WHartAddress(device.nickname));
    error = true;

    if (flowState == SentAdvertiseLinks)
    {
        operationsQueue.CancelOperations(advLinksHandle);
    }
    else if (flowState == SentSession)
    {
        operationsQueue.CancelOperations(sessionHandle);
    }
    else if (flowState == SentService)
    {
        //should not be here
        LOG_WARN("StopFlow on Q2O intercepted in SentService. Should not happen.");
    }
}

}
}
