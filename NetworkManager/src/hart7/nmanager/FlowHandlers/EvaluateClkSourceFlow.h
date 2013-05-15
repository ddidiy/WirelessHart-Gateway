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
 * EvaluateClkSourceFlow.h
 *
 *  Created on: Sep 15, 2010
 *      Author: Mihai.Stef
 */

#ifndef EVALUATECLKSOURCEFLOW_H_
#define EVALUATECLKSOURCEFLOW_H_

#include "../CommonData.h"
#include "../operations/WHOperationQueue.h"
#include "../alarms/CheckDevicePresenceFlow.h"
#include <nlib/log.h>

namespace hart7 {
namespace nmanager {

/**
 * Flow that attempts to find a secondary clock source for the devices.
 */
class EvaluateClkSourceFlow
{
        LOG_DEF("h7.n.EvaluateClkSourceFlow");

    public:
        /**
         * Constructor.
         */
        EvaluateClkSourceFlow(CommonData& commonData_, operations::WHOperationQueue& operationsQueue_);
        ~EvaluateClkSourceFlow();

        /**
         * Evaluaets clock sources. In case the device does not respond, calls a check device presence flow.
         */
        void evaluateClkSrc();

        /**
         * Finish of the device presence flow check.
         */
        void DevicePresenceFlowFinished(uint16_t address, bool hasError);

        /**
         * Called whenever an operation set is confirmed.
         */
        void ProcessConfirm(NE::Model::Operations::EngineOperationsListPointer operations, bool hasError);

    private:

        void SendOperations(NE::Model::Operations::EngineOperationsListPointer engineOperations);
        void ResolveOperations(NE::Model::Operations::EngineOperationsListPointer operations, bool hasError);
        void CheckDevicePresences();

    private:

        CommonData& commonData;
        operations::WHOperationQueue& operationsQueue;
        NE::Model::Operations::EngineOperationsListPointer engineOps;
        typedef std::vector<CheckDevicePresenceFlowPointer> CheckDevicePresencesList;
        CheckDevicePresencesList devicePresenceFlow;
        std::set<uint32_t> timeoutAddresses;
};

}
}
#endif /* EVALUATECLKSOURCEFLOW_H_ */
