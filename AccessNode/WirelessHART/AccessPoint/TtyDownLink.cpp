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

// $Id: TtyDownLink.cpp,v 1.31.4.1 2013/05/15 19:19:17 owlman Exp $

//////////////////////////////////////////////////////////////////////////////
/// @author	Marius Negreanu
/// @date	Thu Dec 18 13:40:22 2008 UTC
//////////////////////////////////////////////////////////////////////////////

#include "Shared/Time.h"
#include "TtyDownLink.h"
CTtyDownLink::CTtyDownLink(CWHAccessPointCfg& cfg_, bool p_bRawLog/*=false*/, bool useEscape/*=true*/ )
	: CPacketStream::CPacketStream(p_bRawLog, useEscape), cfg(cfg_)
{
	m_oMsgQ.MaxPriorities(EQ_TTY_MAX_PRIO) ;

	m_oMsgQ.Capacity(EQ_TTY_ACK, Q_TTY_ACK_CAPACITY );
	m_oMsgQ.MaxRetries(EQ_TTY_ACK, 0 );

	m_oMsgQ.Capacity(EQ_TTY_NORMAL, Q_TTY_NORMAL_CAPACITY ) ;

	m_oMsgQ.Name("AP->TQ");
	m_oMsgQ.AnchorLink( this ) ;

	lastCommunicationTime = time(NULL);
}

void CTtyDownLink::Request( ProtocolPacketPtr&  pkt )
{
	TTYNtwkHdr hdr ;
	hdr.m_nHandle = networkorder16( m_oMsgQ.Enque(EQ_TTY_NORMAL,pkt) );
	pkt->PushFront( (uint8_t*)&hdr, sizeof(hdr) );

	uint16_t crc = computeCRC( pkt->Data(), pkt->DataLen() );
	pkt->PushBack( &crc, CRC_LENGTH );
}


//////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////
ProtocolPacketPtr CTtyDownLink::Read()
{
	ProtocolPacketPtr  pkt = ProtocolPacketPtr(CPacketStream::Read( )) ;
	int crtTime = time(NULL);
	if( !pkt )
	{
		if (crtTime - lastCommunicationTime > cfg.m_nTrMaxInactivity)
		{
			lastCommunicationTime = crtTime;
			ResetTR();
		}

		return pkt ;
	}

	lastCommunicationTime = crtTime;
	const char *type =CTtyDownMatcher::GetTypeString(pkt->Data(), pkt->DataLen() ) ;
	CTtyDownMatcher::EPKT_TYPE pt = CTtyDownMatcher::GetType(pkt->Data(), pkt->DataLen() ) ;

	/*
	if( m_bRawLog )
	{
		char fmt[32];
		snprintf(fmt, sizeof(fmt), "TR->AP PKT_TYPE[%s]",type);
		LOG_HEX( fmt, pkt->Data(), pkt->DataLen() );
	}*/

	switch( pt )
	{
		case CTtyDownMatcher::EPKT_BUFF_MIC_ERR:
		case CTtyDownMatcher::EPKT_BUFF_RDY:
			if(	( m_bRawLog && (pt == CTtyDownMatcher::EPKT_BUFF_RDY) )	// Log everything is raw logging is enabled
		    ||	( pt != CTtyDownMatcher::EPKT_BUFF_RDY ) ) // Do not log EPKT_BUFF_RDY, it's too noisy
			{
//				LOG( "TR=>AP [EPKT_TYPE:%s]    (Bytes:%d)",  type, pkt->DataLen());
			}
			m_oMsgQ.Unblock();
			break;

		case CTtyDownMatcher::EPKT_BUFF_FULL:
			if(	( m_bRawLog && (pt == CTtyDownMatcher::EPKT_BUFF_RDY) )	// Log everything is raw logging is enabled
		    ||	( pt != CTtyDownMatcher::EPKT_BUFF_RDY ) ) // Do not log EPKT_BUFF_RDY, it's too noisy
			{
//				LOG( "TR=>AP [EPKT_TYPE:%s]    (Bytes:%d)", type, pkt->DataLen());
			}
			m_oMsgQ.Block();
			break;

		case CTtyDownMatcher::EPKT_NTWK_MSG:
			/// Return the NTWK_MSG to the GW.
			if ( ntwkMsgHandler(pkt) )
				{
					SendAck(TTYNtwkHdr::access(pkt)->m_nHandle );
					return pkt ;
				}
			break ;

		case CTtyDownMatcher::EPKT_NTWK_ACK:
			/// Return the NTWK_ACK packet to the GW.
			ntwkAckHandler(pkt) ;
			break ;

		case CTtyDownMatcher::EPKT_NTWK_REQINFO:
			writeNetInfo(pkt);
			break;

		case CTtyDownMatcher::EPKT_MESG_REQDISC:
		/// Return the NTWK_MSG to the router.
		if (reqDiscHandler(pkt)) {
			SendAck(TTYNtwkHdr::access(pkt)->m_nHandle);
			return pkt;
		}
		break;

		default:
			ERR("TR=>AP [EPKT_TYPE:EPKT_INVALID] (Bytes:%d)", pkt->DataLen());
	}
//	delete pkt ;
	return ProtocolPacketPtr() ;
}

