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

#include "C833_ReadNetworkDeviceNeighbourHealth.h"


uint8_t Compose_C833_ReadNetworkDeviceNeighbourHealth_Req(C833_ReadNetworkDeviceNeighbourHealth_Req* request, ComposerContext* context,
		BinaryStream* toStream)
{
	context->cmdId = CMDID_C833_ReadNetworkDeviceNeighbourHealth;

	STREAM_WRITE_BYTES(toStream, &request->UniqueID, 5);
	STREAM_WRITE_UINT8(toStream, request->NeighbourIndex);
	STREAM_WRITE_UINT8(toStream, request->NeighbourEntriesToRead);

	return RCS_N00_Success;
}

/**
 * @Debinarize
 */
uint8_t Parse_C833_ReadNetworkDeviceNeighbourHealth_Req(C833_ReadNetworkDeviceNeighbourHealth_Req* request, ParserContext* context,
		BinaryStream* fromStream)
{
#ifdef _DEBUG
// 	if (7 > fromStream->remainingBytes)
// 		return RCS_E05_TooFewDataBytesReceived;
#endif
	STREAM_READ_BYTES(fromStream, &request->UniqueID, 5);
	STREAM_READ_UINT8(fromStream, &request->NeighbourIndex);
	STREAM_READ_UINT8(fromStream, &request->NeighbourEntriesToRead);

	return RCS_N00_Success;
}

/**
 *
 */
uint8_t Compose_C833_ReadNetworkDeviceNeighbourHealth_Resp(C833_ReadNetworkDeviceNeighbourHealth_Resp* response, ComposerContext* context,
		BinaryStream* toStream)
{
	context->cmdId = CMDID_C833_ReadNetworkDeviceNeighbourHealth;

	STREAM_WRITE_BYTES(toStream, &response->UniqueID, 5);
	STREAM_WRITE_UINT8(toStream, response->NeighbourIndex);
	STREAM_WRITE_UINT8(toStream, response->NeighbourCount);
	for (int i = 0; i < response->NeighbourCount; i++)
	{
		STREAM_WRITE_UINT16(toStream, response->Neighbours[i].NeighbourNickname);
		STREAM_WRITE_UINT8(toStream, response->Neighbours[i].NeighbourRSL);
		STREAM_WRITE_UINT32(toStream, response->Neighbours[i].TransmittedPacketCount);
		STREAM_WRITE_UINT32(toStream, response->Neighbours[i].TransmittedPacketWithNoACKCount);
		STREAM_WRITE_UINT32(toStream, response->Neighbours[i].ReceivedPacketCount);
	}

	return RCS_N00_Success;
}

/**
 *
 */
uint8_t Parse_C833_ReadNetworkDeviceNeighbourHealth_Resp(C833_ReadNetworkDeviceNeighbourHealth_Resp* response, ParserContext* context,
		BinaryStream* fromStream)
{
//#ifdef _DEBUG
//	if (39 > fromStream->remainingBytes)
//		return RCS_E05_TooFewDataBytesReceived;
//#endif
	// ReqSize
	if(fromStream->remainingBytes < 22)
		{
			return RCS_E05_TooFewDataBytesReceived;
		}
	STREAM_READ_BYTES(fromStream, &response->UniqueID, 5);
	STREAM_READ_UINT8(fromStream, &response->NeighbourIndex);
	STREAM_READ_UINT8(fromStream, &response->NeighbourCount);

	// Already read 7 BYTES;  Mandatory 22 Bytes include data for first Neighbour
	if((fromStream->remainingBytes) < (response->NeighbourCount*15 ))
		{
			return RCS_E05_TooFewDataBytesReceived;
		}

	for (int i = 0; i < response->NeighbourCount; i++)
	{
		STREAM_READ_UINT16(fromStream, &response->Neighbours[i].NeighbourNickname);
		STREAM_READ_UINT8(fromStream, &response->Neighbours[i].NeighbourRSL);
		STREAM_READ_UINT32(fromStream, &response->Neighbours[i].TransmittedPacketCount);
		STREAM_READ_UINT32(fromStream, &response->Neighbours[i].TransmittedPacketWithNoACKCount);
		STREAM_READ_UINT32(fromStream, &response->Neighbours[i].ReceivedPacketCount);

	}

	return RCS_N00_Success;
}
