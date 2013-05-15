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

 /***************************************************************************
                          app.cpp  -  description
                             -------------------
    begin                : Fri Apr 12 2002
    email                : marcel.ionescu@nivis.com
 ***************************************************************************/
#include <signal.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/file.h>
#include <stdlib.h>

#include "SignalsMgr.h"

#include "app.h"
#include "Common.h"
#include "AlarmEx.h"
#include "Utils.h"
#include "IniParser.h"

#define DEFAULT_MODEM_TO_DN_BAUD_RATE B19200

int     CApp::m_nSyncFd = 0;
int		CApp::m_nStopTimeout = 30;
char	CApp::m_szModule[64] = {0,};
volatile int     CApp::m_nStop = 0;

CApp::CApp( const char * p_szAppName )
{
	m_nStarted = 0;
	strncpy( m_szModule, p_szAppName, sizeof(m_szModule));
	m_szModule [ sizeof(m_szModule) -1 ] = 0;
//	m_nStop = 0;
	m_ushConnStatsWritePos = 0;
	m_tLastStatsTime = time(NULL);/// first stat must be after LOG2FLASH_STATS_INTERVAL, not at start

}

CApp::~CApp()
{
    if( m_nStarted > 0 )
    {
        flock(m_nSyncFd,LOCK_UN);
        close(m_nSyncFd);
    }
    m_tLastStatsTime = 0;
	log2flash_maintenance();
}

/** make any necessary initialisation  */
int CApp::Init( const char *p_lpszLogFile, int p_nMaxLogSize /*= 524288*/ )
{
//Modified by Claudiu Hobeanu on 2004/10/25 14:34
//  Changes : move signals handle in CSignalsMgr

	CSignalsMgr::Ignore(SIGHUP);
	CSignalsMgr::Ignore(SIGTTOU);
	CSignalsMgr::Ignore(SIGTTIN);
	CSignalsMgr::Ignore(SIGTSTP);
	CSignalsMgr::Ignore(SIGPIPE);

	CSignalsMgr::Install( SIGTERM, HandlerSIGTERM );
	CSignalsMgr::Install( SIGINT, HandlerSIGTERM );

	///NEVER USE CSignalsMgr TO HANDLE SIGABRT/SIGSEGV
	signal( SIGABRT, HandlerFATAL );/// do NOT use delayed processing with HandlerFATAL. Stack trace must be dumped on event
	signal( SIGSEGV, HandlerFATAL );/// do NOT use delayed processing with HandlerFATAL. Stack trace must be dumped on event
	signal( SIGFPE,  HandlerFATAL );/// do NOT use delayed processing with HandlerFATAL. Stack trace must be dumped on event
    signal( SIGILL,  HandlerFATAL );

	//close stdin, stdout and stderr then open them as /dev/null (fix problems with fd's after forking)
#if !defined( DONT_CLOSE_STD )
	close(0);
	close(1);
	close(2);
	open("/dev/null", O_RDWR);
	open("/dev/null", O_RDWR);
	open("/dev/null", O_RDWR);
#endif
	strcpy(m_szAppPidFile, p_lpszLogFile);
	int nLen = strlen(m_szAppPidFile);

	if (	m_szAppPidFile[nLen-1] == 'g' && m_szAppPidFile[nLen-2] == 'o'
		&&	m_szAppPidFile[nLen-3] == 'l' && m_szAppPidFile[nLen-4] == '.' )
	{	m_szAppPidFile[nLen-4] = 0;
	}

	char szLockFile[256];

	strcpy(szLockFile, m_szAppPidFile);
	strcat(szLockFile, ".flock");

	strcat(m_szAppPidFile, ".pid" );

    //open ilog file with default parameters... to have something...
    if( !g_stLog.Open(p_lpszLogFile, "Start session", p_nMaxLogSize))
        return 0;

    LOG( "CApp(%s)::Init - version: %s", m_szModule, version() );

    m_nSyncFd = open( szLockFile, O_RDWR | O_CREAT, 0666 );
    if( flock( m_nSyncFd, LOCK_EX | LOCK_NB ) )
    {
        LOG( "Process %d try to start but another instance of program is running ",getpid());
        return 0;
    }

	SetCloseOnExec(m_nSyncFd);

	LOG("System MEMORY Available: %dkB", GetSysFreeMemK() );

// 	if( ! m_stTmpSharedVar.Init( NIVIS_TMP"shared_var.shd", SHARED_TMP ) )
// 		return 0;

	if (!m_oModulesActivity.Open())
	{	return 0;
	}

    return 1;
}


