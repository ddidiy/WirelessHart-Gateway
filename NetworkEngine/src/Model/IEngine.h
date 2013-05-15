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
 * IEngine.h
 *
 *  Created on: Mar 3, 2009
 *      Author: mulderul
 */

#ifndef IENGINE_H_
#define IENGINE_H_

#include <boost/shared_ptr.hpp>

#include "Common/NEAddress.h"
#include "Common/NEException.h"
#include "Model/IEngineExceptions.h"
#include "Model/Capabilities.h"
#include "Model/MetaDataAttributes.h"
#include "Model/Operations/EngineOperations.h"
#include "Model/Services/SubnetServices.h"
#include "Model/Topology/TopologyTypes.h"
#include "Model/Topology/SubnetTopology.h"

namespace NE {

namespace Model {

class IEngine {

    public:

        /**
         *
         */
        virtual Uint16 getSubnetId() = 0;

        /**
         * Should return the start time of this engine.
         */
        virtual Uint32 getStartTime() = 0;

        /**
         *
         */
        virtual NE::Common::SettingsLogic& getSettingsLogic() = 0;

        /**
         *
         */
        virtual void setSettingsLogic(const NE::Common::SettingsLogic& settingsLogic) = 0;

        /**
         *
         */
        virtual Address32 createAddress32(Address64 address64) = 0;

        /**
         * Returns true if there is a device with the specified address32.
         */
        virtual bool existsDevice(Address32 address) = 0;

        /**
         * Returns true if there is a device with the specified address64.
         */
        virtual bool existsDevice(const Address64& address64) = 0;

        /**
         * Return the device with the given address.
         */
        virtual NE::Model::Device& getDevice(Address32 address) = 0;

        /*
         *
         */
        virtual Address32 getAddress32(const Address64& address64) = 0;

        /**
         *
         */
        virtual Address64 getAddress64(Address32 address32) = 0;

        /**
         *
         */
        virtual bool joinDevice(NE::Model::Operations::EngineOperations& operationsList,
                    NE::Model::Capabilities& capabilities, Address32 address32, Address32 parentAddress32,
                    Uint16 parentJoinsInProgressNo) = 0;

        /**
         *
         */
        void setRemoveStatus(Address32 address);

        /**
         * Create a service for a service requested
         */
        virtual bool createService(NE::Model::Services::Service& serviceRequest, bool sendToRequester) = 0;

        /**
         * Used for Service termination, deactivation and reactivation request.
         */
        virtual bool terminateService(Address32 address, ServiceId serviceId) = 0;

        /**
         * Adds the visible node with the address visibleAddress to the existingAddress.
         */
        virtual void addVisible(Address32 existingAddress, Address32 visibleAddress, Uint8 rsl) = 0;

        /**
         *
         */
        virtual void addDiagnostics(Address32 existingAddress, Address32 neighborAddress, Uint8 rsl, Uint16 sent,
                    Uint16 received, Uint16 failed) = 0;

        /**
         * Resolve operations:
         * 1. all operation confirmed
         * 2. an error occurred -> the device(s) with problems are deleted, for unsent operations
         * undo the change and for success operations mark the resources as not used
         */
        virtual bool
                    resolveOperations(NE::Model::Operations::EngineOperations& operationsList,
                                NE::Model::Operations::EngineOperations& confirmedOperationsList,
                                bool removeOnTimeout) = 0;

        /**
         * Reevaluate one route, that is set to be evaluated.
         */
        virtual bool evaluateNextPath(NE::Model::Operations::EngineOperations& engineOperations) = 0;

        /**
         * Return true if the device is prepared to act as a router
         */
        virtual bool canSendAdvertise(Address32 address) = 0;

        /**
         * Check if there are devices marked for delete
         */
        virtual void checkRemovedDevices(NE::Model::Operations::EngineOperations& engineOperations,
                    AddressSet& addressSet) = 0;

        /**
         *
         */
        virtual void toIndentString(std::ostringstream& stream) = 0;

        /**
         * A new alarm has been received.
         */
        virtual void newAlarm788(Uint16 source, Uint16 destination) = 0;

        /**
         * Allocate a link to check the visibility between the nodes.
         */
        virtual bool allocateCheckLink(NE::Model::Operations::EngineOperations& engineOperations, Address32 address,
                    Address32 peerAddress) = 0;

        /**
         * Deallocate a link used to check the visibility between the nodes.
         */
        virtual bool deallocateCheckLink(NE::Model::Operations::EngineOperations& engineOperations, Address32 address,
                    Address32 peerAddress) = 0;
};

}

}

#endif /* IENGINE_H_ */
