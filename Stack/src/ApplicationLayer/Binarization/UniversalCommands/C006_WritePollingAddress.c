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

#include "C006_WritePollingAddress.h"

uint8_t Compose_C006_WritePollingAddress_Req(C006_WritePollingAddress_Req* request, ComposerContext* context,
		BinaryStream* toStream)
{
	context->cmdId = CMDID_C006_WritePollingAddress;

	STREAM_WRITE_UINT8(toStream, request->pollingAddressDevice);

	STREAM_WRITE_UINT8(toStream, request->loopCurrentModeCode);

	return RCS_N00_Success;
}

uint8_t Parse_C006_WritePollingAddress_Req(C006_WritePollingAddress_Req* request, ParserContext* context,
		BinaryStream* fromStream)
{
	if((fromStream->remainingBytes >= C006_ReqSize_5RevNo ) && (fromStream->remainingBytes < C006_ReqSize_7RevNo))
	{
		STREAM_READ_UINT8(fromStream, &request->pollingAddressDevice);	
		request->loopCurrentModeCode = ( request->pollingAddressDevice == 0 ) ? LoopCurrentModeCodes_Enabled : LoopCurrentModeCodes_Disabled;
	}
	else if (fromStream->remainingBytes >= C006_ReqSize_7RevNo)
	{
		STREAM_READ_UINT8(fromStream, &request->pollingAddressDevice);
		STREAM_READ_UINT8(fromStream, &request->loopCurrentModeCode);
	}	

	return RCS_N00_Success;
}

uint8_t Compose_C006_WritePollingAddress_Resp(C006_WritePollingAddress_Resp* response, ComposerContext* context,
		BinaryStream* toStream)
{
	context->cmdId = CMDID_C006_WritePollingAddress;

	STREAM_WRITE_UINT8(toStream, response->pollingAddressDevice);

	STREAM_WRITE_UINT8(toStream, response->loopCurrentModeCode);

	return RCS_N00_Success;
}

uint8_t Parse_C006_WritePollingAddress_Resp(C006_WritePollingAddress_Resp* response, ParserContext* context,
		BinaryStream* fromStream)
{
	STREAM_READ_UINT8(fromStream, &response->pollingAddressDevice);
	STREAM_READ_UINT8(fromStream, &response->loopCurrentModeCode);

	return RCS_N00_Success;
}
