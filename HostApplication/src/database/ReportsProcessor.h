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


#ifndef REPORTSPROCESSOR_H_
#define REPORTSPROCESSOR_H_


#include <WHartHost/model/reports/TopologyReport.h>


namespace hart7 {
namespace hostapp {


class ReportsProcessor
{

/*TOPOLOGY_REPORT*/
public:
	struct TopologyIndexKey	//used for ordering, searching the elements in a structure
	{
		NickName	deviceNN;
		NickName	neighbourNN;

		bool operator < (const TopologyIndexKey &obj)const
		{
			if(deviceNN < obj.deviceNN)
				return true;
			if(deviceNN == obj.deviceNN)
				return neighbourNN < obj.neighbourNN;
			return false;
		}
		bool operator == (const TopologyIndexKey &obj)
		{
			if(deviceNN == obj.deviceNN)
				return neighbourNN == obj.neighbourNN;
			return false;
		}
	};

	struct TopologyIndexData
	{
		int		indexDevice;	//>= 0
		int		indexNeighbour; //= -1 no neighbour
								//>= 0 neighbour exist

		TopologyIndexData()
		{
			indexDevice = -1;		//no device (invalid data -> force programming errors)
			indexNeighbour = -1;
		}
	};

	typedef std::map<TopologyIndexKey, TopologyIndexData> TopoStructIndexT;

private:
	TopoStructIndexT	m_TopoStructIndex;	//it ensures the intersection of topology_report 
											//  with other reports has linear complexity
																						
private:
	void SkipNeighboursforDevNN(TopoStructIndexT::const_iterator &i/*[in/out]*/, const TopologyReport::DevicesListT &TopoStruct);
	
public:
	TopoStructIndexT* GetTopoStructIndex()	//it is not correct from OOP perspective but it works for performance
	{
		return &m_TopoStructIndex;
	}

public:
	void CreateIndexForTopoStruct(const TopologyReport::DevicesListT &TopoStruct);	// O((m+n)log(m+n)), m = no. of outbound_links, n = no. of nodes without outbound_links
	void FillDevListIndexForNeighbours(TopologyReport::DevicesListT &TopoStruct /*[in/out]*/);  // O(m + n)), m = no. of outbound_links, n = no. of nodes without outbound_links
};


}
}

#endif 
