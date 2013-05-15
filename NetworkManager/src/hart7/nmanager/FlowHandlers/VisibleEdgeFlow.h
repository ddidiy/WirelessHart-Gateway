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
 * VisibleEdgeFlow.h
 *
 *  Created on: Mar 9, 2010
 *      Author: radu
 */

#ifndef VISIBLEEDGEFLOW_H_
#define VISIBLEEDGEFLOW_H_

#include <boost/function.hpp>
#include <time.h>

#include "../IResponseProcessor.h"
#include "../CommonData.h"
#include "../operations/WHOperationQueue.h"
#include "../alarms/AlarmListener.h"
#include <Common/logging.h>

namespace hart7 {

namespace nmanager {

namespace VisibleEdgeFlowState {
enum VisibleEdgeFlowStateEnum
{
    Initial, //
    LinkSent, // the link sent to the device; when they confirm then moves to next state
    Waiting, // it waits the timeout interval time; when the time interval passed without any alarm then success;
    RemoveLinksSent, // the remove links were sent
    Success, // the links have removed
    Failed
};

inline std::string getDescription(VisibleEdgeFlowStateEnum value)
{
    if (value == Initial)
    {
        return "Initial";
    }
    else if (value == LinkSent)
    {
        return "LinkSent";
    }
    else if (value == Waiting)
    {
        return "Waiting";
    }
    else if (value == RemoveLinksSent)
    {
        return "RemoveLinksSent";
    }
    else if (value == Success)
    {
        return "Success";
    }
    else if (value == Failed)
    {
        return "Failed";
    }
    else
    {
        throw NE::Common::NEException("VisibleEdgeFlowState");
    }
}
}

class DevicesManager;

/**
 * Verifies if an edge is viable.
 * Sends a link into the field. After they confirm, waits for TimeOut seconds to see
 * if there are any alarms from the source of the link.
 */
class VisibleEdgeFlow: public AlarmListener, public IResponseProcessor
{
        LOG_DEF("h7.n.VisibleEdgeFlow")
        ;

    public:

        typedef boost::function3<void, uint16_t, uint16_t, bool> VisibleEdgeFinishedHandler;

    private:

        uint16_t source;

        uint16_t destination;

        uint8_t rsl;

        DevicesManager& devicesManager;

        CommonData& commonData;

        operations::WHOperationQueue& operationsQueue;

        NE::Model::Operations::EngineOperationsListPointer linkOperations;
        std::vector<operations::WHOperationPointer> linkWhOperations;
        uint32_t linkOperationsEvent;

        NE::Model::Operations::EngineOperationsListPointer removeLinkOperations;
        std::vector<operations::WHOperationPointer> removeLinkWhOperations;
        uint32_t removeLinkOperationsEvent;

        // the state of the flow
        VisibleEdgeFlowState::VisibleEdgeFlowStateEnum currentFlowState;

        /**
         * During the time it waits it can received this number of alarms.
         * When an alarm is received the TimeOut is reseted and the number of allowed
         * alarms is reduced by one.
         */
        uint8_t noAllowedAlarms;

        /**
         * The start time when the edge is under watch. If from activeTime to (activeTime + TimeOut)
         * there is no alarm received from the source then the edge is considered to be a valid candidate.
         */
        time_t activeTime;

        bool haveError;

        bool stopRequested;

    public:

        VisibleEdgeFinishedHandler visibleEdgeFinishedHandler;

        // the flow timeout; when it expires it is considered success
        uint32_t TimeOut;

    public:

        VisibleEdgeFlow(DevicesManager& devicesManager, CommonData& commonData,
                        operations::WHOperationQueue& operationsQueue);

        virtual ~VisibleEdgeFlow();

        virtual void ProcessConfirm(stack::WHartHandle requestHandle, const stack::WHartLocalStatus& localStatus,
                                    const stack::WHartCommandList& list);

        virtual void ProcessIndicate(stack::WHartHandle handle, const stack::WHartAddress& src,
                                     stack::WHartPriority priority, stack::WHartTransportType transportType,
                                     const stack::WHartCommandList& list);

        WHartHandle TransmitRequest(const WHartAddress& to, const WHartCommandList& commands,
                                    WHartSessionKey::SessionKeyCode sessionCode);

        void StopFlow();

        void newAlarm788(uint16_t src, C788_AlarmPathDown_Resp& alarmPathDown);

        void newAlarm789(uint16_t src, C789_AlarmSourceRouteFailed_Resp& alarmSourceRouteFailed);

        void newAlarm790(uint16_t src, C790_AlarmGraphRouteFailed_Resp& alarmGraphRouteFailed);

        void newAlarm791(uint16_t src, C791_AlarmTransportLayerFailed_Resp& alarmTransportLayerFailed);

        /**
         * Creates a new link and send it into the field
         */
        void createVisibleEdge(uint16_t source, uint16_t destination, uint8_t rsl);

        /**
         * Removed the links that have been added to ensure the devices are visible.
         */
        void removeTestLinks();

        /**
         * Returns true if the flow has time outed the active state in which it waits for alarms.
         * (Didn't receive an alarm)
         */
        bool hasTimeOuted();

        /**
         * Returns true if the links were removed with success.
         */
        bool hasRemovedTestLinks()
        {
            return this->currentFlowState == VisibleEdgeFlowState::Success;
        }

        /**
         * The flow has ended. Do the final job.
         */
        void doFinalJob(bool status);

        uint8_t getRsl()
        {
            return this->rsl;
        }

        inline std::string toString()
        {
            std::stringstream str;
            str << "(" << ToStr(source) << ", " << ToStr(destination) << ")";
            str << ", status: " << VisibleEdgeFlowState::getDescription(currentFlowState);
            if (this->currentFlowState == VisibleEdgeFlowState::Waiting)
            {
                str << ", time : " << std::dec << (int) (time(NULL) - this->activeTime);
            }
            return str.str();
        }

        std::string toStringAlarmListener()
        {
            std::ostringstream stream;
            stream << "VisibleEdgeFlow (" << ToStr(source) << ", " << ToStr(destination) << ")";

            return stream.str();
        }

    private:

        /**
         * Callback called when the operations sent into the field confirm.
         */
        void ProcessConfirmedOperations(bool errorOccured);

};

typedef boost::shared_ptr<VisibleEdgeFlow> VisibleEdgeFlowPointer;

}

}

#endif /* VISIBLEEDGEFLOW_H_ */
