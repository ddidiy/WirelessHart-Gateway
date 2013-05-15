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

#ifndef TOPOLOGYREPORT_H_
#define TOPOLOGYREPORT_H_


#include <WHartHost/model/NickName.h>
#include <WHartHost/model/MAC.h>
#include <WHartHost/model/reports/NeighborHealth.h>

#include <boost/cstdint.hpp> //used for inttypes

#include <list>
#include <vector>
#include <map>
#include <string>


namespace hart7 {
namespace hostapp {

struct GraphNeighbor
{
	GraphNeighbor(int toDevId, int graphId, int index):m_toDevId(toDevId),m_graphId(graphId),m_index(index){m_toDelete = false;}
	GraphNeighbor(int toDevId, int graphId):m_toDevId(toDevId),m_graphId(graphId){m_toDelete = true;}

	//
	bool m_toDelete;  //used in caching process

	int m_toDevId;
	int m_graphId;
	int m_index;
};

struct DeviceGraphNeighbors
{
	DeviceGraphNeighbors(int deviceID_, std::list<GraphNeighbor>& neighbs_):
		deviceID(deviceID_), neighbs(neighbs_)
	{
	}

	DeviceGraphNeighbors(int deviceID_, const GraphNeighbor& neighb_):
	deviceID(deviceID_)
	{
		neighbs.push_back(neighb_);
	}

	int deviceID;
	std::list<GraphNeighbor> neighbs;
};

typedef std::list<DeviceGraphNeighbors> DevicesGraphNeighborsListT;


class TopologyReport
{
public:
	TopologyReport()
	{
		m_GWIndex = NO_TOPO_GW_INDEX;
	}

public:
	static const int NO_TOPO_GW_INDEX = 0xFFFFFFFF;
	static const int NO_TOP0_NEIGHB_INDEX = 0xFFFFFFFF;

public:
	struct Device
	{
		struct Neighbour
		{
			NickName		DeviceNickName;
			NeighborHealth	Health;

			int				DevListIndex;  //bound the neighbour to the coresponding device in 'DevList',
										   //information used in creation of devices graph
			Neighbour()
			{
				DevListIndex = NO_TOP0_NEIGHB_INDEX; //invalid
			}
		};

		struct Graph
		{
			boost::uint16_t			GraphID;
			std::vector<NickName>	neighbList;
		};

		typedef std::vector<Neighbour> NeighborsListT;
		typedef std::vector<Graph> GraphsListT;

		NickName	DeviceNickName;
		MAC			DeviceMAC;
		
		boost::uint8_t	DeviceType;
		boost::uint8_t	PowerSupplyStatus;

		boost::uint16_t BatteryLifeStatus;
		boost::uint16_t NotificationMask;	//only for new devices is used in processing

		int				softwareRev;
		int				deviceCode;
		int				joinCount;
	    bool            adapter;

		std::string		LongTag;

		NeighborsListT NeighborsList;
		GraphsListT GraphsList;

		Device() :
		    PowerSupplyStatus(0), BatteryLifeStatus(0), NotificationMask(0)
		{
		    joinCount=0;
		}
	};
	typedef std::vector<Device> DevicesListT;

	DevicesListT		m_DevicesList;
	int					m_GWIndex;		//for optimization purpose
};

}
}

#endif
