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

#include "C64765_NivisMetaCommand.h"


uint8_t Compose_C64765_NivisMetaCommand_Req(C64765_NivisMetaCommand_Req* request, ComposerContext* context,
		BinaryStream* toStream)
{
	context->cmdId = CMDID_C64765_NivisMetaCommand;

	STREAM_WRITE_UINT16(toStream, request->Nickname);
    STREAM_WRITE_BYTES(toStream, request->DeviceUniqueId, 5);
	//STREAM_WRITE_UINT8(toStream, request->CommandSize);
	STREAM_WRITE_BYTES(toStream, request->Command, request->CommandSize);

	return RCS_N00_Success;
}

/**
 * @Debinarize
 */
uint8_t Parse_C64765_NivisMetaCommand_Req(C64765_NivisMetaCommand_Req* request, ParserContext* context,
		BinaryStream* fromStream)
{
#ifdef _DEBUG
// 	if (5 > fromStream->remainingBytes)
// 		return RCS_E05_TooFewDataBytesReceived;
#endif

	STREAM_READ_UINT16(fromStream, &request->Nickname);
	STREAM_READ_BYTES(fromStream, &request->DeviceUniqueId, 5);

	STREAM_PEEK_UINT8(fromStream, 2, &request->CommandSize);	//skip 2 octets of CmdId
	request->CommandSize += 3;	// 3 octets cmdId and size

//	STREAM_READ_UINT8(fromStream, &request->CommandSize); // skip a byte
//	STREAM_READ_UINT8(fromStream, &request->CommandSize); // skip a byte
//	STREAM_READ_UINT8(fromStream, &request->CommandSize); // read the real size of the inside command
//	// go back 3 chars
//	(fromStream)->nextByte -= 3;
//	(fromStream)->remainingBytes += 3;
//    request->CommandSize += 3;

	STREAM_READ_BYTES(fromStream, &request->Command, request->CommandSize);

	return RCS_N00_Success;
}

/**
 *
 */
uint8_t Compose_C64765_NivisMetaCommand_Resp(C64765_NivisMetaCommand_Resp* response, ComposerContext* context,
		BinaryStream* toStream)
{
	context->cmdId = CMDID_C64765_NivisMetaCommand;

	STREAM_WRITE_UINT16(toStream, response->Nickname);
	STREAM_WRITE_BYTES(toStream, response->DeviceUniqueId, 5);
	//STREAM_WRITE_UINT8(toStream, response->CommandSize);
	STREAM_WRITE_BYTES(toStream, response->Command, response->CommandSize);

	return RCS_N00_Success;
}

/**
 *
 */
uint8_t Parse_C64765_NivisMetaCommand_Resp(C64765_NivisMetaCommand_Resp* response, ParserContext* context,
		BinaryStream* fromStream)
{
#ifdef _DEBUG
// 	if (5 > fromStream->remainingBytes)
// 		return RCS_E05_TooFewDataBytesReceived;
#endif

	STREAM_READ_UINT16(fromStream, &response->Nickname);
	STREAM_READ_BYTES(fromStream, &response->DeviceUniqueId, 5);

	STREAM_PEEK_UINT8(fromStream, 2, &response->CommandSize);   //skip 2 octets of CmdId
	response->CommandSize += 3;  // 3 octets cmdId and size

//	STREAM_READ_UINT8(fromStream, &response->CommandSize); // skip a byte
//	STREAM_READ_UINT8(fromStream, &response->CommandSize); // skip a byte
//	STREAM_READ_UINT8(fromStream, &response->CommandSize); // read the real size of the inside command
//	// go back 3 chars
//	(fromStream)->nextByte -= 3;
//	(fromStream)->remainingBytes += 3;
//	response->CommandSize += 3;

	STREAM_READ_BYTES(fromStream, &response->Command, response->CommandSize);

	return RCS_N00_Success;
}
