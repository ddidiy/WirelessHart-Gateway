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
 * GraphRoutingAlgorithmInterface.h
 *
 *  Created on: 17.08.2009
 *      Author: radu.pop
 */

#ifndef GRAPHROUTINGALGORITHMINTERFACE_H_
#define GRAPHROUTINGALGORITHMINTERFACE_H_

#include "Model/Topology/Edge.h"
#include "Model/Topology/Path.h"
#include "Model/Topology/Node.h"

namespace NE {

namespace Model {

namespace Topology {

/**
 * This interface specifies the interface for algorithms that determines the routing graphs from
 * a source to a destination. These algorithms work directly on the NetworkEngine model.
 */
class GraphRoutingAlgorithmInterface {

    public:

        GraphRoutingAlgorithmInterface() {
        }

        virtual ~GraphRoutingAlgorithmInterface() {
        }

        /**
         * Select the nodes&edges that are potential part of the graph.
         */
        virtual bool evaluateGraphPath(NE::Model::Operations::EngineOperations& engineOperations, Path& path) = 0;

};

}

}

}
#endif /* GRAPHROUTINGALGORITHMINTERFACE_H_ */
