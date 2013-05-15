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

#include <WHartHost/model/MAC.h>

#include <algorithm>
#include <vector>

#include <WHartHost/Utils.h>

namespace hart7
{
namespace hostapp
{

static const boost::uint8_t null_address[8] =
{ 0x00, 0x1B, 0x1E, 0, 0, 0, 0, 0 };

#define MAC_STRING_SIZE					23 // XX-XX-XX-XX-XX-XX-XX-XX
#define MAC_STRING_SPLIT_SIZE			2
#define MAC_UNIQUEID_OFFSET				3

MAC::MAC(const MAC& mac)
{
	(*this) = mac;
}
MAC::MAC(const boost::uint8_t * address_)
{
	memcpy(address, null_address, sizeof(address));
	if (address_)
		memcpy(address + MAC_UNIQUEID_OFFSET, address_, sizeof(WHartUniqueID));

	textAddress.clear();
	for (int i = 0; i < SIZE; i++)
	{
		textAddress += ConvertToHex(*((unsigned char*) (address + i)));
		if (i < SIZE - 1)
		{
			textAddress.append("-");
		}
	}
}

// the format should by like: XX-XX-XX-XX-XX-XX-XX-XX (old format: XXXX:XXXX:XXXX:XXXX)
MAC::MAC(const std::string& textAddress_)
{
	if (textAddress_.size() != MAC_STRING_SIZE)
	{
		THROW_EXCEPTION1(nlib::ArgumentException, boost::str(boost::format("Invalid MAC string: %1% !") % textAddress_));
	}

	char szMAC[MAC_STRING_SIZE + 1];
	strcpy(szMAC, textAddress_.c_str());
	char* token = strtok(szMAC, "-");

	unsigned char i = 0;
	unsigned int no = 0;
	while (token != NULL)
	{
		if (strlen(token) != MAC_STRING_SPLIT_SIZE)
		{
			THROW_EXCEPTION1(nlib::ArgumentException, boost::str(boost::format("Invalid MAC string: %1% !") % textAddress_));
		}

		sscanf(token, "%X", &no);
		address[i++] = no;
		token = strtok(NULL, "-");
	}

	textAddress = textAddress_;
}

//conversion
const WHartUniqueID MAC::Address() const
{
	WHartUniqueID addr;
	memcpy(addr.bytes, address + MAC_UNIQUEID_OFFSET, sizeof(WHartUniqueID));
	return addr;
}

const std::string MAC::ToString() const
{
	return textAddress;
}

void MAC::DumpToStream(std::ostream& p_rStream) const
{
	p_rStream << textAddress;
}

//operators
const MAC& MAC::operator=(const MAC& rhs)
{
	memcpy(address, rhs.address, SIZE);
	textAddress = rhs.textAddress;
	return (*this);
}

bool operator<(const MAC& lhs, const MAC& rhs)
{
	return lhs.textAddress < rhs.textAddress;
}

bool operator==(const MAC& lhs, const MAC& rhs)
{
	return lhs.textAddress == rhs.textAddress;
}

bool operator!=(const MAC& lhs, const MAC& rhs)
{
	return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& p_rStream, const MAC& p_rMac)
{
	p_rMac.DumpToStream(p_rStream);
	return p_rStream;
}

} //namespace hostapp
} //namespace hart7
