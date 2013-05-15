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

#ifndef _EVENT_LOOP_H_
#define _EVENT_LOOP_H_

// $Id: EventLoop.h,v 1.10.14.1 2013/05/15 19:19:17 owlman Exp $

//////////////////////////////////////////////////////////////////////////////
/// @author	Marius Negreanu
/// @date	Thu Dec 18 13:40:22 2008 UTC
/// @brief	The Tunneling class between Transceiver and Gateway.
//////////////////////////////////////////////////////////////////////////////

#include "HartUpMatcher.h"
#include "HartUpLink.h"

#include "TtyDownMatcher.h"
#include "TtyDownLink.h"

#include "Shared/PacketStream.h"
#include "Shared/SimpleTimer.h"


//////////////////////////////////////////////////////////////////////////////
/// @class CEventLoop
/// @brief The Tunneling class between Transceiver and Gateway.
/// @ingroup AccessPoint
//////////////////////////////////////////////////////////////////////////////
class CEventLoop
{
public:
	CEventLoop( CWHAccessPointCfg& p_oConfig ) ;	///< Default Constructor
	~CEventLoop() ;	///< Destructor

public:
/// Start a CEventLoop.
	int	Start() ;
/// Main event handling loop.
	int	Run() ;
/// Stop the CEventLoop.
	int	Stop() ;
/// Send TTYNtwkAckOutAck when receiving CHartUpMatcher::EPKT_WREQUEST
	void WirelessRequestCallback(ProtocolPacketPtr& p_pPkt) ;
	void GWLinkDisconnected();
private:
/// Read and handle the UDP link.
	bool	processUp() ;
/// Read Handle the TTY link.
	bool	processDown() ;
/// Convert an DownLink packet to a UpLink packet.
	bool	convertUpToDown( ProtocolPacketPtr& p_pPkt ) ;
/// Convert an DownLink packet to a UpLink packet.
	bool	convertDownToUp( ProtocolPacketPtr& p_pPkt ) ;

private:
	CHartUpLink	*m_pUpGwLnk ;	///< Gateway UDP link.
	CTtyDownLink	*m_pDownTrLnk ;	///< Transceiver TTY link.
	CWHAccessPointCfg& 		m_oConfig ;
};


#endif	//_EVENT_LOOP_H_