//exists gracefully
void	CApp::HandlerSIGTERM(int)
{
	LOG("Signal to stop set: max %d sec", m_nStopTimeout );
	SetStop();
	int timeLeft = alarm(0);
	alarm( _Min(timeLeft,m_nStopTimeout-1) );
	AlarmEx( SIGKILL, m_nStopTimeout );   //set watchdog
}

#if 0
////////////////////////////////////////////////////////////////////////////////
/// Read the %fp register and walk down the frames.
////////////////////////////////////////////////////////////////////////////////
void trace(int frames, char * out, size_t& outSz)
{
	/* Get the current frame pointer. */
	register unsigned long fp __asm("fp");
	unsigned long *frame_pointer=fp;

	for ( int i=0; i<frames; i++ )
	{
		n += snprintf("%lx ", n, *(frame_pointer - 1) - 8) ;
		frame_pointer = (unsigned long*)*(frame_pointer - 3) ;
	}
}
#endif

////////////////////////////////////////////////////////////////////////////////
/// @author Marcel Ionescu
/// @brief Handle out of memory / invalid access - fatal errros: SIGSEGV/SIGABRT - log2flash fault address
/// @retval none
/// @remarks To identify the crash: build a unstripped version from the
/// same source tree as the crashing program  (tag releases!).
/// The unstripped version can be found in Accessnode/out/<toolchain>/...
/// Use addr2line (from the correct toolchain) to find the crash line. Example for VR900:
/// /opt/nivis/m68k-unknown-linux-uclibc/bin/m68k-unknown-linux-uclibc-addr2line -f -i -C -e <exe> <addr_list_any_format>
/// <exe> is executable WITH debug info
/// <addr_list_any_format> is any of/several 0xXXXXXXXX xXXXXXXXX XXXXXXXX separated by spaces
/// @note
/// @note Do NOT use delayed processing (CSignalsManager) with HandlerFATAL. Stack trace must be dumped on event or is lost
/// @note
////////////////////////////////////////////////////////////////////////////////
#define BKTRACE(a,b,c) if(__builtin_frame_address(a)) { b += sprintf((str+b), "%p ", __builtin_return_address(a)); } else { c = 0;}
void HandlerFATAL(int p_signal)
{
	static char str[4098];	/// 11 bytes per address. Watch for overflow
	int i;
	int n=0, bContinue=1 ;	// used by BKTRACE macro
	*str=0;/// reset every time

// ARM/CYG has an older uClibc with no program_invocation_short_name support
// so use __progname
#if defined(ARM) || defined(CYG)
	extern const char *__progname;
#endif

	for ( i=0; bContinue && (i<16) ; ++i)
	{	switch (i)	/// parameter for __builtin_frame_address must be a constant integer
		{	case 0:	BKTRACE(0,n,bContinue);	break ;
// ARM doesn't support more than 1 stack unwind.
#if !defined(ARM) && !defined(CYG)
			case 1:	BKTRACE(1,n,bContinue);	break ;
			case 2:	BKTRACE(2,n,bContinue);	break ;
			case 3:	BKTRACE(3,n,bContinue);	break ;
			case 4:	BKTRACE(4,n,bContinue);	break ;
			case 5:	BKTRACE(5,n,bContinue);	break ;
			case 6:	BKTRACE(6,n,bContinue);	break ;
			case 7:	BKTRACE(7,n,bContinue);	break ;
			case 8:	BKTRACE(8,n,bContinue);	break ;
			case 9:	BKTRACE(9,n,bContinue);	break ;
			case 10:BKTRACE(10,n,bContinue);	break ;
			case 11:BKTRACE(11,n,bContinue);	break ;
			case 12:BKTRACE(12,n,bContinue);	break ;
			case 13:BKTRACE(13,n,bContinue);	break ;
			case 14:BKTRACE(14,n,bContinue);	break ;
			case 15:BKTRACE(15,n,bContinue);	break ;
#elif defined(ARM)
			case 1:
			n+=snprintf( str+n, sizeof(str)-n-1,
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				"%08x %08x %08x %08x %08x %08x %08x %08x "
				);
			break;
#endif
		}
	}

	log2flash("PANIC [%s] Pid[%lu] %s %u. Backtrace [%s]",
// ARM/CYG has an older uClibc with no program_invocation_short_name support
// so use __progname
#if !defined(ARM) && !defined(CYG)
		program_invocation_short_name,
#else
		__progname,
#endif
		getpid(),
		(p_signal == SIGSEGV) ? "SIGSEGV" :
		(p_signal == SIGABRT) ? "SIGABRT" :
        (p_signal == SIGILL) ? "SIGILL" :
		(p_signal == SIGFPE)  ? "SIGFPE" :"UNK", p_signal, str);
	/// make sure to flush the log to disk. Standard LOG does it at this time. Put an extra \n to be absolutely sure
	LOG("PANIC [%s] Pid[%lu] %s %u. Backtrace [%s]\n",
#if !defined(ARM) && !defined(CYG)
		program_invocation_short_name,
#else
		__progname,
#endif
		getpid(),
		(p_signal == SIGSEGV) ? "SIGSEGV" :
		(p_signal == SIGABRT) ? "SIGABRT" :
        (p_signal == SIGILL) ? "SIGILL" :
		(p_signal == SIGFPE)  ? "SIGFPE"  : "UNK", p_signal, str);
	exit(EXIT_FAILURE);
}


