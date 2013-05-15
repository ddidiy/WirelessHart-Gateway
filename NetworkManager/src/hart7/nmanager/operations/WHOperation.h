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
 * WHOperation.h
 *
 *  Created on: Dec 16, 2008
 *      Author: Radu Pop
 */

#ifndef WHOPERATION_H_
#define WHOPERATION_H_

#include <boost/scoped_array.hpp>

#include <nlib/log.h>
#include <WHartStack/WHartStack.h>
#include <Model/Operations/EngineOperations.h>

#include "CommonOperationReferenceCounter.h"

using namespace NE::Model::Operations;

namespace hart7 {

namespace nmanager {

namespace operations {

using namespace hart7::stack;

/**
 * Any change in the network engine model will be reflected in one or more wireless hart
 * operations that will be sent in the field. A join device operation, a remove device
 * are events that generate lists of wireless hart operations.
 * @author Radu Pop
 */
class WHOperation
{

    private:

        /**
         * The address of the destination.
         */
        WHartAddress destinationAddress;

        /**
         * The request handle used to send the operation into the field.
         * When an operation is created the handle value will be 0. When a confirmed is received
         * this value is used to match the operation for which the response is received.
         */
        WHartHandle handle;

        /**
         * The engine operation corresponding to the current WH operation.
         */
        IEngineOperationPointer engineOperationPointer;

        /**
         * Represents the event that generated this operation. An event generates one or more operations.
         */
        uint32_t operationEvent;

        /**
         * Represents the type of the event that generated this operation
         */

        NetworkEngineEventType::NetworkEngineEventTypeEnum operationEventType;

        /**
         * The wireless hart command that will be sent into the field.
         */
        WHartCommand wHartCommand;

        OperationState::OperationStateEnum operationState;

        uint8_t responseCode;

    public:

        // allocate on heap
        boost::scoped_array<uint8_t> commandData;

    public:

        WHOperation();

        WHOperation(Uint16 subnetId_, const Uint16 address16_, Uint16 engineOperationId_);

        WHOperation(WHartAddress destinationAddress_, uint16_t commandID_, void* command_);

        virtual ~WHOperation();

        WHartAddress& getDestinationAddress()
        {
            return destinationAddress;
        }

        void setDestinationAddress(WHartAddress destinationAddress)
        {
            this->destinationAddress = destinationAddress;
        }

        IEngineOperationPointer getEngineOperation()
        {
            return engineOperationPointer;
        }

        void setEngineOperation(IEngineOperationPointer engineOperationPointer_)
        {
            engineOperationPointer = engineOperationPointer_;
        }

        OperationState::OperationStateEnum getOperationStatus() const
        {
            return this->engineOperationPointer->getState();
        }

        void setOperationStatus(OperationState::OperationStateEnum operationStatus)
        {
            this->engineOperationPointer->setState(operationStatus);
        }

        WHartHandle getHandle() const
        {
            return handle;
        }

        void setHandle(WHartHandle handle)
        {
            this->handle = handle;
        }

        uint32_t getOperationEvent() const
        {
            return operationEvent;
        }

        void setOperationEvent(uint32_t operationEvent_)
        {
            this->operationEvent = operationEvent_;
        }

        NetworkEngineEventType::NetworkEngineEventTypeEnum getOperationEventType() const
        {
            return operationEventType;
        }

        void setOperationEventType(NetworkEngineEventType::NetworkEngineEventTypeEnum operationEventType_)
        {
            this->operationEventType = operationEventType_;
        }


        WHartCommand& getWHartCommand()
        {
            return wHartCommand;
        }

        void setWHartCommand(WHartCommand wHartCommand)
        {
            this->wHartCommand = wHartCommand;
        }

        void setResponseCode(uint8_t responseCode_)
        {
            responseCode = responseCode_;
            if (engineOperationPointer)
            {
                engineOperationPointer->setResponseCode(responseCode);
            }
        }

        uint8_t getResponseCode()
        {
            return responseCode;
        }

    public:

        std::string toString();

};

typedef boost::shared_ptr<WHOperation> WHOperationPointer;

}

}

}

#endif /* WHOPERATION_H_ */
