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

#ifndef _TTY_HDR_H_
#define _TTY_HDR_H_

// $Id: TtyDownHdr.h,v 1.12.4.1 2013/05/15 19:19:17 owlman Exp $

//////////////////////////////////////////////////////////////////////////////
/// @author	Marius Negreanu
/// @date	Thu Dec 18 13:40:22 2008 UTC
//////////////////////////////////////////////////////////////////////////////

#include "Shared/h.h"
#include "Shared/ProtocolPacket.h"

//////////////////////////////////////////////////////////////////////////////
/// @ingroup AccessPoint
//////////////////////////////////////////////////////////////////////////////
struct TTYNtwkHdr {
	uint8_t		m_nType ;		///< Packet type(@see CTtyDownMatcher)
	uint16_t	m_nHandle ;		///< Packet Message Handle
	inline static struct TTYNtwkHdr* access(const ProtocolPacketPtr& p_pPkt) {
		return (TTYNtwkHdr*) p_pPkt->Data();
	}
	TTYNtwkHdr()
		: m_nType(CTtyDownMatcher::EPKT_NTWK_MSG)
	{ }

} GCC_PACKED ;

//////////////////////////////////////////////////////////////////////////////
/// Tty Network Acknowledge
/// @ingroup AccessPoint
//////////////////////////////////////////////////////////////////////////////
struct TTYNtwkAckOutAck {
	uint8_t		m_nType ;		///< Packet type(@see CTtyDownMatcher)
	uint16_t	m_nHandle ;		///< Packet Message Handle
	uint32_t	m_nTai ;		///< TAI time. little endian
	uint8_t		m_rgSecFrac[3];	///< seconds fraction. little endian
	TTYNtwkAckOutAck()
		: m_nType(CTtyDownMatcher::EPKT_MESG_ACK)
		  , m_nHandle(0)
		  , m_nTai(0)
	{ }
} GCC_PACKED ;


//////////////////////////////////////////////////////////////////////////////
/// Tty Network Info
/// @ingroup AccessPoint
//////////////////////////////////////////////////////////////////////////////
struct TTYNtwkReqInfoAck {
	uint8_t		m_nType ;		///< Packet type(@see CTtyDownMatcher)
	uint16_t	m_nHandle ;		///< Packet Message Handle
	uint16_t	m_nNetworkId ;		///< TR Network ID
	uint8_t		m_rgJoinKey[16] ;	///< TR join key
	uint8_t		m_EUI64[8] ;
	uint8_t		m_LongTag[32] ;

	TTYNtwkReqInfoAck()
		: m_nType(CTtyDownMatcher::EPKT_NTWK_REQINFO_RESP)
		  , m_nHandle(0)
		  , m_nNetworkId(0)
	{
		memset(m_rgJoinKey, 0, 16);
		memset(m_EUI64, 0, 8);
		memset(m_LongTag, 0, 32);
	}
} GCC_PACKED ;

//////////////////////////////////////////////////////////////////////////////
/// Tty Disconnect
/// @ingroup AccessPoint
//////////////////////////////////////////////////////////////////////////////
struct TTYNtwkReqDisconnect {
	uint8_t		m_nType ;		///< Packet type(@see CTtyDownMatcher)
	uint16_t	m_nHandle ;		///< Packet Message Handle

	TTYNtwkReqDisconnect()
		: m_nType(CTtyDownMatcher::EPKT_MESG_REQDISC)
		  , m_nHandle(0)
	{ }
} GCC_PACKED ;

//////////////////////////////////////////////////////////////////////////////
/// @ingroup AccessPoint
//////////////////////////////////////////////////////////////////////////////
struct TTYNtwkAckInHdr {
	uint8_t		m_nType ;		///< Packet type(@see CTtyDownMatcher)
	TTYNtwkAckInHdr()
		: m_nType(CTtyDownMatcher::EPKT_NTWK_ACK)
	{}
} GCC_PACKED ;

//////////////////////////////////////////////////////////////////////////////
/// @ingroup AccessPoint
//////////////////////////////////////////////////////////////////////////////
typedef struct TTYNtwkAckInElm {
	uint16_t        m_nHandle ;	///< Packet Message Handle
	uint8_t         m_nStatus ;	///< Confirmation status
} GCC_PACKED *TTYNtwkAckInElmP;


#endif	//_TTY_HDR_H_
