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

/****************************************************
* Name:         PThreadSemaphore.h
* Author:       Mircea Cirjaliu
* Date:         10.03.2011
* Description:  Semaphore using PThread primitives.
* Changes:      10.03.2011 – file creation
* Revisions:    11.03.2011 – minor improvements
* Comments:     Causes Illegal instructions on VR900.
****************************************************/

#include "SemaphoreWrapper.h"
#include "Shared/LogDefs.h"

#include <string.h>
#include <errno.h>

// constructor for bynary semaphore
CPThreadSemaphore :: CPThreadSemaphore()
{
    pthread_mutex_init( &m_oMutex, NULL );
    pthread_cond_init( &m_oCond, NULL );

    m_nMax = 1;
    m_nCount = 0;
}

// constructor for counting semaphore
CPThreadSemaphore :: CPThreadSemaphore( uint32_t p_nMax )
{
    pthread_mutex_init( &m_oMutex, NULL );
    pthread_cond_init( &m_oCond, NULL );

    m_nMax = p_nMax;
    m_nCount = 0;
}

CPThreadSemaphore :: ~CPThreadSemaphore()
{
    int result;

    if ( ( result = pthread_mutex_destroy( &m_oMutex ) ) != 0 )
        NLOG_ERR( "%s, pthread_mutex_destroy() failed with result %d.", __PRETTY_FUNCTION__, result );
    
    if ( ( result = pthread_cond_destroy( &m_oCond ) ) != 0 )
        NLOG_ERR( "%s, pthread_cond_destroy() failed with result %d.", __PRETTY_FUNCTION__, result );
}

bool CPThreadSemaphore :: Wait()
{
    int result;
    bool fail = false;

    //LOG( "Thread 0x%x entered Wait().", pthread_self() );

    if ( ( result = pthread_mutex_lock( &m_oMutex ) ) != 0 )
    {
        NLOG_ERR( "%s, pthread_mutex_lock() failed with result %d.", __PRETTY_FUNCTION__, result );
        return false;
    }

    //LOG( "Thread 0x%x acquired mutex in Wait().", pthread_self() );

    while ( m_nCount == 0 )
    {
        if ( ( result = pthread_cond_wait( &m_oCond, &m_oMutex ) ) != 0 )
        {
            NLOG_ERR( "%s, pthread_cond_wait() failed with result %d.", __PRETTY_FUNCTION__, result );
            fail = true;
            break;
        }
    
        //LOG( "Thread 0x%x, pthread_cond_wait() exited, m_nCount is %d.", pthread_self(), m_nCount );
    }

    m_nCount--;

    //LOG( "Thread 0x%x decremented m_nCount to %d.", pthread_self(), m_nCount );

    if ( ( result = pthread_mutex_unlock( &m_oMutex ) ) != 0 )
    {
        NLOG_ERR( "%s, pthread_mutex_unlock() failed with result %d.", __PRETTY_FUNCTION__, result );
        return false;
    }

    //LOG( "Thread 0x%x exitting Wait().", pthread_self() );

    return !fail;
}

bool CPThreadSemaphore :: Signal()
{
    int result;
    bool fail = false;

    //LOG( "Thread 0x%x entered Signal().", pthread_self() );

    if ( ( result = pthread_mutex_lock( &m_oMutex ) ) != 0 )
    {
        NLOG_ERR( "%s, pthread_mutex_lock() failed with result %d.", __PRETTY_FUNCTION__, result );
        return false;
    }

    //LOG( "Thread 0x%x acquired mutex in Signal().", pthread_self() );

    if ( m_nCount < m_nMax )
    {
        m_nCount++;

        if ( m_nCount == 1 )
        {
            //LOG( "Thread 0x%x inside Signal() will signal.", pthread_self() );

            if ( ( result = pthread_cond_signal( &m_oCond ) ) != 0 )
            {
                NLOG_ERR( "%s, pthread_cond_signal() failed with result %d.", __PRETTY_FUNCTION__, result );
                fail = true;
            }
        }
        
        //LOG( "Thread 0x%x inside Signal() incremented m_nCount to %d.", pthread_self(), m_nCount );
    }
    else
    {
        NLOG_ERR( "%s, tried to signal the semaphore beyound limit %d.", __PRETTY_FUNCTION__, m_nMax );
        fail = true;
    }

    if ( ( result = pthread_mutex_unlock( &m_oMutex ) ) != 0 )
    {
        NLOG_ERR( "%s, pthread_mutex_unlock() failed with result %d.", __PRETTY_FUNCTION__, result );
        return false;
    }

    //LOG( "Thread 0x%x freed mutex & exitting Signal().", pthread_self() );

    return !fail;
}