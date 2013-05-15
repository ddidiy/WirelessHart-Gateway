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

////////////////////////////////////////////////////////////////////////////////
/// @file Packet.cpp
/// @author Costin Grigorescu <costin.grigorescu@nivis.com>, (C) 2010
/// @brief C++ implementation: IpPacket
////////////////////////////////////////////////////////////////////////////////

#include "Packet.h"

uint16_t IpPacket::Checksum(const uint8_t *p_pData, uint16_t p_unDataLen, uint16_t p_unPrevCheckSum )
{
	uint32_t t = p_unPrevCheckSum ^ 0xFFFF;

	while(p_unDataLen--)
	{
		t += ((uint16_t)*(p_pData++)) << 8;
		if( !(p_unDataLen --) ) break;
		t += *(p_pData++);
	}
	uint16_t unResult = ~((uint16_t)t + (uint16_t)(t >> 16));
	return unResult;
}
