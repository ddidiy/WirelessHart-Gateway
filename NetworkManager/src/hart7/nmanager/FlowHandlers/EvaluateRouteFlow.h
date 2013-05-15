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
 * EvaluateRouteFlow.h
 *
 *  Created on: Sep 1, 2009
 *      Author: Andrei Petrut
 */

#ifndef EVALUATEROUTEFLOW_H_
#define EVALUATEROUTEFLOW_H_

#include "../CommonData.h"
#include "../operations/WHOperationQueue.h"
#include "../alarms/CheckDevicePresenceFlow.h"
#include <nlib/log.h>
#include <vector>

namespace hart7 {
namespace nmanager {

/**
 * Responsible with calling EvaluateRoute on Network Engine, send operations, and confirm responses.
 */
class EvaluateRouteFlow
{

        LOG_DEF("h7.n.EvaluateRoute")
        ;

    public:

        typedef boost::function0<void> RouteEvaluatedHandler;

        /**
         * Constructor.
         */
        EvaluateRouteFlow(CommonData& commonData, operations::WHOperationQueue& operationsQueue);

        /**
         * Evaluates all routes, up to a maximum of routes that can be evaluated at once.
         * Sends operations to devices.
         */
        void EvaluateRoutes();

    private:

        void SendOperations(NE::Model::Operations::EngineOperationsListPointer engineOperations);

        void ResolveOperations(NE::Model::Operations::EngineOperationsListPointer allOps, NE::Model::Operations::EngineOperationsListPointer operations, bool hasError);
        void ResolveOperationsDelete(NE::Model::Operations::EngineOperationsListPointer allOps, NE::Model::Operations::EngineOperationsListPointer operations, bool hasError);

        void CheckRemoveStatus();

        void ProcessTimeouts(NE::Model::Operations::EngineOperationsListPointer allOperations, NE::Model::Operations::EngineOperationsListPointer operations, bool hasError);

        void DevicePresenceFlowFinished(uint16_t address, bool hasError);

        void CheckDevicePresences();

        void ResolveOperationsDeleteEvent(NE::Model::Operations::EngineOperationsListPointer operations, bool hasError);

    public:

        /**
         * Handler that gets called whenever a route is evaluated and the operations are confirmed.
         */
        RouteEvaluatedHandler RouteEvaluated;

    private:

        CommonData& commonData;

        operations::WHOperationQueue& operationsQueue;

        NE::Model::Operations::EngineOperationsListPointer engineOperations;

        bool isEvaluating;

        typedef std::vector<CheckDevicePresenceFlowPointer> CheckDevicePresencesList;

        CheckDevicePresencesList devicePresenceFlows;

        std::set<uint32_t> timeoutAddresses;

        int confirmedCount;

        NE::Model::Operations::EngineOperationsListPointer
                    deleteDeviceOperations;
        uint32_t deleteDeviceOperationEvent;
};

}
}

#endif /* EVALUATEROUTEFLOW_H_ */
