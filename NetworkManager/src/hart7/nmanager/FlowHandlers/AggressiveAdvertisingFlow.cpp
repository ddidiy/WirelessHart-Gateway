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
 * AggressiveAdvertisingFlow.cpp
 *
 *  Created on: Jun 22, 2010
 *      Author: radu marginean
 */

#include "AggressiveAdvertisingFlow.h"

namespace hart7 {

namespace nmanager {

const uint32_t ALLOCATION_HANDLER = 0xFFFFFFFF;


AggressiveAdvertisingFlow::AggressiveAdvertisingFlow(IRequestSend& requestSend_,
                                           CommonData& commonData_, operations::WHOperationQueue& operationsQueue_) :
      FlowHandler(requestSend_, commonData_, operationsQueue_)
{
    aggressiveAdvTimerInterval = 250;
    defaultAdvTimerInterval = 900;
    remainingTimer = 1000 * 60 * 30;
}

AggressiveAdvertisingFlow::~AggressiveAdvertisingFlow()
{
}

void AggressiveAdvertisingFlow::ProcessConfirm(WHartHandle requestHandle, const WHartLocalStatus& localStatus,
                                            const WHartCommandList& list)
{
    // do nothing
}

void AggressiveAdvertisingFlow::ProcessIndicate(WHartHandle handle, const WHartAddress& src, WHartPriority priority,
                                             WHartTransportType transportType, const WHartCommandList& list)
{
    // do nothing
}

void AggressiveAdvertisingFlow::ProcessConfirmedOperations(bool errorOccured)
{
    if (errorOccured)
    {
        remainingTimer = 0;
        StopAggressiveAdvertising();
    }
}

bool AggressiveAdvertisingFlow::ChangeTrancieverAddvInterval (uint32_t timerInterval, bool agressive)
{
    DevicesTable& devicesTable = NetworkEngine::instance().getDevicesTable();
    Devices32Container& devices = devicesTable.getDevices();
    NE::Model::Tdma::SubnetTdma& tdma = NetworkEngine::instance().getSubnetTdma();

    NE::Model::Operations::EngineOperationsListPointer transcieversOperations;
    transcieversOperations.reset(new EngineOperations());
    if (agressive)
    {
        transcieversOperations->reasonOfOperations = "Start aggressive advertisment";
    }
    else
    {
        transcieversOperations->reasonOfOperations = "Stop aggressive advertisment";
    }

    std::vector<operations::WHOperationPointer> whartOperations;

    for (Devices32Container::iterator it = devices.begin(); it != devices.end(); ++it)
    {
        if ((it->second).capabilities.isBackbone())
        {
            IEngineOperationPointer operation(new WriteTimerIntervalOperation(it->first, timerInterval, WirelessTimerCode_Advertisment));
            operation->setDependency(WaveDependency::FIRST);
            transcieversOperations->addOperation(operation);

            if (agressive)
            {
                tdma.broadcastAllocation(*transcieversOperations, ALLOCATION_HANDLER, it->first, PublishPeriod::P_250_MS,
                                         true, false, LinkTypes::NORMAL);
            }
            else
            {
                tdma.broadcastDeallocation(*transcieversOperations, ALLOCATION_HANDLER, it->first, LinkTypes::NORMAL);
            }
        }
    }

    if (transcieversOperations->getEngineOperations().size() == 0)
    {
            return false;
    }

    uint32_t whartOperationsEvent;
    SendOperations(transcieversOperations, whartOperationsEvent, true, operations::OperationDependencyType::None);

    return true;
}

bool AggressiveAdvertisingFlow::InitiateAggresiveAdvertising (uint32_t intervalMsecs)
{
    LOG_TRACE("StartAggressiveAdv()");

    remainingTimer = intervalMsecs;
    return ChangeTrancieverAddvInterval (aggressiveAdvTimerInterval, true);
}

bool AggressiveAdvertisingFlow::StopAggressiveAdvertising()
{
    LOG_TRACE("StopAggressiveAdv()");

    return ChangeTrancieverAddvInterval (defaultAdvTimerInterval, false);
}

void AggressiveAdvertisingFlow::TimePassed(uint32_t timeMsec)
{
    if (remainingTimer > 0)
    {
        if (remainingTimer > timeMsec)
        {
            remainingTimer -= timeMsec;
        }
        else
        {
            remainingTimer = 0;
            StopAggressiveAdvertising();
        }
    }
}


}

}
