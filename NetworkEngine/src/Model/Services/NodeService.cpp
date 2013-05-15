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
 * NodeService.cpp
 *
 *  Created on: Sep 21, 2009
 *      Author: ioanpocol
 */

#include "NodeService.h"
#include "Model/NetworkEngine.h"
#include "Model/Services/SubnetServices.h"

using namespace NE::Model::Services;
using namespace NE::Model::Operations;
using namespace NE::Model::Topology;

NodeService::NodeService() {
}

NodeService::NodeService(Address32 address_, Address32 parentAddress_,
            NE::Model::Topology::NodeType::NodeTypeEnum nodeType_) :
    address(address_), parentAddress(parentAddress_), nodeType(nodeType_) {

    if (nodeType_ == NodeType::MANAGER) {
        startServiceId = 0;
    } else {
        startServiceId = MANAGEMENT_SERVICE;
    }
    lastServiceId = startServiceId;
    lastRouteId = MANAGEMENT_ROUTE;
    status = Status::NEW;

    reuseServiceId = false;
    reuseRouteId = false;
}

NodeService::~NodeService() {
}

Uint32 NodeService::getNextRouteId() {
    if (reuseRouteId == false) {
        lastRouteId++;
        if (lastRouteId > 0xFF) {
            reuseRouteId = true;
        }
    }

    // ensure that the route is less then 255 (0xFF)
    if (reuseRouteId == true) {
        // tries to find a free route id in MANAGEMENT_ROUTE-255 interval

        std::set<uint16_t> posRoutes;
        for (int i = MANAGEMENT_ROUTE + 1; i <= 255; ++i) {
            posRoutes.insert(i);
        }

        for (RouteList::iterator it = routes.begin(); it != routes.end(); ++it) {
            posRoutes.erase((Uint16) it->getRouteId());
        }

        if (posRoutes.size() > 0) {
            lastRouteId = *posRoutes.begin();
            LOG_TRACE("recovered routeID: " << (int) lastRouteId);
        } else {
            LOG_ERROR("There is no free route id => overwrites the route id 255!");
            lastRouteId = 0xFF;
        }
    }

    return ((address << 16) + lastRouteId);
}

Uint32 NodeService::getNextServiceId() {
    if (reuseServiceId == false) {
        ++lastServiceId;
        if (lastServiceId > 0xFF) {
            reuseServiceId = true;
        }
    }

    // ensure that the service is less then 255 (0xFF)
    if (reuseServiceId == true) {
        // tries to find a free service id in MANAGEMENT_SERVICE - 0xFF interval

        // skip over first service ID since it is reserved for GW.
        std::set<uint16_t> posServices;
        for (int i = startServiceId + 1; i <= 255; ++i) {
            posServices.insert(i);
        }

        for (ServiceList::iterator it = services.begin(); it != services.end(); ++it) {
            posServices.erase((Uint16) it->getServiceId());
        }

        if (posServices.size() > 0) {
            lastServiceId = *posServices.begin();
        } else {
            LOG_ERROR("There is no free service id => overwrites the service id 255!");
            lastServiceId = 0xFF;
        }
    }

    return ((address << 16) + lastServiceId);
}

Status::StatusEnum& NodeService::getStatus() {
    return status;
}

void NodeService::setStatus(Status::StatusEnum status_) {
    status = status_;
}

RouteService& NodeService::getRouteService(Uint32 routeId) {
    for (RouteList::iterator it = routes.begin(); it != routes.end(); ++it) {
        if (it->getRouteId() == routeId) {
            return *it;
        }
    }

    std::ostringstream stream;
    stream << "The NodeService " << ToStr(address) << " has no route: " << ToStr(routeId);
    LOG_ERROR(stream.str());
    throw NEException(stream.str());
}

Service& NodeService::getService(Uint32 serviceId) {
    for (ServiceList::iterator it = services.begin(); it != services.end(); ++it) {
        if (it->getServiceId() == serviceId) {
            return *it;
        }
    }

    std::ostringstream stream;
    stream << "The NodeService " << ToStr(address) << " has no service: " << ToStr(serviceId);
    LOG_ERROR(stream.str());
    throw NEException(stream.str());
}

bool NodeService::terminateService(ServiceId serviceId, Address32& peerAddress, ServiceId& requestServiceId) {
    for (ServiceList::iterator it = services.begin(); it != services.end(); ++it) {
        if (it->getServiceId() == serviceId) {
            peerAddress = it->getPeerAddress();
            requestServiceId = it->getRequestServiceId();

            services.erase(it);

            return true;
        }
    }

    return false;
}

bool NodeService::terminateService(ServiceId requestServiceId) {
    for (ServiceList::iterator it = services.begin(); it != services.end(); ++it) {
        if (it->getRequestServiceId() == requestServiceId) {
            services.erase(it);

            return true;
        }
    }

    return false;
}

