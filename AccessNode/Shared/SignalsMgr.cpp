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

/********************************************************************
	file	:	SignalsMgr.cpp
	created	:	2004/10/25 11:53
	author	:	Claudiu Hobeanu
	
	purpose:	implementation of the CSignalsMgr class.
*********************************************************************/

#include "SignalsMgr.h"

CSignalsMgr::SigInfo	CSignalsMgr::m_pSigVector[MAX_SIG_NUM];




//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSignalsMgr::CSignalsMgr()
{
		
}


CSignalsMgr::~CSignalsMgr()
{
	
}

int CSignalsMgr::Install(int nSigNum, sighandler_t p_pSigHandler )
{
	if(::signal( nSigNum, GenericHandler ) == SIG_ERR)
	{	return 0;
	}
	m_pSigVector[nSigNum].m_nStatus = INSTALLED;
	m_pSigVector[nSigNum].m_nRaised = 0;
	m_pSigVector[nSigNum].m_pSigHandler = p_pSigHandler;

	return 1;
}

int CSignalsMgr::Ignore(int nSigNum)
{
	if(::signal( nSigNum, SIG_IGN ) == SIG_ERR)
	{	return 0;
	}
	m_pSigVector[nSigNum].m_nStatus = IGNORED;
	m_pSigVector[nSigNum].m_nRaised = 0;
	m_pSigVector[nSigNum].m_pSigHandler = NULL;

	return 1;
}

int CSignalsMgr::Uninstall(int nSigNum)
{
	if(::signal( nSigNum, SIG_DFL ) == SIG_ERR)
	{	return 0;
	}
	m_pSigVector[nSigNum].m_nStatus = DEFAULT;
	m_pSigVector[nSigNum].m_nRaised = 0;
	m_pSigVector[nSigNum].m_pSigHandler = NULL;

	return 1;
}


void CSignalsMgr::GenericHandler(int nSigNum) 
{
	Install(nSigNum,m_pSigVector[nSigNum].m_pSigHandler);
	if (m_pSigVector[nSigNum].m_pSigHandler) 
	{	m_pSigVector[nSigNum].m_pSigHandler(nSigNum);
	}
	Raise(nSigNum);
}
