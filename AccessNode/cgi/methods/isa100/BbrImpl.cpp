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


#include <string.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>


#include "BbrImpl.h"
#include "../core/FileImpl.h"
#define ENG_MODE NIVIS_TMP "tr_cmd_eng_mode.txt"
#define ACKED ".acked"
#define PENDING ".pending"
#define RESPONDED ".responded"
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int BbrImpl::setEngMode( int mode, int channel, int pwrlvl )
{
	CFileImpl resp( ENG_MODE RESPONDED );
	//if ( resp.creationTime() + 30/*sec*/ < time(NULL) ) return false ;

	FILE* df = fopen(  ENG_MODE , "w" );
	if ( NULL == df ) return false ;

	int rv = fprintf( df, "cmd=set\nengMode=%i\nchannel=%i\npower_level=%X", mode, channel, pwrlvl  );
	if ( rv < 0 ) return false ;
	fclose(df);

	systemf_to( 40, "killall -USR1 backbone");

	// wait for response
	time_t dueTime=time(NULL)+30 ;
	while ( time(NULL) < dueTime )
	{
		if ( resp.exists() )
		{
			return true ;
		}
		sleep(1);
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int BbrImpl::getEngMode( )
{
	//newer file
	CFileImpl resp( ENG_MODE RESPONDED );
	//if ( resp.creationTime() + 30/*sec*/ < time(NULL) ) return false ;

	resp.remove() ;
	FILE* df = fopen(  ENG_MODE , "w" );
	if ( NULL == df ) return false ;
	int rv = fprintf( df, "cmd=get" );
	if ( rv < 0 ) return false ;
	fclose(df);

	systemf_to( 40, "killall -USR1 backbone");

	// wait for response
	time_t dueTime=time(NULL)+30 ;
	while ( time(NULL) < dueTime )
	{
		if ( resp.exists() )
		{
			df = fopen( ENG_MODE RESPONDED , "r" );
			if ( ! df ) return -1 ;
			char*dline=NULL;
			size_t n;
			while ( -1 != (rv=getline(&dline, &n, df)) )
			{
				int engMode =0;
				rv = sscanf( dline, "engMode=%i", &engMode );
				if ( rv == 1 ) {
					free( dline ) ;
					return engMode ;
				}
			}

			free( dline ) ;
			return -1 ;
		}
		sleep(1);
	}
	return -1 ;
}
