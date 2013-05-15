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

#include "HartUpMsgQueue.h"

unsigned CHartUpMsgQueue::Enque( ProtocolPacketPtr& p_pElm , unsigned p_nHandle)
{
//	LOG( "%s MsgQ.Enque [Try enque]  (Handle:x%04X Capacity:%u Qsz:%u)", m_szName
//		, p_nHandle
//		, m_nCapacity
//		, m_prgBucket.size()
//		);
	// Test Queue Overflow
	if ( m_prgBucket.size() >= m_nCapacity )
	{
//		m_bHasGaps = true ;
//		Shrink() ;
//		if ( m_nInsertPos >= m_nCapacity )
//		{
			LOG( "%s MsgQ.Enque [Message discarded:queue full] (Handle:x%04X Capacity:%u Qsz:%u)", m_szName
			, p_nHandle
			, m_nCapacity
			, m_prgBucket.size()
			);
			return 0 ;
//		}
	}
	// already inserted
	for (NodeInfoPtrList::iterator it = m_prgBucket.begin(); it != m_prgBucket.end(); ++it)
//	for ( unsigned i=0; i<m_nInsertPos; ++i )
	{
		if ( it->m_nHandle == p_nHandle )
		{
			it->m_nRetries ++ ;
			LOG( "%s MsgQ.Enque [Handle already in queue] (Handle:x%04X Capacity:%u Qsz:%u)", m_szName
				, p_nHandle
				, m_nCapacity
				, m_prgBucket.size()
				) ;
			return p_nHandle ;
		}
	}
	// first insertion
	m_prgBucket.push_back(NodeInfo());
	NodeInfo& node = m_prgBucket.back();

	node.pkt = p_pElm ;
	node.m_nPktLifeTime = 0 ;
	node.m_nNextSendTimestamp = 0 ;
	node.m_nRetries = 0 ;
	node.m_nHandle = p_nHandle ;
	node.toBeErased = false;

	LOG( "%s MsqQ.Enque [Enque done] (Handle:x%04X Capacity:%u Qsz:%u)", m_szName
		, node.m_nHandle
		, m_prgBucket.capacity()
		, m_prgBucket.size()
		) ;
	return node.m_nHandle ;
}


