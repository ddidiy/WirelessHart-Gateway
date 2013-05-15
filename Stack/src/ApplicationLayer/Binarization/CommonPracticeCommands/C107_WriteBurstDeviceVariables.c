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

#include "C107_WriteBurstDeviceVariables.h"

uint8_t Compose_C107_WriteBurstDeviceVariables_Req(C107_WriteBurstDeviceVariables_Req* request,
		ComposerContext* context, BinaryStream* toStream)
{
	context->cmdId = CMDID_C107_WriteBurstDeviceVariables;
#ifdef _DEBUG
// 	if (request->noOfDeviceVariables == 0 || 4 < request->noOfDeviceVariables)
// 		return RCS_E01_Undefined1;
#endif

	for (uint8_t i = 0; i < request->noOfDeviceVariables; i++)
	{
		STREAM_WRITE_UINT8(toStream, request->deviceVariableCode[i]);
	}
	STREAM_WRITE_UINT8(toStream, request->burstMessage);
	return RCS_N00_Success;
}



uint8_t Parse_C107_WriteBurstDeviceVariables_Req(C107_WriteBurstDeviceVariables_Req* request, ParserContext* context,
		BinaryStream* fromStream)
{
	request->noOfDeviceVariables = fromStream->remainingBytes -1;
	
	// truncated to 8 device variables 
	if(request->noOfDeviceVariables > MaxNoOfDeviceVariables)
		request->noOfDeviceVariables = MaxNoOfDeviceVariables;

	for (uint8_t i = 0; i < request->noOfDeviceVariables; i++)
	{
		STREAM_READ_UINT8(fromStream, &request->deviceVariableCode[i]);
	}
	for (uint8_t i =request->noOfDeviceVariables; i < MaxNoOfDeviceVariables; i++)
	{
		request->deviceVariableCode[i] = 0xFA;
	}
	
	if (request->noOfDeviceVariables == MaxNoOfDeviceVariables)
		{
		STREAM_READ_UINT8(fromStream, &request->burstMessage);
		}
	else 
		{
		request->burstMessage = 0;
		}
	return RCS_N00_Success;
}

uint8_t Compose_C107_WriteBurstDeviceVariables_Resp(C107_WriteBurstDeviceVariables_Resp* response,
		ComposerContext* context, BinaryStream* toStream)
{
	context->cmdId = CMDID_C107_WriteBurstDeviceVariables;
	for (uint8_t i = 0; i < MaxNoOfDeviceVariables; i++)
	{
		STREAM_WRITE_UINT8(toStream, response->deviceVariableCode[i]);
	}
	STREAM_WRITE_UINT8(toStream, response->burstMessage);
	return RCS_N00_Success;
}

uint8_t Parse_C107_WriteBurstDeviceVariables_Resp(C107_WriteBurstDeviceVariables_Resp* response,
		ParserContext* context, BinaryStream* fromStream)
{
	
	for (uint8_t i = 0; i < MaxNoOfDeviceVariables; i++)
	{
		STREAM_READ_UINT8(fromStream, &response->deviceVariableCode[i]);
	}
	STREAM_READ_UINT8(fromStream, &response->burstMessage);
	return RCS_N00_Success;
}
