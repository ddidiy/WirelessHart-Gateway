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

// $Id: MsgQueue.cpp,v 1.41.20.1 2013/05/15 19:19:17 owlman Exp $

//////////////////////////////////////////////////////////////////////////////
/// @author	Marius Negreanu
/// @date	Thu Dec 18 13:40:22 2008 UTC
//////////////////////////////////////////////////////////////////////////////

#include "MsgQueue.h"

#include "Shared/Time.h"
#include "Shared/PacketStream.h"

#include <algorithm>
//////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////
unsigned CMsgQueue::Enque( ProtocolPacketPtr& p_pElm , unsigned p_nHandle/*=0*/)
{
	// Test Queue Overflow
	if ( m_prgBucket.size() >= m_nCapacity )
	{
//		m_bHasGaps = true ;
//		Shrink() ;
//		if ( m_nInsertPos >= m_nCapacity )
//		{
			LOG( "%s MsgQ.Enque [Queue full] (Handle:x%04X Capacity:%u Qsz:%u State:x%02X)", m_szName
				, m_nHandle
				, m_nCapacity
				, m_prgBucket.size()
				, m_nState
			);
			return false ;
//		}
	}

	m_prgBucket.push_back(NodeInfo());

	NodeInfo& info = m_prgBucket.back();
	info.pkt = p_pElm ;
	info.m_nPktLifeTime = 0 ;
	info.m_nNextSendTimestamp = 0 ;
	info.m_nRetries = 0 ;
	info.m_nHandle = p_nHandle | m_nHandle++;
	info.toBeErased = false;

//	m_prgBucket.push_back(info);

	m_nHandle %= QHANDLE_MAX+1 ;
	LOG( "%s MsqQ.Enque [Msg enqued] (Handle:x%04X Capacity:%u Qsz:%i State:x%02X)", m_szName
		, info.m_nHandle
		, m_prgBucket.capacity()
		, m_prgBucket.size()
		, m_nState
	) ;
	return info.m_nHandle ;
}
//////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////
bool CMsgQueue::Confirm(
	unsigned p_nHandle , unsigned p_nStatus )
{
	//DumpTimestamp() ;
	for (NodeInfoPtrList::iterator it = m_prgBucket.begin(); it != m_prgBucket.end(); it++)
//	for ( unsigned idx = 0; idx < m_nInsertPos; ++idx )
	{
		if ( it->m_nHandle == p_nHandle )
		{
			if ( ! it->pkt )
			{
//				LOG( "%s MsgQ.Confirm (Handle:x%04X Retry:%d Qsz:%d State:x%02X)"
//						, m_szName
//						, it->m_nHandle
//						, it->m_nRetries
//						, m_prgBucket.size()
//						, m_nState
//						) ;
				remove(it);
				Unblock() ;
				return true ;
			}

			if ( p_nStatus == EQSTATE_OUT_OF_MEMORY )
			{
				it->m_nNextSendTimestamp -= m_nAckTimeout ;
				it->m_nNextSendTimestamp += m_nRetryTimeout;
				++(it->m_nRetries) ;
//				LOG( "%s MsgQ.Add.Confirm (Handle:x%04X Qsz:%d Retry:%d Status:x07 OUT_OF_MEM)"
//						, m_szName
//						, it->m_nHandle
//						, m_prgBucket.size()
//						, it->m_nRetries ) ;
				return true ;
			}

			if (p_nStatus == EQSTATE_TR_BUFFER_FULL)
			{
				it->m_nNextSendTimestamp -= m_nAckTimeout ;
				it->m_nNextSendTimestamp += m_nRetryTimeout;
				++(it->m_nRetries) ;

				return true;
			}

			/*if (p_nStatus!=EQSTATE_UNBLOCKED )
			{
			}*/

//			LOG( "%s MsgQ.Del.Confirm (Handle:x%04X Qsz:%d Status:x%02X %s)"
//					, m_szName
//					, it->m_nHandle
//					, m_prgBucket.size()
//					, p_nStatus
//					,	(p_nStatus==EQSTATE_UNBLOCKED)		? " (success)"
//					: (p_nStatus==EQSTATE_TIMEOUT)	? " (TIMEOUT)"
//					: " (?)" ) ;
			remove(it);
			Unblock() ;

			return true;
		}
	}
	WARN( "%s MsgQ.Confirm Handle not found (Handle:x%04X Qsz:%d)"
		, m_szName
		, p_nHandle
		, m_prgBucket.size()
		);
	return false ;
}

