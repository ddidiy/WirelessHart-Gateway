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
 * WHOperationQueueLogger.h
 *
 *  Created on: Mar 30, 2010
 *      Author: radu
 */

#ifndef WHOPERATIONQUEUELOGGER_H_
#define WHOPERATIONQUEUELOGGER_H_

#include <nlib/log.h>
#include <Common/NETypes.h>
#include "WHOperation.h"
#include <set>
#include <iomanip>

namespace hart7 {

namespace nmanager {

namespace operations {

struct EventDataLog
{
    public:

        uint32_t eventId;
        std::string eventName;
        time_t dateSent;
        time_t dateConfirmed;
        std::set<uint16_t> operationIds; // all the operations from the event
        std::set<uint16_t> packetHandles; // the packet handles that contain at least one operation from this event

        friend std::ostream& operator<<(std::ostream& str, EventDataLog& event);
};

/**
 * Responsible with logging of information about a new event.
 */
struct LogQueueEvent
{
        LOG_DEF("h7.n.o.LogQueue")
        ;

        void logEvent(EventDataLog& event)
        {
            std::ostringstream str;
            str << "evId=" << std::setw(6) << (int) event.eventId;
            str << "; DT=" << std::setw(4) << (int) (event.dateConfirmed - event.dateSent);
            str << "; opsNo=" << std::setw(3) << (int) event.operationIds.size();
            str << "; pcksNo=" << std::setw(2) << (int) event.packetHandles.size();
            str << "; pckHandles:";
            std::set<uint16_t>::iterator itHandle = event.packetHandles.begin();
            for (; itHandle != event.packetHandles.end(); ++itHandle)
            {
                str << " " << (*itHandle);
            }

            str << "; ops:";
            std::set<uint16_t>::iterator itOp = event.operationIds.begin();
            for (; itOp != event.operationIds.end(); ++itOp)
            {
                str << " " << (*itOp);
            }

            str << "; " << event.eventName.substr(0, 50);

            LOG_INFO(str.str());
        }

};

/**
 * Responsible with logging of every new event, sent command, confirmed command and confirmed event.
 */
struct LogQueue
{
    public:
        LOG_DEF("h7.n.o.WHOperationQueue")
        ;

        LogQueueEvent logQueueEvent;

        std::map<uint32_t, EventDataLog> events;

        struct Packet
        {
            public:
                WHartHandle handle;
                WHartAddress address;
                std::set<uint16_t> operationIds;
                time_t dateSent;
                time_t dateConfirmed;
        };

        //typedef uint16_t WHartHandle;
        std::map<WHartHandle, Packet> packets;

    public:

        void logNewEvent(uint32_t currentEventIndex, std::string& eventName,
                         std::vector<WHOperationPointer>& operations)
        {
            if (operations.size() == 0)
            {
                return;
            }

            std::ostringstream str;
            str << "logNewEvent() : evId=" << (int) currentEventIndex;
            str << "; op[Cmd]Ids: ";

            std::set<uint16_t> opIds;
            std::vector<WHOperationPointer>::iterator it = operations.begin();
            for (; it != operations.end(); ++it)
            {
                opIds.insert((*it)->getEngineOperation()->getOperationId());
                str << (int) (*it)->getEngineOperation()->getOperationId() << "[";
                str << (int) (*it)->getWHartCommand().commandID << "] ";
            }

            EventDataLog event;
            event.eventId = currentEventIndex;
            event.eventName = eventName;
            event.dateSent = time(NULL);
            event.dateConfirmed = time(NULL);
            event.operationIds = opIds;

            events.insert(std::make_pair(currentEventIndex, event));

            str << "; " << event.eventName; //.substr(0, 50);

            LOG_INFO(str.str());
        }

        void logConfirmEvent(uint32_t eventId, bool hasError)
        {
            std::map<uint32_t, EventDataLog>::iterator itEvent = events.find(eventId);
            if (itEvent == events.end())
            {
                LOG_ERROR("logConfEvent() : there is no evId=" << (int) eventId);
                return;
            }

            std::ostringstream str;
            str << "logConfEvent() : evId=" << (int) eventId;
            itEvent->second.dateConfirmed = time(NULL);

            str << ";  noPcks=" << (int) (itEvent->second.packetHandles.size());
            str << ";  DT=" << (int) (itEvent->second.dateConfirmed - itEvent->second.dateSent);
            if (hasError == true)
            {
                str << "; ERR";
            }

            LOG_INFO(str.str());

            logQueueEvent.logEvent(itEvent->second);

            events.erase(itEvent);
        }

        void logSendPacket(WHartAddress& address, WHartHandle& handle, std::list<WHOperationPointer>& operations)
        {
            std::ostringstream str;
            str << "logSendPacket() : > " << address;
            str << ", handle=" << (int) handle;
            str << ", op[Ev]Ids: ";

            std::set<uint16_t> opIds;
            std::list<WHOperationPointer>::iterator it = operations.begin();
            for (; it != operations.end(); ++it)
            {
                opIds.insert((*it)->getEngineOperation()->getOperationId());
                str << (int) (*it)->getEngineOperation()->getOperationId();
                str << "[" << (int) (*it)->getOperationEvent() << "] ";

                // update the event's noPackets
                std::map<uint32_t, EventDataLog>::iterator itEv = events.find((*it)->getOperationEvent());
                if (itEv != events.end())
                {
                    itEv->second.packetHandles.insert(handle);
                }
                else
                {
                    LOG_ERROR("logSendPacket() : there is no evId=" << (int) (*it)->getOperationEvent());
                }
            }

            Packet packet;
            packet.handle = handle;
            packet.address = address;
            packet.dateSent = time(NULL);
            packet.dateConfirmed = time(NULL);
            packet.operationIds = opIds;

            packets.insert(std::make_pair(handle, packet));

            LOG_INFO(str.str());
        }

        void logConfirmPacket(WHartHandle& handle, WHartLocalStatus localStatus,
                              std::list<WHOperationPointer>& operations, const WHartCommandList& responses)
        {
            std::ostringstream str;
            str << "logConfPacket() : < ";
            if (operations.size() > 0)
            {
                str << operations.front()->getDestinationAddress();
            }
            str << ", handle=" << (int) handle;
            str << ", opIds: ";

            std::list<WHOperationPointer>::iterator it = operations.begin();
            for (; it != operations.end(); ++it)
            {
                str << (int) (*it)->getEngineOperation()->getOperationId() << " ";
                str << "[" << (int) (*it)->getOperationEvent() << "] ";
            }

            std::map<WHartHandle, Packet>::iterator itPack = packets.find(handle);
            if (itPack == packets.end())
            {
                LOG_ERROR("logConfPacket() : there is no packet with handle : " << (int) handle);
                return;
            }

            itPack->second.dateConfirmed = time(NULL);

            str << ";  DT=" << (int) (itPack->second.dateConfirmed - itPack->second.dateSent);
            LOG_INFO(str.str());

            packets.erase(itPack);
        }
};


}

}

}

#endif /* WHOPERATIONQUEUELOGGER_H_ */
