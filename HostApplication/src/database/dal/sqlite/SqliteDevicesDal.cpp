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

#include <stdio.h>
#include <list>
#include "SqliteDevicesDal.h"

#include <float.h>
#include <limits>

#include <WHartHost/Utils.h>



namespace hart7 {
namespace hostapp {

SqliteDevicesDal::SqliteDevicesDal(sqlitexx::Connection& connection_) :
		connection(connection_), 
		m_oUpdateDeviceReadings_Prepare(connection_), 
		m_oUpdatePublishFlag_compiled(connection_),
		m_oUpdateLastReadAndPublishFlag_compiled(connection_),
		m_oInsertGraph_compiled(connection_),
		m_oDeleteGraph_compiled(connection_),
		m_oInsertRoute_compiled(connection_),
		m_oDeleteRoute_compiled(connection_),
		m_oInsertSourceRoute_compiled(connection_),
		m_oDeleteSourceRoute_compiled(connection_),
		m_oInsertService_compiled(connection_),
		m_oDeleteService_compiled(connection_),
		m_oInsertSuperframes_compiled(connection_),
		m_oDeleteSuperframes_compiled(connection_),
		m_oInsertScheduleLink_compiled(connection_),
		m_oDeleteScheduleLink1_compiled(connection_),
		m_oDeleteScheduleLink2_compiled(connection_),
		m_oInsertDeviceHealth_compiled(connection_),
		m_oDeleteDeviceHealth_compiled(connection_),
		m_oInsertNeighbsHealth_compiled(connection_),
		m_oDeleteNeighbsHealth_compiled(connection_),
		m_oInsertNeighbsSigLevel_compiled(connection_),
		m_oDeleteNeighbsSigLevel_compiled(connection_),
        m_oInsertBurstMessageCounter_compiled(connection_),
        m_oUpdateBurstMessageCounter_compiled(connection_),
        m_oDeleteBurstMessageCounter_compiled(connection_),
        m_oDeleteBurstMessageCounters_compiled(connection_),
        m_oUpdateSetPublishersLog_compiled(connection_),
        m_oDeleteSetPublishersLog_compiled(connection_)
{
}

SqliteDevicesDal::~SqliteDevicesDal()
{
}

//init
void SqliteDevicesDal::VerifyTables()
{
	LOG_INFO_APP("[DevicesDAL]: Verify devices tables structure...");
	
	//devices
	{
		const char* query = "SELECT DeviceID, DeviceRole, DeviceCode, SoftwareRevision, Address64, DeviceTag, Nickname, DeviceStatus, "
		                    "       DeviceLevel, RejoinCount, LastRead, PowerSupplyStatus, PublishStatus "
		                    "  FROM Devices WHERE 0";
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
	{
		const char* query = "SELECT HistoryID, DeviceID, Timestamp, DeviceStatus FROM DeviceHistory WHERE 0";
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
	{
		const char* query = "SELECT DeviceID, IP, Port FROM DeviceConnections WHERE 0";
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

	//readings
	{
		const char* query = "SELECT ChannelID, DeviceID, BurstMessage, DeviceVariableSlot, Name, CmdNo, DeviceVariable, Classification, UnitCode "
			                "  FROM Channels WHERE 0";
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
	{
		const char*
		    query = "SELECT ChannelID, DeviceID, BurstMessage, DeviceVariableSlot, Name, CmdNo, DeviceVariable, Classification, "
		            "       UnitCode, Timestamp"
			        "  FROM ChannelsHistory WHERE 0";
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
	{
		const char* query = "SELECT ChannelID, ReadingTime, Miliseconds, Value, Status, CommandID FROM Readings WHERE 0";
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

	//commands
	{
		const char* query = "SELECT CommandID, DeviceID, CommandCode, CommandStatus, TimePosted, TimeResponsed, ErrorCode, ErrorReason, "
		                    "       Response, Generated, ParametersDescription "
		                    "  FROM Commands WHERE 0";
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

//devices
void SqliteDevicesDal::ResetDevices(Device::DeviceStatus newStatus)
{
	LOG_INFO_APP("[DevicesDAL]: Reset all devices from db");
	{
		sqlitexx::CSqliteStmtHelper oSqlDev (connection);
		int rez = SQLITE_OK;
		if ((rez = oSqlDev.Prepare( "UPDATE Devices SET DeviceStatus = ?001, PublishStatus = ?002")) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'ResetDevices->Prepare' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		// get
		sqlite3_stmt *stmt = NULL;
		if (!(stmt = oSqlDev.GetStmt()))
		{
			LOG_ERROR_APP("[DevicesDAL]: 'ResetDevices->GetStmt' FAILED");
			std::exception ex;
			throw ex;
		}

		int index = 1;
		if ((rez = sqlite3_bind_int ( stmt, index++, (int)newStatus)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'ResetDevices->sqlite3_bind_int' for 'newStatus' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, (int)Device::PS_NO_DATA)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'ResetDevices->sqlite3_bind_int' for 'PublishStatus' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		
		if ((rez = oSqlDev.Step_Exec()) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'ResetDevices->Step_Exec' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
	}
}

void SqliteDevicesDal::GetDevices(DeviceList& list)
{
	LOG_DEBUG_APP("[DevicesDAL]: Get all devices from database");

	sqlitexx::CSqliteStmtHelper oSqlDev (connection);

	int rez = SQLITE_OK;
	if ((rez = oSqlDev.Prepare(
	            "SELECT DeviceID, DeviceRole, Nickname, Address64, DeviceTag, DeviceStatus, DeviceLevel, DeviceCode, "
				"       SoftwareRevision, PowerSupplyStatus "
	            "  FROM Devices;")) != SQLITE_OK)
	{
		connection.LogIfLastError(rez);
		LOG_ERROR_APP("[DevicesDAL]: 'getdevices->Prepare' FAILED, err=" << rez);
		std::exception ex;
		throw ex;
	}

	
	if ((rez=oSqlDev.Step_GetRow()) != SQLITE_ROW)
	{
		connection.LogIfLastError(rez);
		LOG_ERROR_APP("[DevicesDAL]: 'getdevices->Step_GetRow' FAILED, err=" << rez);
		std::exception ex;
		throw ex;
	}

	list.reserve(8);

	do 
	{
		try
		{
			MAC mac(oSqlDev.Column_GetText(3));

			list.push_back(Device());
			Device& device = *list.rbegin();

			device.id = oSqlDev.Column_GetInt(0);
			device.Type((Device::DeviceType)oSqlDev.Column_GetInt(1));
			device.Nickname(NickName((WHartShortAddress)oSqlDev.Column_GetInt(2)));
			device.Mac(mac);
			device.SetTAG(oSqlDev.Column_GetText(4));
			device.Status((Device::DeviceStatus)oSqlDev.Column_GetInt(5));
			device.Level(oSqlDev.Column_GetInt(6));
			device.SetDeviceCode(oSqlDev.Column_GetInt(7));
			device.SetSoftwareRevision(oSqlDev.Column_GetInt(8));
			device.PowerStatus(oSqlDev.Column_GetInt(9));
		}
		catch (...)
		{
		}
	}
	while (oSqlDev.Step_GetRow() == SQLITE_ROW);

	LOG_DEBUG_APP("GetDevices: no="<<list.size());
}

void SqliteDevicesDal::DeleteDevice(int id)
{
	LOG_DEBUG_APP("[DevicesDAL]: delete device id=" << id);

	sqlitexx::Command sqlCommand(connection, "DELETE FROM Devices WHERE DeviceID = ?001");
	sqlCommand.BindParam(1, id);
	sqlCommand.ExecuteNonQuery();
}

void SqliteDevicesDal::AddDevice(Device& device)
{
	LOG_DEBUG_APP("[DevicesDAL]: Add device with MAC:" << device.Mac());
	
	{
		sqlitexx::Command
			sqlCommand(connection,
				"INSERT OR REPLACE INTO Devices(DeviceRole, Nickname, Address64, DeviceTag, DeviceStatus, DeviceLevel, DeviceCode, "
			    "                               SoftwareRevision, PowerSupplyStatus, RejoinCount) "
			    "VALUES (?001, ?002, ?003, ?004, ?005, ?006, ?007, ?008, ?009, ?010);");
		sqlCommand.BindParam(1, (int)device.Type());
		sqlCommand.BindParam(2, (int)device.Nickname().Address());
		sqlCommand.BindParam(3, device.Mac().ToString());
		sqlCommand.BindParam(4, device.GetTAG());
		sqlCommand.BindParam(5, (int)device.Status());
		sqlCommand.BindParam(6, device.Level());
		sqlCommand.BindParam(7, device.GetDeviceCode());
		sqlCommand.BindParam(8, device.GetSoftwareRevision());
		sqlCommand.BindParam(9, device.PowerStatus());
		sqlCommand.BindParam(10, device.GetRejoinCount());
		sqlCommand.ExecuteNonQuery();
		device.id = sqlCommand.GetLastInsertRowID();
	}

	if (device.GetDeviceCode() > 0)
	{ // tries to insert a generic device code. If device code already exists then nothing will happen.
		sqlitexx::Command sqlCommand(connection, "INSERT OR IGNORE INTO DevicesCodes(DeviceCode, Model, Company) VALUES(?001, 'N/A', 'N/A');");
		sqlCommand.BindParam(1, device.GetDeviceCode());
		sqlCommand.ExecuteNonQuery();
		LOG_DEBUG_APP("[DevicesDAL]: Insert generic record for deviceCode=" << device.GetDeviceCode() << ". DeviceMAC=" << device.Mac() << ".");
	}

	{ //update history
		sqlitexx::Command sqlCommand(connection, "INSERT INTO DeviceHistory(DeviceID, Timestamp, DeviceStatus) VALUES (?001, ?002, ?003)");
		sqlCommand.BindParam(1, device.id);
		sqlCommand.BindParam(2, CMicroSec().GetElapsedTimeStr());
		sqlCommand.BindParam(3, (int)device.Status());
		sqlCommand.ExecuteNonQuery();
	}
}

void SqliteDevicesDal::UpdateDevice(Device& device)
{
	LOG_DEBUG_APP("[DevicesDAL]: Update device with MAC:" << device.Mac());
	{
		sqlitexx::CSqliteStmtHelper oSqlDev (connection);
		int rez = SQLITE_OK;
		if ((rez = oSqlDev.Prepare(
		            "INSERT INTO DeviceHistory(DeviceID, Timestamp, DeviceStatus) "
		            "SELECT ?001, ?002, ?003 FROM Devices WHERE DeviceID = ?001 AND DeviceStatus != ?003")) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'UpdateDeviceHistory->Prepare' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		// get
		sqlite3_stmt *stmt = NULL;
		if (!(stmt = oSqlDev.GetStmt()))
		{
			LOG_ERROR_APP("[DevicesDAL]: 'UpdateDeviceHistory->GetStmt' FAILED");
			std::exception ex;
			throw ex;
		}

		int index = 1;
		if ((rez = sqlite3_bind_int ( stmt, index++, device.id)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'UpdateDeviceHistory->sqlite3_bind_int' for 'deviceID' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_text ( stmt, index++, CMicroSec().GetElapsedTimeStr(), -1, SQLITE_STATIC)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'UpdateDeviceHistory->sqlite3_bind_text' for 'time' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, (int)device.Status())) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'UpdateDeviceHistory->sqlite3_bind_int' for 'Status' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = oSqlDev.Step_Exec()) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'UpdateDeviceHistory->Step_Exec' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
	}

	{
		sqlitexx::CSqliteStmtHelper oSqlDev (connection);
		int rez = SQLITE_OK;
		if ((rez = oSqlDev.Prepare(
		        "UPDATE OR REPLACE Devices "
		        "   SET DeviceRole = ?001, Nickname = ?002, Address64 = ?003,"
		        "       DeviceTag = ?004, DeviceStatus = ?005, DeviceCode = ?006,"
		        "       SoftwareRevision = ?007, PowerSupplyStatus = ?008, RejoinCount = ?009, PublishStatus = ?010 "
		        " WHERE DeviceID = ?011")) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'UpdateDevice->Prepare' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}

		// get
		sqlite3_stmt *stmt = NULL;
		if (!(stmt = oSqlDev.GetStmt()))
		{
			LOG_ERROR_APP("[DevicesDAL]: 'UpdateDevice->GetStmt' FAILED");
			std::exception ex;
			throw ex;
		}

		if ((rez = sqlite3_bind_int ( stmt, 1, (int)device.Type())) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'UpdateDevice->sqlite3_bind_int' for 'deviceRole' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int(stmt, 2, (int)device.Nickname().Address())) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'UpdateDevice->sqlite3_bind_int' for 'nickname' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_text(stmt, 3, device.Mac().ToString().c_str(), -1, SQLITE_STATIC)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'UpdateDevice->sqlite3_bind_text' for 'MAC' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_text(stmt, 4, device.GetTAG().c_str(), -1, SQLITE_STATIC)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'UpdateDevice->sqlite3_bind_text' for 'TAG' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int(stmt, 5, (int)device.Status())) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'UpdateDevice->sqlite3_bind_int' for 'status' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int(stmt, 6, device.GetDeviceCode())) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'UpdateDevice->sqlite3_bind_int' for 'GetDeviceCode' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int(stmt, 7, device.GetSoftwareRevision())) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'UpdateDevice->sqlite3_bind_int' for 'GetSoftwareRevision' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int(stmt, 8, device.PowerStatus())) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'UpdateDevice->sqlite3_bind_int' for 'PowerStatus' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int(stmt, 9, device.GetRejoinCount())) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'UpdateDevice->sqlite3_bind_int' for 'RejoinCount' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
        if ((rez = sqlite3_bind_int(stmt, 10, device.GetPublishStatus())) != SQLITE_OK)
        {
            connection.LogIfLastError(rez);
            LOG_ERROR_APP("[DevicesDAL]: 'UpdateDevice->sqlite3_bind_int' for 'PublishStatus' FAILED, err=" << rez);
            std::exception ex;
            throw ex;
        }
		if ((rez = sqlite3_bind_int(stmt, 11, device.id)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'UpdateDevice->sqlite3_bind_int' for 'deviceID' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = oSqlDev.Step_Exec()) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'UpdateDevice->Step_Exec' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
	}

	if (device.GetDeviceCode() > 0)
	{ // tries to insert a generic device code. If device code already exists then nothing will happen.
		sqlitexx::CSqliteStmtHelper oSqlDev (connection);
		int rez = SQLITE_OK;
		if ((rez = oSqlDev.Prepare("INSERT OR IGNORE INTO DevicesCodes(DeviceCode, Model, Company) VALUES(?001, 'N/A', 'N/A');")) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'UpdateDevice->Prepare for INSERT a new device code' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}

		// get
		sqlite3_stmt *stmt = NULL;
		if (!(stmt = oSqlDev.GetStmt()))
		{
			LOG_ERROR_APP("[DevicesDAL]: 'UpdateDevice->GetStmt' FAILED");
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int(stmt, 1, device.GetDeviceCode())) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'UpdateDevice->sqlite3_bind_int' for GetDeviceCode=" << device.GetDeviceCode() << " FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}

		if ((rez = oSqlDev.Step_Exec()) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'UpdateDevice->Step_Exec' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}

		LOG_DEBUG_APP("[DevicesDAL]: Insert generic record for deviceCode=" << device.GetDeviceCode() << ". DeviceMAC=" << device.Mac() << ".");
	}
}

//topology
void SqliteDevicesDal::CreateDeviceGraph(int fromDevice, int toDevice, int graphID, int neighbIndex)
{
	LOG_DEBUG_APP("[DevicesDAL]: create graph to=" << toDevice << " from device id=" << fromDevice << " with graphID=" << graphID
                << " with index=" << neighbIndex);

	{
		int rez = SQLITE_OK;
		if (!m_oInsertGraph_compiled.GetStmt())
		{
			if ((rez = m_oInsertGraph_compiled.Prepare( "INSERT INTO GraphNeighbors(DeviceID, PeerID, GraphID, NeighborIndex)"
				" VALUES (?001, ?002, ?003, ?004)")) != SQLITE_OK)
			{
				connection.LogIfLastError(rez);
				LOG_ERROR_APP("[DevicesDAL]: 'CreateDeviceGraph->Prepare' FAILED, err=" << rez);
				std::exception ex;
				throw ex;
			}
		}

		// get
		sqlite3_stmt *stmt = NULL;
		if (!(stmt = m_oInsertGraph_compiled.GetStmt()))
		{
			LOG_ERROR_APP("[DevicesDAL]: 'CreateDeviceGraph->GetStmt' FAILED");
			std::exception ex;
			throw ex;
		}

		int index = 1;
		if ((rez = sqlite3_bind_int ( stmt, index++, fromDevice)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'CreateDeviceGraph->sqlite3_bind_int' for 'fromDevice' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, toDevice)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'CreateDeviceGraph->sqlite3_bind_int' for 'toDevice' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, graphID)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'CreateDeviceGraph->sqlite3_bind_int' for 'graphID' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, neighbIndex)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'CreateDeviceGraph->sqlite3_bind_int' for 'neighbIndex' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = m_oInsertGraph_compiled.Step_Exec()) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'CreateDeviceGraph->Step_Exec' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
	}
}

void SqliteDevicesDal::CleanDevicesGraphs()
{
	LOG_DEBUG_APP("[DevicesDAL]: clean graphs");

	sqlitexx::Command sqlCommand(connection, "DELETE FROM GraphNeighbors");
	sqlCommand.ExecuteNonQuery();
}

void SqliteDevicesDal::CleanDeviceGraphs(int deviceID)
{
	LOG_DEBUG_APP("[DevicesDAL]: clean graphs for device id=" << deviceID);

	sqlitexx::Command sqlCommand(connection, "DELETE FROM GraphNeighbors WHERE DeviceID = ?001;");
	sqlCommand.BindParam(1, deviceID);
	sqlCommand.ExecuteNonQuery();
}

void SqliteDevicesDal::DeleteGraph(int deviceId, int neighborID, int graphID)
{
	LOG_DEBUG_APP("[DevicesDAL]: clean graph for device id=" << deviceId<<" with neighbID=" << neighborID << " and graph id="<<graphID);
	{
		int rez = SQLITE_OK;
		
		if (!m_oDeleteGraph_compiled.GetStmt())
		{
			if ((rez = m_oDeleteGraph_compiled.Prepare(
			            "DELETE FROM GraphNeighbors "
				        " WHERE DeviceID = ?001 AND GraphID = ?002 AND PeerID = ?003;")) != SQLITE_OK)
			{
				connection.LogIfLastError(rez);
				LOG_ERROR_APP("[DevicesDAL]: 'DeleteGraph->Prepare' FAILED, err=" << rez);
				std::exception ex;
				throw ex;
			}
		}
		// get
		sqlite3_stmt *stmt = NULL;
		if (!(stmt = m_oDeleteGraph_compiled.GetStmt()))
		{
			LOG_ERROR_APP("[DevicesDAL]: 'DeleteGraph->GetStmt' FAILED");
			std::exception ex;
			throw ex;
		}

		int index = 1;
		if ((rez = sqlite3_bind_int ( stmt, index++, deviceId)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'DeleteGraph->sqlite3_bind_int' for 'deviceId' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, graphID)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'DeleteGraph->sqlite3_bind_int' for 'graphID' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, neighborID)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'DeleteGraph->sqlite3_bind_int' for 'neighborID' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}

		if ((rez = m_oDeleteGraph_compiled.Step_Exec()) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'DeleteGraph->Step_Exec' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
	}
}

//gateway
void SqliteDevicesDal::AddDeviceConnections(int deviceID, const std::string& host, int port)
{
	LOG_DEBUG_APP("[DevicesDAL]: Add device connection for deviceID=" << deviceID);

	sqlitexx::Command sqlCommand(connection, ""
		"INSERT OR REPLACE INTO DeviceConnections(DeviceID, IP, Port) "
		"VALUES (?001, ?002, ?003);");
	sqlCommand.BindParam(1, deviceID);
	sqlCommand.BindParam(2, host);
	sqlCommand.BindParam(3, port);
	sqlCommand.ExecuteNonQuery();
}

void SqliteDevicesDal::RemoveDeviceConnections(int deviceID)
{
	LOG_DEBUG_APP("[DevicesDAL]: remove device connection for deviceID=" << deviceID);

	sqlitexx::Command sqlCommand(connection, "DELETE FROM DeviceConnections WHERE DeviceID = ?001;");
	sqlCommand.BindParam(1, deviceID);
	sqlCommand.ExecuteNonQuery();
}

//wh_general_cmd
void SqliteDevicesDal::AddCommand(int deviceID, int cmdNo, std::string dataBuffer, int cmdID, int responseCode)
{

	LOG_DEBUG_APP("[DevicesDAL]: Add response command to WHResponseDataBuffer table" );

	sqlitexx::Command sqlCommand(connection,
	    "INSERT INTO WHResponseDataBuffer(DeviceID, CommandID, Timestamp, Miliseconds, CmdNo, ResponseCode, DataBuffer) "
		"VALUES (?001, ?002, ?003, ?004, ?005, ?006, ?007);");
	sqlCommand.BindParam(1, deviceID);
	sqlCommand.BindParam(2, cmdID);
	sqlCommand.BindParam(3, CMicroSec().GetElapsedTimeStr());
	sqlCommand.BindParam(4, 0);
	sqlCommand.BindParam(5, cmdNo);
	sqlCommand.BindParam(6, responseCode);
	sqlCommand.BindParam(7, dataBuffer);
	sqlCommand.ExecuteNonQuery();

}

//readings
void SqliteDevicesDal::CreateChannel(int deviceID, const PublishChannel &channel)
{
	LOG_DEBUG_APP("[DevicesDAL]: create channels for deviceID=" << deviceID);

	sqlitexx::Command sqlCommand(connection, 
		"INSERT INTO Channels(DeviceID, BurstMessage, DeviceVariableSlot, Name, CmdNo, DeviceVariable, Classification, UnitCode) "
		"VALUES (?001, ?002, ?003, ?004, ?005, ?006, ?007, ?008);");
	sqlCommand.BindParam(1, deviceID);
	sqlCommand.BindParam(2, (int)channel.burstMessage);
	sqlCommand.BindParam(3, (int)channel.deviceVariableSlot);
	sqlCommand.BindParam(4, channel.name.c_str());
	sqlCommand.BindParam(5, (int)channel.cmdNo);
	sqlCommand.BindParam(6, (int)channel.deviceVariable);
	sqlCommand.BindParam(7, (int)channel.classification);
	sqlCommand.BindParam(8, (int)channel.unitCode);
	sqlCommand.ExecuteNonQuery();
	channel.channelID = sqlCommand.GetLastInsertRowID();
}

void SqliteDevicesDal::MoveChannelToHistory(int channelID)
{
	LOG_DEBUG_APP("[DevicesDAL]: move to history channelID=" << channelID);
	{
		sqlitexx::Command sqlCommand(connection,
			"INSERT INTO ChannelsHistory "
			"SELECT ChannelID, DeviceID, BurstMessage, DeviceVariableSlot, Name, CmdNo, DeviceVariable, Classification, UnitCode, datetime('now') "
			"  FROM Channels "
		    " WHERE ChannelID = ?001");
		sqlCommand.BindParam(1, channelID);
		sqlCommand.ExecuteNonQuery();
	}
	{
		sqlitexx::Command sqlCommand(connection, "DELETE FROM Channels WHERE ChannelID = ?001");
		sqlCommand.BindParam(1, channelID);
		sqlCommand.ExecuteNonQuery();
	}
}

void SqliteDevicesDal::UpdateChannel(int channelID, int variableCode, int classification, int unitCode, const std::string &name)
{
	LOG_DEBUG_APP("[DevicesDAL]: update channelID=" << channelID);

	sqlitexx::Command sqlCommand(connection, ""
		"UPDATE Channels SET DeviceVariable = ?001, Classification = ?002, UnitCode = ?003, Name = ?004 "
	    " WHERE ChannelID = ?005");
	sqlCommand.BindParam(1, variableCode);
	sqlCommand.BindParam(2, classification);
	sqlCommand.BindParam(3, unitCode);
	sqlCommand.BindParam(4, name);
	sqlCommand.BindParam(5, channelID);
	sqlCommand.ExecuteNonQuery();
}

void SqliteDevicesDal::DeleteReading(int channeNo)
{
	LOG_DEBUG_APP("[DevicesDAL]: delete reading with channelID=" << channeNo);

	sqlitexx::Command sqlCommand(connection, "DELETE FROM Readings WHERE ChannelID = ?001;");
    sqlCommand.BindParam(1, channeNo);
	sqlCommand.ExecuteNonQuery();
}

void SqliteDevicesDal::AddEmptyReading(int DeviceID, int channelID)
{
	LOG_DEBUG_APP("[DevicesDAL]: add in readings channelID=" << channelID << " for deviceID=" << DeviceID);

	sqlitexx::Command sqlCommand(connection,
	    "INSERT INTO Readings (ChannelID, ReadingTime, Miliseconds, Value, Status, CommandID) "
	    "SELECT ?001, '1970-01-01 00:00:00', 0, 0, 0, 0;");
    sqlCommand.BindParam(1, channelID);
	sqlCommand.ExecuteNonQuery();
}

static const char* gettime(const struct timeval &tv)
{
    static char szTime[50];
    static struct tm * timeinfo;

    timeinfo = gmtime(&tv.tv_sec);
    sprintf(szTime, "%04d-%02d-%02d %02d:%02d:%02d", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour,
            timeinfo->tm_min, timeinfo->tm_sec);
    return szTime;
}

void SqliteDevicesDal::UpdateReading(const DeviceReading& reading)
{
	int valueType = 0; // float value = (-FLT_MAX, +FLT_MAX)

//	const char * hexValue = ConvertToHex(*((unsigned int*)&reading.m_value));
	if (reading.m_value == std::numeric_limits<float>::infinity()) // Infinity test
	{
		valueType = 1; // float value = -FLT_MAX | +FLT_MAX
		const_cast<DeviceReading&>(reading).m_value = -888.888;
		LOG_DEBUG_APP("[DevicesDAL]: Reading for channelID=" << reading.m_channelNo << " is Infinity; value = " << reading.m_value << "; write -888.888 instead ");
	}
	else if ((reading.m_value) != (reading.m_value)) // NaN test
	{
		valueType = 2; // float value = NaN
		const_cast<DeviceReading&>(reading).m_value = -888.888;
		LOG_DEBUG_APP("[DevicesDAL]: Reading for channelID=" << reading.m_channelNo << " is NaN; value = " << reading.m_value << "; write -888.888 instead ");
	}
	else
	{
		LOG_DEBUG_APP("[DevicesDAL]: update in Readings table for channelID = " << reading.m_channelNo << " with value = " << reading.m_value);
	}
	
	double readVal = reading.m_value;
	int rez = SQLITE_OK;
	if (!m_oUpdateDeviceReadings_Prepare.GetStmt())
	{
		if ((rez = m_oUpdateDeviceReadings_Prepare.Prepare(
		        "UPDATE Readings "
		        "   SET ReadingTime=?001, Value=?002, ValueType=?003, Miliseconds=?004, Status=?005, CommandID=?006 "
		        " WHERE ChannelID=?007;")) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'UpdateReading->Prepare' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
	}
    sqlite3_stmt *stmt = NULL;
	if (!(stmt = m_oUpdateDeviceReadings_Prepare.GetStmt()))
	{
		LOG_ERROR_APP("SqliteDevicesDal::UpdateReading: FAILED to get prepare stmt");
		std::exception ex;
		throw ex;
	}
	if ((rez = sqlite3_bind_text(stmt, 1, gettime(reading.m_tv), -1, SQLITE_STATIC)) != SQLITE_OK)
	{
		connection.LogIfLastError(rez);
		LOG_ERROR_APP("[DevicesDAL]: 'UpdateReading->sqlite3_bind_text' FAILED, err=" << rez);
		std::exception ex;
		throw ex;
	}
	if ((rez = sqlite3_bind_double(stmt, 2, readVal)) != SQLITE_OK)
	{
		connection.LogIfLastError(rez);
		LOG_ERROR_APP("[DevicesDAL]: 'UpdateReading->sqlite3_bind_double' FAILED, err=" << rez);
		std::exception ex;
		throw ex;
	}
	if ((rez = sqlite3_bind_int(stmt, 3, valueType)) != SQLITE_OK)
	{
		connection.LogIfLastError(rez);
		LOG_ERROR_APP("[DevicesDAL]: 'UpdateReading->sqlite3_bind_double' FAILED, err=" << rez);
		std::exception ex;
		throw ex;
	}
	if ((rez = sqlite3_bind_int(stmt, 4, (int)(reading.m_tv.tv_usec/1000))) != SQLITE_OK)
	{
		connection.LogIfLastError(rez);
		LOG_ERROR_APP("[DevicesDAL]: 'UpdateReading->sqlite3_bind_int' FAILED, err=" << rez);
		std::exception ex;
		throw ex;
	}
	if (reading.m_hasStatus == true)
	{
		if ((rez = sqlite3_bind_int(stmt, 5, (int)reading.m_valueStatus)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'UpdateReading->sqlite3_bind_int' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
	}
	else
	{
		if ((rez = sqlite3_bind_null(stmt, 5)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'UpdateReading->sqlite3_bind_null' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
	}
	if ((rez = sqlite3_bind_int(stmt, 6, (int)reading.m_nCommandID)) != SQLITE_OK)
	{
		connection.LogIfLastError(rez);
		LOG_ERROR_APP("[DevicesDAL]: 'UpdateReading->sqlite3_bind_int' FAILED, err=" << rez);
		std::exception ex;
		throw ex;
	}
	if ((rez = sqlite3_bind_int(stmt, 7, reading.m_channelNo)) != SQLITE_OK)
	{
		connection.LogIfLastError(rez);
		LOG_ERROR_APP("[DevicesDAL]: 'UpdateReading->sqlite3_bind_int' FAILED, err=" << rez);
		std::exception ex;
		throw ex;
	}
	if ((rez = m_oUpdateDeviceReadings_Prepare.Step_Exec()) != SQLITE_OK)
	{
		connection.LogIfLastError(rez);
		if (valueType == 0)
			LOG_ERROR_APP("[DevicesDAL]: 'UpdateReading->Step_Exec' FAILED, err=" << rez << " for input values:"
				<< " readingTime=" << gettime(reading.m_tv)
				<< " value=" << readVal
				<< " miliseconds=" << (int)(reading.m_tv.tv_usec/1000)
				<< " hasStatus=" << (reading.m_hasStatus == true ? "true":"false")
				<< " commandID=" << (int)reading.m_nCommandID
				<< " channelID=" << (int)reading.m_channelNo);
		else
			LOG_ERROR_APP("[DevicesDAL]: 'UpdateReading->Step_Exec' FAILED, err=" << rez << " for input values:"
				<< " readingTime=" << gettime(reading.m_tv)
				<< " value=" << ((valueType == 1) ? "Infinity" : "NaN")
				<< " miliseconds=" << (int)(reading.m_tv.tv_usec/1000)
				<< " hasStatus=" << (reading.m_hasStatus == true ? "true":"false")
				<< " commandID=" << (int)reading.m_nCommandID
				<< " channelID=" << (int)reading.m_channelNo);
		Log_Reading_Info(reading.m_channelNo);
		std::exception ex;
		throw ex;
	}
}

void SqliteDevicesDal::Log_Reading_Info(int channelNo)
{
	LOG_DEBUG_APP("[DevicesDAL]: Get reading from database for channel=" << channelNo);

	sqlitexx::CSqliteStmtHelper oSqlDev (connection);
	int rez = SQLITE_OK;
	if ((rez = oSqlDev.Prepare( "SELECT ReadingTime, Miliseconds, Value, ValueType, Status, CommandID, ChannelID FROM Readings WHERE ChannelID = ?001;")) != SQLITE_OK)
	{
		connection.LogIfLastError(rez);
		LOG_ERROR_APP("[DevicesDAL]: 'Log_Reading_Info->Prepare' FAILED, err=" << rez);
		std::exception ex;
		throw ex;
	}
	// get
	sqlite3_stmt *stmt = NULL;
	if (!(stmt = oSqlDev.GetStmt()))
	{
		LOG_ERROR_APP("[DevicesDAL]: 'Log_Reading_Info->GetStmt' FAILED");
		std::exception ex;
		throw ex;
	}

	int index = 1;
	if ((rez = sqlite3_bind_int ( stmt, index++, channelNo)) != SQLITE_OK)
	{
		connection.LogIfLastError(rez);
		LOG_ERROR_APP("[DevicesDAL]: 'Log_Reading_Info->sqlite3_bind_int' for 'deviceID' FAILED, err=" << rez);
		std::exception ex;
		throw ex;
	}
	if ((rez=oSqlDev.Step_GetRow()) != SQLITE_ROW)
	{
		connection.LogIfLastError(rez);
		LOG_ERROR_APP("[DevicesDAL]: 'Log_Reading_Info->Step_GetRow' FAILED, err=" << rez);
		std::exception ex;
		throw ex;
	}

	do 
	{
		
		double val = -888.888;
		int val2 = -888.888;
		int valueType = 0;
		oSqlDev.Column_GetInt(4, &valueType);

		if (valueType == 0)
			LOG_DEBUG_APP("[Log_Reading_Info]: "
				<< " readingTime=" << oSqlDev.Column_GetText(0)
				<< " miliseconds=" << oSqlDev.Column_GetInt(1)
				<< " value=" << (oSqlDev.Column_GetDouble(2, &val) == SQLITE_OK ? val : -888.888)
				<< " status=" << (oSqlDev.Column_GetInt(4, &val2) == SQLITE_OK ? val2: -888.888)
				<< " commandid=" << oSqlDev.Column_GetInt(5)
				<< " channelID=" << oSqlDev.Column_GetInt(6));
		else
			LOG_DEBUG_APP("[Log_Reading_Info]: "
				<< " readingTime=" << oSqlDev.Column_GetText(0)
				<< " miliseconds=" << oSqlDev.Column_GetInt(1)
				<< " value=" << (valueType == 1 ? "Infinity" : "NaN")
				<< " status=" << (oSqlDev.Column_GetInt(4, &val2) == SQLITE_OK ? val2: -888.888)
				<< " commandid=" << oSqlDev.Column_GetInt(5)
				<< " channelID=" << oSqlDev.Column_GetInt(6));

	}
	while (oSqlDev.Step_GetRow() == SQLITE_ROW);
}

void SqliteDevicesDal::UpdatePublishFlag(int devID, Device::PublishStatus status)
{
	LOG_DEBUG_APP("[DevicesDAL]: update publish_status for deviceID=" << devID << " with status=" << (int)status);

	int res = 0;
	if (!m_oUpdatePublishFlag_compiled.GetStmt())
	{
		if ((res=m_oUpdatePublishFlag_compiled.Prepare("UPDATE Devices SET PublishStatus = ? WHERE DeviceID = ?")) != SQLITE_OK)
		{
			connection.LogIfLastError(res);
			LOG_ERROR_APP("SqliteDevicesDal::m_oUpdatePublishFlag: FAILED prepare err=" << res);
			std::exception ex;
			throw ex;
		}
	}
	if ((res = m_oUpdatePublishFlag_compiled.BindInt(1,(int)status))!= SQLITE_OK)
	{
		connection.LogIfLastError(res);
		LOG_ERROR_APP("SqliteDevicesDal::m_oUpdatePublishFlag: FAILED bind 1 err=" << res);
		std::exception ex;
		throw ex;
	};
	if ((res = m_oUpdatePublishFlag_compiled.BindInt(2,devID))!= SQLITE_OK)
	{
		connection.LogIfLastError(res);
		LOG_ERROR_APP("SqliteDevicesDal::m_oUpdatePublishFlag: FAILED bind 2 err=" << res);
		std::exception ex;
		throw ex;
	};
	if ((res = m_oUpdatePublishFlag_compiled.Step_Exec())!= SQLITE_OK)
	{
		connection.LogIfLastError(res);
		LOG_ERROR_APP("SqliteDevicesDal::m_oUpdatePublishFlag: exec FAILED err=" << res);
		std::exception ex;
		throw ex;
	};
}

void SqliteDevicesDal::UpdateReadingTimeforDevice(int deviceID, const struct timeval &tv, Device::PublishStatus status)
{
	LOG_DEBUG_APP("[DevicesDAL]: update readingtime for deviceID=" << deviceID);

	int res = 0;
	if (!m_oUpdateLastReadAndPublishFlag_compiled.GetStmt())
	{
		if ((res = m_oUpdateLastReadAndPublishFlag_compiled.Prepare(
		        "UPDATE Devices "
		        "   SET LastRead = ?001, "
		        "       PublishStatus = ?002 "
		        " WHERE DeviceID = ?003")) != SQLITE_OK)
		{
			connection.LogIfLastError(res);
			LOG_ERROR_APP("SqliteDevicesDal::m_oUpdateLastReadAndPublishFlag: FAILED prepare err=" << res);
			std::exception ex;
			throw ex;
		}
	}

	if ((res = m_oUpdateLastReadAndPublishFlag_compiled.BindText(1, gettime(tv))) != SQLITE_OK)
	{
		connection.LogIfLastError(res);
		LOG_ERROR_APP("SqliteDevicesDal::m_oUpdateLastReadAndPublishFlag: FAILED bind 1 err=" << res);
		std::exception ex;
		throw ex;
	}
	if ((res = m_oUpdateLastReadAndPublishFlag_compiled.BindInt(2, (int)status)) != SQLITE_OK)
	{
		connection.LogIfLastError(res);
		LOG_ERROR_APP("SqliteDevicesDal::m_oUpdateLastReadAndPublishFlag: FAILED bind 2 err=" << res);
		std::exception ex;
		throw ex;
	}
	if ((res = m_oUpdateLastReadAndPublishFlag_compiled.BindInt(3, deviceID)) != SQLITE_OK)
	{
		connection.LogIfLastError(res);
		LOG_ERROR_APP("SqliteDevicesDal::m_oUpdateLastReadAndPublishFlag: FAILED bind 3 err=" << res);
		std::exception ex;
		throw ex;
	}
	if ((res = m_oUpdateLastReadAndPublishFlag_compiled.Step_Exec()) != SQLITE_OK)
	{
		connection.LogIfLastError(res);
		LOG_ERROR_APP("SqliteDevicesDal::m_oUpdateLastReadAndPublishFlag: exec FAILED err=" << res);
		std::exception ex;
		throw ex;
	}
}

void SqliteDevicesDal::GetOrderedChannels(int deviceID, std::vector<PublishChannel> &channels)
{

	LOG_DEBUG_APP("[DevicesDAL]: get ordered channels for deviceID=" << deviceID);

	sqlitexx::CSqliteStmtHelper oSqlStmtHlp (connection);
	int rez = SQLITE_OK;
	if ((rez = oSqlStmtHlp.Prepare(
	            "SELECT ChannelID, BurstMessage, DeviceVariableSlot, Name, CmdNo, DeviceVariable, Classification, UnitCode "
	            "  FROM Channels WHERE DeviceID = ?001 "
	            " ORDER BY CmdNo, BurstMessage, DeviceVariableSlot;")) != SQLITE_OK)
	{
		connection.LogIfLastError(rez);
		LOG_ERROR_APP("[DevicesDAL]: 'GetOrderedChannels->Prepare' FAILED, err=" << rez);
		std::exception ex;
		throw ex;
	}

	oSqlStmtHlp.BindInt(1, deviceID);
	if (oSqlStmtHlp.Step_GetRow() != SQLITE_ROW)
	{
		return;
	}

	channels.reserve(10);
	do 
	{
		PublishChannel channel;
		channel.channelID = oSqlStmtHlp.Column_GetInt(0);
		channel.burstMessage= oSqlStmtHlp.Column_GetInt(1);
		channel.deviceVariableSlot = oSqlStmtHlp.Column_GetInt(2);
		channel.name = oSqlStmtHlp.Column_GetText(3);
		channel.cmdNo = oSqlStmtHlp.Column_GetInt(4);
		channel.deviceVariable = oSqlStmtHlp.Column_GetInt(5);
		channel.classification = oSqlStmtHlp.Column_GetInt(6);
		channel.unitCode = oSqlStmtHlp.Column_GetInt(7);
		
		channels.push_back(channel);
	}
	while (oSqlStmtHlp.Step_GetRow() == SQLITE_ROW);
}

//routes
void SqliteDevicesDal::CleanDeviceRoutes()
{
	LOG_DEBUG_APP("[DevicesDAL]: clean routes report");

	sqlitexx::Command sqlCommand(connection, "DELETE FROM Routes");
	sqlCommand.ExecuteNonQuery();	
}

void SqliteDevicesDal::CleanDeviceRoutes(int deviceID)
{
	LOG_DEBUG_APP("[DevicesDAL]: clean all routes for deviceID=" << deviceID);

	sqlitexx::Command sqlCommand(connection, "DELETE FROM Routes WHERE DeviceID = ?001;");
	sqlCommand.BindParam(1, deviceID);
	sqlCommand.ExecuteNonQuery();	
}

void SqliteDevicesDal::CleanDeviceRoute(int deviceID, int routeID)
{
	LOG_DEBUG_APP("[DevicesDAL]: clean routes for deviceID=" << deviceID<<" and routeID="<<routeID);
	{
		int rez = SQLITE_OK;
		if (!m_oDeleteRoute_compiled.GetStmt())
		{
			if ((rez = m_oDeleteRoute_compiled.Prepare(
			            "DELETE FROM Routes "
				        " WHERE DeviceID = ?001 AND RouteID = ?002;")) != SQLITE_OK)
			{
				connection.LogIfLastError(rez);
				LOG_ERROR_APP("[DevicesDAL]: 'CleanDeviceRoute->Prepare' FAILED, err=" << rez);
				std::exception ex;
				throw ex;
			}
		}
		// get
		sqlite3_stmt *stmt = NULL;
		if (!(stmt = m_oDeleteRoute_compiled.GetStmt()))
		{
			LOG_ERROR_APP("[DevicesDAL]: 'CleanDeviceRoute->GetStmt' FAILED");
			std::exception ex;
			throw ex;
		}

		int index = 1;
		if ((rez = sqlite3_bind_int ( stmt, index++, deviceID)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'CleanDeviceRoute->sqlite3_bind_int' for 'deviceID' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, routeID)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'CleanDeviceRoute->sqlite3_bind_int' for 'routeID' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = m_oDeleteRoute_compiled.Step_Exec()) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'CleanDeviceRoute->Step_Exec' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
	}
}

void SqliteDevicesDal::InsertRoute(const Route& p_rRoute)
{
	LOG_DEBUG_APP("[DevicesDAL]: Add route for device id=" << p_rRoute.m_nDeviceId);
	{
		int rez = SQLITE_OK;
		if (!m_oInsertRoute_compiled.GetStmt())
		{
			if ((rez = m_oInsertRoute_compiled.Prepare(
			            "INSERT INTO Routes(RouteID, DeviceID, PeerID, GraphID, SourceRoute, Timestamp) "
				        "VALUES(?001, ?002, ?003, ?004, ?005, ?006); ")) != SQLITE_OK)
			{
				connection.LogIfLastError(rez);
				LOG_ERROR_APP("[DevicesDAL]: 'InsertRoute->Prepare' FAILED, err=" << rez);
				std::exception ex;
				throw ex;
			}
		}

		// get
		sqlite3_stmt *stmt = NULL;
		if (!(stmt = m_oInsertRoute_compiled.GetStmt()))
		{
			LOG_ERROR_APP("[DevicesDAL]: 'InsertRoute->GetStmt' FAILED");
			std::exception ex;
			throw ex;
		}

		int index = 1;
		if ((rez = sqlite3_bind_int ( stmt, index++, p_rRoute.m_nRouteId)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertRoute->sqlite3_bind_int' for 'p_rRoute.m_nRouteId' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, p_rRoute.m_nDeviceId)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertRoute->sqlite3_bind_int' for 'p_rRoute.m_nDeviceId' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, p_rRoute.m_nPeerId)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertRoute->sqlite3_bind_int' for 'p_rRoute.m_nPeerId' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, p_rRoute.m_nGraphId)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertRoute->sqlite3_bind_int' for 'p_rRoute.m_nGraphId' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, p_rRoute.m_nSourceRoute)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertRoute->sqlite3_bind_int' for 'p_rRoute.m_nSourceRoute' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_text ( stmt, index++, p_rRoute.time.c_str(), -1, SQLITE_STATIC)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertRoute->sqlite3_bind_int' for 'time' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = m_oInsertRoute_compiled.Step_Exec()) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertRoute->Step_Exec' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
	}
}

//services
void SqliteDevicesDal::CleanDeviceServices()
{
	LOG_DEBUG_APP("[DevicesDAL]: clean services report");

	sqlitexx::Command sqlCommand(connection, "DELETE FROM Services ");
	sqlCommand.ExecuteNonQuery();
}

void SqliteDevicesDal::CleanDeviceServices(int deviceID)
{
	LOG_DEBUG_APP("[DevicesDAL]: clean all services for deviceID=" << deviceID);

	sqlitexx::Command sqlCommand(connection, "DELETE FROM Services WHERE DeviceID = ?001;");
	sqlCommand.BindParam(1, deviceID);
	sqlCommand.ExecuteNonQuery();
}

void SqliteDevicesDal::CleanDeviceService(int deviceID, int serviceID)
{
	LOG_DEBUG_APP("[DevicesDAL]: clean service for deviceID=" << deviceID<<" and serviceID="<<serviceID);
	{
		int rez = SQLITE_OK;
		if (!m_oDeleteService_compiled.GetStmt())
		{
			if ((rez = m_oDeleteService_compiled.Prepare(
			            "DELETE FROM Services "
				        " WHERE DeviceID = ?001 AND ServiceID = ?002;")) != SQLITE_OK)
			{
				connection.LogIfLastError(rez);
				LOG_ERROR_APP("[DevicesDAL]: 'CleanDeviceService->Prepare' FAILED, err=" << rez);
				std::exception ex;
				throw ex;
			}
		}

		// get
		sqlite3_stmt *stmt = NULL;
		if (!(stmt = m_oDeleteService_compiled.GetStmt()))
		{
			LOG_ERROR_APP("[DevicesDAL]: 'CleanDeviceService->GetStmt' FAILED");
			std::exception ex;
			throw ex;
		}

		int index = 1;
		if ((rez = sqlite3_bind_int ( stmt, index++, deviceID)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'CleanDeviceService->sqlite3_bind_int' for 'deviceID' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, serviceID)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'CleanDeviceService->sqlite3_bind_int' for 'serviceID' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = m_oDeleteService_compiled.Step_Exec()) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'CleanDeviceService->Step_Exec' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
	}
}

void SqliteDevicesDal::InsertService(const Service& p_rService)
{
	LOG_DEBUG_APP("[DevicesDAL]: Add service for device id=" << p_rService.m_nDeviceId);
	{
		int rez = SQLITE_OK;
		if (!m_oInsertService_compiled.GetStmt())
		{
			if ((rez = m_oInsertService_compiled.Prepare(
			    "INSERT INTO Services (ServiceID, DeviceID, PeerID, ApplicationDomain, SourceFlag, SinkFlag, IntermittentFlag, Period, RouteID, Timestamp)"
				"VALUES (?001, ?002, ?003, ?004, ?005, ?006, ?007, ?008, ?009, ?010); ")) != SQLITE_OK)
			{
				connection.LogIfLastError(rez);
				LOG_ERROR_APP("[DevicesDAL]: 'InsertService->Prepare' FAILED, err=" << rez);
				std::exception ex;
				throw ex;
			}
		}

		// get
		sqlite3_stmt *stmt = NULL;
		if (!(stmt = m_oInsertService_compiled.GetStmt()))
		{
			LOG_ERROR_APP("[DevicesDAL]: 'InsertService->GetStmt' FAILED");
			std::exception ex;
			throw ex;
		}

		int index = 1;
		if ((rez = sqlite3_bind_int(stmt, index++, p_rService.m_nServiceId)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertService->sqlite3_bind_int' for 'ServiceID' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, p_rService.m_nDeviceId)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertService->sqlite3_bind_int' for 'DeviceID' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, p_rService.m_nPeerId)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertService->sqlite3_bind_int' for 'PeerID' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, p_rService.m_nApplicationDomain)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertService->sqlite3_bind_int' for 'ApplicationDomain' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, p_rService.m_nSourceFlag)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertService->sqlite3_bind_int' for 'SourceFlag' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, p_rService.m_nSinkFlag)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertService->sqlite3_bind_int' for 'SinkFlag' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, p_rService.m_nIntermittentFlag)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertService->sqlite3_bind_int' for 'IntermittentFlag' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, p_rService.m_nPeriod)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertService->sqlite3_bind_int' for 'Period' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, p_rService.m_nRouteId)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertService->sqlite3_bind_int' for 'RouteID' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_text ( stmt, index++, p_rService.time.c_str(), -1, SQLITE_STATIC)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertService->sqlite3_bind_int' for 'Timestamp' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = m_oInsertService_compiled.Step_Exec()) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertService->Step_Exec' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
	}
}

//SourceRoutes
void SqliteDevicesDal::CleanDeviceSourceRoutes()
{
	LOG_DEBUG_APP("[DevicesDAL]: clean source routes report");
	sqlitexx::Command sqlCommand(connection, "DELETE FROM SourceRoutes;");
	sqlCommand.ExecuteNonQuery();	
}

void SqliteDevicesDal::CleanDeviceSourceRoutes(int p_nDeviceId)
{
	LOG_DEBUG_APP("[DevicesDAL]: clean source routes for device id=" << p_nDeviceId);

	sqlitexx::Command sqlCommand(connection, "DELETE FROM SourceRoutes WHERE deviceID = ?001;");
	sqlCommand.BindParam(1, p_nDeviceId);
	sqlCommand.ExecuteNonQuery();	
}

void SqliteDevicesDal::CleanDeviceSourceRoutes(int p_nDeviceId, int p_nRouteID)
{
	LOG_DEBUG_APP("[DevicesDAL]: clean source routes for deviceID=" << p_nDeviceId<<" and routeID="<<p_nRouteID);
	{
		int rez = SQLITE_OK;
		if (!m_oDeleteSourceRoute_compiled.GetStmt())
		{
			if ((rez = m_oDeleteSourceRoute_compiled.Prepare(
			            "DELETE FROM SourceRoutes "
					    " WHERE deviceID = ?001 AND RouteID = ?002;")) != SQLITE_OK)
			{
				connection.LogIfLastError(rez);
				LOG_ERROR_APP("[DevicesDAL]: 'CleanDeviceSourceRoutes->Prepare' FAILED, err=" << rez);
				std::exception ex;
				throw ex;
			}
		}
		// get
		sqlite3_stmt *stmt = NULL;
		if (!(stmt = m_oDeleteSourceRoute_compiled.GetStmt()))
		{
			LOG_ERROR_APP("[DevicesDAL]: 'CleanDeviceSourceRoutes->GetStmt' FAILED");
			std::exception ex;
			throw ex;
		}

		int index = 1;
		if ((rez = sqlite3_bind_int ( stmt, index++, p_nDeviceId)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'CleanDeviceSourceRoutes->sqlite3_bind_int' for 'p_nDeviceId' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, p_nRouteID)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'CleanDeviceSourceRoutes->sqlite3_bind_int' for 'p_nRouteID' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}

		if ((rez = m_oDeleteSourceRoute_compiled.Step_Exec()) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'CleanDeviceSourceRoutes->Step_Exec' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
	}
}

void SqliteDevicesDal::InsertSourceRoute(const SourceRoute& p_rSourceRoute)
{
	LOG_DEBUG_APP("[DevicesDAL]: Add source route for device id=" << p_rSourceRoute.m_nDeviceId);
	{
		int rez = SQLITE_OK;
		if (!m_oInsertSourceRoute_compiled.GetStmt())
		{
			if ((rez = m_oInsertSourceRoute_compiled.Prepare( "INSERT INTO SourceRoutes(DeviceID, RouteID, Devices, Timestamp)"
				" VALUES(?001, ?002, ?003, ?004); ")) != SQLITE_OK)
			{
				connection.LogIfLastError(rez);
				LOG_ERROR_APP("[DevicesDAL]: 'InsertSourceRoute->Prepare' FAILED, err=" << rez);
				std::exception ex;
				throw ex;
			}
		}
		// get
		sqlite3_stmt *stmt = NULL;
		if (!(stmt = m_oInsertSourceRoute_compiled.GetStmt()))
		{
			LOG_ERROR_APP("[DevicesDAL]: 'InsertSourceRoute->GetStmt' FAILED");
			std::exception ex;
			throw ex;
		}

		int index = 1;
		if ((rez = sqlite3_bind_int ( stmt, index++, p_rSourceRoute.m_nDeviceId)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertSourceRoute->sqlite3_bind_int' for 'p_nDeviceId' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, p_rSourceRoute.m_nRouteId)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertSourceRoute->sqlite3_bind_int' for 'p_nRouteID' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_text ( stmt, index++, p_rSourceRoute.m_strDevices.c_str(), -1, SQLITE_STATIC)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertSourceRoute->sqlite3_bind_int' for 'sourceroutes' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_text ( stmt, index++, p_rSourceRoute.time.c_str(), -1, SQLITE_STATIC)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertSourceRoute->sqlite3_bind_int' for 'time' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = m_oInsertSourceRoute_compiled.Step_Exec()) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertSourceRoute->Step_Exec' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
	}
}

//superframes
void SqliteDevicesDal::CleanDeviceSuperframes()
{
	LOG_DEBUG_APP("[DevicesDAL]: clean Superframes report");

	sqlitexx::Command sqlCommand(connection, "DELETE FROM Superframes;");
	sqlCommand.ExecuteNonQuery();
}

void SqliteDevicesDal::CleanDeviceSuperframes(int deviceID)
{
	LOG_DEBUG_APP("[DevicesDAL]: clean all Superframes for deviceID=" << deviceID);

	sqlitexx::Command sqlCommand(connection, "DELETE FROM Superframes WHERE DeviceID = ?001;");
	sqlCommand.BindParam(1, deviceID);
	sqlCommand.ExecuteNonQuery();
}

void SqliteDevicesDal::InsertSuperframe(const Superframe& p_rSuperframe)
{
	LOG_DEBUG_APP("[DevicesDAL]: Add Superframe for device id=" << p_rSuperframe.m_nDeviceId);
	{
		int rez = SQLITE_OK;
		if (!m_oInsertSuperframes_compiled.GetStmt())
		{
			if ((rez = m_oInsertSuperframes_compiled.Prepare( "INSERT INTO Superframes(SuperframeID, DeviceID, NumberOfTimeSlots, Active, HandheldSuperframe, Timestamp)"
				" VALUES(?001, ?002, ?003, ?004, ?005, ?006); ")) != SQLITE_OK)
			{
				connection.LogIfLastError(rez);
				LOG_ERROR_APP("[DevicesDAL]: 'InsertSuperframe->Prepare' FAILED, err=" << rez);
				std::exception ex;
				throw ex;
			}
		}

		// get
		sqlite3_stmt *stmt = NULL;
		if (!(stmt = m_oInsertSuperframes_compiled.GetStmt()))
		{
			LOG_ERROR_APP("[DevicesDAL]: 'InsertSuperframe->GetStmt' FAILED");
			std::exception ex;
			throw ex;
		}

		int index = 1;
		if ((rez = sqlite3_bind_int ( stmt, index++, p_rSuperframe.m_nSuperframeId)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertSuperframe->sqlite3_bind_int' for 'p_rSuperframe.m_nSuperframeId' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, p_rSuperframe.m_nDeviceId)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertSuperframe->sqlite3_bind_int' for 'p_rSuperframe.m_nDeviceId' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, p_rSuperframe.m_nNumberOfTimeSlots)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertSuperframe->sqlite3_bind_int' for 'newStatus' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, p_rSuperframe.m_nActive)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertSuperframe->sqlite3_bind_int' for 'p_rSuperframe.m_nActive' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, p_rSuperframe.m_nHandheldSuperframe)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertSuperframe->sqlite3_bind_int' for 'p_rSuperframe.m_nHandheldSuperframe' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_text ( stmt, index++, p_rSuperframe.time.c_str(), -1, SQLITE_STATIC)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertSuperframe->sqlite3_bind_int' for 'time' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = m_oInsertSuperframes_compiled.Step_Exec()) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertSuperframe->Step_Exec' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
	}
}

void SqliteDevicesDal::DeleteSuperframe(int deviceID, int superframeID)
{
	LOG_DEBUG_APP("[DevicesDAL]: clean Superframe ID=" << superframeID << " for deviceID=" << deviceID);
	{
		int rez = SQLITE_OK;
		if (!m_oDeleteSuperframes_compiled.GetStmt())
		{
			if ((rez = m_oDeleteSuperframes_compiled.Prepare(
			            "DELETE FROM Superframes "
				        " WHERE DeviceID = ?001 AND SuperframeID = ?002;")) != SQLITE_OK)
			{
				connection.LogIfLastError(rez);
				LOG_ERROR_APP("[DevicesDAL]: 'DeleteSuperframe->Prepare' FAILED, err=" << rez);
				std::exception ex;
				throw ex;
			}
		}

		// get
		sqlite3_stmt *stmt = NULL;
		if (!(stmt = m_oDeleteSuperframes_compiled.GetStmt()))
		{
			LOG_ERROR_APP("[DevicesDAL]: 'DeleteSuperframe->GetStmt' FAILED");
			std::exception ex;
			throw ex;
		}

		int index = 1;
		if ((rez = sqlite3_bind_int ( stmt, index++, deviceID)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'DeleteSuperframe->sqlite3_bind_int' for 'deviceID' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, superframeID)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'DeleteSuperframe->sqlite3_bind_int' for 'SuperframeID' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = m_oDeleteSuperframes_compiled.Step_Exec()) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'DeleteSuperframe->Step_Exec' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
	}
}

//device schedule link
void SqliteDevicesDal::CleanDeviceScheduleLinks()
{
	LOG_DEBUG_APP("[DevicesDAL]: clean device schedule links report.");

	sqlitexx::Command sqlCommand(connection, "DELETE FROM DeviceScheduleLinks;");
	sqlCommand.ExecuteNonQuery();
}

void SqliteDevicesDal::CleanDeviceScheduleLinks(int p_nDeviceId)
{
	LOG_DEBUG_APP("[DevicesDAL]: clean all device schedule links for deviceID=" << p_nDeviceId);

	sqlitexx::Command sqlCommand(connection, "DELETE FROM DeviceScheduleLinks WHERE DeviceID = ?001;");
	sqlCommand.BindParam(1, p_nDeviceId);
	sqlCommand.ExecuteNonQuery();
}

void SqliteDevicesDal::CleanDeviceScheduleLink(int p_nDeviceId,const DeviceScheduleLink& link)
{
	LOG_DEBUG_APP("[DevicesDAL]: clean device schedule link: deviceID=" << p_nDeviceId << " superframeID=" << link.m_nSuperframeId
									<< " peerID=" << link.m_nPeerId << " slotIndex=" << link.m_nSlotIndex);
	{
		int rez = SQLITE_OK;
		if (!m_oDeleteScheduleLink1_compiled.GetStmt())
		{
			if ((rez = m_oDeleteScheduleLink1_compiled.Prepare(
			            "DELETE FROM DeviceScheduleLinks "
				        " WHERE DeviceID = ?001 AND SuperframeID = ?002 AND PeerID = ?003 AND SlotIndex = ?004;")) != SQLITE_OK)
			{
				connection.LogIfLastError(rez);
				LOG_ERROR_APP("[DevicesDAL]: 'CleanDeviceScheduleLink->Prepare' FAILED, err=" << rez);
				std::exception ex;
				throw ex;
			}
		}
		// get
		sqlite3_stmt *stmt = NULL;
		if (!(stmt = m_oDeleteScheduleLink1_compiled.GetStmt()))
		{
			LOG_ERROR_APP("[DevicesDAL]: 'CleanDeviceScheduleLink->GetStmt' FAILED");
			std::exception ex;
			throw ex;
		}

		int index = 1;
		if ((rez = sqlite3_bind_int ( stmt, index++, p_nDeviceId)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'CleanDeviceScheduleLink->sqlite3_bind_int' for 'p_nDeviceId' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, link.m_nSuperframeId)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'CleanDeviceScheduleLink->sqlite3_bind_int' for 'link.m_nSuperframeId' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, link.m_nPeerId)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'CleanDeviceScheduleLink->sqlite3_bind_int' for 'link.m_nPeerId' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, link.m_nSlotIndex)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'CleanDeviceScheduleLink->sqlite3_bind_int' for 'link.m_nSlotIndex' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = m_oDeleteScheduleLink1_compiled.Step_Exec()) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'CleanDeviceScheduleLink->Step_Exec' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
	}
}

void SqliteDevicesDal::InsertDeviceScheduleLink(int p_nDeviceId,const DeviceScheduleLink& p_rDeviceScheduleLink)
{
	LOG_DEBUG_APP("[DevicesDAL]: Add device schedule link for device id=" << p_nDeviceId
		<< " SuperframeId=" << p_rDeviceScheduleLink.m_nSuperframeId 
		<< " PeerId=" << p_rDeviceScheduleLink.m_nPeerId 
		<< " SlotIndex=" << p_rDeviceScheduleLink.m_nSlotIndex
		<< " ChannelOffset=" << p_rDeviceScheduleLink.m_nChannelOffset
		<< " Transmit=" << p_rDeviceScheduleLink.m_nTransmit
		<< " Receive=" << p_rDeviceScheduleLink.m_nReceive
		<< " Shared=" << p_rDeviceScheduleLink.m_nShared
		<< " LinkType=" << p_rDeviceScheduleLink.m_nLinkType );
	{
		int rez = SQLITE_OK;
		if (!m_oInsertScheduleLink_compiled.GetStmt())
		{
			if ((rez = m_oInsertScheduleLink_compiled.Prepare( "INSERT INTO DeviceScheduleLinks(SuperframeID, DeviceID, PeerID, SlotIndex, ChannelOffset, Transmit, Receive, Shared, LinkType, Timestamp)"
				" VALUES(?001, ?002, ?003, ?004, ?005, ?006, ?007, ?008, ?009, ?010); ")) != SQLITE_OK)
			{
				connection.LogIfLastError(rez);
				LOG_ERROR_APP("[DevicesDAL]: 'InsertDeviceScheduleLink->Prepare' FAILED, err=" << rez);
				std::exception ex;
				throw ex;
			}
		}

		// get
		sqlite3_stmt *stmt = NULL;
		if (!(stmt = m_oInsertScheduleLink_compiled.GetStmt()))
		{
			LOG_ERROR_APP("[DevicesDAL]: 'InsertDeviceScheduleLink->GetStmt' FAILED");
			std::exception ex;
			throw ex;
		}

		int index = 1;
		if ((rez = sqlite3_bind_int ( stmt, index++, p_rDeviceScheduleLink.m_nSuperframeId)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertDeviceScheduleLink->sqlite3_bind_int' for 'p_rDeviceScheduleLink.m_nSuperframeId' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, p_nDeviceId)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertDeviceScheduleLink->sqlite3_bind_int' for 'p_nDeviceId' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, p_rDeviceScheduleLink.m_nPeerId)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertDeviceScheduleLink->sqlite3_bind_int' for 'p_rDeviceScheduleLink.m_nPeerId' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, p_rDeviceScheduleLink.m_nSlotIndex)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertDeviceScheduleLink->sqlite3_bind_int' for 'p_rDeviceScheduleLink.m_nSlotIndex' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, p_rDeviceScheduleLink.m_nChannelOffset)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertDeviceScheduleLink->sqlite3_bind_int' for 'p_rDeviceScheduleLink.m_nChannelOffset' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, p_rDeviceScheduleLink.m_nTransmit)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertDeviceScheduleLink->sqlite3_bind_int' for 'p_rDeviceScheduleLink.m_nTransmit' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, p_rDeviceScheduleLink.m_nReceive)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertDeviceScheduleLink->sqlite3_bind_int' for 'p_rDeviceScheduleLink.m_nReceive' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, p_rDeviceScheduleLink.m_nShared)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertDeviceScheduleLink->sqlite3_bind_int' for 'p_rDeviceScheduleLink.m_nShared' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, p_rDeviceScheduleLink.m_nLinkType)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertDeviceScheduleLink->sqlite3_bind_int' for 'p_rDeviceScheduleLink.m_nLinkType' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_text ( stmt, index++, p_rDeviceScheduleLink.time.c_str(), -1, SQLITE_STATIC)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertDeviceScheduleLink->sqlite3_bind_int' for 'p_rDeviceScheduleLink.time' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = m_oInsertScheduleLink_compiled.Step_Exec()) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertDeviceScheduleLink->Step_Exec' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
	}

}
void SqliteDevicesDal::DeleteLink(int p_nDeviceId, int superframeID, int neighbID, int slotNo)
{
	LOG_DEBUG_APP("[DevicesDAL]: delete device schedule link for superframeID=" << superframeID << " between devID=" << p_nDeviceId << " and devID=" << neighbID << " and slot No=" << slotNo);
	{
		int rez = SQLITE_OK;
		if (!m_oDeleteScheduleLink2_compiled.GetStmt())
		{
			if ((rez = m_oDeleteScheduleLink2_compiled.Prepare(
			            "DELETE FROM DeviceScheduleLinks "
				        " WHERE DeviceID = ?001 AND SuperframeID = ?002 AND PeerID = ?003 AND SlotIndex = ?004;")) != SQLITE_OK)
			{
				connection.LogIfLastError(rez);
				LOG_ERROR_APP("[DevicesDAL]: 'DeleteLink->Prepare' FAILED, err=" << rez);
				std::exception ex;
				throw ex;
			}
		}
		// get
		sqlite3_stmt *stmt = NULL;
		if (!(stmt = m_oDeleteScheduleLink2_compiled.GetStmt()))
		{
			LOG_ERROR_APP("[DevicesDAL]: 'DeleteLink->GetStmt' FAILED");
			std::exception ex;
			throw ex;
		}

		int index = 1;
		if ((rez = sqlite3_bind_int ( stmt, index++, p_nDeviceId)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'DeleteLink->sqlite3_bind_int' for 'p_nDeviceId' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, superframeID)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'DeleteLink->sqlite3_bind_int' for 'superframeID' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, neighbID)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'DeleteLink->sqlite3_bind_int' for 'neighbID' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, slotNo)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'DeleteLink->sqlite3_bind_int' for 'slotNo' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = m_oDeleteScheduleLink2_compiled.Step_Exec()) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'DeleteLink->Step_Exec' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
	}
}

//device health
void SqliteDevicesDal::CleanDeviceHealthReport(int deviceID)
{
	LOG_DEBUG_APP("[DevicesDAL]: clean device health for deviceID=" << deviceID);

	{
		int rez = SQLITE_OK;
		if (!m_oDeleteDeviceHealth_compiled.GetStmt())
		{
			if ((rez = m_oDeleteDeviceHealth_compiled.Prepare(
			    "DELETE FROM ReportDeviceHealth "
				" WHERE DeviceID = ?001;")) != SQLITE_OK)
			{
				connection.LogIfLastError(rez);
				LOG_ERROR_APP("[DevicesDAL]: 'CleanDeviceHealthReport->Prepare' FAILED, err=" << rez);
				std::exception ex;
				throw ex;
			}
		}
		// get
		sqlite3_stmt *stmt = NULL;
		if (!(stmt = m_oDeleteDeviceHealth_compiled.GetStmt()))
		{
			LOG_ERROR_APP("[DevicesDAL]: 'CleanDeviceHealthReport->GetStmt' FAILED");
			std::exception ex;
			throw ex;
		}

		int index = 1;
		if ((rez = sqlite3_bind_int ( stmt, index++, deviceID)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'CleanDeviceHealthReport->sqlite3_bind_int' for 'deviceID' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = m_oDeleteDeviceHealth_compiled.Step_Exec()) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'CleanDeviceHealthReport->Step_Exec' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
	}
}

void SqliteDevicesDal::InsertDeviceHealthReport(const DeviceHealth &devHealth)
{
	LOG_DEBUG_APP("[DevicesDAL]: Add device health for device id=" << devHealth.m_nDeviceId
		<< " PowerStatus=" << devHealth.m_nPowerStatus 
		<< " Generated=" << devHealth.m_nGenerated 
		<< " Terminated=" << devHealth.m_nTerminated
		<< " DllFailures=" << devHealth.m_nDllFailures
		<< " NlFailures=" << devHealth.m_nNlFailures
		<< " CrcErrors=" << devHealth.m_nCrcErrors
		<< " NonceLost=" << devHealth.m_nNonceLost);

	{
		int rez = SQLITE_OK;
		if (!m_oInsertDeviceHealth_compiled.GetStmt())
		{
			if ((rez = m_oInsertDeviceHealth_compiled.Prepare( "INSERT INTO ReportDeviceHealth(DeviceID, PowerStatus, Generated, Terminated, "
				" DllFailures, NlFailures, CrcErrors, NonceLost, Timestamp)"
				" VALUES(?001, ?002, ?003, ?004, ?005, ?006, ?007, ?008, ?009); ")) != SQLITE_OK)
			{
				connection.LogIfLastError(rez);
				LOG_ERROR_APP("[DevicesDAL]: 'InsertDeviceHealthReport->Prepare' FAILED, err=" << rez);
				std::exception ex;
				throw ex;
			}
		}
		// get
		sqlite3_stmt *stmt = NULL;
		if (!(stmt = m_oInsertDeviceHealth_compiled.GetStmt()))
		{
			LOG_ERROR_APP("[DevicesDAL]: 'InsertDeviceHealthReport->GetStmt' FAILED");
			std::exception ex;
			throw ex;
		}

		int index = 1;
		if ((rez = sqlite3_bind_int ( stmt, index++, devHealth.m_nDeviceId)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertDeviceHealthReport->sqlite3_bind_int' for 'devHealth.m_nDeviceId' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, devHealth.m_nPowerStatus)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertDeviceHealthReport->sqlite3_bind_int' for 'devHealth.m_nPowerStatus' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, devHealth.m_nGenerated)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertDeviceHealthReport->sqlite3_bind_int' for 'devHealth.m_nGenerated' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, devHealth.m_nTerminated)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertDeviceHealthReport->sqlite3_bind_int' for 'devHealth.m_nTerminated' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, devHealth.m_nDllFailures)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertDeviceHealthReport->sqlite3_bind_int' for 'devHealth.m_nDllFailures' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, devHealth.m_nNlFailures)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertDeviceHealthReport->sqlite3_bind_int' for 'devHealth.m_nNlFailures' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, devHealth.m_nCrcErrors)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertDeviceHealthReport->sqlite3_bind_int' for 'devHealth.m_nCrcErrors' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, devHealth.m_nNonceLost)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertDeviceHealthReport->sqlite3_bind_int' for 'devHealth.m_nNonceLost' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_text ( stmt, index++, devHealth.time.c_str(), -1, SQLITE_STATIC)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertDeviceHealthReport->sqlite3_bind_int' for 'newStatus' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = m_oInsertDeviceHealth_compiled.Step_Exec()) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertDeviceHealthReport->Step_Exec' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
	}
}

//neighbors health report
void SqliteDevicesDal::CleanDeviceNeighborHealth(int deviceID, int neighbID)
{
	LOG_DEBUG_APP("[DevicesDAL]: clean device neighbor health for deviceID=" << deviceID << " and peerID=" << neighbID);

	{
		int rez = SQLITE_OK;
		if (!m_oDeleteNeighbsHealth_compiled.GetStmt())
		{
			if ((rez = m_oDeleteNeighbsHealth_compiled.Prepare(
			            "DELETE FROM ReportNeighborHealthList "
				        " WHERE DeviceID = ?001 AND PeerID = ?002;")) != SQLITE_OK)
			{
				connection.LogIfLastError(rez);
				LOG_ERROR_APP("[DevicesDAL]: 'CleanDeviceNeighborHealth->Prepare' FAILED, err=" << rez);
				std::exception ex;
				throw ex;
			}
		}
		// get
		sqlite3_stmt *stmt = NULL;
		if (!(stmt = m_oDeleteNeighbsHealth_compiled.GetStmt()))
		{
			LOG_ERROR_APP("[DevicesDAL]: 'CleanDeviceNeighborHealth->GetStmt' FAILED");
			std::exception ex;
			throw ex;
		}

		int index = 1;
		if ((rez = sqlite3_bind_int ( stmt, index++, deviceID)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'CleanDeviceNeighborHealth->sqlite3_bind_int' for 'deviceID' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, neighbID)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'CleanDeviceNeighborHealth->sqlite3_bind_int' for 'neighbID' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}

		if ((rez = m_oDeleteNeighbsHealth_compiled.Step_Exec()) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'CleanDeviceNeighborHealth->Step_Exec' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
	}
}

void SqliteDevicesDal::CleanDeviceNeighborsHealth(int deviceID)
{
	LOG_DEBUG_APP("[DevicesDAL]: clean device neighbors health for deviceID=" << deviceID);

	sqlitexx::Command sqlCommand(connection, "DELETE FROM ReportNeighborHealthList WHERE DeviceID = ?001;");
	sqlCommand.BindParam(1, deviceID);
	sqlCommand.ExecuteNonQuery();
}

void SqliteDevicesDal::InsertDeviceNeighborsHealth(const DeviceNeighborsHealth& p_rDeviceNeighborsHealth)
{
	LOG_DEBUG_APP("[DevicesDAL]: Add device neighbors health for device id=" << p_rDeviceNeighborsHealth.m_nDeviceID);

	std::list<NeighborHealth>::const_iterator it = p_rDeviceNeighborsHealth.m_oNeighborsList.begin();
	{
		int rez = SQLITE_OK;
		if (!m_oInsertNeighbsHealth_compiled.GetStmt())
		{
			if ((rez = m_oInsertNeighbsHealth_compiled.Prepare(
			            "INSERT INTO ReportNeighborHealthList(DeviceID, PeerID, ClockSource, RSL, Transmissions, FailedTransmissions, Receptions, Timestamp) "
				        "VALUES(?001, ?002, ?003, ?004, ?005, ?006, ?007, ?008); ")) != SQLITE_OK)
			{
				connection.LogIfLastError(rez);
				LOG_ERROR_APP("[DevicesDAL]: 'InsertDeviceNeighborsHealth->Prepare' FAILED, err=" << rez);
				std::exception ex;
				throw ex;
			}
		}

		for ( ; it != p_rDeviceNeighborsHealth.m_oNeighborsList.end() ; ++it )
		{
			// get
			sqlite3_stmt *stmt = NULL;
			if (!(stmt = m_oInsertNeighbsHealth_compiled.GetStmt()))
			{
				LOG_ERROR_APP("[DevicesDAL]: 'InsertDeviceNeighborsHealth->GetStmt' FAILED");
				std::exception ex;
				throw ex;
			}

			int index = 1;
			if ((rez = sqlite3_bind_int ( stmt, index++, p_rDeviceNeighborsHealth.m_nDeviceID)) != SQLITE_OK)
			{
				connection.LogIfLastError(rez);
				LOG_ERROR_APP("[DevicesDAL]: 'InsertDeviceNeighborsHealth->sqlite3_bind_int' for 'p_rDeviceNeighborsHealth.m_nDeviceID' FAILED, err=" << rez);
				std::exception ex;
				throw ex;
			}
			if ((rez = sqlite3_bind_int ( stmt, index++, it->m_nPeerID)) != SQLITE_OK)
			{
				connection.LogIfLastError(rez);
				LOG_ERROR_APP("[DevicesDAL]: 'InsertDeviceNeighborsHealth->sqlite3_bind_int' for 'it->m_nPeerID' FAILED, err=" << rez);
				std::exception ex;
				throw ex;
			}
			if ((rez = sqlite3_bind_int ( stmt, index++, it->m_nClockSource)) != SQLITE_OK)
			{
				connection.LogIfLastError(rez);
				LOG_ERROR_APP("[DevicesDAL]: 'InsertDeviceNeighborsHealth->sqlite3_bind_int' for 'it->m_nClockSource' FAILED, err=" << rez);
				std::exception ex;
				throw ex;
			}
			if ((rez = sqlite3_bind_int ( stmt, index++, it->m_nRSL)) != SQLITE_OK)
			{
				connection.LogIfLastError(rez);
				LOG_ERROR_APP("[DevicesDAL]: 'InsertDeviceNeighborsHealth->sqlite3_bind_int' for 'it->m_nRSL' FAILED, err=" << rez);
				std::exception ex;
				throw ex;
			}
			if ((rez = sqlite3_bind_int ( stmt, index++, it->m_nTransmissions)) != SQLITE_OK)
			{
				connection.LogIfLastError(rez);
				LOG_ERROR_APP("[DevicesDAL]: 'InsertDeviceNeighborsHealth->sqlite3_bind_int' for 'it->m_nTransmissions' FAILED, err=" << rez);
				std::exception ex;
				throw ex;
			}
			if ((rez = sqlite3_bind_int ( stmt, index++, it->m_nFailedTransmissions)) != SQLITE_OK)
			{
				connection.LogIfLastError(rez);
				LOG_ERROR_APP("[DevicesDAL]: 'InsertDeviceNeighborsHealth->sqlite3_bind_int' for 'it->m_nFailedTransmissions' FAILED, err=" << rez);
				std::exception ex;
				throw ex;
			}
			if ((rez = sqlite3_bind_int ( stmt, index++, it->m_nReceptions)) != SQLITE_OK)
			{
				connection.LogIfLastError(rez);
				LOG_ERROR_APP("[DevicesDAL]: 'InsertDeviceNeighborsHealth->sqlite3_bind_int' for 'it->m_nReceptions' FAILED, err=" << rez);
				std::exception ex;
				throw ex;
			}
			if ((rez = sqlite3_bind_text ( stmt, index++, it->time.c_str(), -1, SQLITE_STATIC)) != SQLITE_OK)
			{
				connection.LogIfLastError(rez);
				LOG_ERROR_APP("[DevicesDAL]: 'InsertDeviceNeighborsHealth->sqlite3_bind_int' for 'time' FAILED, err=" << rez);
				std::exception ex;
				throw ex;
			}
			if ((rez = m_oInsertNeighbsHealth_compiled.Step_Exec()) != SQLITE_OK)
			{
				connection.LogIfLastError(rez);
				LOG_ERROR_APP("[DevicesDAL]: 'InsertDeviceNeighborsHealth->Step_Exec' FAILED, err=" << rez);
				std::exception ex;
				throw ex;
			}
			m_oInsertNeighbsHealth_compiled.ResetStep();
		}
	}
}


//neighbors signal level report
void SqliteDevicesDal::CleanDevNeighbSignalLevel(int deviceID, int neighbID)
{
	LOG_DEBUG_APP("[DevicesDAL]: clean  neighb_signals for device id=" << deviceID << " neighbID=" << neighbID);
	{
		int rez = SQLITE_OK;
		if (!m_oDeleteNeighbsSigLevel_compiled.GetStmt())
		{
			if ((rez = m_oDeleteNeighbsSigLevel_compiled.Prepare(
			            "DELETE FROM ReportNeighborSignalLevels "
				        " WHERE DeviceID = ?001 AND PeerID = ?002;")) != SQLITE_OK)
			{
				connection.LogIfLastError(rez);
				LOG_ERROR_APP("[DevicesDAL]: 'CleanDevNeighbSignalLevel->Prepare' FAILED, err=" << rez);
				std::exception ex;
				throw ex;
			}
		}
		// get
		sqlite3_stmt *stmt = NULL;
		if (!(stmt = m_oDeleteNeighbsSigLevel_compiled.GetStmt()))
		{
			LOG_ERROR_APP("[DevicesDAL]: 'CleanDevNeighbSignalLevel->GetStmt' FAILED");
			std::exception ex;
			throw ex;
		}

		int index = 1;
		if ((rez = sqlite3_bind_int ( stmt, index++, deviceID)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'CleanDevNeighbSignalLevel->sqlite3_bind_int' for 'deviceID' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, neighbID)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'CleanDevNeighbSignalLevel->sqlite3_bind_int' for 'neighbID' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}

		if ((rez = m_oDeleteNeighbsSigLevel_compiled.Step_Exec()) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'CleanDevNeighbSignalLevel->Step_Exec' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
	}
}

void SqliteDevicesDal::CleanDevNeighbSignalLevels(int deviceID)
{
	LOG_DEBUG_APP("[DevicesDAL]: clean  neighb_signals for device id=" << deviceID);

	sqlitexx::Command sqlCommand(connection, "DELETE FROM ReportNeighborSignalLevels WHERE DeviceID = ?001;");
	sqlCommand.BindParam(1, deviceID);
	sqlCommand.ExecuteNonQuery();
}

void SqliteDevicesDal::InsertDevNeighbSignalLevels(int deviceID, int neighbID, int signalLevel)
{
	LOG_DEBUG_APP("[DevicesDAL]: create neighbour_signal to dev=" << neighbID << " for device id=" << deviceID);

	{
		int rez = SQLITE_OK;
		if (!m_oInsertNeighbsSigLevel_compiled.GetStmt())
		{
			if ((rez = m_oInsertNeighbsSigLevel_compiled.Prepare( "INSERT INTO ReportNeighborSignalLevels(DeviceID, PeerID, RSL)"
				" VALUES (?001, ?002, ?003);")) != SQLITE_OK)
			{
				connection.LogIfLastError(rez);
				LOG_ERROR_APP("[DevicesDAL]: 'InsertDevNeighbSignalLevels->Prepare' FAILED, err=" << rez);
				std::exception ex;
				throw ex;
			}
		}
		// get
		sqlite3_stmt *stmt = NULL;
		if (!(stmt = m_oInsertNeighbsSigLevel_compiled.GetStmt()))
		{
			LOG_ERROR_APP("[DevicesDAL]: 'InsertDevNeighbSignalLevels->GetStmt' FAILED");
			std::exception ex;
			throw ex;
		}

		int index = 1;
		if ((rez = sqlite3_bind_int ( stmt, index++, deviceID)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertDevNeighbSignalLevels->sqlite3_bind_int' for 'deviceID' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, neighbID)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertDevNeighbSignalLevels->sqlite3_bind_int' for 'neighbID' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = sqlite3_bind_int ( stmt, index++, signalLevel)) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertDevNeighbSignalLevels->sqlite3_bind_int' for 'signalLevel' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
		if ((rez = m_oInsertNeighbsSigLevel_compiled.Step_Exec()) != SQLITE_OK)
		{
			connection.LogIfLastError(rez);
			LOG_ERROR_APP("[DevicesDAL]: 'InsertDevNeighbSignalLevels->Step_Exec' FAILED, err=" << rez);
			std::exception ex;
			throw ex;
		}
	}
}

void SqliteDevicesDal::Log_SignalLvel_Info(int deviceID)
{
	LOG_DEBUG_APP("[DevicesDAL]: Get signal levels from database for devID=" << deviceID);

	sqlitexx::CSqliteStmtHelper oSqlDev (connection);
	int rez = SQLITE_OK;
	if ((rez = oSqlDev.Prepare( "SELECT DeviceID, PeerID, RSL FROM ReportNeighborSignalLevels WHERE DeviceID = ?001;")) != SQLITE_OK)
	{
		connection.LogIfLastError(rez);
		LOG_ERROR_APP("[DevicesDAL]: 'Log_SignalLvel_Info->Prepare' FAILED, err=" << rez);
		std::exception ex;
		throw ex;
	}
	// get
	sqlite3_stmt *stmt = NULL;
	if (!(stmt = oSqlDev.GetStmt()))
	{
		LOG_ERROR_APP("[DevicesDAL]: 'Log_SignalLvel_Info->GetStmt' FAILED");
		std::exception ex;
		throw ex;
	}

	int index = 1;
	if ((rez = sqlite3_bind_int ( stmt, index++, deviceID)) != SQLITE_OK)
	{
		connection.LogIfLastError(rez);
		LOG_ERROR_APP("[DevicesDAL]: 'Log_SignalLvel_Info->sqlite3_bind_int' for 'deviceID' FAILED, err=" << rez);
		std::exception ex;
		throw ex;
	}
	if ((rez=oSqlDev.Step_GetRow()) != SQLITE_ROW)
	{
		connection.LogIfLastError(rez);
		LOG_ERROR_APP("[DevicesDAL]: 'Log_SignalLvel_Info->Step_GetRow' FAILED, err=" << rez);
		std::exception ex;
		throw ex;
	}

	do 
	{
		LOG_DEBUG_APP("[Log_SignalLvel_Info]: deviceID=" << oSqlDev.Column_GetInt(0)
		            << " peerID=" << oSqlDev.Column_GetInt(1)
					<< " signalLevel=" << oSqlDev.Column_GetInt(2));
	}
	while (oSqlDev.Step_GetRow() == SQLITE_ROW);

}

//alarms
void SqliteDevicesDal::InsertAlarm(int p_nDeviceId, int p_nAlarmType, int  PeerID_GraphID, const std::string& time)
{
	LOG_DEBUG_APP("[DevicesDAL]: insert alarm for deviceID=" << p_nDeviceId
		<<"AlarmType="<<p_nAlarmType
		<<"PeerID_GraphID="<<PeerID_GraphID);

	sqlitexx::Command sqlCommand(connection, 
		"INSERT INTO Alarms(DeviceID, AlarmType, AlarmTime, PeerID_GraphID) "
		"VALUES(?001, ?002, ?003, ?004); ");
	sqlCommand.BindParam(1, p_nDeviceId);
	sqlCommand.BindParam(2, p_nAlarmType);
	sqlCommand.BindParam(3, time);
	sqlCommand.BindParam(4, PeerID_GraphID);
	sqlCommand.ExecuteNonQuery();
}


void SqliteDevicesDal::InsertAlarm(int p_nDeviceId, int p_nAlarmType, int PeerID_GraphID, int MIC, const std::string& time)
{
	LOG_DEBUG_APP("[DevicesDAL]: insert alarm for deviceID=" << p_nDeviceId
		<<"AlarmType="<<p_nAlarmType
		<<"PeerID_GraphID="<<PeerID_GraphID
		<<"MIC="<<MIC);

	sqlitexx::Command sqlCommand(connection, 
		"INSERT INTO Alarms(DeviceID, AlarmType, AlarmTime, PeerID_GraphID, MIC) "
		"VALUES(?001, ?002, ?003, ?004, ?005); ");
	sqlCommand.BindParam(1, p_nDeviceId);
	sqlCommand.BindParam(2, p_nAlarmType);
	sqlCommand.BindParam(3, time);
	sqlCommand.BindParam(4, PeerID_GraphID);
	sqlCommand.BindParam(5, MIC);
	sqlCommand.ExecuteNonQuery();
}

//burst
void SqliteDevicesDal::UpdateBurstMessageCounter(int p_nDeviceId, BurstMessage& p_rBurstMessage, const CMicroSec&  p_rLastUpdate, int p_nReceived, int p_nMissed)
{
    LOG_DEBUG_APP("[DevicesDAL]: UpdateBurstMessageCounter for burst message " << ((int)p_rBurstMessage.burstMessage) << " for device id=" << p_nDeviceId);

    int rez = SQLITE_OK;
    if (!m_oUpdateBurstMessageCounter_compiled.GetStmt())
    {
        if ((rez = m_oUpdateBurstMessageCounter_compiled.Prepare(
                    "UPDATE BurstCounters "
                    "   SET LastUpdate = ?001, Received = ?002, Missed = ?003 "
                    " WHERE DeviceID = ?004 AND BurstMessage = ?005 AND CommandNumber = ?006;")) != SQLITE_OK)
        {
            connection.LogIfLastError(rez);
            LOG_ERROR_APP("[DevicesDAL]: 'UpdateBurstMessageCounter->Prepare' FAILED, err=" << rez);
            std::exception ex;
            throw ex;
        }
    }
    // get
    sqlite3_stmt *stmt = NULL;
    if (!(stmt = m_oUpdateBurstMessageCounter_compiled.GetStmt()))
    {
        LOG_ERROR_APP("[DevicesDAL]: 'UpdateBurstMessageCounter->GetStmt' FAILED");
        std::exception ex;
        throw ex;
    }

    if ((rez = sqlite3_bind_text(stmt, 1, p_rLastUpdate.GetElapsedTimeStr(), -1, SQLITE_STATIC)) != SQLITE_OK)
    {
        connection.LogIfLastError(rez);
        LOG_ERROR_APP("[DevicesDAL]: 'UpdateBurstMessageCounter->sqlite3_bind_text' for 'LastUpdate' FAILED, err=" << rez);
        std::exception ex;
        throw ex;
    }
    if ((rez = sqlite3_bind_int(stmt, 2, p_nReceived)) != SQLITE_OK)
    {
        connection.LogIfLastError(rez);
        LOG_ERROR_APP("[DevicesDAL]: 'UpdateBurstMessageCounter->sqlite3_bind_int' for 'Received' FAILED, err=" << rez);
        std::exception ex;
        throw ex;
    }
    if ((rez = sqlite3_bind_int(stmt, 3, p_nMissed)) != SQLITE_OK)
    {
        connection.LogIfLastError(rez);
        LOG_ERROR_APP("[DevicesDAL]: 'UpdateBurstMessageCounter->sqlite3_bind_int' for 'Missed' FAILED, err=" << rez);
        std::exception ex;
        throw ex;
    }
    if ((rez = sqlite3_bind_int(stmt, 4, p_nDeviceId)) != SQLITE_OK)
    {
        connection.LogIfLastError(rez);
        LOG_ERROR_APP("[DevicesDAL]: 'UpdateBurstMessageCounter->sqlite3_bind_int' for 'DeviceID' FAILED, err=" << rez);
        std::exception ex;
        throw ex;
    }
    if ((rez = sqlite3_bind_int(stmt, 5, (int)p_rBurstMessage.burstMessage)) != SQLITE_OK)
    {
        connection.LogIfLastError(rez);
        LOG_ERROR_APP("[DevicesDAL]: 'UpdateBurstMessageCounter->sqlite3_bind_int' for 'BurstMessage' FAILED, err=" << rez);
        std::exception ex;
        throw ex;
    }
    if ((rez = sqlite3_bind_int(stmt, 6, (int)p_rBurstMessage.cmdNo)) != SQLITE_OK)
    {
        connection.LogIfLastError(rez);
        LOG_ERROR_APP("[DevicesDAL]: 'UpdateBurstMessageCounter->sqlite3_bind_int' for 'CommandNumber' FAILED, err=" << rez);
        std::exception ex;
        throw ex;
    }
    if ((rez = m_oUpdateBurstMessageCounter_compiled.Step_Exec()) != SQLITE_OK)
    {
        connection.LogIfLastError(rez);
        LOG_ERROR_APP("[DevicesDAL]: 'UpdateBurstMessageCounter->Step_Exec' FAILED, err=" << rez);
        std::exception ex;
        throw ex;
    }
}

void SqliteDevicesDal::CreateBurstMessageCounter(int p_nDeviceId, const BurstMessage& p_rBurstMessage)
{
    LOG_DEBUG_APP("[DevicesDAL]: CreateBurstMessageCounter for burst message " << ((int)p_rBurstMessage.burstMessage) << " for device id=" << p_nDeviceId);

    int rez = SQLITE_OK;
    if (!m_oInsertBurstMessageCounter_compiled.GetStmt())
    {
        if ((rez = m_oInsertBurstMessageCounter_compiled.Prepare(
                    "INSERT INTO BurstCounters(DeviceID, BurstMessage, CommandNumber, LastUpdate, Received, Missed) "
                    "VALUES(?001, ?002, ?003, '1970-01-01 00:00:00', 0, 0);")) != SQLITE_OK)
        {
            connection.LogIfLastError(rez);
            LOG_ERROR_APP("[DevicesDAL]: 'CreateBurstMessageCounter->Prepare' FAILED, err=" << rez);
            std::exception ex;
            throw ex;
        }
    }

    sqlite3_stmt *stmt = NULL;
    if (!(stmt = m_oInsertBurstMessageCounter_compiled.GetStmt()))
    {
        LOG_ERROR_APP("[DevicesDAL]: 'CreateBurstMessageCounter->GetStmt' FAILED");
        std::exception ex;
        throw ex;
    }
    if ((rez = sqlite3_bind_int(stmt, 1, p_nDeviceId)) != SQLITE_OK)
    {
        connection.LogIfLastError(rez);
        LOG_ERROR_APP("[DevicesDAL]: 'CreateBurstMessageCounter->sqlite3_bind_int' for 'deviceID' FAILED, err=" << rez);
        std::exception ex;
        throw ex;
    }
    if ((rez = sqlite3_bind_int(stmt, 2, (int)p_rBurstMessage.burstMessage)) != SQLITE_OK)
    {
        connection.LogIfLastError(rez);
        LOG_ERROR_APP("[DevicesDAL]: 'CreateBurstMessageCounter->sqlite3_bind_int' for 'burstMessage' FAILED, err=" << rez);
        std::exception ex;
        throw ex;
    }
    if ((rez = sqlite3_bind_int(stmt, 3, (int)p_rBurstMessage.cmdNo)) != SQLITE_OK)
    {
        connection.LogIfLastError(rez);
        LOG_ERROR_APP("[DevicesDAL]: 'CreateBurstMessageCounter->sqlite3_bind_int' for 'cmdNo' FAILED, err=" << rez);
        std::exception ex;
        throw ex;
    }
    if ((rez = m_oInsertBurstMessageCounter_compiled.Step_Exec()) != SQLITE_OK)
    {
        connection.LogIfLastError(rez);
        LOG_ERROR_APP("[DevicesDAL]: 'CreateBurstMessageCounter->Step_Exec' FAILED, err=" << rez);
        std::exception ex;
        throw ex;
    }
}

void SqliteDevicesDal::DeleteBurstMessageCounter(int p_nDeviceId, int p_nBurstMessage, int p_nCmdNo)
{
    LOG_DEBUG_APP("[DevicesDAL]: DeleteBurstMessageCounter for burst message " << ((int)p_nBurstMessage) << " with cmdNo=" << (int)p_nCmdNo << " for device id=" << p_nDeviceId);

    int rez = SQLITE_OK;
    if (!m_oDeleteBurstMessageCounter_compiled.GetStmt())
    {
        if ((rez = m_oDeleteBurstMessageCounter_compiled.Prepare(
                    "DELETE FROM BurstCounters "
                    " WHERE DeviceID = ?001 AND BurstMessage = ?002 AND CommandNumber = ?003;")) != SQLITE_OK)
        {
            connection.LogIfLastError(rez);
            LOG_ERROR_APP("[DevicesDAL]: 'DeleteBurstMessageCounter->Prepare' FAILED, err=" << rez);
            std::exception ex;
            throw ex;
        }
    }

    sqlite3_stmt *stmt = NULL;
    if (!(stmt = m_oDeleteBurstMessageCounter_compiled.GetStmt()))
    {
        LOG_ERROR_APP("[DevicesDAL]: 'DeleteBurstMessageCounter->GetStmt' FAILED");
        std::exception ex;
        throw ex;
    }
    if ((rez = sqlite3_bind_int(stmt, 1, p_nDeviceId)) != SQLITE_OK)
    {
        connection.LogIfLastError(rez);
        LOG_ERROR_APP("[DevicesDAL]: 'DeleteBurstMessageCounter->sqlite3_bind_int' for 'deviceID' FAILED, err=" << rez);
        std::exception ex;
        throw ex;
    }
    if ((rez = sqlite3_bind_int(stmt, 2, (int)p_nBurstMessage)) != SQLITE_OK)
    {
        connection.LogIfLastError(rez);
        LOG_ERROR_APP("[DevicesDAL]: 'DeleteBurstMessageCounter->sqlite3_bind_int' for 'burstMessage' FAILED, err=" << rez);
        std::exception ex;
        throw ex;
    }
    if ((rez = sqlite3_bind_int(stmt, 3, (int)p_nCmdNo)) != SQLITE_OK)
    {
        connection.LogIfLastError(rez);
        LOG_ERROR_APP("[DevicesDAL]: 'DeleteBurstMessageCounter->sqlite3_bind_int' for 'cmdNo' FAILED, err=" << rez);
        std::exception ex;
        throw ex;
    }
    if ((rez = m_oDeleteBurstMessageCounter_compiled.Step_Exec()) != SQLITE_OK)
    {
        connection.LogIfLastError(rez);
        LOG_ERROR_APP("[DevicesDAL]: 'DeleteBurstMessageCounter->Step_Exec' FAILED, err=" << rez);
        std::exception ex;
        throw ex;
    }
}

void SqliteDevicesDal::DeleteBurstMessageCounters(int p_nDeviceId)
{
    LOG_DEBUG_APP("[DevicesDAL]: DeleteBurstMessageCounters for device id=" << p_nDeviceId);

    int rez = SQLITE_OK;
    if (!m_oDeleteBurstMessageCounters_compiled.GetStmt())
    {
        if ((rez = m_oDeleteBurstMessageCounters_compiled.Prepare(
                    "DELETE FROM BurstCounters "
                    " WHERE DeviceID = ?001;")) != SQLITE_OK)
        {
            connection.LogIfLastError(rez);
            LOG_ERROR_APP("[DevicesDAL]: 'DeleteBurstMessageCounters->Prepare' FAILED, err=" << rez);
            std::exception ex;
            throw ex;
        }
    }

    sqlite3_stmt *stmt = NULL;
    if (!(stmt = m_oDeleteBurstMessageCounters_compiled.GetStmt()))
    {
        LOG_ERROR_APP("[DevicesDAL]: 'DeleteBurstMessageCounters->GetStmt' FAILED");
        std::exception ex;
        throw ex;
    }

    if ((rez = sqlite3_bind_int(stmt, 1, p_nDeviceId)) != SQLITE_OK)
    {
        connection.LogIfLastError(rez);
        LOG_ERROR_APP("[DevicesDAL]: 'DeleteBurstMessageCounters->sqlite3_bind_int' for 'deviceID' FAILED, err=" << rez);
        std::exception ex;
        throw ex;
    }
    if ((rez = m_oDeleteBurstMessageCounters_compiled.Step_Exec()) != SQLITE_OK)
    {
        connection.LogIfLastError(rez);
        LOG_ERROR_APP("[DevicesDAL]: 'DeleteBurstMessageCounters->Step_Exec' FAILED, err=" << rez);
        std::exception ex;
        throw ex;
    }
}

void SqliteDevicesDal::CreateBurstMessage(int p_nDeviceId, const BurstMessage& p_rBurstMessage)
{
	LOG_DEBUG_APP("[DevicesDAL]: CreateBurstMessage for burst message " << ((int)p_rBurstMessage.burstMessage) 
				<< " with cmdNo=" << (int)p_rBurstMessage.cmdNo
				<< " for device id=" << p_nDeviceId);

	hostapp::MAC emptyMac(0);
	std::string queryString("INSERT INTO BurstMessages(DeviceID, BurstMessage, CommandNumber, UpdatePeriod, MaxUpdatePeriod, SubDeviceMAC) ");
	queryString += (p_rBurstMessage.subDeviceMAC == emptyMac) ? "VALUES(?001, ?002, ?003, ?004, ?005, NULL)" : "VALUES(?001, ?002, ?003, ?004, ?005, ?006)";

	sqlitexx::Command sqlCommand(connection, queryString);
	sqlCommand.BindParam(1, p_nDeviceId);
	sqlCommand.BindParam(2, p_rBurstMessage.burstMessage);
	sqlCommand.BindParam(3, p_rBurstMessage.cmdNo);
	sqlCommand.BindParam(4, p_rBurstMessage.updatePeriod);
	sqlCommand.BindParam(5, p_rBurstMessage.maxUpdatePeriod);
	if (p_rBurstMessage.subDeviceMAC != emptyMac)
	    sqlCommand.BindParam(6, p_rBurstMessage.subDeviceMAC.ToString());

	sqlCommand.ExecuteNonQuery();
}

void SqliteDevicesDal::UpdateBurstMessage(int p_nDeviceId, const BurstMessage& p_rBurstMessage)
{
	LOG_DEBUG_APP("[DevicesDAL]: UpdateBurstMessage for burst message " << ((int)p_rBurstMessage.burstMessage) 
		<< " with cmdNo=" << (int)p_rBurstMessage.cmdNo
		<< " for device id=" << p_nDeviceId);

	hostapp::MAC emptyMac(0);
	std::string queryString("UPDATE BurstMessages ");
	queryString += (p_rBurstMessage.subDeviceMAC == emptyMac) ?
			"SET UpdatePeriod = ?001, MaxUpdatePeriod = ?002 " :
			"SET UpdatePeriod = ?001, MaxUpdatePeriod = ?002, SubDeviceMAC = ?006 ";
	queryString += " WHERE DeviceID = ?003 AND BurstMessage = ?004 AND CommandNumber = ?005";

	sqlitexx::Command sqlCommand(connection, queryString);

	sqlCommand.BindParam(1, p_rBurstMessage.updatePeriod);
	sqlCommand.BindParam(2, p_rBurstMessage.maxUpdatePeriod);
	sqlCommand.BindParam(3, p_nDeviceId);
	sqlCommand.BindParam(4, p_rBurstMessage.burstMessage);
	sqlCommand.BindParam(5, p_rBurstMessage.cmdNo);
	if (p_rBurstMessage.subDeviceMAC == emptyMac)
        sqlCommand.BindParam(6, p_rBurstMessage.subDeviceMAC.ToString());

	sqlCommand.ExecuteNonQuery();
}

void SqliteDevicesDal::DeleteBurstMessage(int p_nDeviceId, int p_nBurstMessage, int p_nCmdNo)
{
	LOG_DEBUG_APP("[DevicesDAL]: DeleteBurstMessage for burst message " << ((int)p_nBurstMessage) 
				<< " with cmdNo=" << (int)p_nCmdNo
				<< " for device id=" << p_nDeviceId);

	sqlitexx::Command sqlCommand(connection,
		"DELETE FROM BurstMessages "
		" WHERE DeviceID = ?001 AND BurstMessage = ?002 AND CommandNumber = ?003");
	sqlCommand.BindParam(1, p_nDeviceId);
	sqlCommand.BindParam(2, p_nBurstMessage);
	sqlCommand.BindParam(3, p_nCmdNo);
	sqlCommand.ExecuteNonQuery();
}

void SqliteDevicesDal::DeleteBurstMessages(int p_nDeviceId)
{
	LOG_DEBUG_APP("[DevicesDAL]: DeleteBurstMessage for device id=" << p_nDeviceId);

	sqlitexx::Command sqlCommand(connection, "DELETE FROM BurstMessages WHERE DeviceID = ?001");
	sqlCommand.BindParam(1, p_nDeviceId);
	sqlCommand.ExecuteNonQuery();
}

void SqliteDevicesDal::GetOrderedBursts(int deviceID, std::vector<BurstMessage> &bursts)
{
	LOG_DEBUG_APP("[DevicesDAL]: get ordered bursts for deviceID=" << deviceID);

	sqlitexx::CSqliteStmtHelper oSqlStmtHlp (connection);

	int rez = SQLITE_OK;
	if ((rez = oSqlStmtHlp.Prepare(
	                "SELECT BurstMessage, CommandNumber, UpdatePeriod, MaxUpdatePeriod, SubDeviceMAC "
					"  FROM BurstMessages WHERE DeviceID = ?001 "
					" ORDER BY CommandNumber, BurstMessage;")) != SQLITE_OK)
	{	
		connection.LogIfLastError(rez);
		LOG_ERROR_APP("[DevicesDAL]: 'GetOrderedBursts->Prepare' FAILED, err=" << rez);
		std::exception ex;
		throw ex;
	}

	oSqlStmtHlp.BindInt(1, deviceID);
	if (oSqlStmtHlp.Step_GetRow() != SQLITE_ROW)
	{
		return;
	}

	bursts.reserve(10);
	do 
	{
		BurstMessage burst;
		burst.burstMessage = oSqlStmtHlp.Column_GetInt(0);
		burst.cmdNo= oSqlStmtHlp.Column_GetInt(1);

		oSqlStmtHlp.Column_GetDouble(2, &burst.updatePeriod);
		oSqlStmtHlp.Column_GetDouble(3, &burst.maxUpdatePeriod);
        std::string szSubDeviceMAC;
        oSqlStmtHlp.Column_GetText(4, &szSubDeviceMAC);
        try {
            hart7::hostapp::MAC mac(szSubDeviceMAC);
            burst.subDeviceMAC = mac;
        } catch (...) {
            hart7::hostapp::MAC emptyMac(0);
            burst.subDeviceMAC = emptyMac;
        }
		
		bursts.push_back(burst);
	}
	while (oSqlStmtHlp.Step_GetRow() == SQLITE_ROW);
}

void SqliteDevicesDal::CreateTrigger(int p_nDeviceId, const Trigger& p_rTrigger)
{
	LOG_DEBUG_APP("[DevicesDAL]: CreateTrigger for burst message " << ((int)p_rTrigger.burstMessage) 
		<< " with cmdNo=" << (int)p_rTrigger.cmdNo
		<< " for device id=" << p_nDeviceId);

	sqlitexx::Command sqlCommand(connection,
		"INSERT INTO BurstTriggers(DeviceID, BurstMessage, CommandNumber, ModeSelection, Classification, UnitCode, Value)"
		" VALUES(?001, ?002, ?003, ?004, ?005, ?006, ?007)");
	sqlCommand.BindParam(1, p_nDeviceId);
	sqlCommand.BindParam(2, p_rTrigger.burstMessage);
	sqlCommand.BindParam(3, p_rTrigger.cmdNo);
	sqlCommand.BindParam(4, p_rTrigger.modeSelection);
	sqlCommand.BindParam(5, p_rTrigger.classification);
	sqlCommand.BindParam(6, p_rTrigger.unitCode);
	sqlCommand.BindParam(7, (double)p_rTrigger.triggerLevel);

	sqlCommand.ExecuteNonQuery();
}
void SqliteDevicesDal::UpdateTrigger(int p_nDeviceId, const Trigger& p_rTrigger)
{
	LOG_DEBUG_APP("[DevicesDAL]: UpdateTrigger for burst message " << ((int)p_rTrigger.burstMessage) 
		<< " with cmdNo=" << (int)p_rTrigger.cmdNo
		<< " for device id=" << p_nDeviceId);

	sqlitexx::Command sqlCommand(connection,
		"UPDATE BurstTriggers"
		"   SET ModeSelection = ?001, Classification = ?002, UnitCode = ?003, Value = ?004"
		" WHERE DeviceID = ?005 AND BurstMessage = ?006 AND CommandNumber = ?007");
	sqlCommand.BindParam(1, p_rTrigger.modeSelection);
	sqlCommand.BindParam(2, p_rTrigger.classification);
	sqlCommand.BindParam(3, p_rTrigger.unitCode);
	sqlCommand.BindParam(4, (double)p_rTrigger.triggerLevel);
	sqlCommand.BindParam(5, p_nDeviceId);
	sqlCommand.BindParam(6, p_rTrigger.burstMessage);
	sqlCommand.BindParam(7, p_rTrigger.cmdNo);
	sqlCommand.ExecuteNonQuery();
}

void SqliteDevicesDal::DeleteTrigger(int p_nDeviceId, int p_nBurstMessage, int p_nCmdNo)
{
	LOG_DEBUG_APP("[DevicesDAL]: DeleteTrigger for device id=" << p_nDeviceId);

	sqlitexx::Command sqlCommand(connection, "DELETE FROM BurstTriggers WHERE DeviceID = ?001");
	sqlCommand.BindParam(1, p_nDeviceId);
	sqlCommand.ExecuteNonQuery();
}

void SqliteDevicesDal::GetOrderedTriggers(int deviceID, std::vector<Trigger> &triggers)
{
	LOG_DEBUG_APP("[DevicesDAL]: get ordered triggers for deviceID=" << deviceID);

	sqlitexx::CSqliteStmtHelper oSqlStmtHlp (connection);

	int rez = SQLITE_OK;
	if ((rez = oSqlStmtHlp.Prepare(
	            "SELECT BurstMessage, CommandNumber, ModeSelection, Classification, UnitCode, Value"
		        "  FROM BurstTriggers WHERE DeviceID = ?001 "
		        " ORDER BY CommandNumber, BurstMessage;")) != SQLITE_OK)
	{
		connection.LogIfLastError(rez);
		LOG_ERROR_APP("[DevicesDAL]: 'GetOrderedTriggers->Prepare' FAILED, err=" << rez);
		std::exception ex;
		throw ex;
	}

	oSqlStmtHlp.BindInt(1, deviceID);
	if (oSqlStmtHlp.Step_GetRow() != SQLITE_ROW)
	{
		return;
	}

	triggers.reserve(10);
	do 
	{
		Trigger trigger;
		trigger.burstMessage = oSqlStmtHlp.Column_GetInt(0);
		trigger.cmdNo= oSqlStmtHlp.Column_GetInt(1);
		trigger.modeSelection = oSqlStmtHlp.Column_GetInt(2);
		trigger.classification= oSqlStmtHlp.Column_GetInt(3);
		trigger.unitCode = oSqlStmtHlp.Column_GetInt(4);
		double tmp;
		oSqlStmtHlp.Column_GetDouble(5, &tmp);
		trigger.triggerLevel = tmp;
		triggers.push_back(trigger);
	}
	while (oSqlStmtHlp.Step_GetRow() == SQLITE_ROW);
}

void SqliteDevicesDal::UpdateSetPublishersLog(int deviceId, int state, std::string error, std::string message)
{
    LOG_DEBUG_APP("[DevicesDAL]: update SetPublisherLog for deviceId=" << deviceId);

    {
        int rez = SQLITE_OK;
        if (!m_oUpdateSetPublishersLog_compiled.GetStmt())
        {
            if ((rez = m_oUpdateSetPublishersLog_compiled.Prepare(
                            "INSERT OR REPLACE INTO DeviceSetPublishersLog (DeviceID, State, Error, Message) "
                            "VALUES (?001, ?002, ?003, ?004);")) != SQLITE_OK)
            {
                connection.LogIfLastError(rez);
                LOG_ERROR_APP("[DevicesDAL]: 'UpdateDeviceSetPublishersLog->Prepare' FAILED, err=" << rez);
                std::exception ex;
                throw ex;
            }
        }
        // get statement
        sqlite3_stmt *stmt = NULL;
        if (!(stmt = m_oUpdateSetPublishersLog_compiled.GetStmt()))
        {
            LOG_ERROR_APP("[DevicesDAL]: 'UpdateDeviceSetPublishersLog->GetStmt' FAILED");
            std::exception ex;
            throw ex;
        }
        if ((rez = sqlite3_bind_int(stmt, 1, deviceId)) != SQLITE_OK)
        {
            connection.LogIfLastError(rez);
            LOG_ERROR_APP("[DevicesDAL]: 'UpdateDeviceSetPublishersLog->sqlite3_bind_int' for 'DeviceID' FAILED, err=" << rez);
            std::exception ex;
            throw ex;
        }
        if ((rez = sqlite3_bind_int(stmt, 2, state)) != SQLITE_OK)
        {
            connection.LogIfLastError(rez);
            LOG_ERROR_APP("[DevicesDAL]: 'UpdateDeviceSetPublishersLog->sqlite3_bind_int' for 'State' FAILED, err=" << rez);
            std::exception ex;
            throw ex;
        }
        if ((rez = sqlite3_bind_text(stmt, 3, error.c_str(), -1, SQLITE_STATIC)) != SQLITE_OK)
        {
            connection.LogIfLastError(rez);
            LOG_ERROR_APP("[DevicesDAL]: 'UpdateDeviceSetPublishersLog->sqlite3_bind_text' for 'Error' FAILED, err=" << rez);
            std::exception ex;
            throw ex;
        }
        if ((rez = sqlite3_bind_text(stmt, 4, message.c_str(), -1, SQLITE_STATIC)) != SQLITE_OK)
        {
            connection.LogIfLastError(rez);
            LOG_ERROR_APP("[DevicesDAL]: 'UpdateDeviceSetPublishersLog->sqlite3_bind_text' for 'Message' FAILED, err=" << rez);
            std::exception ex;
            throw ex;
        }
        if ((rez = m_oUpdateSetPublishersLog_compiled.Step_Exec()) != SQLITE_OK)
        {
            connection.LogIfLastError(rez);
            LOG_ERROR_APP("[DevicesDAL]: 'UpdateDeviceSetPublishersLog->Step_Exec' FAILED, err=" << rez);
            std::exception ex;
            throw ex;
        }
    }
}

void SqliteDevicesDal::DeleteSetPublishersLog(int deviceId)
{
    LOG_DEBUG_APP("[DevicesDAL]: delete SetPublisherLog for deviceId=" << deviceId );

    {
        int rez = SQLITE_OK;
        if (!m_oDeleteSetPublishersLog_compiled.GetStmt())
        {
            if ((rez = m_oDeleteSetPublishersLog_compiled.Prepare(
                        "DELETE FROM DeviceSetPublishersLog WHERE DeviceID = ?001;")) != SQLITE_OK)
            {
                connection.LogIfLastError(rez);
                LOG_ERROR_APP("[DevicesDAL]: 'DeleteSetPublishersLog->Prepare' FAILED, err=" << rez);
                std::exception ex;
                throw ex;
            }
        }
        // get statement
        sqlite3_stmt *stmt = NULL;
        if (!(stmt = m_oDeleteSetPublishersLog_compiled.GetStmt()))
        {
            LOG_ERROR_APP("[DevicesDAL]: 'DeleteSetPublishersLog->GetStmt' FAILED");
            std::exception ex;
            throw ex;
        }
        if ((rez = sqlite3_bind_int(stmt, 1, deviceId)) != SQLITE_OK)
        {
            connection.LogIfLastError(rez);
            LOG_ERROR_APP("[DevicesDAL]: 'DeleteSetPublishersLog->sqlite3_bind_int' for 'deviceId' FAILED, err=" << rez);
            std::exception ex;
            throw ex;
        }
        if ((rez = m_oDeleteSetPublishersLog_compiled.Step_Exec()) != SQLITE_OK)
        {
            connection.LogIfLastError(rez);
            LOG_ERROR_APP("[DevicesDAL]: 'DeleteSetPublishersLog->Step_Exec' FAILED, err=" << rez);
            std::exception ex;
            throw ex;
        }
    }
}

}//namespace hostapp
}//namespace hart7
