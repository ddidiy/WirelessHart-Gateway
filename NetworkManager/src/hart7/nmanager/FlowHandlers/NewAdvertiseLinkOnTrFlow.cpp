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
 * NewAdvertiseLinkOnTrFlow.cpp
 *
 *  Created on: Jun 22, 2010
 *      Author: radu
 */

#include "NewAdvertiseLinkOnTrFlow.h"
#include <boost/bind.hpp>
#include "SMState/SMStateLog.h"

namespace hart7 {

namespace nmanager {

NewAdvertiseLinkOnTrFlow::NewAdvertiseLinkOnTrFlow(IRequestSend& requestSend_, CommonData& commonData_,
                                                   operations::WHOperationQueue& operationsQueue_) :
    FlowHandler(requestSend_, commonData_, operationsQueue_)
{
}

NewAdvertiseLinkOnTrFlow::~NewAdvertiseLinkOnTrFlow()
{
}

bool NewAdvertiseLinkOnTrFlow::sendNewAdvLink(Address32 brAddress)
{
    LOG_TRACE("sendNewAdvLink()");

    advertiseEngineOperations.reset(new NE::Model::Operations::EngineOperations());
    advertiseEngineOperations->reasonOfOperations = "ADD new adv link on TR.";

    if (commonData.networkEngine.getSubnetTdma().broadcastAllocateNextLink(*advertiseEngineOperations, brAddress))
    {
        advertiseEngineOperations->setRequesterAddress32(brAddress);
        advertiseEngineOperations->setNetworkEngineEventType(NetworkEngineEventType::NONE);

        try
        {
            Device& dev = commonData.networkEngine.getDevice(brAddress);
            dev.setAction(DeviceAction::SERVICE_IN);
        }
        catch (std::exception& ex)
        {
            LOG_WARN("Error while setting device status: " << ex.what());
        }

        // radu : the operations are only into NM => so we have to log them here
        SMState::SMStateLog::logAllInfo(advertiseEngineOperations->reasonOfOperations);
        SMState::SMStateLog::logOperations(advertiseEngineOperations->reasonOfOperations, *advertiseEngineOperations);

        SendOperations(advertiseEngineOperations, lastOperationsHandle, true);

        return true;
    }

    return false;
}

void NewAdvertiseLinkOnTrFlow::sendRemoveAdvLink(Address32 brAddress, uint32_t allocationHandler)
{
    LOG_TRACE("sendRemoveAdvLink()");
    advertiseEngineOperations.reset(new NE::Model::Operations::EngineOperations());
    advertiseEngineOperations->reasonOfOperations = "REMOVE adv link from TR. ";

    commonData.networkEngine.getSubnetTdma().broadcastDeallocation(*advertiseEngineOperations, allocationHandler,
                                                                   brAddress, LinkTypes::NORMAL);

    advertiseEngineOperations->setRequesterAddress32(brAddress);
    advertiseEngineOperations->setNetworkEngineEventType(NetworkEngineEventType::NONE);

    SMState::SMStateLog::logAllInfo(advertiseEngineOperations->reasonOfOperations);
    SMState::SMStateLog::logOperations(advertiseEngineOperations->reasonOfOperations, *advertiseEngineOperations);

    try
    {
        Device& dev = commonData.networkEngine.getDevice(brAddress);
        dev.setAction(DeviceAction::SERVICE_IN);
    }
    catch (std::exception& ex)
    {
        LOG_WARN("Error while setting device status: " << ex.what());
    }

    SendOperations(advertiseEngineOperations, lastOperationsHandle, true);
}

void NewAdvertiseLinkOnTrFlow::ProcessConfirmedOperations(bool errorOccured)
{
    LOG_TRACE("ProcessConfirmedOperations(): k3t errorOcurred: " << errorOccured);

    ResolveOperations(advertiseEngineOperations, advertiseEngineOperations, false);

    if (newAdvertiseLinkOnTrFlowFinished)
    {
        newAdvertiseLinkOnTrFlowFinished(errorOccured);
    }
    else
    {

        LOG_TRACE("ProcessConfirmedOperations(): k3t newAdvertiseLinkOnTrFlowFinished ii null");
    }

}

void NewAdvertiseLinkOnTrFlow::StopFlow()
{
    LOG_TRACE("NewAdvertiseLinkOnTrFlow::StopFlow()");

    operationsQueue.CancelOperations(lastOperationsHandle);
}


}

}
