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
/// @file Packet.h
/// @author Costin Grigorescu <costin.grigorescu@nivis.com>, (C) 2010
/// @brief C++ interface: IpPacket
////////////////////////////////////////////////////////////////////////////////

#ifndef PACKET_H_
#define PACKET_H_

#include <stdint.h>
#include <unistd.h>

/// @addtogroup libshared
/// @{

#define MAX_IP_PACKET_SIZE 0xFFFF

#define GET_HEX_LIMITED(_ptr_, _len_, _limit_) GetHex(_ptr_, _Min(_len_, _limit_)), (((_len_)>(_limit_)) ? ".." : "")

struct ICMP_Payload
{
	/*
	0                   1                   2                   3
	0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|     Type      |     Code      |          Checksum             |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	*/
	uint8_t	m_u8_Type;		// Type
	uint8_t m_u8_Code;		// Code
	uint16_t m_u16_Checksum;	// Checksum
	uint8_t payload[0];
}__attribute__((packed));

struct UDP_Payload
{
	/*
	0      7 8     15 16    23 24    31
	+--------+--------+--------+--------+
	|  Source Port    |Destination  Port|
	+--------+--------+--------+--------+
	|     Length      |    Checksum     |
	+--------+--------+--------+--------+
	*/
	uint16_t m_u16_SourcePort;	// Source Port
	uint16_t m_u16_DestinationPort;	// Destination Port
	uint16_t m_u16_Length;		// Length
	uint16_t m_u16_Checksum;	// Checksum
	uint8_t payload[0];
}__attribute__((packed));

struct IpPacket
{
	struct IPv4
	{
		/*
		0                   1                   2                   3
		0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		|Version|  IHL  |Type of Service|          Total Length         |
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		|         Identification        |Flags|      Fragment Offset    |
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		|  Time to Live |    Protocol   |         Header Checksum       |
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		|                       Source Address                          |
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		|                    Destination Address                        |
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		|                    Options                    |    Padding    |
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*/
		uint8_t  m_u8_Version_IHL;		// Version, IHL
		uint8_t  m_u8_TypeOfService;		// Type of Service
		uint16_t m_u16_TotalLength;		// Total Length
		uint16_t m_u16_Identification;		// Identification
		uint16_t m_u16_Flags_FragmentOffset;	// Flags Fragment Offset
		uint8_t  m_u8_TTL;			// Time to Live
		uint8_t  m_u8_Protocol;			// Protocol
		uint16_t m_u16_Checksum;		// Header Checksum
		uint32_t m_o32_SourceAddress;		// Source Address
		uint32_t m_o32_DestinationAddress;	// Destination Address
		union
		{
			ICMP_Payload m_oICMP_Payload;
			UDP_Payload m_oUDP_Payload;
			uint8_t m_oIP_Payload[0];
			uint8_t m_oUNK_Payload[0];
		}__attribute__((packed));
		inline uint8_t IHL() const {return m_u8_Version_IHL & 0x0f;}
	}__attribute__((packed));
	struct IPv6
	{
		/*
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		|Version| Traffic Class |           Flow Label                  |
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		|         Payload Length        |  Next Header  |   Hop Limit   |
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		|                                                               |
		+                                                               +
		|                                                               |
		+                         Source Address                        +
		|                                                               |
		+                                                               +
		|                                                               |
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		|                                                               |
		+                                                               +
		|                                                               |
		+                      Destination Address                      +
		|                                                               |
		+                                                               +
		|                                                               |
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*/
		uint8_t  m_u8_Version_TrafficClass;
		uint8_t  m_u24_TrafficClass_FlowLabel[3];
		uint16_t m_u16_PayloadLength;
		uint8_t  m_u8_NextHeader;
		uint8_t  m_u8_HopLimit;
		uint8_t  m_o128_SourceAddress[16];
		uint8_t  m_o128_DestinationAddress[16];
		union
		{
			ICMP_Payload m_oICMP_Payload;
			UDP_Payload m_oUDP_Payload;
			uint8_t m_oIP_Payload[0];
			uint8_t m_oUNK_Payload[0];
		}__attribute__((packed));
	}__attribute__((packed));
	union
	{
		struct IPv4 m_stIp4;
		struct IPv6 m_stIp6;
	}__attribute__((packed));
	inline uint8_t Version() const {return m_stIp4.m_u8_Version_IHL >> 4;}
	static uint16_t Checksum(const uint8_t *p_pData, uint16_t p_unDataLen, uint16_t p_unPrevCheckSum = 0);
}__attribute__((packed));

/// @}
#endif
