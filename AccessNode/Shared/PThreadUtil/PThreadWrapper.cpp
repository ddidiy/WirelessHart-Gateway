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

/**********************************************************************
*      created:	November 29, 2004
*	file:		PThreadWrapper.cpp
*	author:		Song Duk Kim, song.kim@nivis.com
*
*	purpose:	 a wrapper class for linux pthreads
**********************************************************************/

//Modified by Claudiu Hobeanu on 2005/03/10 11:30
//  Changes : rewrite of the implemention 

//should be first, because include <pthread.h> who had to be the first conform with specs
#include "PThreadWrapper.h"

#include <time.h>
#include <stdio.h>

#include "../Common.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

int CPThreadWrapper::m_nRefThId = 0;
pthread_key_t CPThreadWrapper::m_nKey;
pthread_mutex_t CPThreadWrapper::m_oBaseMutex;
bool CPThreadWrapper::m_bOnLine = false;
bool CPThreadWrapper::m_bGlobalStopReq = false;


CPThreadWrapper::CPThreadWrapper( bool bSelfDelete )
{
	m_bSelfDelete	= bSelfDelete;
	
	m_bRunning		= false;
	m_bStopRequested = false;

	m_nThreadID = 0;
	m_nWrapThId = INVALID_THREAD_ID;

	//should be up by now, but is not critical :) 
	if (!m_bOnLine) 
	{	LibInit();
	}
}

CPThreadWrapper::~CPThreadWrapper()
{
	LOG("CWrapThread::~CPThreadWrapper -- ThId = %X success", m_nWrapThId );
}

bool CPThreadWrapper::Start()
{
	//should be up by now, but is not critical :) 
	if (!m_bOnLine) 
	{	LibInit();
	}
	
	if (m_nWrapThId != INVALID_THREAD_ID) 
    {	return false;
    }
	
	pthread_mutex_lock( &m_oBaseMutex );
	m_nWrapThId = m_nRefThId++;
	pthread_mutex_unlock( &m_oBaseMutex );
	
	m_bRunning = true;
	if (pthread_create( &m_nThreadID, NULL, CPThreadWrapper::ThreadFunc, (void *)this ))
	{	LOG_ERR("CPThreadWrapper::Start: pthread_create failed");
		m_bRunning = false;
	}
	return m_bRunning;
}

void *CPThreadWrapper::ThreadFunc( void *lpParam )
{
	if (m_bOnLine) 
    {	if ( pthread_setspecific( m_nKey, &(((CPThreadWrapper *)lpParam )->m_nWrapThId )) != 0 )
		{	LOG_ERR("CPThreadWrapper::ThreadFunc() : pthread_setspecific() ");
		}
    }
	
	LOG("CPThreadWrapper -- ThId = %X is RUNNING ", ((CPThreadWrapper*)lpParam)->m_nWrapThId  );
	
	((CPThreadWrapper*)lpParam)->run();	
	
	LOG("CPThreadWrapper -- ThId = %X is ENDING", ((CPThreadWrapper*)lpParam)->m_nWrapThId );
	
	if (((CPThreadWrapper*)lpParam)->m_bSelfDelete) 
    {	delete (CPThreadWrapper*)lpParam;
    } 
	((CPThreadWrapper*)lpParam)->m_bRunning = false;
	return 0;
}


void CPThreadWrapper::pth_sleep( int p_nMSec )
{
	int nSlice = 10;
	int nElapsedTime = 0;

	struct timespec sleepTime;
	
	sleepTime.tv_sec = 0;
	sleepTime.tv_nsec = p_nMSec < nSlice ? p_nMSec : nSlice * 1000000;
	
	while(  nElapsedTime < p_nMSec  && !IsStopReq() )
    {	nanosleep( &sleepTime, NULL );
		nElapsedTime += nSlice;
    }	
}

bool CPThreadWrapper::WaitEnd( int p_nMSec )
{
	int nSlice = 10;
	int nElapsedTime = 0;

	struct timespec sleepTime;
	
	sleepTime.tv_sec = 0;
	sleepTime.tv_nsec = p_nMSec < nSlice ? p_nMSec : nSlice * 1000000;
	
	while(  nElapsedTime < p_nMSec  && IsRunning() )
    {	nanosleep( &sleepTime, NULL );
		nElapsedTime += nSlice;
    }	
	return IsRunning();
}

//used for logging
int CPThreadWrapper::GetWrapThId()
{
	if (!m_bOnLine) 
	{	return INVALID_THREAD_ID;
	}
	
	void *vpData = pthread_getspecific( m_nKey );
	return vpData ? *((int*)vpData) : INVALID_THREAD_ID;
}


//	LibInit -- expected to be called from main thread
//	if LinInit fails the only downfall is that ths will have a WrapThId 
//	and will affect logging
bool CPThreadWrapper::LibInit()
{
	if (m_bOnLine) 
	{	return true;
	}
	if (pthread_mutex_init(&m_oBaseMutex, NULL ))
	{	return m_bOnLine;
	}

	if (pthread_key_create(&m_nKey, NULL))
	{	return m_bOnLine;
	}
	
	m_bOnLine = true;
	return m_bOnLine;
}

//	LibClose -- expected to be called from main thread
void CPThreadWrapper::LibClose()
{
	if (!m_bOnLine) 
	{	return;
	}
	m_bOnLine = false;
	pthread_mutex_destroy(&m_oBaseMutex);
	pthread_key_delete(m_nKey);	
}
