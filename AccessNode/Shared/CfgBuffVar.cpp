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

// CfgBuffVar.cpp: implementation of the CCfgBuffVar class.
//
//////////////////////////////////////////////////////////////////////

#include "CfgBuffVar.h"
#include "Common.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCfgBuffVar::CCfgBuffVar()
{
	m_szFile = NULL;
	m_szGroup = NULL;
	m_szVar = NULL;

	m_szValue[0] = 0;
}

CCfgBuffVar::~CCfgBuffVar()
{
	Close();
}

void CCfgBuffVar::Init(const char *p_szFile, const char *p_szGroup, const char *p_szVar)
{
	Close();

	m_szFile = strdup(p_szFile);
	m_szGroup = strdup(p_szGroup);
	m_szVar = strdup(p_szVar);

	strcpy(m_szValue,"###invalid###");
}

void CCfgBuffVar::Close()
{
	if (m_szFile)
	{
		free(m_szFile);
		m_szFile = NULL;
	}

	if (m_szGroup)
	{
		free(m_szGroup);
		m_szGroup = NULL;
	}

	if (m_szVar)
	{
		free(m_szVar);
		m_szVar = NULL;
	}

	m_szValue[0] = 0;
}

void CCfgBuffVar::SetVar(const char *p_szValue)
{
	if (strcmp(p_szValue,m_szValue) == 0)
	{	return;
	}

	LOG("CCfgBuffVar::SetVar: f=%s, g=%s var=%s val= %s -> %s", m_szFile, m_szGroup,m_szVar, 
				m_szValue, p_szValue);

	strncpy(m_szValue, p_szValue, sizeof(m_szValue) - 1);
	m_szValue[sizeof(m_szValue) - 1] = 0; // just for safety

	ExportVariable(m_szFile,m_szGroup,m_szVar,m_szValue);
}
