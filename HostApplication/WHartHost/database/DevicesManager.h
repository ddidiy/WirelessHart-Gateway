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

#ifndef DEVICESMANAGER_H_
#define DEVICESMANAGER_H_


#include <WHartHost/model/reports/TopologyReport.h>
#include <WHartHost/model/DeviceReading.h>
#include <WHartHost/model/NotificationDevInfo.h>
#include <WHartHost/model/reports/DeviceHealth.h>
#include <WHartHost/model/reports/NeighbSignalLevels.h>
#include <WHartHost/model/reports/TopologyReport.h>
#include <WHartHost/model/reports/Service.h>
#include <WHartHost/model/reports/Superframe.h>
#include <WHartHost/model/reports/Route.h>
#include <WHartHost/model/reports/SourceRoute.h>
#include <WHartHost/model/reports/DeviceScheduleLink.h>
#include <WHartHost/model/reports/Alarm.h>


#include <WHartHost/gateway/GatewayIO.h>

#include <loki/Function.h> //for callback

#include <Shared/MicroSec.h>

#include <nlib/log.h>
#include <nlib/exception.h>

#include <set>
#include <list>
#include <map>
#include <queue>

#include "src/database/NodesRepository.h"
#include "CommonManager.h"


namespace hart7 {
namespace hostapp {

class ReportsProcessor;
class IFactoryDal;
class DevicesGraph;
struct Route;
struct SourceRoute;
struct Service;
struct Superframe;
struct DeviceScheduleLink;
struct DeviceNeighborsHealth;


class NoGatewayException : public nlib::Exception
{
public:
	NoGatewayException() :
		nlib::Exception("There is no gateway defined on system!")
	{
	}
	NoGatewayException(std::string str) :
		nlib::Exception(boost::str(boost::format("There is no gateway: %1% !") % str))
	{
	}
};


/*
 * it manages devices
 */
class DevicesManager
{
	LOG_DEF("hart7.hostapp.DevicesManager");
	
public:
	typedef Loki::Function<void(int)> BurstCounterTaskPeriodCallback;
	typedef Loki::Function<void(int, const CMicroSec&, C839_ChangeNotification_Resp&)> PubNotificationReceiveCallback;
	typedef Loki::Function<void(int)> PubDeviceDeletionCallback;
	typedef Loki::Function<void(int, std::list<BurstMessage>&, std::list<BurstMessage>&, std::list<BurstMessage>&)> PubDeviceBurstsChangeCallback;
    typedef Loki::Function<void(MAC&, PublisherInfo&)> UpdatePublishersCacheCallback;
	typedef Loki::Function<void(MAC&, PublisherInfo&, int)> UpdateStoredPublishersCallback;
	typedef Loki::Function<void(const MAC&, int, SetPublisherState, SetPublisherError, std::string)> UpdatePublisherStateCallback;
    typedef Loki::Function<void()> IssueCommandCallback;
    typedef Loki::Function<void(const DeviceReading& p_oReading)> CacheReadingCallback;

public:
	DevicesManager(bool useCache, const std::string &strParams="", int StorageType = STORE_TYPE_VIRT);
	virtual ~DevicesManager();

//topology
private:
	void FindAccessPointDevs(TopologyReport::DevicesListT& topology, int gwTopoIndex);
	void ResetDevicesChanges();
	void UnregisterExistingDevices(TopologyReport::DevicesListT& topology, bool &doUpdateIPv6Index);
	void ProcessRegisteredDevices(DevicesGraph &devGraph, TopologyReport::DevicesListT& topology, bool &doUpdateIPv6Index);
	void UpdateChangedDevicesInDB(TopologyReport::DevicesListT& topology);
	void SaveTopologyInDB(DevicesGraph &devGraph, TopologyReport::DevicesListT& topology);
	void SaveDeviceNeighbours(DevicesGraph &devGraph, int vertexNo);
	void ChangeDeviceGraphs(const DevicePtr& nodeFrom, const TopologyReport::Device::GraphsListT& graphs);
public:
	void ResetTopology();
	void ProcessTopology(TopologyReport::DevicesListT& topology, int gwTopoIndex, bool doUnregDevs);

//general wh_cmd
public:
	void InsertRawCommand(DevicePtr device, int cmdNo, std::string dataBuffer, int cmdID, int responseCode); 

//topology notification
public:
	void UnregisterExistingDevices(std::vector<MAC> &macs/*[in/out]*/, int &unregisteredNo /*[out]*/);
	void AddNewDeviceInfo(NotificationDevInfo &devInfo);
	void AddNewDevice(DevicePtr device);
	void RegisterDevice(DevicePtr device);
	void UpdateDevice(DevicePtr device, bool nicknameChanged, bool statusChanged);
	bool IsDeviceInfoInProgress(MAC &mac);
	void AddDeviceInfoInProgress(MAC &mac);
	void DelDeviceInfoInProgress(MAC &mac);
private:
	void ReplaceNeighbours(DevicePtr fromDev, NotificationDevInfo::NeighborsListT &list);


//devices
public:
	DevicePtr FindDevice(boost::int32_t deviceID) const;
	DevicePtr FindDevice(const MAC& deviceMAC) const;
	DevicePtr FindDevice(const NickName& deviceNickname) const;
	DevicePtr FindRegisteredDevice(const NickName& deviceNickname) const;
	const DevicePtr GatewayDevice() const;
	const DevicePtr SystemManagerDevice() const;
	int RegisteredDevicesNo() const {return m_Repository.registeredNodesByNickName.size();}

//readings
public:
	void UpdatePublishFlag(int devID, Device::PublishStatus status);
	void UpdateReadingTimeforDevice(int deviceID, const struct timeval &tv, Device::PublishStatus status);
	void ProcessDBChannels(int deviceID, PublishChannelSetT &storedList);
	void ProcessReading(const DeviceReading& p_rReading);
	void ProcessReadingsTransaction(const DeviceReadingsMapT& p_oReadingsCache);

private:
	void ProcessChannelsDifference(int deviceID, PublishChannelSetT &storedList, const std::vector<PublishChannel> &channels);

// callbacks setting
public:
	void SetBurstCounterTaskPeriodCallback(BurstCounterTaskPeriodCallback p_oCallback);
	void UpdateBurstCounterTaskPeriod(int p_nUpdatePeriod/*ms*/);

