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


#include <WHartHost/periodictasks/DoProcessDBRequests.h>



namespace hart7 {
namespace hostapp {


//do read
void DoProcessDBRequests::DoTask(int periodTime/*ms*/)
{

	//wait until mh connects to gw
	while (!m_isGWConnected)
	{
//		sleep(2);
		return;
	}

	DBCommandsList newCommands;
	try
	{
		//read new posted commands from db
		m_dbCommands.GetNewCommands(newCommands);
	}
	catch (std::exception& ex)
	{
		LOG_ERROR_APP("[ProcessDBReqTask]: failed! error=" << ex.what());
		return;
	}

	if (!newCommands.empty())
	{
		LOG_DEBUG_APP("[ProcessDBReqTask]: CommandsCount=" << newCommands.size());

		//notify new commands
		for (DBCommandsList::iterator it = newCommands.begin(); it != newCommands.end(); it++)
		{

			LOG_INFO_APP("[ProcessDBReqTask]: FireNewCommand: Command=" << (*it));
			m_processor.ProcessRequest(*it, m_dbCommands, m_devices);
		}
	}
}

//gw notifications
void DoProcessDBRequests::HandleGWConnect(const std::string& host, int port)
{
	m_isGWConnected = true;
}
void DoProcessDBRequests::HandleGWDisconnect()
{
	m_isGWConnected = false;
}

}
}
