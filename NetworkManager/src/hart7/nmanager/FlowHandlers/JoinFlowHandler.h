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
 * JoinFlowHandler.h
 *
 *  Created on: May 22, 2009
 *      Author: andrei.petrut
 */

#ifndef JOINFLOWHANDLER_H_
#define JOINFLOWHANDLER_H_

#include <boost/function.hpp>

#include "FlowHandler.h"
#include "../IRequestSend.h"
#include "../nmodel/HartDevice.h"
#include <Common/logging.h>

namespace hart7 {
namespace nmanager {

class DevicesManager;

/**
 * Handles a join flow. Takes a device from the JoinRequest Sent state to the Quarantine state.
 */
class JoinFlowHandler: public FlowHandler
{
        LOG_DEF("h7.n.JoinFlowHandler");

    public:

        enum JoinFlowState
        {
            Initial,
            KeyNickSessionSent,
            ExistingDeviceRemoved,
            OperationsSending,
            OperationsResolved,
            Failed
        };

        /**
         * Handler prototype for Join Finished.
         */
        typedef boost::function2<void, const HartDevice&, bool> JoinFinishedHandler;

        /**
         * Constructor.
         */
        JoinFlowHandler(DevicesManager& devicesManager, IRequestSend& requestSend, CommonData& commonData,
                        operations::WHOperationQueue& operationsQueue);

        virtual ~JoinFlowHandler();

        /**
         * Processes the join request, decides whether the device can be admitted into the network
         * based on the parent and network constraints.
         */
        void ProcessJoinRequest(const stack::WHartAddress& src, const stack::WHartCommandList& list, bool overrideMaxChildren = false);

        /**
         * Forcefully stops the current join flow.
         */
        void StopFlow();

        Address32 getParentAddress32()
        {
            return this->parentAddress32;
        }

        Address32 getAddress32()
        {
        	return this->joinedDeviceAddress32;
        }

    private:

        uint32_t WriteKeyNickSession(NE::Model::Capabilities& capabilities, Address32 address32,
                                     WHartUniqueID uniqueID, Address32 parentAddress32, const NE::Model::SecurityKey& networkKey,
                                     const NE::Model::SecurityKey& key);

        void OnJoinFinished(bool status);

        void ProcessSuccessGWConfirm();

        void ProcessSuccessDeviceConfirm();

        bool RemoveExistingDevice();

        WHartHandle TransmitRequest(const WHartAddress& to, const WHartCommandList& commands,
                                    WHartSessionKey::SessionKeyCode sessionCode);

        void JoinDevice();

        virtual void ProcessConfirmedOperations(bool errorOccured);

    public:
        /**
         * Callback for the Join Finished event.
         */
        JoinFinishedHandler JoinFinished;
        uint32_t TimeOut;

        HartDevice device;

        bool isActive;
    private:

        DevicesManager& devicesManager;

        JoinFlowState currentState;

        NE::Model::Operations::EngineOperationsListPointer keyNickSessionOperations;
        uint32_t keyNickSessionHandle;

        WHartHandle joinResponseHandle;
        WHartHandle gwSuperframeHandle;
        WHartHandle gwGraphHandle;

        stack::WHartAddress joiningDevice;
        stack::WHartAddress proxyToDevice;

        Address64 joinEuidAddress;

        NE::Model::Operations::EngineOperationsListPointer joinOperations;
        uint32_t joinOperationsEvent;

        std::vector<operations::WHOperationPointer> joinWhOperations;

        Address32 joinedDeviceAddress32;

        int expectedConfirmCount;
        NE::Model::Capabilities deviceCapabilities;
        Address32 parentAddress32;

        Address64 longAddressDevice;

        bool haveError;
        bool stopRequested;
};

}
}

#endif /* JOINFLOWHANDLER_H_ */
