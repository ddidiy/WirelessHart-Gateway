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


#include "AppCommandsFactory.h"
#include "RequestProcessor.h"
#include "ResponseProcessor.h"

#include <WHartHost/cmdsprocessor/CommandsProcessor.h>

#include <boost/format.hpp>

#include "SaveRespErr.h"


namespace hart7 {
namespace hostapp {

CommandsProcessor::CommandsProcessor(gateway::GatewayIO &gateway_):gateway(gateway_)
{
}


//process
void CommandsProcessor::ProcessRequest(DBCommand& command, DBCommandsManager& commands, DevicesManager& devices)
{
	
	try
	{
		AbstractAppCommandPtr appCmd = AppCommandsFactory().Create(command, devices);
		
		appCmd->dbCommand = command;
		appCmd->pCommands = &commands;
		appCmd->pDevices = &devices;

		LOG_DEBUG_APP("[ProcessRequest] processing... Request=" << *appCmd );

		RequestProcessor().Process(appCmd, *this);
		
		if (appCmd->dbCommand.commandStatus == DBCommand::csNew)
			commands.SetCommandSent(command);
	}
	catch (InvalidCommandException& ex)
	{
		LOG_ERROR_APP("[ProcessRequest] failed! Command=" << command << " error=" << ex.what());
		SaveRespErr(commands, command).CommandFailed(DBCommand::rsFailure_InvalidCommand);
	}
	catch (DeviceNodeRegisteredException& ex)
	{
		LOG_ERROR_APP("[ProcessRequest] failed! Command=" << command << " error=" << ex.what());
		SaveRespErr(commands, command).CommandFailed(DBCommand::rsFailure_DeviceNotRegistered);
	}
	catch (DeviceNotFoundException& ex)
	{
		LOG_ERROR_APP("[ProcessRequest] failed! Command=" << command << " error=" << ex.what());
		SaveRespErr(commands, command).CommandFailed(DBCommand::rsFailure_InvalidDevice);
	}
	catch (gateway::GWChannelException& ex)
	{
		LOG_ERROR_APP("[ProcessRequest] failed! Command=" << command << " error=" << ex.what());
		SaveRespErr(commands, command).CommandFailed(DBCommand::rsFailure_HostSendChannelError);
	}
	catch (gateway::GWTrackingException& ex)
	{
		LOG_ERROR_APP("[ProcessRequest] failed! Command=" << command << " error=" << ex.what());
		SaveRespErr(commands, command).CommandFailed(DBCommand::rsFailure_HostSendTrackingError);
	}
	catch (gateway::GWSerializeException& ex)
	{
		LOG_ERROR_APP("[ProcessRequest] failed! Command=" << command << " error=" << ex.what());
		SaveRespErr(commands, command).CommandFailed(DBCommand::rsFailure_HostSerializationError);
	}
	catch (gateway::GWCmdAlreadySentException& ex)
	{
		LOG_ERROR_APP("[ProcessRequest] failed! Command=" << command << " error=" << ex.what());
		SaveRespErr(commands, command).CommandFailed(DBCommand::rsFailure_HostGWCmdAlreadySentError);
	}
	catch (std::exception& ex)
	{
		LOG_ERROR_APP("[ProcessRequest] failed! Command=" << command << " error=" << ex.what());
		SaveRespErr(commands, command).CommandFailed(DBCommand::rsFailure_InternalError);
	}
	catch (...)
	{
		LOG_ERROR_APP("[ProcessRequest] failed! Command=" << command << " unknown exception!");
		SaveRespErr(commands, command).CommandFailed(DBCommand::rsFailure_InternalError);
	}
}
void CommandsProcessor::ProcessResponse(gateway::GatewayIO::GWResponse &response)
{
	
	LOG_DEBUG_APP("[ProcessResponse] processing... Response=" << *(response.appData.appCmd) );

	try
	{
		ResponseProcessor().Process(response, *this);
	}
	catch (std::exception& ex)
	{
		LOG_ERROR_APP("[ProcessResponse] failed! error=" << ex.what() << " Response=" << *(response.appData.appCmd) );

		SaveRespErr(*response.appData.appCmd->pCommands, response.appData.appCmd->dbCommand).CommandFailed(DBCommand::rsFailure_InternalError);
	}
	catch (...)
	{
		LOG_ERROR_APP("[ProcessResponse] failed! unknown error. Response=" << *(response.appData.appCmd) );

		SaveRespErr(*response.appData.appCmd->pCommands, response.appData.appCmd->dbCommand).CommandFailed(DBCommand::rsFailure_InternalError);
	}

}


} // namespace hostapp
} // namespace hart7
