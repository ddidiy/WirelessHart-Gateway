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

#ifndef SQLITEFACTORYDAL_H_
#define SQLITEFACTORYDAL_H_

//#include <nlib/sqlitexx/Connection.h>
#include "local/Connection.h"

#include "../IFactoryDal.h"
#include "SqliteCommandsDal.h"
#include "SqliteDevicesDal.h"

namespace hart7 {
namespace hostapp {

class SqliteFactoryDal : public IFactoryDal
{
public:
	SqliteFactoryDal();
	virtual ~SqliteFactoryDal();
	
//init
public:
	void Open(const std::string& dbPath, int timeoutSeconds = 10);
	void VerifyDb();

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
public:
	void VacuumDatabase();	
	void CleanupOldRecords(nlib::DateTime olderThanDate, int maxCount);

//
private:
	sqlitexx::Connection connection;
	sqlitexx::Transaction transaction;

	SqliteCommandsDal commandsDal;
	SqliteDevicesDal	devicesDal;
};

} //namespace hostapp
} //namespace hart7

#endif /*SQLITEFACTORYDAL_H_*/
