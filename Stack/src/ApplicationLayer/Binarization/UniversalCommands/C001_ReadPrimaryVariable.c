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

#include "C001_ReadPrimaryVariable.h"

uint8_t Compose_C001_ReadPrimaryVariable_Req(C001_ReadPrimaryVariable_Req* request, ComposerContext* context,
		BinaryStream* toStream)
{
	context->cmdId = CMDID_C001_ReadPrimaryVariable;
	return RCS_N00_Success;
}

uint8_t Parse_C001_ReadPrimaryVariable_Req(C001_ReadPrimaryVariable_Req* request, ParserContext* context,
		BinaryStream* fromStream)
{
	return RCS_N00_Success;
}

uint8_t Compose_C001_ReadPrimaryVariable_Resp(C001_ReadPrimaryVariable_Resp* response, ComposerContext* context,
		BinaryStream* toStream)
{
#ifdef _DEBUG
//   if(toStream->remainingBytes < C001_RespSize)
//     return RCS_E01_Undefined1;
#endif

	context->cmdId = CMDID_C001_ReadPrimaryVariable;

	STREAM_WRITE_UINT8(toStream, response->primaryVariableUnit);
	STREAM_WRITE_FLOAT(toStream, response->primaryVariable);

	return RCS_N00_Success;
}

uint8_t Parse_C001_ReadPrimaryVariable_Resp(C001_ReadPrimaryVariable_Resp* response, ParserContext* context,
		BinaryStream* fromStream)
{
	 STREAM_READ_UINT8(fromStream, &response->primaryVariableUnit);
	 STREAM_READ_FLOAT(fromStream, &response->primaryVariable);

	return RCS_N00_Success;
}
