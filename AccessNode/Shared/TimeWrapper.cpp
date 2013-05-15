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


#include "TimeWrapper.h"
#include "MicroSec.h"

#include <stdio.h>
#include <time.h>


CTimeWrapper :: CTimeWrapper()
{
    this->tv_sec = 0;
    this->tv_usec = 0;
}

CTimeWrapper :: CTimeWrapper( const struct timeval &tv )
{
    this->tv_sec = tv.tv_sec;
    this->tv_usec = tv.tv_usec;
}

CTimeWrapper :: CTimeWrapper( const time_t p_lSec, const suseconds_t p_lUSec )
{
    this->tv_sec = p_lSec;
    this->tv_usec = p_lUSec;
}

CTimeWrapper :: CTimeWrapper( const int p_nTimeValueUSec )
{
    tv_usec = p_nTimeValueUSec % MICROSEC_IN_SEC;
    tv_sec = p_nTimeValueUSec / MICROSEC_IN_SEC;
}


CTimeWrapper & CTimeWrapper :: operator =( const int p_nTimeValueUSec )
{
    tv_usec = p_nTimeValueUSec % MICROSEC_IN_SEC;
    tv_sec = p_nTimeValueUSec / MICROSEC_IN_SEC;

    return *this;
}

//CTimeWrapper & CTimeWrapper :: operator +=( const int p_nTimeStepUSec )
//{
//    int usec = p_nTimeStepUSec % MICROSEC_IN_SEC;
//    int sec = p_nTimeStepUSec / MICROSEC_IN_SEC;
//
//    tv_usec += usec;
//    tv_sec += sec;
//
//    if ( ( tv_sec < 0 && tv_usec >= 0 ) || ( tv_sec > 0 && tv_usec >= MICROSEC_IN_SEC ) )
//    {
//        tv_sec++;
//        tv_usec -= MICROSEC_IN_SEC;
//    }
//    else if ( ( tv_sec > 0 && tv_usec < 0 ) || ( tv_sec < 0 && tv_usec <= -MICROSEC_IN_SEC ) )
//    {
//        tv_sec--;
//        tv_usec += MICROSEC_IN_SEC;
//    }
//    
//    return *this;
//}

//CTimeWrapper & CTimeWrapper :: operator -=( const int p_nTimeStepUSec )
//{
//    int usec = p_nTimeStepUSec % MICROSEC_IN_SEC;
//    int sec = p_nTimeStepUSec / MICROSEC_IN_SEC;
//
//    tv_usec -= usec;
//    tv_sec -= sec;
//
//    if ( p_nTimeStepUSec >= 0 && tv_sec > 0 && tv_usec < 0 )
//    {
//        tv_sec--;
//        tv_usec += MICROSEC_IN_SEC;
//    }
//    else if ( p_nTimeStepUSec < 0 && tv_sec < 0 && tv_usec >= 0 )
//    {
//        tv_sec++;
//        tv_usec -= MICROSEC_IN_SEC;
//    }
//
//    return *this;
//}

CTimeWrapper & CTimeWrapper :: operator +=( const struct timeval &p_roTv )
{
    tv_usec += p_roTv.tv_usec;
    tv_sec += p_roTv.tv_sec;

    if ( ( tv_sec < 0 && tv_usec >= 0 ) || ( tv_sec > 0 && tv_usec >= MICROSEC_IN_SEC ) )
    {
        tv_sec++;
        tv_usec -= MICROSEC_IN_SEC;
    }
    else if ( ( tv_sec > 0 && tv_usec < 0 ) || ( tv_sec < 0 && tv_usec <= -MICROSEC_IN_SEC ) )
    {
        tv_sec--;
        tv_usec += MICROSEC_IN_SEC;
    }
    
    return *this;
}

CTimeWrapper operator -( const struct timeval &p_roTv, const CTimeWrapper &p_roTw )
{
    CTimeWrapper result;
    
    result.tv_sec = p_roTv.tv_sec - p_roTw.tv_sec;
    result.tv_usec = p_roTv.tv_usec - p_roTw.tv_usec;

    // second operand is negative, this means direction is +
    bool bNegative = p_roTw.Negative();

    if ( !bNegative && result.tv_sec > 0 && result.tv_usec < 0 )
    {
        result.tv_sec--;
        result.tv_usec += MICROSEC_IN_SEC;
    }
    else if ( bNegative && result.tv_sec < 0 && result.tv_usec >= 0 )
    {
        result.tv_sec++;
        result.tv_usec -= MICROSEC_IN_SEC;
    }

    return result;
}

