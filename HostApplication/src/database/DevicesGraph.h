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

#ifndef DEVICESGRAPH_H_
#define DEVICESGRAPH_H_

#include <list>
#include <vector>
#include <WHartHost/model/Device.h>
#include <WHartHost/model/reports/NeighborHealth.h>


namespace hart7 {
namespace hostapp {

//this class creates a graph to enable algorithms such as BFS (Breadth First Search) and 
	//DFS (Depth First Search) to be performed
class DevicesGraph
{
//
public:
	struct NeighbourData
	{
		short			vertexNo;
		NeighborHealth	health;

	};

//
private:
	struct VertexData
	{
		DevicePtr					dev;
		std::list<NeighbourData>	neighbours;
	};

private:
	std::vector<VertexData>	m_vertices;


//vertices
public:
	//0 -ok
	//1 -already set
	int SetVerticesNo(int n)
	{
		if (m_vertices.size() > 0)
			return 1;
		m_vertices.resize(n);
		return 0;
	}
	int GetVerticesNo()
	{
		return m_vertices.size();
	}

//vertex
public:
	void AddDataToVertex(int i, DevicePtr dev)
	{
		m_vertices[i].dev = dev; 
	}
	DevicePtr GetVertexData(int i)
	{
		return m_vertices[i].dev;
	}

//arc
public:
	void AddArc(int i, int j, const NeighborHealth &health)
	{
		m_vertices[i].neighbours.push_back(NeighbourData());
		NeighbourData &neighbour = *m_vertices[i].neighbours.rbegin();
		neighbour.vertexNo = j;
		neighbour.health = health;
	}

	bool IsArc(int from, int to)
	{
		for (std::list<NeighbourData>::const_iterator i = m_vertices[from].neighbours.begin(); 
					i != m_vertices[from].neighbours.end(); ++i)
		{
			if ((*i).vertexNo == to)
				return true;
		}
		return false;
	}

//neighbours
public:
	std::list<NeighbourData>& GetNeighbours(int i) //the return value should not be this way (break the principle of OOP) but is good for optimization
	{
		return m_vertices[i].neighbours;
	}

	
	
};

}
}

#endif 
