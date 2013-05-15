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


#include <WHartHost/database/DevicesManager.h>
#include <WHartHost/model/reports/Route.h>
#include <WHartHost/model/reports/SourceRoute.h>
#include <WHartHost/model/reports/Service.h>
#include <WHartHost/model/reports/Superframe.h>
#include <WHartHost/model/reports/DeviceScheduleLink.h>
#include <WHartHost/model/reports/DeviceHealth.h>
#include <WHartHost/model/reports/NeighborHealth.h>

#include <cmath>
#include <string.h>
#include <stdio.h>

#include <boost/format.hpp>

#include "ReportsProcessor.h"
#include "dal/IFactoryDal.h"
#include "EntitiesStore.h"
#include "DevicesGraph.h"

#include <queue>

namespace hart7 {
namespace hostapp {


static void SkipNeighboursforDevNN(ReportsProcessor::TopoStructIndexT::const_iterator &i/*[in/out]*/,
								   ReportsProcessor::TopoStructIndexT::const_iterator iEnd, 
								   TopologyReport::DevicesListT& topology)
{
	int indexDevice = (i++)->second.indexDevice;
	while(i != iEnd)
	{
		if (i->second.indexDevice != indexDevice)
			break;
		++i;
	}
}

extern EntitiesStore::StoreType convert(int val);

DevicesManager::DevicesManager(bool useCache, const std::string &strParams, int StorageType) :
	factoryDal(*EntitiesStore::GetInstance(strParams, convert(StorageType))->GetFactoryDal()),
	m_oHandlePubNotificationRecv(0),
	m_oHandlePubDeviceDeletion(0),
	m_oHandlePubDeviceBurstsChange(0),
	m_oUpdatePublishersCache(0),
	m_useCache(useCache)
{
	isGatewayConnected = false;
	m_pRepProcessor = new ReportsProcessor();
}

DevicesManager::~DevicesManager()
{
	delete m_pRepProcessor;
}

//topology
void DevicesManager::FindAccessPointDevs(TopologyReport::DevicesListT& topology, int gwTopoIndex)
{
	m_pRepProcessor->CreateIndexForTopoStruct(topology);
	m_pRepProcessor->FillDevListIndexForNeighbours(topology);
	if (gwTopoIndex == TopologyReport::NO_TOPO_GW_INDEX)
	{
		THROW_EXCEPTION1(NoGatewayException, "no gw in received topology structure");
	}
	for (int i = 0; i < (int)topology[gwTopoIndex].NeighborsList.size(); ++i)
	{
		if (topology[gwTopoIndex].NeighborsList[i].DevListIndex != TopologyReport::NO_TOP0_NEIGHB_INDEX)
			if (topology[topology[gwTopoIndex].NeighborsList[i].DevListIndex].DeviceType != Device::dtSystemManager)
				topology[topology[gwTopoIndex].NeighborsList[i].DevListIndex].DeviceType = Device::dtAccessPoint;
	}
}
void DevicesManager::ResetDevicesChanges()
{
	for (NodesRepository::NodesByMACT::iterator it = m_Repository.allNodesByMAC.begin(); it != m_Repository.allNodesByMAC.end(); it++)
		it->second->ResetChanged();
}
void DevicesManager::UnregisterExistingDevices(TopologyReport::DevicesListT& topology, bool &doUpdateIPv6Index)
{
	LOG_DEBUG_APP("[ProcessTopology] unregister_devices=" << topology.size()  << ": begin");
	
	if (topology.size() == 0)
	{
		for(NodesRepository::NodesByNickNameT::const_iterator i = m_Repository.registeredNodesByNickName.begin(); i != m_Repository.registeredNodesByNickName.end(); i++)
		{ 	//for every ip set unregister state
			LOG_WARN_APP("[ProcessTopology] unregister_devices - dev_nn = " <<  i->first <<
		 					" was set unregistered");
			i->second->Status(Device::dsUnregistered);
			doUpdateIPv6Index = true;
		}
	}
	else
	{
		ReportsProcessor::TopoStructIndexT::const_iterator j = m_pRepProcessor->GetTopoStructIndex()->begin();
	
		for(NodesRepository::NodesByNickNameT::const_iterator i = m_Repository.registeredNodesByNickName.begin(); i != m_Repository.registeredNodesByNickName.end(); i++)
		{ //for every ip check if it is in topology_struct
			
			if (j == m_pRepProcessor->GetTopoStructIndex()->end())
			{
				LOG_WARN_APP("[ProcessTopology] unregister_devices - dev_nn = " <<  i->first <<
		 					" was set unregistered");
				i->second->Status(Device::dsUnregistered);
				doUpdateIPv6Index = true;
				continue;
			}
			if (i->first == j->first.deviceNN)
			{
				//found a match...
			}
			else 
			{
				if (j->first.deviceNN < i->first)
				{
					do
					{
						SkipNeighboursforDevNN(j, m_pRepProcessor->GetTopoStructIndex()->end(), topology);
						if (j == m_pRepProcessor->GetTopoStructIndex()->end())
							break;
					}while (j->first.deviceNN < i->first);
					if (j == m_pRepProcessor->GetTopoStructIndex()->end())
					{
						LOG_WARN_APP("[ProcessTopology] unregister_devices - dev_nn = " <<  i->first <<
			 						" was set unregistered");
						i->second->Status(Device::dsUnregistered);
						doUpdateIPv6Index = true;
						continue; // continue 'for'
					}
					if (i->first < j->first.deviceNN)
					{
						LOG_WARN_APP("[ProcessTopology] unregister_devices - dev_nn = " <<  i->first <<
			 						" was set unregistered");
						i->second->Status(Device::dsUnregistered);
						doUpdateIPv6Index = true;
						continue;
					}
					
					//found a match...
				}
				else 
				{
					LOG_WARN_APP("[ProcessTopology] unregister_devices - dev_nn = " <<  i->first <<
			 					" was set unregistered");
					i->second->Status(Device::dsUnregistered);
					doUpdateIPv6Index = true;
					continue;
				}

				if (i->second->Mac() != topology[j->second.indexDevice].DeviceMAC)
				{
					LOG_WARN_APP("[ProcessTopology] unregister_devices - dev_nn = " <<  i->first <<
			 					" was set unregistered because of mac difference");
					i->second->Status(Device::dsUnregistered);
					doUpdateIPv6Index = true;
					continue;
				}
			}
			
			//increment here
			SkipNeighboursforDevNN(j, m_pRepProcessor->GetTopoStructIndex()->end(), topology);
		}
	}
	LOG_DEBUG_APP("[ProcessTopology] unregister_devices=" << topology.size()  << ": end");

}
void DevicesManager::ProcessRegisteredDevices(DevicesGraph &devGraph, TopologyReport::DevicesListT& topology, bool &doUpdateIPv6Index)
{
	
	LOG_DEBUG_APP("[ProcessTopology] process registered devices=" << topology.size() << ": begin");

	if (m_smDevPtr)	//it may not be set
		m_smDevPtr->SetVertexNo(Device::NO_VERTEX_NO);
	m_gwDevPtr->SetVertexNo(Device::NO_VERTEX_NO);

	devGraph.SetVerticesNo(topology.size());

	int vertexNo = 0;
	for (TopologyReport::DevicesListT::const_iterator it = topology.begin(); it != topology.end(); ++it)
	{
		DevicePtr node = m_Repository.Find(it->DeviceMAC);
		if (!node)
		{
			node.reset(new Device());
			node->Mac(it->DeviceMAC);
			node->Nickname(it->DeviceNickName);
			doUpdateIPv6Index = true;
			m_Repository.Add(node);
		}
		else
		{
			node->Nickname(it->DeviceNickName);
			if (node->Changed())				//if node changed then do updateNickNameIndex
				doUpdateIPv6Index = true;
		}

		node->Status(Device::dsRegistered);
		if (node->Changed())					//if status changed then do updateNickNameIndex
			doUpdateIPv6Index = true;
		

		node->SetVertexNo(vertexNo);	
		if (node->Type() == Device::dtSystemManager)
		{
			node->Level(0);
			m_smDevPtr = node;		//for optimisation
		}

		if (node->Type() == Device::dtGateway)
			m_gwDevPtr = node;		//for optimisation

		devGraph.AddDataToVertex(vertexNo, node);
		for (TopologyReport::Device::NeighborsListT::const_iterator j = it->NeighborsList.begin();
						j != it->NeighborsList.end(); ++j)
		{
			if (j->DevListIndex != -1)
				devGraph.AddArc(vertexNo, j->DevListIndex,j->Health);
		}
		vertexNo++;

		node->Type((Device::DeviceType) it->DeviceType);
		node->PowerStatus(it->PowerSupplyStatus);
		node->SetTAG(it->LongTag);

		node->SetSoftwareRevision(it->softwareRev);
		node->SetDeviceCode(it->deviceCode);
		node->SetRejoinCount(it->joinCount);
		node->SetAdapter(it->adapter);

		
		//also notification bit mask
		node->SetNotificationBitMask(node->GetNotificationBitMask() | it->NotificationMask);
	}
	
	LOG_DEBUG_APP("[ProcessTopology] process registered devices=" << topology.size() << ": end");
}
void DevicesManager::UpdateChangedDevicesInDB(TopologyReport::DevicesListT& topology)
{
	LOG_DEBUG_APP("[ProcessTopology] update devices=" << topology.size() << " in db: begin");
	try
	{
		int unregisteredDevices = 0;
		int registeredDevice = 0;

		//now update to db
		factoryDal.BeginTransaction();

		for (NodesRepository::NodesByMACT::iterator it = m_Repository.allNodesByMAC.begin(); it != m_Repository.allNodesByMAC.end(); it++)
		{
			if (it->second->Changed())
			{
				if (it->second->id == -1)
				{
					factoryDal.Devices().AddDevice(*it->second);
					
					LOG_INFO_APP("[ProcessTopology] discovered new registered Device=" << *(it->second) );

					if (!m_Repository.allNodesById.insert(NodesRepository::NodesByIDT::value_type(it->second->id, it->second)).second)
					{
						LOG_WARN_APP("[ProcessTopology] same DeviceID:" << it->second->id << "was detected (will be ignored)!");
					}
				}
				else
				{
					factoryDal.Devices().UpdateDevice(*it->second);
					if (it->second->IsRegistered())
					{
						LOG_INFO_APP("[ProcessTopology] DB updated for registered Device=" << *(it->second));
					}
					else
					{
						LOG_INFO_APP("[ProcessTopology] DB updated for non-registered Device=" << *(it->second));
					}
				}
			}

			if (it->second->IsRegistered())
			{

				registeredDevice++;
				if (!it->second->Changed())
				{
					LOG_INFO_APP("[ProcessTopology] DB not updated for registered Device=" << *(it->second));
				}
			}
			else
			{
				unregisteredDevices++;
				if (!it->second->Changed())
				{
					LOG_INFO_APP("[ProcessTopology] DB not updated for non-registered Device=" << *(it->second));
				}
			}
		}
		
		factoryDal.CommitTransaction();

		LOG_INFO_APP("[ProcessTopology] devicesCount=" << m_Repository.allNodesByMAC.size() << " RegisteredCount="
	    << registeredDevice << " UnregisteredCount=" << unregisteredDevices);

	}
	catch (std::exception& ex)
	{
		factoryDal.RollbackTransaction();
		LOG_ERROR_APP("[ProcessTopology] failed to updated device in db! error=" << ex.what());
		ResetDevicesChanges();
		throw;
	}
	catch (...)
	{
		factoryDal.RollbackTransaction();
		LOG_ERROR_APP("[ProcessTopology] failed to updated device in db! unknown error!");
		ResetDevicesChanges();
		throw;
	}
	LOG_DEBUG_APP("[ProcessTopology] update devices=" << topology.size() << " in db: end");
}

void DevicesManager::SaveTopologyInDB(DevicesGraph &devGraph, TopologyReport::DevicesListT& topology)
{
	LOG_DEBUG_APP("[ProcessTopology] Save topology in db: begin");

	try
	{
		factoryDal.BeginTransaction();

		int vertexNo = 0;
		for (TopologyReport::DevicesListT::const_iterator it = topology.begin(); it	!= topology.end(); it++)
		{
			DevicePtr fromDevice;
			fromDevice = devGraph.GetVertexData(vertexNo);
			ChangeDeviceGraphs(fromDevice, it->GraphsList);

			vertexNo++;
		}

		factoryDal.CommitTransaction();
	}
	catch (std::exception& ex)
	{
		factoryDal.RollbackTransaction();
		LOG_WARN_APP("[ProcessTopology] failed to insert graphs in db! error=" << ex.what());
	}
	catch (...)
	{
		factoryDal.RollbackTransaction();
		LOG_WARN_APP("[ProcessTopology] failed to insert graphs in db! unknown error!");
	}

	LOG_DEBUG_APP("[ProcessTopology] Save topology in db: end");
}

void DevicesManager::SaveDeviceNeighbours(DevicesGraph &devGraph, int vertexNo)
{

	DeviceNeighborsHealth neighbsHealth(devGraph.GetVertexData(vertexNo)->id);
	for (std::list<DevicesGraph::NeighbourData>::const_iterator itNeighbour = devGraph.GetNeighbours(vertexNo).begin(); itNeighbour
		!= devGraph.GetNeighbours(vertexNo).end(); itNeighbour++)
	{
		neighbsHealth.m_oNeighborsList.push_back(itNeighbour->health);
		NeighborHealth &health = *neighbsHealth.m_oNeighborsList.rbegin();
		health.m_nPeerID = devGraph.GetVertexData(itNeighbour->vertexNo)->id;
	}
	factoryDal.Devices().InsertDeviceNeighborsHealth(neighbsHealth);

}
void DevicesManager::ChangeDeviceGraphs(const DevicePtr& nodeFrom, const TopologyReport::Device::GraphsListT& graphs)
{
	for (TopologyReport::Device::GraphsListT::const_iterator itGraph = graphs.begin(); itGraph != graphs.end(); itGraph++)
	{
		for (int i = 0; i < (int)itGraph->neighbList.size(); i++)
		{
			DevicePtr nodeTo = m_Repository.Find(itGraph->neighbList[i]);
			if (nodeTo) //found
			{
				factoryDal.Devices().DeleteGraph(nodeFrom->id, nodeTo->id, itGraph->GraphID);
				factoryDal.Devices().CreateDeviceGraph(nodeFrom->id, nodeTo->id, itGraph->GraphID, i);
			}
			else
			{
				LOG_WARN_APP("saving Graphs: Unknown device graph reported=" << itGraph->neighbList[i]);
			}
		}
	}
}
void DevicesManager::ResetTopology()
{
	m_Repository.Clear();

	DeviceList devices;
	factoryDal.Devices().ResetDevices(Device::dsUnregistered);
	factoryDal.Devices().GetDevices(devices);

	for (DeviceList::iterator it = devices.begin(); it != devices.end(); it++)
	{
		DevicePtr node(new Device(*it));
		
		if (!m_Repository.Add(node))
		{
			LOG_ERROR_APP("[ResetTopology]: duplicated mac was found in db = " << node->Mac());
			continue;
		}
		
		if (node->Type() == Device::dtGateway)
			m_gwDevPtr = node;
		if (node->Type() == Device::dtSystemManager)
			m_smDevPtr = node;
	}

	m_Repository.UpdateRegisteredNickNameIndex();
	m_Repository.UpdateIdIndex();

	//just check that we have configurated a gateway
	GatewayDevice();
}
void DevicesManager::ProcessTopology(TopologyReport::DevicesListT& topology, int gwTopoIndex, bool doUnregDevs)
{
	//optimization, the case when no topology received
	static int noTopoCount = 0;
	if (topology.size() == 0)
	{
		noTopoCount++;
		noTopoCount = noTopoCount == 0 ? noTopoCount + 1 : noTopoCount;

		if (noTopoCount > 1)
			return;
	}
	
	//no more needed
	noTopoCount = 0;
	
	//process devices...
	DevicesGraph devGraph;
	{
		ResetDevicesChanges();
		bool doUpdateIPv6Index = false;
		if (doUnregDevs)
			UnregisterExistingDevices(topology, doUpdateIPv6Index);
		else
			LOG_WARN_APP("[ProcessTopology]: processing topology without unregistering devices;");

		
		ProcessRegisteredDevices(devGraph, topology, doUpdateIPv6Index);
		if (doUpdateIPv6Index)
		{
			m_Repository.UpdateNickNameIndex();
			m_Repository.UpdateRegisteredNickNameIndex();
		}
	}
	
	UpdateChangedDevicesInDB(topology);
	SaveTopologyInDB(devGraph, topology);
}


//general wh_cmd
void DevicesManager::InsertRawCommand(DevicePtr device, int cmdNo, std::string dataBuffer, int cmdID, int responseCode)
{
	factoryDal.Devices().AddCommand(device->id, cmdNo, dataBuffer, cmdID, responseCode);
}


//topology notification
static bool MySortPredicate(const MAC& mac1, const MAC& mac2)
{
  return mac1 < mac2;
}
void DevicesManager::UnregisterExistingDevices(std::vector<MAC> &macs/*[in/out]*/, int &unregisteredNo /*[out]*/)
{
	std::sort(macs.begin(), macs.end(), MySortPredicate);
	bool doUpdateIPv6Index = false;

	unregisteredNo = 0;
	LOG_DEBUG_APP("[TopologyNotification]: unregister_devices: begin");
	
	if (macs.size() == 0)
	{
		for(NodesRepository::NodesByNickNameT::const_iterator i = m_Repository.registeredNodesByNickName.begin(); i != m_Repository.registeredNodesByNickName.end(); ++i)
		{ 	//for every ip set unregister state
			LOG_WARN_APP("[TopologyNotification]: unregister_devices - dev_nn = " <<  i->first <<
		 				" was set unregistered");

			i->second->Status(Device::dsUnregistered);
			factoryDal.Devices().UpdateDevice(*i->second);
			doUpdateIPv6Index = true;
			unregisteredNo++;	
		}
	}
	else
	{
		std::vector<MAC>::const_iterator j = macs.begin();
	
		for(NodesRepository::NodesByMACT::const_iterator i = m_Repository.allNodesByMAC.begin(); i != m_Repository.allNodesByMAC.end(); ++i)
		{ //for every ip check if it is in topology_struct
			
			if (j == macs.end())
			{
				if (i->second->Status() != Device::dsUnregistered);
				{
					LOG_WARN_APP("[TopologyNotification]: unregister_devices - dev_mac = " <<  i->first <<
		 				" was set unregistered");
					i->second->Status(Device::dsUnregistered);
					factoryDal.Devices().UpdateDevice(*i->second);
					doUpdateIPv6Index = true;
					unregisteredNo++;
				}
				continue;
			}
			if (i->first == *j)
			{
				//found a match...
			}
			else 
			{
				if (*j < i->first)
				{
					do
					{
						++j;
						if (j == macs.end())
							break;
					}while (*j < i->first);
					if (j == macs.end())
					{
						if (i->second->Status() != Device::dsUnregistered);
						{
							LOG_WARN_APP("[TopologyNotification]: unregister_devices - dev_mac = " <<  i->first <<
			 					" was set unregistered");
							i->second->Status(Device::dsUnregistered);
							factoryDal.Devices().UpdateDevice(*i->second);
							doUpdateIPv6Index = true;
							unregisteredNo++;
						}
						continue; // continue 'for'
					}
					if (i->first < *j)
					{
						if (i->second->Status() != Device::dsUnregistered);
						{
							LOG_WARN_APP("[TopologyNotification]: unregister_devices - dev_mac = " <<  i->first <<
			 					" was set unregistered");
							i->second->Status(Device::dsUnregistered);
							factoryDal.Devices().UpdateDevice(*i->second);
							doUpdateIPv6Index = true;
							unregisteredNo++;
						}
						continue;
					}
					
					//found a match...
				}
				else 
				{
					if (i->second->Status() != Device::dsUnregistered);
					{
						LOG_WARN_APP("[TopologyNotification]: unregister_devices - dev_mac = " <<  i->first <<
			 				" was set unregistered");
						i->second->Status(Device::dsUnregistered);
						factoryDal.Devices().UpdateDevice(*i->second);
						doUpdateIPv6Index = true;
						unregisteredNo++;
					}
					continue;
				}

				//device has its status...
			}
			
			//increment here
			++j;
		}
	}

	LOG_DEBUG_APP("[TopologyNotification]: unregister_devices: end");

	if (doUpdateIPv6Index)
	{
		m_Repository.UpdateNickNameIndex();
		m_Repository.UpdateRegisteredNickNameIndex();
	}

}
void DevicesManager::AddNewDeviceInfo(NotificationDevInfo &devInfo)
{
	DevicePtr node = m_Repository.Find(devInfo.DeviceMAC);
	if (node)
	{
		LOG_WARN_APP("[NotificationDevInfo]: duplicated mac was found in db = " << devInfo.DeviceMAC);
		return;
	}
		
	node.reset(new Device());
	node->Mac(devInfo.DeviceMAC);
	
	node->Status(Device::dsRegistered);
	node->Nickname(devInfo.DeviceNickName);

	node->Type((Device::DeviceType) devInfo.DeviceType);
	node->SetTAG(devInfo.LongTag);

	node->SetSoftwareRevision(devInfo.softwareRev);
	node->SetDeviceCode(devInfo.deviceCode);

	node->SetNotificationBitMask(devInfo.notifBitMask);

	try
	{
		//now update to db
		factoryDal.BeginTransaction();
		factoryDal.Devices().AddDevice(*node);					
		LOG_INFO_APP("[NotificationDevInfo]: notification registered Device=" << *node);
		factoryDal.CommitTransaction();
	}
	catch (std::exception& ex)
	{
		factoryDal.RollbackTransaction();
		LOG_ERROR_APP("[NotificationDevInfo]: failed to updated device in db! error=" << ex.what());
		throw;
	}
	catch (...)
	{
		factoryDal.RollbackTransaction();
		LOG_ERROR_APP("[NotificationDevInfo]: failed to updated device in db! unknown error!");
		throw;
	}

	m_Repository.Add(node);
	if (!m_Repository.registeredNodesByNickName.insert(NodesRepository::NodesByNickNameT::value_type(devInfo.DeviceNickName, node)).second)
	{
		LOG_WARN_APP("[NotificationDevInfo]: Same NickName:" << devInfo.DeviceNickName << "was detected (will be ignored)!");
	}
	if (!m_Repository.allNodesById.insert(NodesRepository::NodesByIDT::value_type(node->id, node)).second)
	{
		LOG_WARN_APP("[NotificationDevInfo]: Same DeviceID:" << node->id << "was detected (will be ignored)!");
	}

	ReplaceNeighbours(node, devInfo.NeighborsList);

	//change this(it's not ok how it looks)---
	factoryDal.Devices().CleanDeviceGraphs(node->id);
	ChangeDeviceGraphs(node, devInfo.GraphsList);
	//---
}
void DevicesManager::AddNewDevice(DevicePtr device)
{
	const WHartUniqueID &devAddr = device->Mac().Address();
	const WHartUniqueID &gwAddr = stack::Gateway_UniqueID();
	const WHartUniqueID &nnAddr = stack::NetworkManager_UniqueID();
	if (!memcmp(devAddr.bytes, gwAddr.bytes, sizeof(devAddr.bytes)))
		device->Type(Device::dtGateway);
	if (!memcmp(devAddr.bytes, nnAddr.bytes, sizeof(devAddr.bytes)))
		device->Type(Device::dtSystemManager);


	try
	{
		//now update to db
		factoryDal.BeginTransaction();
		factoryDal.Devices().AddDevice(*device);					
		LOG_INFO_APP("[Add New Device]: new registered Device=" << *device);
		factoryDal.CommitTransaction();
	}
	catch (std::exception& ex)
	{
		factoryDal.RollbackTransaction();
		LOG_ERROR_APP("[Add New Device]: failed to updated device in db! error=" << ex.what());
		throw;
	}
	catch (...)
	{
		factoryDal.RollbackTransaction();
		LOG_ERROR_APP("[Add New Device]: failed to updated device in db! unknown error!");
		throw;
	}

	m_Repository.Add(device);
	if (device->IsRegistered())
	{
		if (!m_Repository.registeredNodesByNickName.insert(NodesRepository::NodesByNickNameT::value_type(device->Nickname(), device)).second)
		{
			LOG_WARN_APP("[Add New Device]: Same NickName:" << device->Nickname() << "was detected (will be ignored)!");
		}
	}
	if (!m_Repository.allNodesById.insert(NodesRepository::NodesByIDT::value_type(device->id, device)).second)
	{
		LOG_WARN_APP("[Add New Device]: Same DeviceID:" << device->id << "was detected (will be ignored)!");
	}
}
void DevicesManager::RegisterDevice(DevicePtr device)
{
	if (device->IsRegistered())
	{
		LOG_WARN_APP("[RegisterDevice]: device already registered");
		return;
	}
	device->Status(Device::dsRegistered);

	const WHartUniqueID &devAddr = device->Mac().Address();
	const WHartUniqueID &gwAddr = stack::Gateway_UniqueID();
	const WHartUniqueID &nnAddr = stack::NetworkManager_UniqueID();
	if (!memcmp(devAddr.bytes, gwAddr.bytes, sizeof(devAddr.bytes)))
		device->Type(Device::dtGateway);
	if (!memcmp(devAddr.bytes, nnAddr.bytes, sizeof(devAddr.bytes)))
		device->Type(Device::dtSystemManager);

	
	if (!m_Repository.registeredNodesByNickName.insert(NodesRepository::NodesByNickNameT::value_type(device->Nickname(), device)).second)
	{
		LOG_WARN_APP("[RegisterDevice]: Same NickName:" << device->Nickname() << "was detected (will be ignored)!");
	}
	factoryDal.Devices().UpdateDevice(*device);
}
void DevicesManager::UpdateDevice(DevicePtr device, bool nicknameChanged, bool statusChanged)
{
	factoryDal.Devices().UpdateDevice(*device);
	if (nicknameChanged)
	{
		m_Repository.UpdateNickNameIndex();
		m_Repository.UpdateRegisteredNickNameIndex();
	}
	if (statusChanged)
	{
		m_Repository.UpdateRegisteredNickNameIndex();
	}

	LOG_INFO_APP("[UpdateDevice]: updated device, mac=" << device->Mac());
}
bool DevicesManager::IsDeviceInfoInProgress(MAC &mac)
{
	std::set<MAC>::iterator i;
	if ((i = m_devInfoInProgress.find(mac)) == m_devInfoInProgress.end())
	{
		LOG_INFO_APP("[NotificationDevInfo]: device not in get_info_in_progres_list, mac=" << mac);
		return false;
	}
	LOG_INFO_APP("[NotificationDevInfo]: already device in get_info_in_progres_list, mac=" << mac);
	return true;
}
void DevicesManager::AddDeviceInfoInProgress(MAC &mac)
{
	if (m_devInfoInProgress.insert(std::set<MAC>::value_type(mac)).second == false)
	{
		LOG_WARN_APP("[NotificationDevInfo]: already device in get_info_in_progres_list, mac=" << mac);
		return;
	}
	LOG_INFO_APP("[NotificationDevInfo]: added device in get_info_in_progres_list, mac=" << mac);
}
void DevicesManager::DelDeviceInfoInProgress(MAC &mac)
{
	std::set<MAC>::iterator i;
	if ((i = m_devInfoInProgress.find(mac)) == m_devInfoInProgress.end())
	{
		LOG_WARN_APP("[NotificationDevInfo]: device not in get_info_in_progres_list, mac=" << mac);
		return;
	}
	m_devInfoInProgress.erase(i);
	LOG_INFO_APP("[NotificationDevInfo]: deleted device in get_info_in_progres_list, mac=" << mac);
}
void DevicesManager::ReplaceNeighbours(DevicePtr fromDev, NotificationDevInfo::NeighborsListT &list)
{

	std::list<NeighbourSignalLevel> devNeighbSignalLevels;
	DeviceNeighborsHealth deviceNeighborsHealth(fromDev->id,fromDev->Mac());


	for (int i = 0; i < (int)list.size(); ++i)
	{
		DevicePtr toDev = m_Repository.Find(list[i].DeviceNickName);
		if (toDev == NULL)
		{
			LOG_WARN_APP("[NotificationDevInfo]: neighbours-> nickname was not found in db = " << list[i].DeviceNickName);
			continue;
		}
		
		if(list[i].Health.m_nTransmissions == 0 && list[i].Health.m_nFailedTransmissions == 0)
		{
			devNeighbSignalLevels.push_back(NeighbourSignalLevel(toDev->id, list[i].Health.m_nRSL));
		}else
		{
			//devNeighborsHealth
			deviceNeighborsHealth.m_oNeighborsList.push_back(NeighborHealth(toDev->id,hart7::hostapp::NeighborHealth::g_nNoClockSource, 
				list[i].Health.m_nRSL, list[i].Health.m_nTransmissions, list[i].Health.m_nFailedTransmissions, list[i].Health.m_nReceptions));
		}
	}

	factoryDal.Devices().CleanDeviceNeighborsHealth(fromDev->id);
	factoryDal.Devices().CleanDevNeighbSignalLevels(fromDev->id);

	for (std::list<NeighbourSignalLevel>::const_iterator it = devNeighbSignalLevels.begin(); 
		it != devNeighbSignalLevels.end(); ++it)
		factoryDal.Devices().InsertDevNeighbSignalLevels(fromDev->id, it->neighDevID, it->signalLevel);
	
	factoryDal.Devices().InsertDeviceNeighborsHealth(deviceNeighborsHealth);

}



//devices
DevicePtr DevicesManager::FindDevice(boost::int32_t deviceID) const
{
	return m_Repository.Find(deviceID);
}
DevicePtr DevicesManager::FindDevice(const MAC& deviceMAC) const
{
	return m_Repository.Find(deviceMAC);
}
DevicePtr DevicesManager::FindRegisteredDevice(const NickName& nickName) const
{
	return m_Repository.FindRegistered(nickName);
}
DevicePtr DevicesManager::FindDevice(const NickName& nickName) const
{
	return m_Repository.Find(nickName);
}
const DevicePtr DevicesManager::GatewayDevice() const
{
	if (m_gwDevPtr)
		return m_gwDevPtr;
	THROW_EXCEPTION0(NoGatewayException);
}
const DevicePtr DevicesManager::SystemManagerDevice() const
{
	return m_smDevPtr;
}


//readings
void DevicesManager::UpdatePublishFlag(int devID, Device::PublishStatus status)
{
    DevicePtr dev = m_Repository.Find(devID);
    if (dev) dev->SetPublishStatus(status);

	factoryDal.Devices().UpdatePublishFlag(devID, status);
}

void DevicesManager::UpdateReadingTimeforDevice(int deviceID, const struct timeval &tv, Device::PublishStatus status)
{
    DevicePtr dev = m_Repository.Find(deviceID);
    if (dev) dev->SetPublishStatus(status);

	factoryDal.Devices().UpdateReadingTimeforDevice(deviceID, tv, status);
}

void DevicesManager::ProcessDBChannels(int deviceID, PublishChannelSetT &storedList)
{
	std::vector<PublishChannel> channels;
	factoryDal.Devices().GetOrderedChannels(deviceID, channels);
	ProcessChannelsDifference(deviceID, storedList, channels);
}

void DevicesManager::ProcessReading(const DeviceReading& p_rReading)
{
	if (m_nReadingsSavePeriod == 0)
	{
		factoryDal.Devices().UpdateReading(p_rReading);
		if (p_rReading.m_nCommandID == -1) /*burst*/
		{
			UpdateReadingTimeforDevice(p_rReading.m_deviceID, p_rReading.m_tv, Device::PS_FRESH_DATA);
		}
	}
	else
	{
		CacheReading(p_rReading);
	}
}

void DevicesManager::ProcessReadingsTransaction(const DeviceReadingsMapT& p_oReadingsCache)
{
	try
	{
		factoryDal.BeginTransaction();
		for (DeviceReadingsMapT::const_iterator it = p_oReadingsCache.begin(); it != p_oReadingsCache.end(); ++it)
		{
			factoryDal.Devices().UpdateReading(it->second);
			if (it->second.m_nCommandID == -1) /*burst*/
			{
				UpdateReadingTimeforDevice(it->second.m_deviceID, it->second.m_tv, Device::PS_FRESH_DATA);
			}
		}
		factoryDal.CommitTransaction();
	}
	catch (std::exception& ex)
	{
		factoryDal.RollbackTransaction();
		throw;
	}
    catch (...)
    {
        factoryDal.RollbackTransaction();
        throw;
    }
}

void DevicesManager::ProcessChannelsDifference(int deviceID, PublishChannelSetT &storedList, 
	const std::vector<PublishChannel> &channels)
{
	PublishChannelSetT::iterator i = storedList.begin();

	std::vector<PublishChannel>::const_iterator j = channels.begin();

	LOG_DEBUG_APP("[ChannelsProcessing]: channels size = " << channels.size() << " for deviceID=" << deviceID);

	try
	{
		factoryDal.BeginTransaction();

		for (; i != storedList.end(); ++i)
		{

			if (j == channels.end())
			{
				//create channel
				factoryDal.Devices().CreateChannel(deviceID, *i);
				factoryDal.Devices().AddEmptyReading(deviceID, i->channelID);
				LOG_DEBUG_APP("[ChannelsProcessing]: create -> i: CmdNo= " << i->cmdNo << " burstMessage = " << (int)i->burstMessage << " deviceVariableSlot = " << i->deviceVariableSlot);
				continue;
			}
			
			LOG_DEBUG_APP("[ChannelsProcessing]: i: CmdNo= " << i->cmdNo << " burstMessage = " << (int)i->burstMessage << " deviceVariableSlot = " << (int)i->deviceVariableSlot);
			LOG_DEBUG_APP("[ChannelsProcessing]: j: CmdNo= " << j->cmdNo << " burstMessage = " << (int)j->burstMessage << " deviceVariableSlot = " << (int)j->deviceVariableSlot);
			
			if ( (*i) == (*j) )
			{
				//found a match...
			}
			else
			{
				if ( (*j) < (*i) )
				{
					do
					{
						//move channel
						factoryDal.Devices().MoveChannelToHistory(j->channelID);
						factoryDal.Devices().DeleteReading(j->channelID);
						LOG_DEBUG_APP("[ChannelsProcessing]: delete -> j: CmdNo= " << j->cmdNo << " burstMessage = " << (int)j->burstMessage << " deviceVariableSlot = " << j->deviceVariableSlot);
						++j;
						if (j == channels.end())
							break;
					} while ( (*j) < (*i) );

					if (j == channels.end())
					{
						//create chanel
						factoryDal.Devices().CreateChannel(deviceID, *i);
						factoryDal.Devices().AddEmptyReading(deviceID, i->channelID);
						LOG_DEBUG_APP("[ChannelsProcessing]: create -> i: CmdNo= " << i->cmdNo << " burstMessage = " << (int)i->burstMessage << " deviceVariableSlot = " << i->deviceVariableSlot);
						continue;
					}
					if ( (*i) < (*j) )
					{
						//create channel
						factoryDal.Devices().CreateChannel(deviceID, *i);
						factoryDal.Devices().AddEmptyReading(deviceID, i->channelID);
						LOG_DEBUG_APP("[ChannelsProcessing]: create -> i: CmdNo= " << i->cmdNo << " burstMessage = " << (int)i->burstMessage << " deviceVariableSlot = " << i->deviceVariableSlot);
						continue;
					}

					//found a match...
				}
				else
				{
					//create channel
					factoryDal.Devices().CreateChannel(deviceID, *i);
					factoryDal.Devices().AddEmptyReading(deviceID, i->channelID);
					LOG_DEBUG_APP("[ChannelsProcessing]: create -> i: CmdNo= " << i->cmdNo << " burstMessage = " << (int)i->burstMessage << " deviceVariableSlot = " << i->deviceVariableSlot);
					continue;
				}
			}

			//
			i->channelID = j->channelID;

			//check channel info
			if (i->deviceVariable != j->deviceVariable ||
				i->unitCode != j->unitCode ||
				i->classification != j->classification || 
				i->name != j->name)
			{
				LOG_DEBUG_APP("[ChannelsProcessing]: update -> "
					<< "i: CmdNo= " << i->cmdNo << " burstMessage = " << (int)i->burstMessage << " deviceVariable = " << i->deviceVariable << " unitCode =" << (int)(i->unitCode) 
					<< "j: CmdNo= " << j->cmdNo << " burstMessage = " << (int)j->burstMessage << "deviceVariable = " << j->deviceVariable << " unitCode =" << (int)j->unitCode);
				//update channel
				factoryDal.Devices().UpdateChannel((int)j->channelID, (int)i->deviceVariable, (int)i->classification, (int)i->unitCode, i->name);
			}

			++j;
		}
		
		while (j != channels.end())
		{
			LOG_DEBUG_APP("[ChannelsProcessing]: delete -> j: CmdNo= " << j->cmdNo << " burstMessage = " << (int)j->burstMessage << "deviceVariable= " << j->deviceVariable << " unitCode = " << j->unitCode);
								
			factoryDal.Devices().MoveChannelToHistory(j->channelID);
			factoryDal.Devices().DeleteReading(j->channelID);
			++j;
		}

		factoryDal.CommitTransaction();
		LOG_INFO_APP("[ChannelsProcessing]: processing done ok " << "for deviceID=" << deviceID);
	}
	catch (std::exception& ex)
	{
		LOG_ERROR_APP("[ChannelsProcessing]: processing failed! error=" << ex.what() << "for deviceID=" << deviceID);
		factoryDal.RollbackTransaction();
	}
	catch (...)
	{
		LOG_ERROR_APP("[ChannelsProcessing]: processing failed! unknown error!" << "for deviceID=" << deviceID);
		factoryDal.RollbackTransaction();
	}
}


// callbacks
void DevicesManager::SetBurstCounterTaskPeriodCallback(BurstCounterTaskPeriodCallback p_oCallback)
{
	m_oHandleBurstCounterTaskPeriod = p_oCallback;
}
void DevicesManager::UpdateBurstCounterTaskPeriod(int p_nUpdatePeriod/*ms*/)
{
	if (m_oHandleBurstCounterTaskPeriod)
	{
		m_oHandleBurstCounterTaskPeriod(p_nUpdatePeriod);
		return;
	}
	LOG_ERROR_APP("---> NO_HANDLE_DEFINED for UpdateBurstCounterTaskPeriod");
}
void DevicesManager::SetPubNotificationCallback(PubNotificationReceiveCallback p_oCallback)
{
	m_oHandlePubNotificationRecv = p_oCallback;
}
void DevicesManager::PubNotificationReceived(int p_nDeviceID, const CMicroSec& p_rRecvTime, C839_ChangeNotification_Resp& p_rData)
{
	if (m_oHandlePubNotificationRecv)
	{
		m_oHandlePubNotificationRecv(p_nDeviceID, p_rRecvTime, p_rData);
		return;
	}
	LOG_ERROR_APP("---> NO_HANDLE_DEFINED for PubNotificationReceived");
}
void DevicesManager::SetPubDeviceDeletionCallback(PubDeviceDeletionCallback p_oCallback)
{
	m_oHandlePubDeviceDeletion = p_oCallback;
}

void DevicesManager::PubDeviceDeletionReceived(int p_nDeviceId)
{
	if (m_oHandlePubDeviceDeletion)
	{	m_oHandlePubDeviceDeletion(p_nDeviceId);
		return;
	}
	LOG_ERROR_APP("---> NO_HANDLE_DEFINED for PubDeviceDeletionReceived");
}
void DevicesManager::SetPubDeviceBurstsChangeCallback(PubDeviceBurstsChangeCallback p_oCallback)
{
	m_oHandlePubDeviceBurstsChange = p_oCallback;
}
void DevicesManager::PubDeviceBurstsChangeReceived(int p_nDeviceId,
	std::list<BurstMessage> &p_oAddedBursts,
	std::list<BurstMessage> &p_oDeletedBursts,
	std::list<BurstMessage> &p_oChangedBursts)
{
	if (m_oHandlePubDeviceBurstsChange)
	{	m_oHandlePubDeviceBurstsChange(p_nDeviceId, p_oAddedBursts, p_oDeletedBursts, p_oChangedBursts);
		return;
	}
	LOG_ERROR_APP("---> NO_HANDLE_DEFINED for PubDeviceBurstsChangeReceived");
}

void DevicesManager::SetUpdatePublishersCacheCallback(UpdatePublishersCacheCallback p_oUpdatePublishersCache)
{
	m_oUpdatePublishersCache = p_oUpdatePublishersCache;
}

void DevicesManager::UpdatePublishersCache(MAC& p_rMac, PublisherInfo& p_rPublisherInfo)
{
	if (m_oUpdatePublishersCache)
	{	m_oUpdatePublishersCache(p_rMac, p_rPublisherInfo);
		return;
	}

	LOG_ERROR_APP("---> NO_HANDLE_DEFINED for UpdatePublishersCache");
}

void DevicesManager::SetUpdateStoredPublishersCallback(UpdateStoredPublishersCallback p_oCallback)
{
    m_oUpdateStoredPublishers = p_oCallback;
}

void DevicesManager::UpdateStoredPublishers(MAC& p_rMac, PublisherInfo& p_rPublisherInfo, int p_nIssueCase)
{
   if (m_oUpdateStoredPublishers)
   {   m_oUpdateStoredPublishers(p_rMac, p_rPublisherInfo, p_nIssueCase);
       return;
   }
   LOG_ERROR_APP("---> NO_HANDLE_DEFINED for UpdateStoredPublishers");
}

void DevicesManager::SetUpdatePublisherStateCallback(UpdatePublisherStateCallback p_oCallback)
{
    m_oUpdatePublisherState = p_oCallback;
}

void DevicesManager::UpdatePublisherState(const MAC& p_rMac, int p_nBurst, SetPublisherState p_eState, SetPublisherError p_eError, std::string p_sMessage)
{
   if (m_oUpdatePublisherState)
   {   m_oUpdatePublisherState(p_rMac, p_nBurst, p_eState, p_eError, p_sMessage);
       return;
   }
   LOG_ERROR_APP("---> NO_HANDLE_DEFINED for UpdatePublisherState");
}

void DevicesManager::SetIssueTopologyCommandCallback(IssueCommandCallback p_oCallback)
{
    m_oIssueTopologyCommand = p_oCallback;
}

void DevicesManager::IssueTopologyCommand()
{
   if (m_oIssueTopologyCommand)
   {
	   m_oIssueTopologyCommand();
       return;
   }
   LOG_ERROR_APP("---> NO_HANDLE_DEFINED for IssueTopologyCommand");
}

void DevicesManager::SetCacheReadingCallback(CacheReadingCallback p_oCallback)
{
	m_oCacheReading = p_oCallback;
}

void DevicesManager::CacheReading(const DeviceReading& p_oReading)
{
   if (m_oCacheReading)
   {
	   m_oCacheReading(p_oReading);
	   return;
   }
   LOG_ERROR_APP("---> NO_HANDLE_DEFINED for CacheReading");
}

// bursts
void DevicesManager::BeginUpdateBurstMessageCounter()
{
	factoryDal.BeginTransaction();
}

void DevicesManager::UpdateBurstMessage(int p_nDeviceId, BurstMessage& p_rBurstMessage)
{
	factoryDal.Devices().UpdateBurstMessage(p_nDeviceId, p_rBurstMessage);
}

void DevicesManager::UpdateBurstMessageCounter(int p_nDeviceId, BurstMessage& p_rBurstMessage, const CMicroSec&  p_rLastUpdate, int p_nReceived, int p_nMissed)
{
	factoryDal.Devices().UpdateBurstMessageCounter(p_nDeviceId, p_rBurstMessage, p_rLastUpdate, p_nReceived, p_nMissed);
}
void DevicesManager::EndUpdateBurstMessageCounter(bool commit)
{
	if (commit)
	{
		factoryDal.CommitTransaction();
		return;
	}
	factoryDal.RollbackTransaction();
}

void DevicesManager::RemoveBursts(int deviceID)
{
	try
	{
		factoryDal.BeginTransaction();
		
		factoryDal.Devices().DeleteBurstMessageCounters(deviceID);
		factoryDal.Devices().DeleteBurstMessages(deviceID);
	
		factoryDal.CommitTransaction();
		LOG_INFO_APP("[BurstProcessing]: deleting bursts done ok " << "for deviceID=" << deviceID);
	}
	catch (std::exception& ex)
	{
		LOG_ERROR_APP("[BurstProcessing]: deleting bursts failed! error=" << ex.what() << "for deviceID=" << deviceID);
		factoryDal.RollbackTransaction();
	}
	catch (...)
	{
		LOG_ERROR_APP("[BurstProcessing]: deleting bursts failed! unknown error!" << "for deviceID=" << deviceID);
		factoryDal.RollbackTransaction();
	}

}
void DevicesManager::ProcessDBBursts(int deviceID, BurstMessageSetT &storedList, std::list<BurstMessage> &added/*[out]*/,
		std::list<BurstMessage> &updated/*[out]*/, std::list<BurstMessage> &deleted/*[out]*/)
{
	std::vector<BurstMessage> bursts;
	factoryDal.Devices().GetOrderedBursts(deviceID, bursts);
	ProcessBurstDifference(deviceID, storedList, bursts, added, updated, deleted);
}
void DevicesManager::ProcessDBTriggers(int deviceID, TriggerSetT &storedList)
{
	std::vector<Trigger> triggers;
	factoryDal.Devices().GetOrderedTriggers(deviceID, triggers);
	ProcessTriggersDifference(deviceID, storedList, triggers);
}
void DevicesManager::ProcessBurstDifference(int deviceID, BurstMessageSetT &storedList, 
											const std::vector<BurstMessage> &bursts, std::list<BurstMessage> &added/*[out]*/,
		std::list<BurstMessage> &updated/*[out]*/, std::list<BurstMessage> &deleted/*[out]*/)
{
	BurstMessageSetT::iterator i = storedList.begin();

	std::vector<BurstMessage>::const_iterator j = bursts.begin();

	LOG_DEBUG_APP("[BurstProcessing]: bursts size = " << bursts.size() << " for deviceID=" << deviceID);

	try
	{
		factoryDal.BeginTransaction();

		for (; i != storedList.end(); ++i)
		{

			if (j == bursts.end())
			{
				//create burst
				factoryDal.Devices().CreateBurstMessage(deviceID, *i);
				factoryDal.Devices().CreateBurstMessageCounter(deviceID, *i);
				added.push_back(*i);
				LOG_DEBUG_APP("[BurstProcessing]: create -> i: CmdNo= " << i->cmdNo << " burstMessage = " << (int)i->burstMessage);
				continue;
			}
			
			LOG_DEBUG_APP("[BurstProcessing]: i: CmdNo= " << i->cmdNo << " burstMessage = " << (int)i->burstMessage);
			LOG_DEBUG_APP("[BurstProcessing]: j: CmdNo= " << j->cmdNo << " burstMessage = " << (int)j->burstMessage);
			
			if ( (*i) == (*j) )
			{
				//found a match...
			}
			else
			{
				if ( (*j) < (*i) )
				{
					do
					{
						//delete burst
						factoryDal.Devices().DeleteBurstMessage(deviceID, j->burstMessage, j->cmdNo);
						factoryDal.Devices().DeleteBurstMessageCounter(deviceID, j->burstMessage, j->cmdNo);
						deleted.push_back(*j);
						LOG_DEBUG_APP("[BurstProcessing]: delete -> j: CmdNo= " << j->cmdNo << " burstMessage = " << (int)j->burstMessage);
						++j;
						if (j == bursts.end())
							break;
					} while ( (*j) < (*i) );

					if (j == bursts.end())
					{
						//create burst
						factoryDal.Devices().CreateBurstMessage(deviceID, *i);
						factoryDal.Devices().CreateBurstMessageCounter(deviceID, *i);
						added.push_back(*i);
						LOG_DEBUG_APP("[BurstProcessing]: create -> i: CmdNo= " << i->cmdNo << " burstMessage = " << (int)i->burstMessage);
						continue;
					}
					if ( (*i) < (*j) )
					{
						//create burst
						factoryDal.Devices().CreateBurstMessage(deviceID, *i);
						factoryDal.Devices().CreateBurstMessageCounter(deviceID, *i);
						added.push_back(*i);
						LOG_DEBUG_APP("[BurstProcessing]: create -> i: CmdNo= " << i->cmdNo << " burstMessage = " << (int)i->burstMessage);
						continue;
					}

					//found a match...
				}
				else
				{
					//create burst
					factoryDal.Devices().CreateBurstMessage(deviceID, *i);
					factoryDal.Devices().CreateBurstMessageCounter(deviceID, *i);
					added.push_back(*i);
					LOG_DEBUG_APP("[BurstProcessing]: create -> i: CmdNo= " << i->cmdNo << " burstMessage = " << (int)i->burstMessage);
					continue;
				}
			}

			if (i->updatePeriod != j->updatePeriod ||
				i->maxUpdatePeriod != j->maxUpdatePeriod)
			{
				//check burst info
				LOG_DEBUG_APP("[BurstProcessing]: update -> "
					<< "i: CmdNo= " << i->cmdNo << " burstMessage = " << (int)i->burstMessage << " updatePeriod = " << i->updatePeriod << " maxUpdatePeriod =" << (int)(i->maxUpdatePeriod) 
					<< "j: CmdNo= " << j->cmdNo << " burstMessage = " << (int)j->burstMessage << " updatePeriod = " << j->updatePeriod << " maxUpdatePeriod =" << (int)j->maxUpdatePeriod);
				//update burst
				factoryDal.Devices().UpdateBurstMessage(deviceID, *i);
				updated.push_back(*i);
			}

			++j;
		}
		
		while (j != bursts.end())
		{
			LOG_DEBUG_APP("[BurstProcessing]: delete -> j: CmdNo= " << j->cmdNo << " burstMessage = " << (int)j->burstMessage << "updatePeriod= " << j->updatePeriod << " maxUpdatePeriod = " << j->maxUpdatePeriod);					
			factoryDal.Devices().DeleteBurstMessage(deviceID, j->burstMessage, j->cmdNo);
			factoryDal.Devices().DeleteBurstMessageCounter(deviceID, j->burstMessage, j->cmdNo);
			deleted.push_back(*j);
			++j;
		}

		factoryDal.CommitTransaction();
		LOG_INFO_APP("[BurstProcessing]: processing done ok " << "for deviceID=" << deviceID);
	}
	catch (std::exception& ex)
	{
		LOG_ERROR_APP("[BurstProcessing]: processing failed! error=" << ex.what() << "for deviceID=" << deviceID);
		factoryDal.RollbackTransaction();
	}
	catch (...)
	{
		LOG_ERROR_APP("[BurstProcessing]: processing failed! unknown error!" << "for deviceID=" << deviceID);
		factoryDal.RollbackTransaction();
	}
}
void DevicesManager::ProcessTriggersDifference(int deviceID, TriggerSetT &storedList, 
											   const std::vector<Trigger> &triggers)
{
	TriggerSetT::iterator i = storedList.begin();

	std::vector<Trigger>::const_iterator j = triggers.begin();

	LOG_DEBUG_APP("[TriggersProcessing]: triggers size = " << triggers.size() << " for deviceID=" << deviceID);

	try
	{
		factoryDal.BeginTransaction();

		for (; i != storedList.end(); ++i)
		{

			if (j == triggers.end())
			{
				//create trigger
				factoryDal.Devices().CreateTrigger(deviceID, *i);
				LOG_DEBUG_APP("[TriggersProcessing]: create -> i: CmdNo= " << i->cmdNo << " burstMessage = " << (int)i->burstMessage);
				continue;
			}

			LOG_DEBUG_APP("[TriggersProcessing]: i: CmdNo= " << i->cmdNo << " burstMessage = " << (int)i->burstMessage);
			LOG_DEBUG_APP("[TriggersProcessing]: j: CmdNo= " << j->cmdNo << " burstMessage = " << (int)j->burstMessage);

			if ( (*i) == (*j) )
			{
				//found a match...
			}
			else
			{
				if ( (*j) < (*i) )
				{
					do
					{
						//delete trigger
						factoryDal.Devices().DeleteTrigger(deviceID, j->burstMessage, j->cmdNo);
						LOG_DEBUG_APP("[TriggersProcessing]: delete -> j: CmdNo= " << j->cmdNo << " burstMessage = " << (int)j->burstMessage);
						++j;
						
						if (j == triggers.end())
							break;
					} while ( (*j) < (*i) );

					if (j == triggers.end())
					{
						//create trigger
						factoryDal.Devices().CreateTrigger(deviceID, *i);
						LOG_DEBUG_APP("[TriggersProcessing]: create -> i: CmdNo= " << i->cmdNo << " burstMessage = " << (int)i->burstMessage);
						continue;
					}
					if ( (*i) < (*j) )
					{
						//create trigger
						factoryDal.Devices().CreateTrigger(deviceID, *i);
						LOG_DEBUG_APP("[TriggersProcessing]: create -> i: CmdNo= " << i->cmdNo << " burstMessage = " << (int)i->burstMessage);
						continue;
					}

					//found a match...
				}
				else
				{
					//create trigger
					factoryDal.Devices().CreateTrigger(deviceID, *i);
					LOG_DEBUG_APP("[TriggersProcessing]: create -> i: CmdNo= " << i->cmdNo << " burstMessage = " << (int)i->burstMessage);
					continue;
				}
			}

			//check trigger info
			if (i->modeSelection != j->modeSelection ||
				i->classification != j->classification ||
				i->unitCode != j->unitCode ||
				i->triggerLevel != j->triggerLevel)
			{
				LOG_DEBUG_APP("[TriggersProcessing]: update -> "
					<< "i: CmdNo= " << i->cmdNo << " burstMessage = " << (int)i->burstMessage << " modeSelection = " << (int)i->modeSelection << " classification =" << (int)i->classification << " unitCode = " << (int)i->unitCode << " triggerLevel =" << (int)(i->triggerLevel) 
					<< "j: CmdNo= " << j->cmdNo << " burstMessage = " << (int)j->burstMessage << " modeSelection = " << (int)j->modeSelection << " classification =" << (int)j->classification << " unitCode = " << (int)j->unitCode << " triggerLevel =" << (int)j->triggerLevel);
				//update trigger
				factoryDal.Devices().UpdateTrigger(deviceID, *i);
			}

			++j;
		}

		while (j != triggers.end())
		{
			LOG_DEBUG_APP("[TriggersProcessing]: delete -> j: CmdNo= " << j->cmdNo << " burstMessage = " << (int)j->burstMessage);					
			factoryDal.Devices().DeleteTrigger(deviceID, j->burstMessage, j->cmdNo);
			++j;
		}

		factoryDal.CommitTransaction();
		LOG_INFO_APP("[TriggersProcessing]: processing done ok " << "for deviceID=" << deviceID);
	}
	catch (std::exception& ex)
	{
		LOG_ERROR_APP("[TriggersProcessing]: processing failed! error=" << ex.what() << "for deviceID=" << deviceID);
		factoryDal.RollbackTransaction();
	}
	catch (...)
	{
		LOG_ERROR_APP("[TriggersProcessing]: processing failed! unknown error!" << "for deviceID=" << deviceID);
		factoryDal.RollbackTransaction();
	}
}



bool DevicesManager::CleanReports(int p_nDevID)
{
	try{
		LOG_INFO_APP("[CleanReports]: Clean reports for deviceID="<<p_nDevID);
		factoryDal.BeginTransaction();
		factoryDal.Devices().CleanDeviceScheduleLinks(p_nDevID);
		factoryDal.Devices().CleanDeviceGraphs(p_nDevID);
		factoryDal.Devices().CleanDeviceHealthReport(p_nDevID);
		factoryDal.Devices().CleanDeviceNeighborsHealth(p_nDevID);
		factoryDal.Devices().CleanDevNeighbSignalLevels(p_nDevID);
		factoryDal.Devices().CleanDeviceRoutes(p_nDevID);
		factoryDal.Devices().CleanDeviceServices(p_nDevID);
		factoryDal.Devices().CleanDeviceSourceRoutes(p_nDevID);
		factoryDal.Devices().CleanDeviceSuperframes(p_nDevID);
		factoryDal.CommitTransaction();
		return true;
	}
	catch (std::exception& ex)
	{
		LOG_ERROR_APP("[CleanReports]: processing failed! error=" << ex.what() << "for deviceID=" << p_nDevID);
		factoryDal.RollbackTransaction();
		return false;
	}
	catch (...)
	{
		LOG_ERROR_APP("[CleanReports]: processing failed! unknown error!" << "for deviceID=" << p_nDevID);
		factoryDal.RollbackTransaction();
		return false;
	}
}
bool DevicesManager::CleanReports()
{
	try{
		LOG_INFO_APP("[CleanReports]: Clean reports for all devices in db.");
		factoryDal.BeginTransaction();
		factoryDal.Devices().CleanDeviceScheduleLinks();
		factoryDal.Devices().CleanDevicesGraphs();
		factoryDal.Devices().CleanDeviceRoutes();
		factoryDal.Devices().CleanDeviceServices();
		factoryDal.Devices().CleanDeviceSourceRoutes();
		factoryDal.Devices().CleanDeviceSuperframes();
		factoryDal.CommitTransaction();
		return true;
	}
	catch (std::exception& ex)
	{
		LOG_ERROR_APP("[CleanReports]: processing failed! error=" << ex.what());
		factoryDal.RollbackTransaction();
		return false;
	}
	catch (...)
	{
		LOG_ERROR_APP("[CleanReports]: processing failed! unknown error!" );
		factoryDal.RollbackTransaction();
		return false;
	}
}


//graphs
void DevicesManager::ReplaceDeviceGraphs(int deviceId, std::list<GraphNeighbor>& p_rGraphList)
{
	if (p_rGraphList.size() == 0)
	{
		LOG_WARN_APP("[ReplaceDeviceGraphs]: Graph list with 0 elements received for deviceID=" << deviceId);
		return;
	}
	int graphId=0;
	try
	{
		factoryDal.BeginTransaction();

		factoryDal.Devices().CleanDeviceGraphs(deviceId);
		std::list<GraphNeighbor>::const_iterator it = p_rGraphList.begin();
		for (; it != p_rGraphList.end(); ++it)
		{
			graphId = it->m_graphId;
			factoryDal.Devices().CreateDeviceGraph(deviceId, it->m_toDevId, it->m_graphId, it->m_index);
		}

		factoryDal.CommitTransaction();
	}
	catch (std::exception& ex)
	{
		LOG_ERROR_APP("[ReplaceDeviceGraphs]: context: processing failed! error=" << ex.what() << "for deviceID=" << deviceId
			<<" graphID="<<graphId);
		std::list<GraphNeighbor>::const_iterator it = p_rGraphList.begin();
		for (; it != p_rGraphList.end(); ++it)
		{
			LOG_ERROR_APP("[ReplaceDeviceGraphs]: detail: processing failed! error=" << ex.what() << "for deviceID=" << deviceId
				<<" graphID="<< it->m_graphId << " neighbID=" << it->m_toDevId << " index=" << it->m_index);
		}
		factoryDal.RollbackTransaction();
	}
	catch (...)
	{
		LOG_ERROR_APP("[ReplaceDeviceGraphs]: context: processing failed! unknown error=" << "for deviceID=" << deviceId
			<<" graphID="<<graphId);
		std::list<GraphNeighbor>::const_iterator it = p_rGraphList.begin();
		for (; it != p_rGraphList.end(); ++it)
		{
			LOG_ERROR_APP("[ReplaceDeviceGraphs]: detail: processing failed! unknown error=" << "for deviceID=" << deviceId
				<<" graphID="<< it->m_graphId << " neighbID=" << it->m_toDevId << " index=" << it->m_index);
		}
		factoryDal.RollbackTransaction();
	}

}
void DevicesManager::ChangeDeviceGraphs(int deviceId, std::list<GraphNeighbor>& p_rGraphList, bool skipCache)
{
	if (p_rGraphList.size() == 0)
	{
		LOG_WARN_APP("[ReplaceDeviceGraphs]: Graph list with 0 elements received for deviceID=" << deviceId);
		return;
	}

	if (m_useCache && skipCache == false)
	{
		LOG_DEBUG_APP("[ChangeDeviceGraphs]: update graph operation is cached.");
		m_devicesGraphNeighbsCache.push_back(DeviceGraphNeighbors(deviceId,p_rGraphList));
		return;
	}

	int graphId = 0;
	try
	{
		factoryDal.BeginTransaction();
		std::list<GraphNeighbor>::const_iterator it = p_rGraphList.begin();
		for (; it != p_rGraphList.end(); ++it)
		{
			graphId = it->m_graphId;
			factoryDal.Devices().DeleteGraph(deviceId, it->m_toDevId, it->m_graphId);
			factoryDal.Devices().CreateDeviceGraph(deviceId, it->m_toDevId, it->m_graphId, it->m_index);
			
		}

		factoryDal.CommitTransaction();
	}
	catch (std::exception& ex)
	{
		LOG_ERROR_APP("[ChangeDeviceGraphs]: context: processing failed! error=" << ex.what() << "for deviceID=" << deviceId
			<<" graphID="<<graphId);
		std::list<GraphNeighbor>::const_iterator it = p_rGraphList.begin();
		for (; it != p_rGraphList.end(); ++it)
		{
			LOG_ERROR_APP("[ChangeDeviceGraphs]: detail: processing failed! error=" << ex.what() << "for deviceID=" << deviceId
				<<" graphID="<< it->m_graphId << " neighbID=" << it->m_toDevId << " index=" << it->m_index);
		}
		factoryDal.RollbackTransaction();
	}
	catch (...)
	{
		LOG_ERROR_APP("[ChangeDeviceGraphs]: context: processing failed! unknown error=" << "for deviceID=" << deviceId
			<<" graphID="<<graphId);
		std::list<GraphNeighbor>::const_iterator it = p_rGraphList.begin();
		for (; it != p_rGraphList.end(); ++it)
		{
			LOG_ERROR_APP("[ChangeDeviceGraphs]: detail: processing failed! unknown error=" << "for deviceID=" << deviceId
				<<" graphID="<< it->m_graphId << " neighbID=" << it->m_toDevId << " index=" << it->m_index);
		}
		factoryDal.RollbackTransaction();
	}
}

void DevicesManager::DeleteGraph(int deviceId, int neighborID, int graphID)
{

	if (m_useCache)
	{
		LOG_DEBUG_APP("[DeleteGraph]: delete graph operation is cached.");
		m_devicesGraphNeighbsCache.push_back(DeviceGraphNeighbors(deviceId, GraphNeighbor(neighborID, graphID)));
		return;
	}

	factoryDal.Devices().DeleteGraph(deviceId, neighborID, graphID);
}

void DevicesManager::UpdateSetPublishersLog(PublisherStateMAP_T& p_PublishersStateMap)
{
    LOG_DEBUG_APP("[DoSetBurstNotification]: Save Publishers log in DB");
    try
    {
        int counter = 0;
        factoryDal.BeginTransaction();
        for (PublisherStateMAP_T::iterator it = p_PublishersStateMap.begin(); it != p_PublishersStateMap.end(); ++it)
        {
            if (counter >= 30)
            {
                factoryDal.CommitTransaction();
                counter = 0;
            }

            if (it->second.stateChanged)
            {
                it->second.stateChanged = false;
                DevicePtr device = FindDevice(it->first);
                if (device)
                {
                    factoryDal.Devices().UpdateSetPublishersLog(device->id, it->second.setPublisherState,
                        GetPublisherErrorList(it->second.setPublisherError, it->second.burstErrorList),
                        GetPublisherMessageList(it->second.setPublisherMessage, it->second.burstErrorList));
                    counter++;

                    LOG_INFO_APP("[DoSetBurstNotification]: Save SetPublisher state in DB. Device=" << it->first
                            << ", State=" << GetSetPublishStateText(it->second.setPublisherState)
                            << ", Error=" << GetPublisherErrorList(it->second.setPublisherError, it->second.burstErrorList)
                            << ", Message=" << GetPublisherMessageList(it->second.setPublisherMessage, it->second.burstErrorList));
                }
                else
                {
                    LOG_WARN_APP("[DoSetBurstNotification]: Device with MAC=" << it->first << " not found as registered!");
                }

            }
        }

        factoryDal.CommitTransaction();
    }
    catch (std::exception& ex)
    {
        LOG_ERROR_APP("[DoSetBurstNotification]: Error on saving log in DB! error=" << ex.what());
        factoryDal.RollbackTransaction();
    }
    catch (...)
    {
        LOG_ERROR_APP("[DoSetBurstNotification]: Error on saving log in DB! System error!");
        factoryDal.RollbackTransaction();
    }
}

void DevicesManager::DeleteSetPublishersLog(int deviceId)
{
    factoryDal.Devices().DeleteSetPublishersLog(deviceId);
}

//routes
void DevicesManager::ReplaceRoutes(int deviceID, RoutesListT& p_rRoute)
{
	if(p_rRoute.size() == 0)
	{
		LOG_WARN_APP("[ReplaceRoutes]: no routes list");
		return;
	}

	RoutesListT::const_iterator it = p_rRoute.begin();
	try
	{
		factoryDal.BeginTransaction();
		factoryDal.Devices().CleanDeviceRoutes(deviceID);

		for(; it != p_rRoute.end(); ++it)
			if (it->m_nDeviceId == deviceID)	//it's strange but it is required because of the way it is used; (DO SOMETHING)
				factoryDal.Devices().InsertRoute(*it);

		factoryDal.CommitTransaction();
	}
	catch(std::exception& ex)
	{
		LOG_ERROR_APP("[ReplaceRoutes]: context: processing failed! error=" << ex.what() << " values: " << (*it));
		it = p_rRoute.begin();
		for(; it != p_rRoute.end(); ++it)
			if (it->m_nDeviceId == deviceID)	//it's strange but it is required because of the way it is used; (DO SOMETHING)
				LOG_ERROR_APP("[ReplaceRoutes]: detail: processing failed! error=" << ex.what() << " values: " << (*it));
		factoryDal.RollbackTransaction();
	}
	catch (...)
	{
		LOG_ERROR_APP("[ReplaceRoutes]: context: processing failed! unknown error=" << " values: " << (*it));
		it = p_rRoute.begin();
		for(; it != p_rRoute.end(); ++it)
			if (it->m_nDeviceId == deviceID)	//it's strange but it is required because of the way it is used; (DO SOMETHING)
				LOG_ERROR_APP("[ReplaceRoutes]: detail: processing failed! unknown error=" << " values: " << (*it));
		factoryDal.RollbackTransaction();
	}
}
void DevicesManager::ChangeRoutes(RoutesListT& p_rRoute, bool skipCache)
{
	if(p_rRoute.size() == 0)
	{
		LOG_WARN_APP("[ChangeRoutes]: no routes list");
		return;
	}

	if (m_useCache && skipCache == false)
	{
		LOG_DEBUG_APP("[ChangeRoutes]: update route operation is cached.");
		m_routesCache.splice(m_routesCache.end(), p_rRoute);
		return;
	}

	RoutesListT::const_iterator it = p_rRoute.begin();
	try
	{
		factoryDal.BeginTransaction();

		for(; it != p_rRoute.end(); ++it)
		{
			factoryDal.Devices().CleanDeviceRoute(it->m_nDeviceId, it->m_nRouteId);
			factoryDal.Devices().InsertRoute(*it);
		}

		factoryDal.CommitTransaction();
	}
	catch(std::exception& ex)
	{
		LOG_ERROR_APP("[ChangeRoutes]: context: processing failed! error=" << ex.what() << " values: " << (*it));
		it = p_rRoute.begin();
		for(; it != p_rRoute.end(); ++it)
			LOG_ERROR_APP("[ChangeRoutes]: detail: processing failed! error=" << ex.what() << " values: " << (*it));
		factoryDal.RollbackTransaction();
	}
	catch (...)
	{
		LOG_ERROR_APP("[ChangeRoutes]: context: processing failed! unknown error=" << " values: " << (*it));
		it = p_rRoute.begin();
		for(; it != p_rRoute.end(); ++it)
			LOG_ERROR_APP("[ChangeRoutes]: detail: processing failed! unknown error=" << " values: " << (*it));
		factoryDal.RollbackTransaction();
	}
}
void DevicesManager::DeleteRoute(int deviceID, int routeID)
{
	if (m_useCache)
	{
		LOG_DEBUG_APP("[DeleteRoute]: delete route operation is cached.");
		m_routesCache.push_back(Route(routeID, deviceID));
		return;
	}

	factoryDal.Devices().CleanDeviceRoute(deviceID, routeID);
}

//Source Routes
void DevicesManager::ReplaceSourceRoutes(int p_nDeviceId, SourceRoutesListT& p_rSourceRoutes)
{
	if(p_rSourceRoutes.size() == 0)
	{
		LOG_WARN_APP("[ReplaceSourceRoutes]: no SourceRoutes list");
		return;
	}

	SourceRoutesListT::const_iterator it = p_rSourceRoutes.begin();
	try
	{
		factoryDal.BeginTransaction();
		factoryDal.Devices().CleanDeviceSourceRoutes(p_nDeviceId);

		for(; it != p_rSourceRoutes.end(); ++it)
			if (it->m_nDeviceId == p_nDeviceId)	//it's strange but it is required because of the way it is used; (DO SOMETHING)
				factoryDal.Devices().InsertSourceRoute(*it);

		factoryDal.CommitTransaction();
	}
	catch(std::exception& ex)
	{
		LOG_ERROR_APP("[ReplaceSourceRoutes]: context: processing failed! error=" << ex.what() << " values: " << (*it));
		it = p_rSourceRoutes.begin();
		for(; it != p_rSourceRoutes.end(); ++it)
			if (it->m_nDeviceId == p_nDeviceId)	//it's strange but it is required because of the way it is used; (DO SOMETHING)
				LOG_ERROR_APP("[ReplaceSourceRoutes]: detail: processing failed! error=" << ex.what() << " values: " << (*it));
		factoryDal.RollbackTransaction();
	}
	catch (...)
	{
		LOG_ERROR_APP("[ReplaceSourceRoutes]: context: processing failed! unknown error=" << " values: " << (*it));
		it = p_rSourceRoutes.begin();
		for(; it != p_rSourceRoutes.end(); ++it)
			if (it->m_nDeviceId == p_nDeviceId)	//it's strange but it is required because of the way it is used; (DO SOMETHING)
				LOG_ERROR_APP("[ReplaceSourceRoutes]: detail: processing failed! unknown error=" << " values: " << (*it));
		factoryDal.RollbackTransaction();
	}
}

void DevicesManager::ChangeSourceRoutes(SourceRoutesListT& p_rSourceRoutes, bool skipCache)
{
	if(p_rSourceRoutes.size() == 0)
	{
		LOG_WARN_APP("[ChangeSourceRoutes]: no SourceRoutes list");
		return;
	}

	if (m_useCache && skipCache == false)
	{
		LOG_DEBUG_APP("[ChangeSourceRoutes]: update source-route operation is cached.");
		m_sourceRoutesCache.splice(m_sourceRoutesCache.end(), p_rSourceRoutes);
		return;
	}

	SourceRoutesListT::const_iterator it = p_rSourceRoutes.begin();
	try
	{
		factoryDal.BeginTransaction();
		

		for(; it != p_rSourceRoutes.end(); ++it)
		{
			factoryDal.Devices().CleanDeviceSourceRoutes(it->m_nDeviceId, it->m_nRouteId);
			factoryDal.Devices().InsertSourceRoute(*it);
		}

		factoryDal.CommitTransaction();
	}
	catch(std::exception& ex)
	{
		LOG_ERROR_APP("[ChangeSourceRoutes]: context: processing failed! error=" << ex.what() << " values: " << (*it));
		it = p_rSourceRoutes.begin();
		for(; it != p_rSourceRoutes.end(); ++it)
			LOG_ERROR_APP("[ChangeSourceRoutes]: detail: processing failed! error=" << ex.what() << " values: " << (*it));
		factoryDal.RollbackTransaction();
	}
	catch (...)
	{
		LOG_ERROR_APP("[ChangeSourceRoutes]: context: processing failed! unknown error=" << " values: " << (*it));
		it = p_rSourceRoutes.begin();
		for(; it != p_rSourceRoutes.end(); ++it)
			LOG_ERROR_APP("[ChangeSourceRoutes]: detail: processing failed! unknown error=" << " values: " << (*it));
		factoryDal.RollbackTransaction();
	}
}
void DevicesManager::DeleteSourceRoute(int p_nDeviceId, int routeID)
{
	if (m_useCache)
	{
		LOG_DEBUG_APP("[DeleteSourceRoute]: delete source-route operation is cached.");
		m_sourceRoutesCache.push_back(SourceRoute(p_nDeviceId,routeID));
		return;
	}

	factoryDal.Devices().CleanDeviceSourceRoutes(p_nDeviceId, routeID);
}


//services
void DevicesManager::ReplaceServices(int deviceID, ServicesListT &p_rServices)
{
	if (p_rServices.size() == 0)
	{
		LOG_WARN_APP("[ReplaceServices]: no services list");
		return;
	}

	ServicesListT::const_iterator it = p_rServices.begin();
	try
	{
		factoryDal.BeginTransaction();

		factoryDal.Devices().CleanDeviceServices(deviceID);

		for (; it != p_rServices.end(); ++it)
			if (it->m_nDeviceId == deviceID)	//it's strange but it is required because of the way it is used; (DO SOMETHING)
				factoryDal.Devices().InsertService(*it);

		factoryDal.CommitTransaction();
	}
	catch (std::exception& ex)
	{
		LOG_ERROR_APP("[ReplaceServices]: context: processing failed! error=" << ex.what() << " values: " << (*it));
		it = p_rServices.begin();
		for (; it != p_rServices.end(); ++it)
			if (it->m_nDeviceId == deviceID)	//it's strange but it is required because of the way it is used; (DO SOMETHING)
				LOG_ERROR_APP("[ReplaceServices]: detail: processing failed! error=" << ex.what() << " values: " << (*it));
		factoryDal.RollbackTransaction();
	}
	catch (...)
	{
		LOG_ERROR_APP("[ReplaceServices]: context: processing failed! unknown error=" << " values: " << (*it));
		it = p_rServices.begin();
		for (; it != p_rServices.end(); ++it)
			if (it->m_nDeviceId == deviceID)	//it's strange but it is required because of the way it is used; (DO SOMETHING)
				LOG_ERROR_APP("[ReplaceServices]: detail: processing failed! unknown error=" << " values: " << (*it));
		factoryDal.RollbackTransaction();
	}
}
void DevicesManager::ChangeServices(ServicesListT &p_rServices, bool skipCache)
{
	if (p_rServices.size() == 0)
	{
		LOG_WARN_APP("[ChangeServices]: no services list");
		return;
	}

	if (m_useCache && skipCache == false)
	{
		LOG_DEBUG_APP("[ChangeServices]: update service operation is cached.");
		m_servicesCache.splice(m_servicesCache.end(), p_rServices);
		return;
	}

	ServicesListT::const_iterator it = p_rServices.begin();
	try
	{
		factoryDal.BeginTransaction();
		for (; it != p_rServices.end(); ++it)
		{
			factoryDal.Devices().CleanDeviceService(it->m_nDeviceId, it->m_nServiceId);
			factoryDal.Devices().InsertService(*it);
		}
		factoryDal.CommitTransaction();
	}
	catch (std::exception& ex)
	{
		LOG_ERROR_APP("[ChangeServices]: context: processing failed! error=" << ex.what() << " values: " << (*it));
		it = p_rServices.begin();
		for (; it != p_rServices.end(); ++it)
			LOG_ERROR_APP("[ChangeServices]: detail: processing failed! error=" << ex.what() << " values: " << (*it));
		factoryDal.RollbackTransaction();
	}
	catch (...)
	{
		LOG_ERROR_APP("[ChangeServices]: context: processing failed! unknown error=" << " values: " << (*it));
		it = p_rServices.begin();
		for (; it != p_rServices.end(); ++it)
			LOG_ERROR_APP("[ChangeServices]: detail: processing failed! unknown error=" << " values: " << (*it));
		factoryDal.RollbackTransaction();
	}
}
void DevicesManager::DeleteService(int p_nDeviceID, int p_nServiceID)
{

	if (m_useCache)
	{
		LOG_DEBUG_APP("[DeleteService]: delete service operation is cached.");
		m_servicesCache.push_back(Service(p_nServiceID, p_nDeviceID));
		return;
	}

	try{
		factoryDal.BeginTransaction();
		factoryDal.Devices().CleanDeviceService(p_nDeviceID, p_nServiceID);
		factoryDal.CommitTransaction();
	}
	catch (std::exception& ex)
	{
		LOG_ERROR_APP("[DeleteService]: processing failed! error=" << ex.what() << "for deviceID=" << p_nDeviceID<<" and serviceID="<<p_nServiceID);
		factoryDal.RollbackTransaction();
	}
	catch (...)
	{
		LOG_ERROR_APP("[DeleteService]: processing failed! unknown error!" << "for deviceID=" << p_nDeviceID<<" and serviceID="<<p_nServiceID);
		factoryDal.RollbackTransaction();
	}
}
//superframes
void DevicesManager::ReplaceSuperframes(int deviceID, SuperframesListT& superframes)
{
	if(superframes.size() == 0)
	{
		LOG_WARN_APP("[ReplaceSuperframes]: no superframes list");
		return;
	}

	SuperframesListT::const_iterator it = superframes.begin();
	try
	{
		factoryDal.BeginTransaction();
		factoryDal.Devices().CleanDeviceSuperframes(deviceID);
		for(;it != superframes.end();++it)
			if (it->m_nDeviceId == deviceID)	//it's strange but it is required because of the way it is used; (DO SOMETHING)
				factoryDal.Devices().InsertSuperframe(*it);

		factoryDal.CommitTransaction();
	}
	catch(std::exception ex)
	{
		LOG_ERROR_APP("[ReplaceSuperframes]: coontext: processing failed! error=" << ex.what() << " values: " << (*it));
		it = superframes.begin();
		for(;it != superframes.end();++it)
			if (it->m_nDeviceId == deviceID)	//it's strange but it is required because of the way it is used; (DO SOMETHING)
				LOG_ERROR_APP("[ReplaceSuperframes]: detail: processing failed! error=" << ex.what() << " values: " << (*it));
		factoryDal.RollbackTransaction();
	}
	catch(...)
	{
		LOG_ERROR_APP("[ReplaceSuperframes]: coontext: processing failed! unknown error=" << " values: " << (*it));
		it = superframes.begin();
		for(;it != superframes.end();++it)
			if (it->m_nDeviceId == deviceID)	//it's strange but it is required because of the way it is used; (DO SOMETHING)
				LOG_ERROR_APP("[ReplaceSuperframes]: detail: processing failed! unknown error=" << " values: " << (*it));
		factoryDal.RollbackTransaction();
	}
}
void DevicesManager::ChangeSuperframes(SuperframesListT& superframes, bool skipCache)
{
	if(superframes.size() == 0)
	{
		LOG_WARN_APP("[ChangeSuperframes]: no superframes list");
		return;
	}

	if (m_useCache && skipCache == false)
	{
		LOG_DEBUG_APP("[ChangeSuperframes]: update superframe operation is cached.");
		m_superframesCache.splice(m_superframesCache.end(), superframes);
		return;
	}

	SuperframesListT::const_iterator it = superframes.begin();
	try
	{
		factoryDal.BeginTransaction();		
		for(;it != superframes.end();++it)
		{
			factoryDal.Devices().DeleteSuperframe(it->m_nDeviceId, it->m_nSuperframeId);
			factoryDal.Devices().InsertSuperframe(*it);
		}

		factoryDal.CommitTransaction();
	}
	catch(std::exception ex)
	{
		LOG_ERROR_APP("[ChangeSuperframes]: context: processing failed! error=" << ex.what() << " values: " << (*it));
		it = superframes.begin();
		for(;it != superframes.end();++it)
			LOG_ERROR_APP("[ChangeSuperframes]: detail: processing failed! error=" << ex.what() << " values: " << (*it));
		factoryDal.RollbackTransaction();
	}
	catch(...)
	{
		LOG_ERROR_APP("[ChangeSuperframes]: context: processing failed! unknown error=" << " values: " << (*it));
		it = superframes.begin();
		for(;it != superframes.end();++it)
			LOG_ERROR_APP("[ChangeSuperframes]: detail: processing failed! unknown error=" << " values: " << (*it));
		factoryDal.RollbackTransaction();
	}
}
void DevicesManager::DeleteSuperframe(int deviceID, int superframeID)
{
	if (m_useCache)
	{
		LOG_DEBUG_APP("[DeleteSuperframe]: delete superframe operation is cached.");
		m_superframesCache.push_back(Superframe(superframeID,deviceID));
		return;
	}

	factoryDal.Devices().DeleteSuperframe(deviceID, superframeID);
}

//device schedule link
void DevicesManager::ReplaceDevicesScheduleLinks(DevicesScheduleLinks &links)
{

	if (links.size() == 0)
	{
		LOG_WARN_APP("[ReplaceDevicesScheduleLinks]: no links list");
		return;
	}

	unsigned int i = 0;

	std::list<DeviceScheduleLink>::const_iterator it;
	try
	{
		factoryDal.BeginTransaction();

		for (; i < links.size(); ++i)
		{
			factoryDal.Devices().CleanDeviceScheduleLinks(links[i].m_nDeviceId);
			for (it = links[i].linkList.begin(); it != links[i].linkList.end(); ++it)
			{
				factoryDal.Devices().InsertDeviceScheduleLink(links[i].m_nDeviceId, *it);
			}
		}
		factoryDal.CommitTransaction();
	}
	catch (std::exception& ex)
	{
		LOG_ERROR_APP("[ReplaceDevicesScheduleLinks]: context: processing failed! error=" << ex.what() << "for deviceID=" << links[i].m_nDeviceId << " values: " << (*it));
		for (it = links[i].linkList.begin(); it != links[i].linkList.end(); ++it)
			LOG_ERROR_APP("[ReplaceDevicesScheduleLinks]: detail: processing failed! error=" << ex.what() << "for deviceID=" << links[i].m_nDeviceId << " values: " << (*it));
		
		factoryDal.RollbackTransaction();
	}
	catch (...)
	{
		LOG_ERROR_APP("[ReplaceDevicesScheduleLinks]: context: processing failed! unknown error=" << "for deviceID=" << links[i].m_nDeviceId << " values: " << (*it));
		for (it = links[i].linkList.begin(); it != links[i].linkList.end(); ++it)
			LOG_ERROR_APP("[ReplaceDevicesScheduleLinks]: detail: processing failed! unknown error=" << "for deviceID=" << links[i].m_nDeviceId << " values: " << (*it));

		factoryDal.RollbackTransaction();
	}
}
void DevicesManager::ChangeDevicesScheduleLinks(int p_nDeviceId, std::list<DeviceScheduleLink> &links, bool skipCache)
{
	if (links.size() == 0)
	{
		LOG_WARN_APP("[ChangeDevicesScheduleLinks]: no links list");
		return;
	}

	if (m_useCache && skipCache == false)
	{
		LOG_DEBUG_APP("[ChangeDevicesScheduleLinks]: update schedule link operation is cached.");
		m_devicesScheduleLinksCache.push_back(DevicesLinks(p_nDeviceId,links));
		return;
	}


	std::list<DeviceScheduleLink>::const_iterator it = links.begin();
	try
	{
		factoryDal.BeginTransaction();
		for(;it != links.end();++it)
		{
			factoryDal.Devices().CleanDeviceScheduleLink(p_nDeviceId, (*it));
			factoryDal.Devices().InsertDeviceScheduleLink(p_nDeviceId, (*it));
		}
		factoryDal.CommitTransaction();
	}
	catch (std::exception& ex)
	{
		LOG_ERROR_APP("[ChangeDevicesScheduleLinks]: context: processing failed! error=" << ex.what() << "for deviceID=" << p_nDeviceId << " values: " << (*it));
		it = links.begin();
		for(;it != links.end();++it)
			LOG_ERROR_APP("[ChangeDevicesScheduleLinks]: detatil: processing failed! error=" << ex.what() << "for deviceID=" << p_nDeviceId << " values: " << (*it));
		
		factoryDal.RollbackTransaction();
	}
	catch (...)
	{
		LOG_ERROR_APP("[ChangeDevicesScheduleLinks]: context: processing failed! unknown error=" << "for deviceID=" << p_nDeviceId << " values: " << (*it));
		it = links.begin();
		for(;it != links.end();++it)
			LOG_ERROR_APP("[ChangeDevicesScheduleLinks]: detatil: processing failed! unknown error=" << "for deviceID=" << p_nDeviceId << " values: " << (*it));
		factoryDal.RollbackTransaction();
	}
}
void DevicesManager::DeleteLink(int p_nDeviceId, int superframeID, int neighbID, int slotNo)
{
	if (m_useCache)
	{
		LOG_DEBUG_APP("[DeleteLink]: delete schedule link operation is cached.");
		m_devicesScheduleLinksCache.push_back(DevicesLinks(p_nDeviceId, DeviceScheduleLink(superframeID, neighbID,slotNo)));
		return;
	}

	factoryDal.Devices().DeleteLink(p_nDeviceId, superframeID, neighbID, slotNo);
}

//device health
void DevicesManager::ReplaceDevicesHealthReport(std::vector<DeviceHealth> &devicesHealth)
{
	if (devicesHealth.size() == 0)
	{
		LOG_WARN_APP("[ReplaceDevicesHealthReport]: no device health list");
		return;
	}

	unsigned int i = 0;
	try
	{
		factoryDal.BeginTransaction();

		for (; i < devicesHealth.size(); ++i)
		{
			factoryDal.Devices().CleanDeviceHealthReport(devicesHealth[i].m_nDeviceId);
			factoryDal.Devices().InsertDeviceHealthReport(devicesHealth[i]);
		}
		factoryDal.CommitTransaction();
	}
	catch (std::exception& ex)
	{
		LOG_ERROR_APP("[ReplaceDevicesHealthReport]: context: processing failed! error=" << ex.what() << " values: " << devicesHealth[i]);
		i = 0;
		for (; i < devicesHealth.size(); ++i)
			LOG_ERROR_APP("[ReplaceDevicesHealthReport]: detail: processing failed! error=" << ex.what() << " values: " << devicesHealth[i]);
		factoryDal.RollbackTransaction();
	}
	catch (...)
	{
		LOG_ERROR_APP("[ReplaceDevicesHealthReport]: context: processing failed! unknown error="  << " values: " << devicesHealth[i]);
		i = 0;
		for (; i < devicesHealth.size(); ++i)
			LOG_ERROR_APP("[ReplaceDevicesHealthReport]: detail: processing failed! unknown error=" << " values: " << devicesHealth[i]);
		factoryDal.RollbackTransaction();
	}
}
void DevicesManager::ReplaceDeviceHealthReport(DeviceHealth &deviceHealth, bool skipCache)
{

	if (m_useCache && skipCache == true)
	{
		LOG_DEBUG_APP("[ReplaceDeviceHealthReport]: update device health operation is cached.");
		m_devicesHealthCache.push_back(deviceHealth);
		return;
	}

	try
	{
		factoryDal.BeginTransaction();

		factoryDal.Devices().CleanDeviceHealthReport(deviceHealth.m_nDeviceId);
		factoryDal.Devices().InsertDeviceHealthReport(deviceHealth);
		
		factoryDal.CommitTransaction();
	}
	catch (std::exception& ex)
	{
		LOG_ERROR_APP("[ReplaceDeviceHealthReport]: processing failed! error=" << ex.what() << " values: " << deviceHealth);
		factoryDal.RollbackTransaction();
	}
	catch (...)
	{
		LOG_ERROR_APP("[ReplaceDeviceHealthReport]: processing failed! unknown error!" << " values: " << deviceHealth);
		factoryDal.RollbackTransaction();
	}
}

//device  neighbors health
void DevicesManager::ChangeDeviceNeighborsHealth(DeviceNeighborsHealth &m_oNeighborsHealthCache, bool skipCache)
{

	if (m_useCache && skipCache == false)
	{
		LOG_DEBUG_APP("[ChangeDeviceNeighborsHealth]: update device neighbors health operation is cached.");
		m_devicesNeighsHealthCache.push_back(m_oNeighborsHealthCache);
		return;
	}

	try
	{
		factoryDal.BeginTransaction();

		for(std::list<NeighborHealth>::iterator it = m_oNeighborsHealthCache.m_oNeighborsList.begin(); 
					it != m_oNeighborsHealthCache.m_oNeighborsList.end(); ++it)
		{
			factoryDal.Devices().CleanDeviceNeighborHealth(m_oNeighborsHealthCache.m_nDeviceID, it->m_nPeerID);
		}
		factoryDal.Devices().InsertDeviceNeighborsHealth(m_oNeighborsHealthCache);

		factoryDal.CommitTransaction();
	}
	catch (std::exception& ex)
	{
		LOG_ERROR_APP("[ChangeDeviceNeighborsHealth]: context: processing failed! error=" << ex.what() << "for deviceID=" << m_oNeighborsHealthCache.m_nDeviceID);
		for(std::list<NeighborHealth>::iterator it = m_oNeighborsHealthCache.m_oNeighborsList.begin(); 
			it != m_oNeighborsHealthCache.m_oNeighborsList.end(); ++it)
		{
			LOG_ERROR_APP("[ChangeDeviceNeighborsHealth]: detail: processing failed! error=" << ex.what() << (*it));
		}
		factoryDal.RollbackTransaction();
	}
	catch (...)
	{
		LOG_ERROR_APP("[ChangeDeviceNeighborsHealth]: context: processing failed! unknown error=" << "for deviceID=" << m_oNeighborsHealthCache.m_nDeviceID);
		for(std::list<NeighborHealth>::iterator it = m_oNeighborsHealthCache.m_oNeighborsList.begin(); 
			it != m_oNeighborsHealthCache.m_oNeighborsList.end(); ++it)
		{
			LOG_ERROR_APP("[ChangeDeviceNeighborsHealth]: detail: processing failed! unknown error=" << (*it));
		}
		factoryDal.RollbackTransaction();
	}
}
void DevicesManager::ReplaceDeviceNeighborsHealth(DeviceNeighborsHealth &m_oNeighborsHealthCache)
{

	try
	{
		factoryDal.BeginTransaction();

		factoryDal.Devices().CleanDeviceNeighborsHealth(m_oNeighborsHealthCache.m_nDeviceID);
		factoryDal.Devices().InsertDeviceNeighborsHealth(m_oNeighborsHealthCache);
		
		factoryDal.CommitTransaction();
	}
	catch (std::exception& ex)
	{
		LOG_ERROR_APP("[ReplaceDeviceNeighborsHealth]: processing failed! error=" << ex.what() << "for deviceID=" << m_oNeighborsHealthCache.m_nDeviceID);
		factoryDal.RollbackTransaction();
	}
	catch (...)
	{
		LOG_ERROR_APP("[ReplaceDeviceNeighborsHealth]: processing failed! unknown error!" << "for deviceID=" << m_oNeighborsHealthCache.m_nDeviceID);
		factoryDal.RollbackTransaction();
	}
}
void DevicesManager::ReplaceDevicesNeighborsHealth(DevicesNeighborsHealth &m_oNeighborsHealthCache)
{

	if (m_oNeighborsHealthCache.size() == 0)
	{
		LOG_WARN_APP("[ReplaceDevicesNeighborsHealth]: no device neighbor health list");
		return;
	}

	unsigned int i = 0;
	try
	{
		factoryDal.BeginTransaction();

		for (; i < m_oNeighborsHealthCache.size(); ++i)
		{
			if (m_oNeighborsHealthCache[i].m_oNeighborsList.size() == 0)
				continue;
			factoryDal.Devices().CleanDeviceNeighborsHealth(m_oNeighborsHealthCache[i].m_nDeviceID);
			factoryDal.Devices().InsertDeviceNeighborsHealth(m_oNeighborsHealthCache[i]);
		}
		factoryDal.CommitTransaction();
	}
	catch (std::exception& ex)
	{
		LOG_ERROR_APP("[ReplaceDevicesNeighborsHealth]: context: processing failed! error=" << ex.what() << "for deviceID=" << m_oNeighborsHealthCache[i].m_nDeviceID);
		i = 0;
		for (; i < m_oNeighborsHealthCache.size(); ++i)
			LOG_ERROR_APP("[ReplaceDevicesNeighborsHealth]: detail: processing failed! error=" << ex.what() << "for deviceID=" << m_oNeighborsHealthCache[i].m_nDeviceID);
		factoryDal.RollbackTransaction();
	}
	catch (...)
	{
		LOG_ERROR_APP("[ReplaceDevicesNeighborsHealth]: processing failed! unknown error!" << "for deviceID=" << m_oNeighborsHealthCache[i].m_nDeviceID);
		i = 0;
		for (; i < m_oNeighborsHealthCache.size(); ++i)
			LOG_ERROR_APP("[ReplaceDevicesNeighborsHealth]: detail: processing failed! unknown error=" << "for deviceID=" << m_oNeighborsHealthCache[i].m_nDeviceID);
		factoryDal.RollbackTransaction();
	}
}

//neighbors signal level report
void DevicesManager::ChangeDevNeighbSignalLevels(int deviceID, const std::list<NeighbourSignalLevel>& p_rDevNeighbSignalLevels, bool skipCache)
{
	if (p_rDevNeighbSignalLevels.size() == 0)
	{
		LOG_WARN_APP("[ChangeDevNeighbSignalLevels]: no neighbor signals list");
		return;
	}

	if (m_useCache && skipCache == false)
	{
		LOG_DEBUG_APP("[ChangeDevNeighbSignalLevels]: update device neighbors signal level operation is cached.");
		m_neighbsSignalLevelCache.push_back(NeighbourSignalLevels(deviceID,p_rDevNeighbSignalLevels));
		return;
	}

	std::list<NeighbourSignalLevel>::const_iterator it = p_rDevNeighbSignalLevels.begin();
	try
	{
		factoryDal.BeginTransaction();
		for (; it != p_rDevNeighbSignalLevels.end(); ++it)
		{
			factoryDal.Devices().CleanDevNeighbSignalLevel(deviceID, it->neighDevID);
			factoryDal.Devices().InsertDevNeighbSignalLevels(deviceID, it->neighDevID, it->signalLevel);
		}

		factoryDal.CommitTransaction();
	}
	catch (std::exception& ex)
	{
		factoryDal.RollbackTransaction();
		LOG_ERROR_APP("[ChangeDevNeighbSignalLevels]: processing failed! error=" << ex.what() << "for deviceID=" << deviceID << " values:" << (*it));
		LOG_ERROR_APP("[ChangeDevNeighbSignalLevels]: the packet has the values:");
		int i = 0;
		for (it = p_rDevNeighbSignalLevels.begin(); it != p_rDevNeighbSignalLevels.end(); ++it)
		{
			LOG_ERROR_APP("[ChangeDevNeighbSignalLevels]: indice=" << i << " with values: " << (*it));
		}
		LOG_ERROR_APP("[ChangeDevNeighbSignalLevels]: values from db are:");
		factoryDal.Devices().Log_SignalLvel_Info(deviceID);
		
	}
	catch (...)
	{
		factoryDal.RollbackTransaction();
		LOG_ERROR_APP("[ChangeDevNeighbSignalLevels]: processing failed! unknown error=" << "for deviceID=" << deviceID << " values:" << (*it));
		LOG_ERROR_APP("[ChangeDevNeighbSignalLevels]: the packet has the values:");
		int i = 0;
		for (it = p_rDevNeighbSignalLevels.begin(); it != p_rDevNeighbSignalLevels.end(); ++it)
		{
			LOG_ERROR_APP("[ChangeDevNeighbSignalLevels]: indice=" << i << " with values: " << (*it));
		}
		LOG_ERROR_APP("[ChangeDevNeighbSignalLevels]: values from db are:");
		factoryDal.Devices().Log_SignalLvel_Info(deviceID);
	}
}
void DevicesManager::ReplaceDevNeighbSignalLevels(int deviceID, const std::list<NeighbourSignalLevel>& p_rDevNeighbSignalLevels)
{
	if (p_rDevNeighbSignalLevels.size() == 0)
	{
		LOG_WARN_APP("[ChangeDevNeighbSignalLevels]: no neighbor signals list");
		return;
	}

	std::list<NeighbourSignalLevel>::const_iterator it = p_rDevNeighbSignalLevels.begin();
	try
	{
		factoryDal.BeginTransaction();
		factoryDal.Devices().CleanDevNeighbSignalLevels(deviceID);
		for (; it != p_rDevNeighbSignalLevels.end(); ++it)
			factoryDal.Devices().InsertDevNeighbSignalLevels(deviceID, it->neighDevID, it->signalLevel);
		
		factoryDal.CommitTransaction();
	}
	catch (std::exception& ex)
	{
		LOG_ERROR_APP("[ReplaceDevNeighbSignalLevels]: context: processing failed! error=" << ex.what() << "for deviceID=" << deviceID << " values:" << (*it));
		it = p_rDevNeighbSignalLevels.begin();
		for (; it != p_rDevNeighbSignalLevels.end(); ++it)
			LOG_ERROR_APP("[ReplaceDevNeighbSignalLevels]: detail: processing failed! error=" << ex.what() << "for deviceID=" << deviceID << " values:" << (*it));
		factoryDal.RollbackTransaction();
	}
	catch (...)
	{
		LOG_ERROR_APP("[ReplaceDevNeighbSignalLevels]: context: processing failed! unknown error=" << "for deviceID=" << deviceID << " values:" << (*it));
		it = p_rDevNeighbSignalLevels.begin();
		for (; it != p_rDevNeighbSignalLevels.end(); ++it)
			LOG_ERROR_APP("[ReplaceDevNeighbSignalLevels]: detail: processing failed! unknown error=" << "for deviceID=" << deviceID << " values:" << (*it));
		factoryDal.RollbackTransaction();
	}
}

//alarms
void DevicesManager::InsertAlarm(const Alarm &p_rAlarm)
{

	if (m_useCache)
	{
		m_alarmsCache.push_back(p_rAlarm);
		return;
	}

	if (p_rAlarm.hasMIC)
		factoryDal.Devices().InsertAlarm(p_rAlarm.m_nDeviceId, p_rAlarm.m_nAlarmType, p_rAlarm.m_PeerID_GraphId, p_rAlarm.m_MIC, p_rAlarm.time);
	else
		factoryDal.Devices().InsertAlarm(p_rAlarm.m_nDeviceId, p_rAlarm.m_nAlarmType, p_rAlarm.m_PeerID_GraphId, p_rAlarm.time);

}

//reports cache
void DevicesManager::FlushGraphs()
{
	if (m_devicesGraphNeighbsCache.size() == 0)
	{
		LOG_DEBUG_APP("[FlushGraphs]: Graph list empty");
		return;
	}

	DevicesGraphNeighborsListT::iterator it = m_devicesGraphNeighbsCache.begin();
	int graphID=0;
	try
	{
		for (; it != m_devicesGraphNeighbsCache.end(); ++it)
		{
			std::list<GraphNeighbor>::const_iterator jt = it->neighbs.begin();
			for (; jt != it->neighbs.end(); ++jt)
			{
				graphID = jt->m_graphId;
				factoryDal.Devices().DeleteGraph(it->deviceID, jt->m_toDevId, jt->m_graphId);
				if (jt->m_toDelete == false)
					factoryDal.Devices().CreateDeviceGraph(it->deviceID, jt->m_toDevId, jt->m_graphId, jt->m_index);
			}
		}
	}
	catch (std::exception& ex)
	{
		LOG_ERROR_APP("[FlushGraphs]: context: processing failed! error=" << ex.what() << "for deviceID=" << it->deviceID
			<<" graphID="<<graphID);
		it = m_devicesGraphNeighbsCache.begin();
		for (; it != m_devicesGraphNeighbsCache.end(); ++it)
		{
			std::list<GraphNeighbor>::const_iterator jt = it->neighbs.begin();
			for (; jt != it->neighbs.end(); ++jt)
			{
				LOG_ERROR_APP("[FlushGraphs]: detail: processing failed! error=" << ex.what() << "for deviceID=" << it->deviceID
					<<" graphID="<< jt->m_graphId << " neighbID=" << jt->m_toDevId << " index=" << jt->m_index << " todelete=" << jt->m_toDelete ? "true":"false");
			}
		}

		m_devicesGraphNeighbsCache.clear();
		throw ex;
	}
	catch (...)
	{
		LOG_ERROR_APP("[FlushGraphs]: context: processing failed! unknown error=" << "for deviceID=" << it->deviceID
			<<" graphID="<<graphID);
		it = m_devicesGraphNeighbsCache.begin();
		for (; it != m_devicesGraphNeighbsCache.end(); ++it)
		{
			std::list<GraphNeighbor>::const_iterator jt = it->neighbs.begin();
			for (; jt != it->neighbs.end(); ++jt)
			{
				LOG_ERROR_APP("[FlushGraphs]: detail: processing failed! unknown error=" << "for deviceID=" << it->deviceID
					<<" graphID="<< jt->m_graphId << " neighbID=" << jt->m_toDevId << " index=" << jt->m_index << " todelete=" << jt->m_toDelete ? "true":"false");
			}
		}

		m_devicesGraphNeighbsCache.clear();
		throw;
	}
	m_devicesGraphNeighbsCache.clear();
}
void DevicesManager::FlushRoutes()
{
	if(m_routesCache.size() == 0)
	{
		LOG_DEBUG_APP("[FlushRoutes]: no routes list");
		return;
	}

	RoutesListT::const_iterator it = m_routesCache.begin();
	try
	{
		for(; it != m_routesCache.end(); ++it)
		{
			factoryDal.Devices().CleanDeviceRoute(it->m_nDeviceId, it->m_nRouteId);
			if (it->m_toDelete == false)
				factoryDal.Devices().InsertRoute(*it);
		}
	}
	catch(std::exception& ex)
	{
		LOG_ERROR_APP("[FlushRoutes]: context: processing failed! error=" << ex.what() << " values: " << (*it));
		it = m_routesCache.begin();
		for(; it != m_routesCache.end(); ++it)
			LOG_ERROR_APP("[FlushRoutes]: detail: processing failed! error=" << ex.what() << " values: " << (*it));
		m_routesCache.clear();
		throw ex;
	}
	catch (...)
	{
		LOG_ERROR_APP("[FlushRoutes]: context: processing failed! unknown error=" << " values: " << (*it));
		it = m_routesCache.begin();
		for(; it != m_routesCache.end(); ++it)
			LOG_ERROR_APP("[FlushRoutes]: detail: processing failed! unknown error=" << " values: " << (*it));
		m_routesCache.clear();
		throw;
	}
	m_routesCache.clear();
}
void DevicesManager::FlushSourceRoutes()
{
	if(m_sourceRoutesCache.size() == 0)
	{
		LOG_DEBUG_APP("[FlushSourceRoutes]: no SourceRoutes list");
		return;
	}

	SourceRoutesListT::const_iterator it = m_sourceRoutesCache.begin();
	try
	{
		for(; it != m_sourceRoutesCache.end(); ++it)
		{
			factoryDal.Devices().CleanDeviceSourceRoutes(it->m_nDeviceId, it->m_nRouteId);
			if (it->m_toDelete == false)
				factoryDal.Devices().InsertSourceRoute(*it);
		}
	}
	catch(std::exception& ex)
	{
		LOG_ERROR_APP("[FlushSourceRoutes]: context: processing failed! error=" << ex.what() << " values: " << (*it));
		it = m_sourceRoutesCache.begin();
		for(; it != m_sourceRoutesCache.end(); ++it)
			LOG_ERROR_APP("[FlushSourceRoutes]: detail: processing failed! error=" << ex.what() << " values: " << (*it));
		m_sourceRoutesCache.clear();
		throw ex;
	}
	catch (...)
	{
		LOG_ERROR_APP("[FlushSourceRoutes]: context: processing failed! unknown error=" << " values: " << (*it));
		it = m_sourceRoutesCache.begin();
		for(; it != m_sourceRoutesCache.end(); ++it)
			LOG_ERROR_APP("[FlushSourceRoutes]: detail: processing failed! unknown error=" << " values: " << (*it));
		m_sourceRoutesCache.clear();
		throw;
	}
	m_sourceRoutesCache.clear();
}
void DevicesManager::FlushServices()
{
	if (m_servicesCache.size() == 0)
	{
		LOG_DEBUG_APP("[FlushServices]: no services list");
		return;
	}

	ServicesListT::const_iterator it = m_servicesCache.begin();
	try
	{
		for (; it != m_servicesCache.end(); ++it)
		{
			factoryDal.Devices().CleanDeviceService(it->m_nDeviceId, it->m_nServiceId);
			if (it->m_toDelete == false)
				factoryDal.Devices().InsertService(*it);
		}
	}
	catch (std::exception& ex)
	{
		
		LOG_ERROR_APP("[FlushServices]: context: processing failed! error=" << ex.what() << " values: " << (*it));
		it = m_servicesCache.begin();
		for (; it != m_servicesCache.end(); ++it)
			LOG_ERROR_APP("[FlushServices]: detail: processing failed! error=" << ex.what() << " values: " << (*it));
		m_servicesCache.clear();
		throw ex;
	}
	catch (...)
	{
		LOG_ERROR_APP("[FlushServices]: context: processing failed! unknown error=" << " values: " << (*it));
		it = m_servicesCache.begin();
		for (; it != m_servicesCache.end(); ++it)
			LOG_ERROR_APP("[FlushServices]: detail: processing failed! unknown error=" << " values: " << (*it));
		m_servicesCache.clear();
		throw;
	}
	m_servicesCache.clear();
}
void DevicesManager::FlushSuperframes()
{
	if(m_superframesCache.size() == 0)
	{
		LOG_DEBUG_APP("[FlushSuperframes]: no superframes list");
		return;
	}

	SuperframesListT::const_iterator it = m_superframesCache.begin();
	try
	{
		for(;it != m_superframesCache.end();++it)
		{
			factoryDal.Devices().DeleteSuperframe(it->m_nDeviceId, it->m_nSuperframeId);
			if (it->m_toDelete == false)
				factoryDal.Devices().InsertSuperframe(*it);
		}
	}
	catch(std::exception ex)
	{
		LOG_ERROR_APP("[FlushSuperframes]: context: processing failed! error=" << ex.what() << " values: " << (*it));
		it = m_superframesCache.begin();
		for(;it != m_superframesCache.end();++it)
			LOG_ERROR_APP("[FlushSuperframes]: detail: processing failed! error=" << ex.what() << " values: " << (*it));
		m_superframesCache.clear();
		throw ex;
	}
	catch(...)
	{
		LOG_ERROR_APP("[FlushSuperframes]: context: processing failed! unknown error=" << " values: " << (*it));
		it = m_superframesCache.begin();
		for(;it != m_superframesCache.end();++it)
			LOG_ERROR_APP("[FlushSuperframes]: detail: processing failed! unknown error=" << " values: " << (*it));
		m_superframesCache.clear();
		throw;
	}
	m_superframesCache.clear();
}
void DevicesManager::FlushDeviceScheduleLinks()
{
	if (m_devicesScheduleLinksCache.size() == 0)
	{
		LOG_DEBUG_APP("[FlushDeviceScheduleLinks]: no links list");
		return;
	}

	int deviceID = 0;

	DevicesScheduleLinksListT::iterator xt = m_devicesScheduleLinksCache.begin();
	for (; xt != m_devicesScheduleLinksCache.end(); ++xt)
	{
		deviceID = xt->m_nDeviceId;

		std::list<DeviceScheduleLink>::const_iterator it = xt->linkList.begin();
		try
		{
			for(;it != xt->linkList.end();++it)
			{
				factoryDal.Devices().CleanDeviceScheduleLink(deviceID, (*it));
				factoryDal.Devices().InsertDeviceScheduleLink(deviceID, (*it));
			}
		}
		catch (std::exception& ex)
		{

			LOG_ERROR_APP("[FlushDeviceScheduleLinks]: context: processing failed! error=" << ex.what() << "for deviceID=" << deviceID << " values: " << (*it));
			xt = m_devicesScheduleLinksCache.begin();
			for (; xt != m_devicesScheduleLinksCache.end(); ++xt)
			{
				it = xt->linkList.begin();
				for(;it != xt->linkList.end();++it)
					LOG_ERROR_APP("[FlushDeviceScheduleLinks]: detail: processing failed! error=" << ex.what() << "for deviceID=" << xt->m_nDeviceId << " values: " << (*it));
			}
			m_devicesScheduleLinksCache.clear();
			throw ex;
		}
		catch (...)
		{
			LOG_ERROR_APP("[FlushDeviceScheduleLinks]: context: processing failed! unknown error=" << "for deviceID=" << deviceID << " values: " << (*it));
			xt = m_devicesScheduleLinksCache.begin();
			for (; xt != m_devicesScheduleLinksCache.end(); ++xt)
			{
				it = xt->linkList.begin();
				for(;it != xt->linkList.end();++it)
					LOG_ERROR_APP("[FlushDeviceScheduleLinks]: detail: processing failed! unknown error=" << "for deviceID=" << xt->m_nDeviceId << " values: " << (*it));
			}
			m_devicesScheduleLinksCache.clear();
			throw;
		}
	}
	m_devicesScheduleLinksCache.clear();
	
}
void DevicesManager::FlushDeviceHealth()
{
	if (m_devicesHealthCache.size() == 0)
	{
		LOG_DEBUG_APP("[FlushDeviceHealth]: no device health list");
		return;
	}

	DevicesHealthListT::iterator it = m_devicesHealthCache.begin();
	for(; it != m_devicesHealthCache.end(); ++it)
	{
		try
		{
			factoryDal.Devices().CleanDeviceHealthReport(it->m_nDeviceId);
			factoryDal.Devices().InsertDeviceHealthReport(*it);
		}
		catch (std::exception& ex)
		{
			LOG_ERROR_APP("[FlushDeviceHealth]: context: processing failed! error=" << ex.what() << " values: " << *it);
			it = m_devicesHealthCache.begin();
			for(; it != m_devicesHealthCache.end(); ++it)
				LOG_ERROR_APP("[FlushDeviceHealth]: detail: processing failed! error=" << ex.what() << " values: " << *it);
			m_devicesHealthCache.clear();
			throw ex;
		}
		catch (...)
		{
			LOG_ERROR_APP("[FlushDeviceHealth]: context: processing failed! unknown error=" << " values: " << *it);
			it = m_devicesHealthCache.begin();
			for(; it != m_devicesHealthCache.end(); ++it)
				LOG_ERROR_APP("[FlushDeviceHealth]: detail: processing failed! unknown error=" << " values: " << *it);
			m_devicesHealthCache.clear();
			throw;
		}
	}
	m_devicesHealthCache.clear();
}
void DevicesManager::FlushDeviceNeighbHealth()
{
	if (m_devicesNeighsHealthCache.size() == 0)
	{
		LOG_DEBUG_APP("[FlushDeviceNeighbHealth]: no device neighbors health list");
		return;
	}

	DevicesNeighborsHealthListT::iterator xt = m_devicesNeighsHealthCache.begin();
	for (; xt != m_devicesNeighsHealthCache.end(); ++xt)
	{
		std::list<NeighborHealth>::iterator it = xt->m_oNeighborsList.begin();
		try
		{
			for(; it != xt->m_oNeighborsList.end(); ++it)
			{
				factoryDal.Devices().CleanDeviceNeighborHealth(xt->m_nDeviceID, it->m_nPeerID);
			}
			
			factoryDal.Devices().InsertDeviceNeighborsHealth(*xt);
		}
		catch (std::exception& ex)
		{
			LOG_ERROR_APP("[FlushDeviceNeighbHealth]: context: processing failed! error=" << ex.what() << "deviceID=" << xt->m_nDeviceID << " neighbID=" << it->m_nPeerID);
			xt = m_devicesNeighsHealthCache.begin();
			for (; xt != m_devicesNeighsHealthCache.end(); ++xt)
			{
				it = xt->m_oNeighborsList.begin();
				for(; it != xt->m_oNeighborsList.end(); ++it)
					LOG_ERROR_APP("[FlushDeviceNeighbHealth]: detail: processing failed! error=" << ex.what() << "deviceID=" << xt->m_nDeviceID << " neighbID=" << it->m_nPeerID);
			}
			m_devicesNeighsHealthCache.clear();
			throw ex;
		}
		catch (...)
		{
			LOG_ERROR_APP("[FlushDeviceNeighbHealth]: context: processing failed! unknown error=" << "deviceID=" << xt->m_nDeviceID << " neighbID=" << it->m_nPeerID);
			xt = m_devicesNeighsHealthCache.begin();
			for (; xt != m_devicesNeighsHealthCache.end(); ++xt)
			{
				it = xt->m_oNeighborsList.begin();
				for(; it != xt->m_oNeighborsList.end(); ++it)
					LOG_ERROR_APP("[FlushDeviceNeighbHealth]: detail: processing failed! unknown error=" << "deviceID=" << xt->m_nDeviceID << " neighbID=" << it->m_nPeerID);
			}
			m_devicesNeighsHealthCache.clear();
			throw;
		}	
	}
	m_devicesNeighsHealthCache.clear();
	
}
void DevicesManager::FlushDeviceNeighbsSignalLevel()
{
	if (m_neighbsSignalLevelCache.size() == 0)
	{
		LOG_DEBUG_APP("[FlushDeviceNeighbsSignalLevel]: no neighbor signals list");
		return;
	}

	int deviceID = 0;
	NeighbourSignalLevelsListT::iterator xt = m_neighbsSignalLevelCache.begin();
	for (; xt != m_neighbsSignalLevelCache.end(); ++xt)
	{
		deviceID = xt->deviceID;

		std::list<NeighbourSignalLevel>::const_iterator it = xt->neighbors.begin();
		try
		{
			for (; it != xt->neighbors.end(); ++it)
			{
				factoryDal.Devices().CleanDevNeighbSignalLevel(deviceID, it->neighDevID);
				factoryDal.Devices().InsertDevNeighbSignalLevels(deviceID, it->neighDevID, it->signalLevel);
			}
		}
		catch (std::exception& ex)
		{

			LOG_ERROR_APP("[FlushDeviceNeighbsSignalLevel]: processing failed! error=" << ex.what() << "for deviceID=" << deviceID << " values:" << (*it));
			LOG_ERROR_APP("[FlushDeviceNeighbsSignalLevel]: the cache has the values:");
			int i = 0;
			xt = m_neighbsSignalLevelCache.begin();
			for (; xt != m_neighbsSignalLevelCache.end(); ++xt)
			{
				for (it = xt->neighbors.begin(); it != xt->neighbors.end(); ++it)
				{
					LOG_ERROR_APP("[FlushDeviceNeighbsSignalLevel]: indice=" << i << " for deviceID=" << xt->deviceID <<" with values: " << (*it));
				}
			}
			
			m_neighbsSignalLevelCache.clear();
			throw ex;

		}
		catch (...)
		{
			LOG_ERROR_APP("[FlushDeviceNeighbsSignalLevel]: processing failed! unknown error=" << "for deviceID=" << deviceID << " values:" << (*it));
			LOG_ERROR_APP("[FlushDeviceNeighbsSignalLevel]: the cache has the values:");
			int i = 0;
			xt = m_neighbsSignalLevelCache.begin();
			for (; xt != m_neighbsSignalLevelCache.end(); ++xt)
			{
				for (it = xt->neighbors.begin(); it != xt->neighbors.end(); ++it)
				{
					LOG_ERROR_APP("[FlushDeviceNeighbsSignalLevel]: indice=" << i << " for deviceID=" << xt->deviceID << " with values: " << (*it));
				}
			}
			m_neighbsSignalLevelCache.clear();
			throw;
		}
	}
	m_neighbsSignalLevelCache.clear();
}
void DevicesManager::FlushAlarms()
{
	if (m_alarmsCache.size() == 0)
	{
		LOG_DEBUG_APP("[FlushAlarms]: no neighbor signals list");
		return;
	}

	AlarmsListT::iterator it = m_alarmsCache.begin();
	for (; it != m_alarmsCache.end(); ++it)
	{
		try
		{
			if (it->hasMIC)
				factoryDal.Devices().InsertAlarm(it->m_nDeviceId, it->m_nAlarmType, it->m_PeerID_GraphId, it->m_MIC, it->time);
			else
				factoryDal.Devices().InsertAlarm(it->m_nDeviceId, it->m_nAlarmType, it->m_PeerID_GraphId, it->time);
		}
		catch (std::exception& ex)
		{
			LOG_ERROR_APP("[FlushAlarms]: context: processing failed! error=" << ex.what() << "deviceID=" <<it->m_nDeviceId);
			it = m_alarmsCache.begin();
			for (; it != m_alarmsCache.end(); ++it)
				LOG_ERROR_APP("[FlushAlarms]: detail: processing failed! error=" << ex.what() << "deviceID=" <<it->m_nDeviceId);
			m_alarmsCache.clear();
		}
		catch (...)
		{
			LOG_ERROR_APP("[FlushAlarms]: context: processing failed! unknown error=" << "deviceID=" <<it->m_nDeviceId);
			it = m_alarmsCache.begin();
			for (; it != m_alarmsCache.end(); ++it)
				LOG_ERROR_APP("[FlushAlarms]: detail: processing failed! unknown error=" << "deviceID=" <<it->m_nDeviceId);
			m_alarmsCache.clear();
		}
	}
	
	m_alarmsCache.clear();
}
void DevicesManager::FlushEachReport()
{
	try
	{
		factoryDal.BeginTransaction(); 	FlushGraphs(); factoryDal.CommitTransaction();
	}
	catch (std::exception& ex)
	{ 	LOG_ERROR_APP("[FlushEachReport]: FlushGraphs processing failed! error=" << ex.what()); factoryDal.RollbackTransaction();
	}
	catch (...)
	{	LOG_ERROR_APP("[FlushEachReport]: FlushGraphs processing failed! unknown error!"); factoryDal.RollbackTransaction();
	}

	try
	{
		factoryDal.BeginTransaction(); 	FlushRoutes(); factoryDal.CommitTransaction();
	}
	catch (std::exception& ex)
	{ 	LOG_ERROR_APP("[FlushEachReport]: FlushRoutes processing failed! error=" << ex.what()); factoryDal.RollbackTransaction();
	}
	catch (...)
	{	LOG_ERROR_APP("[FlushEachReport]: FlushRoutes processing failed! unknown error!"); factoryDal.RollbackTransaction();
	}

	try
	{
		factoryDal.BeginTransaction(); 	FlushSourceRoutes(); factoryDal.CommitTransaction();
	}
	catch (std::exception& ex)
	{ 	LOG_ERROR_APP("[FlushEachReport]: FlushSourceRoutes processing failed! error=" << ex.what()); factoryDal.RollbackTransaction();
	}
	catch (...)
	{	LOG_ERROR_APP("[FlushEachReport]: FlushSourceRoutes processing failed! unknown error!"); factoryDal.RollbackTransaction();
	}


	try
	{
		factoryDal.BeginTransaction(); 	FlushServices(); factoryDal.CommitTransaction();
	}
	catch (std::exception& ex)
	{ 	LOG_ERROR_APP("[FlushEachReport]: FlushServices processing failed! error=" << ex.what()); factoryDal.RollbackTransaction();
	}
	catch (...)
	{	LOG_ERROR_APP("[FlushEachReport]: FlushServices processing failed! unknown error!"); factoryDal.RollbackTransaction();
	}

	try
	{
		factoryDal.BeginTransaction(); 	FlushSuperframes(); factoryDal.CommitTransaction();
	}
	catch (std::exception& ex)
	{ 	LOG_ERROR_APP("[FlushEachReport]: FlushSuperframes processing failed! error=" << ex.what()); factoryDal.RollbackTransaction();
	}
	catch (...)
	{	LOG_ERROR_APP("[FlushEachReport]: FlushSuperframes processing failed! unknown error!"); factoryDal.RollbackTransaction();
	}

	try
	{
		factoryDal.BeginTransaction(); 	FlushDeviceScheduleLinks(); factoryDal.CommitTransaction();
	}
	catch (std::exception& ex)
	{ 	LOG_ERROR_APP("[FlushEachReport]: FlushDeviceScheduleLinks processing failed! error=" << ex.what()); factoryDal.RollbackTransaction();
	}
	catch (...)
	{	LOG_ERROR_APP("[FlushEachReport]: FlushDeviceScheduleLinks processing failed! unknown error!"); factoryDal.RollbackTransaction();
	}

	try
	{
		factoryDal.BeginTransaction(); 	FlushDeviceHealth(); factoryDal.CommitTransaction();
	}
	catch (std::exception& ex)
	{ 	LOG_ERROR_APP("[FlushEachReport]: FlushDeviceHealth processing failed! error=" << ex.what()); factoryDal.RollbackTransaction();
	}
	catch (...)
	{	LOG_ERROR_APP("[FlushEachReport]: FlushDeviceHealth processing failed! unknown error!"); factoryDal.RollbackTransaction();
	}

	try
	{
		factoryDal.BeginTransaction(); 	FlushDeviceNeighbHealth(); factoryDal.CommitTransaction();
	}
	catch (std::exception& ex)
	{ 	LOG_ERROR_APP("[FlushEachReport]: FlushDeviceNeighbHealth processing failed! error=" << ex.what()); factoryDal.RollbackTransaction();
	}
	catch (...)
	{	LOG_ERROR_APP("[FlushEachReport]: FlushDeviceNeighbHealth processing failed! unknown error!"); factoryDal.RollbackTransaction();
	}

	try
	{
		factoryDal.BeginTransaction(); 	FlushDeviceNeighbsSignalLevel(); factoryDal.CommitTransaction();
	}
	catch (std::exception& ex)
	{ 	LOG_ERROR_APP("[FlushEachReport]: FlushDeviceNeighbsSignalLevel processing failed! error=" << ex.what()); factoryDal.RollbackTransaction();
	}
	catch (...)
	{	LOG_ERROR_APP("[FlushEachReport]: FlushDeviceNeighbsSignalLevel processing failed! unknown error!"); factoryDal.RollbackTransaction();
	}

	try
	{
		factoryDal.BeginTransaction(); 	FlushAlarms(); factoryDal.CommitTransaction();
	}
	catch (std::exception& ex)
	{ 	LOG_ERROR_APP("[FlushEachReport]: FlushAlarms processing failed! error=" << ex.what()); factoryDal.RollbackTransaction();
	}
	catch (...)
	{	LOG_ERROR_APP("[FlushEachReport]: FlushAlarms processing failed! unknown error!"); factoryDal.RollbackTransaction();
	}

}
void DevicesManager::FlushReports()
{
	LOG_DEBUG_APP("[FlushReports]: begin");

	try
	{
		factoryDal.BeginTransaction();

		FlushGraphs();
		FlushRoutes();
		FlushSourceRoutes();
		FlushServices();
		FlushSuperframes();
		FlushDeviceScheduleLinks();
		FlushDeviceHealth();
		FlushDeviceNeighbHealth();
		FlushDeviceNeighbsSignalLevel();
		FlushAlarms();

		factoryDal.CommitTransaction();
	}
	catch (std::exception& ex)
	{
		LOG_ERROR_APP("[FlushReports]: processing failed! error=" << ex.what() << ", so flush each report separately...");
		factoryDal.RollbackTransaction();
		FlushEachReport();
	}
	catch (...)
	{
		LOG_ERROR_APP("[FlushReports]: processing failed! unknown error!" << ", so flush each report separately...");
		factoryDal.RollbackTransaction();
		FlushEachReport();
	}

	LOG_DEBUG_APP("[FlushReports]: end");
}
bool DevicesManager::IsAnyReportToFlush()
{
	return (m_devicesGraphNeighbsCache.size() + m_routesCache.size() + m_sourceRoutesCache.size()
			+ m_servicesCache.size() + m_superframesCache.size() + m_devicesScheduleLinksCache.size()
			+ m_devicesHealthCache.size() + m_devicesNeighsHealthCache.size() + m_neighbsSignalLevelCache.size()
			+ m_alarmsCache.size()) > 0;	
}

//gateway connection
void DevicesManager::ChangeGatewayStatus(Device::DeviceStatus newStatus)
{
	try
	{
		DevicePtr gateway = GatewayDevice();
		gateway->Status(newStatus);
		if (gateway->Changed())
		{
			factoryDal.Devices().UpdateDevice(*gateway);
			gateway->ResetChanged();
		}
	}
	catch(std::exception& ex)
	{
		LOG_ERROR_APP("ChangeGatewayState: Failed to update gateway status! error=" << ex.what());
	}
	catch(...)
	{
		LOG_ERROR_APP("ChangeGatewayState: Failed to update gateway status! unknown error!");
	}
}
void DevicesManager::HandleGWConnect(const std::string& host, int port)
{
	LOG_INFO_APP("[DevicesManager]:Gateway reconnected ...");
	isGatewayConnected = true;

	ChangeGatewayStatus(Device::dsUnregistered);
	DevicePtr gateway = GatewayDevice();
	if (gateway)
	{
		factoryDal.Devices().AddDeviceConnections(gateway->id, host, port);
	}
	else
	{
		LOG_WARN_APP("Gateway device not found...");
	}
}
void DevicesManager::HandleGWDisconnect()
{
	LOG_WARN_APP("[DevicesManager]: Gateway disconnected!");
	isGatewayConnected = false;

	CleanReports();

	ChangeGatewayStatus(Device::dsNotConnected);
	DevicePtr gateway = GatewayDevice();
	if (gateway)
	{
		factoryDal.Devices().RemoveDeviceConnections(gateway->id);
	}
	else
	{
		LOG_WARN_APP("Gateway device not found...");
	}
	
	ResetTopology();
}




}//hostapp
}//hart7
