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

//////////////////////////////////////////////////////////////////////////////
/// @file	WatchdogMngr.cpp
/// @author	Marius Negreanu.
/// @date	Jul 21, 2009
/// @brief	.
/// $Id: WatchdogMngr.cpp,v 1.40.16.1 2013/05/15 19:19:17 owlman Exp $
//////////////////////////////////////////////////////////////////////////////

#include <sys/inotify.h>
#include <sys/select.h>
#include <signal.h>
#include <search.h>
 #include <sys/time.h> // gettimeofday

#include "Shared/h.h"
#include "Shared/IniParser.h"
#include "Shared/SimpleTimer.h"
#include "WatchdogMngr.h"


/* size of the event structure, not counting name */
#define NTF_EVENT_SIZE  (sizeof (struct inotify_event))

/* reasonable guess as to size of 1024 events */
#define NTF_BUF_LEN        (1024 * (NTF_EVENT_SIZE + 16))

/*used for prepare_for_reboot.sh: mili seconds*/
#define ON_QUIT_DELAY				(1*60*1000)
#define ON_QUIT_DELAY_FTP_LOGS		(3*60*1000)

/*check pidfiles period*/
#define WTD_PID_VRFY_INTERVAL	(15*WTD_PID_VRFY_UNIT)

/*WTD_PID_VRFY_INTERVAL multiplier. Seconds in a minute*/
#define WTD_PID_VRFY_UNIT	(60)


