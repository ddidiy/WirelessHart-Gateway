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

#ifndef _WTD_APP_H_
#define _WTD_APP_H_

#include "extendedapp.h"
#include "Argv.h"
#include "Shared/h.h"
#include "WatchdogMngr.h"
#include "SignalManager.h"
#include "WatchdogAction.h"
#include "TCPLogSync.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
class CWtdgApp : public CExtendedApp {
public:
	CWtdgApp( Argv& p_arg)
		: CExtendedApp( "SW_WTD" )
		, arg(p_arg)
		, m_keepAlive(false)
		, m_bWatchMem(false)
		, m_bReloadCfg(false)
	{
	}
//	~CWtdgApp() {}
	int Init();
	int Run() ;
	void Close(bool stopwd=true);
	static int Usage() ;
	virtual void OnSigAlrm(int signum)
	{
		LOG("OnAlarm");
		m_oWdgMngr.OnQuit();
	}
	virtual void OnSigInt(int signum)
	{
		m_bRunning=false;
	}
	virtual void OnSigHup(int signum)
	{
		m_bReloadCfg = true ;
	}

private:
	Argv&          arg ;
	CWatchdogMngr  m_oWdgMngr ;
	const char   * pidListFile ;
	bool           m_keepAlive ;
	int            m_bWatchMem ; // int since CIniParser is too s to know abt bool
	bool           m_bReloadCfg ;
};
#endif	// _WTD_APP_H_
