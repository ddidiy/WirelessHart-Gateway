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


#ifndef DOFLUSHREPORTSCACHE_H_
#define DOFLUSHREPORTSCACHE_H_

#include <Shared/MicroSec.h>

#include <WHartHost/database/DevicesManager.h>

#include <nlib/log.h>

#include <boost/format.hpp>



namespace hart7 {
namespace hostapp {

/*
 *  it cleans up some tables from db
 */
class DoFlushReportsCache
{
	
public:
	DoFlushReportsCache(DevicesManager& devices, int flushReportsPeriod):
			m_devices(devices),
			m_flushReportsPeriod(flushReportsPeriod)

	{

		m_lastFlushTime.MarkStartTime();
		LOG_INFO_APP("[DoFlushReportsCache]: started with a period = " << flushReportsPeriod);
	}

//do flush
public:
	void DoTask(int periodTime/*ms*/);

//
private:
	DevicesManager&	m_devices;

	CMicroSec		m_lastFlushTime;
	int				m_flushReportsPeriod;
};

}//namespace hostapp
}//namespace hart7



#endif