int CWatchdogMngr::Open(const char*pidListFile)
{
	if ( -1 == (m_fd=inotify_init()) )
	{
		PERR( WTD "inotify_init");
		return false ;
	}
	m_pPidListFile = pidListFile ;
	//if ( ! addFileWatch(m_pPidListFile) ) return false ;
	return true ;
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
int CWatchdogMngr::LoadPidList()
{
	CIniParser oIniParser;

	if ( !oIniParser.Load(m_pPidListFile) )
	{
		throw CWatchdog::exception::exception("Unable to load pidListFile") ;
	}

	LOG("Loaded pidListFile [%s]", m_pPidListFile);

	char*  currentPidList[WTD_PIDLIST_ENTRIES]={0,};
	size_t currentPidListLen=0;
	char   pidFile[256];
	for ( unsigned pos=0
		; oIniParser.GetVar("WTD", "watch", pidFile, sizeof(pidFile), pos)
		  && currentPidListLen < WTD_PIDLIST_ENTRIES
		;  ++pos )
	{
		void*key=pidFile;
		void *el=lfind( &key,currentPidList,&currentPidListLen,sizeof(currentPidList[0]),qsortcmp ) ;
		if ( NULL == el )
		{
			currentPidList[currentPidListLen] = strdup(pidFile);
			LOG( WTD"fw.cfg:[%s]", currentPidList[currentPidListLen] );
			++currentPidListLen ;
		}
	}
	qsort( currentPidList, currentPidListLen, sizeof(currentPidList[0]), qsortcmp );

	/// Check if any pidFile from m_ppPidList is missing from currentPidList.
	for (   unsigned j=0; j < m_nPidListLength && m_ppPidList[j]; ++j )
	{
		void *el=bsearch( &m_ppPidList[j], currentPidList, currentPidListLen, sizeof(currentPidList[0]), qsortcmp) ;
		if ( NULL == el )
		{
			LOG( WTD "STOP watching pid[%s]", m_ppPidList[j]);
			systemf_to( 20, "log2flash 'WTD: STOP watching pid[%s]'&", m_ppPidList[j]);
		}
	}
	/// Free the old list to prepare the copy of the new one.
	clearPidList() ;

	m_nPidListLength=currentPidListLen ;
	m_ppPidList[m_nPidListLength] = 0;

	LOG( WTD"WATCH:%d modules", m_nPidListLength );
	if ( !m_nPidListLength )
	{
		return true ;
	}
	memmove(m_ppPidList, currentPidList, WTD_PIDLIST_ENTRIES*sizeof(currentPidList[0]) );
	for ( unsigned j = 0; j< m_nPidListLength ; j++)
	{
		LOG( WTD "WATCH:[%s]", m_ppPidList[j]);
	}
	return true ;
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void CWatchdogMngr::Sleep() {
	fd_set rfds;
	struct timeval tv;
	int retval;
	//char buf[NTF_BUF_LEN];
	m_nTimeout = m_lPingInterval/m_nPingDivisor ;
	if ( m_nTimeout > 3 ) m_nTimeout = 3 ;

	FD_ZERO(&rfds);
	FD_SET(m_fd, &rfds);

	tv.tv_sec  = m_nTimeout ;
	tv.tv_usec = 0;

	retval = select(m_fd + 1, &rfds, NULL, NULL, &tv);
	/* Don't rely on the value of tv now! */
	
	// don't care about files modified. because he's bad
#if 0
	if (retval == -1 )
	{	// TODO: better checking/return policy when EINTR
		if ( errno != EINTR )
		{
			throw CWatchdog::exception::exception("error in select()");
		}
		return;
	} else if (!retval)
	{	/*File WASN'T modified and timeout expired */
		m_status &= ~FW_CFG_MODIFIED ;
		return;
	}
	//else File WAS modified.
	m_status |= FW_CFG_MODIFIED ;

	retval = read(m_fd, buf, NTF_BUF_LEN);
	if (retval < 0)
	{
		if (errno != EINTR)
		{	/* need to reissue system call */
			throw CWatchdog::exception::exception("error in select()");
		}
	} else if (retval>0)	/* NTF_BUF_LEN too small? */
	{
		int i=0;

		while (i < retval)
		{
			struct inotify_event *event;

			event = (struct inotify_event *) &buf[i];

			LOG( WTD "wd=%d mask=%u cookie=%u len=%u", event->wd,
				event->mask, event->cookie, event->len);

			if (event->len)
				LOG( WTD "name=%s", event->name);

			i += NTF_EVENT_SIZE + event->len;
		}
	}
#endif
}
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
int CWatchdogMngr::PidMissing()
{
	char szBlockedModules[256]={0,} ;
	if ( checkDelPidFiles((const char**)m_ppPidList,szBlockedModules) )
	{
		return false ;
	}
	ERR("Pids missing:[%s]",szBlockedModules);
	systemf_to( 20, "log2flash 'WTD: ERR:Pids missing:[%s]. Force REBOOT.'&",szBlockedModules) ;
	return true ;
}
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void CWatchdogMngr::OnQuit()
{
	CSimpleTimer t ;

	Ping(); //to make sure that prepare_for_reboot.sh has time to run
	systemf_to( ON_QUIT_DELAY, NIVIS_FIRMWARE "prepare_for_reboot.sh&"  );
	LOG("Waiting[%u seconds] for prepare_for_reboot.sh",ON_QUIT_DELAY) ;

	int nTimeLeftToPing = ON_QUIT_DELAY;

	if (FileIsExecutable(FTP_SCRIPT))
	{
		nTimeLeftToPing = ON_QUIT_DELAY_FTP_LOGS;
	}

	t.SetTimer( nTimeLeftToPing) ;
	while( ! t.IsSignaling() )
	{
		Ping() ;
		Sleep() ;
	}
	Ping() ;
	systemf_to( 20, "log2flash 'WTD: Finished running quit procedure.'&");
}
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
enum CWatchdog::Type CWatchdogMngr::GetType()
{
	char wtd[16]={0} ;
	CIniParser oIniParser;

	if ( !oIniParser.Load(INI_FILE) )
		throw CWatchdog::exception::exception("Unable to parse IniFile") ;

	if ( ! oIniParser.GetVar("GLOBAL","WATCH_DOG_TYPE", wtd, sizeof(wtd))
	||   ! strcasecmp(wtd,"internal") )
	{
		LOG( WTD "Using the internal watchdog") ;
		return CWatchdog::WTD_INTERNAL ;
	}

	if ( ! strcasecmp(wtd,"max6371") )
	{
		LOG( WTD "Using the max6371 watchdog") ;
		return CWatchdog::WTD_MAX6371 ;
	}
	return CWatchdog::WTD_INTERNAL ;
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
int CWatchdogMngr::LoadConfig( )
{
	char wtd[16]={0} ;
	int divisor ;
	CIniParser oIniParser;
	if ( !oIniParser.Load(INI_FILE) )
		throw CWatchdog::exception::exception("Unable to parse IniFile") ;

	oIniParser.GetVar("GLOBAL","MODULES_WATCH_DOG", wtd, sizeof(wtd) ) ;

	if(  wtd[0]=='n' || wtd[0]=='N'
	  || wtd[0]=='f' || wtd[0]=='F'
	  || wtd[0]=='0'
	   )
	{
		if ( m_status&CFG_ENABLED )
		{
			m_status&=~CFG_ENABLED ;
			LOG("Disabling watchdog");
		}
	}
	else if ( ! (m_status&CFG_ENABLED) )
	{
		m_status|=CFG_ENABLED;
		LOG("Enabling watchdog");
	}
	// Set the PingDivisor
	if ( oIniParser.GetVar("GLOBAL","WATCH_DOG_DIVISOR", &divisor))
	{
		SetPingDivisor(divisor) ;
	}
	if ( ! oIniParser.GetVar("GLOBAL","FREEMEM_THRESHOLD", &m_nMemThreshold) )
	{
		m_nMemThreshold = 5*1024*1024 ;
	}
	if ( ! oIniParser.GetVar("GLOBAL","FREETMP_THRESHOLD", &m_nTmpThreshold) )
	{
		m_nTmpThreshold = 4*1024*1024 ;
	}
	return (m_status&CFG_ENABLED) ;
}


int CWatchdogMngr::Add(CWatchdog* p_wdg, int p_type)
{
	if ( p_type >= __MAX_WATCHDOGS )
		return false ;

	m_wdg[p_type] = p_wdg ;
	m_nInsertPos = _max_(m_nInsertPos, p_type) ;
	m_lPingInterval = _max_(m_lPingInterval, p_wdg->GetPingInterval() );

	return true ;
}
void CWatchdogMngr::Ping()
{
#define _USEC_IN_SEC 1000000 /* Microseconds in a second */
	struct timeval now;
	gettimeofday(&now, (struct timezone *)NULL);

	int usecDiff = (now.tv_usec + now.tv_sec*_USEC_IN_SEC) - (m_nLastPingTime.tv_usec + m_nLastPingTime.tv_sec*_USEC_IN_SEC);
	usecDiff -= m_nTimeout*_USEC_IN_SEC ;
	if ( usecDiff > 600000 /* we alow a 600msec drift*/ )
	{
		LOG("I was %d uSeconds late on ping.", usecDiff );
		systemf_to( 20, "log2flash 'WTD: I was %d uSeconds late on ping.' &", usecDiff);
	}
	m_nLastPingTime = now;

	if ( m_nCurrentWtd < __MAX_WATCHDOGS )
		m_wdg[m_nCurrentWtd]->Ping() ;

	for ( uint8_t i=0; i< m_nInsertPos && m_nCurrentWtd==CWatchdog::WTD_ALL; ++i )
		m_wdg[i]->Ping() ;

#undef _USEC_IN_SEC
}
void CWatchdogMngr::Close()
{
	if ( m_nCurrentWtd < __MAX_WATCHDOGS )
	{
		m_wdg[m_nCurrentWtd]->Close() ;
		delete m_wdg[m_nCurrentWtd];
	}

	for ( uint8_t i=0; i< m_nInsertPos && m_nCurrentWtd==CWatchdog::WTD_ALL; ++i)
	{
		m_wdg[i]->Close() ;
		delete m_wdg[i] ;
	}
	reset();
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
int CWatchdogMngr::addFileWatch(const char* pidListFile)
{
	if ( -1 == access(pidListFile, R_OK) && m_status&CFG_ENABLED )
	{
		PERR( WTD "Disabling WTD: addFileWatch->access");
		systemf_to( 20, "log2flash 'WTD: Disabling WTD: addFileWatch(%s) access->[%s]' &", pidListFile, strerror(errno) );
		m_status&=~CFG_ENABLED ;
		return false ;
	}

	m_wd = inotify_add_watch( m_fd, pidListFile, IN_MODIFY) ;
	if (m_wd < 0)
	{
		PERR( WTD "inotify_add_watch");
		systemf_to( 20, "log2flash 'WTD: inotify_add_watch(%s) failed [%s]' &", pidListFile, strerror(errno) );
		throw CWatchdog::exception::exception("Unable to add notify watch") ;
	}
	return true ;
}
//////////////////////////////////////////////////////////////////////////////
/// @brief Set the default values for members.
//////////////////////////////////////////////////////////////////////////////
void CWatchdogMngr::reset()
{
	m_nPidListLength=0;
	m_fd=-1;
	m_wd=-1;
	m_status=FW_CFG_MODIFIED|CFG_ENABLED;
	m_nCurrentWtd=0;
	m_nInsertPos=0;
	m_lPingInterval=0;
	m_nPingDivisor=5;
	m_nMemThreshold=5*1024*1024;
	m_nTmpThreshold=4*1024*1024;
	memset(m_ppPidList,0,WTD_PIDLIST_ENTRIES);
	memset(m_wdg,0,__MAX_WATCHDOGS);
	gettimeofday(&m_nLastPingTime,(struct timezone *)NULL);
}

//////////////////////////////////////////////////////////////////////////////
/// @brief removes all the pidfiles specified, but not too frequently
/// @author	Mihai Buha (mihai.buha@nivis.com)
/// @param[in]	p_pszNames null-terminated array of filenames
/// @param[out]	p_szMissNames if not null, fill with names of
///			missing files. Take care to allocate enough memory for
///			"Watchdog: <all filenames in p_pszNames> " or else bad
///			things will happen.
/// @retval true  All pidfiles existed or timeout not expired
/// @retval false Some pidfile was missing (controlling app was dead -
///			did not create a file during latest 2 intervals)
//////////////////////////////////////////////////////////////////////////////
bool CWatchdogMngr::checkDelPidFiles (const char** p_pszNames, char* p_szMissNames)
{
	clock_t timestamp ;
	static clock_t sStartTime = GetClockTicks();
	static clock_t last_checked;
	static bool last_existed = true;
	bool status = true;
	bool exists = true;

	timestamp = GetClockTicks();
	if( timestamp < last_checked)
	{ // large uptime overflowed the clock_t
		last_checked = timestamp;
		sStartTime = timestamp;
	}
	if( (timestamp - last_checked < WTD_PID_VRFY_INTERVAL * sysconf(_SC_CLK_TCK)) 
		|| (timestamp - sStartTime < WTD_PID_VRFY_INTERVAL * sysconf(_SC_CLK_TCK))
		)
	{
		return true;
	}

	last_checked = timestamp;
	for( unsigned i=0; p_pszNames[i]; ++i)
	{
		int nFileLen = GetFileLen(p_pszNames[i]);

		if (nFileLen == 0)
		{	
			usleep(30*1000);//wait a little maybe in process of writing the pid file
			nFileLen = GetFileLen(p_pszNames[i]);
		}

		if ( nFileLen > 0)
		{	unlink( p_pszNames[i]);
			continue;
		}
		if (nFileLen == 0)
		{	
			ERR("checkDelPidFiles: pidfile[%s] len==0",  p_pszNames[i]);
			systemf_to( 20 ,"log2flash 'WTD: ERR:checkDelPidFiles: pidfile[%s] len==0' &",  p_pszNames[i]);
			unlink( p_pszNames[i]);			
		}

		LOG( "ERR:Missing [%s]", p_pszNames[i]);

		if ( exists )
		{
			exists = false;
		}

		if( p_szMissNames )
		{
			strcat( p_szMissNames, p_pszNames[i]);
			strcat( p_szMissNames, ",");
		}
	}
	// Take snapshot if any of the pidfiles is missing.
	if (! exists)
	{
		system_to( 60, NIVIS_TMP"take_system_snapshot.sh "ACTIVITY_DATA_PATH"snapshot_warning.txt &");
	}

	status = exists || last_existed;
	last_existed = exists;
	return status;
}
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void	CWatchdogMngr::clearPidList()
{
	for ( unsigned i=0; i< m_nPidListLength; ++i)
	{
		free(m_ppPidList[i]);
		m_ppPidList[i] = 0 ;
	}
	m_nPidListLength=0;
}
