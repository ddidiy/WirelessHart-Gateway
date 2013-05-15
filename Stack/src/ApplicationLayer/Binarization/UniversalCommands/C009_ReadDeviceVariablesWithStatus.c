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

#include "C009_ReadDeviceVariablesWithStatus.h"

uint8_t Compose_C009_ReadDeviceVariablesWithStatus_Req(C009_ReadDeviceVariablesWithStatus_Req* request,
		ComposerContext* context, BinaryStream* toStream)
{
	context->cmdId = CMDID_C009_ReadDeviceVariablesWithStatus;

#ifdef _DEBUG
// 	if (request->variablesSize < 1 || 8 < request->variablesSize)
// 		return RCS_E01_Undefined1;
#endif

	//CHECKME [nicu.dascalu] - can be optimized with WRITE_BYTES ...
	for (int i = 0; i < request->variablesSize; i++)
	{
		STREAM_WRITE_UINT8(toStream, request->slotDVC[i]);
	}

	return RCS_N00_Success;
}

uint8_t Parse_C009_ReadDeviceVariablesWithStatus_Req(C009_ReadDeviceVariablesWithStatus_Req* request,
		ParserContext* constext, BinaryStream* fromStream)
{
	//CHECKME [nicu.dascalu] - can be optimized with WRITE_BYTES ...
	request->variablesSize = fromStream->remainingBytes > 8 ? 8 : fromStream->remainingBytes;
	for (int i = 0; i < request->variablesSize; i++)
	{
		 STREAM_READ_UINT8(fromStream, &request->slotDVC[i]);
	}

	return RCS_N00_Success;
}

uint8_t Compose_C009_ReadDeviceVariablesWithStatus_Resp(C009_ReadDeviceVariablesWithStatus_Resp* response,
		ComposerContext* context, BinaryStream* toStream)
{
	context->cmdId = CMDID_C009_ReadDeviceVariablesWithStatus;

#ifdef _DEBUG
// 	if (response->variablesSize < 1 || 8 < response->variablesSize)
// 		return RCS_E01_Undefined1;
#endif

	/*
	 * 1-> 0 Unsigned-8 Slot 0: Device Variable Code
	 * 8-> Slot structure
	 * 4-> Time
	 */

	STREAM_WRITE_UINT8(toStream, response->extendedFieldDeviceStatus);

	for (int i = 0; i < response->variablesSize; i++)
	{
		STREAM_WRITE_UINT8(toStream, response->slots[i].deviceVariableCode);
		STREAM_WRITE_UINT8(toStream, response->slots[i].deviceVariableClassificationCode);
		STREAM_WRITE_UINT8(toStream,response->slots[i].unitCode);
		STREAM_WRITE_FLOAT(toStream,response->slots[i].deviceVariableValue);
		STREAM_WRITE_UINT8(toStream,response->slots[i].deviceVariableStatus);
	}

	STREAM_WRITE_TIME(toStream,response->slot0TimeStamp);

	return RCS_N00_Success;
}

uint8_t Parse_C009_ReadDeviceVariablesWithStatus_Resp(C009_ReadDeviceVariablesWithStatus_Resp* response,
		ParserContext* constext, BinaryStream* fromStream)
{
	/*
	 * 1-> 0 Unsigned-8 Slot 0: Device Variable Code
	 * 8-> Slot structure
	 * 4-> Time
	 */

	memset(response, 0, sizeof(C009_ReadDeviceVariablesWithStatus_Resp));

	if (fromStream->remainingBytes >= sizeof(response->extendedFieldDeviceStatus))
	{
	    STREAM_READ_UINT8(fromStream, &response->extendedFieldDeviceStatus);
	}

	while (fromStream->remainingBytes >= 8/*slot size*/)
	{
	    if (response->variablesSize < 8)
	    {
            STREAM_READ_UINT8(fromStream, &response->slots[response->variablesSize].deviceVariableCode);
            STREAM_READ_UINT8(fromStream, &response->slots[response->variablesSize].deviceVariableClassificationCode);
            STREAM_READ_UINT8(fromStream, &response->slots[response->variablesSize].unitCode);
            STREAM_READ_FLOAT(fromStream, &response->slots[response->variablesSize].deviceVariableValue);
            STREAM_READ_UINT8(fromStream, &response->slots[response->variablesSize].deviceVariableStatus);
	    }
	    else
	    {
	        Slot dummySlot;
            STREAM_READ_UINT8(fromStream, &dummySlot.deviceVariableCode);
            STREAM_READ_UINT8(fromStream, &dummySlot.deviceVariableClassificationCode);
            STREAM_READ_UINT8(fromStream, &dummySlot.unitCode);
            STREAM_READ_FLOAT(fromStream, &dummySlot.deviceVariableValue);
            STREAM_READ_UINT8(fromStream, &dummySlot.deviceVariableStatus);
	    }

		response->variablesSize++;
	}

	if (response->variablesSize == 0)
		return RCS_E05_TooFewDataBytesReceived;

	if (response->variablesSize > 8)
		response->variablesSize = 8;

	if (fromStream->remainingBytes >= sizeof(response->slot0TimeStamp))
	{
	    STREAM_READ_TIME(fromStream, &response->slot0TimeStamp);
	}

	return RCS_N00_Success;
}

