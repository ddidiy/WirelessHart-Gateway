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
 * CheckDevicePresenceFlow.h
 *
 *  Created on: Mar 24, 2010
 *      Author: radu pop
 */

#ifndef CHECKDEVICEPRESENCEFLOW_H_
#define CHECKDEVICEPRESENCEFLOW_H_

#include <boost/function.hpp>
#include <time.h>

#include "../IResponseProcessor.h"
#include "../IRequestSend.h"
#include "../CommonData.h"
#include "../operations/WHOperationQueue.h"
#include <Common/logging.h>

namespace hart7 {

namespace nmanager {

/**
 * Sends a command to a device to check if the device is responding. If not, removes the device from the system.
 * @author Radu Pop
 */
class CheckDevicePresenceFlow
{
        LOG_DEF("h7.n.CheckDevicePresenceFlow")
        ;
    public:

        CheckDevicePresenceFlow(CommonData& commonData_, operations::WHOperationQueue& operationsQueue_,
                                bool reEvaluateEdge);

        virtual ~CheckDevicePresenceFlow();

        /**
         * Stops the current flow.
         */
        void StopFlow();

        /**
         * Initiate the flow to send a device.
         */
        void checkDevice(uint16_t deviceAddress, uint16_t reportingPeer, uint8_t joinPriority);

        /**
         * Called to initiate the delete of the device with the specified nickname from the network.
         */
        void DeleteDevice(uint16_t deviceAdress);

    private:

        void RemoveExistingDevice();
        /**
         * Callback called when the operations sent into the field confirm.
         */
        void ProcessConfirmedOperations(bool errorOccured);


        /**
         * Called when a device is removed.
         */
        void CheckRemoveStatus(bool callConfirm);

        void ResolveOperations(NE::Model::Operations::EngineOperationsListPointer operations, bool callConfirm);
        void ResolveOperationsDelete(NE::Model::Operations::EngineOperationsListPointer operations, bool callConfirm);

    public:
        /**
         * Callback that is called when the flow has finished.
         */
        typedef boost::function2<void, uint16_t, bool> CheckDevicePresenceFlowFinishedHandler;
        CheckDevicePresenceFlowFinishedHandler checkDevicePresenceFlowFinishedHandler;

        uint16_t deviceAddress;

    private:
        bool reEvaluateFailingEdge;

        uint16_t reportingPeer;

        Address64 longAddressDevice;

        CommonData& commonData;

        operations::WHOperationQueue& operationsQueue;

        NE::Model::Operations::EngineOperationsListPointer linkOperations;

        std::vector<operations::WHOperationPointer> linkWhOperations;

        uint32_t linkOperationsEvent;

        // the state of the flow
        //VisibleEdgeFlowState::VisibleEdgeFlowStateEnum currentFlowState;

        bool haveError;

        bool stopRequested;

        NE::Model::Operations::EngineOperationsListPointer deleteDeviceOperations;

        uint32_t deleteDeviceOperationEvent;

        int confirmedCount;


        std::vector<operations::WHOperationPointer> deleteDeviceWhOperations;

};

typedef boost::shared_ptr<CheckDevicePresenceFlow> CheckDevicePresenceFlowPointer;

}

}

#endif /* CHECKDEVICEPRESENCEFLOW_H_ */
