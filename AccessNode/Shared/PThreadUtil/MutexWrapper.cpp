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
* Name:        MutexWrapper.cpp
* Author:      Ghervase Gabriel
* Date:        27.07.2010
* Description: definition of the wrapper class for posix mutexes
* Changes:     27.07.2010 – Ghervase Gabriel – file creation
* Revisions:   27.07.2010 – Ghervase Gabriel – file creation
****************************************************/


#include "MutexWrapper.h"
#include "Shared/LogDefs.h"

#include <string.h>
#include <errno.h>


///////////////////////////////////////////////////////////////////////////////////
// Name:        CMutexWrapper
// Author:      Ghervase Gabriel
// Description: contructor for creating a mutex with implicit attributes
// Parameters:  none
// Return:      none
//////////////////////////////////////////////////////////////////////////////////
CMutexWrapper::CMutexWrapper()
{
    pthread_mutex_init(&m_oMutex, 0);
}

#ifndef HW_VR900
///////////////////////////////////////////////////////////////////////////////////
// Name:        CMutexWrapper
// Author:      Ghervase Gabriel
// Description: contructor for creating a mutex with explicit attributes
// Parameters:  p_nPrioceiling  - the priority ceiling of initialised mutexes,
//                                which is the minimum priority level at which
//                                the critical section guarded by the mutex is executed
//                                Must be within the maximum range of priorities
//                                defined under the SCHED_FIFO scheduling policy
//              p_nProtocol     - protocol to be followed ; can be one of :
//                                    PTHREAD_PRIO_NONE,
//                                    PTHREAD_PRIO_INHERIT,
//                                    PTHREAD_PRIO_PROTECT
//              p_nPshared      - mutex's process-shared state;
//                                determines whether the mutex can be used to
//                                synchronize threads within the current process
//                                or threads within all processes on the system
//                                Values :
//                                    PTHREAD_PROCESS_PRIVATE,
//                                    PTHREAD_PROCESS_SHARED
//              p_nType         - type of mutex. Values:
//                                    PTHREAD_MUTEX_NORMAL,
//                                    PTHREAD_MUTEX_RECURSIVE,
//                                    PTHREAD_MUTEX_ERRORCHECK,
//                                    PTHREAD_MUTEX_DEFAULT
// Return:      none
//////////////////////////////////////////////////////////////////////////////////
CMutexWrapper::CMutexWrapper(int p_nPrioceiling, int p_nProtocol, int p_nPshared, int p_nType)
{
    int result;
    pthread_mutexattr_t m_pAttr;

    pthread_mutexattr_init(&m_pAttr);
    pthread_mutexattr_setprioceiling(&m_pAttr, p_nPrioceiling);
    pthread_mutexattr_setprotocol(&m_pAttr, p_nProtocol);
    pthread_mutexattr_setpshared(&m_pAttr, p_nPshared);

    if ( ( result = pthread_mutexattr_settype(&m_pAttr, p_nType) ) != 0 )
    {
        NLOG_ERR( "%s, pthread_mutexattr_settype() failed with result %d.", __PRETTY_FUNCTION__, result );
        return;
    }
    
    if ( ( result = pthread_mutex_init(&m_oMutex, &m_pAttr) ) != 0 )
    {
        NLOG_ERR( "%s, pthread_mutex_init() failed with result %d.", __PRETTY_FUNCTION__, result );
        return;
    }
}
#endif

CMutexWrapper::~CMutexWrapper()
{
    int result;
    if ( ( result = pthread_mutex_destroy(&m_oMutex) ) != 0 )
    {
        NLOG_ERR( "%s, pthread_mutex_destroy() failed with result %d.", __PRETTY_FUNCTION__, result );
    }
}

///////////////////////////////////////////////////////////////////////////////////
// Name:        Lock
// Author:      Ghervase Gabriel
// Description: acquire lock on mutex
// Parameters:  none
// Return:      true - lock was acquired
//              false - lock was not acquired, or fail
//////////////////////////////////////////////////////////////////////////////////
bool CMutexWrapper::Lock()
{
    int result;
    if ( ( result = pthread_mutex_lock(&m_oMutex) ) != 0 )
    {
        NLOG_ERR( "%s, pthread_mutex_lock() failed with result %d.", __PRETTY_FUNCTION__, result );
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////
// Name:        Unlock
// Author:      Ghervase Gabriel
// Description: release lock on mutex
// Parameters:  none
// Return:      true - lock was acquired
//              false - lock was not acquired, or fail
//////////////////////////////////////////////////////////////////////////////////
bool CMutexWrapper::Unlock()
{
    int result;
    if ( ( result = pthread_mutex_unlock(&m_oMutex) ) != 0 )
    {
        NLOG_ERR( "%s, pthread_mutex_unlock() failed with result %d.", __PRETTY_FUNCTION__, result );
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////
// Name:        Try
// Author:      Ghervase Gabriel
// Description: try lock on mutex; acquire lock if free and return if not
// Parameters:  none
// Return:      true - lock was acquired
//              false - lock was not acquired, or fail
//////////////////////////////////////////////////////////////////////////////////
bool CMutexWrapper::Try()
{
    int result = pthread_mutex_trylock(&m_oMutex);
    
    if (result == 0)
    {   
        return true;            // mutex was free and therefore I acquired the lock
    }
    else if ( result != EBUSY )
    {
        NLOG_ERR( "%s, pthread_mutex_trylock() failed with result %d.", __PRETTY_FUNCTION__, result );
    }

    //mutex already locked, or fail
    return false;
}
