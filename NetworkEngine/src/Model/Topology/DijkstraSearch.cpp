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

#include "DijkstraSearch.h"
#include <limits.h>

using namespace NE::Model::Topology;

bool DijkstraSearch::searchDijkstra(Address32 source, Address32 destination, NodeMap& nodes, AddressList& path) {

    //TODO: ivp - some optimization by put distance and parent on Node, and implement the NodeList as an array with
    //address32 as index with 0-255 range

    std::map<Address32, int> distance;
    std::map<Address32, Address32> parent;
    AddressList Q;

    int min;

    for (NodeMap::iterator it = nodes.begin(); it != nodes.end(); ++it) {
        distance[it->first] = USHRT_MAX;
        parent[it->first] = it->first;
        Q.push_back(it->first);
    }

    distance[source] = 0;

    Address32 u;

    while (!Q.empty()) {

        AddressList::iterator itDelete;
        min = USHRT_MAX;
        for (AddressList::iterator it = Q.begin(); it != Q.end(); ++it) {
            if (distance[*it] <= min) {
                min = distance[*it];
                u = *it;
                itDelete = it;
            }
        }

        Q.erase(itDelete);

        Node& node = nodes[u];

        EdgeList edges = node.getOutBoundEdges();

        for (EdgeList::iterator it = edges.begin(); it != edges.end(); ++it) {
            if (it->getStatus() == Status::DELETED) {
                continue;
            }

            Address32 address = it->getDestination();

            if (distance[address] > distance[u] + it->getEvalEdgeCost()) {
                distance[address] = distance[u] + it->getEvalEdgeCost();
                parent[address] = u;
            }
        }
    }

    //there is no path between nodes
    if (parent[destination] == destination) {
        return false;
    }

    Address32 crtAddress = destination;
    path.push_back(crtAddress);

    while (crtAddress != source) {
        crtAddress = parent[crtAddress];
        path.insert(path.begin(), crtAddress);
    }

    return true;
}

