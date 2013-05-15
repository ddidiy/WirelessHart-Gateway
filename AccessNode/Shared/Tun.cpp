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
/// @file Tun.cpp
/// @author Costin Grigorescu <costin.grigorescu@nivis.com>, (C) 2010
/// @brief C++ implementation: CTun
////////////////////////////////////////////////////////////////////////////////

#include "Tun.h"
#include "Common.h"
#include <sys/ioctl.h>
#include <fcntl.h>

/* WARNING HACK:
 * <linux/ipv6.h> conflicts with <netinet/in.h> and <arpa/inet.h>,
 * so we've got to declare this structure by hand.
 */
struct in6_ifreq {
	struct in6_addr	ifr6_addr;
	uint32_t	ifr6_prefixlen;
	int		ifr6_ifindex;
};
/* /END OF WARNING HACK */

// Ioctl defines
#define TUNSETNOCSUM  _IOW('T', 200, int)
#define TUNSETDEBUG   _IOW('T', 201, int)
#define TUNSETIFF     _IOW('T', 202, int)
#define TUNSETPERSIST _IOW('T', 203, int)
#define TUNSETOWNER   _IOW('T', 204, int)

// TUNSETIFF ifr flags
#define IFF_TUN         0x0001
#define IFF_TAP         0x0002
#define IFF_NO_PI       0x1000
#define IFF_ONE_QUEUE   0x2000

#define TUN_INTERFACE 	"/dev/net/tun"

#ifdef HW_CYG

#ifndef SIOCSIFADDR
#define SIOCSIFADDR 121
#endif

#ifndef SIOCSIFNETMASK
#define SIOCSIFNETMASK 121
#endif

#ifndef SIOCSIFMTU
#define SIOCSIFMTU 121
#endif

#ifndef SIOCSIFFLAGS
#define SIOCSIFFLAGS 121
#endif

#endif


CTun::CTun()
{
}

bool CTun::Init(unsigned p_uHexLimit, const char* p_szTunName)
{
	m_uHexLimit = p_uHexLimit;
	strncpy(m_szTunName, p_szTunName, sizeof m_szTunName-1);
	m_szTunName[sizeof m_szTunName-1]=0;
	// open interface
	if ((m_nTunHandle = open(TUN_INTERFACE, O_RDWR)) < 0)
	{
		PERR("CTun::Init, open "TUN_INTERFACE);
		return false;
	}
	NLOG_DBG ("CTun::Init, open "TUN_INTERFACE);
	// set some parameters
	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
	strncpy(ifr.ifr_name, m_szTunName, IFNAMSIZ);
	if (ioctl(m_nTunHandle, TUNSETIFF, (void *) &ifr) < 0)
	{
		if (errno == EBADFD)
		{
			PERR("CTun::Init, ioctl "TUN_INTERFACE", %s", m_szTunName);
			return false;
		}
	}
	//ioctl (m_nTunHandle, TUNSETNOCSUM, 1); // this sets NOCKSUM ("We don't need checksums calculated for packets coming in this device: trust us!")
	NLOG_DBG ("CTun::Init, ioctl "TUN_INTERFACE", %s", m_szTunName);
	return true;
}

bool CTun::BringUpIp4(unsigned p_IP, unsigned p_Mask, unsigned p_uMtu)
{
	m_nTunSocket = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof addr);
	addr.sin_family = AF_INET;

	addr.sin_addr.s_addr = p_IP;
	memcpy(&ifr.ifr_addr, &addr, sizeof addr);
	if (ioctl(m_nTunSocket, SIOCSIFADDR, &ifr) < 0)
	{
		PERR("SIOCSIFADDR: "TUN_INTERFACE", %s", m_szTunName);
		return false;
	}

	addr.sin_addr.s_addr = p_Mask;
	memcpy(&ifr.ifr_addr, &addr, sizeof addr);
	if (ioctl(m_nTunSocket, SIOCSIFNETMASK, &ifr) < 0)
	{
		PERR("SIOCSIFNETMASK: "TUN_INTERFACE", %s", m_szTunName);
		return false;
	}
	return bringUp(p_uMtu);
}

