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

#include "C000_ReadUniqueIdentifier.h"



uint8_t Compose_C000_ReadUniqueIdentifier_Req(C000_ReadUniqueIdentifier_Req* request, ComposerContext* context,
                                              BinaryStream* toStream)
{
  context->cmdId = CMDID_C000_ReadUniqueIdentifier;
  return RCS_N00_Success;
}

uint8_t Parse_C000_ReadUniqueIdentifier_Req(C000_ReadUniqueIdentifier_Req* request, ParserContext* context,
                                            BinaryStream* fromStream)
{
  return RCS_N00_Success;
}

uint8_t Compose_C000_ReadUniqueIdentifier_Resp(C000_ReadUniqueIdentifier_Resp* response, ComposerContext* context,
                                               BinaryStream* toStream)
{
  context->cmdId = CMDID_C000_ReadUniqueIdentifier;

#ifdef _DEBUG
//   if(toStream->remainingBytes < C000_RespSize)
//     return RCS_E01_Undefined1;
#endif


  STREAM_WRITE_UINT8(toStream, C000_Byte0); // hardcoded by specification
  STREAM_WRITE_UINT16(toStream, response->expandedDeviceType);
  STREAM_WRITE_UINT8(toStream, response->minReqPreamblesNo);
  STREAM_WRITE_UINT8(toStream, response->protocolMajorRevNo);
  STREAM_WRITE_UINT8(toStream, response->deviceRevisionLevel);
  STREAM_WRITE_UINT8(toStream, response->softwareRevisionLevel);

  /*
  * 7 Unsigned-5 (Most Significant 5 Bits) Hardware Revision Level of the electronics in
  *	this particular device. Does Not Necessarily Trace Individual
  *	Component Changes. Level 31 is Reserved.
  * 7 Enum (Least Significant 3 Bits) Physical Signaling Code (see Common Table
  *	10, Physical Signaling Codes)
  */
  uint8_t byte7 = (((uint8_t)response->hardwareRevisionLevel << 3)|((uint8_t)(response->physicalSignalingCode & 0x07)));

  //STREAM_WRITE_UINT8(toStream, response->hardwareRevisionLevel);
  //STREAM_WRITE_UINT8(toStream, response->physicalSignalingCode);

  STREAM_WRITE_UINT8(toStream, byte7);
  STREAM_WRITE_UINT8(toStream, response->flags);
  STREAM_WRITE_UINT24(toStream, response->deviceID);

  // TODO: Beni - check Hart Revisions: 5, 6, 7 ...
  if (response->protocolMajorRevNo > 5)
  {
      STREAM_WRITE_UINT8(toStream, response->minRespPreamblesNo);
      STREAM_WRITE_UINT8(toStream, response->maxNoOfDeviceVars);
      STREAM_WRITE_UINT16(toStream, response->configChangeCounter);
      STREAM_WRITE_UINT8(toStream, response->extendedFieldDeviceStatus);
      STREAM_WRITE_UINT16(toStream, response->manufacturerIDCode);
      STREAM_WRITE_UINT16(toStream, response->privateLabelDistributorCode);
      STREAM_WRITE_UINT8(toStream, response->deviceProfile);
  }

  return RCS_N00_Success;
}

uint8_t Parse_C000_ReadUniqueIdentifier_Resp(C000_ReadUniqueIdentifier_Resp* response, ParserContext* context,
                                             BinaryStream* fromStream)
{
//   if (fromStream->remainingBytes < C000_RespSize)
//      {
//            return RCS_E01_Undefined1; //too few bytes
//      }
//   context->maxNeededParsedSize = sizeof(C000_ReadUniqueIdentifier_Resp);

  uint8_t hardcoded = 0;
  STREAM_READ_INT8(fromStream, &hardcoded); //read harcoded value 254
  if(hardcoded != C000_Byte0)
	    return RCS_E01_Undefined1; //FIXME [nicu.dascalu] - hardcoded value.


  STREAM_READ_UINT16(fromStream, &response->expandedDeviceType);
  STREAM_READ_UINT8(fromStream, &response->minReqPreamblesNo);
  STREAM_READ_UINT8(fromStream, &response->protocolMajorRevNo);
  STREAM_READ_UINT8(fromStream, &response->deviceRevisionLevel);
  STREAM_READ_UINT8(fromStream, &response->softwareRevisionLevel);

  uint8_t byte7;
  STREAM_READ_UINT8(fromStream, &byte7);
  response->hardwareRevisionLevel = (uint8_t)((byte7 & 0xF8) >> 3);     // bits 7 to 3
  response->physicalSignalingCode = (uint8_t)(byte7 & 0x07);            // bits 2 to 0

  STREAM_READ_UINT8(fromStream, &response->flags);
  STREAM_READ_UINT24(fromStream, &response->deviceID);

  // TODO: Beni - check Hart Revisions: 5, 6, 7 ...
  if (response->protocolMajorRevNo > 5)
  {
      STREAM_READ_UINT8(fromStream, &response->minRespPreamblesNo);
      STREAM_READ_UINT8(fromStream, &response->maxNoOfDeviceVars);
      STREAM_READ_UINT16(fromStream, &response->configChangeCounter);
      STREAM_READ_UINT8(fromStream, &response->extendedFieldDeviceStatus);

      int neededSize = sizeof(response->manufacturerIDCode) + sizeof(response->privateLabelDistributorCode) + sizeof(response->deviceProfile);
      if (fromStream->remainingBytes >= neededSize)
      {
        STREAM_READ_UINT16(fromStream, &response->manufacturerIDCode);
        STREAM_READ_UINT16(fromStream, &response->privateLabelDistributorCode);
        STREAM_READ_UINT8(fromStream, &response->deviceProfile);
      }
      else
      {
        response->manufacturerIDCode = 0;
        response->privateLabelDistributorCode = 0;
        response->deviceProfile = 0;
      }
  }

  return RCS_N00_Success;
}
