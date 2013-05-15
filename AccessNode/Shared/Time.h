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

#ifndef _N_TIME_H_
#define _N_TIME_H_

#include <sys/time.h>
#include <time.h>

/// @addtogroup libshared
/// @{
#ifndef USEC_IN_SEC
#define USEC_IN_SEC 1000000L
#endif
class CTime : public timeval {
public:
	CTime()
	{
		tv_sec=0 ;
		tv_usec=0;
	}
	CTime(time_t sec, suseconds_t microsec )
	{
		tv_sec=sec;
		tv_usec=microsec;
	}
public:
	static CTime Now() ;
	time_t Seconds() const { return tv_sec ; }
	unsigned Microseconds() const { return tv_usec ; }
	CTime& operator -=( const CTime& t1)
	{
		tv_usec -= t1.tv_usec;
		if ( tv_usec < 0 )
		{
			--tv_sec ;
			tv_usec += USEC_IN_SEC ;// 1.000.000 microseconds in a second
		}
		tv_sec  -= t1.tv_sec;
		return *this;
	}
	CTime operator -( const CTime& t1) const
	{
		CTime temp ;
		temp.tv_usec = tv_usec - t1.tv_usec;
		temp.tv_sec = tv_sec ;
		if ( temp.tv_usec < 0 )
		{
			--temp.tv_sec ;
			temp.tv_usec += USEC_IN_SEC ;// 1.000.000 microseconds in a second
		}
		temp.tv_sec  -= t1.tv_sec;
		return temp ;
	}
	bool operator <(CTime& t1 )
	{
		if ( (tv_sec*USEC_IN_SEC+tv_usec) < (t1.tv_sec*USEC_IN_SEC+t1.tv_usec) )
			return true ;
		return false ;
	}
	char const* LocaltimeString()
	{
		if ( ! ctime_r(&tv_sec, m_szAscBuf) ) return NULL ;
		return m_szAscBuf ;
	}
protected:
	char m_szAscBuf[32];
};
/// @}
#endif //_N_TIME_H_
