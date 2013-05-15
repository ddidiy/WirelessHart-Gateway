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


#ifndef _TIME_WRAPPER_H_
#define _TIME_WRAPPER_H_

#include <sys/time.h>

struct CTimeWrapper : public timeval
{
    CTimeWrapper();
    CTimeWrapper( const struct timeval &tv );
    CTimeWrapper( const time_t p_lSec, const suseconds_t p_lUSec );
    CTimeWrapper( const int p_nTimeValueUSec );

    CTimeWrapper & operator =( const int p_nTimeValueUSec );

    //CTimeWrapper & operator +=( const int p_nTimeValueUSec );
    //CTimeWrapper & operator -=( const int p_nTimeValueUSec );

    CTimeWrapper & operator +=( const struct timeval &p_roTv );

    friend CTimeWrapper operator -( const struct timeval &p_roTv, const CTimeWrapper &p_roTw );
    friend CTimeWrapper operator +( const struct timeval &p_roTv, const CTimeWrapper &p_roTw );

    bool operator <( const struct timeval &tv );
    bool operator >( const struct timeval &tv );
    bool operator ==( const struct timeval &tv ) { return tv_sec == tv.tv_sec && tv_usec == tv.tv_usec; }

    bool Negative() const { return tv_usec < 0 || ( tv_usec == 0 && tv_sec < 0 ); }     // < 0
    bool SeqZero() const { return tv_usec <= 0 && tv_sec <= 0; }                        // <= 0

    int UsTime() const { return tv_sec * 1000000 + tv_usec; }
    int MsTime() const { return tv_sec * 1000 + tv_usec / 1000; }
    int SecTime() const { return tv_sec; }

    // For testing purposes only. It's not thread-safe.
    void PrintDelta( char *p_pcOutput );
    //void PrintAbsolute( char *p_pcOutput );
    //void PrintComponents( char *p_pcOutput );

    static CTimeWrapper Now();
};

#endif // _TIME_WRAPPER_H_