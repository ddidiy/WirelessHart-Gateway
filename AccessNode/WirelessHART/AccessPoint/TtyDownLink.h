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

#ifndef _TTY_DOWN_LINK_H_
#define _TTY_DOWN_LINK_H_

// $Id: TtyDownLink.h,v 1.20.4.1 2013/05/15 19:19:17 owlman Exp $

//////////////////////////////////////////////////////////////////////////////
/// @author	Marius Negreanu
/// @date	Thu Dec 18 13:40:22 2008 UTC
/// @brief  Transceiver TTY link.
//////////////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include "Shared/h.h"
#include "Shared/PacketStream.h"
#include "App.h"
#include "MsgMultiQueue.h"
#include "TtyDownMatcher.h"
#include "TtyDownHdr.h"


enum ETTY_QUEUE_PRIORITY {
	EQ_TTY_ACK,
	EQ_TTY_NORMAL,
	EQ_TTY_MAX_PRIO
};


#define CRC_LENGTH 2
#define Q_TTY_NORMAL_CAPACITY	512
#define Q_TTY_ACK_CAPACITY		512


//////////////////////////////////////////////////////////////////////////////
/// @class CTtyDownLink
/// @brief Transceiver TTY link.
/// @ingroup AccessPoint
//////////////////////////////////////////////////////////////////////////////
class CTtyDownLink : public CPacketStream {
public:
	CTtyDownLink(CWHAccessPointCfg& cfg, bool p_bRawLog=false, bool useEscape=true ) ;
	~CTtyDownLink() {}
	ProtocolPacketPtr Read() ;
	void Request( ProtocolPacketPtr&  pkt ) ;
	void Refresh() { m_oMsgQ.Refresh();}
	void StartXmit() { m_oMsgQ.StartXmit() ;}
	void SendAck( unsigned int p_nHandle ) ;

	void ClearQueues() {
		m_oMsgQ.Clear(EQ_TTY_MAX_PRIO);
	}


	void ResetTR();


protected:
	bool ntwkAckHandler( ProtocolPacketPtr& pkt ) ;
	bool ntwkMsgHandler( ProtocolPacketPtr& pkt ) ;
	bool writeNetInfo( ProtocolPacketPtr& pkt );
	bool reqDiscHandler(ProtocolPacketPtr& p_pPkt);
	void writeReqDisconnect(uint16_t msgHandle = 0xFFFE);

protected:
	CWHAccessPointCfg& cfg;
	CMsgMultiQueue	m_oMsgQ ;
	int lastCommunicationTime;
} ;

#endif
