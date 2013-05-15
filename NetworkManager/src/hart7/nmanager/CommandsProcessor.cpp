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
 * CommandsProcessor.cpp
 *
 *  Created on: May 22, 2009
 *      Author: Andy
 */

#include "CommandsProcessor.h"

#include "AllNetworkManagerCommands.h"
#include <hart7/util/NMLog.h>

namespace hart7 {
namespace nmanager {

using namespace hart7::stack;

void CommandsProcessor::ProcessConfirm(WHartHandle requestHandle, const WHartLocalStatus& localStatus,
                                       const WHartCommandList& list)
{
	if (isSendingCommand)
	{
		errorOccured = true;
		lastStatus = localStatus;
	}

	{
		std::map<stack::WHartHandle, IResponseProcessorPlaceholder>::iterator it = responseProcessors.find(requestHandle);
		if (it != responseProcessors.end())
		{
			try
			{
				it->second.ResponseProcessor.ProcessConfirm(requestHandle, localStatus, list);
			}
			catch (std::exception& ex)
			{
				LOG_WARN("Exception while confirming packet. ex=" << ex.what());
			}

			responseProcessors.erase(it);
		}
		else
		{
			//TODO log unexpected confirm
		}

		hart7::util::NMLog::logDeviceHistory();
	}
}

void CommandsProcessor::ProcessIndicate(WHartHandle handle, const WHartAddress& src, WHartPriority priority,
                                        WHartTransportType transportType, const WHartCommandList& list)
{
    if (IsRegistrationRequest(list))
    {
        if (HandleJoinRequest)
        {
            HandleJoinRequest(src, list);
        }
    }
    else if (transportType == wharttRequestUnicast)
    {
        genericCommandsHandler.HandleRequests(handle, src, list);
    }
    else if (transportType == wharttResponseUnicast)
    {
        genericCommandsHandler.HandleResponses(handle, src, list);
    }
    else if (transportType == wharttPublishNotify)
    {
        genericCommandsHandler.HandlePublishes(handle, src, list);
    }
}

bool CommandsProcessor::IsRegistrationRequest(const WHartCommandList& commands)
{
    if (commands.count == 3)
    {
        if (commands.list[0].commandID == CMDID_C000_ReadUniqueIdentifier && commands.list[1].commandID
                    == CMDID_C020_ReadLongTag && commands.list[2].commandID == CMDID_C787_ReportNeighborSignalLevels)
        {
            return true;
        }
    }

    return false;
}

WHartHandle CommandsProcessor::TransmitRequest(const WHartAddress& dest, WHartPriority priority,
                                               WHartServiceID serviceID, WHartTransportType transportType,
                                               const WHartCommandList& list,
                                               WHartSessionKey::SessionKeyCode sessionCode,
                                               IResponseProcessor& responseProcessor)
{
	isSendingCommand = true;
	errorOccured = false;
    WHartHandle handle = TransmitRequestEvent(dest, priority, transportType, serviceID, list, sessionCode);
    isSendingCommand = false;

    if (errorOccured)
    {
    	responseProcessor.ProcessConfirm(handle, lastStatus, list);
    }
    else if (hart7::stack::transport::WHartTransportData::IsAcknowledgeableType(transportType)
            && handle != 0xFFFF)     // do not keep responseProcessors for ManagerStack
    {
    	responseProcessors.insert(std::make_pair(handle, IResponseProcessorPlaceholder(responseProcessor)));

    	LOG_DEBUG("ResponseProcessor count=" << responseProcessors.size());
    }

    return handle;
}

void CommandsProcessor::CancelRequest(WHartHandle handle)
{
    std::map<WHartHandle, IResponseProcessorPlaceholder>::iterator it = responseProcessors.find(handle);
    if (it != responseProcessors.end())
    {
        responseProcessors.erase(it);
    }
}

void CommandsProcessor::TransmitResponse(WHartHandle indicatedHandle, WHartServiceID serviceID,
                                         const WHartCommandList& list, WHartSessionKey::SessionKeyCode sessionCode)
{
    TransmitResponseEvent(indicatedHandle, serviceID, list, sessionCode);
}

bool CommandsProcessor::CanSendToAddress(const WHartAddress& dest)
{
	if (CanSendToDestinationEvent)
		return CanSendToDestinationEvent(dest);

	return false;
}


}
}
