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

#ifndef NODESLIST_H_
#define NODESLIST_H_

#include <WHartHost/model/Device.h>

#include <map>
#include <nlib/log.h>


namespace hart7 {
namespace hostapp {

class NodesRepository;
typedef boost::shared_ptr<NodesRepository> NodesRepositoryPtr;

/*
 * devices cache
 */
class NodesRepository
{
	LOG_DEF("hart7.hostapp.runtime.NodesRepository");
	
public:
	typedef std::map<MAC, DevicePtr> NodesByMACT;
	typedef std::map<NickName, DevicePtr> NodesByNickNameT;
	typedef std::map<boost::int32_t, DevicePtr> NodesByIDT;

//search
public:
	DevicePtr Find(const MAC& macAddress) const;
	DevicePtr Find(const boost::int32_t deviceID) const;
	DevicePtr Find(const NickName& nickname) const;

	DevicePtr FindRegistered(const NickName& nickname) const;

//insert
public:
	int Add(const DevicePtr& node);	//0 - fail
									//1 -ok

//update
public:
	void UpdateRegisteredNickNameIndex();
	void UpdateNickNameIndex();
	void UpdateIdIndex();

//clear
public:
	void Clear();	

//
public:
	NodesByMACT allNodesByMAC;
	NodesByIDT allNodesById;
	NodesByNickNameT registeredNodesByNickName;
	NodesByNickNameT allNodesByNickName;
};

} //namespace hostapp
} //namespace hart7

#endif /*NODESLIST_H_*/
