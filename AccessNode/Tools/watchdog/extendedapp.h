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

#ifndef _EXTENDED_APP_
#define _EXTENDED_APP_

#include "Shared/app.h"
//////////////////////////////////////////////////////////////////////////////
class CExtendedApp : public CApp {
public:
	CExtendedApp( const char * p_szAppName ) : CApp( p_szAppName ), m_bRunning(false) {}
//	~CExtendedApp() {}
	virtual void OnAlarm(int) {}
	virtual void OnSigInt(int) {}
protected:
	bool m_bRunning ;
} ;
#endif	// _EXTENDED_APP_
