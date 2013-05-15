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

#include "C823_RequestSession.h"

uint8_t Compose_C823_RequestSession_Req(C823_RequestSession_Req* request, ComposerContext* context, BinaryStream* toStream)
{
  context->cmdId = CMDID_C823_RequestSession;

  STREAM_WRITE_UINT16(toStream, request->m_unPeerDeviceNickname);

  return RCS_N00_Success;
}

uint8_t Parse_C823_RequestSession_Req(C823_RequestSession_Req* request, ParserContext* context, BinaryStream* fromStream)
{
#if defined(_DEBUG)
//   if (fromStream->remainingBytes > C823_ReqSize)
//     return RCS_E01_Undefined1; // too many bytes
#endif

  STREAM_READ_UINT16(fromStream, &request->m_unPeerDeviceNickname);

  return RCS_N00_Success;
}

uint8_t Compose_C823_RequestSession_Resp(C823_RequestSession_Resp* response, ComposerContext* context,
                                       BinaryStream* toStream)
{
  context->cmdId = CMDID_C823_RequestSession;

  STREAM_WRITE_UINT16(toStream, response->m_unPeerDeviceNickname);
  STREAM_WRITE_UINT32(toStream, response->m_ulPeerDeviceNonceCounterVal);
  for (uint8_t i = 0; i < 16; i++)
  {
    STREAM_WRITE_UINT8(toStream, response->m_aKeyVal[i]);
  }

  return RCS_N00_Success;
}

uint8_t Parse_C823_RequestSession_Resp(C823_RequestSession_Resp* response, ParserContext* context,
                                     BinaryStream* fromStream)
{
#if defined(_DEBUG)
//   if (fromStream->remainingBytes > C823_RespSize)
//     return RCS_E01_Undefined1; // too many bytes
#endif

  STREAM_READ_UINT16(fromStream, &response->m_unPeerDeviceNickname);
  STREAM_READ_UINT32(fromStream, &response->m_ulPeerDeviceNonceCounterVal);
  for (uint8_t i = 0; i < 16; i++)
  {
    STREAM_READ_UINT8(fromStream, &response->m_aKeyVal[i]);
  }

  return RCS_N00_Success;
}
