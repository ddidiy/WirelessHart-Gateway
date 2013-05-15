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

#include "Service.h"
#include "Model/NetworkEngine.h"

using namespace NE::Common;
using namespace NE::Model::Tdma;
using namespace NE::Model::Services;

Service::Service () {
   address = 0;
   requestServiceId = 0;
   serviceId = 0;
   allocationHandler = 0;
   source = false;
   sink = false;
   intermittent = false;
   applicationDomain = ApplicationDomain::MAINTENANCE;
   peerAddress = 0;
   period = 0;
   routeId = 0;
   oldPeriod = 0;
   requestPending = false;
   management = false;
   status = Status::NOT_PRESENT;
}

Service::Service(Address32 address_, ServiceId requestServiceId_, ServiceId serviceId_, bool source_, bool sink_,
            bool intermittent_, ApplicationDomain::ApplicationDomainEnum applicationDomain_, Address32 peerAddress_,
            Uint32 period_, Uint32 routeId_, bool management_) :

    address(address_), requestServiceId(requestServiceId_), serviceId(serviceId_), source(source_), sink(sink_), intermittent(
                intermittent_), applicationDomain(applicationDomain_), peerAddress(peerAddress_), period(period_), routeId(
                routeId_), management(management_) {

    status = Status::NEW;
    oldPeriod = 0;
    allocationHandler = serviceId;
    requestPending = false;

    proxyService = false;
}

ServiceId Service::getRequestServiceId() {
    return requestServiceId;
}

bool Service::isProxyService() {
    return proxyService;
}

void Service::setProxyService(bool _proxyService) {
    proxyService = _proxyService;;
}

ServiceId Service::getServiceId() {
    return serviceId;
}

void Service::setServiceId(ServiceId serviceId_) {
    serviceId = serviceId_;
}

Address32 Service::getAddress() {
    return address;
}

Uint32 Service::getRouteId() {
    return routeId;
}

bool Service::isSource() {
    return source;
}

bool Service::isSink() {
    return sink;
}

bool Service::isIntermittent() {
    return intermittent;
}

ApplicationDomain::ApplicationDomainEnum Service::getApplicationDomain() {
    return applicationDomain;
}

PublishPeriod::PublishPeriodEnum Service::getPublishPeriod() {
    Uint16 value = period / (32 * 250);

    if (value <= 1) {
        return PublishPeriod::P_250_MS;
    } else if (value <= 2) {
        return PublishPeriod::P_500_MS;
    } else if (value <= 4) {
        return PublishPeriod::P_1_S;
    } else if (value <= 8) {
        return PublishPeriod::P_2_S;
    } else if (value <= 16) {
        return PublishPeriod::P_4_S;
    } else if (value <= 32) {
        return PublishPeriod::P_8_S;
    } else if (value <= 64) {
        return PublishPeriod::P_16_S;
    } else if (value <= 128) {
        return PublishPeriod::P_32_S;
    } else {
        return PublishPeriod::P_64_S;
    }
}

PublishPeriod::PublishPeriodEnum Service::getOldPublishPeriod() {
    Uint16 value = oldPeriod / (32 * 250);

    if (value <= 1) {
        return PublishPeriod::P_250_MS;
    } else if (value <= 2) {
        return PublishPeriod::P_500_MS;
    } else if (value <= 4) {
        return PublishPeriod::P_1_S;
    } else if (value <= 8) {
        return PublishPeriod::P_2_S;
    } else if (value <= 16) {
        return PublishPeriod::P_4_S;
    } else if (value <= 32) {
        return PublishPeriod::P_8_S;
    } else if (value <= 64) {
        return PublishPeriod::P_16_S;
    } else if (value <= 128) {
        return PublishPeriod::P_32_S;
    } else {
        return PublishPeriod::P_64_S;
    }
}

Uint32 Service::getAllocationHandler() {
    return allocationHandler;
}

//WARN: the position of Count is used hard-coded in TimeslotAllocation.cpp
Uint32 Service::setNextAllocationHandler() {
    Uint8 counter = (allocationHandler << 16) >> 24;
    counter++;
    allocationHandler = serviceId + (((Uint16) counter) << 8);

    return allocationHandler;
}