void CApp::Close()
{
	//m_stTmpSharedVar.Close();
	m_oModulesActivity.Close();

	if( m_nSyncFd > 0 )
	{	flock(m_nSyncFd, LOCK_UN);
		close(m_nSyncFd);
		m_nSyncFd = 0;
	}

	AlarmEx( SIGKILL, 0 );	//stop watchdog
	LOG("APP ENDED -- OK");
	g_stLog.Close();
}


#ifndef VERSION
#define VERSION "not defined"
#endif

/** return version (as a string) as definet in global..conf included by global Makefile */
const char * CApp::version( void )
{
	return VERSION;
}


const char* CApp::GetVersion()
{
	return VERSION;
}

////////////////////////////////////////////////////////////////////////////////
/// @author Marcel Ionescu
/// @brief log2flash statistics for buffered connections (from sources already logged)
/// @note log2flash no faster than once eVery LOG2FLASH_STATS_INTERVAL
/// @note it can, however, log slower, in case the caller call this method less often
////////////////////////////////////////////////////////////////////////////////
void CApp::log2flash_maintenance( void  )
{
	time_t tNow = time( NULL );
	if( (tNow - m_tLastStatsTime) < LOG2FLASH_STATS_INTERVAL)
		return;/// not just yet

	m_tLastStatsTime = tNow;
	/// dump all, then reset counters
	for( unsigned i=0; (i < LOG2FLASH_RECORDS) && m_aPrevConn[i].m_szConnection[0]; ++i )
	{
		if( m_aPrevConn[i].m_unCounter )
		{
			log2flash( "%s: Connections from %s last hour: %d", m_szModule, m_aPrevConn[i].m_szConnection, m_aPrevConn[i].m_unCounter );
			m_aPrevConn[i].m_unCounter = 0;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
/// @author Marcel Ionescu
/// @brief log2flash connections new sources (as in p_szTxt) only
/// @param p_szTxt - connection originator specifier (IP/Type, or whaterver other specifier) 
/// @remarks An p_szTxt is only logged to flash on first connection
/// @remarks Subsequenct connections DO NOT get logged to flash, unless internal buffer is exceeded
/// @note if internal buffer is exceeded, overwrite oldest and log
/// @note based on the assumption that hammering is done from a single (or a small group of) IP
/// @note TAKE CARE: current approach is inappropriate for hammering from a wide range of IP's
////////////////////////////////////////////////////////////////////////////////
void CApp::log2flash_newconn( const char * p_szTxt )
{
	for( unsigned i=0; (i < LOG2FLASH_RECORDS) && m_aPrevConn[i].m_szConnection[0]; ++i )
	{
		if( !strcmp(p_szTxt, m_aPrevConn[i].m_szConnection) )
		{	// found: already logged2flash, do nothing
			++m_aPrevConn[i].m_unCounter;
			LOG("log2flash_newconn: [%s] already recorded at idx %d, counter %d (WriteIdx %d)", 
				p_szTxt, i, m_aPrevConn[i].m_unCounter, m_ushConnStatsWritePos);
			return;
		}
	}
	// not found: record it over oldest
	m_aPrevConn[ m_ushConnStatsWritePos ].Set( p_szTxt  );

	LOG("log2flash_newconn: record [%s] at idx %d", p_szTxt,  m_ushConnStatsWritePos);
	log2flash( "%s: Connection  from %s", m_szModule, p_szTxt );

	if( ++m_ushConnStatsWritePos >= LOG2FLASH_RECORDS )
		m_ushConnStatsWritePos = 0; // circular advance 
}
