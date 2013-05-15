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

// PersistentInteger.cpp: implementation of the CPersistentInteger class.
//
//////////////////////////////////////////////////////////////////////

#include <string.h>
#include <stdio.h>

#include "Common.h"
#include "PersistentInteger.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPersistentInteger::CPersistentInteger(const char* p_szFile )
{
	m_szFile[0] = 0;
	SetStorage( p_szFile );
}

CPersistentInteger::~CPersistentInteger()
{

}

void CPersistentInteger::SetStorage(const char *p_szFile)
{
	if (!p_szFile) 
	{	return;
	}

	strcpy( m_szFile, p_szFile );
}

int CPersistentInteger::Save()
{
	FILE* f = fopen( m_szFile,"w");
	if (!f) 
	{	LOG_ERR("CPersistentInteger::Save: file %s", m_szFile);
		return 0;
	}
	
	int ret = fprintf(f,"%d", m_nValue ) > 0;
	if( !ret )
	{	LOG_ERR("CPersistentInteger::Save: writing in file %s ", m_szFile);
	}

	fclose(f);
	return ret;
}

int CPersistentInteger::Load()
{
	FILE* f = fopen( m_szFile,"r");
	if (!f) 
	{	LOG_ERR("CPersistentInteger::Load: file %s", m_szFile);
		return 0;
	}

	int nTmp =0;
	int ret = fscanf(f,"%d", &nTmp ) > 0;

	if (!ret) 
	{	LOG_ERR("CPersistentInteger::Load: reading from file %s", m_szFile);
	}
	else
	{	m_nValue = nTmp;
	}

	fclose(f);
	return ret;
}
