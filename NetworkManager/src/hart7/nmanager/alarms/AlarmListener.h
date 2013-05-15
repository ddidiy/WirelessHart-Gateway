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
 * AlarmListener.h
 *
 *  Created on: Mar 8, 2010
 *      Author: radu pop
 */

#ifndef ALARMLISTENER_H_
#define ALARMLISTENER_H_

#include <stdint.h>
#include "boost/shared_ptr.hpp"
#include <ApplicationLayer/Model/DataLinkLayerCommands.h>
#include <ApplicationLayer/Model/NetworkLayerCommands.h>

namespace hart7 {

namespace nmanager {

/**
 * This should be implemented by classes who want to be notified when an alarm (commands 788, 789, 790 and 791)
 * is received.
 * @author Radu Pop
 */
class AlarmListener
{

    public:

        virtual ~AlarmListener()
        {
        }

        /**
         * Called when a new 788 alarm appears.
         */
        virtual void newAlarm788(uint16_t src, C788_AlarmPathDown_Resp& alarmPathDown) = 0;

        /**
         * Called when a new 789 alarm appears.
         */
        virtual void newAlarm789(uint16_t src, C789_AlarmSourceRouteFailed_Resp& alarmSourceRouteFailed) = 0;

        /**
         * Called when a new 790 alarm appears.
         */
        virtual void newAlarm790(uint16_t src, C790_AlarmGraphRouteFailed_Resp& alarmGraphRouteFailed) = 0;

        /**
         * Called when a new 791 alarm appears.
         */
        virtual void newAlarm791(uint16_t src, C791_AlarmTransportLayerFailed_Resp& alarmTransportLayerFailed) = 0;

        /**
         * Returns a string representation of the listener.
         */
        virtual std::string toStringAlarmListener() = 0;
};

typedef boost::shared_ptr<AlarmListener> AlarmListenerPointer;

}

}

#endif /* ALARMLISTENER_H_ */
