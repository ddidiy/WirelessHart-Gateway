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

#ifndef _MAX6371_WATCHDOG_H_
#define _MAX6371_WATCHDOG_H_

#include "Watchdog.h"
#include "Shared/DevMem.h"

//////////////////////////////////////////////////////////////////////////////
/// MAX6371 external watchdog class.
//////////////////////////////////////////////////////////////////////////////
class CMax6371Watchdog : public CWatchdog {
public:
	CMax6371Watchdog() {}
	~CMax6371Watchdog() {}
	int Open( const void* p_uiPinAddr ) ;
	int Close();
	int Ping();

private:
	CDevMem m_oDevMem ;
	unsigned int m_uiWtdAddr ;
};
#endif	// _MAX6371_WATCHDOG_H_
