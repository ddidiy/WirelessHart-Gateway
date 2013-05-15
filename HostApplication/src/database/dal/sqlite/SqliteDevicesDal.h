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

#ifndef SQLITEDEVICESDAL_H_
#define SQLITEDEVICESDAL_H_

#include "local/Connection.h"

#include <nlib/log.h>

#include "../IDevicesDal.h"

namespace hart7 {
namespace hostapp {

class SqliteDevicesDal : public IDevicesDal
{
	LOG_DEF("hart7.hostapp.dal.SqliteDevicesDal");
	
public:
	SqliteDevicesDal(sqlitexx::Connection& connection);
	virtual ~SqliteDevicesDal();

//init
public:
	void VerifyTables();

//device
private:
	void ResetDevices(Device::DeviceStatus newStatus);
	void GetDevices(DeviceList& list);
	void DeleteDevice(int id);
	void AddDevice(Device& device);
	void UpdateDevice(Device& device);
	void AddCommand(int deviceID, int cmdNo, std::string dataBuffer, int cmdID, int responseCode);

//topology
private:
	void CreateDeviceGraph(int fromDevice, int toDevice, int graphID, int neighbIndex);
	void CleanDevicesGraphs();
	void CleanDeviceGraphs(int deviceID);
	void DeleteGraph(int deviceId, int neighborID, int graphID);

//gateway
private:
	void AddDeviceConnections(int deviceID, const std::string& host, int port);
	void RemoveDeviceConnections(int deviceID);


//readings
public:
	void CreateChannel(int deviceID, const PublishChannel &channel);
	void MoveChannelToHistory(int channelID);
	void UpdateChannel(int channelID, int variableCode, int classification, int unitCode, const std::string &name);
	void DeleteReading(int channeNo);
	void AddEmptyReading(int DeviceID, int channelID);
	void UpdateReading(const DeviceReading& reading);
	void Log_Reading_Info(int channelNo);
	void UpdatePublishFlag(int devID, Device::PublishStatus status);
	void UpdateReadingTimeforDevice(int deviceID, const struct timeval &tv, Device::PublishStatus status);
	void GetOrderedChannels(int deviceID, std::vector<PublishChannel> &channels);

//routes
public:
	void CleanDeviceRoutes();
	void CleanDeviceRoutes(int deviceID);
	void CleanDeviceRoute(int deviceID, int routeID);
	void InsertRoute(const Route& p_rRoute);

//services
public:
	void CleanDeviceServices();
	void CleanDeviceServices(int deviceID);
	void CleanDeviceService(int deviceID, int serviceID);
	void InsertService(const Service& p_rService);

//SourceRoutes
public:
	void CleanDeviceSourceRoutes();
	void CleanDeviceSourceRoutes(int p_nDeviceId);
	void CleanDeviceSourceRoutes(int p_nDeviceId, int p_nRouteID);
	void InsertSourceRoute(const SourceRoute& p_rSourceRoute);

//superframes
public:
	void CleanDeviceSuperframes();
	void CleanDeviceSuperframes(int deviceID);
	void InsertSuperframe(const Superframe& p_rSuperframe);
	void DeleteSuperframe(int deviceID, int superframeID);

//device schedule link
public:
	void CleanDeviceScheduleLinks();
	void CleanDeviceScheduleLinks(int p_nDeviceId);
	void CleanDeviceScheduleLink(int p_nDeviceId,const DeviceScheduleLink& link);
	void InsertDeviceScheduleLink(int p_nDeviceId, const DeviceScheduleLink& p_rLink);
	void DeleteLink(int p_nDeviceId, int superframeID, int neighbID, int slotNo);


//device health
public:
	void CleanDeviceHealthReport(int deviceID);
	void InsertDeviceHealthReport(const DeviceHealth &devHealth);


//neighbors health report
public:
	void CleanDeviceNeighborHealth(int deviceID, int neighbID);
	void CleanDeviceNeighborsHealth(int deviceID);
	void InsertDeviceNeighborsHealth(const DeviceNeighborsHealth& p_rDeviceNeighborsHealth);


//neighbors signal level report
public:
	void CleanDevNeighbSignalLevel(int deviceID, int neighbID);
	void CleanDevNeighbSignalLevels(int deviceID);
	void InsertDevNeighbSignalLevels(int deviceID, int neighbID, int signalLevel);
	void Log_SignalLvel_Info(int deviceID);

//alarms
public:
	void InsertAlarm(int p_nDeviceId, int p_nAlarmType, int PeerID_GraphId, const std::string& time);
	void InsertAlarm(int p_nDeviceId, int p_nAlarmType, int PeerID_GraphId, int MIC, const std::string& time);


//bursts
public:
	void UpdateBurstMessageCounter(int p_nDeviceId, BurstMessage& p_rBurstMessage, const CMicroSec&  p_rLastUpdate, int p_nReceived, int p_nMissed);
	void CreateBurstMessageCounter(int p_nDeviceId, const BurstMessage& p_rBurstMessage);
	void DeleteBurstMessageCounter(int p_nDeviceId, int p_nBurstMessage, int p_nCmdNo);
	void DeleteBurstMessageCounters(int p_nDeviceId);
	void CreateBurstMessage(int p_nDeviceId, const BurstMessage& p_rBurstMessage);
	void UpdateBurstMessage(int p_nDeviceId, const BurstMessage& p_rBurstMessage);
	void DeleteBurstMessage(int p_nDeviceId, int p_nBurstMessage, int p_nCmdNo);
	void DeleteBurstMessages(int p_nDeviceId);
	void GetOrderedBursts(int deviceID, std::vector<BurstMessage> &bursts);
	void CreateTrigger(int p_nDeviceId, const Trigger& p_rTrigger);
	void UpdateTrigger(int p_nDeviceId, const Trigger& p_rTrigger);
	void DeleteTrigger(int p_nDeviceId, int p_nBurstMessage, int p_nCmdNo);
	void GetOrderedTriggers(int deviceID, std::vector<Trigger> &triggers);

//Log
public:
	void UpdateSetPublishersLog(int deviceId, int state, std::string error, std::string message);
	void DeleteSetPublishersLog(int deviceId);

private:
	sqlitexx::Connection& connection;
	sqlitexx::CSqliteStmtHelper m_oUpdateDeviceReadings_Prepare;
	sqlitexx::CSqliteStmtHelper m_oUpdatePublishFlag_compiled;
	sqlitexx::CSqliteStmtHelper m_oUpdateLastReadAndPublishFlag_compiled;
	sqlitexx::CSqliteStmtHelper m_oInsertGraph_compiled;
	sqlitexx::CSqliteStmtHelper m_oDeleteGraph_compiled;
	sqlitexx::CSqliteStmtHelper m_oInsertRoute_compiled;
	sqlitexx::CSqliteStmtHelper m_oDeleteRoute_compiled;
	sqlitexx::CSqliteStmtHelper m_oInsertSourceRoute_compiled;
	sqlitexx::CSqliteStmtHelper m_oDeleteSourceRoute_compiled;
	sqlitexx::CSqliteStmtHelper m_oInsertService_compiled;
	sqlitexx::CSqliteStmtHelper m_oDeleteService_compiled;
	sqlitexx::CSqliteStmtHelper m_oInsertSuperframes_compiled;
	sqlitexx::CSqliteStmtHelper m_oDeleteSuperframes_compiled;
	sqlitexx::CSqliteStmtHelper m_oInsertScheduleLink_compiled;
	sqlitexx::CSqliteStmtHelper m_oDeleteScheduleLink1_compiled;
	sqlitexx::CSqliteStmtHelper m_oDeleteScheduleLink2_compiled;
	sqlitexx::CSqliteStmtHelper m_oInsertDeviceHealth_compiled;
	sqlitexx::CSqliteStmtHelper m_oDeleteDeviceHealth_compiled;
	sqlitexx::CSqliteStmtHelper m_oInsertNeighbsHealth_compiled;
	sqlitexx::CSqliteStmtHelper m_oDeleteNeighbsHealth_compiled;
	sqlitexx::CSqliteStmtHelper m_oInsertNeighbsSigLevel_compiled;
	sqlitexx::CSqliteStmtHelper m_oDeleteNeighbsSigLevel_compiled;
    sqlitexx::CSqliteStmtHelper m_oInsertBurstMessageCounter_compiled;
    sqlitexx::CSqliteStmtHelper m_oUpdateBurstMessageCounter_compiled;
    sqlitexx::CSqliteStmtHelper m_oDeleteBurstMessageCounter_compiled;
    sqlitexx::CSqliteStmtHelper m_oDeleteBurstMessageCounters_compiled;
    sqlitexx::CSqliteStmtHelper m_oUpdateSetPublishersLog_compiled;
    sqlitexx::CSqliteStmtHelper m_oDeleteSetPublishersLog_compiled;
};

}
}

#endif /*SQLITEDEVICESDAL_H_*/
