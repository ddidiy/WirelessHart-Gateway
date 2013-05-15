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
* Name:        ThreadSafeQ.h
* Author:      Ghervase Gabriel
* Date:        3.08.2010
* Description: declaration of a thread safe queue class
* Changes:     3.08.2010 - Ghervase Gabriel - file creation
* Revisions:   3.08.2010 - Ghervase Gabriel - file creation
****************************************************/

#ifndef _THREADSAFEQ_H_
#define _THREADSAFEQ_H_

#include <list>
#include "Shared/MutexWrapper.h"


/// @ingroup libshared
template <typename T>
class CThreadSafeQ
{
//variables
private:
    std::list<T> m_oQueue;
    CMutexWrapper m_oQLock;

//methods
public:
	void Enqueue(const T& p_rElement);
    bool Dequeue(T& p_rElement);
    void Clear();
};


///////////////////////////////////////////////////////////////////////////////////
// Name:        Enqueue
// Author:      Ghervase Gabriel
// Description: inserts a message to the back of the queue
// Parameters:  p_rElement - element to add in queue
// Return:      void
//////////////////////////////////////////////////////////////////////////////////
template<typename T>
void CThreadSafeQ<T>::Enqueue(const T& p_rElement)
{
    m_oQLock.Lock();
        m_oQueue.push_back(p_rElement);
    m_oQLock.Unlock();
}

///////////////////////////////////////////////////////////////////////////////////
// Name:        Dequeue
// Author:      Ghervase Gabriel
// Description: removes and retrieves the element at the front of the Q
// Parameters:  p_rElement - output parameter to store the element
// Return:      true if an element was retrieved
//              false if the Q is empty (nothing to retrieve)
//////////////////////////////////////////////////////////////////////////////////
template<typename T>
bool CThreadSafeQ<T>::Dequeue(T& p_rElement)
{
    bool retval;
    m_oQLock.Lock();
        if(m_oQueue.empty())
        {   retval = false;
        }
        else
        {   p_rElement = m_oQueue.front();
            m_oQueue.pop_front();
            retval = true;
        }
    m_oQLock.Unlock();

    return retval;
}

///////////////////////////////////////////////////////////////////////////////////
// Name:        Clear
// Author:      Ghervase Gabriel
// Description: deletes all messages from the queue
// Parameters:  none
// Return:      void
//////////////////////////////////////////////////////////////////////////////////
template<typename T>
void CThreadSafeQ<T>::Clear()
{
    m_oQLock.Lock();
        m_oQueue.clear();
    m_oQLock.Unlock();
}


#endif //_THREADSAFEQ_H_
