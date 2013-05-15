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

/*****************************************************************************
 * Author:      Marius Negreanu
 * Date:        Wed Jan 14 15:40:03 EET 2009
 ****************************************************************************/

#include <cxxtest/TestSuite.h>
#include "MsgQueue.h"

uint32_t loop_counter =0;


#define HOOK_STD_TIME(filename,elements, time_vec) {FILE*f=fopen(filename,"w"); for(unsigned i=0; i<elements;++i){fprintf(f,"%d ",time_vec[i]);}fclose(f); }


class CPacketStreamStub : public CPacketStream {
public:
	CPacketStreamStub() {};
	~CPacketStreamStub(){};
public:
	int Write( ProtocolPacket* pkt )
	{
		return 0;
	}
} ;


class TMsgQueue : public CxxTest::TestSuite
{
public:
	void testHandleOverflow( void )
	{
		printf("\n");
		g_stLog.OpenStdout() ;
		CMsgQueue q(10) ;
		q.Name("MsgQueue");
		unsigned h ;
		for ( unsigned i=0; i<QHANDLE_MAX+3; ++i )
		{
			h = q.Enque( new ProtocolPacket ); FLOG("handle:%u", h );
			TS_ASSERT( q.Confirm(h,0) ) ;
		}
	};
	void test1PktQueue( void )
	{
		printf("\n");
		g_stLog.OpenStdout() ;
		CMsgQueue q(1) ;
		q.Name("TEST_QUEUE");
		unsigned h ;
		for ( unsigned i=0; i<QHANDLE_MAX+3; ++i )
		{
			h = q.Enque( new ProtocolPacket ); FLOG("handle:%u", h );
			TS_ASSERT( q.Confirm(h,0) ) ;
		}
	};
	void testPktExpire( void )
	{
		printf("\n");
		time_t a[512] ={1,
			1, 1,2,3,3,4,5,6,6,7,8,9,10,
			11,12,13,14,15,16,17,18,19,
			20,21,22,23,24,25,26,27,28,29,
			30,31,32,33,34,35,36,37,38,39
		};
		g_stLog.OpenStdout() ;

		CPacketStreamStub ps ;
		for ( int i=sizeof(a)/sizeof(a[0]); i< 512; ++i ) a[i]=i ;

		HOOK_STD_TIME("in.txt", (sizeof(a)/sizeof(a[0])), a) ;

		CMsgQueue q(10,false) ;
		q.Name("TEST_QUEUE");
		q.AnchorLink(&ps);

		unsigned h ;
		h = q.Enque( new ProtocolPacket );
		for ( unsigned i=0; i<32; ++i )
		{
			q.StartXmit() ;
			q.DropExpired() ;
			q.Shrink() ;
			//TS_ASSERT( q.Confirm(h,0) ) ;
		}
		unlink("in.txt");
	}
};
