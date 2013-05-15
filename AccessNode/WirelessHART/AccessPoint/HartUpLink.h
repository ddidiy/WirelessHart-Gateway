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

#ifndef _HART_UP_LINK_H_
#define _HART_UP_LINK_H_

// $Id: HartUpLink.h,v 1.23.6.1.2.1 2013/05/15 19:19:17 owlman Exp $

//////////////////////////////////////////////////////////////////////////////
/// @author	Marius Negreanu
/// @date	Thu Dec 18 13:40:22 2008 UTC
//////////////////////////////////////////////////////////////////////////////

#include "Shared/PacketStream.h"
#include "HartUpMsgMultiQueue.h"
#include "HartUpHdr.h"

#define HART_PRIMARY_MASTER 1
#define HART_SESSION_EXPIRE ((uint32_t)300)
#define UDP_LONG_ADDR_SIZE 8
#define UDP_SHORT_ADDR_SIZE 2


enum EUDP_QUEUE_PRIORITY {
	EQ_UDP_KEEP_ALIVE,
	EQ_UDP_NORMAL,
	EQ_UDP_MAX_PRIO
} ;

enum UDP_LINK_STATE {
	EUDP_LINK_DISCONNECTED,
	EUDP_LINK_CONNECTED,
	EUDP_MAX_STATES
} ;

//////////////////////////////////////////////////////////////////////////////
/// @class CHartUpLink
/// @brief Gateway UDP link.
/// @ingroup AccessPoint
//////////////////////////////////////////////////////////////////////////////
class CHartUpLink : public CPacketStream {
public:
	CHartUpLink( bool rawLog=false, bool escape=true)
		: CPacketStream::CPacketStream(rawLog,escape)
		, m_nKeepAliveTimeout(10*1000)
		, m_nAckTimeout(3)
		, m_nLinkState(EUDP_LINK_DISCONNECTED)
		, m_bForceDisconnect(false)
		, m_nHandle(QHANDLE_MAX)
		, m_szName("AP=>GW")
	{
		m_pMsgQ = new CHartUpMsgMultiQueue() ;
		m_pMsgQ->MaxPriorities(EQ_UDP_MAX_PRIO);
		m_pMsgQ->Capacity( EQ_UDP_NORMAL, 512);
		m_pMsgQ->Capacity( EQ_UDP_KEEP_ALIVE, 512);
		m_pMsgQ->MaxRetries(EQ_UDP_KEEP_ALIVE, 1);
		m_pMsgQ->IsBlockable(EQ_UDP_KEEP_ALIVE, false);
//		m_pMsgQ->MaxRetries(EQ_UDP_MAX_PRIO, 0);
		m_pMsgQ->Name("AP->GQ");
		m_pMsgQ->AnchorLink( this ) ;
		armKeepAliveTimer();
	}

	/// Opens link
	int OpenLink( char const* p_sConn, unsigned int  p_nConnParam, unsigned int p_nConnParam2 = 0  )
	{
		m_nBaseUdpPort = p_nConnParam ;
		return CPacketStream::OpenLink(p_sConn, p_nConnParam, p_nConnParam2 ) ;
	}
	~CHartUpLink()
	{
		delete m_pMsgQ ;
	}
	unsigned int& State() { return m_nLinkState; }

	/// Callbacks
	void WirelessRequestCallback(Callback<void, ProtocolPacketPtr&> p_callback) {
		m_fpWirelessRequestCallback = p_callback;
	}
	void DisconnectedCallback(Callback<void> p_callback) {
		m_fpDisconnectedCallback = p_callback;
	}

	/// Clears message queue
	void ClearQueues() {
		m_pMsgQ->Clear(EQ_UDP_MAX_PRIO);
	}

	/// Handlers for messages
	void SessionInit() ;
	void SessionClose() ;
	void KeepAlive() ;

	/// Checks timeouts
	void Refresh()
	{
		m_pMsgQ->Refresh() ;


		if ( m_oKeepAliveTimer.IsSignaling() )
		{
			if ( ! CheckSession() )
			{
				armKeepAliveTimer();
				return ;
			}
			if ( m_bForceDisconnect )
			{
				LOG( "%s [Forcing disconnection: call SesionClose]", m_szName);
				SessionClose() ;
				return ;
			}
			KeepAlive() ;
			armKeepAliveTimer();
		}
	}

	/// Get/Set ACK timeout
	void AckTimeout(unsigned int p_nAckTimeout ) { m_nAckTimeout = p_nAckTimeout; }
	unsigned int AckTimeout() const { return m_nAckTimeout ; }
	/// Transmits one packet from the queue
	void StartXmit() { m_pMsgQ->StartXmit() ;}
	/// Reads a packet from the UDP link
	ProtocolPacketPtr Read() ;
	void WirelessRequest( uint8_t p_rgMsg[], size_t p_nMsgSz, unsigned int p_nHandle ) ;
	/// Checks session for communication.
	bool CheckSession()
	{
		if ( m_nLinkState == EUDP_LINK_CONNECTED )
			return true ;

		LOG( "%s [Link lost; reconnecting]", m_szName ) ;
		SessionInit();
		return false;
	}
protected:
	void sessionInitHandler(ProtocolPacketPtr& p_pPkt ) ;
	void sessionCloseHandler(ProtocolPacketPtr& p_pPkt ) ;
	void invalidPacketHandler(ProtocolPacketPtr& p_pPkt) ;
	void keepAliveHandler(ProtocolPacketPtr& p_pPkt );
	bool wirelessRequestHandler(ProtocolPacketPtr& p_pPkt );
	void SendAck( uint16_t transactionId );
	/// @brief Reset the keepalive timer the connection state.
	void armKeepAliveTimer()
	{
		//LOG( "%s UpLink.KeepaAliveTimer [reset] (keepAliveTimeout:%u)", m_szName, m_nKeepAliveTimeout);
		m_oKeepAliveTimer.SetTimer(m_nKeepAliveTimeout);
	}
protected:
	CHartUpMsgMultiQueue	*m_pMsgQ ;
	CSimpleTimer	m_oKeepAliveTimer ;	///< Timer for KeepAlive sendings.
	unsigned int	m_nKeepAliveTimeout;///< Miliseconds of Link inactivity.
	unsigned int	m_nAckTimeout ;		///< Miliseconds to wait for a GW ACK
	unsigned int	m_nLinkState ;		///<
	unsigned int	m_nBaseUdpPort ;	///<
	unsigned int	m_nRemoteUdpPort ;
	bool		m_bForceDisconnect ;///< If true, set m_nLinkState=EUDP_LINK_DISCONNECTED
	unsigned int	m_nHandle ;			///< Handle for packets originating in AP not in TR.
	Callback<void, ProtocolPacketPtr&> m_fpWirelessRequestCallback ;
	Callback<void> m_fpDisconnectedCallback ;
	const char*	m_szName ;			///< link name
} ;

#endif // _HART_UP_LINK_H_
