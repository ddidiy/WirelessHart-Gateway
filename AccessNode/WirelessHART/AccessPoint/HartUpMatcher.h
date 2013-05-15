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

#ifndef _HART_UP_MATCHER_H_
#define _HART_UP_MATCHER_H_

// $Id: HartUpMatcher.h,v 1.9.32.1 2013/05/15 19:19:17 owlman Exp $

//////////////////////////////////////////////////////////////////////////////
/// @author	Marius Negreanu
/// @date	Thu Dec 18 13:40:22 2008 UTC
/// @brief	Implements a UDP packet matcher (CHartUpMatcher).
//////////////////////////////////////////////////////////////////////////////

#include "Shared/ProtocolPacket.h"
#include "HartUpHdr.h"


//////////////////////////////////////////////////////////////////////////////
/// @class CHartUpMatcher
/// @ingroup AccessPoint
/// @brief Maps an incomming integer value to an enum as the Packet type.
//////////////////////////////////////////////////////////////////////////////
class CHartUpMatcher
{
public:
	enum EPKT_TYPE {			///< Path	 Description
		EPKT_SESSION_INIT=0x00,	///< AP<=>GW
		EPKT_SESSION_CLOSE=0x01,///< AP<=>GW
		EPKT_KEEP_ALIVE=0x02,	///< AP<=>GW
		EPKT_WIRED_HART=0x03,	///< AP<=>GW
		EPKT_DIRECT_HART=0x04,	///<
		EPKT_WREQUEST=0x0A,		///< AP<=>GW Wireless Request.
		EPKT_INVALID			///< Invalid packet.
	} ;
	enum EPKT_STATUS {
		ESTATUS_SUCCESS=0,		///< Success
		ESTATUS_TOO_FEW_BYTES=5,///< Error Too Few Data Bytes
		ESTATUS_DR_DEAD=35,		///< Error DR Dead (device not connected or no response)
		ESTATUS_NO_BUFFERS=61,	///< No Buffers
		ESTATUS_NO_ALRMEVNT=62,	///< No Alarm/Event Buffers
		ESTATUS_LOW_PRIO=63		///< Priority Too Low
	} ;
	inline static CHartUpMatcher::EPKT_TYPE GetType(uint8_t* p_nPkt, unsigned p_nLen ) ;
	inline static const char* GetTypeString(uint8_t* p_nPkt, /**/unsigned p_nLen ) ;
protected:
	CHartUpMatcher() {} ;
};

//////////////////////////////////////////////////////////////////////////////
/// @brief Get the packet type of a buffer as integer.
/// @param[in] p_pPkt Incomming packet data.
/// @param[in] p_nLen Incomming packet size.
/// @return Integer packet type.
//////////////////////////////////////////////////////////////////////////////
CHartUpMatcher::EPKT_TYPE
CHartUpMatcher::GetType( uint8_t* p_nPkt, /**/unsigned p_nLen )
{
	if( ! p_nPkt || p_nLen < sizeof(HartPDUHdr) ) return EPKT_INVALID ;

	switch( ((HartPDUHdr*)p_nPkt)->m_nMsgId ) {
		case EPKT_SESSION_INIT: return EPKT_SESSION_INIT;
		case EPKT_SESSION_CLOSE: return EPKT_SESSION_CLOSE;
		case EPKT_KEEP_ALIVE: return EPKT_KEEP_ALIVE;
		case EPKT_WIRED_HART: return EPKT_WIRED_HART;
		case EPKT_DIRECT_HART: return EPKT_DIRECT_HART;
		case EPKT_WREQUEST: return EPKT_WREQUEST;
		default: return EPKT_INVALID ;
	}
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Get the packet type of a buffer as a string.
/// @param[in] p_pPkt Incomming packet data.
/// @param[in] p_nLen Incomming packet size.
/// @return String packet type.
//////////////////////////////////////////////////////////////////////////////
const char * CHartUpMatcher::GetTypeString( uint8_t* p_nPkt, /**/unsigned p_nLen )
{
	CHartUpMatcher::EPKT_TYPE pt = CHartUpMatcher::GetType( p_nPkt, p_nLen ) ;
	switch( pt ) {
		case EPKT_SESSION_INIT:		return "EPKT_SESSION_INIT" ;
		case EPKT_SESSION_CLOSE:	return "EPKT_SESSION_CLOSE" ;
		case EPKT_KEEP_ALIVE:		return "EPKT_KEEP_ALIVE" ;
		case EPKT_WIRED_HART:		return "EPKT_WIRED_HART" ;
		case EPKT_DIRECT_HART:		return "EPKT_DIRECT_HART" ;
		case EPKT_WREQUEST:			return "EPKT_WREQUEST";
		default:					return "EPKT_INVALID" ;
	}
}
#define HART_MAX_STRING_TYPE_LEN sizeof("EPKT_SESSION_CLOSE")
#endif	//_HART_UP_MATCHER_H_
