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
 * RequestServiceFlowHandler.cpp
 *
 *  Created on: Sep 17, 2009
 *      Author: Andy
 */

#include "RequestServiceFlowHandler.h"

#include <Model/Services/Service.h>
#include <Model/Services/ServiceTypes.h>
#include <ApplicationLayer/Model/NetworkLayerCommands.h>
#include <boost/bind.hpp>

#include "../operations/WHEngineOperationsVisitor.h"

namespace hart7 {
namespace nmanager {

using namespace hart7::stack;

bool RequestServiceFlowHandler::DelayedCommandHandler::HandleCommand(const WHartAddress& src,
                                                                     const stack::WHartCommand& request,
                                                                     WHartCommandWrapper& response_)
{
    LOG_DEBUG("RequestServiceFlowHandler::DelayedCommandHandler::HandleCommand 799");

    if ((request.commandID == CMDID_C799_RequestService) && (src == source))
    {
        C799_RequestService_Req *c799 = (C799_RequestService_Req*) request.command;
        if (c799->m_ucServiceId == serviceRequest.m_ucServiceId && c799->m_tPeriod.u32 == serviceRequest.m_tPeriod.u32
                    && c799->m_unNicknameOfPeer == serviceRequest.m_unNicknameOfPeer && c799->m_ucServiceRequestFlags
                    == serviceRequest.m_ucServiceRequestFlags && c799->m_ucServiceApplicationDomain
                    == serviceRequest.m_ucServiceApplicationDomain)
        {
            if (CreateServiceCallback)
            {
                LOG_DEBUG(
                          "RequestServiceFlowHandler::DelayedCommandHandler::HandleCommand  799 CreateServiceCallback() ");
                CreateServiceCallback();
            }

            if (finishRequested) // ready to respond
            {
                LOG_DEBUG("RequestServiceFlowHandler::DelayedCommandHandler::HandleCommand 799 finishRequested ");
                if (finishCallback)
                {
                    finishCallback(response_);
                }
                finished = true;
            }
            else // not ready to respond, send Delayed Response Running
            {
                LOG_DEBUG("Service not ready yet. Delayed response running...");
                response_.commandBuffer.reset(new uint8_t[256]);

                // not needed since DR_RUNNING is an error code and these will not be serialized
                C799_RequestService_Resp* response = (C799_RequestService_Resp*) response_.commandBuffer.get();

                response->m_ucServiceRequestFlags = serviceRequest.m_ucServiceRequestFlags;
                response->m_ucServiceApplicationDomain = serviceRequest.m_ucServiceApplicationDomain;
                response->m_unNicknameOfPeer = serviceRequest.m_unNicknameOfPeer;
                response->m_tPeriod.u32 = serviceRequest.m_tPeriod.u32;
                response->m_ucServiceId = serviceRequest.m_ucServiceId;
                response->m_ucRouteId = 0;

                response_.command.commandID = CMDID_C799_RequestService;
                response_.command.responseCode = RCS_E34_DelayedResponseRunning;
                response_.command.command = (void*) response_.commandBuffer.get();
            }

            return true;
        }
    }
    return false;
}

bool RequestServiceFlowHandler::DelayedCommandHandler::CommandHandlerFinished()
{
    return finished;
}

void RequestServiceFlowHandler::DelayedCommandHandler::FinishCommand(OnFinishedCallback callback)
{
    LOG_DEBUG("RequestServiceFlowHandler::DelayedCommandHandler::FinishCommand 799");
    finishCallback = callback;
    finishRequested = true;
}

RequestServiceFlowHandler::RequestServiceFlowHandler(CommonData& commondata_,
                                                     const RegisterDefaultHandlerCallback& registerDefault_) :
    commondata(commondata_), registerDefault(registerDefault_)
{
}

void RequestServiceFlowHandler::CreateDelayedResponseInitiate(WHartCommandWrapper& response_)
{
    LOG_DEBUG("Creating Service and Delayed Response Initiate. 799");
    response_.commandBuffer.reset(new uint8_t[256]);
	// not needed since DR_INIT is an error code and these will not be serialized.
    C799_RequestService_Resp* response = (C799_RequestService_Resp*) response_.commandBuffer.get();

    response_.command.commandID = CMDID_C799_RequestService;
    response_.command.responseCode = RCS_E33_DelayedResponseInitiated;
    response->m_ucServiceRequestFlags = serviceRequest.m_ucServiceRequestFlags;
    response->m_ucServiceApplicationDomain = serviceRequest.m_ucServiceApplicationDomain;
    response->m_unNicknameOfPeer = serviceRequest.m_unNicknameOfPeer;
    response->m_tPeriod.u32 = GetProcessedPeriod(serviceRequest.m_tPeriod.u32);
    response->m_ucServiceId = serviceRequest.m_ucServiceId;
    response->m_ucRouteId = 0;
    response_.command.command = (void*) response_.commandBuffer.get();
}

NE::Model::Services::ApplicationDomain::ApplicationDomainEnum Convert(ServiceApplicationDomain appDomain)
{
    switch (appDomain)
    {
        case ServiceApplicationDomain_Publish:
            return NE::Model::Services::ApplicationDomain::PUBLISH;
        case ServiceApplicationDomain_Event:
            return NE::Model::Services::ApplicationDomain::EVENT;
        case ServiceApplicationDomain_Maintenance:
            return NE::Model::Services::ApplicationDomain::MAINTENANCE;
        case ServiceApplicationDomain_BlockTransfer:
            return NE::Model::Services::ApplicationDomain::BLOCK_TRANSFFER;
        default:
            return NE::Model::Services::ApplicationDomain::MAINTENANCE;
    }
}

void RequestServiceFlowHandler::CreateServiceWithPeer()
{
    LOG_DEBUG("Create service with peer=" << ToStr(serviceRequest.m_unNicknameOfPeer) << ": reqSId=" << std::hex
                << (int) serviceRequest.m_ucServiceId);
    NE::Model::Services::Service service;
    service.address = src.address.nickname;
    service.peerAddress = serviceRequest.m_unNicknameOfPeer;
    service.applicationDomain = Convert(serviceRequest.m_ucServiceApplicationDomain);

    service.source = serviceRequest.m_ucServiceRequestFlags & ServiceRequestFlagsMask_Source;
    service.sink = serviceRequest.m_ucServiceRequestFlags & ServiceRequestFlagsMask_Sink;
    service.intermittent = serviceRequest.m_ucServiceRequestFlags & ServiceRequestFlagsMask_Intermittent;

    service.management = service.applicationDomain == NE::Model::Services::ApplicationDomain::MAINTENANCE; //TODO check
    service.period = GetProcessedPeriod(serviceRequest.m_tPeriod.u32);

    if (service.address == NetworkManager_Nickname() || service.address == Gateway_Nickname())
    {
    	service.requestServiceId = (((uint32_t) service.peerAddress) << 16) + serviceRequest.m_ucServiceId;
    }
    else
    {
    	service.requestServiceId = (((uint32_t) service.address) << 16) + serviceRequest.m_ucServiceId;
    }

    commondata.networkEngine.createService(service, false);

    if (!service.isRequestPending())
    {
        LOG_DEBUG("Finished Create Service. Sending OK... 799");
        routeId = service.routeId;
        OnServiceCreated();
    }
}

uint32_t RequestServiceFlowHandler::GetProcessedPeriod(uint32_t period)
{
    if (period < commondata.settings.minServicePeriod * 32)
    {
    	LOG_INFO("Increasing requested service period from " << (period / 32) << " to " << commondata.settings.minServicePeriod);
    	return commondata.settings.minServicePeriod * 32;
    }
    return period;
}

void RequestServiceFlowHandler::OnServiceCreated()
{
    LOG_DEBUG("RequestServiceFlowHandler::OnServiceCreated(): 799");
    delayCommandHandler->FinishCommand(boost::bind(&RequestServiceFlowHandler::FinalMessageReceived, this, _1));
}

void RequestServiceFlowHandler::FinalMessageReceived(WHartCommandWrapper& response_)
{
    LOG_DEBUG("RequestServiceFlowHandler::FinalMessage");
    response_.commandBuffer.reset(new uint8_t[256]);
    C799_RequestService_Resp* response = (C799_RequestService_Resp*) response_.commandBuffer.get();

    response->m_ucServiceRequestFlags = serviceRequest.m_ucServiceRequestFlags;
    response->m_ucServiceApplicationDomain = serviceRequest.m_ucServiceApplicationDomain;
    response->m_unNicknameOfPeer = serviceRequest.m_unNicknameOfPeer;
    response->m_tPeriod.u32 = GetProcessedPeriod(serviceRequest.m_tPeriod.u32);
    response->m_ucServiceId = serviceRequest.m_ucServiceId;
    response->m_ucRouteId = routeId & 0xFF;

    response_.command.commandID = CMDID_C799_RequestService;
    response_.command.responseCode = 0;
    response_.command.command = (void*) response_.commandBuffer.get();
}

void RequestServiceFlowHandler::RegisterDefaultHandler(const stack::WHartAddress& src_,
                                                       const C799_RequestService_Req& request)
{
    delayCommandHandler.reset(new DelayedCommandHandler());
    delayCommandHandler->source = src_;
    delayCommandHandler->serviceRequest = request;
    delayCommandHandler->CreateServiceCallback = boost::bind(&RequestServiceFlowHandler::CreateServiceWithPeer, this);

    if (registerDefault)
    {
        registerDefault(delayCommandHandler);
    }
}

void RequestServiceFlowHandler::ProcessRequest(const stack::WHartAddress& src_, const C799_RequestService_Req& request,
                                               WHartCommandWrapper& response)
{
    src = src_;
    serviceRequest = request;

    CreateDelayedResponseInitiate(response);

    RegisterDefaultHandler(src_, serviceRequest);

    CreateServiceWithPeer();
}

}
}
