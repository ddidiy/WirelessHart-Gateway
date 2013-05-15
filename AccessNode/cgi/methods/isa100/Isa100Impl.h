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

#ifndef _ISA100_IMPL_H_
#define _ISA100_IMPL_H_

#include <vector>

//////////////////////////////////////////////////////////////////////
/// @class CIsa100Impl
//////////////////////////////////////////////////////////////////////

class CIsa100Impl
{

public:
	struct ProcessStatus
	{
		char			m_szName[ 32 ] ;
		char			m_szDisplayName[ 32 ];
		char  			m_szStatus[ 32 ];
		unsigned		m_ushMemKb;
		char 			m_szProcessorUsage [ 32 ];

		unsigned long long jiffies;
		unsigned long long oldJiff;

		ProcessStatus( const char * p_szName, const char * p_szDisplayName )
				: m_ushMemKb(0), jiffies(0LL), oldJiff(0LL)
		{
			strncpy( m_szName, p_szName, sizeof(m_szName) );
			strncpy( m_szDisplayName, p_szDisplayName, sizeof(m_szDisplayName));
			m_szName[ sizeof(m_szName) - 1 ] = m_szDisplayName[ sizeof(m_szDisplayName) -1 ] = 0;
			strcpy( m_szStatus, "Not Running" );
		};
	};

	typedef std::vector<ProcessStatus> Processes;

	struct SystemStatus
	{
		Processes	processes;
		unsigned 	sysMemTotalKb;
		unsigned	sysMemFreeKb;
		unsigned 	sysFlashTotalKb;
		unsigned 	sysFlashFreeKb;
		char		load[ 64 ];
	} m_systemStatus;

public:
	SystemStatus* getSystemStatus( void );
	bool applyConfigChanges( const char * p_szModule );

private:
	unsigned long long getTotalJiffSnapshot( void );
	bool getProcessesSnapshot( void );
	unsigned long long systemSnapshot( void );
	bool doProcessorStats( void );

	bool isLocked( const char* p_szProcessName, unsigned p_unstaleLimit = 90 );
} ;
#endif	//_ISA100_IMPL_H_
