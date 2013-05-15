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


#include <WHartHost/periodictasks/DoDBCleanUp.h>



namespace hart7 {
namespace hostapp {


//do clean-up
void DoDBCleanUp::DoTask(int periodTime/*ms*/)
{
	
	if ((m_periodCleanupSecs != 0) && (m_lastCleanupTime.GetElapsedSec() > m_periodCleanupSecs))
	{
		try
		{
			LOG_INFO_APP("[DoDBCleanUp]: Removing old entries from DB...");
			m_dbCommands.CleanupOldRecords(nlib::CurrentUniversalTime() - m_removeOlderThanMinutes, m_removeMoreThanMaxCount);
			LOG_INFO_APP("[DoDBCleanUp]: Finished removing old entries from DB!");
		}
		catch(std::exception& ex)
		{
			LOG_WARN_APP("[DoDBCleanUp]: An error occured while removing old entries from the database. error=" << ex.what());
		}

		m_lastCleanupTime.MarkStartTime();
	}

	if (canBeVacuumed && (m_lastVacuumTime.GetElapsedSec() > m_periodVacuumSecs))
	{
		try
		{
			LOG_INFO_APP("[DoDBCleanUp]: Vacuuming the database...");
			//m_dbCommands.VacuumDatabase();
			LOG_INFO_APP("[DoDBCleanUp]: Vacuuming finished!");
		}
		catch(std::exception& ex)
		{
			LOG_WARN_APP("[DoDBCleanUp]: An error occured while vacuuming the database. error=" << ex.what());
		}
		if (m_periodVacuumSecs == 0) //just at first run db should be vaccumed
			canBeVacuumed = false;


		m_lastVacuumTime.MarkStartTime();
	}
}

}
}
