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

#include "ApplicationCommand.h"
#include <WHartStack/util/Binarization.h>

/**
 * A binary search algorithm
 */

int16_t FindEntry(const ParseExecuteComposerEntry* sortedList, uint16_t listSize, const uint16_t cmdId)
{
	int16_t low = 0;
	int16_t high = listSize - 1;

	while (low <= high)
	{
		int16_t mid = (low + high) / 2;
		const ParseExecuteComposerEntry *guess = sortedList + mid;
		if (guess->cmdId == cmdId)
			return mid;

		else if (guess->cmdId < cmdId) //is on left side
			low = mid + 1;
		else
			// is on right side
			high = mid - 1;
	}
	return -1; // not found
}

ParseComposeErrorCodes ParseWirelessRequestCommmand(ParseWirelessCommandData* commandData)
{
	BinaryStream stream;
	STREAM_INIT(&stream, (const uint8_t*) commandData->inputBuffer, commandData->inputBufferSize);

	//parse Wireless Application Command Header
	STREAM_READ_UINT16(&stream, &commandData->parsedCmdId);
	_command_size_t dataLength;
	STREAM_READ_UINT8(&stream, &dataLength);

	// CHECKME
	if (dataLength > stream.remainingBytes)
	{
		// *consumedBufferBytes = 0;
		commandData->consumedBufferBytes = stream.remainingBytes + 3;
		return pcecNotEnoughBytes;
	}

	commandData->consumedBufferBytes = dataLength + 3; // 2 for cmdId + 1 for dataLength

	commandData->tableIndex = FindEntry(commandData->table, commandData->tableSize, commandData->parsedCmdId);
	ParseFunction parser = (commandData->tableIndex == -1) ? 0 : commandData->table[commandData->tableIndex].fnParser;
	if (!parser)
		return pcecNotImplemented;

	//init remaining bytes
	stream.remainingBytes = dataLength;

	ParserContext context;
	context.allocatedCommandSize = commandData->parsedCommandSize;
	context.maxNeededParsedSize =  commandData->parsedCommandSize;

	uint8_t u8Res = (*parser)(commandData->parsedCommand, &context, &stream);

	commandData->parsedCommandSize = context.maxNeededParsedSize;
	//CHECKME [ovidiu rauca] - we suppose is the single request parser problem
	return u8Res ? pcecNotEnoughBytes : pcecSuccess;
}

ParseComposeErrorCodes ParseWirelessResponseCommmand(ParseWirelessCommandData* commandData)
{
	BinaryStream stream;
	STREAM_INIT(&stream, (const uint8_t*) commandData->inputBuffer, commandData->inputBufferSize);

	// [claudiu hobeanu] should be test also that the input buffer has at least 3bytes cmdID + nLen
	//parse Wireless Application Command Header
	STREAM_READ_UINT16(&stream, &commandData->parsedCmdId);
	_command_size_t dataLength;
	STREAM_READ_UINT8(&stream, &dataLength);

	// CHECKME
	if (dataLength > stream.remainingBytes)
	{
		commandData->consumedBufferBytes = stream.remainingBytes + 3;
		return pcecNotEnoughBytes;
	}
	commandData->consumedBufferBytes = dataLength + 3; // 2 for cmdId + 1 for dataLength

	//parse response code and re-init remaining bytes, parse data
	uint8_t responseCode;
	STREAM_READ_UINT8(&stream, &responseCode);
	commandData->parsedResponseCode = responseCode;
	if (IsResponseCodeError(responseCode))
	{
		return pcecSuccess;
	}

	//parse data
	commandData->tableIndex = FindEntry(commandData->table, commandData->tableSize, commandData->parsedCmdId);
	ParseFunction parser = (commandData->tableIndex == -1) ? 0 : commandData->table[commandData->tableIndex].fnParser;
	if (!parser)
		return pcecNotImplemented;

	ParserContext context;
	context.allocatedCommandSize = commandData->parsedCommandSize;
	context.maxNeededParsedSize =  commandData->parsedCommandSize;

	stream.remainingBytes = dataLength - 1; //response code is in data length

	uint8_t u8Res = (*parser)(commandData->parsedCommand, &context, &stream) ;

	commandData->parsedCommandSize = context.maxNeededParsedSize;

	//CHECKME [ovidiu rauca] - we suppose is the single request parser problem
	return u8Res ? pcecNotEnoughBytes : pcecSuccess;
}

