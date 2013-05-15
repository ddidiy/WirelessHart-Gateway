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

/*
 * EdgeGraph.h
 *
 *  Created on: May 11, 2009
 *      Author: ioanpocol
 */

#ifndef GRAPHNEIGHBOR_H_
#define GRAPHNEIGHBOR_H_

#include <map>
#include "Model/Topology/TopologyTypes.h"

namespace NE {

namespace Model {

namespace Topology {

class GraphNeighbor;
typedef std::map<Uint16, GraphNeighbor> GraphNeighborMap;

class GraphNeighbor {

    public:

        Uint16 traffic;

        bool preffered;

        bool lazy;

        bool retry;

        Status::StatusEnum status;

        GraphNeighbor() {
            preffered = false;
            lazy = false;
            retry = false;
        }

        GraphNeighbor(Uint16 traffic_, bool preffered_) :
            traffic(traffic_), preffered(preffered_) {
            lazy = false;
            retry = false;
            status = Status::NEW;
        }

        /**
         * Returns a string representation of this EdgeGraph.
         */
        friend std::ostream& operator<<(std::ostream& stream, const GraphNeighbor& graphNeighbor) {
            stream << " traffic=" << graphNeighbor.traffic << ", preferred = "
                        << graphNeighbor.preffered << ", lazy=" << (int) graphNeighbor.lazy
                        << ", retry=" << (int) graphNeighbor.retry << ", status="
                        << graphNeighbor.status;
            return stream;
        }

};

}
}
}

#endif /* GRAPHNEIGHBOR_H_ */
