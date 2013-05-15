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
 * RouteServices.h
 *
 *  Created on: Sep 21, 2009
 *      Author: ioanpocol
 */

#ifndef ROUTESERVICE_H_
#define ROUTESERVICE_H_

#include <list>

#include "Common/NEAddress.h"
#include "Common/NETypes.h"
#include "Common/logging.h"

namespace NE {
namespace Model {
namespace Services {

class RouteService;
typedef std::list<RouteService> RouteList;

class RouteService {

    private:

        Uint32 address;

        /** the id of the route, the route can be source or graph */
        Uint32 routeId;

        /** even for source route a graphID will be generated, it is mandatory for WirelessHART */
        Uint16 graphId;

        /** the final destination of the route */
        Address32 peerAddress;

        /** true if the route is source */
        bool sourceRoute;

        /** the list of nodes from source route */
        std::list<Address32> destinations;

        /** the status of the route */
        Status::StatusEnum status;

        /** the id of the route, the route can be source or graph */
        Uint32 oldGraphId;

        /** the old value of sourceRoute */
        bool oldSourceRoute;

        /** set true if the route is on allocation pending */
        bool allocationPending;

        /** is management route */
        bool management;

        /** the route is used for proxy */
        bool proxyRoute;

    public:

        /**
         *
         */
        RouteService() {
        }

        /**
         *
         */
        RouteService(Address32 address, Uint32 routeId, Uint32 graphId, Address32 peerAddress, bool sourceRoute, bool manager);

        /**
         *
         */
        virtual ~RouteService();

        /**
         *
         */
        Uint32 getRouteId();

        /**
         *
         */
        Uint16 getGraphId();

        /**
         *
         */
        void updateGraphId(Uint16 graphId);

        /**
         *
         */
        bool isAllocationPending();

        /**
         *
         */
        void setAllocationPending(bool allocationPending);

        /**
         *
         */
        bool isManagement();

        /**
         *
         * @return
         */
        bool isProxyRoute();

        /*
         *
         */
        void setProxyRoute(bool _proxyRoute);

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
        Address32 getPeerAddress();

        /**
         *
         */
        Address32 getAddress();

        /**
         *
         */
        AddressList& getDestinations();

        /**
         *
         * @param destinations
         */
        void setDestinations(AddressList& destinations);

        /**
         *
         */
        bool isSourceRoute();

        /**
         *
         */
        void eraseDestinations();

        /**
         *
         */
        void addDestination(Address32 address32);

        /**
         * Returns a friendly table string representation of this service.
         */
        void toString(std::ostringstream& stream);

        /**
         * Returns a string representation of this route.
         */
        friend std::ostream& operator<<(std::ostream&, const RouteService&);
};

}
}
}

#endif /* ROUTESERVICE_H_ */
