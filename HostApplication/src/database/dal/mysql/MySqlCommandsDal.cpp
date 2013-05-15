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


#ifndef HW_VR900


#include <string>
#include <algorithm>

#include "MySqlCommandsDal.h"


namespace hart7 {
namespace hostapp {

MySqlCommandsDal::MySqlCommandsDal(MySQLConnection& connection_) :
	connection(connection_)
{
}

MySqlCommandsDal::~MySqlCommandsDal()
{
}

//init
void MySqlCommandsDal::VerifyTables()
{
	LOG_DEBUG_APP("[CommandsDAL]: Verify commands tables structure...");
	{
		std::string query = "SELECT CommandID, DeviceID, CommandCode, CommandStatus, TimePosted, TimeResponsed,"
			" ErrorCode, ErrorReason, Response FROM Commands WHERE 0";
		try
		{
			MySQLCommand(connection, query).ExecuteQuery();
		}
		catch (std::exception& ex)
		{
			LOG_ERROR_APP("Verify failed for='" << query << "' error=" << ex.what());
			throw;
		}
	}

	{
		std::string query = "SELECT CommandID, ParameterCode, ParameterValue FROM CommandParameters WHERE 0";
		try
		{
			MySQLCommand(connection, query).ExecuteQuery();
		}
		catch(std::exception& ex)
		{
			LOG_ERROR_APP("Verify failed for='" << query << "' error=" << ex.what());
			throw;
		}
	}
}


//create
void MySqlCommandsDal::CreateCommand(DBCommand& command)
{
	LOG_DEBUG_APP("[CommandsDAL]: Create command:" << command);
	MySQLCommand sqlCommand(connection, ""
		"INSERT INTO Commands(DeviceID, CommandCode, CommandStatus, TimePosted, ErrorCode, Generated)"
		" VALUES(?001, ?002, ?003, ?004, ?005, ?006);");
	sqlCommand.BindParam(1, command.deviceID);
	sqlCommand.BindParam(2, (int)command.commandCode);
	sqlCommand.BindParam(3, (int)command.commandStatus);
	sqlCommand.BindParam(4, command.timePosted);
	sqlCommand.BindParam(5, 0);
	sqlCommand.BindParam(6, (int)command.generatedType);
	sqlCommand.ExecuteNonQuery();
	command.commandID = sqlCommand.GetLastInsertRowID();

	CreateCommandParameters(command.commandID, command.parameters);
	//LOG_DEBUG_APP(sqlCommand.Query());
}
void MySqlCommandsDal::CreateCommandParameters(int commandID, DBCommand::ParametersList& commandParameters)
{
	LOG_DEBUG_APP("[CommandsDAL]: Create parameters for command with ID:" << commandID);

	for (DBCommand::ParametersList::const_iterator it = commandParameters.begin(); it != commandParameters.end(); it++)
	{
		MySQLCommand sqlCommand(connection, ""
			"INSERT INTO CommandParameters(ParameterCode, ParameterValue, CommandID) "
			"VALUES (?001, ?002, ?003)");

		sqlCommand.BindParam(1, (int)it->parameterCode);
		sqlCommand.BindParam(2, it->parameterValue);
		sqlCommand.BindParam(3, commandID);

		sqlCommand.ExecuteNonQuery();
	}
}

//get
bool MySqlCommandsDal::GetCommand(int commandID, DBCommand& command)
{
	LOG_DEBUG_APP("[CommandsDAL]: Try to find command with ID:" << commandID);
	MySQLCommand sqlCommand(connection,
			"SELECT DeviceID, CommandCode, CommandStatus, TimePosted, TimeResponsed, ErrorCode, ErrorReason, Response, Generated"
				" FROM Commands WHERE CommandID = ?001");

	sqlCommand.BindParam(1, commandID);
	MySQLResultSet::Ptr rsCommands = sqlCommand.ExecuteQuery();
	if (rsCommands->RowsCount())
	{
		MySQLResultSet::Iterator itCommand = rsCommands->Begin();
		command.commandID = commandID;
		command.deviceID = itCommand->Value<int>(0);
		command.commandCode = (DBCommand::CommandCode)itCommand->Value<int>(1);
		command.commandStatus = (DBCommand::CommandStatus)itCommand->Value<int>(2);
		command.timePosted = itCommand->Value<nlib::DateTime>(3);
		if (!itCommand->IsNull(4))
		{
			command.timeResponded = itCommand->Value<nlib::DateTime>(4);
		}
		command.errorCode = (DBCommand::ResponseStatus)itCommand->Value<int>(5);
		if (!itCommand->IsNull(6))
		{
			command.errorReason = itCommand->Value<std::string>(6);
		}
		if (!itCommand->IsNull(7))
		{
			command.response = itCommand->Value<std::string>(7);
		}
		command.generatedType = (DBCommand::CommandGeneratedType)itCommand->Value<int>(8);
		return true;
	}
	return false;
}

void MySqlCommandsDal::GetCommandParameters(int commandID, DBCommand::ParametersList& commandParameters)
{
	LOG_DEBUG_APP("[CommandsDAL]: Get parameters for command with ID:" << commandID);
	MySQLCommand sqlCommand(connection, "SELECT ParameterCode, ParameterValue"
		" FROM CommandParameters WHERE CommandID = ?001");

	sqlCommand.BindParam(1, commandID);
	MySQLResultSet::Ptr rsCommandParams = sqlCommand.ExecuteQuery();
	commandParameters.reserve(commandParameters.size() + rsCommandParams->RowsCount());
	for (MySQLResultSet::Iterator itCommandParam = rsCommandParams->Begin(); itCommandParam
			!= rsCommandParams->End(); itCommandParam++)
	{
		DBCommandParameter cp;
		cp.parameterCode = (DBCommandParameter::ParameterCode)itCommandParam->Value<int>(0);
		cp.parameterValue = itCommandParam->Value<std::string>(1);

		commandParameters.push_back(cp);
	}
}

void MySqlCommandsDal::GetNewCommands(DBCommandsList& newCommands)
{
	LOG_DEBUG_APP("[CommandsDAL]: Get all new commands from database");
	MySQLCommand sqlCommand(connection,
			"SELECT CommandID, DeviceID, CommandCode, CommandStatus, TimePosted, Generated"
				" FROM Commands WHERE CommandStatus = ?001");
	sqlCommand.BindParam(1, (int)DBCommand::csNew);

	MySQLResultSet::Ptr rsCommands = sqlCommand.ExecuteQuery();
	newCommands.reserve(newCommands.size() + rsCommands->RowsCount());
	LOG_DEBUG_APP(rsCommands->RowsCount() << " new commands found!");
	for (MySQLResultSet::Iterator itCommand = rsCommands->Begin(); itCommand != rsCommands->End(); itCommand++)
	{
		DBCommand cmd;
		cmd.commandID = itCommand->Value<int>(0);;
		cmd.deviceID = itCommand->Value<int>(1);
		cmd.commandCode = (DBCommand::CommandCode)itCommand->Value<int>(2);
		cmd.commandStatus = (DBCommand::CommandStatus)itCommand->Value<int>(3);
		cmd.timePosted = itCommand->Value<nlib::DateTime>(4);
		cmd.generatedType = (DBCommand::CommandGeneratedType)itCommand->Value<int>(5);

		GetCommandParameters(cmd.commandID, cmd.parameters);
		newCommands.push_back(cmd);
	}
}


//set
void MySqlCommandsDal::SetCommandAsSent(DBCommand& command)
{
	LOG_DEBUG_APP("[CommandsDAL]: Update command:" << command.commandID << " as sent!");
	MySQLCommand sqlCommand(connection, ""
		"UPDATE Commands SET CommandStatus = ?001"
		" WHERE CommandID = ?002");
	sqlCommand.BindParam(1, (int)DBCommand::csSent);
	sqlCommand.BindParam(2, command.commandID);

	sqlCommand.ExecuteNonQuery();

	//now update also model
	command.commandStatus = DBCommand::csSent;
}

void MySqlCommandsDal::SetCommandAsResponded(DBCommand& command, const nlib::DateTime& responseTime,
		const std::string& response)
{
	LOG_DEBUG_APP("[CommandsDAL]: Update command:" << command.commandID << " as responded!");
	MySQLCommand sqlCommand(connection, ""
		"UPDATE Commands SET "
		" CommandStatus = ?001, TimeResponsed = ?002, ErrorCode = ?003, Response = ?004 "
		" WHERE CommandID = ?005");

	sqlCommand.BindParam(1, (int)DBCommand::csResponded);
	sqlCommand.BindParam(2, responseTime);
	sqlCommand.BindParam(3, (int)DBCommand::rsFailure_GatewayReponseCodeBase /*+ 0*/);
	sqlCommand.BindParam(4, response);
	sqlCommand.BindParam(5, command.commandID);

	sqlCommand.ExecuteNonQuery();

	//now update also model
	command.commandStatus = DBCommand::csResponded;
	command.timeResponded = responseTime;
	command.errorCode = DBCommand::rsFailure_GatewayReponseCodeBase /*+ 0*/;
	command.response = response;
}

void MySqlCommandsDal::SetCommandAsFailed(DBCommand& command, const nlib::DateTime& errorTime, int errorCode,
		const std::string& errorReason)
{
	LOG_DEBUG_APP("[CommandsDAL]: Update command:" << command.commandID << " as failed");
	MySQLCommand sqlCommand(connection, ""
		"UPDATE Commands SET"
		" CommandStatus = ?001, TimeResponsed = ?002, ErrorCode = ?003, ErrorReason = ?004"
		" WHERE CommandID = ?005");

	sqlCommand.BindParam(1, (int)DBCommand::csFailed);
	sqlCommand.BindParam(2, errorTime);
	sqlCommand.BindParam(3, errorCode);
	sqlCommand.BindParam(4, errorReason);
	sqlCommand.BindParam(5, command.commandID);

	sqlCommand.ExecuteNonQuery();

	//now update also model
	command.commandStatus = DBCommand::csFailed;
	command.timeResponded = errorTime;
	command.errorCode = DBCommand::rsFailure_GatewayReponseCodeBase /*+ 0*/;
	command.errorReason = errorReason;
}


void MySqlCommandsDal::SetDBCmdIDAsNew(int DBCmsID)
{
	LOG_DEBUG_APP("[CommandsDAL]: Update DBCmdID:" << (boost::int32_t)DBCmsID << " as new");
	MySQLCommand sqlCommand(connection, ""
		"UPDATE Commands SET "
		" CommandStatus = ?001 "
		" WHERE CommandID = ?002");

	sqlCommand.BindParam(1, (int)DBCommand::csNew);
	sqlCommand.BindParam(2, DBCmsID);

	sqlCommand.ExecuteNonQuery();
}


}//hostapp
}//hart7

#endif