void CTtyDownLink::ResetTR()
{
	if (cfg.m_bSendDiscOnReset)
	{
		writeReqDisconnect();
		LOG("Reset TR handled. Sending disconnect ..");
	}
	else if (cfg.m_nTrPowerID > 0)
	{
		systemf_to(30, "tr_ctl.sh restart %d ", cfg.m_nTrPowerID);
		LOG("Restart to TR sent...");
	}
}


static const uint32_t TAI_OFFSET = 0x16925E80+33;
//////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////
void CTtyDownLink::SendAck( unsigned int p_nHandle )
{
	TTYNtwkAckOutAck msg ;
	struct timeval tv ;

	gettimeofday( &tv, NULL) ;

	msg.m_nHandle = p_nHandle ;
	msg.m_nTai = tv.tv_sec + TAI_OFFSET ;
	msg.m_rgSecFrac[0] = tv.tv_usec ;
	msg.m_rgSecFrac[1] = tv.tv_usec >> 8 ;
	msg.m_rgSecFrac[2] = tv.tv_usec >> 16 ;

	ProtocolPacketPtr  pk_out = ProtocolPacketPtr(new ProtocolPacket()) ;
	pk_out->AllocateCopy( (uint8_t*)&msg, sizeof(TTYNtwkAckOutAck), 0, 4);
	uint16_t crc = computeCRC( (uint8_t*)&msg, sizeof( struct TTYNtwkAckOutAck) );
	pk_out->PushBack( &crc, CRC_LENGTH );

	m_oMsgQ.Enque(EQ_TTY_ACK, pk_out) ;
	m_oMsgQ.StartXmit();
}

//////////////////////////////////////////////////////////////////////////////
// Packet handlers for responses coming from Transceiver.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
/// @brief Handles a Network confirmation Packet.
//////////////////////////////////////////////////////////////////////////////
bool CTtyDownLink::ntwkAckHandler( ProtocolPacketPtr& pkt )
{
	uint16_t myCrc;
	uint8_t in_mic[CRC_LENGTH] ;

	pkt->PopBack( in_mic, CRC_LENGTH );
	myCrc = computeCRC( pkt->Data(), pkt->DataLen() ) ;
	if ( memcmp( in_mic, (void*)&myCrc, CRC_LENGTH ) )
	{
		LOG_HEX( "TR=>AP [MIC ERROR]", (const unsigned char*)&myCrc, CRC_LENGTH );
		return false ;
	}

	if( pkt->DataLen() < sizeof(TTYNtwkAckInHdr) + sizeof(TTYNtwkAckInElm) )
	{
		LOG( "TR=>AP [Invalid MessageSize] (size:%u expected:%u]", pkt->DataLen(), sizeof(TTYNtwkAckInHdr) + sizeof(TTYNtwkAckInElm));
		return false ;
	}

	int nbAcks = pkt->DataLen() - sizeof(TTYNtwkAckInHdr) /sizeof(TTYNtwkAckInElm) ;

	char buf[ nbAcks*6];

	TTYNtwkAckInElm *idx = TTYNtwkAckInElmP (pkt->Data(sizeof(TTYNtwkAckInHdr)) );
	while (  idx < TTYNtwkAckInElmP (pkt->Data() + pkt->DataLen()) )
	{
		sprintf( buf, "x%04X ", hostorder16(idx->m_nHandle) );
		m_oMsgQ.Confirm( hostorder16(idx->m_nHandle), idx->m_nStatus ) ;
		++idx ;
	}
//	LOG( "TR=>AP [EPKT_TYPE:%s]    (Bytes:%d Handles:[%s])", CTtyDownMatcher::GetTypeString(pkt->Data(),pkt->DataLen())
//		, pkt->DataLen(), buf);

	m_oMsgQ.StartXmit();
	return true ;
}


