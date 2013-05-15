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

#ifndef IDEVICESDAL_H_
#define IDEVICESDAL_H_

#include <Shared/MicroSec.h>

#include <WHartHost/model/Device.h>
#include <WHartHost/model/reports/Route.h>
#include <WHartHost/model/reports/SourceRoute.h>
#include <WHartHost/model/reports/Service.h>
#include <WHartHost/model/reports/Superframe.h>
#include <WHartHost/model/DeviceReading.h>
#include <WHartHost/model/PublisherInfo.h>
#include <WHartHost/model/reports/DeviceScheduleLink.h>
#include <WHartHost/model/reports/DeviceHealth.h>
#include <WHartHost/model/reports/NeighborHealth.h>



namespace hart7 {
namespace hostapp {


class IDevicesDal
{
public:
	virtual ~IDevicesDal()
	{
	}

//device
public:
	virtual void ResetDevices(Device::DeviceStatus newStatus) = 0;
	virtual void GetDevices(DeviceList& list) = 0;
	virtual void DeleteDevice(int id) = 0;
	virtual void AddDevice(Device& device) = 0;
	virtual void UpdateDevice(Device& device) = 0;

//topology
public:
	virtual void CreateDeviceGraph(int fromDevice, int toDevice, int graphID, int neighbIndex) = 0;
	virtual void CleanDevicesGraphs() = 0;
	virtual void CleanDeviceGraphs(int deviceID) = 0;
	virtual void DeleteGraph(int deviceId, int neighborID, int graphID) = 0;

//gateway
public:
	virtual void AddDeviceConnections(int deviceID, const std::string& host, int port) = 0;
	virtual void RemoveDeviceConnections(int deviceID) = 0;

//wh_general_cmd
public:
	virtual void AddCommand(int deviceID, int cmdNo, std::string dataBuffer, int cmdID, int responseCode) =  0;

//readings
public:
	virtual void CreateChannel(int deviceID,const  PublishChannel &channel) = 0;
	virtual void MoveChannelToHistory(int channelID) = 0;
	virtual void UpdateChannel(int channelID, int variableCode, int classification, int unitCode, const std::string &name) = 0;
	virtual void DeleteReading(int channeNo) = 0;
	virtual void AddEmptyReading(int DeviceID, int channelID) = 0;
	virtual void UpdateReading(const DeviceReading& reading) = 0;
	virtual void Log_Reading_Info(int channelNo) = 0;
	virtual void UpdatePublishFlag(int devID, Device::PublishStatus status) = 0;
	virtual void UpdateReadingTimeforDevice(int deviceID, const struct timeval &tv, Device::PublishStatus status) = 0;
	virtual void GetOrderedChannels(int deviceID, std::vector<PublishChannel> &channels) = 0;

//routes
public:
	virtual void CleanDeviceRoutes() = 0;
	virtual void CleanDeviceRoutes(int deviceID) = 0;
	virtual void CleanDeviceRoute(int deviceID, int routeID) = 0;
	virtual void InsertRoute(const Route& p_rRoute) = 0;

//services
public:
	virtual void CleanDeviceServices() = 0;
	virtual void CleanDeviceServices(int deviceID) = 0;
	virtual void CleanDeviceService(int deviceID, int serviceID) = 0;
	virtual void InsertService(const Service& p_rService) = 0;

//SourceRoutes
public:
	virtual void CleanDeviceSourceRoutes() = 0;
	virtual void CleanDeviceSourceRoutes(int p_nDeviceId) = 0;
	virtual void CleanDeviceSourceRoutes(int deviceId, int routeID) = 0;
	virtual void InsertSourceRoute(const SourceRoute& p_rSourceRoute) = 0;

//superframes
public:
	virtual void CleanDeviceSuperframes() = 0;
	virtual void CleanDeviceSuperframes(int deviceID) = 0;
	virtual void InsertSuperframe(const Superframe& p_rSuperframe) = 0;
	virtual void DeleteSuperframe(int deviceID, int superframeID) = 0;

//device schedule link
public:
	virtual void CleanDeviceScheduleLinks() = 0;
	virtual void CleanDeviceScheduleLinks(int p_nDeviceId) = 0;	
	virtual void CleanDeviceScheduleLink(int p_nDeviceId,const DeviceScheduleLink& link) = 0;
	virtual void InsertDeviceScheduleLink(int p_nDeviceId, const DeviceScheduleLink& p_rLink) = 0;
	virtual void DeleteLink(int p_nDeviceId, int superframeID, int neighbID, int slotNo) = 0;


//device health
public:
	virtual void CleanDeviceHealthReport(int deviceID) = 0;
	virtual void InsertDeviceHealthReport(const DeviceHealth &devHealth) = 0;


//neighbors health report
public:
	virtual void CleanDeviceNeighborHealth(int deviceID, int neighbID) = 0;
	virtual void CleanDeviceNeighborsHealth(int deviceID) = 0;
	virtual void InsertDeviceNeighborsHealth(const DeviceNeighborsHealth& p_rDeviceNeighborsHealth) = 0;


//neighbors signal level report
public:
	virtual void CleanDevNeighbSignalLevel(int deviceID, int neighbID) = 0;
	virtual void CleanDevNeighbSignalLevels(int deviceID) = 0;
	virtual void InsertDevNeighbSignalLevels(int deviceID, int neighbID, int signalLevel) = 0;
	virtual void Log_SignalLvel_Info(int deviceID) = 0;

//alarms
public:
	virtual	void InsertAlarm(int p_nDeviceId, int p_nAlarmType, int PeerID_GraphId, const std::string& time) = 0;
	virtual void InsertAlarm(int p_nDeviceId, int p_nAlarmType, int PeerID_GraphId, int MIC, const std::string& time) = 0;


//bursts
public:
	virtual void UpdateBurstMessageCounter(int p_nDeviceId, BurstMessage& p_rBurstMessage, const CMicroSec&  p_rLastUpdate, int p_nReceived, int p_nMissed) = 0;
	virtual void CreateBurstMessageCounter(int p_nDeviceId, const BurstMessage& p_rBurstMessage) = 0;
	virtual void DeleteBurstMessageCounter(int p_nDeviceId, int p_nBurstMessage, int p_nCmdNo) = 0;
	virtual void DeleteBurstMessageCounters(int p_nDeviceId) = 0;
	virtual void CreateBurstMessage(int p_nDeviceId, const BurstMessage& p_rBurstMessage) = 0;
	virtual void UpdateBurstMessage(int p_nDeviceId, const BurstMessage& p_rBurstMessage) = 0;
	virtual void DeleteBurstMessage(int p_nDeviceId, int p_nBurstMessage, int p_nCmdNo) = 0;
	virtual void DeleteBurstMessages(int p_nDeviceId) = 0;
	virtual void GetOrderedBursts(int deviceID, std::vector<BurstMessage> &bursts) = 0;
	virtual void CreateTrigger(int p_nDeviceId, const Trigger& p_rTrigger) = 0;
	virtual void UpdateTrigger(int p_nDeviceId, const Trigger& p_rTrigger) = 0;
	virtual void DeleteTrigger(int p_nDeviceId, int p_nBurstMessage, int p_nCmdNo) = 0;
	virtual void GetOrderedTriggers(int deviceID, std::vector<Trigger> &triggers) = 0;

//log
public:
    virtual void UpdateSetPublishersLog(int deviceId, int state, std::string error, std::string message) = 0;
    virtual void DeleteSetPublishersLog(int deviceId) = 0;

};


} // namespace hostapp
} // namespace hart7


#endif /*IDEVICESDAL_H_*/
