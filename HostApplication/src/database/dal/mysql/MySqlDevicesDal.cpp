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

#include <stdio.h>
#include "MySqlDevicesDal.h"

namespace hart7 {
namespace hostapp {

MySqlDevicesDal::MySqlDevicesDal(MySQLConnection& connection_) :
	connection(connection_)
{
}

MySqlDevicesDal::~MySqlDevicesDal()
{
}

//init
void MySqlDevicesDal::VerifyTables()
{
	LOG_INFO_APP("[DevicesDAL]: Verify devices tables structure...");

	//devices
	{
		const char*
			query =
				"SELECT DeviceID, DeviceRole, DeviceCode, SoftwareRevision, Address64, DeviceTag,"
				" Nickname, DeviceStatus, DeviceLevel, RejoinCount, LastRead, PowerSupplyStatus, PublishStatus"
				" FROM Devices WHERE 0";
		try
		{
			MySQLCommand(connection, query).ExecuteQuery();
		}
		catch(std::exception& ex)
		{
			LOG_ERROR_APP("Verify failed for='" << query << "' error=" << ex.what());
			throw;
		}
	}
	{
		std::string query = "SELECT HistoryID, DeviceID, Timestamp, DeviceStatus FROM DeviceHistory WHERE 0";
		try
		{
			MySQLCommand(connection, query).ExecuteQuery();
		}
		catch(std::exception& ex)
		{
			LOG_ERROR_APP("Verify failed for='" << query << "' error=" << ex.what());
			throw;
		}
	}
	{
		std::string query = ""
			"SELECT DeviceID, IP, Port"
			" FROM DeviceConnections WHERE 0";
		try
		{
			MySQLCommand(connection, query).ExecuteQuery();
		}
		catch(std::exception& ex)
		{
			LOG_ERROR_APP("Verify failed for='" << query << "' error=" << ex.what());
			throw;
		}
	}

	//readings
	{
		std::string
		    query = "SELECT ChannelID, DeviceID, BurstMessage, DeviceVariableSlot, Name, CmdNo, DeviceVariable, Classification, UnitCode"
			        " FROM Channels WHERE 0";
		try
		{
			MySQLCommand(connection, query).ExecuteQuery();
		}
		catch(std::exception& ex)
		{
			LOG_ERROR_APP("Verify failed for='" << query << "' error=" << ex.what());
			throw;
		}
	}
	{
		std::string
		    query = " SELECT ChannelID, DeviceID, BurstMessage, DeviceVariableSlot, Name, CmdNo, DeviceVariable, Classification, UnitCode, Timestamp"
			        " FROM ChannelsHistory WHERE 0";
		try
		{
			MySQLCommand(connection, query).ExecuteQuery();
		}
		catch(std::exception& ex)
		{
			LOG_ERROR_APP("Verify failed for='" << query << "' error=" << ex.what());
			throw;
		}
	}
	{
		std::string query = "SELECT ChannelID, DeviceID, BurstMessage, DeviceVariableSlot, Name, CmdNo, DeviceVariable, Classification, UnitCode"
			        		" FROM Channels WHERE 0";
		try
		{
			MySQLCommand(connection, query).ExecuteQuery();
		}
		catch(std::exception& ex)
		{
			LOG_ERROR_APP("Verify failed for='" << query << "' error=" << ex.what());
			throw;
		}
	}


	//commands
	{
		std::string query = "SELECT CommandID, DeviceID, CommandCode, CommandStatus, "
			"TimePosted, TimeResponsed, ErrorCode, "
			"ErrorReason, Response, Generated, ParametersDescription"
			" FROM Commands WHERE 0";
		try
		{
			MySQLCommand(connection, query).ExecuteQuery();
		}
		catch(std::exception& ex)
		{
			LOG_ERROR_APP("Verify failed for='" << query << "' error=" << ex.what());
			throw;
		}
	}
	{
		std::string query = "SELECT CommandID, ParameterCode, ParameterValue"
			" FROM CommandParameters WHERE 0";
		try
		{
			MySQLCommand(connection, query).ExecuteQuery();
		}
		catch(std::exception& ex)
		{
			LOG_ERROR_APP("Verify failed for='" << query << "' error=" << ex.what());
			throw;
		}
	}
}

//devices
void MySqlDevicesDal::ResetDevices(Device::DeviceStatus newStatus)
{
	LOG_INFO_APP("[DevicesDAL]: Reset all devices from db");

	MySQLCommand sqlCommand(connection, ""
		"UPDATE Devices SET DeviceStatus = ?001");
	sqlCommand.BindParam(1, (int)newStatus);

	sqlCommand.ExecuteNonQuery();
}
void MySqlDevicesDal::GetDevices(DeviceList& list)
{
	LOG_DEBUG_APP("[DevicesDAL]: Get all devices from database");

	MySQLCommand sqlCommand(connection, ""
		"SELECT DeviceID, DeviceRole, Nickname, Address64, DeviceTag, "
		" DeviceStatus, DeviceLevel, DeviceCode, "
		" SoftwareRevision, PowerSupplyStatus"
		" FROM Devices;");

	MySQLResultSet::Ptr rsDevices = sqlCommand.ExecuteQuery();
	list.reserve(list.size() + rsDevices->RowsCount());

	for (MySQLResultSet::Iterator it = rsDevices->Begin(); it != rsDevices->End(); it++)
	{
		list.push_back(Device());
		Device& device = *list.rbegin();
		device.id = it->Value<int>(0);
		device.Type((Device::DeviceType)it->Value<int>(1));
		device.Nickname(NickName(it->Value<int>(2)));
		device.Mac(MAC(it->Value<std::string>(3)));
		device.SetTAG(it->Value<std::string>(4));
		device.Status((Device::DeviceStatus)it->Value<int>(5));
		device.Level(it->Value<int>(6));
		device.SetDeviceCode(it->Value<int>(7));
		device.SetSoftwareRevision(it->Value<int>(8));
		device.PowerStatus(it->Value<int>(9));
	}

}
void MySqlDevicesDal::DeleteDevice(int id)
{
	LOG_DEBUG_APP("[DevicesDAL]: delete device id=" << id);

	MySQLCommand sqlCommand(connection, ""
			"DELETE FROM Devices WHERE DeviceID = ?001");
		sqlCommand.BindParam(1, id);
		sqlCommand.ExecuteNonQuery();
}
void MySqlDevicesDal::AddDevice(Device& device)
{
	LOG_DEBUG_APP("[DevicesDAL]: Add device with MAC:" << device.Mac());

	//create device
	{
		MySQLCommand
			sqlCommand(
			connection,
			"INSERT INTO Devices(DeviceRole, Nickname, Address64, DeviceTag, "
			"DeviceStatus, DeviceLevel, DeviceCode, SoftwareRevision, PowerSupplyStatus, RejoinCount)"
			" VALUES(?001, ?002, ?003, ?004, ?005, ?006, ?007, ?008, ?009, ?010);");
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

	{ // tries to insert a generic device code. If device code already exists then nothing will happen.
		MySQLCommand sqlCommand(connection, "INSERT OR IGNORE INTO DevicesCodes(DeviceCode, Model, Company) VALUES(?001, 'N/A', 'N/A');");
		sqlCommand.BindParam(1, device.GetDeviceCode());
		sqlCommand.ExecuteNonQuery();
	}

	{ //update history
		MySQLCommand sqlCommand(connection, ""
			"INSERT INTO DeviceHistory(DeviceID, Timestamp, DeviceStatus)"
			" VALUES (?001, ?002, ?003)");

		sqlCommand.BindParam(1, device.id);
		sqlCommand.BindParam(2, CMicroSec().GetElapsedTimeStr());
		sqlCommand.BindParam(3, (int)device.Status());
		sqlCommand.ExecuteNonQuery();
	}

}
void MySqlDevicesDal::UpdateDevice(Device& device)
{
	LOG_DEBUG_APP("[DevicesDAL]: Update device with MAC:" << device.Mac());

	{
		//create history if status changed
		//this should be called before status updated
		MySQLCommand sqlCommand(connection, ""
			"INSERT INTO DeviceHistory(DeviceID, Timestamp, DeviceStatus)"
			" SELECT ?001, ?002, ?003 FROM Devices "
			"		WHERE DeviceID = ?001 AND DeviceStatus != ?003");
		sqlCommand.BindParam(1, device.id);
		sqlCommand.BindParam(2, CMicroSec().GetElapsedTimeStr());
		sqlCommand.BindParam(3, (int)device.Status());
		sqlCommand.ExecuteNonQuery();
	}

	//update device
	{
		MySQLCommand sqlCommand(connection, ""
			"UPDATE Devices SET"
			" DeviceRole = ?001, Nickname = ?002, Address64 = ?003,"
			" DeviceTag = ?004, DeviceStatus = ?005, "
			" DeviceCode = ?006, SoftwareRevision = ?007, PowerSupplyStatus = ?008,"
			" RejoinCount = ?009"
			" WHERE DeviceID = ?010");

		sqlCommand.BindParam(1, (int)device.Type());
		sqlCommand.BindParam(2, (int)device.Nickname().Address());
		sqlCommand.BindParam(3, device.Mac().ToString());
		sqlCommand.BindParam(4, device.GetTAG());
		sqlCommand.BindParam(5, (int)device.Status());
		sqlCommand.BindParam(6, device.GetDeviceCode());
		sqlCommand.BindParam(7, device.GetSoftwareRevision());
		sqlCommand.BindParam(8, device.PowerStatus());
		sqlCommand.BindParam(9, device.GetRejoinCount());
		sqlCommand.BindParam(10, device.id);
		sqlCommand.ExecuteNonQuery();
	}
}

//topology
void MySqlDevicesDal::CreateDeviceGraph(int fromDevice, int toDevice, int graphID, int neighbIndex)
{
	LOG_DEBUG_APP("[DevicesDAL]: create graph to=" << toDevice << " from device id=" << fromDevice 
		<< " with graphID=" << graphID << " with index=" << neighbIndex);

	MySQLCommand sqlCommand(connection, ""
		"INSERT INTO GraphNeighbors(DeviceID, PeerID, GraphID, NeighborIndex)"
		" VALUES (?001, ?002, ?003, ?004)");

	sqlCommand.BindParam(1, fromDevice);
	sqlCommand.BindParam(2, toDevice);
	sqlCommand.BindParam(3, graphID);
	sqlCommand.BindParam(4, neighbIndex);
	sqlCommand.ExecuteNonQuery();
}
void MySqlDevicesDal::CleanDevicesGraphs()
{
	LOG_DEBUG_APP("[DevicesDAL]: clean graphs");

	MySQLCommand sqlCommand(connection, ""
		"DELETE FROM GraphNeighbors");
	sqlCommand.ExecuteNonQuery();
}
void MySqlDevicesDal::CleanDeviceGraphs(int deviceID)
{
	LOG_DEBUG_APP("[DevicesDAL]: clean graphs for device id=" << deviceID);

	MySQLCommand sqlCommand(connection, ""
		"DELETE FROM GraphNeighbors "
		" WHERE DeviceID = ?001;");
	sqlCommand.BindParam(1, deviceID);
	sqlCommand.ExecuteNonQuery();
}
void MySqlDevicesDal::DeleteGraph(int deviceId, int neighborID, int graphID)
{
	LOG_DEBUG_APP("[DevicesDAL]: clean graph for device id=" << deviceId<<" with neighbID=" << neighborID << " and graph id="<<graphID);

	MySQLCommand sqlCommand(connection, ""
		"DELETE FROM GraphNeighbors "
		" WHERE DeviceID = ?001 AND GraphID = ?002 AND PeerID = ?003;");
	sqlCommand.BindParam(1, deviceId);
	sqlCommand.BindParam(2, graphID);
	sqlCommand.BindParam(3, neighborID);
	sqlCommand.ExecuteNonQuery();
}

//gateway
void MySqlDevicesDal::AddDeviceConnections(int deviceID, const std::string& host, int port)
{
	LOG_DEBUG_APP("[DevicesDAL]: Add device connection for deviceID=" << deviceID);

	MySQLCommand sqlCommand(connection, ""
			"INSERT INTO DeviceConnections(DeviceID, IP, Port)"
			" VALUES(?001, ?002, ?003) ON DUPLICATE KEY UPDATE IP=?002, Port=?003;");

	sqlCommand.BindParam(1, deviceID);
	sqlCommand.BindParam(2, host);
	sqlCommand.BindParam(3, port);
	sqlCommand.ExecuteNonQuery();
}
void MySqlDevicesDal::RemoveDeviceConnections(int deviceID)
{
	LOG_DEBUG_APP("[DevicesDAL]: remove device connection for deviceID=" << deviceID);

	MySQLCommand sqlCommand(connection, ""
		"DELETE FROM DeviceConnections "
		" WHERE DeviceID = ?001;");
	sqlCommand.BindParam(1, deviceID);
	sqlCommand.ExecuteNonQuery();

}

//wh_general_cmd
void MySqlDevicesDal::AddCommand(int deviceID, int cmdNo, std::string dataBuffer, int cmdID, int responseCode)
{

	LOG_DEBUG_APP("[DevicesDAL]: Add response command to WHResponseDataBuffer table" );

	MySQLCommand sqlCommand(connection, "INSERT INTO WHResponseDataBuffer(DeviceID, CommandID, Timestamp, Miliseconds, CmdNo, ResponseCode, DataBuffer)"
		" VALUES(?001, ?002, ?003, ?004, ?005, ?006, ?007); ");
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
void MySqlDevicesDal::CreateChannel(int deviceID, const PublishChannel &channel)
{

	LOG_DEBUG_APP("[DevicesDAL]: create channels for deviceID=" << deviceID);

	MySQLCommand sqlCommand(connection, ""
		" INSERT INTO Channels(DeviceID, BurstMessage, DeviceVariableSlot, Name, CmdNo, DeviceVariable, Classification, UnitCode)"
		" VALUES(?001, ?002, ?003, ?004, ?005, ?006, ?007, ?008);");

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
void MySqlDevicesDal::MoveChannelToHistory(int channelID)
{
	LOG_DEBUG_APP("[DevicesDAL]: move to history channelID=" << channelID);

	{
		MySQLCommand sqlCommand(connection, ""
			" INSERT INTO ChannelsHistory "
			" SELECT ChannelID, DeviceID, BurstMessage, DeviceVariableSlot, "
			" Name, CmdNo, DeviceVariable, Classification, UnitCode, Now()"
			" FROM Channels WHERE ChannelID = ?001");
	
		sqlCommand.BindParam(1, channelID);
		sqlCommand.ExecuteNonQuery();
	}
	{
		MySQLCommand sqlCommand(connection, ""
			"DELETE FROM Channels WHERE ChannelID = ?001");
	
		sqlCommand.BindParam(1, channelID);
		sqlCommand.ExecuteNonQuery();
	}
}
void MySqlDevicesDal::UpdateChannel(int channelID, int variableCode, int classification, int unitCode, const std::string &name)
{
	LOG_DEBUG_APP("[DevicesDAL]: update channelID=" << channelID);

	MySQLCommand sqlCommand(connection, ""
		"UPDATE Channels SET DeviceVariable = ?001, Classification = ?002, "
		"UnitCode = ?003, Name = ?004 WHERE ChannelID = ?005");

	sqlCommand.BindParam(1, variableCode);
	sqlCommand.BindParam(2, classification);
	sqlCommand.BindParam(3, unitCode);
	sqlCommand.BindParam(4, name);
	sqlCommand.BindParam(5, channelID);
	sqlCommand.ExecuteNonQuery();
}
void MySqlDevicesDal::DeleteReading(int channeNo)
{
	LOG_DEBUG_APP("[DevicesDAL]: delete reading with channelID=" << channeNo);

	char sqlCmd[500];
	sprintf(sqlCmd, "DELETE from Readings WHERE ChannelID=%d;", channeNo);

	MySQLCommand sqlCommand(connection, sqlCmd);
	sqlCommand.ExecuteNonQuery();
}
void MySqlDevicesDal::AddEmptyReading(int DeviceID, int channelID)
{
	LOG_DEBUG_APP("[DevicesDAL]: add in readings channelID=" << channelID << " for deviceID=" << DeviceID);


	char sqlCmd[1000];

	sprintf(sqlCmd, "INSERT INTO Readings(ChannelID, ReadingTime, Miliseconds, Value, Status, CommandID) \
					 SELECT %d, '1970-01-01 00:00:00',0, 0, 0, 0;", channelID);

	MySQLCommand sqlCommand(connection, sqlCmd);
	sqlCommand.ExecuteNonQuery();
}
static const char* gettime(const struct timeval &tv)
{
  static char szTime[50];
  static struct tm * timeinfo;

  timeinfo = gmtime(&tv.tv_sec);
  sprintf(szTime, "%04d-%02d-%02d %02d:%02d:%02d", timeinfo->tm_year+1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, 
											timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec); 
  return szTime;
}
void MySqlDevicesDal::UpdateReading(const DeviceReading& reading)
{
	LOG_DEBUG_APP("[DevicesDAL]: update in readings for channelID=" << reading.m_channelNo);

	char sqlCmd[4 * 1024];
	if (reading.m_hasStatus == true)
	{
		sprintf(sqlCmd, "UPDATE Readings SET ReadingTime='%s', Value='%04f', Miliseconds=%d, Status=%d, CommandID=%d"
						" WHERE ChannelID=%d;"
						, gettime(reading.m_tv), reading.m_value, (int)(reading.m_tv.tv_usec/1000)
						, (int)reading.m_valueStatus,(int)reading.m_nCommandID, (int)reading.m_channelNo);
	}
	else
	{
		sprintf(sqlCmd, "UPDATE Readings SET ReadingTime='%s', Value='%04f', Miliseconds=%d, CommandID=%d"
						"WHERE ChannelID=%d;"
						, gettime(reading.m_tv), reading.m_value, (int)(reading.m_tv.tv_usec/1000)
						, (int)reading.m_nCommandID, (int)reading.m_channelNo);
	}

	MySQLCommand sqlCommand(connection, sqlCmd);
	sqlCommand.ExecuteNonQuery();
}
void MySqlDevicesDal::UpdatePublishFlag(int devID, Device::PublishStatus status)
{
	char sqlCmd[500];
	sprintf(sqlCmd, "UPDATE Devices SET PublishStatus = %d WHERE DeviceID = %d", status, devID);
	MySQLCommand sqlCommand(connection, sqlCmd);
	sqlCommand.ExecuteNonQuery();
}

void MySqlDevicesDal::UpdateReadingTimeforDevice(int deviceID, const struct timeval &tv, Device::PublishStatus status)
{
	LOG_DEBUG_APP("[DevicesDAL]: update readingtime for deviceID=" << deviceID);

	char sqlCmd[500];
	sprintf(sqlCmd, "UPDATE Devices SET LastRead = '%s', PublishStatus = %d WHERE DeviceID = %d", gettime(tv), (int)status, deviceID);
	MySQLCommand sqlCommand(connection, sqlCmd);
	sqlCommand.ExecuteNonQuery();
}

void MySqlDevicesDal::GetOrderedChannels(int deviceID, std::vector<PublishChannel> &channels)
{
	LOG_DEBUG_APP("[DevicesDAL]: get ordered channels for deviceID=" << deviceID);

	MySQLCommand
	    sqlCommand(
	        connection,
			"SELECT ChannelID, BurstMessage, DeviceVariableSlot, Name, CmdNo, DeviceVariable, Classification, UnitCode"
			"FROM Channels WHERE DeviceID = ?001 "
			"ORDER BY CmdNo, BurstMessage, DeviceVariableSlot;");

	sqlCommand.BindParam(1, deviceID);
	
	MySQLResultSet::Ptr res = sqlCommand.ExecuteQuery();
	channels.resize(res->RowsCount());
	int i = 0;
	for (MySQLResultSet::Iterator itDeviceChannel = res->Begin(); 
					itDeviceChannel != res->End(); itDeviceChannel++)
	{
		channels[i].channelID = itDeviceChannel->Value<int>(0);
		channels[i].burstMessage = itDeviceChannel->Value<int>(1);
		channels[i].deviceVariableSlot = itDeviceChannel->Value<int>(2);
		channels[i].name = itDeviceChannel->Value<std::string>(3);
		channels[i].cmdNo = itDeviceChannel->Value<int>(4);
		channels[i].deviceVariable = itDeviceChannel->Value<int>(5);
		channels[i].classification = itDeviceChannel->Value<int>(6);
		channels[i].unitCode = itDeviceChannel->Value<int>(7);
		++i;
	}
}

//routes
void MySqlDevicesDal::CleanDeviceRoutes()
{
	LOG_DEBUG_APP("[DevicesDAL]: clean routes report.");

	MySQLCommand sqlCommand(connection, ""
		"DELETE FROM Routes");
	sqlCommand.ExecuteNonQuery();	
}
void MySqlDevicesDal::CleanDeviceRoutes(int deviceID)
{
	LOG_DEBUG_APP("[DevicesDAL]: clean all routes for deviceID=" << deviceID);

	MySQLCommand sqlCommand(connection, ""
		"DELETE FROM Routes"
		" WHERE DeviceID = ?001;");
	sqlCommand.BindParam(1, deviceID);
	sqlCommand.ExecuteNonQuery();	
}
void MySqlDevicesDal::CleanDeviceRoute(int deviceID, int routeID)
{
	LOG_DEBUG_APP("[DevicesDAL]: clean routes for deviceID=" << deviceID<<" and routeID="<<routeID);

	MySQLCommand sqlCommand(connection, ""
		"DELETE FROM Routes"
		" WHERE DeviceID = ?001 and WHERE RouteID = ?002;");
	sqlCommand.BindParam(1, deviceID);
	sqlCommand.BindParam(2, routeID);
	sqlCommand.ExecuteNonQuery();
}
void MySqlDevicesDal::InsertRoute(const Route& p_rRoute)
{
	LOG_DEBUG_APP("[DevicesDAL]: Add route for device id=" << p_rRoute.m_nDeviceId);

	MySQLCommand sqlCommand(connection,
		"INSERT INTO Routes(RouteID, DeviceID, PeerID, GraphID, SourceRoute, Timestamp)"
		" VALUES(?001, ?002, ?003, ?004, ?005, ?006); ");

	sqlCommand.BindParam(1, p_rRoute.m_nRouteId);
	sqlCommand.BindParam(2, p_rRoute.m_nDeviceId);
	sqlCommand.BindParam(3, p_rRoute.m_nPeerId);
	sqlCommand.BindParam(4, p_rRoute.m_nGraphId);
	sqlCommand.BindParam(5, p_rRoute.m_nSourceRoute);
	sqlCommand.BindParam(6, p_rRoute.time);

	sqlCommand.ExecuteNonQuery();
}

//services
void MySqlDevicesDal::CleanDeviceServices()
{
	LOG_DEBUG_APP("[DevicesDAL]: clean services report.");

	MySQLCommand sqlCommand(connection, ""
		"DELETE FROM Services ");
	sqlCommand.ExecuteNonQuery();
}
void MySqlDevicesDal::CleanDeviceServices(int deviceID)
{
	LOG_DEBUG_APP("[DevicesDAL]: clean all services for deviceID=" << deviceID);

	MySQLCommand sqlCommand(connection, ""
		"DELETE FROM Services "
		" WHERE DeviceID = ?001;");
	sqlCommand.BindParam(1, deviceID);
	sqlCommand.ExecuteNonQuery();
}
void MySqlDevicesDal::CleanDeviceService(int deviceID, int serviceID)
{
	LOG_DEBUG_APP("[DevicesDAL]: clean service for deviceID=" << deviceID<<" and serviceID="<<serviceID);

	MySQLCommand sqlCommand(connection, ""
		"DELETE FROM Services "
		" WHERE DeviceID = ?001 and ServiceID = ?002;");
	sqlCommand.BindParam(1, deviceID);
	sqlCommand.BindParam(2, serviceID);
	sqlCommand.ExecuteNonQuery();
}
void MySqlDevicesDal::InsertService(const Service& p_rService)
{
	LOG_DEBUG_APP("[DevicesDAL]: Add service for device id=" << p_rService.m_nDeviceId);

	MySQLCommand sqlCommand(connection,
		"INSERT INTO Services(ServiceID, DeviceID, PeerID, ApplicationDomain, SourceFlag, SinkFlag, IntermittentFlag, Period, RouteID, Timestamp)"
		" VALUES(?001, ?002, ?003, ?004, ?005, ?006, ?007, ?008, ?009, ?010); ");

	sqlCommand.BindParam(1, p_rService.m_nServiceId);
	sqlCommand.BindParam(2, p_rService.m_nDeviceId);
	sqlCommand.BindParam(3, p_rService.m_nPeerId);
	sqlCommand.BindParam(4, p_rService.m_nApplicationDomain);
	sqlCommand.BindParam(5, p_rService.m_nSourceFlag);
	sqlCommand.BindParam(6, p_rService.m_nSinkFlag);
	sqlCommand.BindParam(7, p_rService.m_nIntermittentFlag);
	sqlCommand.BindParam(8, p_rService.m_nPeriod);
	sqlCommand.BindParam(9, p_rService.m_nRouteId);
	sqlCommand.BindParam(10, p_rService.time);

	sqlCommand.ExecuteNonQuery();
}

//SourceRoutes
void MySqlDevicesDal::CleanDeviceSourceRoutes()
{
	LOG_DEBUG_APP("[DevicesDAL]: clean source routes report.");

	MySQLCommand sqlCommand(connection, ""
		"DELETE FROM SourceRoutes ");
	sqlCommand.ExecuteNonQuery();
}
void MySqlDevicesDal::CleanDeviceSourceRoutes(int p_nDeviceId)
{
	LOG_DEBUG_APP("[DevicesDAL]: clean source routes for device id=" << p_nDeviceId);

	MySQLCommand sqlCommand(connection, ""
		"DELETE FROM SourceRoutes "
		" WHERE deviceID = ?001;");
	sqlCommand.BindParam(1, p_nDeviceId);

	sqlCommand.ExecuteNonQuery();
}
void MySqlDevicesDal::CleanDeviceSourceRoutes(int p_nDeviceId, int p_nRouteID)
{
	LOG_DEBUG_APP("[DevicesDAL]: clean source routes for device id=" << p_nDeviceId<<" and routeID="<<p_nRouteID);

	MySQLCommand sqlCommand(connection, ""
		"DELETE FROM SourceRoutes "
		" WHERE deviceID = ?001 and routeID= ?002;");
	sqlCommand.BindParam(1, p_nDeviceId);
	sqlCommand.BindParam(2, p_nRouteID);

	sqlCommand.ExecuteNonQuery();
}
void MySqlDevicesDal::InsertSourceRoute(const SourceRoute& p_rSourceRoute)
{
	LOG_DEBUG_APP("[DevicesDAL]: Add source route for device id=" << p_rSourceRoute.m_nDeviceId);

	MySQLCommand sqlCommand(connection,
		"INSERT INTO SourceRoutes(DeviceID, RouteID, Devices, Timestamp)"
		" VALUES(?001, ?002, ?003, ?004); ");

	sqlCommand.BindParam(1, p_rSourceRoute.m_nDeviceId);
	sqlCommand.BindParam(2, p_rSourceRoute.m_nRouteId);
	sqlCommand.BindParam(3, p_rSourceRoute.m_strDevices.c_str());
	sqlCommand.BindParam(4, p_rSourceRoute.time);

	sqlCommand.ExecuteNonQuery();
}

//superframes
void MySqlDevicesDal::CleanDeviceSuperframes()
{
	LOG_DEBUG_APP("[DevicesDAL]: clean Superframes report");

	MySQLCommand sqlCommand(connection, ""
		"DELETE FROM Superframes ");
	sqlCommand.ExecuteNonQuery();
}
void MySqlDevicesDal::CleanDeviceSuperframes(int deviceID)
{
	LOG_DEBUG_APP("[DevicesDAL]: clean all Superframes for deviceID=" << deviceID);

	MySQLCommand sqlCommand(connection, ""
		"DELETE FROM Superframes "
		" WHERE DeviceID = ?001;");
	sqlCommand.BindParam(1, deviceID);
	sqlCommand.ExecuteNonQuery();
}
void MySqlDevicesDal::InsertSuperframe(const Superframe& p_rSuperframe)
{
	LOG_DEBUG_APP("[DevicesDAL]: Add Superframe for device id=" << p_rSuperframe.m_nDeviceId);

	MySQLCommand sqlCommand(connection,
		"INSERT INTO Superframes(SuperframeID, DeviceID, NumberOfTimeSlots, Active, HandheldSuperframe, Timestamp)"
		" VALUES(?001, ?002, ?003, ?004, ?005, ?006); ");

	sqlCommand.BindParam(1, p_rSuperframe.m_nSuperframeId);
	sqlCommand.BindParam(2, p_rSuperframe.m_nDeviceId);
	sqlCommand.BindParam(3, p_rSuperframe.m_nNumberOfTimeSlots);
	sqlCommand.BindParam(4, p_rSuperframe.m_nActive);
	sqlCommand.BindParam(5, p_rSuperframe.m_nHandheldSuperframe);
	sqlCommand.BindParam(6, p_rSuperframe.time);

	sqlCommand.ExecuteNonQuery();
}
void MySqlDevicesDal::DeleteSuperframe(int deviceID, int superframeID)
{
	LOG_DEBUG_APP("[DevicesDAL]: clean Superframe ID=" << superframeID << " for deviceID=" << deviceID);

	MySQLCommand sqlCommand(connection, ""
		"DELETE FROM Superframes "
		" WHERE DeviceID = ?001 AND SuperframeID = ?002;");
	sqlCommand.BindParam(1, deviceID);
	sqlCommand.BindParam(2, superframeID);
	sqlCommand.ExecuteNonQuery();
}


//device schedule link
void MySqlDevicesDal::CleanDeviceScheduleLinks()
{
	LOG_DEBUG_APP("[DevicesDAL]: clean all DeviceScheduleLinks ");

	MySQLCommand sqlCommand(connection, ""
		"DELETE FROM DeviceScheduleLinks ");
	sqlCommand.ExecuteNonQuery();
}
void MySqlDevicesDal::CleanDeviceScheduleLinks(int p_nDeviceId)
{
	LOG_DEBUG_APP("[DevicesDAL]: clean all DeviceScheduleLinks ");

	MySQLCommand sqlCommand(connection, ""
		"DELETE FROM DeviceScheduleLinks "
		" WHERE DeviceID = ?001;");
	sqlCommand.BindParam(1, p_nDeviceId);

	sqlCommand.ExecuteNonQuery();
}
void MySqlDevicesDal::CleanDeviceScheduleLink(int p_nDeviceId,const DeviceScheduleLink& link)
{
	LOG_DEBUG_APP("[DevicesDAL]: clean device schedule link: deviceID=" << p_nDeviceId << " superframeID=" << link.m_nSuperframeId
		<< " peerID=" << link.m_nPeerId << " slotIndex=" << link.m_nSlotIndex);

	MySQLCommand sqlCommand(connection, ""
		"DELETE FROM DeviceScheduleLinks "
		" WHERE DeviceID = ?001 AND SuperframeID = ?002 AND PeerID = ?003 AND SlotIndex = ?004;");
	sqlCommand.BindParam(1, p_nDeviceId);
	sqlCommand.BindParam(2, link.m_nSuperframeId);
	sqlCommand.BindParam(3, link.m_nPeerId);
	sqlCommand.BindParam(4, link.m_nSlotIndex);

	sqlCommand.ExecuteNonQuery();
}
void MySqlDevicesDal::InsertDeviceScheduleLink(int p_nDeviceId, const DeviceScheduleLink& p_rDeviceScheduleLink)
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

	MySQLCommand sqlCommand(connection,
		"INSERT INTO DeviceScheduleLinks(SuperframeID, DeviceID, PeerID, SlotIndex, ChannelOffset, Transmit, Receive, Shared, LinkType, Timestamp)"
		" VALUES(?001, ?002, ?003, ?004, ?005, ?006, ?007, ?008, ?009, ?010); ");

	sqlCommand.BindParam(1, p_rDeviceScheduleLink.m_nSuperframeId);
	sqlCommand.BindParam(2, p_nDeviceId);
	sqlCommand.BindParam(3, p_rDeviceScheduleLink.m_nPeerId);
	sqlCommand.BindParam(4, p_rDeviceScheduleLink.m_nSlotIndex);
	sqlCommand.BindParam(5, p_rDeviceScheduleLink.m_nChannelOffset);
	sqlCommand.BindParam(6, p_rDeviceScheduleLink.m_nTransmit);
	sqlCommand.BindParam(7, p_rDeviceScheduleLink.m_nReceive);
	sqlCommand.BindParam(8, p_rDeviceScheduleLink.m_nShared);
	sqlCommand.BindParam(9, p_rDeviceScheduleLink.m_nLinkType);
	sqlCommand.BindParam(10, p_rDeviceScheduleLink.time);

	sqlCommand.ExecuteNonQuery();
}
void MySqlDevicesDal::DeleteLink(int p_nDeviceId, int superframeID, int neighbID, int slotNo)
{
	LOG_DEBUG_APP("[DevicesDAL]: delete link for superframeID=" << superframeID << " between devID=" << p_nDeviceId << " and devID=" << neighbID);

	MySQLCommand sqlCommand(connection, ""
		"DELETE FROM DeviceScheduleLinks "
		" WHERE DeviceID = ?001 AND SuperframeID = ?002 AND PeerID = ?003 AND SlotIndex = ?004;");
	sqlCommand.BindParam(1, p_nDeviceId);
	sqlCommand.BindParam(2, superframeID);
	sqlCommand.BindParam(3, neighbID);
	sqlCommand.BindParam(4, slotNo);

	sqlCommand.ExecuteNonQuery();
}

//device health
void MySqlDevicesDal::CleanDeviceHealthReport(int deviceID)
{
	LOG_DEBUG_APP("[DevicesDAL]: clean device health for deviceID=" << deviceID);

	MySQLCommand sqlCommand(connection, ""
		"DELETE FROM ReportDeviceHealth "
		" WHERE DeviceID = ?001;");
	sqlCommand.BindParam(1, deviceID);
	sqlCommand.ExecuteNonQuery();
}
void MySqlDevicesDal::InsertDeviceHealthReport(const DeviceHealth &devHealth)
{
	LOG_DEBUG_APP("[DevicesDAL]: Add device health for device id=" << devHealth.m_nDeviceId
		<< " PowerStatus=" << devHealth.m_nPowerStatus 
		<< " Generated=" << devHealth.m_nGenerated 
		<< " Terminated=" << devHealth.m_nTerminated
		<< " DllFailures=" << devHealth.m_nDllFailures
		<< " NlFailures=" << devHealth.m_nNlFailures
		<< " CrcErrors=" << devHealth.m_nCrcErrors
		<< " NonceLost=" << devHealth.m_nNonceLost);

	MySQLCommand sqlCommand(connection, 
		"INSERT INTO ReportDeviceHealth(DeviceID, PowerStatus, Generated, Terminated, "
		" DllFailures, NlFailures, CrcErrors, NonceLost, Timestamp)"
		" VALUES(?001, ?002, ?003, ?004, ?005, ?006, ?007, ?008, ?009); ");

	sqlCommand.BindParam(1, devHealth.m_nDeviceId);
	sqlCommand.BindParam(2, devHealth.m_nPowerStatus);
	sqlCommand.BindParam(3, devHealth.m_nGenerated);
	sqlCommand.BindParam(4, devHealth.m_nTerminated);
	sqlCommand.BindParam(5, devHealth.m_nDllFailures);
	sqlCommand.BindParam(6, devHealth.m_nNlFailures);
	sqlCommand.BindParam(7, devHealth.m_nCrcErrors);
	sqlCommand.BindParam(8, devHealth.m_nNonceLost);
	sqlCommand.BindParam(9, devHealth.time);

	sqlCommand.ExecuteNonQuery();
}

//neighbors health report
void MySqlDevicesDal::CleanDeviceNeighborHealth(int deviceID, int neighbID)
{
	LOG_DEBUG_APP("[DevicesDAL]: clean device neighbor health for deviceID=" << deviceID << " and peerID=" << neighbID);

	MySQLCommand sqlCommand(connection, ""
		"DELETE FROM ReportNeighborHealthList "
		" WHERE DeviceID = ?001 AND PeerID = ?002;");
	sqlCommand.BindParam(1, deviceID);
	sqlCommand.BindParam(2, neighbID);
	sqlCommand.ExecuteNonQuery();
}
void MySqlDevicesDal::CleanDeviceNeighborsHealth(int deviceID)
{
	LOG_DEBUG_APP("[DevicesDAL]: clean device neighbors health for deviceID=" << deviceID);

	MySQLCommand sqlCommand(connection, ""
		"DELETE FROM ReportNeighborHealthList "
		" WHERE DeviceID = ?001;");
	sqlCommand.BindParam(1, deviceID);
	sqlCommand.ExecuteNonQuery();
}
void MySqlDevicesDal::InsertDeviceNeighborsHealth(const DeviceNeighborsHealth& p_rDeviceNeighborsHealth)
{

	LOG_DEBUG_APP("[DevicesDAL]: Add device neighbors health for device id=" << p_rDeviceNeighborsHealth.m_nDeviceID);

	std::list<NeighborHealth>::const_iterator it = p_rDeviceNeighborsHealth.m_oNeighborsList.begin();

	for ( ; it != p_rDeviceNeighborsHealth.m_oNeighborsList.end() ; ++it )
	{
		MySQLCommand sqlCommand(connection,
			"INSERT INTO ReportNeighborHealthList(DeviceID, PeerID, ClockSource, RSL, Transmissions, FailedTransmissions, Receptions, Timestamp)"
			" VALUES(?001, ?002, ?003, ?004, ?005, ?006, ?007, ?008); ");

		sqlCommand.BindParam(1, p_rDeviceNeighborsHealth.m_nDeviceID);	
		sqlCommand.BindParam(2, it->m_nPeerID);
		sqlCommand.BindParam(3, it->m_nClockSource);
		sqlCommand.BindParam(4, it->m_nRSL);
		sqlCommand.BindParam(5, it->m_nTransmissions);
		sqlCommand.BindParam(6, it->m_nFailedTransmissions);
		sqlCommand.BindParam(7, it->m_nReceptions);
		sqlCommand.BindParam(8, it->time);

		sqlCommand.ExecuteNonQuery();
	}
}


//neighbors signal level report
void MySqlDevicesDal::CleanDevNeighbSignalLevel(int deviceID, int neighbID)
{
	LOG_DEBUG_APP("[DevicesDAL]: clean  neighb_signals for device id=" << deviceID << " neighbID=" << neighbID);

	MySQLCommand sqlCommand(connection, ""
		"DELETE FROM ReportNeighborSignalLevels"
		" WHERE DeviceID = ?001 AND PeerID = ?002;");
	sqlCommand.BindParam(1, deviceID);
	sqlCommand.BindParam(2, neighbID);
	sqlCommand.ExecuteNonQuery();
}
void MySqlDevicesDal::CleanDevNeighbSignalLevels(int deviceID)
{
	LOG_DEBUG_APP("[DevicesDAL]: clean  neighb_signals for device id=" << deviceID);

	MySQLCommand sqlCommand(connection, ""
		"DELETE FROM ReportNeighborSignalLevels"
		" WHERE DeviceID = ?001;");
	sqlCommand.BindParam(1, deviceID);
	sqlCommand.ExecuteNonQuery();
}

void MySqlDevicesDal::InsertDevNeighbSignalLevels(int deviceID, int neighbID, int signalLevel)
{
	LOG_DEBUG_APP("[DevicesDAL]: create neighbour_signal to dev=" << neighbID << " for device id=" << deviceID);

	MySQLCommand sqlCommand(connection, ""
		"INSERT INTO ReportNeighborSignalLevels(DeviceID, PeerID, RSL)"
		" VALUES (?001, ?002, ?003)");

	sqlCommand.BindParam(1, deviceID);
	sqlCommand.BindParam(2, neighbID);
	sqlCommand.BindParam(3, signalLevel);
	sqlCommand.ExecuteNonQuery();
}

//alarms
void MySqlDevicesDal::InsertAlarm(int p_nDeviceId, int p_nAlarmType, int PeerID_GraphID, const std::string& time)
{
	LOG_DEBUG_APP("[DevicesDAL]: insert alarm for deviceID=" << p_nDeviceId
		<<"AlarmType="<<p_nAlarmType
		<<"PeerID_GraphID="<<PeerID_GraphID);

	MySQLCommand sqlCommand(connection, 
		"INSERT INTO Alarms(DeviceI, AlarmType, AlarmTime, PeerID_GraphID) "
		" VALUES(?001, ?002, ?003, ?004); ");

	sqlCommand.BindParam(1, p_nDeviceId);
	sqlCommand.BindParam(2, p_nAlarmType);
	sqlCommand.BindParam(3, time);
	sqlCommand.BindParam(4, PeerID_GraphID);

	sqlCommand.ExecuteNonQuery();
}
void MySqlDevicesDal::InsertAlarm(int p_nDeviceId, int p_nAlarmType, int PeerID_GraphID, int MIC, const std::string& time)
{
	LOG_DEBUG_APP("[DevicesDAL]: insert alarm for deviceID=" << p_nDeviceId
		<<"AlarmType="<<p_nAlarmType
		<<"PeerID_GraphID="<<PeerID_GraphID
		<<"MIC="<<MIC);

	MySQLCommand sqlCommand(connection, 
		"INSERT INTO Alarms(DeviceID, AlarmType, AlarmTime, PeerID_GraphID, MIC) "
		" VALUES(?001, ?002, ?003, ?004, ?005); ");

	sqlCommand.BindParam(1, p_nDeviceId);
	sqlCommand.BindParam(2, p_nAlarmType);
	sqlCommand.BindParam(3, time);
	sqlCommand.BindParam(4, PeerID_GraphID);
	sqlCommand.BindParam(5, MIC);

	sqlCommand.ExecuteNonQuery();
}

//burst
void MySqlDevicesDal::UpdateBurstMessageCounter(int p_nDeviceId, BurstMessage& p_rBurstMessage, const CMicroSec&  p_rLastUpdate, int p_nReceived, int p_nMissed)
{
	LOG_DEBUG_APP("[DevicesDAL]: UpdateBurstMessageCounter for burst message " << ((int)p_rBurstMessage.burstMessage) << " for device id=" << p_nDeviceId);

	MySQLCommand sqlCommand(connection,
		"UPDATE BurstCounters"
		" SET LastUpdate = ?001, Received = ?002, Missed = ?003"
		" WHERE DeviceID = ?004 AND BurstMessage = ?005 AND CommandNumber = ?006");


	sqlCommand.BindParam(1, p_rLastUpdate.GetElapsedTimeStr());
	sqlCommand.BindParam(2, p_nReceived);
	sqlCommand.BindParam(3, p_nMissed);
	sqlCommand.BindParam(4, p_nDeviceId);
	sqlCommand.BindParam(5, p_rBurstMessage.burstMessage);
	sqlCommand.BindParam(6, p_rBurstMessage.cmdNo);

	sqlCommand.ExecuteNonQuery();
}
void MySqlDevicesDal::CreateBurstMessageCounter(int p_nDeviceId, const BurstMessage& p_rBurstMessage)
{
	LOG_DEBUG_APP("[DevicesDAL]: CreateBurstMessageCounter for burst message " << ((int)p_rBurstMessage.burstMessage) << " for device id=" << p_nDeviceId);

	char sqlCmd[1000];
	sprintf(sqlCmd, "INSERT INTO BurstCounters(DeviceID, BurstMessage, CommandNumber, LastUpdate, Received, Missed) \
					 VALUES(%d, %d, %d, '1970-01-01 00:00:00', 0, 0);", p_nDeviceId, p_rBurstMessage.burstMessage, p_rBurstMessage.cmdNo);
	
	MySQLCommand sqlCommand(connection, sqlCmd);
	sqlCommand.ExecuteNonQuery();
}
void MySqlDevicesDal::DeleteBurstMessageCounter(int p_nDeviceId, int p_nBurstMessage, int p_nCmdNo)
{
	LOG_DEBUG_APP("[DevicesDAL]: DeleteBurstMessageCounter for burst message " << ((int)p_nBurstMessage) 
				<< " with cmdNo=" << (int)p_nCmdNo
				<< " for device id=" << p_nDeviceId);

	MySQLCommand sqlCommand(connection,
		"DELETE FROM BurstCounters"
		" WHERE DeviceID = ?001 AND BurstMessage = ?002 AND CommandNumber = ?003");

	sqlCommand.BindParam(1, p_nDeviceId);
	sqlCommand.BindParam(2, p_nBurstMessage);
	sqlCommand.BindParam(3, p_nCmdNo);
	
	sqlCommand.ExecuteNonQuery();
}
void MySqlDevicesDal::DeleteBurstMessageCounters(int p_nDeviceId)
{
	LOG_DEBUG_APP("[DevicesDAL]: DeleteBurstMessageCounter for device id=" << p_nDeviceId);

	MySQLCommand sqlCommand(connection,
		"DELETE FROM BurstCounters"
		" WHERE DeviceID = ?001");

	sqlCommand.BindParam(1, p_nDeviceId);
	sqlCommand.ExecuteNonQuery();
}
void MySqlDevicesDal::CreateBurstMessage(int p_nDeviceId, const BurstMessage& p_rBurstMessage)
{
	LOG_DEBUG_APP("[DevicesDAL]: CreateBurstMessage for burst message " << ((int)p_rBurstMessage.burstMessage) 
				<< " with cmdNo=" << (int)p_rBurstMessage.cmdNo
				<< " for device id=" << p_nDeviceId);

	MySQLCommand sqlCommand(connection,
		"INSERT INTO BurstMessages(DeviceID, BurstMessage, CommandNumber, UpdatePeriod, MaxUpdatePeriod, SubDeviceMAC)"
		" VALUES(?001, ?002, ?003, ?004, ?005, ?006)");

	sqlCommand.BindParam(1, p_nDeviceId);
	sqlCommand.BindParam(2, p_rBurstMessage.burstMessage);
	sqlCommand.BindParam(3, p_rBurstMessage.cmdNo);
	sqlCommand.BindParam(4, p_rBurstMessage.updatePeriod);
	sqlCommand.BindParam(5, p_rBurstMessage.maxUpdatePeriod);
    hostapp::MAC emptyMac(0);
    if (p_rBurstMessage.subDeviceMAC == emptyMac)
    {
        sqlCommand.BindParam(6, "NULL");
    }
    else
    {
        sqlCommand.BindParam(6, p_rBurstMessage.subDeviceMAC.ToString());
    }
	sqlCommand.ExecuteNonQuery();
}
void MySqlDevicesDal::UpdateBurstMessage(int p_nDeviceId, const BurstMessage& p_rBurstMessage)
{
	LOG_DEBUG_APP("[DevicesDAL]: UpdateBurstMessage for burst message " << ((int)p_rBurstMessage.burstMessage) 
		<< " with cmdNo=" << (int)p_rBurstMessage.cmdNo
		<< " for device id=" << p_nDeviceId);

	MySQLCommand sqlCommand(connection,
		"UPDATE BurstMessages"
		" SET UpdatePeriod = ?001, MaxUpdatePeriod = ?002, SubDeviceMAC = ?003"
		" WHERE DeviceID = ?004 AND BurstMessage = ?005 AND CommandNumber = ?006");

	sqlCommand.BindParam(1, p_rBurstMessage.updatePeriod);
	sqlCommand.BindParam(2, p_rBurstMessage.maxUpdatePeriod);

	hostapp::MAC emptyMac(0);
    if (p_rBurstMessage.subDeviceMAC == emptyMac)
    {
        sqlCommand.BindParam(3, "NULL");
    }
    else
    {
        sqlCommand.BindParam(3, p_rBurstMessage.subDeviceMAC.ToString());
    }

    sqlCommand.BindParam(4, p_nDeviceId);
	sqlCommand.BindParam(5, p_rBurstMessage.burstMessage);
	sqlCommand.BindParam(6, p_rBurstMessage.cmdNo);

	sqlCommand.ExecuteNonQuery();
}
void MySqlDevicesDal::DeleteBurstMessage(int p_nDeviceId, int p_nBurstMessage, int p_nCmdNo)
{
	LOG_DEBUG_APP("[DevicesDAL]: DeleteBurstMessage for burst message " << ((int)p_nBurstMessage) 
				<< " with cmdNo=" << (int)p_nCmdNo
				<< " for device id=" << p_nDeviceId);

	MySQLCommand sqlCommand(connection,
		"DELETE FROM BurstMessages"
		" WHERE DeviceID = ?001 AND BurstMessage = ?002 AND CommandNumber = ?003");

	sqlCommand.BindParam(1, p_nDeviceId);
	sqlCommand.BindParam(2, p_nBurstMessage);
	sqlCommand.BindParam(3, p_nCmdNo);
	
	sqlCommand.ExecuteNonQuery();
}
void MySqlDevicesDal::DeleteBurstMessages(int p_nDeviceId)
{
	LOG_DEBUG_APP("[DevicesDAL]: DeleteBurstMessage for device id=" << p_nDeviceId);

	MySQLCommand sqlCommand(connection,
		"DELETE FROM BurstMessages"
		" WHERE DeviceID = ?001");

	sqlCommand.BindParam(1, p_nDeviceId);
	sqlCommand.ExecuteNonQuery();
}
void MySqlDevicesDal::GetOrderedBursts(int deviceID, std::vector<BurstMessage> &bursts)
{
	LOG_DEBUG_APP("[DevicesDAL]: get ordered bursts for deviceID=" << deviceID);

	MySQLCommand
	    sqlCommand(
	        connection,
			"SELECT BurstMessage, CommandNumber, UpdatePeriod, MaxUpdatePeriod, SubDeviceMAC "
							 " FROM BurstMessages WHERE DeviceID = ?001 "
							 " ORDER BY CommandNumber, BurstMessage;");

	sqlCommand.BindParam(1, deviceID);
	
	MySQLResultSet::Ptr res = sqlCommand.ExecuteQuery();
	bursts.resize(res->RowsCount());
	int i = 0;
	for (MySQLResultSet::Iterator itDeviceChannel = res->Begin(); 
					itDeviceChannel != res->End(); itDeviceChannel++)
	{
		bursts[i].burstMessage = itDeviceChannel->Value<int>(0);
		bursts[i].cmdNo = itDeviceChannel->Value<int>(1);
		bursts[i].updatePeriod = itDeviceChannel->Value<double>(2);
		bursts[i].maxUpdatePeriod = itDeviceChannel->Value<double>(3);
		try {
		    hart7::hostapp::MAC mac(itDeviceChannel->Value<std::string>(4));
		    bursts[i].subDeviceMAC = mac;
		} catch (...) {
            hart7::hostapp::MAC emptyMac(0);
            bursts[i].subDeviceMAC = emptyMac;
        }
		++i;
	}
}
void MySqlDevicesDal::CreateTrigger(int p_nDeviceId, const Trigger& p_rTrigger)
{
	LOG_DEBUG_APP("[DevicesDAL]: CreateTrigger for burst message " << ((int)p_rTrigger.burstMessage) 
		<< " with cmdNo=" << (int)p_rTrigger.cmdNo
		<< " for device id=" << p_nDeviceId);

	MySQLCommand sqlCommand(connection,
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
void MySqlDevicesDal::UpdateTrigger(int p_nDeviceId, const Trigger& p_rTrigger)
{
	LOG_DEBUG_APP("[DevicesDAL]: UpdateTrigger for burst message " << ((int)p_rTrigger.burstMessage) 
		<< " with cmdNo=" << (int)p_rTrigger.cmdNo
		<< " for device id=" << p_nDeviceId);

	MySQLCommand sqlCommand(connection,
		"UPDATE BurstTriggers"
		" SET ModeSelection = ?001, Classification = ?002, UnitCode = ?003, Value = ?004"
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
void MySqlDevicesDal::DeleteTrigger(int p_nDeviceId, int p_nBurstMessage, int p_nCmdNo)
{
	LOG_DEBUG_APP("[DevicesDAL]: DeleteTrigger for device id=" << p_nDeviceId);

	MySQLCommand sqlCommand(connection,
		"DELETE FROM BurstTriggers"
		" WHERE DeviceID = ?001");

	sqlCommand.BindParam(1, p_nDeviceId);
	sqlCommand.ExecuteNonQuery();
}
void MySqlDevicesDal::GetOrderedTriggers(int deviceID, std::vector<Trigger> &triggers)
{
	LOG_DEBUG_APP("[DevicesDAL]: get ordered triggers for deviceID=" << deviceID);

	MySQLCommand
		sqlCommand(
		connection,
		"SELECT BurstMessage, CommandNumber, ModeSelection, Classification, UnitCode, Value"
		" FROM BurstTriggers WHERE DeviceID = ?001 "
		" ORDER BY CommandNumber, BurstMessage;");

	sqlCommand.BindParam(1, deviceID);

	MySQLResultSet::Ptr res = sqlCommand.ExecuteQuery();
	triggers.resize(res->RowsCount());
	int i = 0;
	for (MySQLResultSet::Iterator itDeviceChannel = res->Begin(); 
		itDeviceChannel != res->End(); itDeviceChannel++)
	{
		triggers[i].burstMessage = itDeviceChannel->Value<int>(0);
		triggers[i].cmdNo = itDeviceChannel->Value<int>(1);
		triggers[i].modeSelection = itDeviceChannel->Value<int>(2);
		triggers[i].classification = itDeviceChannel->Value<int>(3);
		triggers[i].unitCode = itDeviceChannel->Value<int>(4);
		triggers[i].triggerLevel = itDeviceChannel->Value<double>(5);
		++i;
	}
}

void MySqlDevicesDal::UpdateSetPublishersLog(int deviceId, int state, std::string error, std::string message)
{

}

void MySqlDevicesDal::DeleteSetPublishersLog(int deviceId)
{

}

}//namespace hostapp
}//namespace hart7

#endif
