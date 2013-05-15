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
 * NodeServices.cpp
 *
 *  Created on: Sep 21, 2009
 *      Author: ioanpocol
 */

#ifndef SERVICE_H_
#define SERVICE_H_

#include <list>

#include "Common/NEAddress.h"
#include "Common/NETypes.h"
#include "Common/SettingsLogic.h"
#include "Common/logging.h"
#include "Model/Tdma/TdmaTypes.h"
#include "Model/Services/ServiceTypes.h"

using namespace NE::Common;

namespace NE {
namespace Model {
namespace Services {

class Service;

typedef std::list<Service> ServiceList;
typedef std::list<ServiceId> ServiceIdList;

class Service {

    LOG_DEF("I.M.Service");

    public:

        Address32 address;

        ServiceId requestServiceId;

        ServiceId serviceId;

        Uint32 allocationHandler;

        /**
         * ServiceRequestFlagsMask_Source
         */
        bool source;

        /**
         * ServiceRequestFlagsMask_Sink
         */
        bool sink;

        /**
         * ServiceRequestFlagsMask_Intermittent
         */
        bool intermittent;

        ApplicationDomain::ApplicationDomainEnum applicationDomain;

        Address32 peerAddress;

        Uint32 period;

        Uint32 routeId;

        Uint32 oldPeriod;

        /**
         * Service not set yet.
         */
        bool requestPending;

        bool management;

        bool proxyService;

        Status::StatusEnum status;

    public:

        Service();

        Service(Address32 address_, ServiceId requestServiceId_, ServiceId serviceId_, bool source_, bool sink_,
                    bool intermittent_, ApplicationDomain::ApplicationDomainEnum applicationDomain_, Address32 peerAddress_,
                    Uint32 period_, Uint32 routeId_, bool management_);

        /**
         *
         */
        ServiceId getRequestServiceId();

        /**
         *
         */
        ServiceId getServiceId();

        /**
         *
         */
        void setServiceId(ServiceId serviceId);

        /**
         *
         * @return
         */
        bool isProxyService();

        void setProxyService(bool _proxyService);

        /**
         *
         */
        Address32 getAddress();

        /**
         *
         */
        Uint32 getRouteId();

        /**
         *
         */
        bool isSource();

        /**
         *
         */
        bool isSink();

        /**
         *
         */
        bool isIntermittent();

        /**
         *
         */
        ApplicationDomain::ApplicationDomainEnum getApplicationDomain();

        /**
         *
         */
        NE::Model::Tdma::PublishPeriod::PublishPeriodEnum getPublishPeriod();

        /**
         *
         */
        NE::Model::Tdma::PublishPeriod::PublishPeriodEnum getOldPublishPeriod();

        /**
         *
         */
        void setRouteId(Uint32 routeId);

        /**
         *
         */
        Uint32 getPeriod();

        /**
         *
         * @param period
         */
        void setPeriod(Uint32 period);

        /**
         *
         */
        Uint32 getOldPeriod();

        /**
         *
         */
        Address32 getPeerAddress();

        /**
         *
         */
        bool isRequestPending();

        /**
         *
         */
        void setRequestPending(bool requestPending);

        /**
         *
         */
        bool isManagement();

        /**
         *
         */
        void setManagement(bool management);

        /**
         *
         */
        Status::StatusEnum& getStatus();

        /**
         *
         */
        void setStatus(Status::StatusEnum status);

        /**
         *
         */
        Uint32 getAllocationHandler();

        /**
         *
         */
        Uint32 setNextAllocationHandler();

        /**
         * Returns a friendly table string representation of this service.
         */
        void toString(std::ostringstream& stream);

        class IndentString {
            public:
                const Service& service;
                IndentString(const Service& service_) :
                    service(service_) {
                }
        };

        /**
         * Returns a string representation of this service.
         */
        friend std::ostream& operator<<(std::ostream&, const Service&);

        /**
         * Returns a friendly string representation of this service.
         */
        friend std::ostream& operator<<(std::ostream&, const Service::IndentString&);
};

}
}
}

#endif /*SERVICE_H_*/
