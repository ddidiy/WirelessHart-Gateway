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

#include "AlarmEx.h"
#include <time.h>
#include <unistd.h>
#include <stdio.h> // sprintf

#define MAX_SIGNAL 32

struct AlarmStatus
{
	int		pid;
	time_t	sched;
};



void AlarmEx( int sig, int sec )
{
	static AlarmStatus pSignals[MAX_SIGNAL];

	if ( sig < 1 || sig >= MAX_SIGNAL )
	{	return;
	}

	time_t nTime = time(NULL);

	if ( pSignals[sig].pid && nTime < pSignals[sig].sched )
	{	kill( pSignals[sig].pid, SIGKILL );
	}

	if ( sec == 0 )
	{	pSignals[sig].pid = 0;
		pSignals[sig].sched = 0;
		return;
	}

#ifndef USE_VFORK
	if (( pSignals[sig].pid = fork()) == 0) // child
	{
		for( int i=0; i<256; i++)
		{	// close all fd's still open to solve problems open sockets
			close( i );
		}
		sleep(sec);
		kill( getppid(), sig );
		_exit(0);
#else
	if (( pSignals[sig].pid = vfork()) == 0) // child
	{
		char slpKill[32];
		sprintf( slpKill, "sleep %u ; kill -%u %u", sec, sig, getppid() );
		char * const argv[] = {"/bin/sh","-c", slpKill, NULL };
		execv("/bin/sh", argv);
#endif
	}
	pSignals[sig].sched  = time(NULL) + sec;
}