bool NodeService::finishEvaluation(Uint32 graphId) {
    bool result = false;
    for (ServiceList::iterator it = services.begin(); it != services.end(); ++it) {

        try {
            RouteService& route = getRouteService(it->getRouteId());

//            LOG_DEBUG("finishEvaluation - service=" << *it);
//            LOG_DEBUG("finishEvaluation - graphId=" << ToStr(graphId) << ", route= " << route);

            if (route.getGraphId() == graphId) {

                it->setRequestPending(false);
                route.setAllocationPending(false);

                LOG_DEBUG("finishEvaluation - set request pending false");

                result = true;
            }

        } catch (std::exception& ex) {
            LOG_DEBUG("finishEvaluation - ex=" << ex.what());
        }

    }

    return result;
}

ServiceId NodeService::addInboundManagementService(EngineOperations& engineOperations, Uint16 graphId, Uint32 period) {
    Uint32 routeId = (address << 16) + MANAGEMENT_ROUTE;
    Uint32 serviceId = (address << 16) + MANAGEMENT_SERVICE;

    RouteService route(address, routeId, graphId, MANAGER_ADDRESS, nodeType == NodeType::NODE, true);

    if (nodeType != NodeType::NODE) {
        route.setStatus(Status::ACTIVE);
        period = 32;
    } else {
        route.setAllocationPending(true);
    }

    route.addDestination(address);
    route.addDestination(parentAddress);
    routes.push_back(route);

    Service service(address, serviceId, serviceId, true, false, true, ApplicationDomain::MAINTENANCE, MANAGER_ADDRESS,
                period, routeId, true);
    service.setStatus(Status::ACTIVE);

    services.push_back(service);

    IEngineOperationPointer operationRoute(new RouteAddedOperation(address, routeId, graphId, MANAGER_ADDRESS));
    operationRoute->setDependency(WaveDependency::FIFTH);
    engineOperations.addOperation(operationRoute);

    if (nodeType != NodeType::NODE) {

        IEngineOperationPointer operationService(new ServiceAddedOperation(address, serviceId, true, false, true,
                    ApplicationDomain::MAINTENANCE, MANAGER_ADDRESS, period, MANAGEMENT_ROUTE));
        operationService->setDependency(WaveDependency::SEVENTH);
        engineOperations.addOperation(operationService);
    }

    return serviceId;
}

ServiceId NodeService::addOutboundManagementService(EngineOperations& engineOperations, Address32 address_,
            NodeType::NodeTypeEnum nodeType_, Address32 parentAddress_, Uint16 graphId, Uint32 period) {

    Uint32 routeId;
    ServiceId serviceId;

    if (nodeType_ == NodeType::GATEWAY) {
        routeId = (address << 16) + MANAGEMENT_ROUTE;
        serviceId = (address << 16) + startServiceId;
    } else {
        routeId = getNextRouteId();
        serviceId = getNextServiceId();
    }
    if (nodeType_ != NodeType::NODE) {
        period = 32;
    }

    RouteService route(MANAGER_ADDRESS, routeId, graphId, address_, nodeType_ == NodeType::NODE, true);
    if (nodeType_ != NodeType::NODE) {
        route.setStatus(Status::ACTIVE);
    } else {
        route.setAllocationPending(true);
    }
    route.addDestination(parentAddress_);
    route.addDestination(address_);
    routes.push_back(route);

    LOG_DEBUG("addOutboundManagementService - on node=" << ToStr(address) << " addRoute " << ToStr(routeId) << "  == "
                << ToStr(route.getRouteId()));
    getRouteService(routeId);

    Service service(MANAGER_ADDRESS, serviceId, serviceId, true, false, true, ApplicationDomain::MAINTENANCE, address_,
                period, routeId, true);

    service.setStatus(Status::ACTIVE);
    services.push_back(service);

    RouteAddedOperation* operation = new RouteAddedOperation(MANAGER_ADDRESS, routeId, graphId, address_);
    IEngineOperationPointer operationRoute(operation);

    operationRoute->setDependency(WaveDependency::FIFTH);
    engineOperations.addOperation(operationRoute);

    IEngineOperationPointer operationService(new ServiceAddedOperation(MANAGER_ADDRESS, serviceId, true, false, true,
                ApplicationDomain::MAINTENANCE, address_, period, routeId));
    operationService->setDependency(WaveDependency::SEVENTH);

    engineOperations.addOperation(operationService);

    return serviceId;
}

