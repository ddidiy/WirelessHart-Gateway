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

// CmdLineArgs.h: interface for the CCmdLineArgs class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CMDLINEARGS_H__84607EEA_B9F2_46BC_A4A9_3A3F0C877891__INCLUDED_)
#define AFX_CMDLINEARGS_H__84607EEA_B9F2_46BC_A4A9_3A3F0C877891__INCLUDED_

#include <stdlib.h>

#include "../Shared/Common.h"

/// @ingroup libshared
class CCmdLineArgs  
{
public:
	CCmdLineArgs();
	virtual ~CCmdLineArgs();

public:
	int GetArgNetAddress( const char *p_szName,  net_address* p_pNetAddr );
	int GetArgInt( const char* p_szName, int* p_pValue );
	const char* GetArg( const char* p_szName );
	int Init( int argc, char** argv );

private:
	int		m_argc;
	char**	m_argv;
};

#endif // !defined(AFX_CMDLINEARGS_H__84607EEA_B9F2_46BC_A4A9_3A3F0C877891__INCLUDED_)
