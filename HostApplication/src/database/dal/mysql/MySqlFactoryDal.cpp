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


#include "MySqlFactoryDal.h"


namespace hart7 {
namespace hostapp {

MySqlFactoryDal::MySqlFactoryDal() :
	transaction(connection), commandsDal(connection), devicesDal(connection)
{
}

MySqlFactoryDal::~MySqlFactoryDal()
{
	connection.Close();
}


//init
void MySqlFactoryDal::Open(const std::string& serverName, const std::string& user, const std::string& password,
  const std::string& dbName, int timeoutSeconds)
{

	LOG_INFO_APP("Conecting to server:" << serverName << " database:" << dbName);

	connection.ConnectionString(serverName, user, password, dbName);
	connection.Open();

	LOG_INFO_APP("Connection opened...");

	VerifyDb();
}
void MySqlFactoryDal::VerifyDb()
{
	commandsDal.VerifyTables();
	devicesDal.VerifyTables();
	LOG_DEBUG_APP("Tables structure ok!");
}


//transaction
void MySqlFactoryDal::BeginTransaction()
{
	transaction.Begin();
}
void MySqlFactoryDal::CommitTransaction()
{
	transaction.Commit();
}
void MySqlFactoryDal::RollbackTransaction()
{
	transaction.Rollback();
}


//entities
ICommandsDal& MySqlFactoryDal::Commands() const
{
	MySqlFactoryDal* pThis = const_cast<MySqlFactoryDal*>(this);
	return pThis->commandsDal;
}
IDevicesDal& MySqlFactoryDal::Devices() const
{
	MySqlFactoryDal* pThis = const_cast<MySqlFactoryDal*>(this);
	return pThis->devicesDal;
}

//database
void MySqlFactoryDal::VacuumDatabase()
{
	
}
void MySqlFactoryDal::CleanupOldRecords(nlib::DateTime olderThanDate, int maxCount)
{
	const char *pszQry[] ={
		"DELETE FROM CommandParameters WHERE CommandID IN (SELECT C.CommandID from Commands C WHERE C.TimePosted < \"%s\");",	// +24*7
		"DELETE FROM Commands WHERE TimePosted < \"%s\";",	// +24*7
		"DELETE FROM Alarms WHERE AlarmTime < \"%s\";",	// +24*7
		"DELETE FROM CommandParameters WHERE CommandID IN (SELECT C.CommandID from Commands C WHERE C.TimePosted < \"%s\" AND (C.CommandStatus = 2 OR C.CommandStatus = 3));",
		"DELETE FROM Commands WHERE TimePosted < \"%s\" AND (CommandStatus = 2 OR CommandStatus = 3);",
		"DELETE FROM DeviceHistory WHERE Timestamp < \"%s\";",
		"DELETE FROM ChannelsHistory WHERE Timestamp < \"%s\";" };

	LOG_DEBUG_APP("CleanupOldRecords START [" << nlib::ToString(olderThanDate) << "] [" << nlib::ToString(nlib::CurrentUniversalTime() - nlib::util::hours(24*7)) <<"]");
	for( int i = 0; i< sizeof(pszQry) / sizeof(pszQry[0]); ++i)
	{	char szQry[ 1024 ];
		snprintf( szQry, sizeof(szQry), pszQry[i], (i>=2)
			? nlib::ToString(olderThanDate).c_str() 
			: (nlib::ToString(nlib::CurrentUniversalTime() - nlib::util::hours(24*7)).c_str()) );
		szQry[ sizeof(szQry)-1 ] = 0;
		std::string obQry( szQry );
		MySQLCommand command(connection, obQry);
		try
		{
			BeginTransaction();
			command.ExecuteNonQuery();
			CommitTransaction();
		}
		catch(std::exception& ex)
		{
			LOG_ERROR_APP("CATCH(exception): CleanupOldRecords failed at step=" << i << " error=" << ex.what());
			RollbackTransaction();
		}
		catch(...)
		{
			LOG_ERROR_APP("CATCH(...): CleanupOldRecords failed at step=" << i );
			RollbackTransaction();
		}
	}
	LOG_DEBUG_APP("CleanupOldRecords END");
}

} // namespace hostapp
} // namespace hart7

#endif