void NodeService::renewServices(EngineOperations&engineOperations) {
    std::map<Uint32, IEngineOperationPointer> raOperations;
    std::map<Uint32, IEngineOperationPointer> sraOperations;

    for (RouteList::iterator it = routes.begin(); it != routes.end(); ++it) {
        if (it->isManagement()) {
            continue;
        }

        IEngineOperationPointer operation(new RouteAddedOperation(address, it->getRouteId(), it->getGraphId(),
                    it->getPeerAddress()));

        operation->setDependency(WaveDependency::FIFTH);
        engineOperations.addOperation(operation);

        raOperations.insert(std::make_pair(it->getRouteId(), operation));

        if (it->isSourceRoute()) {
            SourceRouteAddedOperation *op = new SourceRouteAddedOperation(address, it->getRouteId(), it->getGraphId(),
                        it->getPeerAddress());
            IEngineOperationPointer operationSR(op);
            op->destinations = it->getDestinations();
            operationSR->setDependency(WaveDependency::SIXTH);
            engineOperations.addOperation(operationSR);

            sraOperations.insert(std::make_pair(it->getRouteId(), operationSR));
        }
    }

    for (ServiceList::iterator it = services.begin(); it != services.end(); ++it) {
        if (it->isRequestPending() || it->isManagement()) {
            continue;
        }
        IEngineOperationPointer operation(new ServiceAddedOperation(address, it->getServiceId(), it->isSource(),
                    it->isSink(), it->isIntermittent(), it->getApplicationDomain(), it->getPeerAddress(),
                    it->getPeriod(), it->getRouteId()));

        operation->setDependency(WaveDependency::SEVENTH);

        std::map<Uint32, IEngineOperationPointer>::iterator itRoute = raOperations.find(it->getRouteId());
        if (itRoute != raOperations.end()) {
//            operation->setOperationDependency(itRoute->second);
        } else {
            itRoute = sraOperations.find(it->getRouteId());
            if (itRoute != sraOperations.end()) {
//                operation->setOperationDependency(itRoute->second);
            }
        }

        engineOperations.addOperation(operation);
    }
}

bool NodeService::addService(Service& serviceRequest, Uint32 graphId, bool generateServiceOperation) {
    LOG_DEBUG("addService - node=" << ToStr(address) << ", serviceRequest=" << serviceRequest);

    Uint32 routeId = 0;

    for (ServiceList::iterator it = services.begin(); it != services.end(); ++it) {
        if (it->getRequestServiceId() == serviceRequest.getRequestServiceId()) {
            serviceRequest.setStatus(it->getStatus());
            serviceRequest.setRouteId(it->getRouteId());
            serviceRequest.setRequestPending(it->isRequestPending());
            LOG_DEBUG("addService - crt status for service request=" << serviceRequest);

            if (it->getPeriod() != serviceRequest.getPeriod()) {
                LOG_DEBUG("addService - changed period for service=" << *it);

                it->setPeriod(serviceRequest.getPeriod());

                serviceRequest.setRequestPending(true);
                it->setRequestPending(true);
                return true;
            } else {
                return false;
            }
        } else if (it->getPeerAddress() == serviceRequest.getPeerAddress()) {
            routeId = it->getRouteId();
        }
    }

    if (serviceRequest.getServiceId() == 0) {
        serviceRequest.setServiceId(getNextServiceId());
    }

    if (routeId == 0) {
        routeId = getNextRouteId();
        RouteService
                    route(serviceRequest.getAddress(), routeId, graphId, serviceRequest.getPeerAddress(), false, false);

        route.setAllocationPending(true);

        routes.push_back(route);

        LOG_DEBUG("addService - added route=" << route);
    }

    Service service(serviceRequest.getAddress(), serviceRequest.getRequestServiceId(), serviceRequest.getServiceId(),
                serviceRequest.isSource(), serviceRequest.isSink(), serviceRequest.isIntermittent(),
                serviceRequest.getApplicationDomain(), serviceRequest.getPeerAddress(), serviceRequest.getPeriod(),
                routeId, false);

    if (!generateServiceOperation) {
        service.setStatus(Status::ACTIVE);
    }

    serviceRequest.setRequestPending(true);
    service.setRequestPending(true);
    services.push_back(service);

    LOG_DEBUG("addService - added service=" << service);

    return true;
}

Service& NodeService::findManagementService(Address32 destination, bool isProxyDestination) {

    if (isProxyDestination) {
        for (ServiceList::iterator it = services.begin(); it != services.end(); ++it) {
            if (it->isProxyService() && it->getPeerAddress() == destination && it->getStatus() != Status::DELETED
                        && it->getStatus() != Status::NEW) {
                return *it;
            }
        }
    }

    for (ServiceList::iterator it = services.begin(); it != services.end(); ++it) {
        if (it->isManagement() && it->getPeerAddress() == destination && (it->getStatus() != Status::DELETED
                    && it->getStatus() != Status::NEW)) {
            return *it;
        }
    }

    std::ostringstream stream;
    stream << std::endl << "Management Service not found for address: " << ToStr(destination)
                << ", isProxyDestination=" << (int) isProxyDestination;
    LOG_ERROR(stream.str());
    throw NEException(stream.str());
}

