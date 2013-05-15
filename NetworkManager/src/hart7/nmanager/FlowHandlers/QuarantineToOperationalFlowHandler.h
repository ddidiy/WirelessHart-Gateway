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
 * QuarantineToOperationalFlowHandler.h
 *
 *  Created on: Sep 2, 2009
 *      Author: andrei.petrut
 */

#ifndef QUARANTINETOOPERATIONALFLOWHANDLER_H_
#define QUARANTINETOOPERATIONALFLOWHANDLER_H_

#include <nlib/log.h>

#include "../nmodel/HartDevice.h"
#include "../CommonData.h"
#include "../IRequestSend.h"
#include "../operations/WHOperationQueue.h"
#include "Model/Operations/EngineOperations.h"
#include "Model/Services/Service.h"

namespace hart7 {
namespace nmanager {

/**
 * This flow is responsible with moving a device from the Quarantined state to the Operational state.
 * Flow can be triggered automatically or by request.
 */
class QuarantineToOperationalFlowHandler: public IResponseProcessor
{
    LOG_DEF("h7.n.Q2OHandler");

    public:

        typedef boost::function2<void, HartDevice&, bool> FinishedHandler;

        /**
         * Constructor.
         */
        QuarantineToOperationalFlowHandler(const HartDevice& device, CommonData& commonData,
                                           operations::WHOperationQueue& operationsQueue);
        virtual ~QuarantineToOperationalFlowHandler()
        {
        }

        /**
         * Starts the flow by initializing the state machine.
         */
        void Start();

        /**
         * Sends the device and the GW session keys with each other.
         */
        void SendDeviceSessionWithGW();

        /**
         * Creates a bidirectional service between the GW and the device.
         */
        void SendDeviceServiceWithGW();

        /**
         * Creates join links on the device for it to advertise with.
         */
        void SendAdvertiseLinks();

        /**
         * Forcefully stop flow.
         */
        void StopFlow();

        FinishedHandler OnFinished;

        bool status;

        enum FlowState
        {
            Initial, SentAdvertiseLinks, SentSession, SentService, ResolvedService
        };

    private:

        void ProcessConfirm(WHartHandle requestHandle, const WHartLocalStatus& localStatus,
                            const WHartCommandList& list);

        void ProcessIndicate(WHartHandle handle, const WHartAddress& src, WHartPriority priority,
                             WHartTransportType transportType, const WHartCommandList& list);

        void ContinueFlow(bool hasError);

        //	NE::Model::Services::ServicePointer RequestService(
        //			NE::Model::Operations::EngineOperations& engineOperations, Address32 source, Address32 destination);

        void ResolveOperations(NE::Model::Operations::EngineOperationsListPointer operations, bool hasError);
        void ResolveOperationsDelete(NE::Model::Operations::EngineOperationsListPointer operations, bool hasError);

        void ResolveOperationConfirmed(NE::Model::Operations::EngineOperationsListPointer allOperations, NE::Model::Operations::EngineOperationsListPointer operations);

        void SessionWithGWSent(bool withError);

        void CheckRemoveStatus();

    private:

        HartDevice device;

        CommonData& commonData;

        operations::WHOperationQueue& operationsQueue;

        FlowState flowState;

        int expectedConfirmCount;

        bool error;

        uint32_t advLinksHandle;

        uint32_t serviceHandle;

        uint32_t sessionHandle;

        NE::Model::Operations::EngineOperationsListPointer serviceOperations;

        NE::Model::Operations::EngineOperationsListPointer deleteDeviceOperations;

        NE::Model::Operations::EngineOperationsListPointer advertiseOperations;
        uint32_t deleteDeviceOperationEvent;

        int confirmedCount;

    public:

        HartDevice& getHartDevice() {
            return device;
        }


};

}
}

#endif /* QUARANTINETOOPERATIONALFLOWHANDLER_H_ */
