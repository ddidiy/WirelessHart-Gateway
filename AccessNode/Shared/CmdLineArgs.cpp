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

// CmdLineArgs.cpp: implementation of the CCmdLineArgs class.
//
//////////////////////////////////////////////////////////////////////

#include "CmdLineArgs.h"

#include <string.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCmdLineArgs::CCmdLineArgs()
{
	m_argc = 0;
	m_argv = NULL;
}

CCmdLineArgs::~CCmdLineArgs()
{

}

int CCmdLineArgs::Init(int argc, char **argv)
{
	if (argc<1 || !argv)
	{	return 0;	
	}

	m_argc = argc;
	m_argv = argv;
	return 1;
}

const char* CCmdLineArgs::GetArg(const char *p_szName)
{
	for(int i = 1; i < m_argc-1; i++)
	{	if (strcasecmp(p_szName, m_argv[i]) == 0)
		{	return m_argv[i+1];
		}
	}

	return NULL;
}

int CCmdLineArgs::GetArgInt(const char *p_szName, int *p_pValue)
{
	const char* szValue = GetArg(p_szName);

	if (!szValue)
	{	return 0;
	}
	
	return sscanf( szValue, "%i", p_pValue ) > 0;	
}

int CCmdLineArgs::GetArgNetAddress(const char */*p_szName*/, net_address */*p_pNetAddr*/)
{
	return 0;
}
