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
* Name:        MutexWrapper.h
* Author:      Ghervase Gabriel
* Date:        27.07.2010
* Description: declaration of a wrapper class for posix mutexes
* Changes:     27.07.2010 – Ghervase Gabriel – file creation
* Revisions:   27.07.2010 – Ghervase Gabriel – file creation
****************************************************/

#ifndef _MUTEX_WRAPPER_H_
#define _MUTEX_WRAPPER_H_


#include <pthread.h>


///////////////////////////////////////////////////////////////////////////////////
/// @brief wrapper class for posix mutexes
/// @class CMutexWrapper
/// @author Ghervase Gabriel
/// @ingroup libshared
///////////////////////////////////////////////////////////////////////////////////
class CMutexWrapper
{
private:
    pthread_mutex_t m_oMutex;
    
public:
    CMutexWrapper();
#ifndef HW_VR900
    CMutexWrapper(int p_nPrioceiling, int p_nProtocol, int p_nPshared, int p_nType);
#endif
    ~CMutexWrapper();
    
    bool Lock();
    bool Unlock();
    bool Try();
};


#endif //_MUTEX_WRAPPER_H_
