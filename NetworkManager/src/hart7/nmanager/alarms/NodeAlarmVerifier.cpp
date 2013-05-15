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
 * NodeAlarmVerifier.cpp
 *
 *  Created on: Mar 23, 2010
 *      Author: radu pop, andrei petrut, mihai stef
 */

#include "NodeAlarmVerifier.h"
#include <boost/bind.hpp>

namespace hart7 {

namespace nmanager {

NodeAlarmVerifier::NodeAlarmVerifier(IRequestSend& requestSend_, CommonData& commonData_,
                                     operations::WHOperationQueue& operationsQueue_) :
    requestSend(requestSend_), commonData(commonData_), operationsQueue(operationsQueue_)
{
}

NodeAlarmVerifier::~NodeAlarmVerifier()
{
}

void NodeAlarmVerifier::DeleteDevice(uint16_t device)
{
    std::map<uint16_t, CheckDevicePresenceFlowPointer>::iterator it = checkFlows.find(device);
    if (it != checkFlows.end())
    {
        LOG_INFO("There is already a flow for device " << ToStr(device) << ". Stopping...");
        it->second->StopFlow();

        checkFlows.erase(it);
    }

    CheckDevicePresenceFlowPointer cdpfp(new CheckDevicePresenceFlow(commonData, operationsQueue, false));
    this->checkFlows.insert(std::make_pair(device, cdpfp));

    cdpfp->checkDevicePresenceFlowFinishedHandler = boost::bind(&NodeAlarmVerifier::CheckDevicePresenceFlowFinished,
                                                                this, _1, _2);
    cdpfp->DeleteDevice(device);
}

void NodeAlarmVerifier::newAlarm788(uint16_t src, C788_AlarmPathDown_Resp& alarmPathDown)
{
    uint16_t dest = alarmPathDown.Nickname;
    LOG_TRACE("newAlarm788 received for (" << ToStr(src) << ", " << ToStr(dest) << ")");

    if (commonData.settings.activateDeviceAlarmCheck == false)
    {
        return;
    }

    // check if destination and source exists and both are OPERATIONAL
    if (!commonData.networkEngine.existsDevice(src))
    {
        LOG_DEBUG("newAlarm788 received from an inexistent device: " << ToStr(src));
        return;
    }

    if (!commonData.networkEngine.existsDevice(dest))
    {
        LOG_DEBUG("newAlarm788 received for an inexistent device: " << ToStr(dest));
        return;
    }

    if (commonData.networkEngine.getDevice(dest).status != NE::Model::DeviceStatus::OPERATIONAL
                || commonData.networkEngine.getDevice(src).status != NE::Model::DeviceStatus::OPERATIONAL)
    {
        LOG_DEBUG("newAlarm788 received for a not OPERATIONAL device: " << ToStr(dest));
        return;
    }


    commonData.networkEngine.newAlarm788(src, dest);

    if (commonData.networkEngine.getDevice(dest).capabilities.isBackbone())
    {
        LOG_TRACE("newAlarm788 ignore alarm for AccessPoint!");
        return;
    }

    if (commonData.networkEngine.getSubnetTopology().isEdgeFailing(src, dest))
    {
        LOG_DEBUG("Edge from " << ToStr(src) << " to " << ToStr(dest) << " is already marked as failing.");
        return;
    }

    if (checkFlows.find(dest) != checkFlows.end())
    {
        LOG_INFO("There is already a flow for device " << ToStr(dest));
        return;
    }

    time_t currentTime = time(NULL);
    this->alarms[dest].push_back(currentTime);

    // 1. remove the first old alarms (outside the time interval)
    do
    {
        if (currentTime - this->alarms[dest].front() > commonData.settings.alarmsTimeIntervalBeforeCheckDevice)
        {
            LOG_DEBUG("remove old alarm");
            this->alarms[dest].pop_front();
        }
        else
        {
            break;
        }
    } while (this->alarms.size() > 0);

    // 2. if there are still the max alarms then start a check flow
    if (this->alarms[dest].size() >= commonData.settings.maxAlarmsNoBeforeCheckDevice)
    {
        LOG_DEBUG("maxAlarmsNoBeforeCheckDevice(" << (int)commonData.settings.maxAlarmsNoBeforeCheckDevice
                    << ") reached in alarmsTimeIntervalBeforeCheckDevice");
        CheckDevicePresenceFlowPointer cdpfp(new CheckDevicePresenceFlow(commonData, operationsQueue, true));
        this->checkFlows.insert(std::make_pair(dest, cdpfp));

        cdpfp->checkDevicePresenceFlowFinishedHandler
                    = boost::bind(&NodeAlarmVerifier::CheckDevicePresenceFlowFinished, this, _1, _2);
        cdpfp->checkDevice(dest, src,
                           commonData.networkEngine.getDevice(dest).getMetaDataAttributes()->getJoinPriority());
    }
}

void NodeAlarmVerifier::newAlarm789(uint16_t src, C789_AlarmSourceRouteFailed_Resp& alarmSourceRouteFailed)
{
    // do nothing
}

void NodeAlarmVerifier::newAlarm790(uint16_t src, C790_AlarmGraphRouteFailed_Resp& alarmGraphRouteFailed)
{
    // do nothing
}

void NodeAlarmVerifier::newAlarm791(uint16_t src, C791_AlarmTransportLayerFailed_Resp& alarmTransportLayerFailed)
{
    // do nothing
}

void NodeAlarmVerifier::CheckDevicePresenceFlowFinished(uint16_t dest, bool haveError)
{
    LOG_TRACE("CheckDevicePresenceFlowFinished()");

    if (haveError == false)
    {
        LOG_DEBUG("CheckDevicePresenceFlowFinished() : device responded  => device " << ToStr(dest) << " ok");

    }
    else
    {
        LOG_DEBUG("CheckDevicePresenceFlowFinished() : device did not respond  => remove device: " << ToStr(dest));
    }

    // remove flow in both cases
    std::map<uint16_t, CheckDevicePresenceFlowPointer>::iterator itCheck = checkFlows.find(dest);
    if (itCheck != checkFlows.end())
    {
        this->checkFlows.erase(itCheck);
    }
}

}

}
