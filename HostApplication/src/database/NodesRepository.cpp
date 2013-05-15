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


#include "NodesRepository.h"

namespace hart7 {
namespace hostapp {


//search
DevicePtr NodesRepository::Find(const MAC& mac) const
{
	NodesByMACT::const_iterator found = allNodesByMAC.find(mac);
	if (found != allNodesByMAC.end())
	{
		return found->second;
	}
	return DevicePtr();
}

DevicePtr NodesRepository::FindRegistered(const NickName& nickName) const
{
	NodesByNickNameT::const_iterator found = registeredNodesByNickName.find(nickName);
	if (found != registeredNodesByNickName.end())
	{
		return found->second;
	}
	return DevicePtr();
}

DevicePtr NodesRepository::Find(const NickName& nickName) const
{
	NodesByNickNameT::const_iterator found = allNodesByNickName.find(nickName);
	if (found != allNodesByNickName.end())
	{
		return found->second;
	}
	return DevicePtr();
}

DevicePtr NodesRepository::Find(const boost::int32_t deviceID) const
{
	NodesByIDT::const_iterator found = allNodesById.find(deviceID);
	if (found != allNodesById.end())
	{
		return found->second;
	}
	return DevicePtr();
}

//insert
int NodesRepository::Add(const DevicePtr& node)
{
	if (!allNodesByMAC.insert(NodesByMACT::value_type(node->Mac(), node)).second)
	{
		LOG_WARN_APP("Same MAC:" << node->Mac() << "was detected (will be ignored)!");
		return 0;
	}
	if (!allNodesByNickName.insert(NodesByNickNameT::value_type(node->Nickname(), node)).second)
	{
		LOG_WARN_APP("Same Nickname:" << node->Nickname() << "was detected (will be ignored)!");
		return 0;
	}
	return 1;
}

//update
void NodesRepository::UpdateRegisteredNickNameIndex()
{
	registeredNodesByNickName.clear();

	for (NodesByMACT::const_iterator it = allNodesByMAC.begin(); it!= allNodesByMAC.end(); it++)
	{
		if (it->second->Status() == Device::dsRegistered)
		{
			if (!registeredNodesByNickName.insert(NodesByNickNameT::value_type(it->second->Nickname(), it->second)).second)
			{
				LOG_WARN_APP("Same registered NickName:" << it->second->Nickname()<< "was detected (will be ignored)!");
			}
		}
	}
}
void NodesRepository::UpdateNickNameIndex()
{
	allNodesByNickName.clear();

	for (NodesByMACT::const_iterator it = allNodesByMAC.begin(); it!= allNodesByMAC.end(); it++)
	{		
		if (!allNodesByNickName.insert(NodesByNickNameT::value_type(it->second->Nickname(), it->second)).second)
		{
			LOG_WARN_APP("Same NickName:" << it->second->Nickname()<< "was detected (will be ignored)!");
		}
	}
}
void NodesRepository::UpdateIdIndex()
{
	allNodesById.clear();
	for (NodesByMACT::const_iterator it = allNodesByMAC.begin(); it != allNodesByMAC.end(); it++)
	{
		if (!allNodesById.insert(NodesByIDT::value_type(it->second->id, it->second)).second)
		{
			LOG_WARN_APP("Same DeviceID:" << it->second->id << "was detected (will be ignored)!");
		}
	}
}

//clear
void NodesRepository::Clear()
{
	allNodesByNickName.clear();
	registeredNodesByNickName.clear();
	allNodesById.clear();
	allNodesByMAC.clear();
}

} //namspace 
} //namespace 
