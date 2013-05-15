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

// FileLock.cpp: implementation of the CFileLock class.
//
//////////////////////////////////////////////////////////////////////

#include "FileLock.h"

#include "Common.h"

#include <sys/file.h> 
#include <stdio.h>

#include <sys/types.h>
#include <utime.h>


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFileLock::CFileLock(const char* p_szFile)
{
	m_nFd = -1;
	m_szFile = NULL;
	if (p_szFile)
	{	SetFile(p_szFile);
	}
}

CFileLock::~CFileLock()
{
	if (m_nFd != -1)
	{	close(m_nFd);
	}
	free(m_szFile);
}

void CFileLock::SetFile(const char *p_szFile)
{
	if (m_nFd != -1)
	{	close(m_nFd);
	}	
	free(m_szFile);

	m_nFd = open( p_szFile, O_RDWR | O_CREAT, 0666 );
	
	if (m_nFd < 0)
	{	LOG_ERR("FileLock: %s",p_szFile);
	}
	m_szFile = strdup(p_szFile);
}

void CFileLock::Lock()
{
	if (m_nFd<0)
	{	LOG("CFileLock::Lock: set file first");
	}
	if(flock( m_nFd, LOCK_EX ))
	{
		LOG_ERR("CFileLock::Lock: flock(LOCK_EX) file %s", m_szFile);
	}
}


bool CFileLock::TryLock()
{
	if (m_nFd<0)
	{	LOG("CFileLock::Lock: set file first");
		return false;
	}
	if(flock( m_nFd, LOCK_NB | LOCK_EX ))
	{
		LOG_ERR("CFileLock::Lock: flock(LOCK_NB|LOCK_EX) file %s", m_szFile);
		return false;
	}
	return true;
}

//return 0 - got lock
//			> 0 no of sec until lock get old
int CFileLock::ForceOldLock (int p_nAfterSec)
{
	if (TryLock())
	{	
		struct utimbuf timeBuff;

		timeBuff.actime = timeBuff.modtime = time(NULL);
		utime(m_szFile,&timeBuff);
		return 0;
	}

	struct stat buf;
	if (stat(m_szFile , &buf))
	{
		LOG_ERR("CFileLock::ForceOldLock: stat ");
		return p_nAfterSec;
	}
	
	int nCrtTime = time(NULL);
	if(  nCrtTime >= buf.st_mtime && nCrtTime - buf.st_mtime < p_nAfterSec)
	{
		int nLeft = p_nAfterSec - (nCrtTime - buf.st_mtime);
		LOG("CFileLock::ForceOldLock: lock old %ds left %ds", nCrtTime - buf.st_mtime, nLeft);
		return nLeft;
	}

	close(m_nFd);

	LOG("CFileLock::ForceOldLock: lock old %ds > %ds-> unlink(%s)", nCrtTime - buf.st_mtime, p_nAfterSec, m_szFile);

	if (unlink(m_szFile ) ) LOG_ERR("CFileLock::ForceOldLock: failed unlink(%s)", m_szFile);
	
	m_nFd = open( m_szFile, O_RDWR | O_CREAT, 0666 );

	if (m_nFd < 0)
	{	LOG_ERR("ForceOldLock: %s",m_szFile);
	}

	if (TryLock())
	{	return 0;
	}
	return p_nAfterSec;
}


void CFileLock::Unlock()
{
	if (m_nFd<0)
	{	LOG("CFileLock::Lock: set file first");
	}

	if(flock( m_nFd, LOCK_UN ))
	{
		LOG_ERR("CFileLock::Unlock: flock(LOCK_UN) file %s", m_szFile);
	}
}