bool NodeService::allocateResources(EngineOperations& engineOperations, Uint16 graphId, Address32 destination,
            bool changeHandler, Uint32& oldHandler, Uint32& newHandler) {

    bool result = false;
    LOG_DEBUG("allocateResources - graphId=" << ToStr(graphId) << ", address=" << ToStr(address) << ", destination="
                << ToStr(destination));

    for (ServiceList::iterator it = services.begin(); it != services.end(); ++it) {

        if (address == MANAGER_ADDRESS || address == GATEWAY_ADDRESS) {
            if (destination != it->getPeerAddress()) {
                continue;
            }
        }

        if (it->isProxyService()) {
            continue;
        }

        RouteService& route = getRouteService(it->getRouteId());
        if ((route.isSourceRoute() && (route.getPeerAddress() == destination)) || (route.getGraphId() == graphId)) {
            result = true;

            if (it->isManagement() && changeHandler) {
                oldHandler = it->getAllocationHandler();
                newHandler = it->setNextAllocationHandler();

                LOG_DEBUG("allocateResources - oldHandler=: " << ToStr(oldHandler) << ", newHandler=" << ToStr(
                            newHandler));
            }

            IEngineOperationPointer operationRA;
            IEngineOperationPointer operationSource;
            if (route.getStatus() == Status::NEW) {

                LOG_DEBUG("allocateResources - is allocation pending");

                route.setStatus(Status::PENDING);

                if (route.isSourceRoute() && it->isManagement()) {
                    LOG_DEBUG("allocateResources - set new graphId=" << (int) graphId << " for route=" << ToStr(
                                route.getRouteId()));

                    route.updateGraphId(graphId);
                }

                operationRA.reset(new RouteAddedOperation(route.getAddress(), route.getRouteId(), graphId,
                            route.getPeerAddress()));

                operationRA->setDependency(WaveDependency::FIFTH);
                engineOperations.addOperation(operationRA);

            }

            if (it->getStatus() == Status::NEW) {
                it->setStatus(Status::PENDING);

                IEngineOperationPointer operation(new ServiceAddedOperation(address, it->getServiceId(),
                            it->isSource(), it->isSink(), it->isIntermittent(), it->getApplicationDomain(),
                            it->getPeerAddress(), it->getPeriod(), it->getRouteId()));
                operation->setDependency(WaveDependency::FIFTH);
                // set dependencies for operation to RouteAddedOperation or SourceRouteAddedOperation
                if (operationSource) {
//                    operation->setOperationDependency(operationSource);
                } else if (operationRA) {
//                    operation->setOperationDependency(operationRA);
                }

                engineOperations.addOperation(operation);
            }
        }
    }

    return result;
}

Uint16 NodeService::computeOutboundTraffic(Uint16 graphId, Address32 destination) {

    LOG_DEBUG("computeTraffic - graphId=" << ToStr(graphId) << ", address=" << ToStr(address) << ", destination="
                << ToStr(destination));

    Uint16 traffic = 0;

    for (ServiceList::iterator it = services.begin(); it != services.end(); ++it) {

        if (address == MANAGER_ADDRESS || address == GATEWAY_ADDRESS) {
            if (destination != it->getPeerAddress()) {
                continue;
            }
        }

        if (it->isProxyService()) {
            continue;
        }

        RouteService& route = getRouteService(it->getRouteId());
        if ((route.isSourceRoute() && (route.getPeerAddress() == destination)) || (route.getGraphId() == graphId)) {
            LOG_DEBUG("allocateResources outbound - oldTraffic=" << traffic << ", add publish=" << (int) it->getPublishPeriod());

            traffic += it->getPublishPeriod();

        }
    }

    return traffic;
}

