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
 * VisibleEdgeFlow.cpp
 *
 *  Created on: Mar 9, 2010
 *      Author: radu
 */

#include "VisibleEdgeFlow.h"
#include "../../util/ManagerUtils.h"
#include "Model/Operations/Join/ChangePriorityEngineOperation.h"
#include "SMState/SMStateLog.h"
#include <boost/bind.hpp>

namespace hart7 {

namespace nmanager {

VisibleEdgeFlow::VisibleEdgeFlow(DevicesManager& devicesManager_, CommonData& commonData_,
                                 operations::WHOperationQueue& operationsQueue_) :
    devicesManager(devicesManager_), commonData(commonData_),
                operationsQueue(operationsQueue_)
{
    LOG_TRACE("VisibleEdgeFlow()");

    this->haveError = false;
    this->stopRequested = false;
    this->currentFlowState = VisibleEdgeFlowState::Initial;

    this->noAllowedAlarms = commonData.settings.noAllowedAlarms;
    this->TimeOut = commonData.settings.visibileEdgeTimeOut;
}

VisibleEdgeFlow::~VisibleEdgeFlow()
{

}

void VisibleEdgeFlow::ProcessConfirm(stack::WHartHandle requestHandle, const stack::WHartLocalStatus& localStatus,
                                     const stack::WHartCommandList& list)
{
    LOG_TRACE("ProcessConfirm() for (" << ToStr(source) << ", " << ToStr(destination) << ") == "
                << VisibleEdgeFlowState::getDescription(currentFlowState));
    if (commonData.utils.IsFailedResponse(localStatus, list))
    {
        this->haveError = true;
        this->currentFlowState = VisibleEdgeFlowState::Failed;
    }

    LOG_DEBUG("ProcessConfirm() received with success for " << toString());
    if (currentFlowState == VisibleEdgeFlowState::LinkSent)
    {
        if (haveError || stopRequested)
        {
            if (visibleEdgeFinishedHandler)
            {
                visibleEdgeFinishedHandler(source, destination, false);
            }

            return;
        }

        // the operations are only into NM => so we have to log them here
        SMState::SMStateLog::logAllInfo(linkOperations->reasonOfOperations);
        SMState::SMStateLog::logOperations(linkOperations->reasonOfOperations, *linkOperations);

        this->currentFlowState = VisibleEdgeFlowState::Waiting;
        LOG_TRACE("ProcessConfirm() for (" << ToStr(source) << ", " << ToStr(destination) << ") <= "
                    << VisibleEdgeFlowState::getDescription(currentFlowState));
        this->activeTime = time(NULL);
        // from now on waits the timeout time to see if there any alarms from the devices
    }
    else if (currentFlowState == VisibleEdgeFlowState::RemoveLinksSent)
    {
        if (haveError || stopRequested)
        {
            if (visibleEdgeFinishedHandler)
            {
                visibleEdgeFinishedHandler(source, destination, false);
            }

            return;
        }

        // the operations are only into NM => so we have to log them here
        SMState::SMStateLog::logAllInfo(removeLinkOperations->reasonOfOperations);
        SMState::SMStateLog::logOperations(removeLinkOperations->reasonOfOperations, *removeLinkOperations);

        this->currentFlowState = VisibleEdgeFlowState::Success;
        LOG_TRACE("ProcessConfirm() for (" << ToStr(source) << ", " << ToStr(destination) << ") <= "
                    << VisibleEdgeFlowState::getDescription(currentFlowState));
    }
    else
    {
        LOG_WARN("Unexpected confirm.");
    }

}

void VisibleEdgeFlow::ProcessIndicate(stack::WHartHandle handle, const stack::WHartAddress& src,
                                      stack::WHartPriority priority, stack::WHartTransportType transportType,
                                      const stack::WHartCommandList& list)
{
    LOG_TRACE("ProcessIndicate()");
}

void VisibleEdgeFlow::StopFlow()
{
    LOG_TRACE("Stopping VisibleEdgeFlowHandler, (" << ToStr(source) << ", "
                << ToStr(destination) << ")" << ", currentFlowState="
                << VisibleEdgeFlowState::getDescription(currentFlowState));

    haveError = true;
    stopRequested = true;

    if (currentFlowState == VisibleEdgeFlowState::LinkSent)
    {
        operationsQueue.CancelOperations(linkOperationsEvent);
    }
    else if (currentFlowState == VisibleEdgeFlowState::RemoveLinksSent)
    {
        operationsQueue.CancelOperations(removeLinkOperationsEvent);
    }
}

void VisibleEdgeFlow::newAlarm788(uint16_t src, C788_AlarmPathDown_Resp& alarmPathDown)
{
    LOG_TRACE("newAlarm788 received for (" << ToStr(src) << ", " << ToStr(alarmPathDown.Nickname) << ")");

    if (noAllowedAlarms == 0)
    {
        if (visibleEdgeFinishedHandler)
        {
            LOG_DEBUG("Number of allowed alarms expired for " << toString() << "");
            visibleEdgeFinishedHandler(this->source, this->destination, false);
        }

        return;
    }

    --noAllowedAlarms;

    // reset the timeout so that it will wait a full time interval
    this->activeTime = time(NULL);
}

void VisibleEdgeFlow::newAlarm789(uint16_t src, C789_AlarmSourceRouteFailed_Resp& alarmSourceRouteFailed)
{
    // do nothing
}

void VisibleEdgeFlow::newAlarm790(uint16_t src, C790_AlarmGraphRouteFailed_Resp& alarmGraphRouteFailed)
{
    // do nothing
}

void VisibleEdgeFlow::newAlarm791(uint16_t src, C791_AlarmTransportLayerFailed_Resp& alarmTransportLayerFailed)
{
    // do nothing
}

void VisibleEdgeFlow::createVisibleEdge(uint16_t source, uint16_t destination, uint8_t rsl)
{
    LOG_TRACE("createVisibleEdge() for (" << ToStr(source) << ", " << ToStr(destination) << ")");

    this->source = source;
    this->destination = destination;
    this->rsl = rsl;

    linkOperations.reset(new EngineOperations());
    std::ostringstream stream;
    stream << "Add links for visible edge (";
    stream << ToStr(source) << ", " << ToStr(destination) << ")";
    linkOperations->reasonOfOperations = stream.str();

    commonData.networkEngine.allocateCheckLink(*linkOperations, source, destination);

    operations::WHEngineOperationsVisitor visitor(linkOperations, commonData);
    operationsQueue.generateWHOperations(linkOperations, linkWhOperations, visitor);

    // the operations are only into NM => so we have to log them here
    SMState::SMStateLog::logAllInfo(linkOperations->reasonOfOperations);
    SMState::SMStateLog::logOperations(linkOperations->reasonOfOperations, *linkOperations);

    if (operationsQueue.addOperations(linkOperations, linkWhOperations, linkOperationsEvent, linkOperations->reasonOfOperations,
                                      boost::bind(&VisibleEdgeFlow::ProcessConfirmedOperations, this, _1), NULL))
    {
        operationsQueue.sendOperations();
    }

    currentFlowState = VisibleEdgeFlowState::LinkSent;
    LOG_TRACE("createVisibleEdge() for (" << ToStr(source) << ", " << ToStr(destination) << ") <= "
                << VisibleEdgeFlowState::getDescription(currentFlowState));
}

void VisibleEdgeFlow::removeTestLinks()
{
    LOG_TRACE("removeTestLinks() for (" << ToStr(source) << ", " << ToStr(destination) << ")");

    removeLinkOperations.reset(new EngineOperations());

    std::ostringstream stream;
    stream << "Remove links for visible edge (";
    stream << ToStr(source) << ", " << ToStr(destination) << ")";
    removeLinkOperations->reasonOfOperations = stream.str();

    commonData.networkEngine.deallocateCheckLink(*removeLinkOperations, source, destination);

    operations::WHEngineOperationsVisitor visitor(removeLinkOperations, commonData);
    operationsQueue.generateWHOperations(removeLinkOperations, removeLinkWhOperations, visitor);

    // the operations are only into NM => so we have to log them here
    SMState::SMStateLog::logAllInfo(removeLinkOperations->reasonOfOperations);
    SMState::SMStateLog::logOperations(removeLinkOperations->reasonOfOperations, *removeLinkOperations);

    if (operationsQueue.addOperations(removeLinkOperations, removeLinkWhOperations, removeLinkOperationsEvent,
                                      removeLinkOperations->reasonOfOperations,
                                      boost::bind(&VisibleEdgeFlow::ProcessConfirmedOperations, this, _1), NULL))
    {
        operationsQueue.sendOperations();
    }

    currentFlowState = VisibleEdgeFlowState::RemoveLinksSent;
}

bool VisibleEdgeFlow::hasTimeOuted()
{
    if (this->currentFlowState != VisibleEdgeFlowState::Waiting)
    {
        return false;
    }

    if (time(NULL) >= this->activeTime + (int) this->TimeOut)
    {
        LOG_TRACE("hasTimeOuted() for (" << ToStr(source) << ", " << ToStr(destination) << ") -> true");
        return true;
    }

    LOG_TRACE("hasTimeOuted() for (" << ToStr(source) << ", " << ToStr(destination) << ") -> false");
    return false;
}

void VisibleEdgeFlow::doFinalJob(bool status)
{
    LOG_TRACE("doFinalJob(" << (status ? "success" : "error") << ") on " << toString());
    commonData.networkEngine.addVisible(this->source, this->destination, this->rsl);
}

WHartHandle VisibleEdgeFlow::TransmitRequest(const WHartAddress& to, const WHartCommandList& commands,
                                             WHartSessionKey::SessionKeyCode sessionCode)
{
    throw NE::Common::NEException("TransmitRequest()");
}

void VisibleEdgeFlow::ProcessConfirmedOperations(bool errorOccured)
{
    ProcessConfirm(0, errorOccured ? stack::WHartLocalStatus::whartlsError_Start
                : stack::WHartLocalStatus::whartlsSuccess, WHartCommandList());
}

}

}
