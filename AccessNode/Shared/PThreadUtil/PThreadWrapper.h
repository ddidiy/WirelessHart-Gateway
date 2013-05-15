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

/*******************************************************
* 
*    Created: November 29, 2004
*    File:    myThread.h
*    Author:  Song Duk Kim, song.kim@nivis.com
*   
*    Purpose: a wrapper class for pthreads in linux
*******************************************************/
#ifndef _PTHREADWRAPPER_H_
#define _PTHREADWRAPPER_H_

//Modified by Claudiu Hobeanu on 2005/03/10 11:30
//  Changes : in interface   

//should be first conform with specs
#include <pthread.h>


#define INVALID_THREAD_ID	-1


#define TH_LOG LOG_DTIME(); LOG_IN("ThId=%d; ", GetWrapThId()); LOG 
/// @ingroup libshared
class CPThreadWrapper
{
public:
	CPThreadWrapper( bool bSelfDelete = false );
	virtual ~CPThreadWrapper();

	bool Start();
	void Stop() { m_bStopRequested = true; }

	void Join() {pthread_join( m_nThreadID, NULL );}
	
	bool IsRunning() { return m_bRunning; }
	bool IsStopReq() { return m_bStopRequested || m_bGlobalStopReq; }

	bool WaitEnd( int p_nMSec );

public:
	static bool LibInit();
	static void LibClose();

	static int	GetWrapThId();
	static void SetGlobalStop() { m_bGlobalStopReq = true; }
	static bool IsGlobalStopReq() { return m_bGlobalStopReq; }

protected:
	virtual void run() = 0;
	
	static void* ThreadFunc( void *lpParam );
	void pth_sleep( int p_nMSec );	

protected:
	pthread_t m_nThreadID;

private:
	
	bool m_bRunning;
	bool m_bStopRequested;
	bool m_bSelfDelete;
	int	m_nWrapThId;

	static pthread_key_t	m_nKey;
	static pthread_mutex_t	m_oBaseMutex;
	static int				m_nRefThId;
	static bool				m_bOnLine;
	static bool				m_bGlobalStopReq;
	
private:
	CPThreadWrapper( CPThreadWrapper & /*p_rTh*/ ) { };
};

#endif
