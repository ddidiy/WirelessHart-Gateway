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

#ifndef _MSG_QUEUE_H_
#define _MSG_QUEUE_H_

// $Id: MsgQueue.h,v 1.34.4.1 2013/05/15 19:19:17 owlman Exp $.

//////////////////////////////////////////////////////////////////////////////
/// @author	Marius Negreanu.
/// @date	Thu Dec 18 13:40:22 2008 UTC.
/// @brief  Implement a timed flow controlled message queue (packet scheduler)
//////////////////////////////////////////////////////////////////////////////

#include "Shared/h.h"
#include "Shared/SimpleTimer.h"
#include "Shared/ProtocolPacket.h"
#include "Shared/PacketStream.h"
#include "Callback.h"

#include <vector>
#include <boost/shared_ptr.hpp>

//////////////////////////////////////////////////////////////////////////////
/// The message will be retried p_nMaxRetries after its deadline time expires.
/// The message will be dropped if it is not acknowledged after p_nMaxRetries.
//////////////////////////////////////////////////////////////////////////////

/// Maximum alocated Queue Handle after which it overflows to 0.
#define QHANDLE_MAX 0x1FFFU

/// @todo see if the qstate can be separated from the packet type.
enum EMSG_QUEUE_STATE {
	EQSTATE_UNBLOCKED=0x00,
	EQSTATE_BLOCKED=0x01,
	EQSTATE_OUT_OF_MEMORY=0x07,
	EQSTATE_TIMEOUT=0x15,
	EQSTATE_TR_BUFFER_FULL= 0x3D,	//RC 61 - buffer full
	EQSTATE_INVALID
};


typedef boost::shared_ptr<ProtocolPacket> ProtocolPacketPtr;

//////////////////////////////////////////////////////////////////////////////
/// @class CMsgQueue
/// @ingroup AccessPoint
/// @brief Implement a timed flow controlled message queue(packet scheduler).
//////////////////////////////////////////////////////////////////////////////
class CMsgQueue {
public:
	CMsgQueue(	unsigned int p_nCapacity=20, bool p_bLogRaw=true
			, unsigned int p_nRetryTimeout=4, unsigned int p_nAckTimeout=2
			, unsigned p_nMaxRetries=3, unsigned int maxSentPackets_ = 1
		)
		: m_pAnchor(NULL)
		, m_nState(EQSTATE_UNBLOCKED)
		, m_nRetryTimeout(p_nRetryTimeout)
		, m_nAckTimeout(p_nAckTimeout)
		, m_nCapacity(p_nCapacity)
		, m_nInsertPos(0)
		, m_nSize(0)
		, m_nHandle(0)
		, m_nMaxRetries(p_nMaxRetries)
		, m_bHasGaps(false)
		, m_bRawLog(p_bLogRaw)
		, sentPacketCount(0)
		, maxSentPackets(maxSentPackets_)
		, m_bIsBlockable(true)
	{
//		m_prgBucket = new NodeInfo [m_nCapacity] ;
		strcpy(m_szName, "NONE");
		LOG( "%s MsgQ.ctor        (capacity:%u retryTimeout:%u ackTimeout:%u maxRetries:%u)", m_szName
			, p_nCapacity
			, p_nRetryTimeout
			, p_nAckTimeout
			, p_nMaxRetries
		);
	}

