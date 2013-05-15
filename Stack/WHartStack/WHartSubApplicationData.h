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
 * WHartSubApp.h
 *
 *  Created on: Dec 5, 2008
 *      Author: nicu.dascalu
 */

#ifndef WHART_SUBAPPLICATION_DATA_H
#define WHART_SUBAPPLICATION_DATA_H

#include <WHartStack/WHartTypes.h>
#include <WHartStack/WHartStack.h>
#include <WHartStack/WHartCommonData.h>
#include <ApplicationLayer/ApplicationCommand.h>
#include <ApplicationLayer/Model/CommandOperations.h>

#include <deque>
#include <boost/shared_ptr.hpp>
#include <nlib/log.h>

namespace hart7 {
namespace stack {
namespace subapp {


class WHartSubApplicationData: public WHartSubApplication
{
	LOG_DEF("h7.s.s.WHartSubApplicationData");
public:
	const static uint16_t SUBAPP_MAX_COMMAND_SIZE = 250; //for NM or GW can be greater
	const static uint16_t SUBAPP_MAX_COMMANDS_COUNT = 25;
	const static uint16_t SUBAPP_MAX_COMMANDS_BUFFER_SIZE = 10000;


	struct QueueCommand
	{
		typedef boost::shared_ptr<QueueCommand> Ptr;
		QueueCommand(WHartHandle handle_, WHartServiceID serviceID_, WHartSessionKey::SessionKeyCode sessionCode_)
			: isRequest(false), handle(handle_), serviceID(serviceID_), sessionCode(sessionCode_)
			{
				writtenBytes = 0;
			}

		QueueCommand(const WHartAddress& dest_, WHartPriority priority_,
				WHartServiceID serviceID_, WHartTransportType transportType_,
				WHartSessionKey::SessionKeyCode sessionCode_)
			{
				isRequest = true;
				dest = dest_;
				priority = priority_;
				serviceID = serviceID_;
				transportType = transportType_;
				sessionCode = sessionCode_;
				writtenBytes = 0;
			}


		bool isRequest;

		WHartHandle handle;
		WHartAddress dest;
		WHartPriority priority;
		WHartServiceID serviceID;
		WHartTransportType transportType;
		WHartSessionKey::SessionKeyCode sessionCode;

		uint8_t buffer[SUBAPP_MAX_COMMAND_SIZE];
		uint16_t writtenBytes;

	};

public:
	WHartSubApplicationData(CommonData& commonData);

	//WHartSubapplication
public:
	// on source, called by upper layer
	WHartHandle TransmitRequest(const WHartAddress& dest, WHartPriority priority, WHartServiceID serviceID,
	  WHartTransportType transportType, const WHartCommandList& list, WHartSessionKey::SessionKeyCode sessionCode);
	// on source, called by lower level
	void TransmitConfirm(WHartHandle requestHandle, const WHartLocalStatus& localStatus, DeviceIndicatedStatus status, const WHartPayload& apdu);
	// on destiantion, called by lower level
	void TransmitIndicate(WHartHandle handle, const WHartAddress& src, WHartPriority priority, DeviceIndicatedStatus status,
	  WHartTransportType transportType, const WHartPayload& apdu, WHartSessionKey::SessionKeyCode sessionCode);
	void TransmitResponse(WHartHandle indicatedHandle, WHartServiceID serviceID, const WHartCommandList& list, WHartSessionKey::SessionKeyCode sessionCode);

	static	bool IsResponseType(WHartTransportType type);

	WHartHandle GenerateNewHandle();
	bool ComposePayload(uint8_t* commandsDataBuffer, uint16_t commandsDataBufferSize, uint16_t& writtenBytes, const WHartCommandList& list,
	  bool isResponse);

	bool ParsePayload(const WHartPayload& apdu, uint8_t* commandsDataBuffer, uint16_t commandsDataBufferSize,
	  WHartCommandList& list, bool isResponse);


public:
	uint16_t composeReqParseRespSize;
	ParseExecuteComposerEntry* composeReqParseResp;

	uint16_t parseReqComposeRespSize;
	ParseExecuteComposerEntry* parseReqComposeResp;

private:
	CommonData& commonData;
	WHartHandle nextHandle;
};

} //namespace subapp
} //namespace stack
} //namespace hart7


#endif /* WHART_SUBAPPLICATION_DATA_H */
