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
#include "../UtilsSolo.h"

#define MSG "mmm"

int
main (int argc, char ** argv)
{
	CUdpSocket s;
	g_stLog.OpenStdout() ;
	s.Create (SOCK_DGRAM, AF_INET6);

	size_t N = 1000;
	char msg[N];
	memset (msg,0,N);

	if (argc > 3)
		memcpy (msg, argv[3], strlen (argv[3]));
	else
		memcpy (msg, MSG, sizeof (MSG));

	int index = InterfaceIndex ("eth0");
	char buf[100];
	s.SendTo6 (argv[1],			// address
			   atoi (argv[2]),	// port
			   msg,				// message
			   strlen (msg),
			   0,
			   index);
}

