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

#include "C048_ReadAdditionalDeviceStatus.h"


uint8_t Compose_C048_ReadAdditionalDeviceStatus_Req(C048_ReadAdditionalDeviceStatus_Req* request,
			ComposerContext* context, BinaryStream* toStream)
{
	context->cmdId = CMDID_C048_ReadAdditionalDeviceStatus;

	int lenToWrite = request->reqLen;
	int varSize;

	varSize = sizeof(request->deviceSpecificStatus1);
	if (lenToWrite >= varSize)
	{
		STREAM_WRITE_BYTES(toStream, request->deviceSpecificStatus1, varSize);
		lenToWrite -= varSize;
	}
	else return RCS_N00_Success;
	
	varSize = sizeof(request->extendedDeviceStatus);
	if (lenToWrite >= varSize)
	{	
		STREAM_WRITE_UINT8(toStream, request->extendedDeviceStatus);
		lenToWrite -= varSize;
	}
	else return RCS_N00_Success;

	varSize = sizeof(request->deviceOperatingMode);
	if (lenToWrite >= varSize)
	{	
		STREAM_WRITE_UINT8(toStream, request->deviceOperatingMode);
		lenToWrite -= varSize;
	}
	else return RCS_N00_Success;

	varSize = sizeof(request->standardizedStatus0);
	if (lenToWrite >= varSize)
	{	
		STREAM_WRITE_UINT8(toStream, request->standardizedStatus0);
		lenToWrite -= varSize;
	}
	else return RCS_N00_Success;

	varSize = sizeof(request->standardizedStatus1);
	if (lenToWrite >= varSize)
	{	
		STREAM_WRITE_UINT8(toStream, request->standardizedStatus1);
		lenToWrite -= varSize;
	}
	else return RCS_N00_Success;

	varSize = sizeof(request->analogChannelSaturatedCode);
	if (lenToWrite >= varSize)
	{	
		STREAM_WRITE_UINT8(toStream, request->analogChannelSaturatedCode);
		lenToWrite -= varSize;
	}
	else return RCS_N00_Success;

	varSize = sizeof(request->standardizedStatus2);
	if (lenToWrite >= varSize)
	{	
		STREAM_WRITE_UINT8(toStream, request->standardizedStatus2);
		lenToWrite -= varSize;
	}
	else return RCS_N00_Success;

	varSize = sizeof(request->standardizedStatus3);
	if (lenToWrite >= varSize)
	{	
		STREAM_WRITE_UINT8(toStream, request->standardizedStatus3);
		lenToWrite -= varSize;
	}
	else return RCS_N00_Success;

	varSize = sizeof(request->analogChannelFixedCode);
	if (lenToWrite >= varSize)
	{	
		STREAM_WRITE_UINT8(toStream, request->analogChannelFixedCode);
		lenToWrite -= varSize;
	}
	else return RCS_N00_Success;

	if (lenToWrite > 0)
	{	STREAM_WRITE_BYTES(toStream, request->deviceSpecificStatus2, lenToWrite);
	}

	return RCS_N00_Success;
}

