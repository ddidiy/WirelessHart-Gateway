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


#include <WHartHost/database/DBCommandsManager.h>
#include "dal/IFactoryDal.h"
#include "EntitiesStore.h"

namespace hart7 {
namespace hostapp {


EntitiesStore::StoreType convert(int val)
{
	switch(val)
	{
	case STORE_TYPE_MEM:
		return EntitiesStore::StoreType_Mem;
	case STORE_TYPE_SQLITE:
		return EntitiesStore::StoreType_SQLite;
	case STORE_TYPE_MYSQL:
		return EntitiesStore::StoreType_MySQL;
	case STORE_TYPE_VIRT:
		return EntitiesStore::StoreType_Virt;
	default:
		return EntitiesStore::StoreType_Virt;
	}
}

DBCommandsManager::DBCommandsManager(const std::string &strParams, int StorageType) :
factoryDal(*EntitiesStore::GetInstance(strParams, convert(StorageType))->GetFactoryDal())
{
}

DBCommandsManager::~DBCommandsManager()
{
}

//observers
void DBCommandsManager::AddCmdResponseHandle(Loki::Function<void(const int, DBCommand::ResponseStatus, void*)> funct)
{
	m_receiveCommandResponse.push_back(funct);
}
void DBCommandsManager::FireReceiveCommandResponse(int commandID, DBCommand::ResponseStatus status, void* pData)
{
	for (std::vector<Loki::Function<void(const int, DBCommand::ResponseStatus, void*)> >::iterator it =
	    m_receiveCommandResponse.begin(); it != m_receiveCommandResponse.end(); it++)
	{
		try
		{
			(*it)(commandID, status, pData);
		}
		catch (std::exception& ex)
		{
			LOG_ERROR_APP("[DBCommand] fireReceiveCommandResponse: failed! error=" << ex.what());
		}
		catch (...)
		{
			LOG_ERROR_APP("[DBCommand] fireReceiveCommandResponse: failed! unknown error!");
		}
	}
}


//create
void DBCommandsManager::CreateCommand(DBCommand& newCommand, const std::string& createdReason)
{
	newCommand.commandStatus = DBCommand::csNew;
	newCommand.timePosted = nlib::CurrentUniversalTime();

	try
	{
		factoryDal.BeginTransaction();
		factoryDal.Commands().CreateCommand(newCommand);
		factoryDal.CommitTransaction();
//		LOG_INFO_APP("[DBCommand] createCommand: " << newCommand << " createdReason=" << createdReason);
	}
	catch (std::exception& ex)
	{
		LOG_ERROR_APP("[DBCommand] createCommand: failed! error=" << ex.what());
		factoryDal.RollbackTransaction();
		return;
	}
	catch (...)
	{
		LOG_ERROR_APP("[DBCommand] createCommand: failed! unknown error!");
		factoryDal.RollbackTransaction();
		return;
	}
	
}

void DBCommandsManager::CreateCommand(DBCommand& newCommand, const std::string& createdReason, int & currCmdID)
{
	newCommand.commandStatus = DBCommand::csNew;
	newCommand.timePosted = nlib::CurrentUniversalTime();

	try
	{
		factoryDal.BeginTransaction();
		factoryDal.Commands().CreateCommand(newCommand);
		factoryDal.CommitTransaction();
//		LOG_INFO_APP("[DBCommand] createCommand: " << newCommand << " createdReason=" << createdReason);
	}
	catch (std::exception& ex)
	{
		LOG_ERROR_APP("[DBCommand] createCommand: failed! error=" << ex.what());
		factoryDal.RollbackTransaction();
		return;
	}
	catch (...)
	{
		LOG_ERROR_APP("[DBCommand] createCommand: failed! unknown error!");
		factoryDal.RollbackTransaction();
		return;
	}
	
	currCmdID = newCommand.commandID;
}


//get
void DBCommandsManager::GetNewCommands(DBCommandsList& newCommands)
{
	factoryDal.Commands().GetNewCommands(newCommands);
}

bool DBCommandsManager::GetCommand(int commandID, DBCommand& command)
{
	try
	{
		if (factoryDal.Commands().GetCommand(commandID, command))
		{
			factoryDal.Commands().GetCommandParameters(commandID, command.parameters);
			return true;
		}
	}
	catch (std::exception& ex)
	{
		LOG_ERROR_APP("[DBCommand] getCommand: CommandID=" << commandID << " failed. error=" << ex.what());
	}
	catch (...)
	{
		LOG_ERROR_APP("[DBCommand] getCommand: CommandID=" << commandID << " failed. unknown error!");
	}
	return false;
}


//set
void DBCommandsManager::SetCommandSent(DBCommand& command)
{
	LOG_INFO_APP("[DBCommand] setCommandSent: Command=" << command);

	try
	{
		factoryDal.Commands().SetCommandAsSent(command);
	}
	catch(std::exception& ex)
	{
		LOG_ERROR_APP("[DBCommand] setCommandSent: Command=" << command << " failed! error=" << ex.what());
	}
}
void DBCommandsManager::SetCommandResponded(DBCommand& command, const nlib::DateTime& respondedTime,
  const std::string& response, void* pData)
{
	LOG_INFO_APP("[DBCommand] setCommandResponded: Command=" << command << " response=" << response);

	FireReceiveCommandResponse(command.commandID, DBCommand::rsSuccess, pData);
	try
	{
		if (command.commandID != DBCommand::NO_COMMAND_ID)
			factoryDal.Commands().SetCommandAsResponded(command, respondedTime, response);

	}
	catch(std::exception& ex)
	{
		LOG_ERROR_APP("[DBCommand] setCommandResponded: Command=" << command << " failed! error=" << ex.what());
	}
}
void DBCommandsManager::SetCommandFailed(DBCommand& command, const nlib::DateTime& failedTime,
  DBCommand::ResponseStatus errorCode, const std::string& errorReason)
{
	LOG_WARN_APP("[DBCommand] setCommandFailed: Command=" << command << " errorCode=" << errorCode << " errorReason="
	    << errorReason);

	FireReceiveCommandResponse(command.commandID, errorCode, NULL);
	try
	{
		if (command.commandID != DBCommand::NO_COMMAND_ID)
			factoryDal.Commands().SetCommandAsFailed(command, failedTime, errorCode, errorReason);
	}
	catch (std::exception& ex)
	{
		LOG_ERROR_APP("[DBCommand] setCommandFailed: Command=" << command << " failed! error=" << ex.what());
	}
}
void DBCommandsManager::SetDBCmdIDAsNew(int dbCmdID)
{
	try
	{
		factoryDal.Commands().SetDBCmdIDAsNew(dbCmdID);
	}
	catch(std::exception& ex)
	{
		LOG_ERROR_APP("[DBCommand] setCommandNew: DBCmdID=" << (boost::int32_t)dbCmdID << " failed! error=" << ex.what());
	}
}

//clean-up
void DBCommandsManager::VacuumDatabase()
{
	factoryDal.VacuumDatabase();
}
void DBCommandsManager::CleanupOldRecords(nlib::DateTime olderThanDate, int maxCount)
{
	factoryDal.CleanupOldRecords(olderThanDate, maxCount);
}


} //namespace hostapp
} // namespace hart7
