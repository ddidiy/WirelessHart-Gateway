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


#include <WHartHost/periodictasks/DoGetTopology.h>



namespace hart7 {
namespace hostapp {


//send commands
void DoGetTopology::IssueTopologyCommand()
{
	try
	{
		DBCommand topologyCommand;
		topologyCommand.commandCode = DBCommand::ccGetTopology;
		topologyCommand.deviceID = m_devices.GatewayDevice()->id;
		topologyCommand.generatedType = DBCommand::cgtAutomatic;
		m_dbCommands.CreateCommand(topologyCommand, "system: topology scheduler", m_topologyCmdID);

		LOG_INFO_APP("[TopologyTask]: Clean reports");
		m_devices.CleanReports();
		
		m_processor.ProcessRequest(topologyCommand, m_dbCommands, m_devices);
		
		LOG_INFO_APP("[TopologyTask]: Automatic Topology request was made!");
	}
	catch(std::exception& ex)
	{
		LOG_ERROR_APP("[TopologyTask]: Error when generate automatic topology request! Error:" << ex.what());
	}
}
void DoGetTopology::IssueTopologyNotifyCommand()
{
	try
	{
		DBCommand topologyNotifyCommand;
		topologyNotifyCommand.commandCode = DBCommand::ccTopologyNotify;
		topologyNotifyCommand.deviceID = m_devices.GatewayDevice()->id;
		topologyNotifyCommand.generatedType = DBCommand::cgtAutomatic;
		m_dbCommands.CreateCommand(topologyNotifyCommand, "system: topology notify scheduler", m_topologyNotifyCmdID);

		m_processor.ProcessRequest(topologyNotifyCommand, m_dbCommands, m_devices);

		LOG_INFO_APP("[TopologyTask]: Automatic Notify Topology request was made!");
	}
	catch(std::exception& ex)
	{
		LOG_ERROR_APP("[TopologyTask]: Error when generate automatic notify topology request! Error:" << ex.what());
	}
}
void DoGetTopology::IssueServicesReportCommand()
{
	try
	{
		int cmdID;
		DBCommand serviceRepCommand;
		serviceRepCommand.commandCode = DBCommand::ccServicesReport;
		serviceRepCommand.deviceID = m_devices.GatewayDevice()->id;
		serviceRepCommand.generatedType = DBCommand::cgtAutomatic;
		m_dbCommands.CreateCommand(serviceRepCommand, "system: services report scheduler", cmdID);

		m_processor.ProcessRequest(serviceRepCommand, m_dbCommands, m_devices);

		LOG_INFO_APP("[TopologyTask]: Automatic Services Report request was made!");
	}
	catch(std::exception& ex)
	{
		LOG_ERROR_APP("[TopologyTask]: Error when generate automatic services report request! Error:" << ex.what());
	}
}
void DoGetTopology::IssueRoutesReportCommand()
{
	try
	{
		int cmdID;
		DBCommand serviceRepCommand;
		serviceRepCommand.commandCode = DBCommand::ccRoutesReport;
		serviceRepCommand.deviceID = m_devices.GatewayDevice()->id;
		serviceRepCommand.generatedType = DBCommand::cgtAutomatic;
		m_dbCommands.CreateCommand(serviceRepCommand, "system: routes report scheduler", cmdID);

		m_processor.ProcessRequest(serviceRepCommand, m_dbCommands, m_devices);

		LOG_INFO_APP("[TopologyTask]: Automatic Routes Report request was made!");
	}
	catch(std::exception& ex)
	{
		LOG_ERROR_APP("[TopologyTask]: Error when generate automatic Routes report request! Error:" << ex.what());
	}
}
void DoGetTopology::IssueSuperframesReportCommand()
{
	try
	{
		int cmdID;
		DBCommand serviceRepCommand;
		serviceRepCommand.commandCode = DBCommand::ccSuperframesReport;
		serviceRepCommand.deviceID = m_devices.GatewayDevice()->id;
		serviceRepCommand.generatedType = DBCommand::cgtAutomatic;
		m_dbCommands.CreateCommand(serviceRepCommand, "system: superframes report scheduler", cmdID);

		m_processor.ProcessRequest(serviceRepCommand, m_dbCommands, m_devices);

		LOG_INFO_APP("[TopologyTask]: Automatic Superframes Report request was made!");
	}
	catch(std::exception& ex)
	{
		LOG_ERROR_APP("[TopologyTask]: Error when generate automatic Superframes report request! Error:" << ex.what());
	}
}
void DoGetTopology::IssueScheduleLinksReportCommand()
{
	try
	{
		int cmdID;
		DBCommand serviceRepCommand;
		serviceRepCommand.commandCode = DBCommand::ccDeviceScheduleLinkReport;
		serviceRepCommand.deviceID = m_devices.GatewayDevice()->id;
		serviceRepCommand.generatedType = DBCommand::cgtAutomatic;
		m_dbCommands.CreateCommand(serviceRepCommand, "system: deviceLinks report scheduler", cmdID);

		m_processor.ProcessRequest(serviceRepCommand, m_dbCommands, m_devices);

		LOG_INFO_APP("[TopologyTask]: Automatic DeviceLinks Report request was made!");
	}
	catch(std::exception& ex)
	{
		LOG_ERROR_APP("[TopologyTask]: Error when generate automatic DeviceLinks report request! Error:" << ex.what());
	}
}

//do get
void DoGetTopology::DoTask(int periodTime/*ms*/)
{

	if (!m_isGWConnected)
	{
		LOG_WARN_APP("[TopologyTask]: gateway disconnected");
		return; // no reason to send commands without a session created
	}

	if (m_granularity == nlib::NOTIME)
	{
		LOG_WARN_APP("[TopologyTask]: granularity disabled");
		return; // disabled
	}

	if (m_issueTopologyNotifyCmd)
	{
		m_issueTopologyNotifyCmd = false;
		IssueTopologyNotifyCommand();
	}

	if (m_issueTopologyCmd)
	{
		
		nlib::DateTime currentTime = nlib::CurrentUniversalTime();
		if (currentTime> m_lastAutomaticCommandTime + m_granularity)
		{	m_issueTopologyCmd = false;
			IssueTopologyCommand();
			m_lastAutomaticCommandTime = currentTime;
		}
	}

	if (m_issueNeededReportsCmd)
	{
		IssueServicesReportCommand();
		IssueRoutesReportCommand();
		IssueSuperframesReportCommand();
		IssueScheduleLinksReportCommand();
		m_issueNeededReportsCmd = false;
	}

}

//resp notifications
void DoGetTopology::HandleRespondedCommand(const int commandID, DBCommand::ResponseStatus status, void *pData)
{
	if (m_topologyNotifyCmdID == commandID && commandID != DBCommand::NO_COMMAND_ID)
	{	
		if (status != DBCommand::rsSuccess)
		{	LOG_WARN_APP("[TopologyTask]: topology notify command was unsuccessfull, so retry it ...");
			m_issueTopologyNotifyCmd = true;
			return;
		}
		

		LOG_INFO_APP("[TopologyTask]: topology notify command was successfull");
	}

	if (m_topologyCmdID == commandID && commandID != DBCommand::NO_COMMAND_ID)
	{	
		if (status != DBCommand::rsSuccess)
		{	LOG_WARN_APP("[TopologyTask]: topology command was unsuccessfull, so retry again after a period...");
			m_issueTopologyCmd = true;
		}
		else
		{	
			if (pData != NULL)
			{
				LOG_INFO_APP("[TopologyTask]: topology command was successfull, so send needed reports...");
				m_issueNeededReportsCmd = true;
				return;
			}
			
			LOG_INFO_APP("[TopologyTask]: topology command was successfull bu no devices found, so no needed to send reports...");
		}
	}

	
}

//gw notifications
void DoGetTopology::HandleGWConnect(const std::string& host, int port)
{
	LOG_INFO_APP("[DoGetTopology]:Gateway reconnected ...");
	m_isGWConnected = true;
}
void DoGetTopology::HandleGWDisconnect()
{
	LOG_WARN_APP("[DoGetTopology]: Gateway disconnected!");
	m_isGWConnected = false;
	m_issueTopologyNotifyCmd = true;
	m_lastAutomaticCommandTime = nlib::CurrentUniversalTime() - m_granularity;
	m_issueTopologyCmd = true;
	m_issueNeededReportsCmd = false;
}


}
}
