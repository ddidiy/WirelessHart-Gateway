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
 * NodeAlarmVerifier.h
 *
 *  Created on: Mar 23, 2010
 *      Author: radu pop, andrei petrut, mihai stef
 */

#ifndef NODEALARMVERIFIER_H_
#define NODEALARMVERIFIER_H_

#include <boost/function.hpp>
#include <time.h>
#include <map>

#include "AlarmListener.h"
#include "AlarmDispatcher.h"
#include "../CommonData.h"
#include "../operations/WHOperationQueue.h"
#include "CheckDevicePresenceFlow.h"
#include <Common/logging.h>
#include <stdint.h>

namespace hart7 {

namespace nmanager {

/**
 * Responsible with handling of alarms and starting different flows for each case.
 */
class NodeAlarmVerifier: public AlarmListener
{
        LOG_DEF("h7.n.NodeAlarmVerifier")
        ;
// At this time, only the Path Down alarm is handled, and a CheckDevicePresence flow is started for each such alarm.
    public:

        /**
         * Constructor.
         */
        NodeAlarmVerifier(IRequestSend& requestSend_, CommonData& commonData_,
                          operations::WHOperationQueue& operationsQueue_);

        virtual ~NodeAlarmVerifier();

        /**
         * Handles a 788 Path Down alarm. For each device that is reported by an alarm, a CheckDevicePresence flow will be started.
         */
        void newAlarm788(uint16_t src, C788_AlarmPathDown_Resp& alarmPathDown);

        /**
         * Handles a 789 Source Route Failed alarm. No handler yet.
         */
        void newAlarm789(uint16_t src, C789_AlarmSourceRouteFailed_Resp& alarmSourceRouteFailed);

        /**
         * Handles a 790 Graph Route Failed alarm. No handler yet.
         */
        void newAlarm790(uint16_t src, C790_AlarmGraphRouteFailed_Resp& alarmGraphRouteFailed);

        /**
         * Handles a 791 Transport Layer Failed alarm. No handler yet.
         */
        void newAlarm791(uint16_t src, C791_AlarmTransportLayerFailed_Resp& alarmTransportLayerFailed);

        // the handler called when a check device flow ends
        void CheckDevicePresenceFlowFinished(uint16_t dest, bool status);

        // Delete a device as if a check failed on it.
        void DeleteDevice(uint16_t device);

        std::string toStringAlarmListener()
        {
            return "NodeAlarmVerifier";
        }

    private:

        IRequestSend& requestSend;

        CommonData& commonData;

        operations::WHOperationQueue& operationsQueue;

        // keeps the time stamps a device is reported as not seen with a 788 alarm
        std::map<uint16_t, std::list<time_t> > alarms;

        std::map<uint16_t, CheckDevicePresenceFlowPointer> checkFlows;


};

typedef boost::shared_ptr<NodeAlarmVerifier> NodeAlarmVerifierPointer;
}

}

#endif /* NODEALARMVERIFIER_H_ */
