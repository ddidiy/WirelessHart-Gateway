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

#include "C109_BurstModeControl.h"

uint8_t Compose_C109_BurstModeControl_Req(C109_BurstModeControl_Req* request, ComposerContext* context,
		BinaryStream* toStream)
{
	context->cmdId = CMDID_C109_BurstModeControl;
	STREAM_WRITE_UINT8(toStream, request->burstModeCode);
	if (!request->burstMessageIsEmpty)
	{
		STREAM_WRITE_UINT8(toStream, request->burstMessage);
	}
	return RCS_N00_Success;
}

uint8_t Parse_C109_BurstModeControl_Req(C109_BurstModeControl_Req* request, ParserContext* context,
		BinaryStream* fromStream)
{
	if (fromStream->remainingBytes > 0)
	{
		STREAM_READ_UINT8(fromStream, &request->burstModeCode);
		request->requestIsEmpty = 0;
		if (request->burstModeCode > 1)
		{
			return RCS_E05_TooFewDataBytesReceived;
		}
	} else
	{
		request->requestIsEmpty = 1;
	}
	if (fromStream->remainingBytes > 0)
	{
		STREAM_READ_UINT8(fromStream, &request->burstMessage);
		request->burstMessageIsEmpty = 0;
	} else
		request->burstMessageIsEmpty = 1;
	return RCS_N00_Success;
}

uint8_t Compose_C109_BurstModeControl_Resp(C109_BurstModeControl_Resp* response, ComposerContext* context,
		BinaryStream* toStream)
{
	context->cmdId = CMDID_C109_BurstModeControl;

	STREAM_WRITE_UINT8(toStream, response->burstModeCode);
	STREAM_WRITE_UINT8(toStream, response->burstMessage);
	return RCS_N00_Success;
}

uint8_t Parse_C109_BurstModeControl_Resp(C109_BurstModeControl_Resp* response, ParserContext* context,
		BinaryStream* fromStream)
{

	STREAM_READ_UINT8(fromStream, &response->burstModeCode);
	STREAM_READ_UINT8(fromStream, &response->burstMessage);
	return RCS_N00_Success;
}
