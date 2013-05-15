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

//
// C++ Implementation: MicroSec
//
// Description: 
//
//
// Author:  Marcel Ionescu
//
//
//

#include <stdio.h>
#include <time.h>


#include "MicroSec.h"

CMicroSec::CMicroSec()
{
	MarkStartTime();
}

void CMicroSec::Init( const struct timeval & p_tmInit )
{
	m_tmStart.tv_sec	= p_tmInit.tv_sec;
	m_tmStart.tv_usec	= p_tmInit.tv_usec;
}

CMicroSec::~CMicroSec()
{
}

// Return the time elapsed since object creation/last MarkStartTime, 
//	in seconds (double, allowing fractions of a second )
double	CMicroSec::GetElapsedSec( void ) const
{
	struct timeval tmStop ;
	gettimeofday(&tmStop, (struct timezone*)0 /*NULL*/);
	return 	(tmStop.tv_sec - m_tmStart.tv_sec) +
			((double)(tmStop.tv_usec - m_tmStart.tv_usec))/(double)MICROSEC_IN_SEC;
}

char* CMicroSec::GetElapsedTimeStr() const
{
	static char szTime[50];
	struct tm * timeinfo = gmtime(&m_tmStart.tv_sec);
	
	sprintf(szTime, "%04d-%02d-%02d %02d:%02d:%02d", timeinfo->tm_year+1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
		timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

	return szTime;
}

////////////////////////////////////////////////////////////////////////////////
/// @author Marcel Ionescu
/// @brief Optimise for speed: use int instead of double. Has reduced range of +- 2147 seconds (35 min)
////////////////////////////////////////////////////////////////////////////////
int CMicroSec::GetElapsedUSec( void ) const
{
	struct timeval tmStop ;
	gettimeofday(&tmStop, (struct timezone*)0 /*NULL*/);
	return 	(tmStop.tv_sec - m_tmStart.tv_sec)*MICROSEC_IN_SEC +
			(tmStop.tv_usec - m_tmStart.tv_usec);
}

////////////////////////////////////////////////////////////////////////////////
/// @author Marcel Ionescu
/// @brief Optimise for speed: use int instead of double. Has reduced range of +- 2147483 seconds (35791 min = 596 hours = 24 days)
////////////////////////////////////////////////////////////////////////////////
int CMicroSec::GetElapsedMSec( void ) const
{
	struct timeval tmStop ;
	gettimeofday(&tmStop, (struct timezone*)0 /*NULL*/);
	return 	(tmStop.tv_sec - m_tmStart.tv_sec)*MILISEC_IN_SEC +
			(tmStop.tv_usec - m_tmStart.tv_usec) / (MICROSEC_IN_SEC/MILISEC_IN_SEC);
}

void	CMicroSec::MarkStartTime( void )
{
	gettimeofday(&m_tmStart, (struct timezone*)0 /*NULL*/);
}

