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


#ifndef MYSQLFACTORYDAL_H_
#define MYSQLFACTORYDAL_H_

#include "../IFactoryDal.h"
#include "MySQLDatabase.h"

#include "MySqlCommandsDal.h"
#include "MySqlDevicesDal.h"

namespace hart7 {
namespace hostapp {

class MySqlFactoryDal : public IFactoryDal
{
	LOG_DEF("hart7.hostapp.MySqlFactoryDal");
public:
	MySqlFactoryDal();
	virtual ~MySqlFactoryDal();

//init
public:
	void VerifyDb();
	void Open(const std::string& serverName, const std::string& user, const std::string& password, const std::string& dbName, int timeoutSeconds = 10);

//transaction
private:
	void BeginTransaction();
	void CommitTransaction();
	void RollbackTransaction();

//entities
private:
	ICommandsDal& Commands() const;
	IDevicesDal& Devices() const;

//database
private:
	void VacuumDatabase();
	void CleanupOldRecords(nlib::DateTime olderThanDate, int maxCount);

//
private:
	MySQLConnection connection;
	MySQLTransaction transaction;

	MySqlCommandsDal commandsDal;
	MySqlDevicesDal	devicesDal;

};

} //namespace hostapp
} //namespace hart7


#endif /*MYSQLFACTORYDAL_H_*/

#endif