struct ExpiredPacketEraser
{
	bool operator()(CMsgQueue::NodeInfo& node)
	{
		return node.toBeErased;
	}
};


//////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////
bool CMsgQueue::DropExpired()
{
//	LOG ( " QSize before DropExpired: %d", m_prgBucket.size());
	time_t now = time(NULL) - (m_nMaxRetries + 1) * m_nAckTimeout ;	//TODO check when can a packet expire and not be deleted from StartXmit()
	//TODO refactor to remove all


	for (NodeInfoPtrList::iterator it = m_prgBucket.begin(); it != m_prgBucket.end(); it++)
//	for( unsigned idx = 0; idx < m_nInsertPos; idx ++ )
	{
		if ( it->pkt
		     && ( (it->m_nRetries > m_nMaxRetries)	// a packet with no ACK after it was retried enough times
		          || (it->m_nPktLifeTime		// a packet with no ACK in it's lifetime
			       && now >= it->m_nPktLifeTime)
		        )
		   )
		{
			WARN( "%s MsqQ.DropExpired (Handle:%u, m_nPktLifeTime:%u m_nRetries:%u m_nMaxRetries:%u Qsz:%u State:x%02X)", m_szName
				, it->m_nHandle
				, now - it->m_nPktLifeTime + (m_nMaxRetries + 1) * m_nAckTimeout
				, it->m_nRetries
				, m_nMaxRetries
				, m_prgBucket.size()
				, m_nState
				) ;
			it->toBeErased = true;
//			remove(it);
		}
	}

	NodeInfoPtrList::iterator firstRemoved = std::remove_if(m_prgBucket.begin(), m_prgBucket.end(), ExpiredPacketEraser());
//	for (NodeInfoPtrList::iterator it = firstRemoved; it != m_prgBucket.end(); ++it )
//		if ((it)->pkt)
//		{
//			delete (it)->pkt ;
//		}
	if (firstRemoved != m_prgBucket.end())
	{
		m_prgBucket.erase(firstRemoved, m_prgBucket.end());
	}

//	LOG ( " QSize after DropExpired: %d", m_prgBucket.size());
	return true ;
}
//////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////
bool CMsgQueue::Shrink()
{
	return true;
}
//////////////////////////////////////////////////////////////////////////////
/// Uses TIMER/AnchorLink.
//////////////////////////////////////////////////////////////////////////////
bool CMsgQueue::StartXmit( )
{
	assert( m_pAnchor ) ;
	if ( m_nState == EQSTATE_BLOCKED ) {
		return false ;
	}

	time_t tv = time(NULL);

	//DumpTimestamp() ;
	for (NodeInfoPtrList::iterator it = m_prgBucket.begin(); it != m_prgBucket.end(); it++)
//	for ( unsigned idx=0; idx < m_nInsertPos; ++idx )
	{
		if ( it->pkt
		&&   it->m_nNextSendTimestamp <= tv )
		{
			++(it->m_nRetries) ;
			//don't resend, unless a confirm says out-of-mem
			it->m_nNextSendTimestamp = tv + m_nAckTimeout ;
			//it's the first sending, record its time
			if( ! it->m_nPktLifeTime )
			{
				it->m_nPktLifeTime = tv;
			}
			/*
			if( m_bRawLog )
			{
				LOG_HEX( m_szName, m_prgBucket[idx].pkt->Data()
					, m_prgBucket[idx].pkt->DataLen());
			}
			*/
//			LOG( "%s MsgQ.StartXmit   (Handle:x%04X Retry:%u Qsz:%d State:x%02X)"
//				, m_szName
//				, it->m_nHandle
//				, it->m_nRetries
//				, m_prgBucket.size()
//				, m_nState
//				);
			m_pAnchor->Write( it->pkt.get() );
			Block() ;
			// Drop the packet that was sent more than m_nMaxRetries times.
			if	( it->m_nRetries >= m_nMaxRetries )
			{
				if (m_nMaxRetries > 1)	// log only for queues that have more than one retry
				{
					LOG( "%s MsgQ.StartXmit DropRetryCount cR:%d, mR:%d", m_szName, it->m_nRetries, m_nMaxRetries);
				}
				remove(it);
			}
			return true ;
		}
	}
	return false ;
}
