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

#include "C779_ReportDeviceHealth.h"

uint8_t Compose_C779_ReportDeviceHealth_Req ( C779_ReportDeviceHealth_Req* request,
                                    ComposerContext* context,
                                    BinaryStream* toStream )
{
  context->cmdId = CMDID_C779_ReportDeviceHealth;

  return RCS_N00_Success; // success
}

uint8_t Parse_C779_ReportDeviceHealth_Req ( C779_ReportDeviceHealth_Req* request,
                                    ParserContext* context,
		                    BinaryStream* fromStream )
{
#if defined(_DEBUG)
//   if (fromStream->remainingBytes > 0)
//     return RCS_E01_Undefined1; // too many bytes
#endif

  return RCS_N00_Success; // success
}

uint8_t Compose_C779_ReportDeviceHealth_Resp ( C779_ReportDeviceHealth_Resp* response,
                                    ComposerContext* context,
                                    BinaryStream* toStream )
{
  context->cmdId = CMDID_C779_ReportDeviceHealth;
  STREAM_WRITE_UINT16(toStream, response->m_unNoOfPacketsGeneratedByDevice);
  STREAM_WRITE_UINT16(toStream, response->m_unNoOfPacketsTerminatedByDevice);
  STREAM_WRITE_UINT8(toStream, response->m_ucNoOfDataLinkLayerMICFailuresDetected);
  STREAM_WRITE_UINT8(toStream, response->m_ucNoOfNetworkLayerMICFailuresDetected);
  STREAM_WRITE_UINT8(toStream, response->m_ucPowerStatus);
  STREAM_WRITE_UINT8(toStream, response->m_ucNoOfCRCErrorsDetected);
  STREAM_WRITE_UINT8(toStream, response->m_ucNoOfNonceCounterValuesNotReceived);

  return RCS_N00_Success; // success
}

uint8_t Parse_C779_ReportDeviceHealth_Resp ( C779_ReportDeviceHealth_Resp* response,
                                    ParserContext* context,
		                    BinaryStream* fromStream )
{
#if defined(_DEBUG)
//   if (fromStream->remainingBytes > C779_RespSize)
//     return RCS_E01_Undefined1; // too many bytes
#endif

  STREAM_READ_UINT16(fromStream, &response->m_unNoOfPacketsGeneratedByDevice);
  STREAM_READ_UINT16(fromStream, &response->m_unNoOfPacketsTerminatedByDevice);
  STREAM_READ_UINT8(fromStream, &response->m_ucNoOfDataLinkLayerMICFailuresDetected);
  STREAM_READ_UINT8(fromStream, &response->m_ucNoOfNetworkLayerMICFailuresDetected);
  STREAM_READ_UINT8(fromStream, &response->m_ucPowerStatus);
  STREAM_READ_UINT8(fromStream, &response->m_ucNoOfCRCErrorsDetected);
  STREAM_READ_UINT8(fromStream, &response->m_ucNoOfNonceCounterValuesNotReceived);

  return RCS_N00_Success; // success
}
