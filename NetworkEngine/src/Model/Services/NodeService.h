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
 * NodeService.h
 *
 *  Created on: Sep 21, 2009
 *      Author: ioanpocol
 */

#ifndef NODESERVICE_H_
#define NODESERVICE_H_

#include <list>
#include <set>
#include "Model/Services/Service.h"

#include "Common/NEAddress.h"
#include "Common/NETypes.h"
#include "Common/logging.h"
#include "Model/Services/RouteService.h"
#include "Model/Topology/TopologyTypes.h"
#include "Model/Tdma/TdmaTypes.h"
#include "Model/Operations/EngineOperations.h"
#include "Model/Operations/Services/ServiceAddedOperation.h"
#include "Model/Operations/Services/ServiceRemovedOperation.h"
#include "Model/Operations/Services/RouteAddedOperation.h"
#include "Model/Operations/Services/RouteRemovedOperation.h"
#include "Model/Operations/Services/SourceRouteAddedOperation.h"
#include "Model/Operations/Services/SourceRouteRemovedOperation.h"

namespace NE {
namespace Model {
namespace Services {

using namespace Tdma;

class NodeService;
typedef std::map<Address32, NodeService> NodeServiceMap;

class NodeService {

    LOG_DEF("I.M.NodeService");

    private:

        Uint16 lastServiceId;

        // this will be set true first time the lastServiceId goes after 255
        bool reuseServiceId;

        Uint16 lastRouteId;

        // this will be set true first time the lastRouteId goes after 255
        bool reuseRouteId;

        Address32 address;

        Address32 parentAddress;

        RouteList routes;

        ServiceList services;

        NE::Model::Topology::NodeType::NodeTypeEnum nodeType;

        Status::StatusEnum status;

        int startServiceId;

        PublishPeriod::PublishPeriodEnum toPublishPeriod(Uint16 traffic);

        Uint16 compactMultiset(std::multiset<Uint16>& nonCompactPublishPeriods, std::multiset<Uint16>& compactPublishPeriods);

    public:

        NodeService();

        NodeService(Address32 address, Address32 parentAddress, NE::Model::Topology::NodeType::NodeTypeEnum nodeType);

        /**
         *
         */
        virtual ~NodeService();

        ServiceList& getServices() {
            return this->services;
        }

        /**
         *
         */
        RouteList& getRouteList() {
            return this->routes;
        }

        /**
         *
         */
        Uint32 getNextRouteId();

        /**
         *
         */
        Uint32 getNextServiceId();


        bool isNodeType() {
            return nodeType == NE::Model::Topology::NodeType::NODE;
        }

        /**
         *
         */
        Status::StatusEnum& getStatus();

        /**
         *
         */
        Service& getService(Uint32 serviceId);

        /**
         *
         */
        RouteService& getRouteService(Uint32 routeId);

        /**
         *
         */
        void setStatus(Status::StatusEnum status);

        /**
         *
         */
        Uint32 addInboundManagementService(NE::Model::Operations::EngineOperations& engineOperations, Uint16 graphId,
                    Uint32 period);

        /**
         *
         */
        Uint32 addOutboundManagementService(NE::Model::Operations::EngineOperations& engineOperations,
                    Address32 address, NE::Model::Topology::NodeType::NodeTypeEnum nodeType, Address32 parentAddress,
                    Uint16 graphId, Uint32 period);

        /**
         *
         */
        void renewServices(NE::Model::Operations::EngineOperations& engineOperations);

        /**
         *
         */
        bool addService(Service& serviceRequest, Uint32 graphId, bool generateServiceOperation = true);

        /**
         *
         */
        bool terminateService(ServiceId serviceId, Address32& peerAddress, ServiceId& requestServiceId);

        /**
         *
         */
        bool terminateService(ServiceId requestServiceId);

        /**
         *
         */
        Service& findManagementService(Address32 destination, bool isProxyDestination);

        /**
         *
         */
        bool finishEvaluation(Uint32 graphId);

        /**
         *
         */
        bool allocateResources(NE::Model::Operations::EngineOperations& engineOperations, Uint16 graphId,
                    Address32 destination, bool changeHandler, Uint32& oldHandler, Uint32& newHandler);

        /**
         *
         */
        Uint16 computeInboundTraffic(Uint16 graphId, Address32 destination, std::multiset<Uint16>& compactPublishPeriods);

        /**
         *
        */
        Uint16 computeOutboundTraffic(Uint16 graphId, Address32 destination);

        /**
         *
         */
        void releaseRemoved(NE::Model::Operations::EngineOperations& engineOperations);

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
         * Return true if the device can advertise
         */
        bool canSendAdvertise(Address32 peerAddress);

        /**
         *
         */
        bool getManagementService(ServiceId& serviceId, Address32 peerAddress);

        /**
         *
         */
        void generateProxyService(NE::Model::Operations::EngineOperations& engineOperations, Address32 address, AddressList& destinations, Uint16 graphId, Uint32 period);

        /**
         *
         */
        bool existsService(Uint32 serviceID);

        /**
         * Returns the change hander.
         */
        Uint32 getCheckHandler();

        /**
         *
         */
        void servicesToString(std::ostringstream& stream);

        /**
         *
         */
        void routesToString(std::ostringstream& stream);
};

}
}
}

#endif /* NODESERVICE_H_ */