bool CTun::BringUpIp6(in6_addr& p_IP, unsigned p_uMtu)
{
	m_nTunSocket = socket(AF_INET6, SOCK_DGRAM, 0);
	ioctl(m_nTunSocket, SIOGIFINDEX, &ifr);

	struct in6_ifreq ifr6;
	ifr6.ifr6_ifindex = ifr.ifr_ifindex;
	ifr6.ifr6_prefixlen = 64;
	ifr6.ifr6_addr = p_IP;

	if (ioctl(m_nTunSocket, SIOCSIFADDR, &ifr6) < 0)
	{
		PERR("SIOCSIFADDR: "TUN_INTERFACE", %s", m_szTunName);
		return false;
	}

	return bringUp(p_uMtu);;
}

bool CTun::bringUp(unsigned p_uMtu)
{
	ifr.ifr_mtu = p_uMtu;
	if (ioctl(m_nTunSocket, SIOCSIFMTU, &ifr) < 0)
	{
		PERR("SIOCSIFMTU: "TUN_INTERFACE", %s", m_szTunName);
		return false;
	}
	ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);
	if (ioctl(m_nTunSocket, SIOCSIFFLAGS, &ifr) < 0)
	{
		PERR("SIOCSIFFLAGS: "TUN_INTERFACE", %s", m_szTunName);
		return false;
	}
	return true;
}

bool CTun::CollectPacket(IpPacket* p_poPacket)
{
	if (!checkRead(10000))
	{
		return false;
	}
	ssize_t br = read(m_nTunHandle, p_poPacket, MAX_IP_PACKET_SIZE);
	if(br < 0)
	{
		PERR("CTun::CollectPacket, read "TUN_INTERFACE);
		return false;
	}
	NLOG_DBG("CollectPacket, if:%s, sz:%d, data[%s%s]", m_szTunName,  br,
		GET_HEX_LIMITED(&p_poPacket, (size_t)br, m_uHexLimit));
	if (br < 1)
	{
		NLOG_WARN("CTun::CollectPacket, IP header too small: expected(1), received(0)");
		return false;
	}
	int nIPversion = p_poPacket->Version();
	switch(nIPversion){
	case 4:
		return collectIp4Packet(&p_poPacket->m_stIp4, br);
	case 6:
		return collectIp6Packet(&p_poPacket->m_stIp6, br);
	default:
		return false;
	}
}

