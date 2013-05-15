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

#include "C840_ReadDeviceStatistics.h"


uint8_t Compose_C840_ReadDeviceStatistics_Req(C840_ReadDeviceStatistics_Req* request, ComposerContext* context,
		BinaryStream* toStream)
{
	context->cmdId = CMDID_C840_ReadDeviceStatistics;

	STREAM_WRITE_BYTES(toStream, &request->UniqueID, 5);

	return RCS_N00_Success;
}

/**
 * @Debinarize
 */
uint8_t Parse_C840_ReadDeviceStatistics_Req(C840_ReadDeviceStatistics_Req* request, ParserContext* context,
		BinaryStream* fromStream)
{
#ifdef _DEBUG
// 	if (5 > fromStream->remainingBytes)
// 		return RCS_E05_TooFewDataBytesReceived;
#endif
	STREAM_READ_BYTES(fromStream, &request->UniqueID, 5);

	return RCS_N00_Success;
}

/**
 *
 */
uint8_t Compose_C840_ReadDeviceStatistics_Resp(C840_ReadDeviceStatistics_Resp* response, ComposerContext* context,
		BinaryStream* toStream)
{
	context->cmdId = CMDID_C840_ReadDeviceStatistics;

	STREAM_WRITE_BYTES(toStream, &response->UniqueID, 5);

	STREAM_WRITE_UINT16(toStream, response->ActiveGraphsNo);
	STREAM_WRITE_UINT16(toStream, response->ActiveFramesNo);
	STREAM_WRITE_UINT16(toStream, response->ActiveLinksNo);
	STREAM_WRITE_UINT8(toStream, response->NeighboursNo);
	STREAM_WRITE_TIME(toStream, response->AverageLatency);

	STREAM_WRITE_UINT16(toStream, response->JoinCount);
	STREAM_WRITE_DATE(toStream, response->LastJoinTime);
	STREAM_WRITE_UINT32(toStream, response->LastJoinTimeToday);

	STREAM_WRITE_UINT32(toStream, response->DeviceGeneratedPackagesNo);
	STREAM_WRITE_UINT32(toStream, response->DeviceTerminatedPackagesNo);
	STREAM_WRITE_UINT32(toStream, response->DataLinkLayerMicFailuresNo);
	STREAM_WRITE_UINT32(toStream, response->NetworkLayerMicFailuresNo);
	STREAM_WRITE_UINT32(toStream, response->CRCErrorsNo);
	STREAM_WRITE_UINT32(toStream, response->NonceCounterValuesNotReceivedByDeviceNo);
	STREAM_WRITE_UINT32(toStream, response->NonceCounterValuesNotReceivedFromDeviceNo);
	STREAM_WRITE_TIME(toStream, response->StandardDeviationOfLatency);

	return RCS_N00_Success;
}

/**
 *
 */
uint8_t Parse_C840_ReadDeviceStatistics_Resp(C840_ReadDeviceStatistics_Resp* response, ParserContext* context,
		BinaryStream* fromStream)
{
#ifdef _DEBUG
// 	if (25 > fromStream->remainingBytes)
// 		return RCS_E05_TooFewDataBytesReceived;
#endif

	STREAM_READ_BYTES(fromStream, &response->UniqueID, 5);

	STREAM_READ_UINT16(fromStream, &response->ActiveGraphsNo);
	STREAM_READ_UINT16(fromStream, &response->ActiveFramesNo);
	STREAM_READ_UINT16(fromStream, &response->ActiveLinksNo);
	STREAM_READ_UINT8(fromStream, &response->NeighboursNo);
	STREAM_READ_TIME(fromStream, &response->AverageLatency);

	STREAM_READ_UINT16(fromStream, &response->JoinCount);
	STREAM_READ_DATE(fromStream, &response->LastJoinTime);
	STREAM_READ_UINT32(fromStream, &response->LastJoinTimeToday);

    STREAM_READ_UINT32(fromStream, &response->DeviceGeneratedPackagesNo);
    STREAM_READ_UINT32(fromStream, &response->DeviceTerminatedPackagesNo);
    STREAM_READ_UINT32(fromStream, &response->DataLinkLayerMicFailuresNo);
    STREAM_READ_UINT32(fromStream, &response->NetworkLayerMicFailuresNo);
    STREAM_READ_UINT32(fromStream, &response->CRCErrorsNo);
    STREAM_READ_UINT32(fromStream, &response->NonceCounterValuesNotReceivedByDeviceNo);
    STREAM_READ_UINT32(fromStream, &response->NonceCounterValuesNotReceivedFromDeviceNo);
    STREAM_READ_TIME(fromStream, &response->StandardDeviationOfLatency);

	return RCS_N00_Success;
}
