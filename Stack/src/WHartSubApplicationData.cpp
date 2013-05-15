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
 * WHartSubApp.c
 *
 *  Created on: Dec 5, 2008
 *      Author: nicu.dascalu
 */

#include <WHartStack/WHartSubApplicationData.h>
#include <WHartStack/util/Binarization.h>

#include <cassert>

namespace hart7 {
namespace stack {
namespace subapp {

bool WHartSubApplicationData::IsResponseType(WHartTransportType type)
{
	switch (type)
	{
	case wharttTransferResponse:
	case wharttResponseUnicast:
	case wharttPublishBroadcast:
	case wharttPublishNotify:
	case wharttResponseBroadcast:
		return true;
	case wharttTransferRequest:
	case wharttRequestUnicast:
	case wharttSearchBroadcast:
	case wharttRequestBroadcast:
		return false;
	default:
		assert(false && "Unknown WHartTransportType !!!");
		return true;
	}
}

WHartSubApplicationData::WHartSubApplicationData(CommonData& commonData_) :
	commonData(commonData_)
{
	nextHandle = 0;
}

/**
 * create a stream, binarize all requests, and send payload to transport
 */
WHartHandle WHartSubApplicationData::TransmitRequest(const WHartAddress& dest, WHartPriority priority,
		WHartServiceID serviceID, WHartTransportType transportType, const WHartCommandList& list,
		WHartSessionKey::SessionKeyCode sessionCode)
{
	//binarize all requests
	uint8_t buffer[SUBAPP_MAX_COMMAND_SIZE];
	uint16_t writtenBytes = 0;
	if (!ComposePayload(buffer, SUBAPP_MAX_COMMAND_SIZE, writtenBytes, list, IsResponseType(transportType)))
	{
		//CHECKME [ovidiu.rauca] this should be the error code ???
		return 0xFFFF;
	}

	WHartHandle handle = GenerateNewHandle();

	WHartPayload apdu(buffer, writtenBytes);
	LOG_DEBUG("TR: h=" << handle << ", dst=" << dest << ", pr=" << priority << ", trType=" << ToString(transportType) << ", APDU=" << apdu);
	lower->TransmitRequest(handle, dest, priority, serviceID, transportType, apdu,
			sessionCode);
	return handle;
}

/**
 * unserialize, and forward to app
 */
void WHartSubApplicationData::TransmitConfirm(WHartHandle requestHandle, const WHartLocalStatus& localStatus, DeviceIndicatedStatus status,
		const WHartPayload& apdu)
{
	if (apdu.dataLen > 0 && localStatus.status == 0)
	{
		//have payload
		uint8_t commandsDataBuffer[SUBAPP_MAX_COMMANDS_BUFFER_SIZE];
		WHartCommand commands[SUBAPP_MAX_COMMANDS_COUNT];

		WHartCommandList responses = { 0, commands };
		if (!ParsePayload(apdu, commandsDataBuffer, SUBAPP_MAX_COMMANDS_BUFFER_SIZE, responses, true))
		{
			return;
		}
		upper->TransmitConfirm(requestHandle, localStatus, status, responses);
	}
	else
	{
		WHartCommandList responses = { 0, NULL };
		upper->TransmitConfirm(requestHandle, localStatus, status, responses);
	}
}

/**
 * deserialize  if is for Stack -> update stack, if not forward to app
 */
void WHartSubApplicationData::TransmitIndicate(WHartHandle handle, const WHartAddress& src, WHartPriority priority, DeviceIndicatedStatus status,
		WHartTransportType transportType, const WHartPayload& apdu, WHartSessionKey::SessionKeyCode sessionCode)
{
	LOG_DEBUG("TI: h=" << handle << ", src=" << src << ", priority=" << priority << " transportType=" << ToString(transportType) << ", APDU=" << apdu);

	//have payload
	uint8_t commandsDataBuffer[SUBAPP_MAX_COMMANDS_BUFFER_SIZE];
	WHartCommand commands[SUBAPP_MAX_COMMANDS_COUNT];

	if (transportType == wharttRequestUnicast)
	{
		//request
		WHartCommandList requests = { 0, commands };

		if (!ParsePayload(apdu, commandsDataBuffer, SUBAPP_MAX_COMMANDS_BUFFER_SIZE, requests, false))
		{
			return;
		}
		upper->TransmitIndicate(handle, src, priority, status, transportType, requests);
	}
	else if (transportType == wharttResponseUnicast || transportType == wharttPublishNotify)
	{
		//response
		WHartCommandList responses = { 0, commands };

		if (!ParsePayload(apdu, commandsDataBuffer, SUBAPP_MAX_COMMANDS_BUFFER_SIZE, responses, true))
		{
			return;
		}
		upper->TransmitIndicate(handle, src, priority, status, transportType, responses);
	}
}

void WHartSubApplicationData::TransmitResponse(WHartHandle indicatedHandle, WHartServiceID serviceID,
		const WHartCommandList& list, WHartSessionKey::SessionKeyCode sessionCode)
{
	//binarize all responses
	uint8_t buffer[SUBAPP_MAX_COMMAND_SIZE];
	uint16_t writtenBytes = 0;
	if (!ComposePayload(buffer, SUBAPP_MAX_COMMAND_SIZE, writtenBytes, list, true))
	{
		return; //TODO throw exception
	}

	lower->TransmitResponse(indicatedHandle, serviceID, WHartPayload(buffer, writtenBytes), sessionCode);
}

WHartHandle WHartSubApplicationData::GenerateNewHandle()
{
	nextHandle++;
	if (0x1FFF < nextHandle)
		nextHandle = 1; //overflow, so reset it

	return MAKE_WHARTHANDLE(0, nextHandle);
}

bool WHartSubApplicationData::ComposePayload(uint8_t* commandsDataBuffer, uint16_t commandsDataBufferSize,
		uint16_t& writtenBytes, const WHartCommandList& list, bool isResponse)
{
//	LOG_DEBUG("Compose payload of commands count=" << (int) list.count << " Response:" << isResponse);
	//binarize all requests
	writtenBytes = 0;
	for (int i = 0; i < list.count; i++)
	{
//		LOG_DEBUG("Composing command with CMDID=" << (int) list.list[i].commandID);
		ComposeWirelessCommandData commandData;
		commandData.cmdId = list.list[i].commandID;
		commandData.composingCommand = list.list[i].command;
		commandData.outputBuffer = commandsDataBuffer + writtenBytes;
		commandData.outputBufferSize = commandsDataBufferSize - writtenBytes;
		commandData.responseCode = list.list[i].responseCode;
		if (isResponse)
		{
			commandData.table = parseReqComposeResp;
			commandData.tableSize = parseReqComposeRespSize;
		}
		else
		{
			commandData.table = composeReqParseResp;
			commandData.tableSize = composeReqParseRespSize;
		}
		commandData.writenBufferBytes = 0;
		ParseComposeErrorCodes composeErrorCode = isResponse
		?	ComposeWirelessResponseCommmand(&commandData) :
			ComposeWirelessRequestCommmand(&commandData);

		if (composeErrorCode != pcecSuccess)
		{
			LOG_ERROR("Cannot compose command with CMDID=:" << (int) list.list[i].commandID << " error code:"
					<< (int) composeErrorCode);
			return false;
		}
		writtenBytes += commandData.writenBufferBytes;

//		LOG_DEBUG("ComposePayload:"
//			<< " commandsDataBuffer=" << (void*)commandsDataBuffer
//			<< " commandsDataBufferSize=" << commandsDataBufferSize
//			<< " writtenBytes=" << writtenBytes
//			<< " commandData.cmdId=" << (int)commandData.cmdId
//			<< " commandData.composingCommand=" << commandData.composingCommand
//			<< " commandData.outputBuffer=" << commandData.outputBuffer
//			<< " commandData.outputBufferSize=" << commandData.outputBufferSize);
	}
	return true;
}

bool WHartSubApplicationData::ParsePayload(const WHartPayload& apdu, uint8_t* commandsDataBuffer,
		uint16_t commandsDataBufferSize, WHartCommandList& list, bool isResponse)
{
	uint16_t consumedCommandsBuffer = 0;
	uint16_t consumedInputBufferBytes = 0;

//	LOG_DEBUG("ParsePayload:(begin)"
//		<< " APDU.data=" << (void*)apdu.data << " APDU.dataLen=" << (int)apdu.dataLen
//		<< " commandsDataBuffer=" << (void*)commandsDataBuffer
//		<< " commandsDataBufferSize=" << commandsDataBufferSize);

	while (apdu.dataLen - consumedInputBufferBytes > 0)
	{
		//CHECKME [nicu.dascalu]: need align to 4 bytes ???
//		consumedCommandsBuffer = AlignToBytes(commandsDataBuffer, consumedCommandsBuffer, 4);

		ParseWirelessCommandData commandData;
		commandData.consumedBufferBytes = 0;
		commandData.inputBuffer = apdu.data + consumedInputBufferBytes;
		commandData.inputBufferSize = apdu.dataLen - consumedInputBufferBytes;
		commandData.parsedCommand = commandsDataBuffer + consumedCommandsBuffer;
		commandData.parsedCommandSize = commandsDataBufferSize - consumedCommandsBuffer;
		//HACK: [Ovidiu.Rauca] - we assume that our binaries are not longer than 256 bytes
		commandData.parsedCommandEffectiveSize = 256;

		if (isResponse)
		{
			commandData.table = composeReqParseResp;
			commandData.tableSize = composeReqParseRespSize;
		}
		else
		{
			commandData.table = parseReqComposeResp;
			commandData.tableSize = parseReqComposeRespSize;
		}

		WHartCommand* currentCommand = &list.list[list.count++];
		ParseComposeErrorCodes parseErrorCode = isResponse
		? ParseWirelessResponseCommmand(&commandData) :
			ParseWirelessRequestCommmand(&commandData);

		if (parseErrorCode != pcecSuccess)
		{
			LOG_ERROR("Cannot parse command=" << commandData.parsedCmdId << " parse error code="
					<< (int) parseErrorCode);
			return false;
		}

		currentCommand->command = commandData.parsedCommand;
		currentCommand->commandID = commandData.parsedCmdId;
		currentCommand->responseCode = isResponse ? commandData.parsedResponseCode : 0;

		consumedInputBufferBytes += commandData.consumedBufferBytes;
		consumedCommandsBuffer += commandData.parsedCommandEffectiveSize;

		LOG_DEBUG("ParsePayload:(step )"
			<< " consumedInputBufferBytes=" << consumedInputBufferBytes
			<< " commandData.inputBuffer=" <<commandData.inputBuffer
			<< " commandData.inputBufferSize=" << commandData.inputBufferSize
			<< " commandData.parsedCommand=" << commandData.parsedCommand
			<< " commandData.parsedCommandSize=" << commandData.parsedCommandSize
			);
	} //while

	return true;
}

} //namespace subapp
} //namespace stack
} //namespace hart7
