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


#include <string.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>

#include "Shared/Utils.h"
#include "methods/core/ConfigImpl.h"
#include "Isa100Impl.h"

#define CONF_OPC 			NIVIS_PROFILE"opc.ini"

int filter(const struct dirent * p_pDirent)
{	const char * p = p_pDirent->d_name;
	while( *p )
	{
		if( !isdigit( *p++ ))
			return 0;
	}
	return 1;
}


////////////////////////////////////////////////////////////////////////////////
/// @author Marcel Ionescu
/// @brief process lock detection
/// @retval true if process lock is detected
/// @note lock detection cannot be 100% reliable because the software watchdog deletes the lock files
////////////////////////////////////////////////////////////////////////////////
bool CIsa100Impl::isLocked( const char* p_szProcessName, unsigned p_unstaleLimit /*= 90*/ )
{
	char szPIDFile[ 256 ], *p;
	struct stat buf;

	if( *p_szProcessName == '(')
		++p_szProcessName;
	sprintf( szPIDFile, NIVIS_TMP"%s", p_szProcessName );
	if( ( p = strchr( szPIDFile, ')') ) )
		*p=0;
	strcat( szPIDFile, ".pid");
	if( stat( szPIDFile, &buf ))
	{	/// PID not found, process may be locked. since we are not sure, we report not locked
		/// should check the logfile now
		const char * pLogFile = "";
		if( !strcmp( p_szProcessName,  "backbone)"))	pLogFile = NIVIS_TMP"backbone.log";
		if( !strcmp( p_szProcessName,  "isa_gw)"))		pLogFile = NIVIS_TMP"isa_gw.log";
		if( !strcmp( p_szProcessName,  "SystemManager)"))pLogFile = NIVIS_TMP"sm.log";
		if( stat( szPIDFile, &buf ))
		{
			//LOG("DEBUG isLocked: no pidfile [%s], no logfile [%s]. Assume Running", szPIDFile, pLogFile );
			return false;	/// we have no info, assume Running
		}
		//LOG("isLocked: no pidfile [%s], use logfile [%s] %u-%u=%u, limit %u", szPIDFile, pLogFile, time( NULL ), buf.st_mtime, time( NULL ) - buf.st_mtime, 20*60);
		return (time( NULL ) - buf.st_mtime) > 20*60 ;
	}

	//LOG("DEBUG isLocked: pidfile [%s] %u-%u=%u, limit %u", szPIDFile, time( NULL ), buf.st_mtime, time( NULL ) - buf.st_mtime, p_unstaleLimit);
	return (time( NULL ) - buf.st_mtime) > (int)p_unstaleLimit ;
}


////////////////////////////////////////////////////////////////////////////////
/// @author Marcel Ionescu
/// @brief read processor usage for all processes processes
/// @brief also read total processor usage from /proc/status
/// @retval total CPU usage
////////////////////////////////////////////////////////////////////////////////
unsigned long long CIsa100Impl::getTotalJiffSnapshot( void )
{	char szFileBuf[ 1024 ];
	unsigned long long usr, nic, sys, idle, iowait, irq, softirq, steal, uTotal, uBusy;

	if(	(!FileRead( "/proc/stat", szFileBuf, sizeof( szFileBuf) ) )
	||	( sscanf(szFileBuf, "cpu %lld %lld %lld %lld %lld %lld %lld %lld", &usr, &nic, &sys, &idle, &iowait, &irq, &softirq, &steal) < 8) )
		return 0LL;

	uTotal = usr + nic + sys + idle + iowait + irq + softirq + steal;
	uBusy  = uTotal - idle - iowait;
	LOG("CPU busy %9llu total %9llu", uBusy, uTotal);
	return uTotal;
}

////////////////////////////////////////////////////////////////////////////////
/// @author Marcel Ionescu
/// @brief read processor usage for all processes processes
/// @retval total CPU usage
////////////////////////////////////////////////////////////////////////////////
bool CIsa100Impl::getProcessesSnapshot( void )
{
	struct dirent **namelist;
	int n = scandir("/proc/", &namelist, &filter, alphasort);
	LOG("getProcessesSnapshot scan => %d processes", n);

	if( n < 0 )
	{	LOG_ERR("ERROR getProcessesSnapshot scandir");
		return false;
	}
	char szName[ 256 ], szFileBuf[ 1024 ], szProcName[ 256 ];
	unsigned long long uUtime, uKTime, uVMSize;
	for( int i = 0; i < n; ++i )
	{
		sprintf( szName , "/proc/%s/stat", namelist[i]->d_name );
		if( FileRead( szName, szFileBuf, sizeof( szFileBuf) ) )
		{
			if( sscanf(szFileBuf, "%*d %s %*c %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %lld %lld %*d %*d %*d %*d %*d %*d %*d %lld",
				szProcName, &uUtime, &uKTime, &uVMSize ) < 4)
				return false;

			for( Processes::iterator it = m_systemStatus.processes.begin(); it != m_systemStatus.processes.end(); ++it )
			{
				if( !strcmp( szProcName, it->m_szName ) )
				{
					it->jiffies += uUtime + uKTime;
					it->m_ushMemKb = uVMSize/1024;
					LOG("PID %5s [%15s] VM %6llu user %7llu kern %7llu => %5llu",
						namelist[i]->d_name, szProcName, uVMSize, uUtime, uKTime, it->jiffies);
					/// We cannot really differentiate Locked/Running because wdt scripts erase pid files so we cannot stat them. Report running always
					strcpy( it->m_szStatus, /*isLocked( szProcName ) ? "Locked":*/ "Running");
				}
			}
		}
		free(namelist[i]);
	}
	free(namelist);
	return true;
}

