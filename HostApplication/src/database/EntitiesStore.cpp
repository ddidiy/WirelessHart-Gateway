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


#include <nlib/log.h>


#include "EntitiesStore.h"

#include "src/database/dal/sqlite/SqliteFactoryDal.h"
#ifndef HW_VR900
#ifndef MIPS
#include "src/database/dal/mysql/MySqlFactoryDal.h"
#endif
#endif
#include "src/database/dal/memory/MemoryFactoryDal.h"



namespace hart7 {
namespace hostapp {

static EntitiesStore *pobj_g = NULL;


EntitiesStore::EntitiesStore(const std::string &strParams, StoreType st)
{
	if (st == StoreType_Mem)
	{
		m_pFactoryDAL = new MemoryFactoryDal();
	}
	else if (st == StoreType_SQLite)
	{
		
		LOG_DEBUG_APP("reading params=" << strParams);
		char dbFilePath[255]="";
		unsigned int dbTimedOut= 40/*sec*/;
		sscanf(strParams.c_str(), "%[^,] , %u", dbFilePath, &dbTimedOut);

		LOG_DEBUG_APP("Connect to the sqlite database: file=" << dbFilePath << "timedout=" << dbTimedOut);

		m_pFactoryDAL = new SqliteFactoryDal();
		((SqliteFactoryDal*)m_pFactoryDAL)->Open(dbFilePath, dbTimedOut);
	}
	else if (st == StoreType_MySQL)
	{
#ifndef HW_VR900
#ifndef MIPS
		char dbServerIP[255]="";
		char dbUserName[255]="";
		char dbPassword[255]="";
		char dbName[255]="";
		unsigned int dbRetryInterval = 60/*sec*/;
		sscanf(strParams.c_str(), "%[^,] , %[^,] , %[^,] , %[^,] , %u", dbServerIP, dbUserName, dbPassword, dbName, &dbRetryInterval);

		LOG_DEBUG_APP("Connect to the mysql database: server=" << dbServerIP << "retry_interval=" << dbRetryInterval);

		m_pFactoryDAL = new MySqlFactoryDal();
		bool connectedToDatabase = false;
		while(!connectedToDatabase)
		{
			try
			{
				((MySqlFactoryDal*)m_pFactoryDAL)->Open(dbServerIP, dbUserName, dbPassword, dbName);
				connectedToDatabase = true;
			}
			catch(std::exception& ex)
			{
				LOG_ERROR_APP("Cannot connect to the database. Error=" << ex.what());
				LOG_DEBUG_APP("Retry to reconnect to the database in " << dbRetryInterval << " seconds");
				sleep(dbRetryInterval);
			}
			catch(...)
			{
				LOG_ERROR_APP("Unknown exception occured when trying to connect to the database!");
				LOG_DEBUG_APP("Retry to reconnect to the database in " << dbRetryInterval << " seconds");
				sleep(dbRetryInterval);
			}
		}
#endif
#endif
	}
	else if (st == StoreType_Virt)
	{
		//TO DO...
	}


}
EntitiesStore::~EntitiesStore()
{
	delete m_pFactoryDAL;
}


//instance
EntitiesStore* EntitiesStore::GetInstance(const std::string &strParams, StoreType st)
{

	if (st < StoreType_Mem || st > StoreType_Virt)
	{
		LOG_ERROR_APP("[EntitiesStore]: invalid selection");
		throw;
	}

	static EntitiesStore *pobj = new EntitiesStore(strParams, st);
	pobj_g = pobj;
	return pobj;
}
void EntitiesStore::DeleteInstance()
{
	if (pobj_g != NULL)
	{
		delete pobj_g;
		pobj_g = NULL;
	}
}



}
}
