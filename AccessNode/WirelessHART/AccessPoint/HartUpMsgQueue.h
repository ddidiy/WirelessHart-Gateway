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

#ifndef _HART_UP_MSG_QUEUE_H_
#define _HART_UP_MSG_QUEUE_H_

#include "MsgQueue.h"

/////////////////////////////////////////////////////
// Queue used by the UDP multi queue.
/////////////////////////////////////////////////////

class CHartUpMsgQueue : public CMsgQueue {
public:
	CHartUpMsgQueue( unsigned int p_nCapacity=20, bool p_bLogRaw=true
			, unsigned int p_nRetryTimeout=4, unsigned int p_nAckTimeout=5
			, uint8_t p_nMaxRetries=3
		)
		: CMsgQueue(p_nCapacity, p_bLogRaw, p_nRetryTimeout, p_nAckTimeout, p_nMaxRetries)
	{
	}
	virtual ~CHartUpMsgQueue() {}
public:
	virtual unsigned Enque( ProtocolPacketPtr& p_pElm, unsigned p_nHandle=0 ) ;

	//	uncomment if you want to make queue non-blocking (and also make Block virtual in base class)
//	virtual void Block()
//	{
//		//Do not block UP queue, since we might have very high traffic from wireless, and UDP has very large timeout compared to serial link.
//	}
} ;


#endif //_HART_UP_MSG_QUEUE_H_

