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

// SimpleStream.cpp: implementation of the CSimpleStream class.
//
//////////////////////////////////////////////////////////////////////

#include "SimpleStream.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSimpleStream::CSimpleStream(	char p_cDelim,  
								char p_cComm, 
								unsigned int p_nSize
							)
{
	m_nSize = p_nSize;
	m_szStream = NULL;
	if (m_nSize) 
	{	m_szStream = new char[m_nSize];	
		m_szStream[0] = 0;
	}
	m_nCrtPos = 0;

	m_cDelim	= p_cDelim;
	m_cComm		= p_cComm;
}

CSimpleStream::~CSimpleStream()
{
	Release(); 
}

void CSimpleStream::Release()
{
	delete m_szStream;
	m_nSize = 0;
	m_szStream = NULL;
	m_nCrtPos = 0;
}

bool CSimpleStream::IsEmpty()
{
	if (!m_szStream) 
	{	return true;
	}

	int i = 0;
	for(; m_szStream[i] && isspace(m_szStream[i]) ; i++ )
	;
	
	return m_szStream[i] == m_cComm || !m_szStream[i];
}

bool CSimpleStream::GetInt (int* p_pInt)
{
	int nRead= 0;
	if( sscanf( m_szStream + m_nCrtPos, "%i%n", p_pInt, &nRead ) <= 0 )
	{	return false;
	}
	m_nCrtPos += nRead;	
	SkipDelim();
	return true;
}

bool CSimpleStream::GetHexInt (int* p_pInt)
{
	int nRead= 0;
	if( sscanf( m_szStream + m_nCrtPos, "%x%n", p_pInt, &nRead ) <= 0 )
	{	return false;
	}
	m_nCrtPos += nRead;	
	SkipDelim();
	return true;	
}

void CSimpleStream::SkipDelim()
{
	for (;;) 
	{
		if (!m_szStream[m_nCrtPos]) 
		{	return;
		}

		if (m_szStream[m_nCrtPos++] == m_cDelim) 
		{	return;
		}		
	}
}

void CSimpleStream::Load(const char *p_szBuff)
{
	int nLen = strlen(p_szBuff) + 1;
	m_nCrtPos = 0;
	if (nLen > m_nSize) 
	{	Release();
		m_nSize = nLen;
		m_szStream = new char[m_nSize];		
	}
	strcpy(m_szStream,p_szBuff);
}
