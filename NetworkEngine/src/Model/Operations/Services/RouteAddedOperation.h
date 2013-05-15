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

#ifndef ROUTEADDEDOPERATION_H_
#define ROUTEADDEDOPERATION_H_

#include <list>

#include "Model/Operations/IEngineOperation.h"
#include "Model/Operations/EngineOperation.h"
#include "Model/Topology/TopologyTypes.h"

#include <ApplicationLayer/Model/WirelessNetworkManagerCommands.h>

namespace NE {
namespace Model {
namespace Operations {

#define DLL_PAYLOAD_CAPACITY 100;

class RouteAddedOperation;
typedef boost::shared_ptr<RouteAddedOperation> RouteAddedOperationPointer;

/**
 * This operation is created and sent in the network every time a node(device) is added or removed from
 * the network and the topology is changed. If a new device joins the network then it result a new route.
 * @author: Ioan Pocol
 * @version: 1, 12.06.2008
 */
class RouteAddedOperation: public NE::Model::Operations::EngineOperation {

    public:

        /** the id of the route, the route can be source or graph */
        Uint32 routeId;

        /** even for source route a graphID will be generated, it is mandatory for WirelessHART */
        Uint16 graphId;

        /** the final destination of the route */
        Address32 peerAddress;

        C974_WriteRoute_RespCodes writeRouteResponseCode;

    public:

        RouteAddedOperation(Address32 owner, Uint32 routeId, Uint16 graphId, Address32 peerAddress);

        ~RouteAddedOperation();

        EngineOperationType::EngineOperationTypeEnum getOperationType();

        std::string getName() {
            return "Route Add";
        }

        Uint32 getRouteId();

        void toStringInternal(std::ostringstream &stream);

        virtual void setResponseCode(Uint8 responseCode);

        virtual Uint8 getResponseCode();

        bool accept(NE::Model::Operations::IEngineOperationsVisitor& visitor);
};

}
}
}

#endif /* ROUTEADDEDOPERATION_H_ */