Uint16 NodeService::computeInboundTraffic(Uint16 graphId, Address32 destination, std::multiset<Uint16>& compactPublishPeriods) {

    LOG_DEBUG("computeTraffic - graphId=" << ToStr(graphId) << ", address=" << ToStr(address) << ", destination="
                << ToStr(destination));

    std::multiset<Uint16> nonCompactPublishPeriods;

    for (ServiceList::iterator it = services.begin(); it != services.end(); ++it) {


        if (address == MANAGER_ADDRESS || address == GATEWAY_ADDRESS) {
            if (destination != it->getPeerAddress()) {
                //LOG_DEBUG("allocateResources - SKIP");
                continue;
            }
        }

        if (it->isProxyService()) {
            continue;
        }

        RouteService& route = getRouteService(it->getRouteId());
        if ((route.isSourceRoute() && (route.getPeerAddress() == destination)) || (route.getGraphId() == graphId)) {
            nonCompactPublishPeriods.insert(toPublishPeriod(it->getPublishPeriod ()));
        }
    }

    // in case method was called several times in a row put toghether current and previous publish periods
    for (std::multiset<Uint16>::const_iterator cit = compactPublishPeriods.begin(); cit != compactPublishPeriods.end (); ++cit) {
        nonCompactPublishPeriods.insert(*cit);
    }

    Uint16 traffic = compactMultiset(nonCompactPublishPeriods, compactPublishPeriods);

    int noCompactablePeriods = 0;
    for (std::multiset<Uint16>::const_iterator cit = compactPublishPeriods.begin(); cit != compactPublishPeriods.end (); ++cit) {
        if (*cit < PublishPeriod::P_250_MS)
            ++noCompactablePeriods;
    }

    if (noCompactablePeriods <= 1)
        return traffic;

    // attempt to compact traffic periods even more
    // with the price of overallocating some traffic
    int bestTraffic = 0XFFFF;
    std::multiset<Uint16> bestConfiguration;

    for (std::multiset<Uint16>::const_iterator cit = compactPublishPeriods.begin(); cit != compactPublishPeriods.end (); ++cit) {

           int testPeriodOne = *cit;

           if (testPeriodOne == PublishPeriod::P_250_MS)
               break;

           for (std::multiset<Uint16>::const_iterator cit2 = compactPublishPeriods.begin(); cit2 != compactPublishPeriods.end (); ++cit2) {

               int testPeriodTwo = *cit2;

               // do not test same config twice
               if (testPeriodTwo <= testPeriodOne)
                   continue;

               std::multiset<Uint16> tmpMset;
               std::multiset<Uint16> compactedTmpSet;

               if (compactPublishPeriods.size() > 2) {

                   for (std::multiset<Uint16>::const_iterator cit3 = compactPublishPeriods.begin();
                               cit3 != compactPublishPeriods.end (); ++cit3) {

                       if (*cit3 == testPeriodOne) continue;
                       if (*cit3 == testPeriodTwo) continue;
                       tmpMset.insert(*cit3);
                   }
               }

               // choose the largest of two
               int testPeriod = testPeriodOne;
               if (testPeriod < testPeriodTwo) testPeriod = testPeriodTwo;
               tmpMset.insert(testPeriod);
               tmpMset.insert(testPeriod);

               int newTraffic = compactMultiset(tmpMset, compactedTmpSet);
               if (newTraffic < bestTraffic){
                   bestTraffic = newTraffic;
                   bestConfiguration = std::multiset<Uint16>(compactedTmpSet);
               }
           }

    }

    int maxOverAllocPercent = 125;
    if (NE::Model::NetworkEngine::instance().getSettingsLogic().percentTrafficMaxOverAlloc <= 100) {
        maxOverAllocPercent = 100 + NE::Model::NetworkEngine::instance().getSettingsLogic().percentTrafficMaxOverAlloc;
    }

    if (bestConfiguration.size() > 0 && 100 * bestTraffic <= maxOverAllocPercent * traffic ) {

        compactPublishPeriods = std::multiset<Uint16>(bestConfiguration);
        traffic = bestTraffic;
    }

    return traffic;
}

Uint16 NodeService::compactMultiset(std::multiset<Uint16>& nonCompactPublishPeriods, std::multiset<Uint16>& compactPublishPeriods) {

    // Start compacting publish periods. Publish periods range from P_64_S = 1 to P_250_MS = 256. Two publish periods of value x can be
    // grouped together in one publish period of value 2x except when x = 256. The algorithm below  will start with the lowest number
    // and will try to compact and fill the slots. In case the publish period is 256 it will be added to the publish periods multiset
    // unchanged. The allocated traffic will be the smallest possible. One possible issue is total the number of links required to
    // service all slots. In practice this number should be small on average.

    compactPublishPeriods.clear();
    while (nonCompactPublishPeriods.size() > 0) {

        int smallest = *(nonCompactPublishPeriods.begin());

        if (nonCompactPublishPeriods.size() == 1) {
            compactPublishPeriods.insert(smallest);
            break;
        }

        if (smallest == PublishPeriod::P_250_MS) {
            compactPublishPeriods.insert(smallest);
            nonCompactPublishPeriods.erase(nonCompactPublishPeriods.begin());
            continue;
        }

        int nextToSmallest = *(++nonCompactPublishPeriods.begin());
        if (nextToSmallest > smallest) {
            compactPublishPeriods.insert(smallest);
            nonCompactPublishPeriods.erase(nonCompactPublishPeriods.begin());
        } else {
            nonCompactPublishPeriods.erase(nonCompactPublishPeriods.begin());
            nonCompactPublishPeriods.erase(nonCompactPublishPeriods.begin());
            if (smallest * 2 < PublishPeriod::P_250_MS)
                nonCompactPublishPeriods.insert(smallest * 2);
            else
                compactPublishPeriods.insert(PublishPeriod::P_250_MS);
        }
    }

    Uint16 traffic = 0;
    for (std::multiset<Uint16>::const_iterator cit = compactPublishPeriods.begin();
                cit != compactPublishPeriods.end (); ++cit) {

           traffic += *cit;
    }

    return traffic;
}


