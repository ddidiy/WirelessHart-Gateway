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

#ifndef _SIGNAL_MANAGER_H_
#define _SIGNAL_MANAGER_H_

#include <signal.h>
class CExtendedApp ;

#ifndef _NSIG
#define  _NSIG (30)
#endif

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
class CSignalManager {
public:
	static CSignalManager* GetInstance()
	{
		if ( ! g_sigHandler )
			g_sigHandler = new CSignalManager() ;

		return g_sigHandler ;
	}
	static void HandleSignal(int signum)
	{
		g_sigHandler->handle( signum );
	}
	void Register(int signum, CExtendedApp* obj, void (CExtendedApp::*method)(int)  )
	{
		sigtbl[signum].obj=obj;
		sigtbl[signum].HandleSignal=method;
		signal( signum, CSignalManager::HandleSignal ) ;
	}
protected:
	void handle(int signum )
	{
		((sigtbl[signum].obj)->*sigtbl[signum].HandleSignal)(signum) ;
	}
	struct sigHandlers {
		CExtendedApp *obj ;
		void (CExtendedApp::*HandleSignal)(int) ;
		sigHandlers() : obj(0), HandleSignal(0) {}
	} sigtbl[_NSIG] ;
	static CSignalManager *g_sigHandler;
};

#endif	// _SIGNAL_MANAGER_H_