ParseComposeErrorCodes ComposeWirelessRequestCommmand(ComposeWirelessCommandData* commandData)
{
	BinaryStream stream;
	STREAM_INIT(&stream, (const uint8_t*) commandData->outputBuffer, commandData->outputBufferSize);

	STREAM_WRITE_UINT16(&stream, commandData->cmdId);
	STREAM_WRITE_UINT8(&stream, 0); //will be overite latter

	commandData->tableIndex = FindEntry(commandData->table, commandData->tableSize, commandData->cmdId);
	ComposeFunction composer = (commandData->tableIndex == 0xFFFF) ? 0
			: commandData->table[commandData->tableIndex].fnComposer;
	if (!composer)
		return pcecNotImplemented;

	ComposerContext context;
	context.cmdId = commandData->cmdId;

	uint8_t composingError = (*composer)(commandData->composingCommand, &context, &stream);
	if (!IsResponseCodeError(composingError))
	{
		uint8_t dataLength = stream.nextByte - (uint8_t*) commandData->outputBuffer - (2 + 1);
		//write size
		BinaryStream s;
		STREAM_INIT(&s, (const uint8_t*) commandData->outputBuffer, commandData->outputBufferSize);
		STREAM_SKIP(&s, 2);
		STREAM_WRITE_UINT8(&s, dataLength);
		commandData->writenBufferBytes = commandData->outputBufferSize - stream.remainingBytes;
		return pcecSuccess;
	}
	//CHECKME [ovidiu rauca] - we suppose is the single request compose problem
	return pcecNotEnoughBytes;
}

ParseComposeErrorCodes ComposeWirelessResponseCommmand(ComposeWirelessCommandData* commandData)
{
	BinaryStream stream;
	STREAM_INIT(&stream, (const uint8_t*) commandData->outputBuffer, commandData->outputBufferSize);

	STREAM_WRITE_UINT16(&stream, commandData->cmdId);

	if (IsResponseCodeError(commandData->responseCode))
	{
		STREAM_WRITE_UINT8(&stream, 1);
		STREAM_WRITE_UINT8(&stream, commandData->responseCode);

		commandData->writenBufferBytes = commandData->outputBufferSize - stream.remainingBytes;
		return pcecSuccess;
	}

	STREAM_WRITE_UINT8(&stream, 0); //will be overite latter
	STREAM_WRITE_UINT8(&stream, commandData->responseCode);
	commandData->tableIndex = FindEntry(commandData->table, commandData->tableSize, commandData->cmdId);
	ComposeFunction composer = (commandData->tableIndex == 0xFFFF) ? 0
			: commandData->table[commandData->tableIndex].fnComposer;
	if (!composer)
		return pcecNotImplemented;

	ComposerContext context;
	context.cmdId = commandData->cmdId;

	uint8_t composingError = (*composer)(commandData->composingCommand, &context, &stream);
	if (composingError == 0)
	{
		uint8_t dataLength = stream.nextByte - (uint8_t*) commandData->outputBuffer - (2 + 1);
		//write size
		BinaryStream s;
		STREAM_INIT(&s, (const uint8_t*) commandData->outputBuffer, commandData->outputBufferSize);
		STREAM_SKIP(&s, 2);
		STREAM_WRITE_UINT8(&s, dataLength);
		commandData->writenBufferBytes = commandData->outputBufferSize - stream.remainingBytes;
		return pcecSuccess;
	}

	return pcecNotEnoughBytes;
}

