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

#ifndef _SEMAPHORE_WRAPPER_H_
#define _SEMAPHORE_WRAPPER_H_

#include <pthread.h>
#include <stdint.h>

class CPThreadSemaphore
{
private:
    pthread_mutex_t m_oMutex;
    pthread_cond_t m_oCond;
    uint32_t m_nMax;
    uint32_t m_nCount;

public:
    CPThreadSemaphore();                        // constructor for bynary semaphore
    CPThreadSemaphore( uint32_t p_nMax );     // constructor for counting semaphore
    ~CPThreadSemaphore();

    bool Wait();
    bool Signal();
};

#endif // _SEMAPHORE_WRAPPER_H_