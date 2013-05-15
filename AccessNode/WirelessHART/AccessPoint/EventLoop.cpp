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

// $Id: EventLoop.cpp,v 1.30.4.1 2013/05/15 19:19:17 owlman Exp $

//////////////////////////////////////////////////////////////////////////////
/// @file	EventLoop.cpp
/// @author	Marius Negreanu
/// @date	Thu Dec 18 13:40:22 2008 UTC
//////////////////////////////////////////////////////////////////////////////

#include <unistd.h>

#include "Shared/h.h"
#include "App.h"
#include "EventLoop.h"
#include "Callback.h"


/// @brief Send TTYNtwkAckOutAck when receiving CHartUpMatcher::EPKT_WREQUEST
void CEventLoop::WirelessRequestCallback(ProtocolPacketPtr& p_pPkt)
{
	/// @todo Remove the mask
	if ( p_pPkt->DataLen() == sizeof(HartPDUHdr) )
	{
		LOG( "AP=>TR [ACK] (transactId:x%04X Pkt.DataLen:%u)"
			, hostorder16(HartPDUHdr::access(p_pPkt)->m_ui16TransactId) & 0x1FFF
			, p_pPkt->DataLen()
		);
//		m_pDownTrLnk->SendAck( networkorder16( hostorder16(HartPDUHdr::access(p_pPkt)->m_ui16TransactId) & 0x1FFF) ) ;
	}
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Default Constructor.
//////////////////////////////////////////////////////////////////////////////
CEventLoop::CEventLoop( CWHAccessPointCfg& p_oConfig)
	:m_pUpGwLnk(NULL)
	,m_pDownTrLnk(NULL)
	,m_oConfig(p_oConfig)
{
	;
}


//////////////////////////////////////////////////////////////////////////////
/// @brief Destructor.
//////////////////////////////////////////////////////////////////////////////
CEventLoop::~CEventLoop()
{
	;
}


//////////////////////////////////////////////////////////////////////////////
/// @brief Start a CEventLoop.
/// @retval true  Success. The links communication links were openned.
/// @retval false Failure.
/// @details Opens a UDP link to SystemManager and a Serial Link to
/// Transceiver. Creates the Retransmission Queues.
//////////////////////////////////////////////////////////////////////////////
int CEventLoop::Start( )
{
	int rv =0;

	m_pUpGwLnk = new CHartUpLink( m_oConfig.RawLog(), false ) ;
	m_pUpGwLnk->Create<CUdpLink> () ;
	rv = m_pUpGwLnk->OpenLink( m_oConfig.m_szGW_IPv4, m_oConfig.m_nGW_Port, m_oConfig.m_nPort ) ;
	if( ! rv )
	{
		ERR("CEventLoop.Start: Failed to Create UP Link.");
		return false ;
	}
	m_pUpGwLnk->AckTimeout(m_oConfig.m_nUdpTimeout);
	m_pUpGwLnk->WirelessRequestCallback( make_callback(&CEventLoop::WirelessRequestCallback,this) ) ;
	m_pUpGwLnk->DisconnectedCallback( make_callback(&CEventLoop::GWLinkDisconnected, this));

	m_pDownTrLnk = new CTtyDownLink( m_oConfig, m_oConfig.RawLog() ) ;
	m_pDownTrLnk->Create < CSerialLink, CNivisFrameParser_v3 > () ;
	rv = m_pDownTrLnk->OpenLink( m_oConfig.m_szTtyDev , m_oConfig.m_nTtyBauds ) ;
	if ( ! rv )
	{
		ERR("CEventLoop.Start: Failed to Create Down Link.");
		return false ;
	}

	GWLinkDisconnected();
	//m_pDownTrLnk->AckTimeout(m_oConfig.m_nTtyTimeout);
	m_pUpGwLnk->SessionInit() ;
	m_pUpGwLnk->SetOffsets(0,CRC_LENGTH);
	return true ;
}

void CEventLoop::GWLinkDisconnected()
{
	m_pDownTrLnk->ResetTR();
	m_pDownTrLnk->ClearQueues();
	m_pUpGwLnk->ClearQueues();
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Main event handling loop.
/// @retval true Allways.
//////////////////////////////////////////////////////////////////////////////
int CEventLoop::Run()
{
	m_pUpGwLnk->Refresh() ;
	m_pDownTrLnk->Refresh() ;

	processUp() ;
	m_pUpGwLnk->StartXmit() ;

	processDown() ;
	m_pDownTrLnk->StartXmit() ;
	return true ;
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Stop the CEventLoop.
/// @retval true Allways.
//////////////////////////////////////////////////////////////////////////////
int CEventLoop::Stop()
{
	m_pUpGwLnk->CloseLink();
	m_pDownTrLnk->CloseLink();
	return true ;
}


//////////////////////////////////////////////////////////////////////////////
/// @brief Read and handle the UDP link.
//////////////////////////////////////////////////////////////////////////////
bool CEventLoop::processUp()
{

	ProtocolPacketPtr pkt = ProtocolPacketPtr(m_pUpGwLnk->Read()) ;
	if ( !pkt ) return false ;

	if ( pkt->DataLen() < sizeof(HartPDUHdr)
	||   pkt->DataLen() != HartPDUHdr::access(pkt)->GetByteCountNtoH()
		)
	{
		WARN("Invalid packet size (DataLen:%u ByteCount:%u).", pkt->DataLen()
			, hostorder16(HartPDUHdr::access(pkt)->GetByteCount() ) );
		//delete pkt ;
		return false ;
	}
	if ( !convertUpToDown(pkt) )
	{
		//delete pkt ;
		WARN("GW=>AP: Failed to convert to downLinkFormat.");
		return false ;
	}
	return true ;
}


//////////////////////////////////////////////////////////////////////////////
/// @brief Read Handle the TTY link.
/// @retval true  The conversion was done successfully.
/// @retval false No data available OR the conversion failed.
//////////////////////////////////////////////////////////////////////////////
bool CEventLoop::processDown( )
{
	ProtocolPacketPtr pkt = m_pDownTrLnk->Read( ) ;
	if ( !pkt ) return false ;

	if ( !convertDownToUp(pkt) )
	{
//		delete pkt ;
		WARN("TR=>AP: Failed to convert to UpLink Format.");
		return false ;
	}
	return true ;
}




//////////////////////////////////////////////////////////////////////////////
/// @brief Convert an UpLink packet to a downlink packet.
/// @param[in][out] pkt The uplink packet to be converted to a downlink packet
/// @details The conversion is done inplace and consists of removing the
/// HartPDUHdr and enqueueing the packet in DownLink queue.
//////////////////////////////////////////////////////////////////////////////
bool CEventLoop::convertUpToDown( ProtocolPacketPtr& pkt )
{
	if ( ! pkt ) return false ;

	pkt->PopFront(NULL, sizeof(HartPDUHdr) ) ;
	m_pDownTrLnk->Request( pkt ) ;
	return true ;
}


//////////////////////////////////////////////////////////////////////////////
/// @brief Convert an DownLink packet to a UpLink packet.
/// @param[in][out] pkt The uplink packet to be converted to a downlink packet
/// @details The conversion is done inplace and consists of removing the
/// TTYNtwkHdr and enqueueing the packet in UpLink queue.
//////////////////////////////////////////////////////////////////////////////
bool CEventLoop::convertDownToUp( ProtocolPacketPtr& pkt )
{
	if ( ! pkt ) return false ;
	TTYNtwkHdr tHdr ;
	pkt->PopFront( &tHdr, sizeof(TTYNtwkHdr) ) ;
	if ( ! pkt->DataLen() ) ///< GW doesn' need the TR empty sync packets
		return true ;
	m_pUpGwLnk->WirelessRequest( pkt->Data(), pkt->DataLen(), tHdr.m_nHandle ) ;
	return true ;
}

