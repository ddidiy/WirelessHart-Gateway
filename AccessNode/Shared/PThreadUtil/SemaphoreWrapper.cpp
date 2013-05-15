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


#include "SemaphoreWrapper.h"
#include "Shared/LogDefs.h"

#include <string.h>
#include <errno.h>

CSemaphoreWrapper :: CSemaphoreWrapper(unsigned int p_nInitialValue)
{
    if ( sem_init( &m_oSem, 0, p_nInitialValue ) != 0 )
        NLOG_ERR( "%s in %s, sem_init() failed.", strerror( errno ), __PRETTY_FUNCTION__ );
}

CSemaphoreWrapper :: ~CSemaphoreWrapper()
{
    if ( sem_destroy( &m_oSem ) != 0 )
        NLOG_ERR( "%s in %s, sem_destroy() failed.", strerror( errno ), __PRETTY_FUNCTION__ );
}

bool CSemaphoreWrapper :: Wait()
{
    if ( sem_wait( &m_oSem ) != 0 )
    {
        NLOG_ERR( "%s in %s, sem_wait() failed.", strerror( errno ), __PRETTY_FUNCTION__ );
        return false;
    }

    return true;
}   

bool CSemaphoreWrapper :: Signal()
{
    if ( sem_post( &m_oSem ) != 0 )
    {
        NLOG_ERR( "%s in %s, sem_post() failed.", strerror( errno ), __PRETTY_FUNCTION__ );
        return false;
    }

    return true;
}