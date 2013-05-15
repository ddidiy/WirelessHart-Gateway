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


#include "ReportsProcessor.h"

#include <list>
#include <queue>

namespace hart7 {
namespace hostapp {


/*TOPOLOGY_REPORT*/

void ReportsProcessor::SkipNeighboursforDevNN(TopoStructIndexT::const_iterator &i/*[in/out]*/, const TopologyReport::DevicesListT &TopoStruct)
{
	assert(i != m_TopoStructIndex.end());
	assert(i->second.indexDevice >= 0 && i->second.indexDevice < (int)TopoStruct.size());
	assert(i->second.indexNeighbour >= -1 /*no neighbour*/ && i->second.indexNeighbour < (int)TopoStruct[i->second.indexDevice].NeighborsList.size());

	int indexDevice = (i++)->second.indexDevice;
	while(i != m_TopoStructIndex.end())
	{
		if (i->second.indexDevice != indexDevice)
			break;
		++i;
	}
}


void ReportsProcessor::CreateIndexForTopoStruct(const TopologyReport::DevicesListT &TopoStruct)
{
	LOG_INFO_APP("sorted_topo_struct_fill -> found topo_rep with size = " << (boost::uint32_t)TopoStruct.size());
	
	LOG_DEBUG_APP("sorted_topo_struct_fill--> begin --------");
	
	int indexDevice = 0;
	m_TopoStructIndex.clear();
	for(TopologyReport::DevicesListT::const_iterator i = TopoStruct.begin(); 
					i != TopoStruct.end(); ++i)
	{ 

		int indexNeigbour = 0;
		LOG_DEBUG_APP("sorted_topo_struct_fill--> 1: indexDevice=" << indexDevice << " indexNeigbour=" << indexNeigbour);
		for(TopologyReport::Device::NeighborsListT::const_iterator y = i->NeighborsList.begin();
			y != i->NeighborsList.end(); ++y)
		{
			TopologyIndexKey key;
			key.deviceNN = i->DeviceNickName;
			key.neighbourNN = y->DeviceNickName;
			TopologyIndexData data;
			data.indexDevice = indexDevice;
			data.indexNeighbour = indexNeigbour;

			if (m_TopoStructIndex.insert(TopoStructIndexT::value_type(key, data)).second == false)
			{
				LOG_WARN_APP("duplicated neighbour for topology -> dev_nn = " << i->DeviceNickName 
					<< " and neighbour_nn = " << y->DeviceNickName);
			}
			LOG_DEBUG_APP("sorted_topo_struct_fill--> 2: indexDevice=" << indexDevice << " indexNeigbour=" << indexNeigbour);
			indexNeigbour++;
		}
	
		if (indexNeigbour == 0)
		{
			TopologyIndexKey key;
			key.deviceNN = i->DeviceNickName;
			TopologyIndexData data;
			data.indexDevice = indexDevice;
			
			if (m_TopoStructIndex.insert(TopoStructIndexT::value_type(key, data)).second == false)
			{
				LOG_WARN_APP("duplicated device for topology -> dev_nn = " << i->DeviceNickName);
			}
		}
		
		indexDevice++;
	}
	LOG_DEBUG_APP("sorted_topo_struct_fill--> end --------");
}

struct TopoNeighbIndex
{
	int		devListIndex;
	int		neighbArrayIndex;
};

struct TopoNeighbour
{
	NickName					deviceNN;
	std::list<TopoNeighbIndex>	indexesList;
};

static void BuildNeighboursList(ReportsProcessor::TopoStructIndexT::const_iterator &i/*[in/out]*/,
								ReportsProcessor::TopoStructIndexT::const_iterator iEnd,
								std::list<TopoNeighbour> &neighboursList/*[in/out]*/)
{
	assert(i->second.indexNeighbour != -1 /*no neighbour*/);

	std::list<TopoNeighbour>::iterator j = neighboursList.begin();
		
	int indexDevice = i->second.indexDevice;
	do	//perform for neighbours
	{
		if (j == neighboursList.end())
		{
			LOG_DEBUG_APP("build_topo_neighbours_list - neighbour dev_nn = " <<  i->first.neighbourNN <<
					" NOT FOUND in list, so add it to the list.");
			
			neighboursList.push_back(TopoNeighbour());
			TopoNeighbour &neighbour = *neighboursList.rbegin();
			neighbour.deviceNN = i->first.neighbourNN;
			neighbour.indexesList.push_back(TopoNeighbIndex());
			TopoNeighbIndex &index = *neighbour.indexesList.rbegin();
			index.devListIndex = i->second.indexDevice;
			index.neighbArrayIndex = i->second.indexNeighbour;
			++i;
			if (i == iEnd)
				break;
			continue;
		}
		if (i->first.neighbourNN == j->deviceNN)
		{
			//found a match...
		}
		else 
		{
			if (j->deviceNN < i->first.neighbourNN)
			{
				do
				{
					++j;
					if (j == neighboursList.end())
						break;
				}while (j->deviceNN < i->first.neighbourNN);
				if (j == neighboursList.end())
				{
					LOG_DEBUG_APP("build_topo_neighbours_list - neighbour dev_nn = " <<  i->first.neighbourNN <<
						" NOT FOUND in list, so add it to the list.");
					neighboursList.push_back(TopoNeighbour());
					TopoNeighbour &neighbour = *neighboursList.rbegin();
					neighbour.deviceNN = i->first.neighbourNN;
					neighbour.indexesList.push_back(TopoNeighbIndex());
					TopoNeighbIndex &index = *neighbour.indexesList.rbegin();
					index.devListIndex = i->second.indexDevice;
					index.neighbArrayIndex = i->second.indexNeighbour;
					++i;
					if (i == iEnd)
						break;
					continue;
				}
				if (i->first.neighbourNN < j->deviceNN)
				{
					LOG_DEBUG_APP("build_topo_neighbours_list - neighbour dev_nn = " <<  i->first.neighbourNN <<
						" NOT FOUND in list, so add it to the list.");
					TopoNeighbour neighbour;
					neighbour.deviceNN = i->first.neighbourNN;
					neighbour.indexesList.push_back(TopoNeighbIndex());
					TopoNeighbIndex &index = *neighbour.indexesList.rbegin();
					index.devListIndex = i->second.indexDevice;
					index.neighbArrayIndex = i->second.indexNeighbour;
					neighboursList.insert(j, neighbour);
					++i;
					if (i == iEnd)
						break;
					continue;
				}
				
				//found a match...
			}
			else 
			{
				LOG_DEBUG_APP("build_topo_neighbours_list - neighbour dev_nn = " <<  i->first.neighbourNN <<
						" NOT FOUND in list, so add it to the list.");
				TopoNeighbour neighbour;
				neighbour.deviceNN = i->first.neighbourNN;
				neighbour.indexesList.push_back(TopoNeighbIndex());
				TopoNeighbIndex &index = *neighbour.indexesList.rbegin();
				index.devListIndex = i->second.indexDevice;
				index.neighbArrayIndex = i->second.indexNeighbour;
				neighboursList.insert(j, neighbour);
				++i;
				if (i == iEnd)
					break;
				continue;
			}
		}
		
		j->indexesList.push_back(TopoNeighbIndex ());
		TopoNeighbIndex &index = *j->indexesList.rbegin();
		index.devListIndex = i->second.indexDevice;
		index.neighbArrayIndex = i->second.indexNeighbour;

		LOG_DEBUG_APP("build_topo_neighbours_list - neighbour_nn = " << i->first.neighbourNN <<
				"indexDevice = " << i->second.indexDevice <<
				"indexNeighbour = " << i->second.indexNeighbour <<
				" added to the list ");
		//increment here
		++i;
		++j;
		if (i == iEnd)
			break;
	}while (i->second.indexDevice == indexDevice);
	
}

static void SetDevListIndexForNeighbours(TopologyReport::DevicesListT &TopoStruct /*[in/out]*/, int topoDevIndex, 
										   std::list<TopoNeighbIndex> &indexesList)
{
	assert (indexesList.size() > 0);

	while (indexesList.size())
	{
		TopoStruct[indexesList.front().devListIndex].NeighborsList[
			indexesList.front().neighbArrayIndex].DevListIndex  = topoDevIndex;
		indexesList.pop_front();
	}
}

void ReportsProcessor::FillDevListIndexForNeighbours(TopologyReport::DevicesListT &TopoStruct /*[in/out]*/)
{
	std::list<TopoNeighbour> neighboursList;
	
	LOG_DEBUG_APP("build_topo_neighbours_list--> begin --------");
	for(TopoStructIndexT::const_iterator i = m_TopoStructIndex.begin(); i != m_TopoStructIndex.end(); )
	{ //for every neighbour add devListIndex
		
		LOG_DEBUG_APP("build_topo_neighbours_list--> for dev_nn = " << i->first.deviceNN);
		if (i->second.indexNeighbour != -1 /*no neighbour*/)
		{
			BuildNeighboursList(i, m_TopoStructIndex.end(), neighboursList);
			continue;
		}
		++i;
	}
	LOG_DEBUG_APP("build_topo_neighbours_list--> end --------");


	LOG_DEBUG_APP("topo_struct_neighbour_fill--> begin --------");
	std::list<TopoNeighbour>::iterator j = neighboursList.begin();

	for(TopoStructIndexT::const_iterator i = m_TopoStructIndex.begin(); i != m_TopoStructIndex.end(); )
	{ 
		
		if (j == neighboursList.end())
		{
			LOG_WARN_APP("topo_struct_neighbour_fill - dev_nn = " <<  i->first.deviceNN <<
	 				" NOT FOUND in neighbours_list so skip it");
			SkipNeighboursforDevNN(i, TopoStruct);
			continue;
		}
		if (i->first.deviceNN == j->deviceNN)
		{
			//found a match...
		}
		else 
		{
			if (j->deviceNN < i->first.deviceNN)
			{
				do
				{
					++j;
					if (j == neighboursList.end())
						break;
				}while (j->deviceNN < i->first.deviceNN);
				if (j == neighboursList.end())
				{
					LOG_WARN_APP("topo_struct_neighbour_fill - dev_nn = " <<  i->first.deviceNN <<
		 					" NOT FOUND in neighbours_list so skip it");
					SkipNeighboursforDevNN(i, TopoStruct);
					continue; // continue 'for'
				}
				if (i->first.deviceNN < j->deviceNN)
				{
					LOG_WARN_APP("topo_struct_neighbour_fill - dev_nn = " <<  i->first.deviceNN <<
		 					" NOT FOUND in neighbours_list so skip it");
					SkipNeighboursforDevNN(i, TopoStruct);
					continue;
				}
				
				//found a match...
			}
			else 
			{
				LOG_WARN_APP("topo_struct_neighbour_fill - dev_nn = " <<  i->first.deviceNN <<
		 				" NOT FOUND in neighbours_list so skip it");
				SkipNeighboursforDevNN(i, TopoStruct);
				continue;
			}
		}

		//set dev_list_index for every neighbour
		SetDevListIndexForNeighbours(TopoStruct, i->second.indexDevice, j->indexesList);

		//increment here
		SkipNeighboursforDevNN(i, TopoStruct);
		++j;
	}
	LOG_DEBUG_APP("topo_struct_neighbour_fill--> end --------");
}


}
}