PublishPeriod::PublishPeriodEnum NodeService::toPublishPeriod(Uint16 traffic) {

    if (traffic > (int) PublishPeriod::P_250_MS) {
        return PublishPeriod::P_250_MS;
    } else if (traffic > (int) PublishPeriod::P_500_MS) {
        return PublishPeriod::P_250_MS;
    } else if (traffic > (int) PublishPeriod::P_1_S) {
        return PublishPeriod::P_500_MS;
    } else if (traffic > (int) PublishPeriod::P_2_S) {
        return PublishPeriod::P_1_S;
    } else if (traffic > (int) PublishPeriod::P_4_S) {
        return PublishPeriod::P_2_S;
    } else if (traffic > (int) PublishPeriod::P_8_S) {
        return PublishPeriod::P_4_S;
    } else if (traffic > (int) PublishPeriod::P_16_S) {
        return PublishPeriod::P_8_S;
    } else if (traffic > (int) PublishPeriod::P_32_S) {
        return PublishPeriod::P_16_S;
    } else if (traffic > (int) PublishPeriod::P_64_S) {
        return PublishPeriod::P_32_S;
    } else {
        return PublishPeriod::P_64_S;
    }
}

void NodeService::releaseRemoved(NE::Model::Operations::EngineOperations& engineOperations) {
    SubnetServices& subnetServices = NetworkEngine::instance().getSubnetServices();
    // RemoveSources depends on RemoveSourceRoutes and RemoveSourceRoutes depends on RemoveServices

    std::map<uint32_t, IEngineOperationPointer> rsOperations;
    for (ServiceList::iterator it = services.begin(); it != services.end(); ++it) {
        if (it->getAddress() == MANAGER_ADDRESS && it->getPeerAddress() == GATEWAY_ADDRESS) {
            continue;
        }

        if (subnetServices.isRemovedStatus(it->getPeerAddress())) {
            it->setStatus(Status::DELETED);
            IEngineOperationPointer operation(new ServiceRemovedOperation(address, it->getPeerAddress(),
                        it->getServiceId(), it->getRouteId(), DeleteReason::REQUESTED_BY_PEER));

            operation->setDependency(WaveDependency::SEVENTH);
            engineOperations.addOperation(operation);

            rsOperations.insert(std::make_pair(it->getRouteId(), operation));
        }
    }

    for (RouteList::iterator it = routes.begin(); it != routes.end(); ++it) {
        if (it->getAddress() == MANAGER_ADDRESS && it->getPeerAddress() == GATEWAY_ADDRESS) {
            continue;
        }

        if (subnetServices.isRemovedStatus(it->getPeerAddress())) {
            it->setStatus(Status::DELETED);

            IEngineOperationPointer operationRemoveSR;
            if (it->isSourceRoute()) // generate the remove source route first
            {
                operationRemoveSR.reset(new SourceRouteRemovedOperation(address, it->getRouteId()));

                operationRemoveSR->setDependency(WaveDependency::EIGTH);

                engineOperations.addOperation(operationRemoveSR);
            }

            IEngineOperationPointer operation(new RouteRemovedOperation(address, it->getRouteId(), it->getGraphId()));

            operation->setDependency(WaveDependency::NINETH);

            std::map<uint32_t, IEngineOperationPointer>::iterator itRServ = rsOperations.begin();

            if (operationRemoveSR) {
//                operation->setOperationDependency(operationRemoveSR); // RemoveRoute depends on RemoveSourceRoute
                if (itRServ != rsOperations.end()) {
//                    operationRemoveSR->setOperationDependency(itRServ->second); // RemoveSourceRoute depends on RemoveService
                }
            } else {
                if (itRServ != rsOperations.end()) {
//                    operation->setOperationDependency(itRServ->second); // RemoveRoute depends on RemoveService
                }
            }

            engineOperations.addOperation(operation);

        }
    }
}

bool NodeService::resolveOperation(NE::Model::Operations::ServiceAddedOperation& operation) {

    for (ServiceList::iterator it = services.begin(); it != services.end(); ++it) {
        if (operation.getServiceId() == it->getServiceId()) {
            //TODO: ivp - on service change this should be changed
            if (operation.getErrorCode() == ErrorCode::SUCCESS) {
                if (it->getStatus() == Status::PENDING) {
                    it->setStatus(Status::ACTIVE);
                    return true;
                } else {
                    //TODO: nothing?
                }
            } else if (operation.getErrorCode() == ErrorCode::TIMEOUT) {
                return false;
            } else if (operation.getErrorCode() == ErrorCode::ERROR) {
                //TODO: depending by the error set ACTIVE or delete it
                if (operation.getState() == OperationState::SENT_IGNORED) {
                    services.erase(it);
                    return true;
                }
                return false;
            }
            //TODO: ivp -is this OK?
            return true;
        }
    }

    LOG_WARN("No service found to match ServiceAddedOperation...");
    return true;
}

