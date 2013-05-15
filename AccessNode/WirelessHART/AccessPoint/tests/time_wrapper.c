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

// time_wrapper.c
#include <stdio.h>
#include <time.h>
time_t __real_time (time_t*t);

time_t __wrap_time (time_t*t)
{
	time_t r ;
	static long current=0;
	FILE * in = fopen("in.txt", "r");
	if ( ! in )
		return __real_time(t) ;

	fseek(in, current, SEEK_SET);
	fscanf(in,"%d", &r );
	current = ftell(in) ;
	fclose(in);
	printf ("time() called. returning %u, %p\n", r, __builtin_return_address (0));

	return r;
}
