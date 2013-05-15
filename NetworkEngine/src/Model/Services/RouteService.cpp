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
 * RouteServices.cpp
 *
 *  Created on: Sep 21, 2009
 *      Author: ioanpocol
 */

#include <iomanip>

#include "RouteService.h"


namespace NE {
namespace Model {
namespace Services {

RouteService::RouteService(Address32 address_, Uint32 routeId_, Uint32 graphId_, Address32 peerAddress_,
            bool sourceRoute_, bool management_) :
    address(address_), routeId(routeId_), graphId(graphId_), peerAddress(peerAddress_), sourceRoute(sourceRoute_),
                management(management_) {

    status = Status::NEW;
    oldGraphId = 0;
    oldSourceRoute = false;
    allocationPending = false;

    proxyRoute = false;
}

RouteService::~RouteService() {
}

Uint32 RouteService::getRouteId() {
    return routeId;
}

bool RouteService::isAllocationPending() {
    return allocationPending;
}

void RouteService::setAllocationPending(bool allocationPending_) {
    allocationPending = allocationPending_;
}

bool RouteService::isManagement() {
    return management && !isProxyRoute();
}

bool RouteService::isProxyRoute() {
    return proxyRoute;
}

void RouteService::setProxyRoute(bool _proxyRoute) {
    proxyRoute = _proxyRoute;
}

void RouteService::setManagement(bool management_) {
    management = management_;
}

Address32 RouteService::getPeerAddress() {
    return peerAddress;
}

Uint16 RouteService::getGraphId() {
    return graphId;
}

AddressList& RouteService::getDestinations() {
    return destinations;
}

Address32 RouteService::getAddress() {
    return address;
}

Status::StatusEnum& RouteService::getStatus() {
    return status;
}

void RouteService::setStatus(Status::StatusEnum status_) {
    status = status_;
}

bool RouteService::isSourceRoute() {
    return sourceRoute;
}

void RouteService::updateGraphId(Uint16 graphId_) {
    oldSourceRoute = sourceRoute;
    sourceRoute = false;
    oldGraphId = graphId;
    graphId = graphId_;
    status = Status::PENDING;
}

void RouteService::eraseDestinations() {
    destinations.clear();
}

void RouteService::addDestination(Address32 address32) {
    destinations.push_back(address32);
}

void RouteService::setDestinations(AddressList& destinations_) {
    destinations.clear();

    for (AddressList::iterator it = destinations_.begin(); it != destinations_.end(); ++it) {
        destinations.push_back(*it);
    }
}

void RouteService::toString(std::ostringstream& stream) {

    stream << " " << ToStr(routeId, 8);
    stream << "     " << ToStr(address);
    stream << "     " << ToStr(peerAddress);
    stream << " " << ToStr(graphId);
    stream << std::setw(6) << boolToString(sourceRoute);
    stream << std::setw(9) << boolToString(oldSourceRoute);
    stream << std::setw(6) << boolToString(allocationPending);
    stream << std::setw(6) << boolToString(management);
    stream << std::setw(11) << Status::getStatusDescription(status);

}

std::ostream& operator<<(std::ostream& stream, const RouteService& routeService) {

    stream << "{ routeId=" << ToStr(routeService.routeId);
    stream << ", graphId=" << ToStr(routeService.graphId);
    stream << ", address=" << ToStr(routeService.address);
    stream << ", peerAddress=" << ToStr(routeService.peerAddress);
    stream << ", sourceRoute=" << (int) routeService.sourceRoute;
    stream << ", oldSourceRoute=" << (int) routeService.oldSourceRoute;
    stream << ", pending=" << (int) routeService.allocationPending;
    stream << ", management=" << (int) routeService.management;
    stream << " status=" << Status::getStatusDescription(routeService.status);

    return stream;

}

}
}
}