void Service::setRouteId(Uint32 routeId_) {
    routeId = routeId_;
}

Uint32 Service::getPeriod() {
    return period;
}

void Service::setPeriod(Uint32 period_) {
    oldPeriod = period;
    period = period_;
}

Uint32 Service::getOldPeriod() {
    return oldPeriod;
}

Address32 Service::getPeerAddress() {
    return peerAddress;
}

bool Service::isRequestPending() {
    return requestPending;
}

void Service::setRequestPending(bool requestPending_) {
    requestPending = requestPending_;
}

bool Service::isManagement() {
    return management && !isProxyService();
}

void Service::setManagement(bool management_) {
    management = management_;
}

Status::StatusEnum& Service::getStatus() {
    return status;
}

void Service::setStatus(Status::StatusEnum status_) {
    status = status_;
}

void Service::toString(std::ostringstream& stream) {
    stream << " " << ToStr(serviceId, 8);
    stream << "     " << ToStr(address);
    stream << "     " << ToStr(peerAddress);
    stream << " " << ToStr(allocationHandler, 8);
    stream << std::setw(6) << boolToString(source);
    stream << std::setw(6) << boolToString(sink);
    stream << std::setw(6) << boolToString(intermittent);
    stream << std::setw(10) << (int) applicationDomain;
    stream << " " << ToStr(routeId, 8);
    stream << std::setw(7) << (int) (period);
    stream << std::setw(12) << (int) oldPeriod;
    stream << std::setw(6) << boolToString(requestPending);
    stream << std::setw(6) << boolToString(management);
    stream << std::setw(11) << Status::getStatusDescription(status);
}

std::ostream& NE::Model::Services::operator<<(std::ostream& stream, const Service::IndentString& indentString) {
    stream << "{ serviceId=" << ToStr(indentString.service.serviceId);
    stream << ", requestServiceId=" << ToStr(indentString.service.requestServiceId);
    stream << ", address=" << ToStr(indentString.service.address);
    stream << ", peerAddress=" << ToStr(indentString.service.peerAddress);
    stream << ", handler=" << ToStr(indentString.service.allocationHandler);
    stream << ", source=" << boolToString(indentString.service.source);
    stream << ", sink=" << boolToString(indentString.service.sink);
    stream << ", intermittent=" << boolToString(indentString.service.intermittent);
    stream << ", appD=" << (int) indentString.service.applicationDomain;
    stream << ", routeId=" << ToStr(indentString.service.routeId);
    stream << ", period=" << (int) indentString.service.period;
    stream << ", oldPeriod=" << (int) indentString.service.oldPeriod;
    stream << ", requestPending=" << boolToString(indentString.service.requestPending);
    stream << ", management=" << boolToString(indentString.service.management);
    stream << ", status=" << Status::getStatusDescription(indentString.service.status) << "}";

    return stream;
}

std::ostream& NE::Model::Services::operator<<(std::ostream& stream, const Service& service) {
    stream << "{ serviceId=" << ToStr(service.serviceId);
    stream << ", requestServiceId=" << ToStr(service.requestServiceId);
    stream << ", address=" << ToStr(service.address);
    stream << ", peerAddress=" << ToStr(service.peerAddress);
    stream << ", handler=" << ToStr(service.allocationHandler);
    stream << ", source=" << boolToString(service.source);
    stream << " sink=" << boolToString(service.sink);
    stream << ", intermittent=" << boolToString(service.intermittent);
    stream << ", appD=" << (int) service.applicationDomain;
    stream << ", routeId=" << ToStr(service.routeId);
    stream << ", period=" << (int) service.period;
    stream << ", oldPeriod=" << (int) service.oldPeriod;
    stream << ", requestPending=" << boolToString(service.requestPending);
    stream << ", management=" << boolToString(service.management);
    stream << ", status=" << Status::getStatusDescription(service.status) << "}";

    return stream;
}
