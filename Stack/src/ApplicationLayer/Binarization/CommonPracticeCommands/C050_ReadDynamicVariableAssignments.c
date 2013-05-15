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

#include "C050_ReadDynamicVariableAssignments.h"

#include "../../Model/CommonPracticeCommands.h"
#include "../../../util/Binarization.h"

#ifdef __cplusplus
extern "C"
{
#endif

uint8_t Compose_C050_ReadDynamicVariableAssignments_Req(C050_ReadDynamicVariableAssignments_Req* request,
		ComposerContext* context, BinaryStream* toStream)
{
	context->cmdId = CMDID_C050_ReadDynamicVariableAssignments;

	return RCS_N00_Success;
}

uint8_t Parse_C050_ReadDynamicVariableAssignments_Req(C050_ReadDynamicVariableAssignments_Req* request,
		ParserContext* context, BinaryStream* fromStream)
{
	return RCS_N00_Success;
}

uint8_t Compose_C050_ReadDynamicVariableAssignments_Resp(C050_ReadDynamicVariableAssignments_Resp* response,
		ComposerContext* context, BinaryStream* toStream)
{
	context->cmdId = CMDID_C050_ReadDynamicVariableAssignments;

	for (uint8_t i = 0; i < C050_RespSize; i++)
	{
		STREAM_WRITE_UINT8(toStream, response->variables[i]);
	}
	return RCS_N00_Success;
}

uint8_t Parse_C050_ReadDynamicVariableAssignments_Resp(C050_ReadDynamicVariableAssignments_Resp* response,
		ParserContext* context, BinaryStream* fromStream)
{

	for (uint8_t i = 0; i < C050_RespSize; i++)
	{
		STREAM_READ_UINT8(fromStream, &response->variables[i]);
	}
	return RCS_N00_Success;
}

#ifdef __cplusplus
}
#endif
