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

#include "C033_ReadDeviceVariables.h"

uint8_t Compose_C033_ReadDeviceVariables_Req(C033_ReadDeviceVariables_Req* request, ComposerContext* context,
		BinaryStream* toStream)
{
	context->cmdId = CMDID_C033_ReadDeviceVariables;

#ifdef _DEBUG
// 	if (request->variablesSize == 0 || 4 < request->variablesSize)
// 		return RCS_E01_Undefined1;
#endif

	for (uint8_t i = 0; i < request->variablesSize; i++)
	{
		STREAM_WRITE_UINT8(toStream, request->variables[i]);
	}
	return RCS_N00_Success;
}

uint8_t Parse_C033_ReadDeviceVariables_Req(C033_ReadDeviceVariables_Req* request, ParserContext* context,
		BinaryStream* fromStream)
{
	request->variablesSize = MIN_VALUE(fromStream->remainingBytes, 4);
	for (uint8_t i = 0; i < request->variablesSize; i++)
	{
		STREAM_READ_UINT8(fromStream, &request->variables[i]);
	}

	return RCS_N00_Success;
}

uint8_t Compose_C033_ReadDeviceVariables_Resp(C033_ReadDeviceVariables_Resp* response, ComposerContext* context,
		BinaryStream* toStream)
{
	context->cmdId = CMDID_C033_ReadDeviceVariables;
#ifdef _DEBUG
// 	if (response->variablesSize == 0 || 4 < response->variablesSize)
// 		return RCS_E01_Undefined1;
#endif

	for (uint8_t i = 0; i < response->variablesSize; i++)
	{
		STREAM_WRITE_UINT8(toStream, response->variables[i].code);
		STREAM_WRITE_UINT8(toStream, response->variables[i].unitsCode);
              #ifdef ISAEVALKIT_EXCEPTION
                STREAM_WRITE_UINT8(toStream, response->variables[i].value[0]);    // only for Hannover trade show
                STREAM_WRITE_UINT8(toStream, response->variables[i].value[1]);
                STREAM_WRITE_UINT8(toStream, response->variables[i].value[2]);
                STREAM_WRITE_UINT8(toStream, response->variables[i].value[3]);
              #else
		STREAM_WRITE_FLOAT(toStream, response->variables[i].value);
              #endif
	}
	return RCS_N00_Success;
}

uint8_t Parse_C033_ReadDeviceVariables_Resp(C033_ReadDeviceVariables_Resp* response, ParserContext* context,
		BinaryStream* fromStream)
{
	response->variablesSize = MIN_VALUE(fromStream->remainingBytes / 6, 4);
	for (uint8_t i = 0; i < response->variablesSize; i++)
	{
		STREAM_READ_UINT8(fromStream, &response->variables[i].code);
		STREAM_READ_UINT8(fromStream, &response->variables[i].unitsCode);

              #ifdef ISAEVALKIT_EXCEPTION
                STREAM_READ_UINT8(fromStream, &response->variables[i].value[0]);  // only for Hannover
                STREAM_READ_UINT8(fromStream, &response->variables[i].value[1]);
                STREAM_READ_UINT8(fromStream, &response->variables[i].value[2]);
                STREAM_READ_UINT8(fromStream, &response->variables[i].value[3]);
              #else
		STREAM_READ_FLOAT(fromStream, &response->variables[i].value);
              #endif
	}
	return RCS_N00_Success;
}
