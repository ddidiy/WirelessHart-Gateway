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
 * AlarmDispatcher.h
 *
 *  Created on: Mar 8, 2010
 *      Author: radu pop
 */

#ifndef ALARMDISPATCHER_H_
#define ALARMDISPATCHER_H_

#include "../AllNetworkManagerCommands.h"
#include <list>
#include <WHartStack/WHartStack.h>
#include "AlarmListener.h"
#include "Common/NEAddress.h"
#include <nlib/log.h>

namespace hart7 {

namespace nmanager {

/**
 * Used to register classes that want to be informed about alarms received from the field.
 * There are different alarm types.
 *
 * @author Radu Pop
 */
class AlarmDispatcher
{
        LOG_DEF("h7.n.a.AlarmDispatcher")
        ;

    private:

        typedef std::pair<uint16_t, uint16_t> AlarmSource;

        // we could have more than 1 listener for an alarm source
        std::multimap<AlarmSource, AlarmListenerPointer> alarm788Listeners;

        // we could have more than 1 listener for an alarm source
        std::list<AlarmListenerPointer> generalAlarmListeners;

    public:

        AlarmDispatcher();

        virtual ~AlarmDispatcher();

        /**
         * Register a listener for 788 alarms for the given source and destination.
         */
        void registerAlarm788Listener(AlarmListenerPointer listener, uint16_t source, uint16_t destination);

        /**
         *
         */
        void registerGeneralAlarm788Listener(AlarmListenerPointer listener);

        /**
         * Remove a listener for 788 alarms for the given source and destination.
         */
        void removeAlarm788Listener(AlarmListenerPointer listener, uint16_t source, uint16_t destination);

        /**
         * When a new 788 alarm appears dispatches the alarm to the interesting parties.
         */
        void dispatchAlarm788(uint16_t source, C788_AlarmPathDown_Resp& alarmPathDown);

        /**
         * When a new 789 alarm appears dispatches the alarm to the interesting parties.
         */
        void dispatchAlarm789(uint16_t source, C789_AlarmSourceRouteFailed_Resp& alarmSourceRouteFailed);

        /**
         * When a new 789 alarm appears dispatches the alarm to the interesting parties.
         */
        void dispatchAlarm790(uint16_t source, C790_AlarmGraphRouteFailed_Resp& alarmGraphRouteFailed);

        /**
         * When a new 789 alarm appears dispatches the alarm to the interesting parties.
         */
        void dispatchAlarm791(uint16_t source, C791_AlarmTransportLayerFailed_Resp& alarmTransportLayerFailed);


        /**
         * To string method.
         */
        std::string toString()
        {
            std::ostringstream stream;
            std::multimap<AlarmSource, AlarmListenerPointer>::iterator it;
            it = this->alarm788Listeners.begin();

            stream << "alarm788Listeners: {";
            for (; it != this->alarm788Listeners.end(); ++it)
            {
                stream << "[(" << ToStr(it->first.first) << ", " << ToStr(it->first.second) << "), ";
                stream << it->second->toStringAlarmListener() << "] ";
            }

            stream << "}";

            return stream.str();
        }
};

}

}

#endif /* ALARMDISPATCHER_H_ */
