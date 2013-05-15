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

#ifndef ICOMMANDSDAL_H_
#define ICOMMANDSDAL_H_

#include <WHartHost/model/DBCommand.h>

namespace hart7 {
namespace hostapp {

class ICommandsDal
{
public:
	virtual ~ICommandsDal()
	{
	}

//create
public:
	virtual void CreateCommand(DBCommand& command) = 0;
	virtual void CreateCommandParameters(int commandID, DBCommand::ParametersList& commandParameters) = 0;

//get
public:
	virtual bool GetCommand(int commandID, DBCommand& command) = 0;
	virtual void GetCommandParameters(int commandID, DBCommand::ParametersList& commandParameters) = 0;
	virtual void GetNewCommands(DBCommandsList& newCommands) = 0;

//set
public:
	virtual void SetCommandAsSent(DBCommand& command) = 0;
	virtual void SetCommandAsResponded(DBCommand& command, const nlib::DateTime& responseTime, const std::string& response) = 0;
	virtual void SetCommandAsFailed(DBCommand& command, const nlib::DateTime& failedTime, int errorCode, const std::string& errorReason) = 0;
	virtual void SetDBCmdIDAsNew(int DBCmsID) = 0;
};

} // namespace hostapp
} // namespace hart7


#endif /*ICOMMANDSDAL_H_*/
