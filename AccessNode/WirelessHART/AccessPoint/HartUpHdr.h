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

#ifndef _HART_HDR_H_
#define _HART_HDR_H_

// $Id: HartUpHdr.h,v 1.19.30.1 2013/05/15 19:19:17 owlman Exp $

//////////////////////////////////////////////////////////////////////////////
/// @author	Marius Negreanu
/// @date	Thu Dec 18 13:40:22 2008 UTC
//////////////////////////////////////////////////////////////////////////////

#include "Shared/h.h"
#include "Shared/ProtocolPacket.h"
//TEMP
#include "MsgQueue.h"

//////////////////////////////////////////////////////////////////////////////
/// @struct HartSessionInitArgv
/// @brief Session Init method response header.
/// @ingroup AccessPoint
//////////////////////////////////////////////////////////////////////////////
struct HartSessionInitArgv {
	uint8_t m_nMasterType ;	///< 1 = Primary Master.
	uint32_t m_nCloseTime ;	///< Milliseconds of inactivity before the session is closed.
	inline static struct HartSessionInitArgv* access(const ProtocolPacketPtr& p_pPkt) {
		return (HartSessionInitArgv*) p_pPkt->Data() ;
	}
} GCC_PACKED ;


//////////////////////////////////////////////////////////////////////////////
/// @struct WirelessRequestArgv
/// @brief Wireless request method sub-header.
/// @ingroup AccessPoint
//////////////////////////////////////////////////////////////////////////////
struct WirelessRequestArgv {
	uint8_t	m_nControl ;
	uint8_t	m_nSrcAddr[] ;	///< 2/8 bytes of address. Copied from GW.
} GCC_PACKED ;

//////////////////////////////////////////////////////////////////////////////
/// @struct HartPDUHdr
/// @brief Hart UDP Message Header.
/// @ingroup AccessPoint
//////////////////////////////////////////////////////////////////////////////
#define HART_TYPE_STX 0x10	///< Flag=bypass IO cache, Type=Request
struct HartPDUHdr {
	#define HART_PROTOCOL_VERSION 1
	uint8_t  m_nVersion ;	///< Protocol version number (always 1)
	uint8_t  m_nMsgFlagType;///< See CHartUpMatcher::EPKT_TYPE
	uint8_t  m_nMsgId ;	///< Message Identifier mapping a response to a request.
	uint8_t	 m_nStatus ;	///< Overall communication status for the message.
	uint16_t m_ui16TransactId ;	///< Unique transaction identifier for mapping a response to a request.
	uint16_t m_ui16ByteCount ;	///< Number of bytes in the message including message header and body.

	uint16_t GetByteCount() { return m_ui16ByteCount ; }
	uint16_t GetByteCountNtoH () { return ntohs(m_ui16ByteCount); }
	void SetByteCount( uint16_t p_ ) { m_ui16ByteCount = p_ ; }
	void SetByteCountHtoN( uint16_t p_ ) { m_ui16ByteCount = htons(p_); }
	void SetTransactIdHtoN( uint16_t p_ ) { m_ui16TransactId = htons(p_); }

	inline static struct HartPDUHdr* access(const ProtocolPacketPtr& p_pPkt) {
		return (HartPDUHdr*) p_pPkt->Data();
	}
	HartPDUHdr( unsigned p_nMsgId )
		: m_nVersion(HART_PROTOCOL_VERSION)
		, m_nMsgFlagType(HART_TYPE_STX)
		, m_nMsgId(p_nMsgId)
	{}
} GCC_PACKED ;


//////////////////////////////////////////////////////////////////////////////
/// @struct HartStatus.
/// @brief  Hold the Message and StatusClass associated to a numeric Status.
/// @see    HartStatusOk.
/// @ingroup AccessPoint
//////////////////////////////////////////////////////////////////////////////
struct HartStatus {
	uint8_t m_nStatus ;
	bool	m_bOk ;
	const char*	m_nDescription ;
} ;

//////////////////////////////////////////////////////////////////////////////
/// @ingroup AccessPoint
//////////////////////////////////////////////////////////////////////////////
struct HartStatusAssoc {
	const char* m_szName ;
	HartStatus* entries ;
} ;
/// SessionInit Message and StatusClass.
extern struct HartStatusAssoc StatusSessionInitAssoc_st;

/// SessionClose Message and StatusClass.
extern struct HartStatusAssoc StatusSessionCloseAssoc_st;

/// KeepAlive Message and StatusClass.
extern struct HartStatusAssoc StatusKeepAliveAssoc_st;

/// WirelessRequest Message and StatusClass.
extern struct HartStatusAssoc WirelessRequestAssoc_st;


//////////////////////////////////////////////////////////////////////////////
/// @class HartStatusOk
/// @ingroup AccessPoint
//////////////////////////////////////////////////////////////////////////////
class HartStatusOk {
public:
	static bool SessionInit( unsigned int p_nStatus )
	{
		return checkStatus(StatusSessionInitAssoc_st, p_nStatus);
	}
	static bool SessionClose( unsigned int p_nStatus )
	{
		return checkStatus(StatusSessionCloseAssoc_st, p_nStatus);
	}
	static bool KeepAlive( unsigned int p_nStatus )
	{
		return checkStatus(StatusKeepAliveAssoc_st, p_nStatus);
	}
	static bool WirelessRequest( unsigned int p_nStatus )
	{
		return checkStatus(WirelessRequestAssoc_st, p_nStatus);
	}

protected:
//////////////////////////////////////////////////////////////////////////////
/// @brief Print and return the boolean value of a status.
/// @param[in] p_rgStatusTable Table holding the Message and StatusClass.
/// @param[in] p_nStatus External received status.
/// @return The boolean value associated to p_nStatus.
//////////////////////////////////////////////////////////////////////////////
	bool static checkStatus(struct HartStatusAssoc& p_rgStatusTable, unsigned int p_nStatus)
	{
		for ( unsigned i=0
			; p_rgStatusTable.entries[i].m_nStatus || p_rgStatusTable.entries[i].m_bOk || p_rgStatusTable.entries[i].m_nDescription
			; ++i)
		{
			if ( p_rgStatusTable.entries[i].m_nStatus == p_nStatus )
			{
				LOG( "GW=>AP (method:%s packetStatus:%s)"
					, p_rgStatusTable.m_szName
					, p_rgStatusTable.entries[i].m_nDescription);
				return p_rgStatusTable.entries[i].m_bOk ;
			}
		}
		return false ;
	}
} ;
#endif // _HART_HDR_H_
