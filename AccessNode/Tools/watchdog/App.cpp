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

#include "App.h"
#include "Shared/IniParser.h"


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
int CWtdgApp::Init()
{
	if ( ! CApp::Init(NIVIS_TMP"watchdog.log", 64 * 1024) )
		return false ;

	arg.parse();

	// Decide what type of watchdog we're using
	CWatchdog::Type wt ;
	try
	{
		wt = CWatchdogMngr::GetType() ;
	}
	catch (std::runtime_error &e)
	{
		if ( e.what()[0] )
			PERR("%s",e.what());
		return false ;
	}

	// Use the command line supplied pid-list-file
	if ( ! (pidListFile=(char*)arg.isSet('p')) )
		pidListFile=FW_CFG_FILE;

	m_oWdgMngr.Open(pidListFile) ; // init_inotify
	m_oWdgMngr.LoadConfig();
	m_oWdgMngr.LoadPidList();

	CWatchdog* wdg ;
	// Create a new watchdog object
	if ( wt == CWatchdog::WTD_INTERNAL || wt == CWatchdog::WTD_ALL)
	{
		wdg = new CDevWatchdog ;
		if ( m_oWdgMngr.IsCfgEnabled() && ! ((CDevWatchdog*)wdg)->Open( (char*)arg.isSet('d')) )
			return false ;

		m_oWdgMngr.Add( wdg, CWatchdog::WTD_INTERNAL );
		m_oWdgMngr.UseWatchdog(CWatchdog::WTD_INTERNAL);
	}
	if ( wt == CWatchdog::WTD_MAX6371 || wt == CWatchdog::WTD_ALL)
	{
		wdg = new CMax6371Watchdog ;
		if ( m_oWdgMngr.IsCfgEnabled() && ! ((CMax6371Watchdog*)wdg)->Open( (char*)arg.isSet('d')) )
			return false ;
		m_oWdgMngr.Add( wdg, CWatchdog::WTD_MAX6371 );
		m_oWdgMngr.UseWatchdog(CWatchdog::WTD_MAX6371) ;
	}
	if ( wt == CWatchdog::WTD_ALL )
	{
		// If using all watchdogs, then tell the watchdogmanager to loop through all
		m_oWdgMngr.UseWatchdog(CWatchdog::WTD_ALL) ;
	}


	do{
		CIniParser oIniParser ;
		if ( !oIniParser.Load(INI_FILE) )
		{
			ERR("unable to load "INI_FILE);
			break ;
		}

		if ( ! oIniParser.GetVar("GLOBAL","WATCH_DOG_WATCHMEM", &m_bWatchMem) )
		{
			m_bWatchMem = false ;
		}
	}while(0);

	// SIGINT handler
	CSignalManager::GetInstance()->Register( SIGINT,  static_cast<CExtendedApp*>(this), static_cast<void (CExtendedApp::*)(int)>(&CWtdgApp::OnSigInt) );
	CSignalManager::GetInstance()->Register( SIGTERM, static_cast<CExtendedApp*>(this), static_cast<void (CExtendedApp::*)(int)>(&CWtdgApp::OnSigInt) );
	CSignalManager::GetInstance()->Register( SIGHUP,  static_cast<CExtendedApp*>(this), static_cast<void (CExtendedApp::*)(int)>(&CWtdgApp::OnSigHup) );


	// If keep-alive parameter was supplied, arm an alarm after keep-alive-param seconds
	unsigned ka;
	if ( (ka=(unsigned)arg.isSet('k')) )
	{
		m_keepAlive = true ;

		ka=atoi((char*)ka) ;
		systemf_to( 20, "log2flash 'WTD: Keepalive %dsec'&",ka);

		CSignalManager::GetInstance()->Register( SIGALRM, static_cast<CExtendedApp*>(this), static_cast<void (CExtendedApp::*)(int)>(&CWtdgApp::OnSigAlrm) );
		alarm(ka) ;
	}

	return true ;
}
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
int CWtdgApp::Run()
{
	m_bRunning=true ;
	m_oWdgMngr.Ping();
	for(;m_bRunning;)
	{
		try {
			
			m_oWdgMngr.Sleep();
			m_oWdgMngr.Ping();
			if (m_bReloadCfg)
			{
				Close(true);
				Init();
				m_bReloadCfg = false;
				m_bRunning = true ;
			}

			if (m_keepAlive || !m_oWdgMngr.IsCfgEnabled() )
			{
				continue ;
			}

			if ( m_oWdgMngr.PidMissing() )
			{
				m_oWdgMngr.OnQuit() ;
				systemf_to( 20, "log2flash 'WTD: Pids missing'&");
				return false ;
			}
			if ( m_bWatchMem )
				m_oWdgMngr.RunActions( ) ;
		}
		catch (std::runtime_error &e)
		{
			if ( e.what()[0] ) {
				PERR( "%s",e.what());
				systemf_to( 20, "log2flash 'WTD: %s'&",e.what() );
			}
			systemf_to( 20, "log2flash 'WTD: exceptions'&" );
			return false ;
		}
	}
	return true ;
}
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void CWtdgApp::Close(bool stopwd/*=true*/)
{
	m_bRunning = false ;
	if ( stopwd )
	{
		systemf_to( 20, "log2flash 'WTD: Hardware watchdog STOPPED. The board will NOT reboot' &");
		m_oWdgMngr.Close() ;
	}
	else
		systemf_to( 20, "log2flash 'WTD: Hardware watchdog RUNNING. The board will reboot.' &");
	//should close/delete logger
	return CApp::Close();
}
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
int CWtdgApp::Usage()
{
	puts( "Usage: watchdog version: $Date: 2013/05/15 19:19:17 $:$Revision: 1.39.16.1 $\n"
		"\t-d|--device=<device>\tWhen using internal wtd, this may point to /dev/watchdog\n"
		"\t\t\t\twhen using external watchdog, this may point to device address.\n"
		"\t-p|--pidlist=<file>\tPidlist file.(default="FW_CFG_FILE"\n"
		"\t-t|--timeout=<seconds>\tPid Recheck period\n"
		"\t-k|--keep-alive=<seconds>\tKeep the device up for <seconds>.\n"
		"\n"
		"\tThe watchdog will be closed cleanly on SIGINT, not on SIGKILL\n"
	);
	return EXIT_FAILURE ;
}
