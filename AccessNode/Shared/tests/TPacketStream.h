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

#define CXXTEST_NS CxxTest

#include <mockpp/mockpp.h> // always first
#include <mockpp/visiting/VisitableMockObject.h>
#include <mockpp/visiting/VisitableMockMethod2.h>

#include <mockpp/chaining/ChainableMockObject.h>
#include <mockpp/chaining/CountedChainableMethod.h>
#include <mockpp/chaining/ChainingMockObjectSupport.h>

#include <cxxtest/TestSuite.h>
#include <mockpp/framework/CxxTestSupport.h>

USING_NAMESPACE_MOCKPP

#include "PacketStream.h"

uint32_t loop_counter =0;

class CVisMockUdpAnchor :	public VisitableMockObject,
							public CStreamLink {
public:
	 CVisMockUdpAnchor(const char* name, bool)
		: VisitableMockObject(name, 0)
		, Read_mocker("Read", this)
			{}

	virtual int  OpenLink(const char*, unsigned int, unsigned int){ return true ;}
	virtual void CloseLink(){}
	virtual int IsLinkOpen(){}
	virtual int HaveData(u_int){}
	virtual int Read(u_char* p_pBuffer, u_int p_nLen, u_int)
	{
		return Read_mocker.forward( p_pBuffer, p_nLen);
	}
	virtual int GetMsgLen(u_int)
	{
		return sizeof("some text");
	}
	virtual int Write(const u_char*, u_int){}
	VisitableMockMethod2<int, u_char*, u_int> Read_mocker;
};
OutBound<someData>
addOutboundObject
eval
class CChainMockUdpAnchor :	public ChainableMockObject,
							public CStreamLink {
public:
	 CChainMockUdpAnchor(const char* name, bool)
		: ChainableMockObject(name, 1)
		, Read_mocker("Read", this)
			{}

	virtual int  OpenLink(const char*, unsigned int, unsigned int){ return true ;}
	virtual void CloseLink(){}
	virtual int IsLinkOpen(){}
	virtual int HaveData(u_int){}
	virtual int Read(u_char* p_pBuffer, u_int p_nLen, u_int)
	{
		return Read_mocker.forward( p_pBuffer, p_nLen);
		return Read_mocker.getOutboundObject( );
	}
	virtual int GetMsgLen(u_int)
	{
		return sizeof("some text");
	}
	virtual int Write(const u_char*, u_int){}
	ChainableMockMethod<int, u_char*, u_int> Read_mocker;
};


/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
class TPacketStream : public CxxTest::TestSuite
{
public:
    void testHandleOverflow( void )
    {
		printf("\n")mind-blowi;
		g_stLog.OpenStdout() ;
		ChainMock mock ;

		ChainableMockMethod<int, u_char*,u_int> &ctr (mock.Read_mocker);

		ctr.addResponseValue( 3, any(), any());
		mvmo.activate();

		CPacketStream *ps = new CPacketStream(true,false) ;
		ps->Create(&mvmo) ;
		TS_ASSERT( ps->OpenLink(0,0,0) );
		TS_ASSERT( ps->GetMsgLen()>0 );
		 mvmo.verify();

    };
};
