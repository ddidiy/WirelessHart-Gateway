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

#ifndef _HART_DOWN_MATCHER_H_
#define _HART_DOWN_MATCHER_H_

// $Id: TtyDownMatcher.h,v 1.10.4.1 2013/05/15 19:19:17 owlman Exp $

//////////////////////////////////////////////////////////////////////////////
/// @author	Marius Negreanu
/// @date	Thu Dec 18 13:40:22 2008 UTC
/// @brief	Implements a TTY packet matcher (CTtyDownMatcher).
//////////////////////////////////////////////////////////////////////////////

#include "Shared/ProtocolPacket.h"


//////////////////////////////////////////////////////////////////////////////
/// @class CTtyDownMatcher
/// @ingroup AccessPoint
/// @brief Maps an incomming integer value to an enum as the Packet type.
//////////////////////////////////////////////////////////////////////////////
class CTtyDownMatcher
{
public:
	enum EPKT_TYPE {					///< Path    Description
		EPKT_BUFF_RDY=0x00,				///< TR->AP  Transceiver Buffer ready.
		EPKT_BUFF_FULL=0x01,			///< TR->AP  Transceiver Buffer full.
		EPKT_BUFF_CHK=0x02,				///< TR->AP  Transceiver Buffer check.
		EPKT_BUFF_MIC_ERR=0x03,			///< TR->AP  Sent message has bad CRC.
		EPKT_MESG_ACK=0x10,				///< AP->TR  Message acknowledgement.
		EPKT_NTWK_REQINFO_RESP = 0x30,	///< AP->TR	 Response Net Info
		EPKT_NTWK_MSG=0x80,				///< TR<=>AP Network Message.
		EPKT_MESG_APP=0x81,				///< TR<=>AP Application Layer Message.
		EPKT_NTWK_ACK=0x82,				///< TR->AP  Network acknowledgement.
		EPKT_NTWK_REQINFO = 0x83,		///< TR->AP  Request Net Info
		EPKT_MESG_REQDISC = 0x84,		///< AP->TR  Request Disconnect.
//		EPKT_FRAG_START,				///< AP		 Fragment Start.
//		EPKT_FRAG_NEXT,					///< AP		 Next fragment.
		EPKT_INVALID					///< AP		 Invalid packet.
	};
	enum EPKT_STATUS {
		ESTATUS_OK=0x00,			///< Success.
		ESTATUS_OUT_OF_MEMORY=0x07,	///< Out of memory.
		ESTATUS_TIMEOUT=0x15		///< Timeout while sending packet in 6lowPAN Net
	} ;
	inline static CTtyDownMatcher::EPKT_TYPE GetType(uint8_t* p_nPkt, /**/unsigned p_nLen ) ;
	inline static const char* GetTypeString(uint8_t* p_nPkt, /**/unsigned p_nLen ) ;
protected:
	CTtyDownMatcher() {} ;
};

//////////////////////////////////////////////////////////////////////////////
/// @brief Get the packet type of a buffer as integer.
/// @param[in] p_pPkt Incomming packet data.
/// @param[in] p_nLen Incomming packet size.
/// @return Integer packet type.
//////////////////////////////////////////////////////////////////////////////
CTtyDownMatcher::EPKT_TYPE
CTtyDownMatcher::GetType( uint8_t* p_pPkt, /**/unsigned p_nLen )
{
	if( ! p_pPkt || !p_nLen ) return EPKT_INVALID ;

	switch( *p_pPkt ) {
		case EPKT_BUFF_RDY:
			return EPKT_BUFF_RDY;
		case EPKT_BUFF_FULL:
			WARN("EPKT_TYPE: EPKT_BUFF_FULL (%d bytes)", p_nLen);
			return EPKT_BUFF_FULL;
		case EPKT_BUFF_CHK:
			return EPKT_BUFF_CHK ;
		case EPKT_BUFF_MIC_ERR:
			ERR("EPKT_TYPE: EPKT_BUFF_MIC_ERR (%d bytes)", p_nLen);
			return EPKT_BUFF_MIC_ERR ;
		case EPKT_MESG_ACK:
			return EPKT_MESG_ACK ;
		case EPKT_NTWK_MSG:
			return EPKT_NTWK_MSG ;
		case EPKT_MESG_APP:
			return EPKT_MESG_APP ;
		case EPKT_NTWK_ACK:
			return EPKT_NTWK_ACK ;
		case EPKT_NTWK_REQINFO:
			return EPKT_NTWK_REQINFO;
		case EPKT_MESG_REQDISC:
			return EPKT_MESG_REQDISC;
		case EPKT_NTWK_REQINFO_RESP:
			return EPKT_NTWK_REQINFO_RESP;
		default:
			return EPKT_INVALID ;
	}
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Get the packet type of a buffer as a string.
/// @param[in] p_pPkt Incomming packet data.
/// @param[in] p_nLen Incomming packet size.
/// @return String packet type.
//////////////////////////////////////////////////////////////////////////////
const char * CTtyDownMatcher::GetTypeString( uint8_t* p_pPkt, /**/unsigned p_nLen )
{
	CTtyDownMatcher::EPKT_TYPE pt = CTtyDownMatcher::GetType( p_pPkt, p_nLen ) ;

	switch( pt ) {
		case EPKT_BUFF_RDY :		return "BUFF_RDY";
		case EPKT_BUFF_FULL:		return "BUFF_FULL";
		case EPKT_BUFF_CHK :		return "BUFF_CHK";
		case EPKT_BUFF_MIC_ERR:		return "BUFF_MIC_ERR";
		case EPKT_MESG_ACK :		return "MESG_ACK";
		case EPKT_NTWK_MSG :		return "NTWK_MSG";
		case EPKT_MESG_APP :		return "MESG_APP";
		case EPKT_NTWK_ACK :		return "NTWK_ACK";
		case EPKT_NTWK_REQINFO:		return "NTWK_REQINFO";
		case EPKT_MESG_REQDISC: 	return "MESG_REQDISC";
		case EPKT_NTWK_REQINFO_RESP:return "NTWK_REQINFO_RESP";
		default:					return "INVALID  ";
	}
}
#define TTY_MAX_STRING_TYPE_LEN sizeof("BUFF_MIC_ERR")
#endif	//_HART_DOWN_MATCHER_H_