	void SetPubNotificationCallback(PubNotificationReceiveCallback p_oCallback);
	void PubNotificationReceived(int p_nDeviceID, const CMicroSec& p_rRecvTime, C839_ChangeNotification_Resp& p_rData);

	void SetPubDeviceDeletionCallback(PubDeviceDeletionCallback p_oCallback);
	void PubDeviceDeletionReceived(int p_nDeviceId);

	void SetPubDeviceBurstsChangeCallback(PubDeviceBurstsChangeCallback p_oCallback);
	void PubDeviceBurstsChangeReceived(int p_nDeviceId,
		std::list<BurstMessage> &p_oAddedBursts,
		std::list<BurstMessage> &p_oDeletedBursts,
		std::list<BurstMessage> &p_oChangedBursts);

	void SetUpdatePublishersCacheCallback(UpdatePublishersCacheCallback p_oUpdatePublishersCache);
	void UpdatePublishersCache(MAC& p_rMac, PublisherInfo& p_rPublisherInfo);

    void SetUpdateStoredPublishersCallback(UpdateStoredPublishersCallback p_oUpdateStoredPublishers);
    void UpdateStoredPublishers(MAC& p_rMac, PublisherInfo& p_rPublisherInfo, int );

    void SetUpdatePublisherStateCallback(UpdatePublisherStateCallback p_oUpdatePublisherState);
    void UpdatePublisherState(const MAC& p_rMac, int p_nBurst, SetPublisherState p_eState, SetPublisherError p_eError, std::string p_sMessage);

    void SetIssueTopologyCommandCallback(IssueCommandCallback p_oIssueTopologyCommand);
    void IssueTopologyCommand();

