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

// $Id: HartUpLink.cpp,v 1.37.4.1 2013/05/15 19:19:17 owlman Exp $

//////////////////////////////////////////////////////////////////////////////
/// @author	Marius Negreanu
/// @date	Thu Dec 18 13:40:22 2008 UTC
//////////////////////////////////////////////////////////////////////////////

#include "Shared/h.h"
#include "Shared/ProtocolPacket.h"
#include "HartUpLink.h"
#include "HartUpMatcher.h"


//////////////////////////////////////////////////////////////////////////////
/// @brief Send a Session Initiate.
/// @param none
/// @retval none
/// @details This message is used to initiate a session between a Host
/// application/client and a field device or gateway server. This message must
/// be exchanged to establish with the server before other messages will be
/// accepted from the client. If requests are sent before a session has been
/// initiated, the server will not respond even if the message is valid in
/// every other way.
//////////////////////////////////////////////////////////////////////////////
void CHartUpLink::SessionInit()
{
	m_nRemoteUdpPort = ((CUdpLink*)*this)->RemotePort() ;
	LOG( "%s SessionInit (basePort:%u)", m_szName, m_nRemoteUdpPort);

	m_nLinkState = EUDP_LINK_DISCONNECTED ;
	HartPDUHdr hdr( CHartUpMatcher::EPKT_SESSION_INIT ) ;
	hdr.SetByteCountHtoN( sizeof(HartPDUHdr)+sizeof(HartSessionInitArgv) );
	HartSessionInitArgv siargv ={HART_PRIMARY_MASTER, networkorder16(HART_SESSION_EXPIRE) };
	// Add PDU Data
	ProtocolPacketPtr pkt = ProtocolPacketPtr(new ProtocolPacket) ;
	pkt->AllocateCopy( (uint8_t*)&hdr, sizeof(hdr), 0, sizeof(HartSessionInitArgv) ) ;
	pkt->PushBack( (void*)&siargv,sizeof(siargv) );
	//HartPDUHdr::access(pkt)->m_ui16TransactId = networkorder16( m_pMsgQ->Enque( EQ_UDP_KEEP_ALIVE, pkt, m_nHandle--) );
	HartPDUHdr::access(pkt)->SetTransactIdHtoN( m_pMsgQ->Enque( EQ_UDP_KEEP_ALIVE, pkt, m_nHandle--) );
	m_pMsgQ->StartXmit() ;
}

