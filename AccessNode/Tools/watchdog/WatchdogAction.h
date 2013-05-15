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

#ifndef _WATCHDOG_ACTION_
#define _WATCHDOG_ACTION_

#include <sys/sysinfo.h>
#include <sys/vfs.h> /* or <sys/statfs.h> */ //statfs
#include <sys/statfs.h> 

/// @brief Return Free space in /tmp
/// @retval 0 On Error
inline long long getFreeTmp()
{
	struct statfs buf;
	long long nTmpFree = 0 ;
	if( !statfs( NIVIS_TMP, &buf))// SUCCESS
	{
		nTmpFree= (buf.f_bavail) * (long long) buf.f_bsize;
	}
	return nTmpFree ;
}

/// @brief Return Free Ram in Bytes
/// @retval 0 On Error
inline long getFreeRam()
{
#ifndef CYG
	struct sysinfo info;
	if ( -1 == sysinfo(&info) )
		return 0 ;
	return info.freeram ;
#else
	return 1000000;
#endif
}

class CWatchdogAction {
public:
	CWatchdogAction()
		: freeMemCrossed(false), bFreeTmpCrossed(false), m_unMinMem( 0xFFFFFFFF ), m_unMinTmp( 0xFFFFFFFF )
	{}
	void WatchFreeMemory( long threshold  )
	{
		long freeram = getFreeRam() ;
		freeram -= getFreeTmp() ;
		m_unMinMem = _Min( freeram, m_unMinMem) ;

		if ( freeram < threshold )
		{
			if ( ! freeMemCrossed )
			{
				systemf_to( 20, "log2flash 'WTD: Free memory (%u kB) critically below threshold:(%u kB)'&", freeram/1024, threshold/1024);
				LOG("WTD: Free memory (%u kB) critically below threshold:(%u kB)", freeram/1024, threshold/1024);
				freeMemCrossed = true ;
			}
		}
		if ( freeram > threshold && freeMemCrossed )
		{
			freeMemCrossed = false ;
			systemf_to( 20, "log2flash 'WTD: Free memory (%u kB) above threshold:(%d kB). Minimum was (%u kB)'&", freeram/1024, threshold/1024, m_unMinMem/1024 );
			LOG("WTD: Free memory (%u kB) above threshold:(%d kB). Minimum was (%u kB)", freeram/1024, threshold/1024, m_unMinMem/1024 );
			m_unMinMem = 0xFFFFFFFF;
		}
	}
	void WatchFreeTmp( long long threshold  )
	{
		long long nTmpFree = getFreeTmp();

		m_unMinTmp = _Min( nTmpFree, m_unMinTmp);
		if ( nTmpFree < threshold )
		{
			if ( ! bFreeTmpCrossed )
			{
				systemf_to( 20, "log2flash 'WTD: Free /tmp space (%llu kB) critically below threshold:(%u kB)'&", nTmpFree/1024, threshold/1024);
				LOG("WTD: Free /tmp space (%llu kB) critically below threshold:(%d kB)", nTmpFree/1024, threshold/1024);
				bFreeTmpCrossed = true ;
			}
		}
		if ( nTmpFree > threshold && bFreeTmpCrossed )
		{
			bFreeTmpCrossed = false ;
			systemf_to( 20, "log2flash 'WTD: Free /tmp space (%llu kB) above threshold:(%d kB). Minimum was (%u kB)'&", nTmpFree/1024, threshold/1024, m_unMinTmp/1024 );
			LOG("WTD:Free /tmp space (%llu kB) above threshold:(%d kB). Minimum was (%u kB)", nTmpFree/1024, threshold/1024, m_unMinTmp/1024 );
			m_unMinTmp = 0xFFFFFFFF;
		}
	}

protected:
	bool freeMemCrossed;
	bool bFreeTmpCrossed;
	long m_unMinMem;
	long long m_unMinTmp;
} ;

#endif	/* _WATCHDOG_ACTION_ */
