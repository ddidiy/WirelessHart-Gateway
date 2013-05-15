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

#include "C078_ReadAggregatedCommands.h"

uint8_t Compose_C078_ReadAggregatedCommands_Req(C078_ReadAggregatedCommands_Req* request, ComposerContext* context,
		BinaryStream* toStream)
{
	context->cmdId = CMDID_C078_ReadAggregatedCommands;

	if (request->noOfCommands == 0 )
	return RCS_E01_Undefined1;

	STREAM_WRITE_UINT8(toStream, request->noOfCommands);

	for (uint8_t i = 0; i < request->noOfCommands; i++)
	{
		STREAM_WRITE_UINT16(toStream, request->commands[i].CmdNo);
		STREAM_WRITE_UINT8(toStream, request->commands[i].CmdSize);
		STREAM_WRITE_BYTES(toStream, request->commands[i].CmdData, request->commands[i].CmdSize); //cmd bytes
	}

	return RCS_N00_Success;
}

uint8_t Parse_C078_ReadAggregatedCommands_Req(C078_ReadAggregatedCommands_Req* request, ParserContext* context,
		BinaryStream* fromStream)
{
	STREAM_READ_UINT8(fromStream, &request->noOfCommands);

	if(request->noOfCommands == 0)
		return RCS_E01_Undefined1;


	for (uint8_t i = 0; i < request->noOfCommands; i++)
	{
		if(fromStream->remainingBytes < 3)
			return RCS_E05_TooFewDataBytesReceived;

                STREAM_READ_UINT16(fromStream, &request->commands[i].CmdNo);
		STREAM_READ_UINT8(fromStream, &request->commands[i].CmdSize);

		if(fromStream->remainingBytes <  request->commands[i].CmdSize)
			return RCS_E05_TooFewDataBytesReceived;

		STREAM_READ_BYTES(fromStream, request->commands[i].CmdData, request->commands[i].CmdSize); //cmd bytes

	}

	return RCS_N00_Success;
}

uint8_t Compose_C078_ReadAggregatedCommands_Resp(C078_ReadAggregatedCommands_Resp* response, ComposerContext* context,
		BinaryStream* toStream)
{
	context->cmdId = CMDID_C078_ReadAggregatedCommands;
#ifdef _DEBUG
// 	if (response->noOfCommands == 0 )
// 	return RCS_E01_Undefined1;
#endif
	//STREAM_WRITE_BYTES (toStream, &response->extendedDeviceStatus, sizeof(response->extendedDeviceStatus));
	STREAM_WRITE_UINT8(toStream, response->extendedDeviceStatus);
	STREAM_WRITE_UINT8(toStream, response->noOfCommands);

	for (uint8_t i = 0; i < response->noOfCommands; i++)
	{
                STREAM_WRITE_UINT16(toStream, response->commands[i].CmdNo);
		STREAM_WRITE_UINT8(toStream, response->commands[i].CmdSize);
		STREAM_WRITE_BYTES(toStream, response->commands[i].CmdData, response->commands[i].CmdSize); //cmd bytes
	}

	return RCS_N00_Success;
}

uint8_t Parse_C078_ReadAggregatedCommands_Resp(C078_ReadAggregatedCommands_Resp* response, ParserContext* context,
		BinaryStream* fromStream)
{

	STREAM_READ_BYTES (fromStream, &response->extendedDeviceStatus, sizeof(response->extendedDeviceStatus));
	STREAM_READ_UINT8(fromStream, &response->noOfCommands);

	if (response->noOfCommands == 0 )
		return RCS_E01_Undefined1;

	for (uint8_t i = 0; i < response->noOfCommands; i++)
	{
		// RESPONSE CODE included in CmdData => 1Byte mandatory
		if(fromStream->remainingBytes < 4)
			return RCS_E05_TooFewDataBytesReceived;

                STREAM_READ_UINT16(fromStream, &response->commands[i].CmdNo);
		STREAM_READ_UINT8(fromStream, &response->commands[i].CmdSize);

		if(fromStream->remainingBytes <  response->commands[i].CmdSize)
			return RCS_E05_TooFewDataBytesReceived;

		STREAM_READ_BYTES(fromStream, response->commands[i].CmdData, response->commands[i].CmdSize); //cmd bytes
	}

	return RCS_N00_Success;
}
