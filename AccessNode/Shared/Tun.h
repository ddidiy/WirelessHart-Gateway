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
/// @file Tun.h
/// @author Costin Grigorescu <costin.grigorescu@nivis.com>, (C) 2010
/// @brief C++ interface: CTun
////////////////////////////////////////////////////////////////////////////////

#ifndef NETWORK_SNIFF_H_
#define NETWORK_SNIFF_H_

#include "Packet.h"
#include <net/if.h>
#include <netinet/in.h>

/// @addtogroup libshared
/// @{

class CTun
{
public:
	CTun();
	bool Init(unsigned p_uHexLimit = 16, const char* p_szTunName = "tun0");
	bool BringUpIp4(unsigned p_IP, unsigned p_mask, unsigned p_uMtu = 1500);
	bool BringUpIp6(in6_addr& p_IP, unsigned p_uMtu = 1500);
	bool CollectPacket(IpPacket* p_poPacket);
	bool InjectPacket(IpPacket* p_poPacket);
	void Close();
private:
	struct Ip4UDPPseudoHdr
	{
		uint32_t	m_o32_SourceAddress;
		uint32_t 	m_o32_DestinationAddress;
		uint8_t 	m_u8_Zero;
		uint8_t 	m_u8_Protocol;
		uint16_t 	m_u16_UDP_length;
		Ip4UDPPseudoHdr(IpPacket::IPv4* p_poPacket);
	}__attribute__((packed));
	struct Ip6PseudoHdr
	{
		uint8_t		m_o128_SourceAddress[16];
		uint8_t		m_o128_DestinationAddress[16];
		uint32_t	m_u32_UpperLayerLength;
		uint8_t		m_u24_Zero[3];
		uint8_t		m_u8_NextHeader;
		Ip6PseudoHdr(IpPacket::IPv6* p_poPacket);
	}__attribute__((packed));
	char m_szTunName[32];
	int m_nTunHandle;
	int m_nTunSocket;
	struct ifreq ifr;
	unsigned m_uHexLimit;
	bool bringUp(unsigned p_uMtu);
	bool checkRead(unsigned int p_nTimeout);
	bool collectIp4Packet(IpPacket::IPv4* p_poPacket, ssize_t br);
	bool injectIp4Packet(IpPacket::IPv4* p_poPacket);
	bool collectIp6Packet(IpPacket::IPv6* p_poPacket, ssize_t br);
	bool injectIp6Packet(IpPacket::IPv6* p_poPacket);
};

/// @}
#endif
