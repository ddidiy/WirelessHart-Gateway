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


#ifndef _DOPROCESSREADINGSCACHE_H_
#define _DOPROCESSREADINGSCACHE_H_


#include <WHartHost/database/DevicesManager.h>
#include <WHartHost/model/DeviceReading.h>


namespace hart7 {
namespace hostapp {

class DoProcessReadingsCache
{
	
public:
	DoProcessReadingsCache(DevicesManager& devices, int p_nReadingsSavePeriod, int p_nMaxReadingsNoPerTransaction) :
		m_devices(devices), m_readingsMap(), m_nReadingsSavePeriod(p_nReadingsSavePeriod),
		m_nMaxReadingsNoPerTransaction(p_nMaxReadingsNoPerTransaction)
	{
		LOG_INFO_APP("DoProcessReadingsCache started.");
	}

public:
	void DoTask(int periodTime/*ms*/);

// readings
public:
	void CacheReading(const DeviceReading& p_rReading);

private:
	void ProcessReadingsCache();

private:
	DevicesManager&		m_devices;

private: // cache for saving reading data in DB
	DeviceReadingsMapT 			m_readingsMap;
	CMicroSec					m_lastCommitTime;

	unsigned int				m_nReadingsSavePeriod; // seconds
	unsigned int				m_nMaxReadingsNoPerTransaction;
};

}//namespace hostapp
}//namespace hart7


#endif
