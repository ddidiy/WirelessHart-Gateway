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

#ifndef COMMANDSMANAGER_H_
#define COMMANDSMANAGER_H_


#include <WHartHost/model/DBCommand.h>
#include <vector>
//#include <boost/function.hpp> //for callback
#include <loki/Function.h> //for callback
#include <nlib/log.h>


#include "CommonManager.h"


namespace hart7 {
namespace hostapp {
class IFactoryDal;


/**
 * it manages dbcommands in db
 */
class DBCommandsManager
{
	LOG_DEF("hart7.hostapp.CommandsManager");

public:
	DBCommandsManager(const std::string &strParams="", int StorageType = STORE_TYPE_VIRT);
	virtual ~DBCommandsManager();


//observers
public:
	void AddCmdResponseHandle(Loki::Function<void(const int, DBCommand::ResponseStatus, void*)> funct);
private:
	std::vector<Loki::Function<void(const int, DBCommand::ResponseStatus, void*)> > m_receiveCommandResponse;
	void FireReceiveCommandResponse(int commandID, DBCommand::ResponseStatus status, void* pData);

//create
public:
	void CreateCommand(DBCommand& newCommand, const std::string& createdReason);
	void CreateCommand(DBCommand& newCommand, const std::string& createdReason, int & currCmdID);

//get
public:
	bool GetCommand(int commandID, DBCommand& command);
	void GetNewCommands(DBCommandsList& newCommands);

//set
public:
	void SetCommandSent(DBCommand& command);
	void SetCommandResponded(DBCommand& command, const nlib::DateTime& respondedTime, const std::string& response, void* pData);
	void SetCommandFailed(DBCommand& command, const nlib::DateTime& failedTime, DBCommand::ResponseStatus errorCode,
			const std::string& errorReason);
	void SetDBCmdIDAsNew(int dbCmdID);

//clean-up
	void VacuumDatabase();
	void CleanupOldRecords(nlib::DateTime olderThanDate, int maxCount);

//
private:
	IFactoryDal& factoryDal;
};

} //namespace hostapp
} //namespace hart7

#endif /*COMMANDSMANAGER_H_*/