bool NodeService::resolveOperation(NE::Model::Operations::ServiceRemovedOperation& operation) {
    for (ServiceList::iterator it = services.begin(); it != services.end(); ++it) {
        if (operation.getServiceId() == it->getServiceId()) {

            if (operation.getErrorCode() == ErrorCode::SUCCESS) {
                if (it->getStatus() == Status::DELETED) {
                    services.erase(it);
                    return true;
                } else {
                    //TODO: ivp - nothing?
                }
            } else if (operation.getErrorCode() == ErrorCode::TIMEOUT) {
                return false;
            } else if (operation.getErrorCode() == ErrorCode::ERROR) {
                //TODO: depending by the error delete it or not
                return true; //Do not delete yet
            }
            //TODO: ivp -is this OK?
            return true;
        }
    }

    LOG_WARN("No service found to match ServiceRemovedOperation...");
    return true;
}

bool NodeService::resolveOperation(NE::Model::Operations::RouteAddedOperation& operation) {
    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();

    for (RouteList::iterator it = routes.begin(); it != routes.end(); ++it) {
        if (operation.getRouteId() == it->getRouteId()) {
            //TODO: ivp - on route change this should be changed
            if (operation.getErrorCode() == ErrorCode::SUCCESS) {
                if (it->getStatus() == Status::PENDING) {

                    //in the case of management route set the inbound/outbound management graphId
                    if (it->isManagement()) {
                        Address32 nodeAddress = it->getAddress();
                        bool isInbound = true;

                        if (nodeAddress == MANAGER_ADDRESS) {
                            nodeAddress = it->getPeerAddress();
                            isInbound = false;
                        }

                        if (!subnetTopology.existsNode(nodeAddress))
                            return true;

                        Node& node = subnetTopology.getNode(nodeAddress);

                        if (isInbound) {
                            node.setInboundGraphId(it->getGraphId());
                        } else {
                            node.setOutboundGraphId(it->getGraphId());
                        }
                    }
                    it->setStatus(Status::ACTIVE);

                    return true;
                } else {
                    //TODO: nothing?
                }
            } else if (operation.getErrorCode() == ErrorCode::TIMEOUT) {
                return false;
            } else if (operation.getErrorCode() == ErrorCode::ERROR) {
                return false;
            }

            // TODO: ivp -is this OK?
            return true;
        }
    }

    LOG_WARN("No route found to match RouteAddedOperation...");
    return true;
}

bool NodeService::resolveOperation(NE::Model::Operations::SourceRouteAddedOperation& operation) {
    SubnetTopology& subnetTopology = NetworkEngine::instance().getSubnetTopology();

    for (RouteList::iterator it = routes.begin(); it != routes.end(); ++it) {
        if (operation.getRouteId() == it->getRouteId()) {
            //TODO: ivp - on route change this should be changed
            if (operation.getErrorCode() == ErrorCode::SUCCESS) {
                if (it->getStatus() == Status::PENDING) {

                    //in the case of management route set the inbound/outbound management graphId
                    if (it->isManagement()) {
                        Address32 nodeAddress = it->getAddress();
                        bool isInbound = true;

                        if (nodeAddress == MANAGER_ADDRESS) {
                            nodeAddress = it->getPeerAddress();
                            isInbound = false;
                        }
                        if (!subnetTopology.existsNode(nodeAddress))
                            return true;

                        Node& node = subnetTopology.getNode(nodeAddress);

                        if (isInbound) {
                            node.setInboundGraphId(it->getGraphId());
                        } else {
                            node.setOutboundGraphId(it->getGraphId());
                        }
                    }
                    it->setStatus(Status::ACTIVE);
                    LOG_DEBUG("allocateResources - set allocation pending false for route:" << ToStr(it->getRouteId()));

                    it->setAllocationPending(false);
                    return true;
                } else {
                    //TODO: nothing?
                }
            } else if (operation.getErrorCode() == ErrorCode::TIMEOUT) {
                return false;
            } else if (operation.getErrorCode() == ErrorCode::ERROR) {

                return false;
            }

            //TODO: ivp -is this OK?
            return true;
        }
    }

    LOG_WARN("No route found to match SourceRouteAddedOperation...");
    return true;
}

bool NodeService::resolveOperation(NE::Model::Operations::RouteRemovedOperation& operation) {

    //test if it is default route
    if (operation.getRouteId() == 0) {
        return true;
    }

    for (RouteList::iterator it = routes.begin(); it != routes.end(); ++it) {
        if (operation.getRouteId() == it->getRouteId()) {

            if (operation.getErrorCode() == ErrorCode::SUCCESS) {
                if (it->getStatus() == Status::DELETED) {
                    routes.erase(it);
                    return true;
                } else {
                    //TODO: nothing?
                }
            } else if (operation.getErrorCode() == ErrorCode::TIMEOUT) {
                return false;
            } else if (operation.getErrorCode() == ErrorCode::ERROR) {
                //TODO: ivp - depending by the error delete it or not
                if (operation.getState() == OperationState::SENT_IGNORED)   //TODO check if this can happen
                {
                    return true;
                }

                return false;
            }

            //TODO: ivp -is this OK?
            return true;
        }
    }

    std::ostringstream stream;
    stream << "No route found to match RouteRemovedOperation... ";
    operation.toString(stream);
    LOG_WARN(stream.str());
    return true;
}