bool CTun::collectIp4Packet(IpPacket::IPv4* p_poPacket, ssize_t br)
{
	uint16_t nIPHdrSize = p_poPacket->IHL() * 4;
	if (br < nIPHdrSize)
	{
		NLOG_WARN("CTun::CollectIp4Packet, [IP] header too small: expected(0x%04x), received(0x%04x)", nIPHdrSize, br);
		return false;
	}
	// verify ip header checksum
	uint16_t rcvChecksum = ntohs(p_poPacket->m_u16_Checksum);
	p_poPacket->m_u16_Checksum = 0;
	uint16_t expChecksum  = IpPacket::Checksum((const uint8_t*)p_poPacket, nIPHdrSize);
	if (expChecksum != rcvChecksum)
	{
		NLOG_WARN("CTun::CollectIp4Packet, [IP] incorrect checksum: expected(0x%04x), received(0x%04x)", expChecksum, rcvChecksum);
		return false;
	}
	p_poPacket->m_u16_Checksum = htons(rcvChecksum);
	// verify packet length
	uint16_t rcvLen = ntohs(p_poPacket->m_u16_TotalLength);
	if (br != rcvLen)
	{
		NLOG_WARN("CTun::CollectIp4Packet, [IP] packet too small: expected(0x%04x), received(0x%04x)", rcvLen, br);
	}
	// verify checksum
	switch(p_poPacket->m_u8_Protocol)
	{
		case IPPROTO_ICMP:
		{
			rcvChecksum = ntohs(p_poPacket->m_oICMP_Payload.m_u16_Checksum);
			p_poPacket->m_oICMP_Payload.m_u16_Checksum = 0;
			expChecksum = IpPacket::Checksum((const uint8_t*)(&p_poPacket->m_oICMP_Payload), rcvLen - nIPHdrSize);
			if (expChecksum != rcvChecksum)
			{
				NLOG_WARN("CTun::CollectIp4Packet, [ICMP] incorrect checksum: expected(0x%04x), received(0x%04x)", expChecksum, rcvChecksum);
				return false;
			}
			p_poPacket->m_oICMP_Payload.m_u16_Checksum = htons(rcvChecksum);
			break;
		}
		case IPPROTO_UDP:
		{
			rcvChecksum = ntohs(p_poPacket->m_oUDP_Payload.m_u16_Checksum);
			// UDP checksum disabled - don't verify
			if (!rcvChecksum) break;
			Ip4UDPPseudoHdr oPHdr(p_poPacket);
			// pseudoheader checksum
			expChecksum =  IpPacket::Checksum((uint8_t*)&oPHdr, sizeof (Ip4UDPPseudoHdr));
			p_poPacket->m_oUDP_Payload.m_u16_Checksum = 0;
			expChecksum = IpPacket::Checksum((uint8_t*)&p_poPacket->m_oUDP_Payload, rcvLen - nIPHdrSize, expChecksum);
			if(!expChecksum ) expChecksum = 0xFFFF;
			if (expChecksum != rcvChecksum)
			{
				NLOG_WARN("CTun::CollectIp4Packet, [UDP] incorrect checksum: expected(0x%04x), received(0x%04x)", expChecksum, rcvChecksum);
				return false;
			}
			p_poPacket->m_oUDP_Payload.m_u16_Checksum = htons(rcvChecksum);
			break;
		}
		default:;
	}
	return true;
}

bool CTun::collectIp6Packet(IpPacket::IPv6* p_poPacket, ssize_t br)
{
	if (br < (ssize_t)offsetof(IpPacket::IPv6, m_oIP_Payload))
	{
		NLOG_WARN("CTun::collectIp6Packet, [IP] header too small: expected(0x%04x), received(0x%04x)", offsetof(IpPacket::IPv6, m_oIP_Payload), br);
		return false;
	}
	// verify packet length
	uint16_t rcvLen = ntohs(p_poPacket->m_u16_PayloadLength);
	if(rcvLen + (ssize_t)offsetof(IpPacket::IPv6, m_oIP_Payload) != br)
	{
		NLOG_WARN("CTun::collectIp6Packet, [IP] packet size wrong: expected(0x%04x), received(0x%04x)", rcvLen + offsetof(IpPacket::IPv6, m_oIP_Payload), br);
		return false;
	}
	// verify checksums
	Ip6PseudoHdr oPHdr(p_poPacket);
	uint16_t rcvChecksum;
	// pseudoheader checksum
	uint16_t expChecksum =  IpPacket::Checksum((uint8_t*)&oPHdr, sizeof oPHdr);
	switch(p_poPacket->m_u8_NextHeader)
	{
		case IPPROTO_ICMPV6:
		{
			rcvChecksum = ntohs(p_poPacket->m_oICMP_Payload.m_u16_Checksum);
			p_poPacket->m_oICMP_Payload.m_u16_Checksum = 0;
			expChecksum = IpPacket::Checksum((const uint8_t*)(&p_poPacket->m_oICMP_Payload), rcvLen, expChecksum);
			if (expChecksum != rcvChecksum)
			{
				NLOG_WARN("CTun::collectIp6Packet, [ICMP] incorrect checksum: expected(0x%04x), received(0x%04x)", expChecksum, rcvChecksum);
				return false;
			}
			p_poPacket->m_oICMP_Payload.m_u16_Checksum = htons(rcvChecksum);
			break;
		}
		case IPPROTO_UDP:
		{
			rcvChecksum = ntohs(p_poPacket->m_oUDP_Payload.m_u16_Checksum);
			if (!rcvChecksum){
				//IPv6 receivers must discard UDP packets containing a zero checksum, and should log the error.
				NLOG_ERR("CTun::collectIp6Packet, [UDP] incorrect zero checksum: packet discarded");
				return false;
			};
			p_poPacket->m_oUDP_Payload.m_u16_Checksum = 0;
			expChecksum = IpPacket::Checksum((uint8_t*)&p_poPacket->m_oUDP_Payload, rcvLen, expChecksum);
			if(!expChecksum ) expChecksum = 0xFFFF;
			if (expChecksum != rcvChecksum)
			{
				NLOG_WARN("CTun::collectIp6Packet, [UDP] incorrect checksum: expected(0x%04x), received(0x%04x)", expChecksum, rcvChecksum);
				return false;
			}
			p_poPacket->m_oUDP_Payload.m_u16_Checksum = htons(rcvChecksum);
			break;
		}
		default:;
	}
	return true;
}

