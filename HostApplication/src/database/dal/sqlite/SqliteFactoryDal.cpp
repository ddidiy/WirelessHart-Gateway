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


#include "SqliteFactoryDal.h"
#include <stdio.h>

namespace hart7 {
namespace hostapp {

SqliteFactoryDal::SqliteFactoryDal() :
	transaction(connection), commandsDal(connection), devicesDal(connection)
{
}

SqliteFactoryDal::~SqliteFactoryDal()
{
	connection.Close();
}


//init
void SqliteFactoryDal::Open(const std::string& dbPath, int timeoutSeconds)
{
	connection.Open(dbPath, timeoutSeconds);
	VerifyDb();
}
void SqliteFactoryDal::VerifyDb()
{
	commandsDal.VerifyTables();
	devicesDal.VerifyTables();
}

//transaction
void SqliteFactoryDal::BeginTransaction()
{
	transaction.Begin();
}
void SqliteFactoryDal::CommitTransaction()
{
	transaction.Commit();
}
void SqliteFactoryDal::RollbackTransaction()
{
	transaction.Rollback();
}

//entities
ICommandsDal& SqliteFactoryDal::Commands() const
{
	SqliteFactoryDal* pThis = const_cast<SqliteFactoryDal*>(this);
	return pThis->commandsDal;
}
IDevicesDal& SqliteFactoryDal::Devices() const
{
	SqliteFactoryDal* pThis = const_cast<SqliteFactoryDal*>(this);
	return pThis->devicesDal;
}

//database
void SqliteFactoryDal::VacuumDatabase()
{
	//std::string vacuum = "VACUUM";
	//sqlitexx::Command(connection, vacuum).ExecuteNonQuery();
}
void SqliteFactoryDal::CleanupOldRecords(nlib::DateTime olderThanDate, int maxCount)
{
	const char *pszQry[] ={
//		"DELETE FROM CommandParameters WHERE CommandID IN (SELECT C.CommandID from Commands C WHERE C.TimePosted < \"%s\");",	// +24*7
//		"DELETE FROM Commands WHERE TimePosted < \"%s\";",	// +24*7
//		"DELETE FROM Alarms WHERE AlarmTime < \"%s\";",	// +24*7
//		"DELETE FROM CommandParameters WHERE CommandID IN (SELECT C.CommandID from Commands C WHERE C.TimePosted < \"%s\" AND (C.CommandStatus = 2 OR C.CommandStatus = 3));",
//		"DELETE FROM Commands WHERE TimePosted < \"%s\" AND (CommandStatus = 2 OR CommandStatus = 3);",
//		"DELETE FROM DeviceHistory WHERE Timestamp < \"%s\";",
		"DELETE FROM ChannelsHistory WHERE Timestamp < \"%s\";" };

	LOG_DEBUG_APP("CleanupOldRecords START [" << nlib::ToString(olderThanDate) << "] [" << nlib::ToString(nlib::CurrentUniversalTime() - nlib::util::hours(24*7)) <<"]");
	for(unsigned int i = 0; i< sizeof(pszQry) / sizeof(pszQry[0]); ++i)
	{	char szQry[ 1024 ];
		snprintf( szQry, sizeof(szQry), pszQry[i], (i>=3)
			? nlib::ToString(olderThanDate).c_str() 
			: (nlib::ToString(nlib::CurrentUniversalTime() - nlib::util::hours(24*7)).c_str()) );
		szQry[ sizeof(szQry)-1 ] = 0;
		std::string obQry( szQry );
		sqlitexx::Command command(connection, obQry);
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

	// keeps only last 5 history records for each device
	{
		sqlitexx::CSqliteStmtHelper oSqlStmtHlp (connection);
		int rez = SQLITE_OK;
		if ((rez = oSqlStmtHlp.Prepare("SELECT h.DeviceID, "
											  "CASE WHEN d.DeviceStatus IS NULL OR d.DeviceStatus = 0 THEN -1 ELSE "
											  "(SELECT MIN(tt.HistoryID) FROM (SELECT t.HistoryID, t.DeviceID FROM DeviceHistory t WHERE t.DeviceID = h.DeviceID ORDER BY t.HistoryID DESC LIMIT 5) tt) "
											  "END MinHistoryID "
									   "  FROM DeviceHistory h LEFT JOIN Devices d ON d.DeviceID = h.DeviceID GROUP BY h.DeviceID;")) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR("[DevicesDAL]: CleanupOldRecords - 'Clean DeviceHistory' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}

		if (oSqlStmtHlp.Step_GetRow() != SQLITE_ROW)
			return;

		try
		{
			BeginTransaction();
			do
			{
				char szQueryString[1024];
				if (oSqlStmtHlp.Column_GetInt(1) == -1)
				{
					snprintf(szQueryString, sizeof(szQueryString), "DELETE FROM DeviceHistory WHERE DeviceID = %d AND Timestamp < \"%s\";",
							oSqlStmtHlp.Column_GetInt(0), nlib::ToString(olderThanDate).c_str());
				}
				else
				{
					snprintf(szQueryString, sizeof(szQueryString), "DELETE FROM DeviceHistory WHERE DeviceID = %d AND HistoryID < %d AND Timestamp < \"%s\";",
							oSqlStmtHlp.Column_GetInt(0), oSqlStmtHlp.Column_GetInt(1), nlib::ToString(olderThanDate).c_str());
				}
				szQueryString[sizeof(szQueryString)-1] = 0;
				std::string queryString(szQueryString);

				sqlitexx::Command command(connection, queryString);
				command.ExecuteNonQuery();
			}
			while (oSqlStmtHlp.Step_GetRow() == SQLITE_ROW);
			CommitTransaction();
		}
		catch(std::exception& ex)
		{
			LOG_ERROR_APP("CATCH(exception): CleanupOldRecords failed when cleans DeviceHistory. Error=" << ex.what());
			RollbackTransaction();
		}
		catch(...)
		{
			LOG_ERROR_APP("CATCH(...): CleanupOldRecords failed when cleans DeviceHistory");
			RollbackTransaction();
		}
	}

    // keeps maxCount records in table Alarms
    {
        sqlitexx::CSqliteStmtHelper oSqlStmtHlp (connection);
        int rez = SQLITE_OK;
        if ((rez = oSqlStmtHlp.Prepare("SELECT MAX(AlarmID) MaxAlarmID, COUNT(*) AlarmCount FROM Alarms;")) != SQLITE_OK)
        {
            connection.LogIfLastError(rez);
            LOG_ERROR("[DevicesDAL]: CleanupOldRecords - 'Clean Alarms' FAILED, err=" << rez);
            std::exception ex;
            throw ex;
        }

        if (oSqlStmtHlp.Step_GetRow() != SQLITE_ROW)
            return;

        try
        {
            if (oSqlStmtHlp.Column_GetInt(1) > maxCount)
            {
                char szQueryAlarms[128];
                snprintf(szQueryAlarms, sizeof(szQueryAlarms), "DELETE FROM Alarms WHERE AlarmID < %d;", oSqlStmtHlp.Column_GetInt(0) - maxCount + 1);
                szQueryAlarms[sizeof(szQueryAlarms)-1] = 0;
                std::string queryAlarms(szQueryAlarms);

                sqlitexx::Command command(connection, queryAlarms);
                command.ExecuteNonQuery();
            }
        }
        catch(std::exception& ex)
        {
            LOG_ERROR_APP("CATCH(exception): CleanupOldRecords failed when cleans Alarms. Error=" << ex.what());
            RollbackTransaction();
        }
        catch(...)
        {
            LOG_ERROR_APP("CATCH(...): CleanupOldRecords failed when cleans Alarms");
            RollbackTransaction();
        }
    }

    // keeps maxCount records in table Commands
    {
        sqlitexx::CSqliteStmtHelper oSqlStmtHlp (connection);
        int rez = SQLITE_OK;
        if ((rez = oSqlStmtHlp.Prepare("SELECT MAX(CommandID) MaxCommandID, COUNT(*) CommandCount FROM Commands;")) != SQLITE_OK)
        {
            connection.LogIfLastError(rez);
            LOG_ERROR("[DevicesDAL]: CleanupOldRecords - 'Clean Commands' FAILED, err=" << rez);
            std::exception ex;
            throw ex;
        }

        if (oSqlStmtHlp.Step_GetRow() != SQLITE_ROW)
            return;

        try
        {
            if (oSqlStmtHlp.Column_GetInt(1) > maxCount)
            {
                char szQueryCommands[128];
                snprintf(szQueryCommands, sizeof(szQueryCommands), "DELETE FROM Commands WHERE CommandID < %d;", oSqlStmtHlp.Column_GetInt(0) - maxCount + 1);
                szQueryCommands[sizeof(szQueryCommands)-1] = 0;
                std::string queryCommands(szQueryCommands);

                sqlitexx::Command command1(connection, queryCommands);
                command1.ExecuteNonQuery();

                char szQueryParams[128];
                snprintf(szQueryParams, sizeof(szQueryParams), "DELETE FROM CommandParameters WHERE CommandID < %d;", oSqlStmtHlp.Column_GetInt(0) - maxCount + 1);
                szQueryParams[sizeof(szQueryParams)-1] = 0;
                std::string queryParams(szQueryParams);

                sqlitexx::Command command2(connection, queryParams);
                command2.ExecuteNonQuery();
            }
        }
        catch(std::exception& ex)
        {
            LOG_ERROR_APP("CATCH(exception): CleanupOldRecords failed when cleans Alarms. Error=" << ex.what());
            RollbackTransaction();
        }
        catch(...)
        {
            LOG_ERROR_APP("CATCH(...): CleanupOldRecords failed when cleans Alarms");
            RollbackTransaction();
        }
    }

	LOG_DEBUG_APP("CleanupOldRecords END");
}

} // namespace hostapp
} // namespace hart7
