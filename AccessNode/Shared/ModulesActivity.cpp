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

// ModulesActivity.cpp: implementation of the CModulesActivity class.
//
//////////////////////////////////////////////////////////////////////

#include "ModulesActivity.h"
#include "Common.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CModulesActivity::CModulesActivity()
{
	m_oEasyBuffer.Assure(MODULE_ACT_LAST);
	memset((char*)m_oEasyBuffer, '0', m_oEasyBuffer.GetSize());
	m_nFd = -1;
}

CModulesActivity::~CModulesActivity()
{
	Close();
}

int CModulesActivity::Open(const char *p_szFile)
{
	if( ( m_nFd = open( p_szFile, O_RDWR ) ) < 0 )
    {
        LOG_ERR( "CModulesActivity::Open - cannot open: %s - recreate;", p_szFile );

		if( ( m_nFd = open( p_szFile, O_CREAT | O_RDWR, S_IRWXU ) ) < 0 )
    	{
        	LOG_ERR( "CModulesActivity::Open - cannot create: %s;", p_szFile );
        	return 0;
    	}

		write( m_nFd, (char*)m_oEasyBuffer, m_oEasyBuffer.GetSize() ) ;
		LOG( "CModulesActivity::Open %s created, %d bytes", p_szFile, m_oEasyBuffer.GetSize());
    }

	LOG( "CModulesActivity::Open(%s)", p_szFile );
	return 1;		
}


void CModulesActivity::SetAct(int p_nModuleId, char p_cAct)
{
	if( lseek( m_nFd, p_nModuleId, SEEK_SET ) < 0 )
	{
		LOG_ERR( "CModulesActivity::SetAct - cannot go to off (%d)",p_nModuleId);
		return;
	}

	flock( m_nFd, LOCK_EX );

	if(write( m_nFd, &p_cAct, 1 ) <= 0 )
	{
		LOG_ERR( "CModulesActivity::SetAct - cannot write at (%d)",p_nModuleId);
	}
	else
	{
		m_oEasyBuffer[p_nModuleId] = p_cAct;
	}
	flock( m_nFd, LOCK_UN );	
}

char CModulesActivity::GetAct(int p_nModuleId)
{
	char cAct = 0;
	if( lseek( m_nFd, p_nModuleId, SEEK_SET ) < 0 )
	{
		LOG_ERR( "CModulesActivity::GetAct - cannot go to off (%d)",p_nModuleId);
		return cAct;
	}

	flock( m_nFd, LOCK_EX );

	if(read( m_nFd, &cAct, 1 ) <= 0 )
	{
		LOG_ERR( "CModulesActivity::GetAct - cannot read at (%d)",p_nModuleId);
	}
	else
	{
		m_oEasyBuffer[p_nModuleId] = cAct;
	}
	flock( m_nFd, LOCK_UN );	
	return cAct;
}


void CModulesActivity::Close()
{
	close(m_nFd);
}

int CModulesActivity::LoadAct()
{
	int nRet = 0;
	if( lseek( m_nFd, 0, SEEK_SET ) < 0 )
	{
		LOG_ERR( "CModulesActivity::LoadAct - cannot go to off (%d)",0);
		return nRet;
	}

	flock( m_nFd, LOCK_EX );

	if(read( m_nFd, (char*)m_oEasyBuffer, m_oEasyBuffer.GetSize() ) <= 0 )
		LOG_ERR( "CModulesActivity::LoadAct - cannot read at (%d)", 0 );	
	else nRet = 1;

	flock( m_nFd, LOCK_UN );	
	return nRet;
}

char CModulesActivity::GetActCached(int p_nModuleId)
{
	if (p_nModuleId >= m_oEasyBuffer.GetSize())
	{	return 0;
	}

	return m_oEasyBuffer[p_nModuleId];	
}


/// Description
///   Returns a boolean value indicating whether any of the AN modules are
///   active
///
/// Return Value
///   0 - All modules are inactive
///   1 - At least one module is active
///
int CModulesActivity::GetAnActivity()
{
    if( ! LoadAct() )
      return 1;

    int nIdVar;

    //Scan running status of all modules
    for( nIdVar = MODULE_ACT_FIRST; nIdVar < MODULE_ACT_LAST; nIdVar++ )
    {
        if( GetActCached(nIdVar) == '1' )
            return 1;
    }

    return 0;
}
