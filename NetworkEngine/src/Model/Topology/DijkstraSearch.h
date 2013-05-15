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

#ifndef DIJKSTRASEARCH_H_
#define DIJKSTRASEARCH_H_

#include "Common/NEAddress.h"
#include "Common/logging.h"
#include "Model/Topology/SubnetTopology.h"

namespace NE {

namespace Model {

namespace Topology {

/**
 * Dijkstra algorithm implementation.
 * @author George Petrehus, Ioan-Vasile Pocol
 */
class DijkstraSearch {

    LOG_DEF("I.M.G.DijkstraSearch")

    public:
        static bool searchDijkstra(Address32 source, Address32 destination, NodeMap& Q, AddressList& path);

};

}
}
}

#endif /* DIJKSTRASEARCH_H_ */