bool CTun::InjectPacket(IpPacket* p_poPacket)
{
	switch(p_poPacket->Version()){
	case 4:
		return injectIp4Packet(&p_poPacket->m_stIp4);
	case 6:
		return injectIp6Packet(&p_poPacket->m_stIp6);
	default:
		return false;
	}
}

bool CTun::injectIp4Packet(IpPacket::IPv4* p_poPacket)
{
	ssize_t packetLen = ntohs(p_poPacket->m_u16_TotalLength);
	if (packetLen > MAX_IP_PACKET_SIZE)
	{
		NLOG_WARN("CTun::injectIp4Packet, packetLen(%d) > MAX_IP_PACKET_SIZE(%d)", packetLen, MAX_IP_PACKET_SIZE );
		return false;
	}
	switch(p_poPacket->m_u8_Protocol)
	{
		case IPPROTO_ICMP:
			p_poPacket->m_oICMP_Payload.m_u16_Checksum = 0;
			p_poPacket->m_oICMP_Payload.m_u16_Checksum = IpPacket::Checksum((const uint8_t*)(&p_poPacket->m_oICMP_Payload),
								packetLen - 4 * p_poPacket->IHL());
			p_poPacket->m_oICMP_Payload.m_u16_Checksum = htons(p_poPacket->m_oICMP_Payload.m_u16_Checksum );
			break;
		case IPPROTO_UDP:
		{
			Ip4UDPPseudoHdr oPHdr(p_poPacket);
			// pseudoheader checksum
			uint16_t checksum = IpPacket::Checksum((uint8_t*)&oPHdr, sizeof (Ip4UDPPseudoHdr));
			p_poPacket->m_oUDP_Payload.m_u16_Checksum = 0;
			checksum = IpPacket::Checksum((uint8_t*)&p_poPacket->m_oUDP_Payload, ntohs(oPHdr.m_u16_UDP_length), checksum);
			if(!checksum){
				p_poPacket->m_oUDP_Payload.m_u16_Checksum = 0xFFFF;
			}else{
				p_poPacket->m_oUDP_Payload.m_u16_Checksum = htons(checksum);
			}
			break;
		}
		default:;
	}
	p_poPacket->m_u16_Checksum = 0;
	p_poPacket->m_u16_Checksum = htons(IpPacket::Checksum((const uint8_t*)p_poPacket, p_poPacket->IHL()*4));
	ssize_t bw = write(m_nTunHandle, p_poPacket, packetLen);
	if(bw < packetLen)
	{
		PERR("CTun::injectIp4Packet, write "TUN_INTERFACE);
		return false;
	}
	NLOG_DBG("injectIp4Packet, if:%s, sz:%d, data[%s%s]", m_szTunName, bw,
		GET_HEX_LIMITED(&p_poPacket, (size_t)bw, m_uHexLimit))
	return true;
}

