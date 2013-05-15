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
 * FlowHandler.h
 *
 *  Created on: Apr 14, 2010
 *      Author: andrei.petrut
 */

#ifndef FLOWHANDLER_H_
#define FLOWHANDLER_H_

#include <vector>
#include "../IResponseProcessor.h"
#include "../IRequestSend.h"
#include "../CommonData.h"
#include "../operations/WHOperationQueue.h"
#include <nlib/log.h>

namespace hart7 {
namespace nmanager {

/**
 * Base class for Flows. Provides basic operations, like Send to devices, handle timeouts,
 * access to NM resources.
 */
class FlowHandler: public IResponseProcessor
{
    public:
        LOG_DEF("FlowHandler")
        ;
        /**
         * Constructor.
         */
        FlowHandler(IRequestSend& requestSend, CommonData& commonData, operations::WHOperationQueue& operationsQueue);

        virtual ~FlowHandler()
        {
        }

        /**
         * Called whenever a list of commands that were sent gets confirmed.
         */
        virtual void ProcessConfirm(stack::WHartHandle requestHandle, const stack::WHartLocalStatus& localStatus,
                                    const stack::WHartCommandList& list);

        /**
         * Called whenever a list of commands is received as request from a device.
         */
        virtual void ProcessIndicate(stack::WHartHandle handle, const stack::WHartAddress& src,
                                     stack::WHartPriority priority, stack::WHartTransportType transportType,
                                     const stack::WHartCommandList& list);

        /**
         * Sends a list of operations to the devices. Can stop sending operations if any operation fails with error or timeout.
         * Supports adding the operations in the queue with different dependency models.
         */
        void SendOperations(const NE::Model::Operations::EngineOperationsListPointer& operations, uint32_t& handle, bool stopOnError,
                            operations::OperationDependencyType::OperationDependency dependency = operations::OperationDependencyType::None);

        /**
         * Called whenever an entire set of operations that was sent with SendOperations gets confirmed.
         */
        virtual void ProcessConfirmedOperations(bool errorOccured) = 0;

        /**
         * Called whenever a single command gets confirmed.
         */
        void ProcessConfirmedCommand(NE::Model::Operations::EngineOperationsListPointer allOperations, NE::Model::Operations::EngineOperationsListPointer& operations, bool callConfirm);

        /**
         * This method calls the NetworkEngine to remove all devices that have failed to respond or responded with an error.
         * It also handles all the remove flow.
         */
        void CheckRemoveStatus(bool callConfirm);

        /**
         * Called whenever operations are confirmed, to notify the Network Engine that the operation succeeded or failed.
         */
        void ResolveOperations(NE::Model::Operations::EngineOperationsListPointer allOperations, NE::Model::Operations::EngineOperationsListPointer operations, bool callConfirm);
        /**
         * Called whenever delete operations are confirmed, to notify the Network Engine that the operation succeeded or failed.
         */
        void ResolveOperationsDelete(NE::Model::Operations::EngineOperationsListPointer allOperations, NE::Model::Operations::EngineOperationsListPointer operations, bool callConfirm);

    private:

        void CallbackConfirmOperations(bool withError);


    public:

        IRequestSend& requestSend;

        CommonData& commonData;

        operations::WHOperationQueue& operationsQueue;

        bool stopRequested;

    private:

        bool callInnerConfirm;

        bool confirmCalled;

        int confirmedCount;

        uint32_t deleteDeviceOperationEvent;


};
}
}

#endif /* FLOWHANDLER_H_ */