bool NodeService::resolveOperation(NE::Model::Operations::SourceRouteRemovedOperation& operation) {

    for (RouteList::iterator it = routes.begin(); it != routes.end(); ++it) {
        if (operation.getRouteId() == it->getRouteId()) {

            if (operation.getErrorCode() == ErrorCode::SUCCESS) {
                if (it->getStatus() == Status::DELETED) {
                    routes.erase(it);
                    return true;
                } else {
                    //TODO: nothing?
                }
            } else if (operation.getErrorCode() == ErrorCode::TIMEOUT) {
                return false;
            } else if (operation.getErrorCode() == ErrorCode::ERROR) {
                //TODO: ivp - depending by the error delete it or not
                if (operation.getState() == OperationState::SENT_IGNORED) //TODO check if this can happen
                {
                    return true;
                }
                return false;
            }

            //TODO: ivp -is this OK?
            return true;
        }
    }

    LOG_WARN("No route found to match SourceRouteRemovedOperation...");
    return true;
}

bool NodeService::canSendAdvertise(Address32 peerAddress) {

    if (nodeType == NodeType::BACKBONE) {
        return true;
    }

    for (RouteList::iterator it = routes.begin(); it != routes.end(); ++it) {
        if (!it->isAllocationPending() && it->isManagement() && (it->getAddress() == peerAddress
                    || it->getPeerAddress() == peerAddress)) {
            return true;
        }
    }

    return false;
}

bool NodeService::existsService(Uint32 serviceID) {
    for (ServiceList::iterator it = services.begin(); it != services.end(); it++) {
        if (it->serviceId == serviceID) {
            return it->getStatus() != Status::DELETED;
        }
    }
    return false;
}

bool NodeService::getManagementService(ServiceId& serviceId, Address32 peerAddress) {
    for (ServiceList::iterator it = services.begin(); it != services.end(); it++) {
        if (it->isManagement() && (it->getPeerAddress() == peerAddress)) {
            serviceId = it->getServiceId();
            return true;
        }
    }

    return false;
}

void NodeService::generateProxyService(EngineOperations& engineOperations, Address32 address,
            AddressList& destinations, Uint16 graphId, Uint32 period) {

    Uint32 routeId = getNextRouteId();
    Uint32 serviceId = getNextServiceId();

    RouteService route(MANAGER_ADDRESS, routeId, graphId, address, true, true);
    route.setDestinations(destinations);
    route.setProxyRoute(true);

    routes.push_back(route);

    Service service(MANAGER_ADDRESS, serviceId, serviceId, true, false, true, ApplicationDomain::MAINTENANCE, address,
                period, routeId, true);
    service.setStatus(Status::ACTIVE);
    service.setProxyService(true);

    services.push_back(service);

    IEngineOperationPointer operationRoute(new RouteAddedOperation(MANAGER_ADDRESS, routeId, graphId, address));
    operationRoute->setDependency(WaveDependency::FIFTH);
    engineOperations.addOperation(operationRoute);

    SourceRouteAddedOperation *op = new SourceRouteAddedOperation(MANAGER_ADDRESS, routeId, graphId, address);
    IEngineOperationPointer operationSR(op);
    op->destinations = route.getDestinations();
    operationSR->setDependency(WaveDependency::SIXTH);
    engineOperations.addOperation(operationSR);

    IEngineOperationPointer operationService(new ServiceAddedOperation(MANAGER_ADDRESS, serviceId, true, false, true,
                ApplicationDomain::MAINTENANCE, address, period, routeId));
    operationService->setDependency(WaveDependency::SEVENTH);
    engineOperations.addOperation(operationService);
}

void NodeService::servicesToString(std::ostringstream& stream) {
    //stream << ToStr(address) << std::endl;
    for (ServiceList::iterator it = services.begin(); it != services.end(); it++) {
        it->toString(stream);

        stream << std::endl;
    }
}

Uint32 NodeService::getCheckHandler() {

    for (ServiceList::iterator it = services.begin(); it != services.end(); ++it) {
        if (it->isManagement()) {
            return it->getAllocationHandler();
        }
    }

    LOG_ERROR("getCheckHandler() No management service for " << ToStr(address));
    return 0;
}

void NodeService::routesToString(std::ostringstream& stream) {
    for (RouteList::iterator it = routes.begin(); it != routes.end(); ++it) {
        it->toString(stream);

        stream << std::endl;
    }

}