uint8_t Parse_C048_ReadAdditionalDeviceStatus_Req(C048_ReadAdditionalDeviceStatus_Req* request,
			ParserContext* context, BinaryStream* fromStream)
{

	memset(request, 0, sizeof(C048_ReadAdditionalDeviceStatus_Req));

	int bytesInStream = fromStream->remainingBytes;

	// HART rev no 5 or 6
	if (bytesInStream == C048_ReqSize_5RevNo ||  bytesInStream == C048_ReqSize_6RevNo)
	{	memset(request, 0xFF, sizeof(C048_ReadAdditionalDeviceStatus_Req));
		return RCS_N00_Success;
	}

	request->reqLen = (uint8_t)(bytesInStream);

	if (bytesInStream < C048_ReqSize_7RevNo)
	{	return RCS_E05_TooFewDataBytesReceived;
	}

	int varSize = sizeof(request->deviceSpecificStatus1);
	if (bytesInStream >= varSize)
	{	STREAM_READ_BYTES(fromStream, &request->deviceSpecificStatus1, varSize)
		bytesInStream -= varSize;
	}
	else return RCS_N00_Success; //success

	varSize = sizeof(request->extendedDeviceStatus);
	if (bytesInStream >= varSize)
	{	STREAM_READ_BYTES(fromStream, &request->extendedDeviceStatus, varSize)
		bytesInStream -= varSize;
	}
	else return RCS_N00_Success; //success

	varSize = sizeof(request->deviceOperatingMode);
	if (bytesInStream >= varSize)
	{	STREAM_READ_BYTES(fromStream, &request->deviceOperatingMode, varSize)
		bytesInStream -= varSize;
	}
	else return RCS_N00_Success; //success

	varSize = sizeof(request->standardizedStatus0);
	if (bytesInStream >= varSize)
	{	STREAM_READ_BYTES(fromStream, &request->standardizedStatus0, varSize)
		bytesInStream -= varSize;
	}
	else return RCS_N00_Success; //success

	varSize = sizeof(request->standardizedStatus1);
	if (bytesInStream >= varSize)
	{	STREAM_READ_BYTES(fromStream, &request->standardizedStatus1, varSize)
		bytesInStream -= varSize;
	}
	else return RCS_N00_Success; //success

	varSize = sizeof(request->analogChannelSaturatedCode);
	if (bytesInStream >= varSize)
	{	STREAM_READ_BYTES(fromStream, &request->analogChannelSaturatedCode, varSize)
		bytesInStream -= varSize;
	}
	else return RCS_N00_Success; //success

	varSize = sizeof(request->standardizedStatus2);
	if (bytesInStream >= varSize)
	{	STREAM_READ_BYTES(fromStream, &request->standardizedStatus2, varSize)
		bytesInStream -= varSize;
	}
	else return RCS_N00_Success; //success

	varSize = sizeof(request->standardizedStatus3);
	if (bytesInStream >= varSize)
	{	STREAM_READ_BYTES(fromStream, &request->standardizedStatus3, varSize)
		bytesInStream -= varSize;
	}
	else return RCS_N00_Success; //success

	varSize = sizeof(request->analogChannelFixedCode);
	if (bytesInStream >= varSize)
	{	STREAM_READ_BYTES(fromStream, &request->analogChannelFixedCode, varSize)
		bytesInStream -= varSize;
	}
	else return RCS_N00_Success; //success

	varSize = (bytesInStream > sizeof(request->deviceSpecificStatus2)) ? sizeof(request->deviceSpecificStatus2) : bytesInStream;
	if (bytesInStream > 0)
	{	STREAM_READ_BYTES(fromStream, &request->deviceSpecificStatus2, varSize);
		bytesInStream -= varSize;
	}

	return RCS_N00_Success; //success
}

uint8_t Compose_C048_ReadAdditionalDeviceStatus_Resp(C048_ReadAdditionalDeviceStatus_Resp* response,
		ComposerContext* context, BinaryStream* toStream)
{
	context->cmdId = CMDID_C048_ReadAdditionalDeviceStatus;

	int lenToWrite = response->respLen;
	int varSize;

	varSize = sizeof(response->deviceSpecificStatus1);
	if (lenToWrite >= varSize)
	{
		STREAM_WRITE_BYTES(toStream, response->deviceSpecificStatus1, varSize)
		lenToWrite -= varSize;
	}
	else return RCS_N00_Success;


	varSize = sizeof(response->extendedDeviceStatus);
	if (lenToWrite >= varSize)
	{
		STREAM_WRITE_UINT8(toStream, response->extendedDeviceStatus);
		lenToWrite -= varSize;
	}
	else return RCS_N00_Success;

	
	varSize = sizeof(response->deviceOperatingMode);
	if (lenToWrite >= varSize)
	{
		STREAM_WRITE_UINT8(toStream, response->deviceOperatingMode);
		lenToWrite -= varSize;
	}
	else return RCS_N00_Success;

	varSize = sizeof(response->standardizedStatus0);
	if (lenToWrite >= varSize)
	{
		STREAM_WRITE_UINT8(toStream, response->standardizedStatus0);
		lenToWrite -= varSize;
	}
	else return RCS_N00_Success;

	varSize = sizeof(response->standardizedStatus1);
	if (lenToWrite >= varSize)
	{
		STREAM_WRITE_UINT8(toStream, response->standardizedStatus1);
		lenToWrite -= varSize;
	}
	else return RCS_N00_Success;

	varSize = sizeof(response->analogChannelSaturatedCode);
	if (lenToWrite >= varSize)
	{
		STREAM_WRITE_UINT8(toStream, response->analogChannelSaturatedCode);
		lenToWrite -= varSize;
	}
	else return RCS_N00_Success;

	varSize = sizeof(response->standardizedStatus2);
	if (lenToWrite >= varSize)
	{
		STREAM_WRITE_UINT8(toStream, response->standardizedStatus2);
		lenToWrite -= varSize;
	}
	else return RCS_N00_Success;

	varSize = sizeof(response->standardizedStatus3);
	if (lenToWrite >= varSize)
	{
		STREAM_WRITE_UINT8(toStream, response->standardizedStatus3);
		lenToWrite -= varSize;
	}
	else return RCS_N00_Success;


	varSize = sizeof(response->analogChannelFixedCode);
	if (lenToWrite >= varSize)
	{
		STREAM_WRITE_UINT8(toStream, response->analogChannelFixedCode);
		lenToWrite -= varSize;
	}
	else return RCS_N00_Success;

	if (lenToWrite > 0)
	{
		STREAM_WRITE_BYTES(toStream, response->deviceSpecificStatus2, lenToWrite );
		lenToWrite -= varSize;
	}

	return RCS_N00_Success;
}

