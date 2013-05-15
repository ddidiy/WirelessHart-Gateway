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

// BuffFile.cpp: implementation of the CBuffFile class.
//
//////////////////////////////////////////////////////////////////////

#include "BuffFile.h"




#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBuffFile::CBuffFile()
{
	m_pBuffer = NULL;
	m_nBuffPos = 0;

	m_nFd = -1;
	m_nCrtFilePos = 0;
}

CBuffFile::~CBuffFile()
{
	Close();
}

int CBuffFile::Open(const char *p_szFile, bool p_bSaveAll)
{
	Close();

	int nFlags = O_RDWR | O_CREAT;

	if (p_bSaveAll)
	{	nFlags |= O_TRUNC;	
	}

	m_nFd = open( p_szFile, nFlags, 0666 );
	
	if (m_nFd<0)
	{	
		LOG_ERR("Error at opening file %s", p_szFile);
		return 0;
	}

	m_bSaveAll		= p_bSaveAll;
	m_nCrtFilePos	= 0;

	m_pBuffer = new char[FLASH_WRITE_BUFF_SIZE];
	m_nBuffPos = 0;		

	return 1;
}

void CBuffFile::Close()
{

	if (m_nFd>=0)
	{	Flush();
		close(m_nFd);
		m_nFd = -1;
	}

	if (m_pBuffer)
	{	delete m_pBuffer;
		m_pBuffer = NULL;
	}

	m_nBuffPos = 0;
}


int CBuffFile::Flush()
{
	if (!m_pBuffer || m_nFd < 0 )
	{	LOG("CBuffFile::Flush (!m_pBuffer || m_nFd < 0)");
		return 0;
	}
	int ret = write(m_nFd, m_pBuffer, m_nBuffPos);

	if (ret < 0)
	{	LOG_ERR("CBuffFile::Flush write error");
		return 0;
	}

	m_nCrtFilePos += ret;
	m_nBuffPos = 0;

	return 1;
}

int CBuffFile::writeData(const char *p_pData, int p_nDataLen)
{
	if (!m_pBuffer || m_nFd < 0 )
	{	LOG("CBuffFile::Write (!m_pBuffer || m_nFd < 0)");
		return 0;
	}
	if (m_nBuffPos + p_nDataLen < FLASH_WRITE_BUFF_SIZE)
	{
		memcpy( m_pBuffer + m_nBuffPos, p_pData, p_nDataLen);	
		m_nBuffPos += p_nDataLen;

		return 1;
	}

	int nBuffLeft = FLASH_WRITE_BUFF_SIZE - m_nBuffPos;

	memcpy(m_pBuffer + m_nBuffPos, p_pData, nBuffLeft );
	m_nBuffPos += nBuffLeft;
	
	if(!Flush())
	{	return 0;
	}
	p_pData += nBuffLeft;
	p_nDataLen -= nBuffLeft;

	if (p_nDataLen < FLASH_WRITE_BUFF_SIZE)
	{	
		memcpy( m_pBuffer + m_nBuffPos, p_pData, p_nDataLen);	
		m_nBuffPos += p_nDataLen;
		return 1;
	}
	
	int ret = write(m_nFd, p_pData, p_nDataLen);

	if (ret < 0)
	{	LOG_ERR("CBuffFile::Write error");
		return 0;
	}

	m_nCrtFilePos += ret;

	return 1;
}

int CBuffFile::WriteAtEnd(const char *p_pData, int p_nDataLen)
{
	if (!m_pBuffer || m_nFd < 0 || !m_bSaveAll)
	{	LOG("CBuffFile::WriteAtEnd (!m_pBuffer || m_nFd < 0 || !m_bSaveAll)");
		return 0;
	}

	return writeData(p_pData,p_nDataLen);
}
int CBuffFile::WriteAtOffset(int p_nOffset, const char *p_pData, int p_nDataLen)
{
	if (!m_pBuffer || m_nFd < 0 || m_bSaveAll)
	{	LOG("CBuffFile::WriteAtOffset (!m_pBuffer || m_nFd < 0 || m_bSaveAll)");
		return 0;
	}

	if (p_nOffset < 0)
	{	
		LOG("CBuffFile::WriteSpare invalid offset %d", p_nOffset);
		return 0;
	}

	if (m_nCrtFilePos + m_nBuffPos == p_nOffset)
	{	
		return writeData(p_pData,p_nDataLen);
	}

	if (!Flush())
	{	return 0;
	}

	int ret = lseek( m_nFd, p_nOffset, SEEK_SET );

	if (ret < 0)
	{	LOG_ERR("CBuffFile::WriteSpare lseek error");
		return 0;
	}

	m_nCrtFilePos = ret;
	return writeData(p_pData,p_nDataLen);
}