CTimeWrapper operator +( const struct timeval &p_roTv, const CTimeWrapper &p_roTw )
{
    CTimeWrapper result;
    
    result.tv_sec = p_roTv.tv_sec + p_roTw.tv_sec;
    result.tv_usec = p_roTv.tv_usec + p_roTw.tv_usec;

    if ( ( result.tv_sec < 0 && result.tv_usec >= 0 ) || ( result.tv_sec > 0 && result.tv_usec >= MICROSEC_IN_SEC ) )
    {
        result.tv_sec++;
        result.tv_usec -= MICROSEC_IN_SEC;
    }
    else if ( ( result.tv_sec > 0 && result.tv_usec < 0 ) || ( result.tv_sec < 0 && result.tv_usec <= -MICROSEC_IN_SEC ) )
    {
        result.tv_sec--;
        result.tv_usec += MICROSEC_IN_SEC;
    }

    return result;
}


bool CTimeWrapper :: operator <( const struct timeval &tv )
{
    if ( tv_sec < tv.tv_sec )
        return true;
    else if ( tv_sec > tv.tv_sec )
        return false;
    else
    {
        if ( tv_usec < tv.tv_usec )
            return true;
        else
            return false;
    }
}

bool CTimeWrapper :: operator >( const struct timeval &tv )
{
    if ( tv_sec > tv.tv_sec )
        return true;
    else if ( tv_sec < tv.tv_sec )
        return false;
    else
    {
        if ( tv_usec > tv.tv_usec )
            return true;
        else
            return false;
    }
}


void CTimeWrapper :: PrintDelta( char *p_pcOutput )
{
    int sec;
    int ms;
    int us;

    if ( Negative() )
    {
        int diffUs = -tv_usec;

        sec = -tv_sec;
        ms = diffUs / 1000 + sec * 1000;
        us = diffUs % 1000;

        sprintf( p_pcOutput, "-%i.%.3i", ms, us );
    }
    else
    {
        sec = tv_sec;
        ms = tv_usec / 1000 + sec * 1000;
        us = tv_usec % 1000;

        sprintf( p_pcOutput, "%i.%.3i", ms, us );
    }
}

//void CTimeWrapper :: PrintAbsolute( char *p_pcOutput )
//{
//    // adjusting seconds based on special case
//    time_t adjTm = ( tv_sec == 0 && tv_usec < 0 ) ? -1 : tv_sec;
//
//    struct tm *time = gmtime( &adjTm );
//    
//    int sec = time->tm_sec;
//    int ms;
//    int us;
//
//    // adjustments
//    if ( tv_sec < 0 && tv_usec <= 0 )
//    {
//        int diffUs = MICROSEC_IN_SEC + tv_usec;
//        if ( diffUs == MICROSEC_IN_SEC )
//            diffUs = 0;
//        else
//             sec--;
//        
//        ms = diffUs / 1000;
//        us = diffUs % 1000;
//    }
//    else if ( tv_sec == 0 && tv_usec < 0 )
//    {
//        int diffUs = MICROSEC_IN_SEC + tv_usec;
//        ms = diffUs / 1000;
//        us = diffUs % 1000;
//    }
//    else
//    {
//        sec = time->tm_sec;
//        ms = tv_usec / 1000;
//        us = tv_usec % 1000;
//    }
//
//    // see struct tm documentation
//    sprintf( p_pcOutput, "%.2i.%.2i.%.4i %.2i:%.2i:%.2i.%.3i.%.3i", time->tm_mday, time->tm_mon + 1, time->tm_year + 1900, time->tm_hour, time->tm_min, sec, ms, us );
//}

//void CTimeWrapper :: PrintComponents( char *p_pcOutput )
//{
//    sprintf( p_pcOutput, "%li %li", tv_sec, tv_usec );
//}


CTimeWrapper CTimeWrapper :: Now()
{
    CTimeWrapper result;
    gettimeofday( &result, NULL );
    return result;
}