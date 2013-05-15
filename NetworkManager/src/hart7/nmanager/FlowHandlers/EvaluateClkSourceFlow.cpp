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
 * EvaluateClkSourceFlow.cpp
 *
 *  Created on: Sep 15, 2010
 *      Author: Mihai.Stef
 */

#include "EvaluateClkSourceFlow.h"
#include  <Common/NETypes.h>
#include <vector>
#include <boost/bind.hpp>
#include <SMState/SMStateLog.h>

namespace hart7 {
namespace nmanager {

EvaluateClkSourceFlow::EvaluateClkSourceFlow(CommonData& commonData_, operations::WHOperationQueue& operationsQueue_) :
    commonData(commonData_), operationsQueue(operationsQueue_)
{
    engineOps.reset(new NE::Model::Operations::EngineOperations());
}

EvaluateClkSourceFlow::~EvaluateClkSourceFlow()
{
}

void EvaluateClkSourceFlow::evaluateClkSrc()
{
    NE::Model::Topology::SubnetTopology& subnetTopology = commonData.networkEngine.getSubnetTopology();

    for (int i = 0; i < commonData.settings.maxEvaluations; i++)
    {
        Address32 address = subnetTopology.popOnEvaluationNodes();
        if (address != 0)
        {
            commonData.networkEngine.evaluateClockSource(*engineOps, address);
            SendOperations(engineOps);
            engineOps.reset(new NE::Model::Operations::EngineOperations());
        }
    }
}

void EvaluateClkSourceFlow::SendOperations(NE::Model::Operations::EngineOperationsListPointer operations)
{
    operations::WHEngineOperationsVisitor visitor(operations, commonData);
    std::vector<operations::WHOperationPointer> whartOperations;

    operationsQueue.generateWHOperations(operations, whartOperations, visitor);

    uint32_t eventOperation = 0;
    if (operationsQueue.addOperations(operations, whartOperations, eventOperation, operations->reasonOfOperations,
                                      boost::bind(&EvaluateClkSourceFlow::ProcessConfirm, this, operations, _1), NULL))
    {
        operationsQueue.sendOperations();
    }
}

void EvaluateClkSourceFlow::ProcessConfirm(NE::Model::Operations::EngineOperationsListPointer operations, bool hasError)
{
    timeoutAddresses.clear();
    if (hasError)
    {
        NE::Model::Topology::SubnetTopology& subnetTopology = commonData.networkEngine.getSubnetTopology();

        for (NE::Model::Operations::EngineOperationsVector::iterator it = operations->operations.begin(); it
                    != operations->operations.end(); ++it)
        {
            //if operation is confirmed with timeout check presence flow called
            if ((*it)->getErrorCode() == NE::Model::Operations::ErrorCode::TIMEOUT)
            {
                timeoutAddresses.insert((*it)->getOwner());
                LOG_DEBUG("Timeout Detected for device " << ToStr((*it)->getOwner()));
            }

            //reset sec clk src in the model if the operations are not confirmed with success
            if ((*it)->getErrorCode() == NE::Model::Operations::ErrorCode::ERROR)
            {
                if (((boost::shared_ptr<SetClockSourceOperation>&) (*it))->flags == 1)
                {
                    subnetTopology.getNode((*it)->getOwner()).resetSecondaryClkSource();
                    subnetTopology.pushOnEvaluationNodes((*it)->getOwner());
                }
                if (((boost::shared_ptr<SetClockSourceOperation>&) (*it))->flags == 0)
                {
                    subnetTopology.getNode((*it)->getOwner()).setSecondaryClkSource(((boost::shared_ptr<
                                SetClockSourceOperation>&) (*it))->neighborAddress);
                    subnetTopology.pushOnEvaluationNodes((*it)->getOwner());
                }
            }
        }
    }
    std::ostringstream reason;
    reason << "Resolve operations: ";
    reason << operations->reasonOfOperations;

    SMState::SMStateLog::logOperations(reason.str(), *operations);
}

void EvaluateClkSourceFlow::CheckDevicePresences()
{
    //HACK:[andy] - keep a copy as apparently the timeoutAddresses can be modified while iterating on it.
    std::set<uint32_t> internalTimeoutAddesses = timeoutAddresses;
    for (std::set<uint32_t>::iterator it = internalTimeoutAddesses.begin(); it != internalTimeoutAddesses.end(); ++it)
    {
        LOG_DEBUG("Checking device presence for address=" << ToStr(*it));
        CheckDevicePresenceFlowPointer flow(new CheckDevicePresenceFlow(commonData, operationsQueue, false));
        flow->checkDevicePresenceFlowFinishedHandler = boost::bind(&EvaluateClkSourceFlow::DevicePresenceFlowFinished,
                                                                   this, *it, _1);
        devicePresenceFlow.push_back(flow);
        flow->checkDevice(*it, //  device address
                          0,   // reporting peer, not used
                          15); // join priority, set to max in this case
    }
}

void EvaluateClkSourceFlow::DevicePresenceFlowFinished(uint16_t address, bool hasError)
{
    LOG_DEBUG("Finished Check Device Presence flow for address=" << ToStr(address));
    for (CheckDevicePresencesList::iterator it = devicePresenceFlow.begin(); it != devicePresenceFlow.end(); ++it)
    {
        if ((*it)->deviceAddress == address)
        {
            devicePresenceFlow.erase(it);
            break;
        }
    }
}

}
}
