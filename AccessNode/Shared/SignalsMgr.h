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
	file	:	SignalsMgr.h
	created	:	2004/10/25 11:53
	author	:	Claudiu Hobeanu

	purpose: interface for the CSignalsMgr class.
*********************************************************************/



#if !defined(AFX_SIGNALSMGR_H__A6A8B4EF_8905_4AE7_B641_9496EECDF4E7__INCLUDED_)
#define AFX_SIGNALSMGR_H__A6A8B4EF_8905_4AE7_B641_9496EECDF4E7__INCLUDED_

/// @addtogroup libshared
/// @{

#include <signal.h>
#include <stdlib.h>

#define MAX_SIG_NUM		32

typedef void (*sighandler_t)(int);



class CSignalsMgr
{
public:
	//DEFAULT must be 0 -> m_pSigVector.m_nStatus to be set to DEFAULT at start
	enum StatusType { DEFAULT = 0, INSTALLED, IGNORED };
	struct SigInfo
	{
		int			m_nRaised;
		int			m_nStatus;
		sighandler_t	m_pSigHandler;
	};

public:
	CSignalsMgr();
	virtual ~CSignalsMgr();

public:
	static int Install( int nSigNum, sighandler_t p_pSigHandler = NULL );
	static int Uninstall( int nSigNum );
	static int Ignore( int nSigNum );

	static int	IsRaised( int nSigNum ) { return m_pSigVector[nSigNum].m_nRaised; }
	static void Raise( int nSigNum ) { m_pSigVector[nSigNum].m_nRaised = 1; }
	static void Reset( int nSigNum ) { m_pSigVector[nSigNum].m_nRaised = 0; }


private:
	static void GenericHandler(int nSigNum) ;
	static SigInfo	m_pSigVector[MAX_SIG_NUM];
};

/// @}
#endif // !defined(AFX_SIGNALSMGR_H__A6A8B4EF_8905_4AE7_B641_9496EECDF4E7__INCLUDED_)
