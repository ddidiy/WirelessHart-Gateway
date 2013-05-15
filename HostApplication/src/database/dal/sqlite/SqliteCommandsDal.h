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

#ifndef SQLITECOMMANDSDAL_H_
#define SQLITECOMMANDSDAL_H_

#include "local/Connection.h"

#include <nlib/log.h>

#include "../ICommandsDal.h"

namespace hart7 {
namespace hostapp {

class SqliteCommandsDal : public ICommandsDal
{
	
public:
	SqliteCommandsDal(sqlitexx::Connection& connection);
	virtual ~SqliteCommandsDal();

//init
public:
	void VerifyTables();

//create
private:
	void CreateCommand(DBCommand& command);
	void CreateCommandParameters(int commandID, DBCommand::ParametersList& commandParameters);
	
//get
private:
	bool GetCommand(int commandID, DBCommand& command);
	void GetNewCommands(DBCommandsList& newCommands);
	void GetCommandParameters(int commandID, DBCommand::ParametersList& commandParameters);

//set
private:
	void SetCommandAsSent(DBCommand& command);
	void SetCommandAsResponded(DBCommand& command, const nlib::DateTime& responseTime, const std::string& response);
	void SetCommandAsFailed(DBCommand& command, const nlib::DateTime& failedTime, int errorCode, const std::string& errorReason);
	void SetDBCmdIDAsNew(int DBCmsID);

//
private:
	sqlitexx::Connection& connection;
	sqlitexx::CSqliteStmtHelper m_oNewCommand_Compiled;
};

} //namespace hostapp
} //namespace hart7

#endif /*SQLITECOMMANDSDAL_H_*/
