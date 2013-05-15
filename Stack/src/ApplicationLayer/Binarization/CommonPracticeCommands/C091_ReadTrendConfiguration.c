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

#include "C091_ReadTrendConfiguration.h"

uint8_t Compose_C091_ReadTrendConfiguration_Req(C091_ReadTrendConfiguration_Req* request, ComposerContext* context,
		BinaryStream* toStream)
{
	context->cmdId = CMDID_C091_ReadTrendConfiguration;
	STREAM_WRITE_UINT8(toStream, request->trendNo);
	return RCS_N00_Success;
}

uint8_t Parse_C091_ReadTrendConfiguration_Req(C091_ReadTrendConfiguration_Req* request, ParserContext* context,
		BinaryStream* fromStream)
{
	STREAM_READ_UINT8(fromStream, &request->trendNo);

	return RCS_N00_Success;
}

uint8_t Compose_C091_ReadTrendConfiguration_Resp(C091_ReadTrendConfiguration_Resp* response, ComposerContext* context,
		BinaryStream* toStream)
{
	context->cmdId = CMDID_C091_ReadTrendConfiguration;
	STREAM_WRITE_UINT8(toStream, response->trendNo);
	STREAM_WRITE_UINT8(toStream, response->totalNoOfTrendsSupported);
	STREAM_WRITE_UINT8(toStream, response->trendControlCode);
	STREAM_WRITE_UINT8(toStream, response->deviceVariableCode);
	STREAM_WRITE_TIME(toStream, response->trendSampleInterval);
	return RCS_N00_Success;
}

uint8_t Parse_C091_ReadTrendConfiguration_Resp(C091_ReadTrendConfiguration_Resp* response, ParserContext* context,
		BinaryStream* fromStream)
{
	STREAM_READ_UINT8(fromStream, &response->trendNo);
	STREAM_READ_UINT8(fromStream, &response->totalNoOfTrendsSupported);
	STREAM_READ_UINT8(fromStream, &response->trendControlCode);
	STREAM_READ_UINT8(fromStream, &response->deviceVariableCode);
	STREAM_READ_TIME(fromStream, &response->trendSampleInterval);

	return RCS_N00_Success;
}
