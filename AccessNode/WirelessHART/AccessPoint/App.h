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

#ifndef _AP_APP_H_
#define _AP_APP_H_

// $Id: App.h,v 1.6.30.1 2013/05/15 19:19:17 owlman Exp $

//////////////////////////////////////////////////////////////////////////////
/// @author	Marius Negreanu
/// @date	Thu Dec 18 13:40:22 2008 UTC
/// @brief  Application Class skeleton.
//////////////////////////////////////////////////////////////////////////////

#include "Shared/app.h"
#include "Cfg.h"

//////////////////////////////////////////////////////////////////////////////
/// @class CWHAccessPointApp
/// @ingroup AccessPoint
//////////////////////////////////////////////////////////////////////////////
class CWHAccessPointApp : public CApp
{
public:
	CWHAccessPointApp() ;
	~CWHAccessPointApp() ;
public:
	int Init() ;
	void Run() ;
	CWHAccessPointCfg	m_oCfg ;
};

#endif	// _AP_APP_H_
