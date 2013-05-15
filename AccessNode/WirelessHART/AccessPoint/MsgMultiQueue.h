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

#ifndef _MSG_MULTI_QUEUE_H_
#define _MSG_MULTI_QUEUE_H_

// $Id: MsgMultiQueue.h,v 1.20.24.1 2013/05/15 19:19:17 owlman Exp $

//////////////////////////////////////////////////////////////////////////////
/// @author	Marius Negreanu
/// @date	Thu Dec 18 13:40:22 2008 UTC
/// @brief  Implement a multi-priority queue (multiqueue).
//////////////////////////////////////////////////////////////////////////////

#include "Shared/SimpleTimer.h"

#include "MsgQueue.h"

#include <vector>
//////////////////////////////////////////////////////////////////////////////
/// @class CMsgMultiQueue
/// @ingroup AccessPoint
//////////////////////////////////////////////////////////////////////////////
class CMsgMultiQueue {
public:
	CMsgMultiQueue( unsigned int p_nPriorities=1
			, unsigned int p_nDropExpiredTimeout=3
			, unsigned int p_nResetStateTimeout=6
		)
		: m_nPriorities(p_nPriorities)
		, m_nDropExpiredTimeout(p_nDropExpiredTimeout*1000)
		, m_nResetStateTimeout(p_nResetStateTimeout*1000)
	{
		//m_prgMultiQ = new CMsgQueue* [m_nPriorities] ;
		m_prgMultiQ.reserve(m_nPriorities);
		for (unsigned i=0; i<p_nPriorities; ++i)
		{
			m_prgMultiQ.push_back(new CMsgQueue());
		}

		LOG( "MsgMultiQ.ctor (dropExpiredTimeout:%u resetStateTimeout:%u priorities:%u)", m_nDropExpiredTimeout, m_nResetStateTimeout, m_nPriorities);

		m_oDropExiredTimer.SetTimer(m_nDropExpiredTimeout);
		m_oResetStateTimer.SetTimer(m_nResetStateTimeout);
	}
//////////////////////////////////////////////////////////////////////////////
/// @brief Destructor
//////////////////////////////////////////////////////////////////////////////
	virtual ~CMsgMultiQueue()
	{
//		for (unsigned i=0; i<m_nPriorities; ++i)
//			delete m_prgMultiQ[i] ;
		//delete[] m_prgMultiQ ;
	}
//////////////////////////////////////////////////////////////////////////////
/// @brief Set the Queue Name.
/// @param[in] p_szName The queue name.
/// @details The name of the queue is used in logs.
//////////////////////////////////////////////////////////////////////////////
	void Name( const char * p_nName )
	{
		Name(m_nPriorities, p_nName);
	}
	void Name( unsigned int p_nPriority, const char * p_nName )
	{
		if ( p_nPriority > m_nPriorities ) return ;
		if ( p_nPriority < m_nPriorities )
		{
			m_prgMultiQ[p_nPriority]->Name(p_nName) ;
			return ;
		}

		for ( uint8_t i=0; i<m_nPriorities; ++i)
			m_prgMultiQ[i]->Name(p_nName);
	}
//////////////////////////////////////////////////////////////////////////////
/// @brief Set the Queue maximum capacity.
/// @param[in] p_nCapacity The Maximum number of packets the queue will hold.
//. @note Calling this method will destroy its old content.
//////////////////////////////////////////////////////////////////////////////
	bool Capacity(unsigned int p_nPriority, unsigned int p_nCapacity )
	{
		if ( p_nPriority >= m_nPriorities )
		{
			ERR("CMsgMultiQueue.Capacity: Invalid Priority[%u]", p_nPriority);
			return false ;
		}
		m_prgMultiQ[p_nPriority]->Capacity(p_nCapacity);
		return true ;
	}
//////////////////////////////////////////////////////////////////////////////
/// @brief Set the maximum attempts to send a packet.
/// @param[in] p_nMaxRetries Maximum attempts to send a packet.
//////////////////////////////////////////////////////////////////////////////
	void MaxRetries( unsigned int p_nPriority, unsigned int p_nMaxRetries )
	{
		if ( p_nPriority > m_nPriorities ) return ;
		if ( p_nPriority < m_nPriorities )
		{
			m_prgMultiQ[p_nPriority]->MaxRetries(p_nMaxRetries ) ;
		}
		else
		{
    		    for ( unsigned i=0;i<m_nPriorities;++i)
			m_prgMultiQ[i]->MaxRetries(p_nMaxRetries);
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	/// @brief Set the queue to be blockable or not.
	/// @param[in] m_bIsBlockable true or false.
	//////////////////////////////////////////////////////////////////////////////
		void IsBlockable(unsigned int p_nPriority, bool blockable)
		{
			if ( p_nPriority > m_nPriorities ) return ;
			if ( p_nPriority < m_nPriorities )
			{
				m_prgMultiQ[p_nPriority]->IsBlockable(blockable ) ;
			}
			else
			{
	    		    for ( unsigned i=0;i<m_nPriorities;++i)
				m_prgMultiQ[i]->IsBlockable(blockable);
			}
		}

//////////////////////////////////////////////////////////////////////////////
/// @brief Block the message Queue.
/// @details A blocked queue will not send packets.
/// @see Unblock and Clear.
//////////////////////////////////////////////////////////////////////////////
	void Block()
	{
		for ( uint8_t i=0; i< m_nPriorities; ++i )
		{
			m_prgMultiQ[i]->Block() ;
		}
		m_oResetStateTimer.SetTimer(m_nResetStateTimeout*1000);
	}
//////////////////////////////////////////////////////////////////////////////
/// @brief Unblock the message queue.
//////////////////////////////////////////////////////////////////////////////
	void Unblock()
	{
		for ( uint8_t i=0; i< m_nPriorities; ++i )
		{
			m_prgMultiQ[i]->Unblock() ;
		}
	}
//////////////////////////////////////////////////////////////////////////////
/// @brief Remove all packets from the Message Queue and reset its state.
//////////////////////////////////////////////////////////////////////////////
	void Clear( unsigned int p_nPriority )
	{
		if ( p_nPriority > m_nPriorities ) return ;
		if ( p_nPriority < m_nPriorities )
		{
			m_prgMultiQ[p_nPriority]->Clear() ;
			return ;
		}

		for ( uint8_t i=0;i<m_nPriorities;++i)
			m_prgMultiQ[i]->Clear() ;
	}
	void AnchorLink( CPacketStream* p_pAnchor )
	{
		for ( uint8_t i=0; i< m_nPriorities; ++i )
		{
			m_prgMultiQ[i]->AnchorLink( p_pAnchor ) ;
		}
	}
	virtual void MaxPriorities( unsigned int p_nPriorities )
	{
		for(std::vector<CMsgQueue*>::iterator it = m_prgMultiQ.begin(); it != m_prgMultiQ.end(); it++)
		{
			CMsgQueue* q = *it;
			delete q;
		}
		m_prgMultiQ.clear();

		m_nPriorities = p_nPriorities ;
		m_prgMultiQ.reserve(m_nPriorities);
		for (unsigned i=0; i<m_nPriorities; ++i)
			m_prgMultiQ.push_back(new CMsgQueue()) ;
	}
	virtual unsigned Enque( unsigned int p_nPriority , ProtocolPacketPtr& p_pPkt , unsigned int p_nHandle=0)
	{
		if ( p_nPriority >= m_nPriorities )
		{
			ERR("MsgMultiQ.Capacity: Invalid Priority[%u]", p_nPriority);
			return false ;
		}
		return m_prgMultiQ[p_nPriority]->Enque(p_pPkt, p_nPriority << 13);
	}
	void Refresh()
	{
		if ( m_oDropExiredTimer.IsSignaling() )
		{
//			LOG( "MsgMultiQ.Refresh [dropExpiredTimer] (dropExpiredTimeout:%u)", m_nDropExpiredTimeout) ;
			DropExpired();
			Shrink() ;
			m_oDropExiredTimer.SetTimer( m_nDropExpiredTimeout ) ;
		}

		if ( m_oResetStateTimer.IsSignaling() )
		{
//			LOG( "MsgMultiQ.Refresh [resetStateTimer]  (resetStateTimeout:%u)", m_nResetStateTimeout) ;
			Unblock() ;
			ArmResetStateTimer() ;
		}

	}
	void ArmResetStateTimer()
	{
			m_oResetStateTimer.SetTimer(m_nResetStateTimeout);
	}
	void StartXmit()
	{
		for ( uint8_t i=0; i< m_nPriorities; ++i )
		{
			m_prgMultiQ[i]->StartXmit() ;
		}
	}
	bool Confirm( unsigned p_nHandle, unsigned p_nStatus )
	{
		uint8_t prio = p_nHandle >> 13;

		if ( prio >= m_nPriorities )
		{
			ERR("CMsgMultiQueue.Confirm: Invalid Message Priority[%u] Handle[x%04X]"
				, prio
				, p_nHandle );
			return false ;
		}

		return m_prgMultiQ[prio]->Confirm( p_nHandle, p_nStatus );
	}
	void DropExpired( )
	{
		for ( unsigned int i=0; i< m_nPriorities; ++i )
		{
			m_prgMultiQ[i]->DropExpired() ;
		}
	}
	void Shrink()
	{
		for ( unsigned int i=0; i< m_nPriorities; ++i )
		{
			m_prgMultiQ[i]->Shrink() ;
		}
	}

protected:
	CSimpleTimer m_oDropExiredTimer ;
	CSimpleTimer m_oResetStateTimer ;
	std::vector<CMsgQueue*>   m_prgMultiQ;
	unsigned int m_nPriorities ;
	unsigned int m_nDropExpiredTimeout ;
	unsigned int m_nResetStateTimeout ;
} ;
#endif
