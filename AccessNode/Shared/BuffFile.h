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

// BuffFile.h: interface for the CBuffFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BUFFFILE_H__214556B2_DB3D_4357_9809_2F5BAE53AA25__INCLUDED_)
#define AFX_BUFFFILE_H__214556B2_DB3D_4357_9809_2F5BAE53AA25__INCLUDED_


#include "Common.h"
#define FLASH_WRITE_BUFF_SIZE	( 4 * 4096 )


class CBuffFile  
{
public:

	CBuffFile();
	virtual ~CBuffFile();

public:
	int Open(const char* p_szFile, bool p_bSaveAll );
	void Close();


public:
	int WriteAtEnd(const char* p_pData, int p_nDataLen);
	int WriteAtOffset(int p_nOffset, const char* p_pData, int p_nDataLen);
	
	int Flush();

protected:
	int writeData(const char* p_pData, int p_nDataLen );

private:
	char*		m_pBuffer;
	int			m_nBuffPos;

	int			m_nFd;
	bool		m_bSaveAll;

	int			m_nCrtFilePos;
};

#endif // !defined(AFX_BUFFFILE_H__214556B2_DB3D_4357_9809_2F5BAE53AA25__INCLUDED_)
