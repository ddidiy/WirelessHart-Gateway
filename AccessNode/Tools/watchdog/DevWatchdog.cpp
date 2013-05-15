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

#include "Shared/h.h"	// log
#include "TCPLogSync.h"
#include "DevWatchdog.h"

//////////////////////////////////////////////////////////////////////////////
/// TODO:vezi daca se pot extrage infos despre ultimul reset(watchdog or powered-off
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
#define DEV_WATCHDOG "/dev/watchdog"
int CDevWatchdog::Open( const void* devName )
{
	if ( ! devName )
		devName=DEV_WATCHDOG ;

	m_nDevFd = open((const char*)devName, O_RDWR ) ;
	if ( -1 == m_nDevFd )
	{
		PERR( "Unable to open device file[%s]", (const char*)devName ) ;
		return false ;
	}
	struct watchdog_info ident ;
	ioctl(m_nDevFd, WDIOC_GETSUPPORT, &ident);

	if ( ! (ident.options & (WDIOF_KEEPALIVEPING|WDIOF_SETTIMEOUT) )
	   )
	{
		ERR( "features missing: KEEPALIVEPING and SETTIMEOUT");
		systemf_to( 20, "log2flash 'WTD: ERR: features missing: KEEPALIVEPING and SETTIMEOUT'&");
		close(m_nDevFd);
		m_nDevFd = -1 ;
		return false ;
	}
	ioctl(m_nDevFd, WDIOC_GETTIMEOUT, &m_lPingInterval);

	LOG("Found [%s] with %lu seconds timeout", ident.identity, m_lPingInterval);
	systemf_to( 20,"log2flash 'WTD: Found [%s] with %lu seconds timeout'&", ident.identity, m_lPingInterval);
	Ping() ;
	return true ;
}
//////////////////////////////////////////////////////////////////////////////
/// This closes and properly disables the watchdog
//////////////////////////////////////////////////////////////////////////////
int CDevWatchdog::Close()
{
	LOG("Stop Dev watchdog.");
	systemf_to( 20, "log2flash 'WTD: Stop Dev watchdog.'&");
	if ( m_nDevFd <= 0 ) return m_nDevFd ;

	int rv = write( m_nDevFd, "V", 1);
	if ( rv != 1 )
	{
		static int p1; if (!p1) { printf("WTD: FAIL:Unable to write stop char to /dev/watchdog[%s]", strerror(errno));p1=1; }
		FWARN("FAIL:Unable to write stop char to /dev/watchdog[%s]", strerror(errno));
		systemf_to( 20, "log2flash 'WTD: FAIL:Unable to write stop char to /dev/watchdog[%s]'&", strerror(errno));
	}
	rv = close(m_nDevFd);
	if ( rv != 0 )
	{
		systemf_to( 20, "log2flash 'WTD: FAIL:Unable to close /dev/watchdog[%s]'&", strerror(errno));
	}
	m_nDevFd = -1 ;
	return rv ;
}
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
int CDevWatchdog::Ping()
{
	int dummy, rv;
	if ( m_nDevFd <= 0 ) return m_nDevFd ;

	rv = ioctl(m_nDevFd, WDIOC_KEEPALIVE, &dummy);
	if ( -1 == rv )
	{
		static int p2; if (!p2) { printf("WTD: Unable to WDIOC_KEEPALIVE file descriptor"); p2=1; }
		throw CWatchdog::exception::exception("Unable to WDIOC_KEEPALIVE file descriptor") ;
	}
	//LOG_INFO(szNow()<<":Ping"<<"\n");

	return rv ;
}
