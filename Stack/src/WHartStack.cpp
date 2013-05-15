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
 * WHartStack.c
 *
 *  Created on: Nov 26, 2008
 *      Author: nicu.dascalu
 */

#include <WHartStack/WHartStack.h>

#include <boost/format.hpp>
#include <nlib/detail/bytes_print.h>

namespace hart7 {
namespace stack {

const WHartShortAddress NetworkManager_Nickname()
{
	return 0xF980;
}

const WHartUniqueID& NetworkManager_UniqueID()
{
	static WHartUniqueID uid;
	static bool b = false;
	if (!b)
	{
		uint8_t nm_uid[] = { 0xF9, 0x80, 0x00, 0x00, 0x01 };
		memcpy(uid.bytes, nm_uid, sizeof(uid.bytes));
		b = true;
	}

	return uid;
}

const WHartShortAddress Gateway_Nickname()
{
	return 0xF981;
}

const WHartUniqueID& Gateway_UniqueID()
{
	static WHartUniqueID uid;
	static bool b = false;
	if (!b)
	{
		uint8_t nm_uid[] = { 0xF9, 0x81, 0x00, 0x00, 0x02 };
		memcpy(uid.bytes, nm_uid, sizeof(uid.bytes));
		b = true;
	}

	return uid;
}

const WHartUniqueID& Zero_UniqueID()
{
	static WHartUniqueID uid;
	static bool b = false;
	if (!b)
	{
		uint8_t nm_uid[] = { 0x00, 0x00, 0x00, 0x00, 0x00 };
		memcpy(uid.bytes, nm_uid, sizeof(uid.bytes));
		b = true;
	}

	return uid;
}

const uint32_t Gateway_DeviceID24()
{
	return 2; // 0x00, 0x00, 0x02
}

const uint16_t Gateway_ExpandedType()
{
	return  0xF981;
}

const WHartShortAddress Broadcast_Nickname()
{
	return 0xFFFF;
}

const WHartShortAddress No_Nickname()
{
	return 0x0000;
}

void WHartAddress::ToString(std::ostream& stream) const
{
	switch (type)
	{
	case whartaNickname:
		if (address.nickname == NetworkManager_Nickname())
			stream << "[NetworkManager]";
		else if (address.nickname == Gateway_Nickname())
			stream << "[Gateway]";
		else if (address.nickname == Broadcast_Nickname())
			stream << "[Broadcast]";
		else
		{
		    stream << "[NickName=0x" << std::hex << std::setw(4) << std::setfill('0') << address.nickname << std::dec << "]";
		}
		break;
	case whartaUniqueID:
        stream << "[UniqueID=0x" << std::hex << std::setw(2) << std::setfill('0') <<  (int) address.uniqueID.bytes[0]
                                 << std::setw(2) << (int) address.uniqueID.bytes[1]
                                 << std::setw(2) << (int) address.uniqueID.bytes[2]
                                 << std::setw(2) << (int) address.uniqueID.bytes[3]
                                 << std::setw(2) << (int) address.uniqueID.bytes[4] <<  std::dec << "]";
        break;

	case whartaProxy:
        stream << "[Proxy=0x" << std::hex << std::setw(4) << std::setfill('0') << address.proxy.nickname
        << ", UniqueID=0x" << std::setw(2) << (int) address.proxy.uniqueID.bytes[0]
           << std::setw(2) << (int) address.proxy.uniqueID.bytes[1]
           << std::setw(2) << (int) address.proxy.uniqueID.bytes[2]
           << std::setw(2) << (int) address.proxy.uniqueID.bytes[3]
           << std::setw(2) << (int) address.proxy.uniqueID.bytes[4] << std::dec <<  "]";
        break;

    case whartaProxyShort:
        stream << "[Proxy=0x" << std::hex << std::setw(4) << std::setfill('0') << address.proxyShort.nickname
        << ", DestNickName=0x" << address.proxyShort.destNickname << std::dec <<  "]";
        break;
    default:
        stream << "[Unknown Type=" << (int)type << "]";
        break;
	}
}

std::ostream& operator<<(std::ostream& stream, const WHartAddress& address)
{
    address.ToString(stream);
    return stream;
}

bool WHartAddress::operator==(const WHartAddress& right) const
{
	if (type == right.type)
	{
		if (type == whartaNickname && address.nickname == right.address.nickname)
		return true;
		if (type == whartaUniqueID && address.uniqueID == right.address.uniqueID)
		return true;
		if (type == whartaProxy && (address.proxy.nickname == right.address.proxy.nickname)
				&& (address.proxy.uniqueID == right.address.proxy.uniqueID))
		return true;
		if (type == whartaProxyShort && (address.proxyShort.nickname == right.address.proxyShort.nickname)
		        && (address.proxyShort.destNickname == right.address.proxyShort.destNickname))
		return true;
	}
	return false;
}

bool WHartAddress::operator<(const WHartAddress& right) const
{
	if (type == right.type)
	{
		if (type == WHartAddress::whartaNickname)
		{
			return address.nickname < right.address.nickname;
		}
		else if (type == WHartAddress::whartaUniqueID)
		{
			return COMPARE_FIXED_ARRAY(address.uniqueID.bytes, right.address.uniqueID.bytes) < 0;
		}
		else if (type == WHartAddress::whartaProxy)
		{
			if (address.proxy.uniqueID == right.address.proxy.uniqueID)
			{
				return address.proxy.nickname < right.address.proxy.nickname;
			}

			return COMPARE_FIXED_ARRAY(address.proxy.uniqueID.bytes, right.address.proxy.uniqueID.bytes) < 0;
		}
        else if (type == WHartAddress::whartaProxyShort)
        {
            if (address.proxyShort.destNickname == right.address.proxyShort.destNickname)
            {
                return address.proxyShort.nickname < right.address.proxyShort.nickname;
            }

            return address.proxyShort.destNickname < right.address.proxyShort.destNickname;
        }
		else
		{
			return false;
		}

	}
	else
	{
		return type < right.type;
	}
}


void WHartDllAddress::ToString(std::ostream& stream) const
{
	switch (type)
	{
		case whartdaGraph:
		    stream << "[GraphID=0x" << std::hex << std::setw(2) << std::setfill('0') << address.graphID << std::dec << "]";
		    break;
		case whartdaProxy:
            stream << "[Proxy=0x" << std::hex << std::setw(2) << std::setfill('0') << (int) address.proxy.bytes[0]
                << (int) address.proxy.bytes[1] << (int) address.proxy.bytes[2] << (int) address.proxy.bytes[3]
                << (int) address.proxy.bytes[4] << std::dec << "]";
            break;
		default:
		    stream << "[Unknown Type=" << (int)type << "]";
		    break;
	}
}

std::ostream& operator<<(std::ostream& stream, const WHartDllAddress& address)
{
    address.ToString(stream);
    return stream;
}

const char* ToString(WHartTransportType type)
{
	switch(type)
	{
		case wharttTransferRequest:
		return "TransferRequest";
		case wharttTransferResponse:
		return "TransferResponse";
		case wharttRequestUnicast:
		return "RequestUnicast";
		case wharttResponseUnicast:
		return "ResponseUnicast";
		case wharttSearchBroadcast:
		return "SearchBroadcast";
		case wharttPublishBroadcast:
		return "PublishBroadcast";
		case wharttRequestBroadcast:
		return "RequestBroadcast";
		case wharttResponseBroadcast:
		return "ResponseBroadcast";
		case wharttPublishNotify:
		return "PublishNotify";
		default:
		return "WHartTransportType unknown!";
	}
}

void WHartPayload::ToString(std::ostream& stream) const
{
	stream << "[Len=" << (int) dataLen << " Data=" << nlib::detail::BytesToString((uint8_t*) data,
                dataLen) << "]";
}

std::ostream& operator<<(std::ostream& output, const WHartPayload& payload)
{
    payload.ToString(output);
    return output;
}


} //namespace stack
} //namespace hart7
