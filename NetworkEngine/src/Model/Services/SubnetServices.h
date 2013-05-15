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

#ifndef SUBNETSERVICES_H_
#define SUBNETSERVICES_H_

#include <utility>
#include <map>
#include "Common/NEAddress.h"
#include "Common/NETypes.h"
#include "Common/logging.h"
#include "Model/Services/NodeService.h"
#include "Model/Operations/EngineOperations.h"
#include "Model/Operations/Services/ServiceAddedOperation.h"
#include "Model/Operations/Services/ServiceRemovedOperation.h"
#include "Model/Operations/Services/RouteAddedOperation.h"
#include "Model/Operations/Services/RouteRemovedOperation.h"
#include "Model/Operations/Services/SourceRouteAddedOperation.h"
#include "Model/Operations/Services/SourceRouteRemovedOperation.h"

using namespace NE::Common;

namespace NE {
namespace Model {
namespace Services {

class SubnetServices {

    LOG_DEF("I.M.SubnetServices");

    private:

        NodeServiceMap nodeServices;

        NodeServiceMap::iterator managerService;
        NodeServiceMap::iterator gatewayService;

        NodeService dummy;

    public:

        SubnetServices();

        virtual ~SubnetServices();

        /**
         *
         */
        void init();

        /**
         * Allocate join services (service and route)
         */
        void addNode(NE::Model::Operations::EngineOperations& engineOperations, Address32 parentAddress,
                    Address32 address, NE::Model::Topology::NodeType::NodeTypeEnum nodeType, Uint16 inboundGraphId,
                    Uint16 outboundGraphId, Uint32 period, Uint32& inboundServiceId, Uint32& outboundServiceId);

        /**
         *
         */
        NodeServiceMap& getNodeServiceMap() {
            return nodeServices;
        }

        /**
         * Find the management service from manager to device
         */
        Service& findManagementService(const Address32& destination, bool isProxyDestination);

        /**
         * On gateway rejoin the settings are not released, the already establish settings
         * are sent to the gateway, in this case Services
         */
        void regenerateGatewayOperations(NE::Model::Operations::EngineOperations& engineOperations);

        /**
         *
         */
        void generateProxyService(NE::Model::Operations::EngineOperations& engineOperations, Address32 address,
                    AddressList& destinations, Uint16 graphId, Uint32 period);

        /**
         *
         */
        bool existsService(Uint32 serviceId);

        /**
         *
         */
        bool allocateService(Service& serviceRequest, Uint32 graphId, bool generateServiceOperation = true);

        /**
         *
         */
        bool terminateService(Address32 address, ServiceId serviceId, bool& inbound, bool& outbound,
                    Address32& nodeAddress);

        /**
         *
         */
        bool finishEvaluation(Address32 address, Uint32 graphId, bool isOutbound);

        /**
         *
         */
        void terminateService(ServiceId requestServiceId);

        /**
         *
         */
        bool resolveOperation(NE::Model::Operations::ServiceAddedOperation& operation);

        /**
         *
         */
        bool resolveOperation(NE::Model::Operations::ServiceRemovedOperation& operation);

        /**
         *
         */
        bool resolveOperation(NE::Model::Operations::RouteAddedOperation& operation);

        /**
         *
         */
        bool resolveOperation(NE::Model::Operations::SourceRouteAddedOperation& operation);

        /**
         *
         */
        bool resolveOperation(NE::Model::Operations::RouteRemovedOperation& operation);

        /**
         *
         */
        bool resolveOperation(NE::Model::Operations::SourceRouteRemovedOperation& operation);

        /**
         * Return true if the status is REMOVED
         */
        bool isRemovedStatus(Address32 address);

        /**
         * Set the status of the nod as REMOVED
         */
        void setRemovedStatus(Address32 address);

        /**
         * Return true if the device is prepared to act as a router
         */
        bool canSendAdvertise(Address32 address);

        /**
         *
         */
        bool getManagementServiceIds(Address32 address, ServiceId& inboundServiceId, ServiceId& outboundServiceId);

        /**
         *
         */
        void releaseRemoved(NE::Model::Operations::EngineOperations& engineOperations);

        /**
         *
         */
        bool allocateResources(NE::Model::Operations::EngineOperations& engineOperations, Uint16 graphId,
                                Address32 source, Address32 destination, bool changeHandler, Uint32& oldHandler,
                                Uint32& newHandler);

        /**
         *
         */
        Uint16 computeOutboundTraffic(Uint16 graphId, Address32 source, Address32 destination);

        /**
        *
        */
        Uint16 computeInboundTraffic(Uint16 graphId, Address32 source, Address32 destination, std::multiset<Uint16>& compactPublishPeriods);

        /**
         * Returns the handler used to send the check links operations.
         */
        Uint32 getCheckHandler();

        /**
         *
         */
        void toString(std::ostringstream& stream);

};

}
}
}

#endif /*SUBNETSERVICES_H_*/
