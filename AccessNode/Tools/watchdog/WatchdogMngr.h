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
/// @file	WatchdogMngr.h
/// @author	Marius Negreanu.
/// @date	Jul 21, 2009
/// @brief	.
/// $Id: WatchdogMngr.h,v 1.14.18.1 2013/05/15 19:19:17 owlman Exp $
//////////////////////////////////////////////////////////////////////////////


#ifndef _WATCDOG_MNGR_H_
#define _WATCDOG_MNGR_H_

#include "Watchdog.h"
#include "WatchdogAction.h"

#include "DevWatchdog.h"
#include "Max6371Watchdog.h"

#define __MAX_WATCHDOGS 8
#define WTD_PIDLIST_ENTRIES	16	/*maximum pidfiles in the pidlist*/

#ifndef _max_
#define _max_(a,b) ((a) > (b) ? (a) : (b))
#endif

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
class CWatchdogMngr {
public:
	CWatchdogMngr()
	{
		reset() ;
	}
	~CWatchdogMngr() {}

public:
	int	LoadPidList();
	int	PidMissing() ;

	int	IsCfgEnabled () {return m_status&CFG_ENABLED; }
	int	LoadConfig(); 

	int	Open(const char* pidListFile) ;

	void	Sleep() ;
	void	OnQuit() ;
	static enum CWatchdog::Type GetType() ;

	int	Add(CWatchdog* p_wdg, int p_type) ;
	void	Ping() ;
	void	SetPingDivisor(int d)
	{
		if ( d <= 0 ) return ;
		m_nPingDivisor = d ;
	}
	void	Close() ;
	void	UseWatchdog(int p_nType) { m_nCurrentWtd = p_nType ; }
	inline void RunActions( )
	{
		m_action.WatchFreeMemory( m_nMemThreshold );
		m_action.WatchFreeTmp(    m_nTmpThreshold );
	}

protected:
	enum {
		FW_CFG_MODIFIED=0x01,
		CFG_ENABLED=0x02
	} ;
	void reset() ;

protected:
	bool	checkDelPidFiles(const char** p_pszNames, char* p_szMissNames = NULL );
	void	clearPidList() ;
	int	addFileWatch( const char* ) ;
	static int qsortcmp(const void *p1, const void *p2) ;

protected:
	char*		m_ppPidList[WTD_PIDLIST_ENTRIES];
	CWatchdog*	m_wdg[__MAX_WATCHDOGS];
	CWatchdogAction	m_action ;
	size_t		m_nPidListLength ;	///< Elements in PidList
	int		m_fd;			///< Inotify_init
	int		m_wd;			///< Add_watch
	unsigned	m_status ;
	uint8_t		m_nCurrentWtd ;
	uint8_t		m_nInsertPos ;
	long		m_lPingInterval ;
	uint8_t		m_nPingDivisor ;
	const char*	m_pPidListFile ;
	int		m_nMemThreshold ;
	int		m_nTmpThreshold ;
	struct timeval	m_nLastPingTime ;
	int		m_nTimeout;

};

inline int CWatchdogMngr::qsortcmp(const void *p1, const void *p2)
{
	if ( ( p1==NULL || *(char * const *)p1==NULL ) && ( p2 && *(char * const *)p2 ))
		return -1;
	if ( (p1 && *(char * const *)p1) && ( p2==NULL || *(char * const *)p2==NULL ) )
		return 1 ;

	return strcmp(* (char * const *) p1, * (char * const *) p2);
}

#endif	// _WATCDOG_MNGR_H_
