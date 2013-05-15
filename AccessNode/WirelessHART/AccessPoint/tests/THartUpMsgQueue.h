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
#include "HartUpMsgQueue.h"

class THartUpMsgQueue : public CxxTest::TestSuite
{
public:
    void testHandleOverflow( void )
    {
		printf("\n");
		g_stLog.OpenStdout() ;
		CHartUpMsgQueue q(1) ;
		q.Name("HartUpMsgQueue");
		unsigned h ;
		for ( unsigned i=0; i<QHANDLE_MAX+4; ++i )
		{
			h = q.Enque( new ProtocolPacket ); FLOG("handle:%u", h );
			TS_ASSERT(q.Confirm(h,0)) ;
		}
    };
};