	virtual ~CMsgQueue()
	{
//		delete [] m_prgBucket ;
		while (m_prgBucket.size() > 0)
		{
			remove(m_prgBucket.begin());
		}
	}
public:
	virtual unsigned Enque( ProtocolPacketPtr& p_pElm, unsigned p_nHandle=0 ) ;
	bool Confirm( unsigned p_nHandle, unsigned p_nStatus) ;
	virtual bool DropExpired() ;
	bool Shrink() ;
	bool StartXmit( ) ;
//////////////////////////////////////////////////////////////////////////////
/// @brief Set the Queue Name.
/// @param[in] p_szName The queue name.
/// @details The name of the queue is used in logs.
//////////////////////////////////////////////////////////////////////////////
	void Name( char const * p_szName )
	{
		strncpy( m_szName, p_szName, sizeof(m_szName)-1 ) ;
		m_szName[sizeof(m_szName)-1] = '\0' ;
	}
//////////////////////////////////////////////////////////////////////////////
/// @brief Set the Queue maximum capacity.
/// @param[in] p_nCapacity The Maximum number of packets the queue will hold.
//. @note Calling this method will destroy its old content.
//////////////////////////////////////////////////////////////////////////////
	void Capacity( unsigned int p_nCapacity )
	{
//		delete [] m_prgBucket ;
		m_nCapacity = p_nCapacity ;
//		m_prgBucket = new NodeInfo [m_nCapacity] ;
	}
//////////////////////////////////////////////////////////////////////////////
/// @brief Set the maximum attempts to send a packet.
/// @param[in] p_nMaxRetries Maximum attempts to send a packet.
//////////////////////////////////////////////////////////////////////////////
	void MaxRetries( unsigned int p_nMaxRetries )
	{
		m_nMaxRetries = p_nMaxRetries ;
	}

//////////////////////////////////////////////////////////////////////////////
/// @brief Set the queue to be blockable or not.
/// @param[in] m_bIsBlockable true or false.
//////////////////////////////////////////////////////////////////////////////
	void IsBlockable(bool blockable)
	{
		m_bIsBlockable = blockable;
		if (!m_bIsBlockable)
		{
			m_nState = EQSTATE_UNBLOCKED;
		}
	}

//////////////////////////////////////////////////////////////////////////////
/// @brief Block the message Queue.
/// @details A blocked queue will not send packets.
/// @see Unblock and Clear.
//////////////////////////////////////////////////////////////////////////////
	/*virtual*/ void Block( )	// make virtual if you want to override in UpMsgQueue to make non-blocking
	{
		if (!m_bIsBlockable) return;

		if ( m_nState == EQSTATE_BLOCKED ) return ;
//		LOG( "%s MsgQ.State       (State:block Qsz:%u)", m_szName, m_prgBucket.size() );
		m_nState = EQSTATE_BLOCKED ;
	}
//////////////////////////////////////////////////////////////////////////////
/// @brief Unblock the message queue.
//////////////////////////////////////////////////////////////////////////////
	void Unblock( )
	{
		if (!m_bIsBlockable) return;

		if ( m_nState == EQSTATE_UNBLOCKED ) return ;
//		LOG( "%s MsgQ.State       (State:release Qsz:%u)", m_szName, m_prgBucket.size() );
		m_nState = EQSTATE_UNBLOCKED ;
	}
//////////////////////////////////////////////////////////////////////////////
/// @brief Remove all packets from the Message Queue and reset its state.
//////////////////////////////////////////////////////////////////////////////
	void Clear( )
	{
		m_nState=EQSTATE_UNBLOCKED;
//		m_nInsertPos=0 ;
//		m_nSize=0 ;
		m_bHasGaps=false ;
//		for (NodeInfoPtrList::iterator it = m_prgBucket.begin(); it != m_prgBucket.end(); ++it)
//		{
//			if (it->pkt)
//				delete (it->pkt);
//		}
		m_prgBucket.clear();
	}
//////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////
	void AnchorLink( CPacketStream* p_pAnchor )
	{
		m_pAnchor = p_pAnchor ;
	}

public:
	struct NodeInfo {
		ProtocolPacketPtr pkt ;
		time_t		m_nPktLifeTime ;
		time_t		m_nNextSendTimestamp ;
		uint16_t	m_nHandle ;
		uint8_t		m_nRetries ;
		bool 		toBeErased;
	} ;
	typedef std::vector<NodeInfo> NodeInfoPtrList;

	void remove (NodeInfoPtrList::iterator it)
	{
//		if (it->pkt)
//		{
//			delete (it)->pkt ;
//		}
//		NodeInfo* nodeInfo = *it;
		m_prgBucket.erase(it);

//		delete nodeInfo;
	}

protected:
	char		m_szName[16] ;
	NodeInfoPtrList m_prgBucket;
	CPacketStream*	m_pAnchor ;
	unsigned int	m_nState ;
	unsigned int    sentPacketCount;
	unsigned int	m_nRetryTimeout ;
	unsigned int	m_nAckTimeout ;
	unsigned int	m_nCapacity ;	///< Maximum of messages to hold.
	uint16_t	m_nInsertPos ;	///< The end of the queue.
	uint16_t	m_nSize ;		///< The number of elements in queue.
	uint16_t	m_nHandle ;		///< Next available handle
	uint8_t		m_nMaxRetries ;	///< The times to send a packet without receiving Acknowledgment.
	bool		m_bHasGaps ;	///< Elements were removed.
	bool		m_bRawLog ;
	bool 		m_bIsBlockable;
	unsigned int 	maxSentPackets;
} ;
#endif
