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

/*
 * WHartTypes.cpp
 *
 *  Created on: Dec 19, 2008
 *      Author: nicu.dascalu
 */
#include <WHartStack/WHartTypes.h>
#include <cstring>
#include <nlib/sformat.h>
#include <nlib/detail/bytes_print.h>
#include <stdio.h>

namespace hart7 {
namespace stack {

WHartUniqueID MakeUniqueID(uint16_t deviceCode, uint32_t deviceID)
{
	WHartUniqueID uniqueID;
	uniqueID.bytes[0] = (deviceCode >> 8);
	uniqueID.bytes[1] = (deviceCode & 0xFF);

	uniqueID.bytes[2] = (deviceID >> 16) & 0xFF;
	uniqueID.bytes[3] = (deviceID >> 8) & 0xFF;
	uniqueID.bytes[4] = (deviceID & 0xFF);
	return uniqueID;
}


bool operator==(const _device_address_t& left, const WHartUniqueID& right)
{
	uint8_t mask = 0x3F; //set first two bits to 0; they should be ignored when comparing

	if ((left[0] & mask) == (right.bytes[0] & mask))
	{	return memcmp(left + 1, right.bytes + 1, sizeof(left) - 1) == 0;
	}

	return false;
}

bool operator==(const WHartUniqueID& left, const _device_address_t& right)
{
	uint8_t mask = 0x3F; //set first two bits to 0; they should be ignored when comparing

	if ((left.bytes[0] & mask) == (right[0] & mask))
	{	return memcmp(left.bytes + 1, right + 1, sizeof(left.bytes) - 1) == 0;
	}

	return false;
}

bool operator==(const WHartUniqueID& left, const WHartUniqueID& right)
{
	uint8_t mask = 0x3F; //set first two bits to 0; they should be ignored when comparing

	if ((left.bytes[0] & mask) == (right.bytes[0] & mask))
	{	return memcmp(left.bytes + 1, right.bytes + 1, sizeof(left.bytes) - 1) == 0;
	}

	return false;
}

bool operator<(const WHartUniqueID& left, const WHartUniqueID& right)
{
	uint8_t mask = 0x3F; //set first two bits to 0; they should be ignored when comparing

	if ( (left.bytes[0] & mask) > (right.bytes[0] & mask) )
	{	return false;
	}

	if ( (left.bytes[0] & mask) < (right.bytes[0] & mask) )
	{	return true;
	}

	return memcmp(left.bytes + 1, right.bytes + 1, sizeof(left.bytes) - 1) < 0;
}


std::string ToStringHexa(uint8_t value)
{
	char buff[2048];
	sprintf(buff,"%X", (int) value);
	return std::string(buff);
}

std::string ToStringHexa(uint16_t value)
{
	char buff[2048];
	sprintf(buff,"%X", value);
	return std::string(buff);
}

std::string ToStringHexa(uint32_t value)
{
	char buff[2048];
	sprintf(buff,"%X", value);
	return std::string(buff);
}

std::string ToString(WHartTime* value)
{
	if (value == NULL)
		return "[time is NULL]";

	char buff[2048];
	sprintf(buff,"[time=%d.%d]", (int) value->tm.miliseconds,(int) value->tm.scalemiliseconds);
	return std::string(buff);
}

std::string ToString(WHartTime40* value)
{
	if (value == NULL)
		return "[time40 is NULL]";

	char buff[2048];
	sprintf(buff, "[time40=%llu]", uint64_t(((uint64_t) value->hi) << 32 | value->u32));
	return std::string(buff);
}

bool CompareUniqueID::operator()(const WHartUniqueID& p_oFirst, const WHartUniqueID& p_oSecond)
{
	return p_oFirst < p_oSecond;
}



} // namespace stack
} // namespace hart7



std::ostream& operator<< (std::ostream& out, const WHartUniqueID& p_rUniqueId )
{
	out << std::hex << std::setw (2) << std::setfill ('0') << (int)p_rUniqueId.bytes[0]
						<< std::hex << std::setw (2) << std::setfill ('0') << (int)p_rUniqueId.bytes[1]
						<< std::hex << std::setw (2) << std::setfill ('0') << (int)p_rUniqueId.bytes[2]
						<< std::hex << std::setw (2) << std::setfill ('0') << (int)p_rUniqueId.bytes[3]
						<< std::hex << std::setw (2) << std::setfill ('0') << (int)p_rUniqueId.bytes[4]
						;

	return out;
}

std::ostream& operator<< (std::ostream& out, const DeviceIndicatedStatus& status )
{
	out << "Status=" << std::hex << std::setw (2) << std::setfill ('0') << (int)status.deviceStatus
	<< ", ExtStatus=" << std::hex << std::setw (2) << std::setfill ('0') << (int)status.deviceExtendedStatus;
	return out;
}
