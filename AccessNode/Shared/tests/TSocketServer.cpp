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

#include "../UdpSocket.h"
#include "../log.h"
#include "../Common.h"

#define MSG "mmm"

int
main (int argc, char ** argv)
{
	CUdpSocket s;
	g_stLog.OpenStdout() ;
	s.Create (SOCK_DGRAM, AF_INET6);

	if (setsockopt (s, SOL_SOCKET, SO_BINDTODEVICE, "eth0", strlen ("eth0")))
	{
		LOG_ERR("setsockopt");
		return false;	
	}

	s.Bind (atoi (argv[2]), argv[1], "eth0"); //"2001:db8:0:85a3::ac1f:8001"
	size_t N = 1000;
	char msg[N];

	//s.SendTo6 ("2001:db8:0:85a3::ac1f:8001", 2345, MSG, sizeof (MSG));
	while (strncmp ("quit", msg, 4))
	{
		memset (msg,0,1000);
		s.RecvFrom6(msg, &N);
		NLOG_INFO ("MSG:  %d %s", N, msg);
		N=1000;
	}
}

