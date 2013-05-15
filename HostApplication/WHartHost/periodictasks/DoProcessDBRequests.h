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


#ifndef _DOPROCESSDBREQUESTS_H_
#define _DOPROCESSDBREQUESTS_H_


#include <WHartHost/database/DBCommandsManager.h>
#include <WHartHost/database/DevicesManager.h>
#include <WHartHost/cmdsprocessor/CommandsProcessor.h>

#include <nlib/log.h>

#include <boost/format.hpp>



namespace hart7 {
namespace hostapp {

/*
 * It reads commands with status 'new' from db
 */
class DoProcessDBRequests
{
	
public:
	DoProcessDBRequests(DBCommandsManager& commands, DevicesManager& devices,
		CommandsProcessor& processor) :
		m_dbCommands(commands), m_devices(devices), m_processor(processor)
	{
		m_isGWConnected = false;
		
		LOG_INFO_APP("DoProcessDBRequests started.");
	}

//do read
public:
	void DoTask(int periodTime/*ms*/);


//gw notifications
public:
	void HandleGWConnect(const std::string& host, int port);
	void HandleGWDisconnect();

//
private:
	bool m_isGWConnected;

private:
	DBCommandsManager&	m_dbCommands;
	DevicesManager&		m_devices;
	CommandsProcessor&  m_processor;
};

}//namespace hostapp
}//namespace hart7



#endif
