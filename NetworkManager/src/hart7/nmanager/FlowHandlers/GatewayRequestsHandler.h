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
 * GatewayRequestsHandler.h
 *
 *  Created on: Oct 20, 2009
 *      Author: Radu Pop
 */

#ifndef GATEWAYREQUESTSHANDLER_H_
#define GATEWAYREQUESTSHANDLER_H_

#include <WHartStack/WHartStack.h>
#include "../AllNetworkManagerCommands.h"
#include "../CommonData.h"
#include <nlib/log.h>
#include "GatewayWrappedRequestsHandler.h"

using namespace hart7::stack;

namespace hart7 {

namespace nmanager {

/**
 * Parses the requests from the gateway.
 */
class GatewayRequestsHandler
{
        LOG_DEF("h7.s.GatewayRequestsHandler")
        ;

    public:

        GatewayRequestsHandler(hart7::nmanager::CommonData& commonData_);

        virtual ~GatewayRequestsHandler();

        GatewayWrappedRequestsHandler& getGatewayWrappedRequestsHandler()
        {
            return gatewayWrappedRequestsHandler;
        }

    private:

        hart7::nmanager::CommonData& commonData;

        GatewayWrappedRequestsHandler gatewayWrappedRequestsHandler;

};

}

}

#endif /* GATEWAYREQUESTSHANDLER_H_ */