unsigned long long CIsa100Impl::systemSnapshot( void )
{
	if( getProcessesSnapshot() )
		return getTotalJiffSnapshot();
	return 0LL;
}

bool CIsa100Impl::doProcessorStats( void )
{
	m_systemStatus.processes.clear();
	m_systemStatus.processes.push_back( ProcessStatus( "(scgi_svc)",		"cgi") );
	m_systemStatus.processes.push_back( ProcessStatus( "(MonitorHost)",		"MonitorHost" )  );
	m_systemStatus.processes.push_back( ProcessStatus( "(modbus_gw)",		"MODBUS" )  );
#if defined( RELEASE_ISA )
	m_systemStatus.processes.push_back( ProcessStatus( "(backbone)",		"Backbone" )  );
#elif defined( RELEASE_WHART )
	m_systemStatus.processes.push_back( ProcessStatus("(whaccesspoint)",		"AccessPoint"));
#endif
#if defined( RELEASE_ISA )
	m_systemStatus.processes.push_back( ProcessStatus( "(isa_gw)",			"Gateway" )  );
	m_systemStatus.processes.push_back( ProcessStatus( "(SystemManager)",		"SystemManager" )  );
#endif
#if defined( RELEASE_WHART )
	m_systemStatus.processes.push_back( ProcessStatus("(WHart_GW.o)",		"Gateway"));
	m_systemStatus.processes.push_back( ProcessStatus("(WHart_NM.o)",		"NetworkManager"));
#endif

	unsigned long long oldSysJiff = systemSnapshot( );
	if( !oldSysJiff)
		return false;

	for( Processes::iterator it = m_systemStatus.processes.begin(); it != m_systemStatus.processes.end(); ++it )
	{
		it->oldJiff = it->jiffies;
		it->jiffies = 0LL;
	}

	sleep(1);	/// Distance between two snapshots - should be long enough

	unsigned long long crtSysJiff = systemSnapshot( );
	unsigned long long deltaSysjiff = crtSysJiff - oldSysJiff;

	if ( !crtSysJiff || !deltaSysjiff )
		return false;

	LOG("CPU delta %3llu", deltaSysjiff);
	Processes::iterator it = m_systemStatus.processes.begin();
	while( it != m_systemStatus.processes.end() )
	{
		sprintf( it->m_szProcessorUsage, "%.1f", (float)(100.0 * (it->jiffies - it->oldJiff) / deltaSysjiff) );
		
		LOG("doProcessorStats: [%15s] jiff %6llu diff %3llu (%4s). Total CPU %4.1f %s", it->m_szName, it->jiffies, it->jiffies - it->oldJiff,
			it->m_szProcessorUsage, (float)(100.0 * it->jiffies / crtSysJiff), it->m_szStatus );


		if( !strcmp( "(scgi_svc)", it->m_szName) )
			it = m_systemStatus.processes.erase( it );
		else
			++it;
	}
	return true;
}

CIsa100Impl::SystemStatus* CIsa100Impl::getSystemStatus( void )
{
	struct sysinfo inf;
	struct statfs  buf;

	if ( sysinfo( &inf) )
	{
		LOG_ERR("ERROR getSystemStatus: sysinfo");
		inf.freeram = inf.totalram = 0;
	}
	if( statfs( "/access_node/", &buf))
	{
		LOG_ERR("ERROR getSystemStatus: statfs");
		buf.f_blocks = buf.f_bavail = buf.f_bsize = 0;
	}

	doProcessorStats();

	int nLen = FileRead("/proc/loadavg", m_systemStatus.load, sizeof(m_systemStatus.load) - 1);
	if (nLen<=0)
	{	nLen =0;
	}
	m_systemStatus.load[ nLen ] = 0;

	m_systemStatus.sysMemTotalKb = inf.totalram / 1024;
	m_systemStatus.sysMemFreeKb  = inf.freeram  / 1024;

	m_systemStatus.sysFlashTotalKb = buf.f_blocks * buf.f_bsize / 1024;
	m_systemStatus.sysFlashFreeKb  = buf.f_bavail * buf.f_bsize / 1024;

	return &m_systemStatus;
}
