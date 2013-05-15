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
 * DevicesJoinPriorityFlow.cpp
 *
 *  Created on: Dec 15, 2009
 *      Author: Radu Pop, Andrei Petrut
 */

#include "DevicesJoinPriorityFlow.h"
#include "Model/Operations/Join/ChangePriorityEngineOperation.h"
#include <ApplicationLayer/Binarization/GatewayCommands.h>
#include <ApplicationLayer/Model/GatewayCommands.h>
#include "../operations/WHOperationQueue.h"
#include "SMState/SMStateLog.h"
#include <hart7/util/NMLog.h>
#include <WHartStack/WHartStack.h>
#include <boost/bind.hpp>

namespace hart7 {

namespace nmanager {

DevicesJoinPriorityFlow::DevicesJoinPriorityFlow(CommonData& commonData_) :
    commonData(commonData_)
{
}

void DevicesJoinPriorityFlow::ProcessConfirmedOperations(bool errorOccured)
{
    ProcessConfirm(0, errorOccured ? stack::WHartLocalStatus::whartlsError_Start
                : stack::WHartLocalStatus::whartlsSuccess, WHartCommandList());
}

void DevicesJoinPriorityFlow::ProcessConfirm(WHartHandle requestHandle, const WHartLocalStatus& localStatus,
                                             const WHartCommandList& list)
{
    // do nothing
}

void DevicesJoinPriorityFlow::ProcessIndicate(WHartHandle handle, const WHartAddress& src, WHartPriority priority,
                                              WHartTransportType transportType, const WHartCommandList& list)
{
    // do nothing
}

DevicesJoinPriorityFlow::~DevicesJoinPriorityFlow()
{
}

void DevicesJoinPriorityFlow::markJoinPriorityForDeviceAsChanged(Uint16 address, uint8_t joinPriority)
{
    LOG_TRACE("markJoinPriorityForDeviceAsChanged() " << ToStr(address) << ", joinPriority= " << (int) joinPriority);
    changedDevicesList[address] = joinPriority;
}

void DevicesJoinPriorityFlow::updateDevices(hart7::nmanager::operations::WHOperationQueue& queue)
{
    if (changedDevicesList.size() == 0)
    {
        return;
    }

    LOG_TRACE("updateDevices() : updateDevices.size() = " << (int) changedDevicesList.size());

    NE::Model::Operations::EngineOperationsListPointer c811Operations;
    c811Operations.reset(new EngineOperations());
    c811Operations->reasonOfOperations = "Change devices priority";

    std::vector<operations::WHOperationPointer> c811WhOperations;

    DevicesTable& deviceTable = commonData.networkEngine.getDevicesTable();
    std::map<Uint16, uint8_t> canNotSend;
    for (std::map<Uint16, uint8_t>::iterator itDev = changedDevicesList.begin(); itDev
                != this->changedDevicesList.end(); ++itDev)
    {
        if (deviceTable.existsDevice(itDev->first) && deviceTable.getDevice(itDev->first).status
                    == DeviceStatus::OPERATIONAL)
        {
            IEngineOperationPointer operation(new ChangePriorityEngineOperation(itDev->first, itDev->second));
            operation->setDependency(WaveDependency::FIRST);
            c811Operations->addOperation(operation);
        }
        else
        {
            canNotSend[itDev->first] = itDev->second;
        }
    }

    if (c811Operations->getEngineOperations().size() == 0) {
        // there are no 811 operations
        return;
    }

    operations::WHEngineOperationsVisitor visitor(c811Operations, commonData);
    queue.generateWHOperations(c811Operations, c811WhOperations, visitor);

    SMState::SMStateLog::logOperations(c811Operations->reasonOfOperations, *c811Operations);

    uint32_t c811OperationsEvent;
    if (queue.addOperations(c811Operations, c811WhOperations, c811OperationsEvent, c811Operations->reasonOfOperations,
                            boost::bind(&DevicesJoinPriorityFlow::ProcessConfirmedOperations, this, _1), NULL))
    {
        queue.sendOperations();
    }

    // delete all the notified addresses
    changedDevicesList.clear();

    for (std::map<Uint16, uint8_t>::iterator itDev = canNotSend.begin(); itDev != canNotSend.end(); ++itDev)
    {
        changedDevicesList[itDev->first] = itDev->second;
    }
}

}

}
