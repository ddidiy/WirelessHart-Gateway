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

#include <string>
#include <algorithm>

#include "SqliteCommandsDal.h"

#include <list>

namespace hart7 {
namespace hostapp {

SqliteCommandsDal::SqliteCommandsDal(sqlitexx::Connection& connection_) :
	connection(connection_), m_oNewCommand_Compiled(connection_)
{
}

SqliteCommandsDal::~SqliteCommandsDal()
{
}

//init
void SqliteCommandsDal::VerifyTables()
{
	LOG_DEBUG_APP("[CommandsDAL]: Verify commands tables structure...");
	{
		const char* query = "SELECT CommandID, DeviceID, CommandCode, CommandStatus, TimePosted, TimeResponsed,"
			" ErrorCode, ErrorReason, Response FROM Commands WHERE 0";
		try
		{
			sqlitexx::Command::ExecuteQuery(connection.GetRawDbObj(), query);
		}
		catch (std::exception& ex)
		{
			LOG_ERROR_APP("Verify failed for='" << query << "' error=" << ex.what());
			throw;
		}
	}

	{
		const char* query = "SELECT CommandID, ParameterCode, ParameterValue FROM CommandParameters WHERE 0";
		try
		{
			sqlitexx::Command::ExecuteQuery(connection.GetRawDbObj(), query);
		}
		catch(std::exception& ex)
		{
			LOG_ERROR_APP("Verify failed for='" << query << "' error=" << ex.what());
			throw;
		}
	}
}


//create
void SqliteCommandsDal::CreateCommand(DBCommand& command)
{
	LOG_DEBUG_APP("[CommandsDAL]: Create command:" << command);
	sqlitexx::Command sqlCommand(connection, ""
		"INSERT INTO Commands (DeviceID, CommandCode, CommandStatus, TimePosted, ErrorCode, Generated) "
		"VALUES (?001, ?002, ?003, ?004, ?005, ?006);");
	sqlCommand.BindParam(1, command.deviceID);
	sqlCommand.BindParam(2, (int)command.commandCode);
	sqlCommand.BindParam(3, (int)command.commandStatus);
	sqlCommand.BindParam(4, command.timePosted);
	sqlCommand.BindParam(5, 0);
	sqlCommand.BindParam(6, (int)command.generatedType);
	sqlCommand.ExecuteNonQuery();
	command.commandID = sqlCommand.GetLastInsertRowID();

	CreateCommandParameters(command.commandID, command.parameters);
}

void SqliteCommandsDal::CreateCommandParameters(int commandID, DBCommand::ParametersList& commandParameters)
{
	LOG_DEBUG_APP("[CommandsDAL]: Create parameters for command with ID:" << commandID);

	for (DBCommand::ParametersList::const_iterator it = commandParameters.begin(); it != commandParameters.end(); it++)
	{
		sqlitexx::Command sqlCommand(connection, ""
			"INSERT INTO CommandParameters (ParameterCode, ParameterValue, CommandID) "
			"VALUES (?001, ?002, ?003)");

		sqlCommand.BindParam(1, (int)it->parameterCode);
		sqlCommand.BindParam(2, it->parameterValue);
		sqlCommand.BindParam(3, commandID);

		sqlCommand.ExecuteNonQuery();
	}
}

//get
bool SqliteCommandsDal::GetCommand(int commandID, DBCommand& command)
{
	LOG_DEBUG_APP("[CommandsDAL]: Try to find command with ID:" << commandID);

	sqlitexx::CSqliteStmtHelper oSql (connection);


	if (oSql.Prepare(
				"SELECT DeviceID, CommandCode, CommandStatus, TimePosted, TimeResponsed, ErrorCode, ErrorReason, Response, Generated "
				"  FROM Commands WHERE CommandID = ?001;") != SQLITE_OK)
	{
		return false;
	}

	oSql.BindInt(1, commandID);

	if ( oSql.Step_GetRow() != SQLITE_ROW)
	{
		return false;
	}
		
	command.commandID = commandID;
	command.deviceID = oSql.Column_GetInt(0);
	command.commandCode = (DBCommand::CommandCode)oSql.Column_GetInt(1);
	command.commandStatus = (DBCommand::CommandStatus)oSql.Column_GetInt(2);
	command.timePosted = oSql.Column_GetDateTime(3);
	if ( !oSql.Column_IsNull(4))
	{
		command.timeResponded = oSql.Column_GetDateTime(4);
	}
	command.errorCode =(DBCommand::ResponseStatus)oSql.Column_GetInt(5);
	if ( !oSql.Column_IsNull(6))
	{
		command.errorReason = oSql.Column_GetText(6);
	}
	if ( !oSql.Column_IsNull(7))
	{
		command.response = oSql.Column_GetText(7);
	}
	command.generatedType = (DBCommand::CommandGeneratedType)oSql.Column_GetInt(8);
	return true;
}

void SqliteCommandsDal::GetCommandParameters(int commandID, DBCommand::ParametersList& commandParameters)
{
	LOG_DEBUG_APP("[CommandsDAL]: Get parameters for command with ID:" << commandID);

	sqlitexx::CSqliteStmtHelper oSql (connection);

	if (oSql.Prepare(
		"SELECT ParameterCode, ParameterValue "
		"  FROM CommandParameters WHERE CommandID = ?001;") != SQLITE_OK)
	{
		return;
	}

	oSql.BindInt(1, commandID);

	if (oSql.Step_GetRow() != SQLITE_ROW)
	{
		return;
	}

	commandParameters.reserve(8);

	do 
	{
		DBCommandParameter cp;
		cp.parameterCode = (DBCommandParameter::ParameterCode)oSql.Column_GetInt(0);
		cp.parameterValue = oSql.Column_GetText(1);

		commandParameters.push_back(cp);
	}
	while(oSql.Step_GetRow() == SQLITE_ROW);
}

void SqliteCommandsDal::GetNewCommands(DBCommandsList& newCommands)
{
	LOG_DEBUG_APP("[CommandsDAL]: Get all new commands from database");

	if (!m_oNewCommand_Compiled.GetStmt())
	{
		LOG_DEBUG_APP("m_oNewCommand_Compiled.Prepare");
		if( m_oNewCommand_Compiled.Prepare(
			"SELECT CommandID, DeviceID, CommandCode, CommandStatus, TimePosted, Generated "
			"  FROM Commands "
		    " WHERE CommandStatus = ?;") != SQLITE_OK)
		{
			LOG_ERROR_APP("m_oNewCommand_Compiled.Prepare -- SQLITE_ERROR=" << (int)SQLITE_OK);
			return;
		}

		LOG_INFO_APP("SqliteCommandsDal::GetNewCommands -- stmt compiled");
	}

	m_oNewCommand_Compiled.BindInt(1, (int)DBCommand::csNew);

	int nNewCmds = 0;

	std::list<DBCommand> oCmdList;
	while ( m_oNewCommand_Compiled.Step_GetRow() == SQLITE_ROW)
	{
		DBCommand cmd;

		m_oNewCommand_Compiled.Column_GetInt( 0, &cmd.commandID);
		m_oNewCommand_Compiled.Column_GetInt( 1, &cmd.deviceID);
		
		
		cmd.commandCode =(DBCommand::CommandCode) m_oNewCommand_Compiled.Column_GetInt( 2);
		cmd.commandStatus = (DBCommand::CommandStatus)m_oNewCommand_Compiled.Column_GetInt( 3);
		m_oNewCommand_Compiled.Column_GetDateTime( 4, &cmd.timePosted);
		cmd.generatedType = (DBCommand::CommandGeneratedType)m_oNewCommand_Compiled.Column_GetInt( 5);

		LOG_DEBUG_APP("GetNewCommands " << "cmdId=" << cmd.commandID << " deviceId=" << cmd.deviceID << " code=" << (int)cmd.commandCode << " status=" << (int)cmd.commandStatus
					);

		oCmdList.push_back(cmd);
		nNewCmds++;
	}


	std::list<DBCommand>::iterator itCmds = oCmdList.begin();
	for (;itCmds != oCmdList.end(); itCmds++)
	{
		GetCommandParameters((*itCmds).commandID, (*itCmds).parameters);

		newCommands.push_back((*itCmds));
	}

	if (nNewCmds)
	{
		LOG_DEBUG_APP(nNewCmds << " new commands found!"); 
	}
}

//set
void SqliteCommandsDal::SetCommandAsSent(DBCommand& command)
{
	LOG_DEBUG_APP("[CommandsDAL]: Update command:" << command.commandID << " as sent!");
	sqlitexx::Command sqlCommand(connection, ""
		"UPDATE Commands "
	    "   SET CommandStatus = ?001"
		" WHERE CommandID = ?002");
	sqlCommand.BindParam(1, (int)DBCommand::csSent);
	sqlCommand.BindParam(2, command.commandID);

	sqlCommand.ExecuteNonQuery();

	//now update also model
	command.commandStatus = DBCommand::csSent;
}

void SqliteCommandsDal::SetCommandAsResponded(DBCommand& command, const nlib::DateTime& responseTime, const std::string& response)
{
	LOG_DEBUG_APP("[CommandsDAL]: Update command:" << command.commandID << " as responded!");
	sqlitexx::Command sqlCommand(connection, ""
		"UPDATE Commands "
	    "   SET CommandStatus = ?001, TimeResponsed = ?002, ErrorCode = ?003, Response = ?004 "
		" WHERE CommandID = ?005;");

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

void SqliteCommandsDal::SetCommandAsFailed(DBCommand& command, const nlib::DateTime& errorTime, int errorCode,
		const std::string& errorReason)
{
	LOG_DEBUG_APP("[CommandsDAL]: Update command:" << command.commandID << " as failed");
	sqlitexx::Command sqlCommand(connection, ""
		"UPDATE Commands "
	    "   SET CommandStatus = ?001, TimeResponsed = ?002, ErrorCode = ?003, ErrorReason = ?004 "
		" WHERE CommandID = ?005;");

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


void SqliteCommandsDal::SetDBCmdIDAsNew(int DBCmsID)
{
	LOG_DEBUG_APP("[CommandsDAL]: Update DBCmdID:" << (boost::int32_t)DBCmsID << " as new");
	sqlitexx::Command sqlCommand(connection, ""
		"UPDATE Commands "
	    "   SET CommandStatus = ?001 "
		" WHERE CommandID = ?002;");

	sqlCommand.BindParam(1, (int)DBCommand::csNew);
	sqlCommand.BindParam(2, DBCmsID);

	sqlCommand.ExecuteNonQuery();
}


}//hostapp
}//hart7