//////////////////////////////////////////////////////////////////////////////
///
/// @brief Send a SessionClose packet.
/// @param none
/// @retval none
/// @details This message is used by the HART Host application/client to request
/// the server close a session.
//////////////////////////////////////////////////////////////////////////////
void CHartUpLink::SessionClose()
{
	LOG( "%s SessionClose (basePort:%u)", m_szName, m_nBaseUdpPort);
	HartPDUHdr hdr( CHartUpMatcher::EPKT_SESSION_CLOSE ) ;
	hdr.SetByteCount( networkorder16(sizeof(HartPDUHdr)) );
	// Add PDU Data
	ProtocolPacketPtr pkt = ProtocolPacketPtr(new ProtocolPacket) ;
	pkt->AllocateCopy( (uint8_t*)&hdr, sizeof(hdr) ) ;
	HartPDUHdr::access(pkt)->m_ui16TransactId = networkorder16( m_pMsgQ->Enque( EQ_UDP_KEEP_ALIVE, pkt, m_nHandle--) );

	m_pMsgQ->StartXmit() ;
	m_nLinkState = EUDP_LINK_DISCONNECTED ;
	m_bForceDisconnect = false ;
	((CUdpLink*)*this)->RemotePort(m_nBaseUdpPort) ;
	//drop all messages in Queue since the connection is closed.
	m_pMsgQ->Clear(EQ_UDP_MAX_PRIO);

	m_fpDisconnectedCallback();
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Send a KeepAlive packet.
/// @param none
/// @retval none
/// @details This message is used by the HART Host application/client to keep
/// a session active with a server. This message only needs to be sent if no
/// communication has occurred for the inactivity close time established when
/// the session was initiated.
//////////////////////////////////////////////////////////////////////////////
void CHartUpLink::KeepAlive()
{
	LOG( "%s UpLink.KeepAlive", m_szName);
	HartPDUHdr hdr( CHartUpMatcher::EPKT_KEEP_ALIVE ) ;
	hdr.SetByteCount( networkorder16(sizeof(HartPDUHdr)) );
	// Add PDU Data
	ProtocolPacketPtr pkt = ProtocolPacketPtr(new ProtocolPacket) ;
	pkt->AllocateCopy( (uint8_t*)&hdr, sizeof(hdr) ) ;
	HartPDUHdr::access(pkt)->m_ui16TransactId = networkorder16( m_pMsgQ->Enque( EQ_UDP_KEEP_ALIVE, pkt, m_nHandle--));
	m_pMsgQ->StartXmit() ;

	m_bForceDisconnect = true;
}


//////////////////////////////////////////////////////////////////////////////
/// @param[in] p_rgMsg	Message to send.
/// @param[in] p_nMsgSz	Size of the message to be sent.
/// @retval none
//////////////////////////////////////////////////////////////////////////////
void CHartUpLink::WirelessRequest( uint8_t p_rgMsg[], size_t p_nMsgSz, unsigned int p_nHandle=0 )
{
	if ( m_nLinkState == EUDP_LINK_DISCONNECTED )
		return SessionInit() ;

	HartPDUHdr hdr( CHartUpMatcher::EPKT_WREQUEST );
	hdr.SetByteCount( networkorder16(sizeof(HartPDUHdr) + p_nMsgSz) );

	ProtocolPacketPtr pkt = ProtocolPacketPtr(new ProtocolPacket) ;
	pkt->AllocateCopy( (uint8_t*)&hdr, sizeof(hdr), 0, p_nMsgSz ) ;
	pkt->PushBack(p_rgMsg, p_nMsgSz);

	if ( p_nHandle )
	{
		HartPDUHdr::access(pkt)->m_ui16TransactId = p_nHandle ;
		p_nHandle = hostorder16(p_nHandle);
		//LOG( "%s UpLink.WirelessRequest [BigEndian] (Handle:x%04X)", m_szName, p_nHandle );
	}
	else
	{
		HartPDUHdr::access(pkt)->m_ui16TransactId = networkorder16(m_nHandle--) ;
		//LOG( "%s UpLink.WirelessRequest [LittleEndian] (Handle:x%04X)", m_szName, HartPDUHdr::access(pkt)->m_ui16TransactId );
	}
	unsigned handle = m_pMsgQ->Enque( EQ_UDP_NORMAL, pkt, p_nHandle );
	if (handle)
	{
    	    HartPDUHdr::access(pkt)->m_ui16TransactId = networkorder16(handle );
	    m_pMsgQ->StartXmit() ;
	}
	else
	{
//	    delete pkt;
	}
	    
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Read data from then link and dispatches the appropriate handler.
/// @param none
/// @retval ProtocolPacket
/// @retval NULL When nothing is read OR when invalid bytes are read.
//////////////////////////////////////////////////////////////////////////////
ProtocolPacketPtr CHartUpLink::Read()
{
	ProtocolPacketPtr pkt = ProtocolPacketPtr(CPacketStream::Read( )) ;
	if( !pkt ) { return pkt ; }

	const char *type =CHartUpMatcher::GetTypeString(pkt->Data(), pkt->DataLen() ) ;
	CHartUpMatcher::EPKT_TYPE pt = CHartUpMatcher::GetType(pkt->Data(), pkt->DataLen() ) ;

//	LOG( "%s UpLink.Read (PKT_TYPE:%s MsgID:x%02X Handle:x%04X)", m_szName
//			, type
//			, HartPDUHdr::access(pkt)->m_nMsgId
//			, hostorder16(HartPDUHdr::access(pkt)->m_ui16TransactId)
//			);

	switch( pt )
	{
		case CHartUpMatcher::EPKT_SESSION_INIT:
			sessionInitHandler(pkt);
			break ;
		case CHartUpMatcher::EPKT_SESSION_CLOSE:
			sessionCloseHandler(pkt);
			break ;
		case CHartUpMatcher::EPKT_KEEP_ALIVE:
			keepAliveHandler(pkt);
			break ;
		case CHartUpMatcher::EPKT_WIRED_HART:
			return pkt ;
		case CHartUpMatcher::EPKT_DIRECT_HART:
			return pkt ;
		case CHartUpMatcher::EPKT_WREQUEST:
			if ( wirelessRequestHandler(pkt) )
				return pkt ;
			break;
		default:
			invalidPacketHandler(pkt);
	}
	if ( pt != CHartUpMatcher::EPKT_INVALID ) armKeepAliveTimer() ;
	m_pMsgQ->Unblock() ; // this should be Unblock(All);

//	delete pkt ;
	return  ProtocolPacketPtr();
}

//////////////////////////////////////////////////////////////////////////////
// Packet handlers for responses coming from Gateway.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
/// @brief Handle a Session Initiate response.
/// @retval none
/// @param p_pPkt Data received from the link.
//////////////////////////////////////////////////////////////////////////////
void CHartUpLink::sessionInitHandler(ProtocolPacketPtr& p_pPkt )
{
	m_pMsgQ->Confirm( hostorder16(HartPDUHdr::access(p_pPkt)->m_ui16TransactId)
			, HartPDUHdr::access(p_pPkt)->m_nStatus
			);

	if ( ! HartStatusOk::SessionInit( HartPDUHdr::access(p_pPkt)->m_nStatus ) )
		return  ;

	m_pMsgQ->Unblock();
	p_pPkt->PopFront( NULL, sizeof(HartPDUHdr) );
	uint32_t closeTime = hostorder32(HartSessionInitArgv::access(p_pPkt)->m_nCloseTime) ;
	unsigned remotePort = ((CUdpLink*)*this)->PktRemotePort() ;
	if ( remotePort > 0xFFFFU )
	{
		WARN( "%s [Invalid port] (remotePort:%u)", m_szName, remotePort);
		return ;
	}

	LOG( "%s (inactivityCloseTime:%u remotePort:%u)", m_szName, closeTime, remotePort );
	((CUdpLink*)*this)->RemotePort(remotePort) ;
	m_nKeepAliveTimeout = closeTime ;
	armKeepAliveTimer() ;
	m_bForceDisconnect = false ;
	m_nLinkState = EUDP_LINK_CONNECTED ;
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Handle a Session Close response.
/// @retval none
/// @param p_pPkt Data received from the link.
//////////////////////////////////////////////////////////////////////////////
void CHartUpLink::sessionCloseHandler(ProtocolPacketPtr& p_pPkt )
{
	m_nLinkState = EUDP_LINK_DISCONNECTED ;
	m_fpDisconnectedCallback();
	((CUdpLink*)*this)->RemotePort(m_nBaseUdpPort) ;

	if ( ! HartStatusOk::SessionClose(HartPDUHdr::access(p_pPkt)->m_nStatus) )
		return  ;
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Handle an Invalid Packet.
/// @retval none
/// @param p_pPkt Data received from the link.
//////////////////////////////////////////////////////////////////////////////
void CHartUpLink::invalidPacketHandler(ProtocolPacketPtr& p_pPkt)
{
	LOG( "%s EPKT_INVALID %u bytes", m_szName, p_pPkt->DataLen());
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Handle a Keep Alive response.
/// @retval none
/// @param p_pPkt Data received from the link.
//////////////////////////////////////////////////////////////////////////////
void CHartUpLink::keepAliveHandler( ProtocolPacketPtr& p_pPkt )
{
	if ( ! HartStatusOk::KeepAlive( HartPDUHdr::access(p_pPkt)->m_nStatus ) )
		return  ;
	armKeepAliveTimer() ;
	m_bForceDisconnect = false ;
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Handle a Wireless Request response.
/// @retval false The packet must die
/// @retval true Packet won't be deleted
/// @param p_pPkt Data received from the link.
//////////////////////////////////////////////////////////////////////////////
bool CHartUpLink::wirelessRequestHandler( ProtocolPacketPtr& p_pPkt )
{
	if ( ! HartStatusOk::WirelessRequest( HartPDUHdr::access(p_pPkt)->m_nStatus ) )
		return false ;

	m_fpWirelessRequestCallback( p_pPkt ) ;

//	armKeepAliveTimer() ;
//	m_bForceDisconnect = false ;

	if ( sizeof(HartPDUHdr) == HartPDUHdr::access(p_pPkt)->GetByteCountNtoH() )
	{
		m_pMsgQ->Confirm( hostorder16(HartPDUHdr::access(p_pPkt)->m_ui16TransactId)
				, HartPDUHdr::access(p_pPkt)->m_nStatus
				);
		return false ;
	}

	if ( sizeof(HartPDUHdr) < HartPDUHdr::access(p_pPkt)->GetByteCountNtoH() )
	{
//		LOG( "%s [Tunnel] (TransactId:x%04X Pkt.DataLen=%u)", m_szName
//			, hostorder16(HartPDUHdr::access(p_pPkt)->m_ui16TransactId)
//			, p_pPkt->DataLen()
//			);

		SendAck(HartPDUHdr::access(p_pPkt)->m_ui16TransactId);
		return true ;
	}
	return false ;
}

void CHartUpLink::SendAck( uint16_t transactionId )
{

	if ( m_nLinkState == EUDP_LINK_DISCONNECTED )
		return SessionInit() ;

	HartPDUHdr hdr( CHartUpMatcher::EPKT_WREQUEST );
	hdr.m_nMsgFlagType = 0x11;	// response
	hdr.m_nStatus = 0;
	hdr.SetByteCount( networkorder16(sizeof(HartPDUHdr)) );

	ProtocolPacketPtr pkt = ProtocolPacketPtr(new ProtocolPacket) ;
	pkt->AllocateCopy( (uint8_t*)&hdr, sizeof(hdr), 0, 0 ) ;

	HartPDUHdr::access(pkt)->m_ui16TransactId = transactionId ;
	transactionId = hostorder16(transactionId);

	unsigned handle = m_pMsgQ->Enque( EQ_UDP_KEEP_ALIVE, pkt, transactionId );
	if (handle)
	{
	    m_pMsgQ->StartXmit() ;
	}
}
