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

#include <sys/ioctl.h>
#include <linux/watchdog.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>	// errno
#include <string.h>	// strerror,strcmp

#include "Shared/h.h"	// log and tihs
#include "Max6371Watchdog.h"


#define MAX6371_PIN_ADDR	0xF0000800
#define MAX6371_PING_INTERVAL	55/*seconds*/

#define MAX6371_CLEAR_BIT	0x00000124
#define MAX6371_SET_BIT		0x00000134

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
int CMax6371Watchdog::Open( const void * p_uiPinAddress )
{
	m_uiWtdAddr = MAX6371_PIN_ADDR ;
	if ( p_uiPinAddress )
		m_uiWtdAddr = atoi((const char*)p_uiPinAddress) ;

	m_lPingInterval = MAX6371_PING_INTERVAL ;
	return true ;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
int CMax6371Watchdog::Close()
{
	return true ;
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
int CMax6371Watchdog::Ping()
{
	m_oDevMem.WriteInt( m_uiWtdAddr, MAX6371_CLEAR_BIT ) ;
	m_oDevMem.WriteInt( m_uiWtdAddr, MAX6371_SET_BIT ) ;
	return true ;
}


