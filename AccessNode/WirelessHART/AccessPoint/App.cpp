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

// $Id: App.cpp,v 1.12.30.1 2013/05/15 19:19:17 owlman Exp $

//////////////////////////////////////////////////////////////////////////////
/// @author	Marius Negreanu
/// @date	Thu Dec 18 13:40:22 2008 UTC
//////////////////////////////////////////////////////////////////////////////

#include "Shared/Utils.h"
#include "Shared/SignalsMgr.h"
#include "App.h"
#include "EventLoop.h"



CWHAccessPointApp::CWHAccessPointApp()
: CApp( "whaccesspoint" )
{
	;
}


CWHAccessPointApp::~CWHAccessPointApp()
{
	;
}


int CWHAccessPointApp::Init()
{
	CSignalsMgr::Install(SIGUSR1);

	if( ! CApp::Init(NIVIS_TMP"whaccesspoint.log") )
	{	ERR( "App.Init() failed.");
		return false ;
	}

	if( ! m_oCfg.Init() )
	{	ERR( "App.Init(): Config.Init failed");
		return false ;
	}
	return true ;
}


void CWHAccessPointApp::Run()
{
	CEventLoop oLoop(m_oCfg) ;
	if( ! oLoop.Start() )
	{
		return ;
	}
	while( ! CApp::IsStop() )
	{
		TouchPidFile( CApp::m_szAppPidFile );
		if ( CSignalsMgr::IsRaised(SIGUSR1) )
		{
			LOG("ReStarting");
			oLoop.Stop();
			CSignalsMgr::Reset(SIGUSR1);
			if( !m_oCfg.Init() )
				ERR( "App.Run(): Config.Init failed");
			oLoop.Start();
		}
		oLoop.Run() ;
	}
}

