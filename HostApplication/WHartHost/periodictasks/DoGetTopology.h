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

#ifndef DOGETTOPOLOGY_H_
#define DOGETTOPOLOGY_H_

#include <WHartHost/database/DBCommandsManager.h>
#include <WHartHost/database/DevicesManager.h>
#include <WHartHost/cmdsprocessor/CommandsProcessor.h>

#include <nlib/log.h>
#include <nlib/datetime.h>


namespace hart7 {
namespace hostapp {


/*
 * it gets topology
 */
class DoGetTopology
{
	LOG_DEF("hart7.hostapp.DoGetTopology");

public:
	DoGetTopology(DBCommandsManager& commands, DevicesManager& devices, 
			CommandsProcessor& processor, const nlib::TimeSpan& granularity) :
		m_dbCommands(commands), m_devices(devices), m_processor(processor), m_granularity(granularity)
	{		
		m_devices.ResetTopology();
		
		m_isGWConnected = false;
		m_topologyCmdID = DBCommand::NO_COMMAND_ID;
		m_topologyNotifyCmdID = DBCommand::NO_COMMAND_ID;
		m_issueTopologyNotifyCmd = true;
		m_lastAutomaticCommandTime = nlib::CurrentUniversalTime() - m_granularity;
		m_issueTopologyCmd = true;
		m_issueNeededReportsCmd = false;

		LOG_INFO_APP("DoGetTopology started");
	}

//send commands
public:
    void IssueTopologyCommand();

private:
	void IssueTopologyNotifyCommand();
	void IssueServicesReportCommand();
	void IssueRoutesReportCommand();
	void IssueSuperframesReportCommand();
	void IssueScheduleLinksReportCommand();

//do get
public:
	void DoTask(int periodTime/*ms*/);

//resp notifications
public:
	void HandleRespondedCommand(const int commandID, DBCommand::ResponseStatus status, void *pData);

//gw notifications
public:
	void HandleGWConnect(const std::string& host, int port);
	void HandleGWDisconnect();

//
private:
	bool m_isGWConnected;

private:
	DBCommandsManager&		m_dbCommands;
	DevicesManager&			m_devices;
	CommandsProcessor&		m_processor;
	nlib::DateTime			m_lastAutomaticCommandTime;
	const nlib::TimeSpan	m_granularity;

private:
	int m_topologyCmdID;
	int m_topologyNotifyCmdID;
	bool m_issueTopologyCmd;
	bool m_issueNeededReportsCmd;
	bool m_issueTopologyNotifyCmd;
	
};

} //namespace hostapp
} //namespace hart7

#endif /*DOGETTOPOLOGY_H_*/