//////////////////////////////////////////////////////////////////////////////
/// @brief Handle a network message comming from transceiver.
/// @param[in] p_pPkt The packet from transceiver.
/// @details The last two bytes of p_pPkt represent the CRC sum.
/// The sum is removed from the packet end and checked for validity.
/// If the packet contains only the Network Header thus no payload,
/// after the CRC sum was removed, then the transceiver wishes to synchonize its time.
/// Empty packets are acknowledged right away.
//////////////////////////////////////////////////////////////////////////////
bool CTtyDownLink::ntwkMsgHandler(ProtocolPacketPtr& p_pPkt)
{
	uint16_t myCrc;
	uint16_t msgHandle  = TTYNtwkHdr::access(p_pPkt)->m_nHandle ;
	uint8_t in_mic[CRC_LENGTH] ;

	// Do not log periodical time sync messages
	//	(messages with no payload, only type+p_nHandle+ccm)
	if( m_bRawLog
	|| p_pPkt->DataLen() != (sizeof(TTYNtwkHdr) + CRC_LENGTH) )
	{
//		LOG( "TR=>AP [EPKT_TYPE:%s]    (Handle:x%04X Bytes:%d)",
//				CTtyDownMatcher::GetTypeString(p_pPkt->Data(), p_pPkt->DataLen() ),
//				hostorder16(msgHandle), p_pPkt->DataLen());
	}
	p_pPkt->PopBack( in_mic, CRC_LENGTH );
	myCrc = computeCRC( p_pPkt->Data(), p_pPkt->DataLen() ) ;
	if ( memcmp( in_mic, (void*)&myCrc, CRC_LENGTH ) )
	{
		LOG_HEX( "TR=>AP [MIC ERROR]", (const unsigned char*)&myCrc, CRC_LENGTH );
		return false ;
	}
	/// Acknowledge only an empty transceiver packet ;
	/// otherwise let the GW send the acknowledgement.
//	if ( sizeof(TTYNtwkHdr) == p_pPkt->DataLen() )
//		SendAck( msgHandle );
	m_oMsgQ.StartXmit();
	return true ;
}


bool CTtyDownLink::writeNetInfo(ProtocolPacketPtr& p_pPkt)
{
//	EPKT_NTWK_REQINFO_RESP
	uint16_t myCrc;
	uint16_t msgHandle  = TTYNtwkHdr::access(p_pPkt)->m_nHandle ;
	uint8_t in_mic[CRC_LENGTH] ;

	p_pPkt->PopBack( in_mic, CRC_LENGTH );
	myCrc = computeCRC( p_pPkt->Data(), p_pPkt->DataLen() ) ;
	if ( memcmp( in_mic, (void*)&myCrc, CRC_LENGTH ) )
	{
		LOG_HEX( "TR=>AP [MIC ERROR]", (const unsigned char*)&myCrc, CRC_LENGTH );
		return false ;
	}

	TTYNtwkReqInfoAck msg ;
	msg.m_nHandle = msgHandle;
	msg.m_nNetworkId = cfg.m_nNetworkId;
	memcpy(msg.m_rgJoinKey, cfg.m_JoinKey, 16);
	memcpy(msg.m_EUI64, cfg.m_EUI64, 8);
	memcpy(msg.m_LongTag, cfg.m_LongTag, 32);

	ProtocolPacketPtr  pk_out = ProtocolPacketPtr(new ProtocolPacket()) ;
	pk_out->AllocateCopy( (uint8_t*)&msg, sizeof(TTYNtwkReqInfoAck), 0, 4);
	uint16_t crc = computeCRC( (uint8_t*)&msg, sizeof( struct TTYNtwkReqInfoAck) );
	pk_out->PushBack( &crc, CRC_LENGTH );

	m_oMsgQ.Enque(EQ_TTY_ACK, pk_out) ;
	m_oMsgQ.StartXmit();

	return true;
}

bool CTtyDownLink::reqDiscHandler(ProtocolPacketPtr& p_pPkt)
{
	uint16_t myCrc;
	uint16_t msgHandle = TTYNtwkHdr::access(p_pPkt)->m_nHandle;
	uint8_t in_mic[CRC_LENGTH];

	p_pPkt->PopBack(in_mic, CRC_LENGTH);
	myCrc = computeCRC(p_pPkt->Data(), p_pPkt->DataLen());
	if (memcmp(in_mic, (void*) &myCrc, CRC_LENGTH)) {
		LOG_HEX("TR=>AP [MIC ERROR]", (const unsigned char*) &myCrc, CRC_LENGTH);
		return false;
	}

	writeReqDisconnect(msgHandle);

	return true;
}

void CTtyDownLink::writeReqDisconnect(uint16_t msgHandle)
{
	TTYNtwkReqDisconnect msg;
	msg.m_nHandle = msgHandle;

	ProtocolPacketPtr  pk_out = ProtocolPacketPtr(new ProtocolPacket()) ;
	pk_out->AllocateCopy((uint8_t*) &msg, sizeof(TTYNtwkReqDisconnect), 0, 4);
	uint16_t crc =
			computeCRC((uint8_t*) &msg, sizeof(struct TTYNtwkReqDisconnect));
	pk_out->PushBack(&crc, CRC_LENGTH);

	m_oMsgQ.Enque(EQ_TTY_ACK, pk_out);
	m_oMsgQ.StartXmit();
}

