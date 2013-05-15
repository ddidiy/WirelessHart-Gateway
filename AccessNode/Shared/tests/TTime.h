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
#include "Shared/Common.h"
#include "Time.h"

class TTime : public CxxTest::TestSuite
{
public:
    void testSubstractWithCarry1( void )
    {
		printf("\n%s\n",__FUNCTION__);
		g_stLog.OpenStdout() ;
		CTime t1(1,0);
		CTime t2(0,1);
		t1-=t2 ;
		TS_ASSERT_EQUALS( t1.Seconds(),0);
		TS_ASSERT_EQUALS( t1.Microseconds(),USEC_IN_SEC-1);
    };
    void testSubstractWithCarry2( void )
    {
		printf("\n%s\n",__FUNCTION__);
		g_stLog.OpenStdout() ;
		CTime t1(0,2);
		CTime t2(0,1);
		t1-=t2 ;
		TS_ASSERT_EQUALS( t1.Seconds(),0);
		TS_ASSERT_EQUALS( t1.Microseconds(),1);
    };
    void testSubstractWithCarry3( void )
    {
		printf("\n%s\n",__FUNCTION__);
		g_stLog.OpenStdout() ;
		CTime t1(0,0);
		CTime t2(0,1);
		t1-=t2 ;
		TS_ASSERT_EQUALS( t1.Seconds(),-1);
		TS_ASSERT_EQUALS( t1.Microseconds(),USEC_IN_SEC-1);
    };
    void testSubstractNormal( void )
    {
		printf("\n%s\n",__FUNCTION__);
		g_stLog.OpenStdout() ;
		CTime t1(1,2);
		CTime t2(0,1);
		t1-=t2 ;
		TS_ASSERT_EQUALS( t1.Seconds(),1);
		TS_ASSERT_EQUALS( t1.Microseconds(),1);
    };
};