bool CTun::injectIp6Packet(IpPacket::IPv6* p_poPacket)
{
	/* ipv6 payload + ipv6 header itself */
	ssize_t packetLen = ntohs(p_poPacket->m_u16_PayloadLength) + offsetof(IpPacket::IPv6, m_oIP_Payload);
	if (packetLen > MAX_IP_PACKET_SIZE)
	{
		NLOG_WARN("CTun::injectIp6Packet, packetLen(%d) > MAX_PACKET_SIZE(%d)", packetLen, MAX_IP_PACKET_SIZE );
		return false;
	}
	Ip6PseudoHdr oPHdr(p_poPacket);
	// pseudoheader checksum
	uint16_t checksum = IpPacket::Checksum((uint8_t*)&oPHdr, sizeof oPHdr);
	switch(p_poPacket->m_u8_NextHeader)
	{
		case IPPROTO_ICMPV6:
			p_poPacket->m_oICMP_Payload.m_u16_Checksum = 0;
			/* substract the ipv6 header size from the whole packet length */
			p_poPacket->m_oICMP_Payload.m_u16_Checksum = htons(IpPacket::Checksum((const uint8_t*)&p_poPacket->m_oICMP_Payload,
								packetLen - offsetof(IpPacket::IPv6, m_oIP_Payload), checksum));
			break;
		case IPPROTO_UDP:
			p_poPacket->m_oUDP_Payload.m_u16_Checksum = 0;
			/* substract the ipv6 header size from the whole packet length */
			checksum = IpPacket::Checksum((const uint8_t*)&p_poPacket->m_oUDP_Payload
					, packetLen - offsetof(IpPacket::IPv6, m_oIP_Payload), checksum);
			if(!checksum){
				p_poPacket->m_oUDP_Payload.m_u16_Checksum = 0xFFFF;
			}else{
				p_poPacket->m_oUDP_Payload.m_u16_Checksum = htons(checksum);
			}
			break;
		default:
			NLOG_ERR("CTun::injectIp6Packet: unsupported IP next header %d, discarding packet", p_poPacket->m_u8_NextHeader);
			return false;
	}
	ssize_t bw = write(m_nTunHandle, p_poPacket, packetLen);
	if(bw < packetLen)
	{
		PERR("CTun::injectIp6Packet, write "TUN_INTERFACE);
		return false;
	}
	NLOG_DBG("injectIp6Packet, if:%s, sz:%d, data[%s%s]", m_szTunName, bw,
		GET_HEX_LIMITED(&p_poPacket, (size_t)bw, m_uHexLimit))
	return true;
}

void CTun::Close()
{
	close(m_nTunSocket);
	close(m_nTunHandle);
}

bool CTun::checkRead (unsigned int p_nTimeout)
{
	if (m_nTunHandle == -1) return false;

	fd_set readfds;
	struct timeval tmval;

	tmval.tv_usec = p_nTimeout%1000000;
	tmval.tv_sec = p_nTimeout/1000000;

	FD_ZERO(&readfds);
	FD_SET(m_nTunHandle, &readfds);

	int nSelected = ::select( m_nTunHandle +1, &readfds, NULL, NULL, &tmval);

	if ( -1 == nSelected )
	{
		NLOG_ERR("CTun::checkRead(fd %u)", m_nTunHandle);
		if (errno == EINTR )
		{
			return false;
		}
		Close();
		return false;
	}
	return FD_ISSET( m_nTunHandle, &readfds );
}

CTun::Ip4UDPPseudoHdr::Ip4UDPPseudoHdr(IpPacket::IPv4* p_poPacket): m_o32_SourceAddress(p_poPacket->m_o32_SourceAddress),
	m_o32_DestinationAddress(p_poPacket->m_o32_DestinationAddress),  m_u8_Zero(0), m_u8_Protocol(p_poPacket->m_u8_Protocol),
	m_u16_UDP_length(p_poPacket->m_oUDP_Payload.m_u16_Length)
{
}

CTun::Ip6PseudoHdr::Ip6PseudoHdr(IpPacket::IPv6* p_poPacket): m_u8_NextHeader(p_poPacket->m_u8_NextHeader)
{
	memcpy(m_o128_SourceAddress, p_poPacket->m_o128_SourceAddress, 32); //copy source and destination addresses
	memset(&m_u32_UpperLayerLength, 0, 7); //memset the UpperLayerLength and Zeros
	m_u32_UpperLayerLength = ntohs(p_poPacket->m_u16_PayloadLength);
	m_u32_UpperLayerLength = htonl(m_u32_UpperLayerLength);
}
