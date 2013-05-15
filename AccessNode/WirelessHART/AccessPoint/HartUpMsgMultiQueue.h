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

#ifndef _HART_UP_MSG_MULTI_QUEUE_H_
#define _HART_UP_MSG_MULTI_QUEUE_H_

#include "MsgMultiQueue.h"
#include "HartUpMsgQueue.h"

///////////////////////////////////////////////////////////
// Multi queue for the UDP connection.
// Contains a queue for each defined priority.
//////////////////////////////////////////////////////////

class CHartUpMsgMultiQueue : public CMsgMultiQueue {
public :
	CHartUpMsgMultiQueue( unsigned int p_nPriorities=1, unsigned int p_nDropExpiredTimeout=3, unsigned int p_nResetStateTimeout=6 )
		: CMsgMultiQueue( p_nPriorities, p_nDropExpiredTimeout, p_nResetStateTimeout)
	{
		for(std::vector<CMsgQueue*>::iterator it = m_prgMultiQ.begin(); it != m_prgMultiQ.end(); it++)
		{
			CMsgQueue* q = *it;
			delete q;
		}
		m_prgMultiQ.clear();
		for (unsigned i=0; i<m_nPriorities; ++i)
		{
			m_prgMultiQ.push_back(new CHartUpMsgQueue());
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
		{
			m_prgMultiQ.push_back(new CHartUpMsgQueue());
		}
	}
	virtual unsigned Enque( unsigned int p_nPriority, ProtocolPacketPtr& p_pPkt, unsigned int p_nHandle=0)
	{
//		LOG("CHartUpMsgMultiQueue.Enqueue");
		if ( p_nPriority >= m_nPriorities )
		{
			ERR( "CHartUpMsgMultiQueue.Enque: Invalid Priority[%u]", p_nPriority);
			return 0 ;
		}
		LOG("MsgMultiQ.Enque (Priority:%u, handle:x%04X,)", p_nPriority, p_nHandle);
		return m_prgMultiQ[p_nPriority]->Enque(p_pPkt, ((p_nPriority << 13) & 0xF000) | p_nHandle);
	}
	virtual ~CHartUpMsgMultiQueue() {}

};

#endif //_HART_UP_MSG_MULTI_QUEUE_H_