uint8_t Parse_C048_ReadAdditionalDeviceStatus_Resp(C048_ReadAdditionalDeviceStatus_Resp* response,
		ParserContext* context, BinaryStream* fromStream)
{
	memset(response, 0, sizeof(C048_ReadAdditionalDeviceStatus_Resp));

	int streamSize = fromStream->remainingBytes;
	response->respLen = (uint8_t)(streamSize);

	if (streamSize < 9)
	{	return RCS_E05_TooFewDataBytesReceived;
	}
	else if ((streamSize >= 9) && (streamSize < 14))
		response->customCommandSize = 9;
	else if (streamSize == 14)
		response->customCommandSize = 14;
	else if ((streamSize > 14 ) && (streamSize < 25))
		response->customCommandSize = streamSize;
	else 	response->customCommandSize = 25;

	int varSize = sizeof(response->deviceSpecificStatus1);
	if (streamSize >= varSize)
	{	STREAM_READ_BYTES(fromStream, &response->deviceSpecificStatus1, varSize);
		streamSize -= varSize;
	}
	else return RCS_N00_Success;

	varSize = sizeof(response->extendedDeviceStatus);
	if (streamSize >= varSize)
	{	STREAM_READ_UINT8(fromStream, &response->extendedDeviceStatus);
		streamSize -= varSize;
	}
	else return RCS_N00_Success;

	varSize = sizeof(response->deviceOperatingMode);
	if (streamSize >= varSize)
	{	STREAM_READ_UINT8(fromStream, &response->deviceOperatingMode);
		streamSize -= varSize;
	}
	else return RCS_N00_Success;

	varSize = sizeof(response->standardizedStatus0);
	if (streamSize >= varSize)
	{	STREAM_READ_UINT8(fromStream, &response->standardizedStatus0);
		streamSize -= varSize;
	}
	else return RCS_N00_Success;

	varSize = sizeof(response->standardizedStatus1);
	if (streamSize >= varSize)
	{	STREAM_READ_UINT8(fromStream, &response->standardizedStatus1);
		streamSize -= varSize;
	}
	else return RCS_N00_Success;

	varSize = sizeof(response->analogChannelSaturatedCode);
	if (streamSize >= varSize)
	{	STREAM_READ_UINT8(fromStream, &response->analogChannelSaturatedCode);
		streamSize -= varSize;
	}
	else return RCS_N00_Success;

	varSize = sizeof(response->standardizedStatus2);
	if (streamSize >= varSize)
	{	STREAM_READ_UINT8(fromStream, &response->standardizedStatus2);
		streamSize -= varSize;
	}
	else return RCS_N00_Success;

	varSize = sizeof(response->standardizedStatus3);
	if (streamSize >= varSize)
	{	STREAM_READ_UINT8(fromStream, &response->standardizedStatus3);
		streamSize -= varSize;
	}
	else return RCS_N00_Success;

	varSize = sizeof(response->analogChannelFixedCode);
	if (streamSize >= varSize)
	{	STREAM_READ_UINT8(fromStream, &response->analogChannelFixedCode);
		streamSize -= varSize;
	}		
	else return RCS_N00_Success;

	varSize = (streamSize > sizeof(response->deviceSpecificStatus2)) ? sizeof(response->deviceSpecificStatus2) : streamSize;
	if (streamSize > 0)
	{	STREAM_READ_BYTES(fromStream, &response->deviceSpecificStatus2,	varSize);
	}

	return RCS_N00_Success; //success
}

