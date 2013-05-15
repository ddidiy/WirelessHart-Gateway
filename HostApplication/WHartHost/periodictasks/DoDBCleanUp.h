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

#ifndef DODBCLEANUP_H_
#define DODBCLEANUP_H_


#include <WHartHost/database/DBCommandsManager.h>

#include <nlib/log.h>
#include <nlib/datetime.h>

#include <boost/format.hpp>



namespace hart7 {
namespace hostapp {

/*
 *  it cleans up some tables from db
 */
class DoDBCleanUp
{
	
public:
	DoDBCleanUp(DBCommandsManager& commands, int periodVacuumMinutes, int periodCleanupMinutes, 
		int removeOlderThanMinutes, int removeMoreThanMaxCount) : m_dbCommands(commands), m_periodVacuumSecs(periodVacuumMinutes*60),
			m_periodCleanupSecs(periodCleanupMinutes*60), 
			m_removeOlderThanMinutes(nlib::util::minutes(removeOlderThanMinutes)),
			m_removeOlderThanSecs(removeOlderThanMinutes*60),
			m_removeMoreThanMaxCount(removeMoreThanMaxCount)
	{

		canBeVacuumed = false;
		if(periodVacuumMinutes >= 0)
			canBeVacuumed = true;


		m_lastVacuumTime.MarkStartTime();
		m_lastCleanupTime.MarkStartTime();

		LOG_INFO_APP("[DoDBCleanUp]: Vacuuming db at a period of " << periodVacuumMinutes <<
				" minutes and rm_older_than_minutes = " << removeOlderThanMinutes
				<< "meaning checking for older than = " 
				<< nlib::ToString(nlib::CurrentUniversalTime() - nlib::util::minutes(removeOlderThanMinutes))
				<< " at period of " << periodCleanupMinutes << " minutes");

		LOG_INFO_APP("[DoDBCleanUp]: started.");
	}

//do clean-up
public:
	void DoTask(int periodTime/*ms*/);

//
private:
	DBCommandsManager&	m_dbCommands;

	CMicroSec		m_lastVacuumTime;
	CMicroSec		m_lastCleanupTime;


	int m_periodVacuumSecs;
	int m_periodCleanupSecs;

	const nlib::TimeSpan m_removeOlderThanMinutes;

	int m_removeOlderThanSecs;
	int m_removeMoreThanMaxCount;

	bool canBeVacuumed;
};

}//namespace hostapp
}//namespace hart7



#endif
