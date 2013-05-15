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
// C++ Interface: MicroSec
//
// Description:
//
//
// Author:  Marcel Ionescu
//
//
//
#ifndef MICROSEC_H
#define MICROSEC_H

/**
	@author  <marcel@mariusn>
*/

#include <sys/time.h>	//struct timeval

/// @addtogroup libshared
/// @{
class CMicroSec{
public:
    CMicroSec();
    ~CMicroSec();

	/// Initialise the internal timeval structure. Use it only if default initialising to now() is not desirable.
	void Init( const struct timeval& p_tmInit );
	double	GetElapsedSec( void ) const;
	void	MarkStartTime( void );
	//TODO: add method GetElapsedTimeStr detailing hours, minutes, seconds, miliseconds
	//TODO: add methods: GetElapsedUSec returning number of microseconds, int (signed!) for speed
	//TODO: add methods: GetElapsedMSec returning number of miliseconds, int (signed!) for speed

	char* GetElapsedTimeStr() const;

	/// Optimise for speed: use int instead of double. Has reduced range of +- 2147 seconds (35 min)
	int GetElapsedUSec( void ) const;
	/// Optimise for speed: use int instead of double. Has reduced range of +- 2147483 seconds (35791 min = 596 hours = 24 days)
	int GetElapsedMSec( void ) const;

	/// Return a pointer to start time (timeval*)
	operator timeval* ( void ){ return &m_tmStart; }

	private:
		//TODO: inherit from struct timeval, do not include it as private member
		struct timeval m_tmStart;
};

#define MICROSEC_IN_SEC	1000000
#define MILISEC_IN_SEC	1000

//Compute the elapsed time. between two struct timeval
//TAKE CARE: the call order is important: parameter1 is old time, parameter2 is new time
#if !defined(elapsed_usec)
#define elapsed_usec(_old_,_new_) (MICROSEC_IN_SEC*((_new_).tv_sec - (_old_).tv_sec) + ((_new_).tv_usec - (_old_).tv_usec))
#endif

//compute the distance between two uSecs, regardless of second boundaries
#if !defined(distance_us)
#define distance_us(_a_, _b_) ( abs((_a_)-(_b_)) <? abs( MICROSEC_IN_SEC + (_b_)-(_a_)))
#endif
/// @}
#endif
