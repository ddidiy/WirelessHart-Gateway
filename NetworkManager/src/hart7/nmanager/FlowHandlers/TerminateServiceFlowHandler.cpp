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

#include "TerminateServiceFlowHandler.h"

#include "Model/Services/SubnetServices.h"
#include "../AllNetworkManagerCommands.h"
#include "hart7/util/NMLog.h"

namespace hart7 {
namespace nmanager {

TerminateServiceFlowHandler::TerminateServiceFlowHandler(CommonData& commonData_) :
    commonData(commonData_)
{

}

void TerminateServiceFlowHandler::ProcessRequest(const stack::WHartAddress& src, const C801_DeleteService_Req& request,
                                                 WHartCommandWrapper& response_)
{
    LOG_DEBUG("ProcessRequest");
    response_.commandBuffer.reset(new uint8_t[256]);
    C801_DeleteService_Resp* response = (C801_DeleteService_Resp*) response_.commandBuffer.get();

    response->m_ucServiceId = request.m_ucServiceId;
    response->m_ucReason = request.m_ucReason;
    response->m_ucNoOfServiceEntriesRemaining = 0;

    if (src.type != WHartAddress::whartaNickname)
    {
        LOG_ERROR("We should get a WHartAddress::whartaNickname address for a 801 command! src : " << src);
        return;
    }

    Address32 address32 = src.address.nickname;
    NE::Model::Services::SubnetServices subnetServices = commonData.networkEngine.getSubnetServices();
    NodeServiceMap& nodeServicesMap = subnetServices.getNodeServiceMap();

    uint32_t engineServiceId = (((uint32_t) address32) << 16) + request.m_ucServiceId;

    if (request.m_ucServiceId > 0x7F)
    {
        response_.command.responseCode = RCS_E16_AccessRestricted;
        response_.command.command = (void*) response_.commandBuffer.get();
        return;
    }

    if (nodeServicesMap.find(address32) == nodeServicesMap.end())
    {
        // the device has no service
        response_.command.responseCode = RCM_E68_DeleteNotAllowed;
        response_.command.command = (void*) response_.commandBuffer.get();
        return;
    }

    if (!subnetServices.existsService(engineServiceId))
    {
        response_.command.responseCode = RCM_E65_EntryNotFound;
        response_.command.command = (void*) response_.commandBuffer.get();
        return;
    }
    //                   C801_E68 = RCM_E68_DeleteNotAllowed

    commonData.networkEngine.terminateService(address32, engineServiceId);

    response->m_ucNoOfServiceEntriesRemaining = nodeServicesMap[address32].getServices().size();

    response_.command.command = (void*) response_.commandBuffer.get();
    response_.command.responseCode = RCS_N00_Success;
}

}
}
