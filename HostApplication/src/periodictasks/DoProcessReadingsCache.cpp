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


#include <WHartHost/periodictasks/DoProcessReadingsCache.h>

namespace hart7 {
namespace hostapp {


//do read
void DoProcessReadingsCache::DoTask(int periodTime/*ms*/)
{
	// save cached readings to DB
	if (!m_readingsMap.empty())
	{
		ProcessReadingsCache();
	}
}

//gw notifications
void DoProcessReadingsCache::CacheReading(const DeviceReading& p_rReading)
{
	LOG_DEBUG_APP("[DoProcessReadingsCache]: ProcessReading - Cache.Insert(deviceID=" << p_rReading.m_deviceID << ", m_channelNo="
			<< p_rReading.m_channelNo << "). ReadingsCount=" << m_readingsMap.size()
			<< ", ElapsedSeconds=" << m_lastCommitTime.GetElapsedSec());

	DeviceReadingKey key(p_rReading.m_deviceID, p_rReading.m_channelNo);
	m_readingsMap[key] = p_rReading;
}

void DoProcessReadingsCache::ProcessReadingsCache()
{
	if (m_readingsMap.size() < m_nMaxReadingsNoPerTransaction &&
		m_lastCommitTime.GetElapsedSec() < m_nReadingsSavePeriod)
	    return; // too soon to save readings in DB or readings count is less than maximum limit

	// Init time period
	m_lastCommitTime.MarkStartTime();
	try
	{
		LOG_DEBUG_APP("[DoProcessReadingsCache]: Readings - BEGIN TRANSACTION. Cache.size=" << m_readingsMap.size());
		m_devices.ProcessReadingsTransaction(m_readingsMap);
		LOG_DEBUG_APP("[DoProcessReadingsCache]: Readings - COMMIT TRANSACTION.");
	}
    catch (std::exception ex)
    {
        LOG_ERROR_APP("[DoProcessReadingsCache]: Readings - ROLLBACK TRANSACTION. Application error! Exception: " << ex.what());
    }
	catch (...)
	{
		LOG_ERROR_APP("[DoProcessReadingsCache]: Readings - ROLLBACK TRANSACTION. System error!");
	}
    m_readingsMap.clear();
}

}
}
