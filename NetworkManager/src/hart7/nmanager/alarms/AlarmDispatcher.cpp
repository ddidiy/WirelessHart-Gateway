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
 * AlarmDispatcher.cpp
 *
 *  Created on: Mar 8, 2010
 *      Author: radu pop
 */

#include "AlarmDispatcher.h"

namespace hart7 {

namespace nmanager {

AlarmDispatcher::AlarmDispatcher()
{

}

AlarmDispatcher::~AlarmDispatcher()
{
}

void AlarmDispatcher::registerAlarm788Listener(AlarmListenerPointer listener, uint16_t source, uint16_t destination)
{
    LOG_TRACE("registerAlarm788Listener() for " << ToStr(source) << ", " << ToStr(destination));
    AlarmSource alarmSrc = std::make_pair(source, destination);

    this->alarm788Listeners.insert(std::make_pair<AlarmSource, AlarmListenerPointer>(alarmSrc, listener));
}

void AlarmDispatcher::registerGeneralAlarm788Listener(AlarmListenerPointer listener)
{
    LOG_TRACE("registerGeneralAlarm788Listener: " << listener->toStringAlarmListener());

    this->generalAlarmListeners.push_back(listener);
}

void AlarmDispatcher::dispatchAlarm788(uint16_t source, C788_AlarmPathDown_Resp& alarmPathDown)
{
    uint16_t destination = alarmPathDown.Nickname;
    AlarmSource alarmSource = std::make_pair(source, destination);

    LOG_TRACE("dispatchAlarm788() src: " << ToStr(alarmSource.first) << ", dest: " << ToStr(alarmSource.second));

    // 1. call specific alarm source listeners
    std::multimap<AlarmSource, AlarmListenerPointer>::iterator it, itlow, itup;
    itlow = this->alarm788Listeners.lower_bound(alarmSource);
    itup = this->alarm788Listeners.upper_bound(alarmSource);

    if (itlow != this->alarm788Listeners.end() && itup != this->alarm788Listeners.end())
    {
        for (it = itlow; it != itup; it++)
        {
            it->second->newAlarm788(alarmSource.first, alarmPathDown);
        }
    }

    // 2. call GENERAL listeners
    std::list<AlarmListenerPointer>::iterator itGenAlarmList = generalAlarmListeners.begin();
    for (; itGenAlarmList != generalAlarmListeners.end(); ++itGenAlarmList)
    {
        (*itGenAlarmList)->newAlarm788(alarmSource.first, alarmPathDown);
    }
}

void AlarmDispatcher::dispatchAlarm789(uint16_t source, C789_AlarmSourceRouteFailed_Resp& alarmSourceRouteFailed)
{
    LOG_TRACE("dispatchAlarm789() src: " << ToStr(source)
                << ", m_unNicknameOfUnreachableNeighbor: "
                << ToStr(alarmSourceRouteFailed.m_unNicknameOfUnreachableNeighbor));

    // call GENERAL listeners
    std::list<AlarmListenerPointer>::iterator itGenAlarmList = generalAlarmListeners.begin();
    for (; itGenAlarmList != generalAlarmListeners.end(); ++itGenAlarmList)
    {
        (*itGenAlarmList)->newAlarm789(source, alarmSourceRouteFailed);
    }
}

void AlarmDispatcher::dispatchAlarm790(uint16_t source, C790_AlarmGraphRouteFailed_Resp& alarmGraphRouteFailed)
{
    LOG_TRACE("dispatchAlarm790() src: " << ToStr(source)
                << ", m_unGraphIdOfFailedRoute: "
                << ToStr(alarmGraphRouteFailed.m_unGraphIdOfFailedRoute));

    // call GENERAL listeners
    std::list<AlarmListenerPointer>::iterator itGenAlarmList = generalAlarmListeners.begin();
    for (; itGenAlarmList != generalAlarmListeners.end(); ++itGenAlarmList)
    {
        (*itGenAlarmList)->newAlarm790(source, alarmGraphRouteFailed);
    }
}

void AlarmDispatcher::dispatchAlarm791(uint16_t source, C791_AlarmTransportLayerFailed_Resp& alarmTransportLayerFailed)
{
    LOG_TRACE("dispatchAlarm791() src: " << ToStr(source)
                << ", m_unNicknameOfUnreachablePeer: "
                << ToStr(alarmTransportLayerFailed.m_unNicknameOfUnreachablePeer));
    // call GENERAL listeners
    std::list<AlarmListenerPointer>::iterator itGenAlarmList = generalAlarmListeners.begin();
    for (; itGenAlarmList != generalAlarmListeners.end(); ++itGenAlarmList)
    {
        (*itGenAlarmList)->newAlarm791(source, alarmTransportLayerFailed);
    }
}

void AlarmDispatcher::removeAlarm788Listener(AlarmListenerPointer listener, uint16_t source, uint16_t destination)
{
    LOG_TRACE("removeAlarm788Listener(" << ToStr(source) << ", " << ToStr(destination) << "), listener: " << listener->toStringAlarmListener());
    LOG_TRACE("removeAlarm788Listener(" << ToStr(source) << ", " << ToStr(destination) << "), listeners: " << toString());
    std::pair<uint16_t, uint16_t> alarmSource = std::make_pair(source, destination);

    std::multimap<AlarmSource, AlarmListenerPointer>::iterator it, itlow, itup;
    itlow = this->alarm788Listeners.lower_bound(alarmSource);
    itup = this->alarm788Listeners.upper_bound(alarmSource);

    if (itlow == this->alarm788Listeners.end())
    {
        LOG_WARN("removeAlarm788Listener(" << ToStr(source) << ", "
                    << ToStr(destination) << "), listener: " << listener->toStringAlarmListener()
                    << " : Trying to unregister a 788 alarm listener that is not registered!");
        return;
    }

    for (it = itlow; it != itup; it++)
    {
        if (it->second == listener)
        {
            LOG_TRACE("delete alarm listener: " << listener->toStringAlarmListener());
            this->alarm788Listeners.erase(it);
            return;
        }
    }
}

}

}