    void SetCacheReadingCallback(CacheReadingCallback p_oCallback);
    void CacheReading(const DeviceReading& p_oReading);

// burst
public:
    void BeginUpdateBurstMessageCounter();
	void UpdateBurstMessageCounter(int p_nDeviceId, BurstMessage& p_rBurstMessage, const CMicroSec&  p_rLastUpdate, int p_nReceived, int p_nMissed);
	void EndUpdateBurstMessageCounter(bool commit);
	void RemoveBursts(int deviceID);
	void ProcessDBBursts(int deviceID, BurstMessageSetT &storedList, std::list<BurstMessage> &added/*[out]*/,
		std::list<BurstMessage> &updated/*[out]*/, std::list<BurstMessage> &deleted/*[out]*/);
	void UpdateBurstMessage(int p_nDeviceId, BurstMessage& p_rBurstMessage);

	void ProcessDBTriggers(int deviceID, TriggerSetT &storedList);

private:
	void ProcessBurstDifference(int deviceID, BurstMessageSetT &storedList, 
		const std::vector<BurstMessage> &bursts, std::list<BurstMessage> &added/*[out]*/,
		std::list<BurstMessage> &updated/*[out]*/, std::list<BurstMessage> &deleted/*[out]*/);
	void ProcessTriggersDifference(int deviceID, TriggerSetT &storedList, 
		const std::vector<Trigger> &triggers);

//reports
public:
	bool CleanReports(int devID);
	bool CleanReports();
	

//graphs
public:
	void ReplaceDeviceGraphs(int deviceId, std::list<GraphNeighbor>& graphList);
	void ChangeDeviceGraphs(int deviceId, std::list<GraphNeighbor>& graphList, bool skipCache = false);
	void DeleteGraph(int deviceId, int neighborID, int graphID);

//clockSource
public:
	void UpdateSetPublishersLog(PublisherStateMAP_T& p_PublishersStateMap);
    void DeleteSetPublishersLog(int deviceId);

//routes
public:
	void ReplaceRoutes(int deviceID, RoutesListT& p_rRoute);
	void ChangeRoutes(RoutesListT& p_rRoute, bool skipCache = false);
	void DeleteRoute(int deviceID, int routeID);

//SourceRoutes
public:
	void ReplaceSourceRoutes(int p_nDeviceId, SourceRoutesListT& p_rSourceRoutes);
	void ChangeSourceRoutes(SourceRoutesListT& p_rSourceRoutes, bool skipCache = false);
	void DeleteSourceRoute(int p_nDeviceId, int routeID);

//services
public:
	void ReplaceServices(int deviceID, ServicesListT &p_rServices);
	void ChangeServices(ServicesListT &p_rServices, bool skipCache = false);
	void DeleteService(int deviceID, int serviceID);

//superframes
public:
	void ReplaceSuperframes(int deviceID, SuperframesListT& p_rSuperframes);
	void ChangeSuperframes(SuperframesListT& p_rSuperframes, bool skipCache = false);
	void DeleteSuperframe(int deviceID, int superframeID);

//device schedule link
public:
	void ReplaceDevicesScheduleLinks(DevicesScheduleLinks &links);
	void ChangeDevicesScheduleLinks(int p_nDeviceId, std::list<DeviceScheduleLink>& links, bool skipCache = false);
	void DeleteLink(int p_nDeviceId, int superframeID, int neighbID, int slotNo);

//device health
public:
	void ReplaceDevicesHealthReport(std::vector<DeviceHealth> &devicesHealth);
	void ReplaceDeviceHealthReport(DeviceHealth &deviceHealth, bool skipCache = false);

//neighbors health report
public:
	void ChangeDeviceNeighborsHealth(DeviceNeighborsHealth &m_oNeighborsHealthCache, bool skipCache = false);
	void ReplaceDeviceNeighborsHealth(DeviceNeighborsHealth &m_oNeighborsHealthCache);
	void ReplaceDevicesNeighborsHealth(DevicesNeighborsHealth &m_oNeighborsHealthCache);

//neighbors signal level report
public:
	void ChangeDevNeighbSignalLevels(int deviceID, const std::list<NeighbourSignalLevel>& p_rDevNeighbSignalLevels, bool skipCache = false);
	void ReplaceDevNeighbSignalLevels(int deviceID, const std::list<NeighbourSignalLevel>& p_rDevNeighbSignalLevels);

//alarms
public:
	void InsertAlarm(const Alarm &p_rAlarm);
	

//reports cache
private:
	void FlushGraphs();
	void FlushRoutes();
	void FlushSourceRoutes();
	void FlushServices();
	void FlushSuperframes();
	void FlushDeviceScheduleLinks();
	void FlushDeviceHealth();
	void FlushDeviceNeighbHealth();
	void FlushDeviceNeighbsSignalLevel();
	void FlushAlarms();
	void FlushEachReport();
public:
	void FlushReports();
	bool IsAnyReportToFlush();
	
	
//gateway connection
private:
	void ChangeGatewayStatus(Device::DeviceStatus newStatus);
public:
	void HandleGWConnect(const std::string& host, int port);
	void HandleGWDisconnect();

public:
	void SetReadingsSavePeriod(int p_nReadingsSavePeriod /*ms*/) { m_nReadingsSavePeriod = p_nReadingsSavePeriod; } // milliseconds

//iterator pattern
public:
	typedef NodesRepository::NodesByMACT::iterator iterator_by_mac;
	typedef NodesRepository::NodesByMACT::const_iterator const_iterator_by_mac;
	typedef NodesRepository::NodesByNickNameT::iterator iterator_by_nick;
	typedef NodesRepository::NodesByNickNameT::const_iterator const_iterator_by_nick;
	iterator_by_mac BeginAllByMac(){return m_Repository.allNodesByMAC.begin();};
	const_iterator_by_mac BeginAllByMac()const {return m_Repository.allNodesByMAC.begin();};
	iterator_by_mac EndAllByMac(){return m_Repository.allNodesByMAC.end();};
	const_iterator_by_mac EndAllByMac() const {return m_Repository.allNodesByMAC.end();};
	iterator_by_nick BeginRegisteredByNick(){return m_Repository.registeredNodesByNickName.begin();};
	const_iterator_by_nick BeginRegisteredByNick()const {return m_Repository.registeredNodesByNickName.begin();};
	iterator_by_nick EndRegisteredByNick(){return m_Repository.registeredNodesByNickName.end();};
	const_iterator_by_nick EndRegisteredByNick() const {return m_Repository.registeredNodesByNickName.end();};


private:
	IFactoryDal& factoryDal;
	bool isGatewayConnected;
	DevicePtr			m_smDevPtr;
	DevicePtr			m_gwDevPtr;
	ReportsProcessor	*m_pRepProcessor;

private: // cache for saving reading data in DB
	unsigned int		m_nReadingsSavePeriod; // seconds

private:
	NodesRepository		m_Repository;

private:
	std::set<MAC>		m_devInfoInProgress;

// callbacks
private:
    BurstCounterTaskPeriodCallback  m_oHandleBurstCounterTaskPeriod;
    PubNotificationReceiveCallback  m_oHandlePubNotificationRecv;
    PubDeviceDeletionCallback       m_oHandlePubDeviceDeletion;
    PubDeviceBurstsChangeCallback   m_oHandlePubDeviceBurstsChange;
    UpdatePublishersCacheCallback   m_oUpdatePublishersCache;
    UpdateStoredPublishersCallback  m_oUpdateStoredPublishers;
    UpdatePublisherStateCallback    m_oUpdatePublisherState;
    IssueCommandCallback            m_oIssueTopologyCommand;
    CacheReadingCallback            m_oCacheReading;

//REPORTS CACHE 
private:
	DevicesGraphNeighborsListT		m_devicesGraphNeighbsCache;
	RoutesListT						m_routesCache;
	SourceRoutesListT				m_sourceRoutesCache;
	ServicesListT					m_servicesCache;
	SuperframesListT				m_superframesCache;
	DevicesScheduleLinksListT		m_devicesScheduleLinksCache;
	DevicesHealthListT				m_devicesHealthCache;
	DevicesNeighborsHealthListT		m_devicesNeighsHealthCache;
	NeighbourSignalLevelsListT		m_neighbsSignalLevelCache;
	AlarmsListT						m_alarmsCache;			
	bool							m_useCache;
};

} //namspace hostapp
} //namspace hart7

#endif /*DEVICESMANAGER_H_*/
