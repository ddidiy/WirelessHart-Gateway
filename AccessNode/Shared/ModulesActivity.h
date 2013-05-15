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

// ModulesActivity.h: interface for the CModulesActivity class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MODULESACTIVITY_H__C1BD0601_1459_415D_8395_E5109AC62F33__INCLUDED_)
#define AFX_MODULESACTIVITY_H__C1BD0601_1459_415D_8395_E5109AC62F33__INCLUDED_

#include "EasyBuffer.h"
#include "AnPaths.h"
/// @addtogroup libshared
/// @{

enum 
{
	MODULE_ACT_FIRST = 0,
	MODULE_ACT_DNLOCAL = 3,
	MODULE_ACT_MESH,
    MODULE_ACT_RS232_0,
    MODULE_ACT_RS232_1,    
    MODULE_ACT_SPI,

    MODULE_ACT_UI,
    MODULE_ACT_RA,
    MODULE_ACT_SH,
    MODULE_ACT_HIST,
    MODULE_ACT_CC,
	MODULE_ACT_FAKE,
	MODULE_ACT_DEBUGGER,
    MODULE_ACT_RF,

	MODULE_ACT_DIVER,
	MODULE_ACT_TUNNEL_ROOT,
	MODULE_ACT_TUNNEL_LEAF,
	MODULE_ACT_FLOW_MGR_ROOT,	
	MODULE_ACT_FLOW_MGR_LEAF,	
	MODULE_ACT_LAST
};


class CModulesActivity  
{
public:
	CModulesActivity();
	virtual ~CModulesActivity();

public:
	int GetAnActivity();
	int		Open (const char* p_szFile = NIVIS_TMP"modules_activity.txt");
	void	Close();

	char	GetActCached (int p_nModuleId);
	int		LoadAct();

	char	GetAct (int p_nModuleId);
	void	SetAct (int p_nModuleId, char p_cAct);

private:
	CEasyBuffer<char>	m_oEasyBuffer;
	int m_nFd;	
};

/// @}
#endif // !defined(AFX_MODULESACTIVITY_H__C1BD0601_1459_415D_8395_E5109AC62F33__INCLUDED_)
