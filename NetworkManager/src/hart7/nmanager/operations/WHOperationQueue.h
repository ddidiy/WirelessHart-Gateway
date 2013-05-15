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
 * WHOperationQueue.h
 *
 *  Created on: Dec 16, 2008
 *      Author: Radu Pop
 */

#ifndef WHOPERATIONQUEUE_H_
#define WHOPERATIONQUEUE_H_

#include <nlib/log.h>
#include <time.h>

#include "WHOperation.h"
#include "WHOperationQueueLogger.h"
#include "WHEngineOperationsVisitor.h"
#include "OperationsResponseCodes.h"
#include <Model/NetworkEngine.h>
#include <Model/Operations/EngineOperations.h>
#include <Common/NETypes.h>

#include "../IResponseProcessor.h"
#include "../IRequestSend.h"
#include "../CommonData.h"
#include "../FlowHandlers/DevicesJoinPriorityFlow.h"

using namespace hart7::nmanager;
using namespace NE::Model::Operations;

namespace hart7 {

namespace nmanager {

namespace operations {

/**
 * Predicate used to find a WHartAddress in a address vector by an address.
 */
class AddressPredicate
{
    public:

        AddressPredicate(WHartAddress _address)
        {
            address = _address;
        }

        bool operator()(WHartAddress& _address)
        {
            return _address == address;
        }

    private:

        WHartAddress address;
};

/**
 * Predicate used to find a WHartAddress in a address by an event.
 */
class OperationsEventPredicate
{
    public:

        OperationsEventPredicate(const uint32_t _event)
        {
            event = _event;
        }

        bool operator()(WHOperationPointer operation)
        {
            return operation->getOperationEvent() == event;
        }

    private:
        uint32_t event;
};

namespace OperationDependencyType {
    enum OperationDependency
    {
        None,
        DependOnSameAddress,
        DependOnCheckRemoved,
        DependOnAll
    };
}

/**
 * Every time a device makes a join/rejoin, requests a service, publish  a list  of EngineOperation is generated.
 * This list of EngineOperation is transformed into a list of WHOperation's. A WHOperation is a wrapper
 * over WirelessCommand. All the operations (WHOperation instances) will have an associated instance of OperationEvent,
 * corresponding to the the type request.
 * This list (operation queue) will contain operations from different operation events.
 *
 * Supports wave dependency (each operations is sent in a wave of operations, and operations from a superior wave are only sent when all operations from
 * the previous wave are confirmed) or operation-based dependency (an operation may depend on one or more operations from the queue, not necessarily its set).
 *
 * @author Radu Pop
 */
class WHOperationQueue: public IResponseProcessor
{

        LOG_DEF("h7.n.o.WHOperationQueue")
        ;

    public:

        /**
         * The maximum number of bytes that can be sent into a package in the worst case scenario.
         * At beginning of the join process the max is 75 bytes.
         * After the join response is received the packet max length is 78.
         * When the packages are sent with the short address destination then max length will be 92.
         * @see Doinel Alban's calculations on this matter
         */
        static const uint MAX_PACKAGE_SIZE_ON_JOIN_RESPONSE = 75;
        static const uint MAX_PACKAGE_SIZE_ON_JOIN = 78;
        static const uint MAX_PACKAGE_SIZE_AFTER_JOIN = 80; //92;

        typedef std::list<WHOperationPointer> WHOperationsList;

        WHOperationQueue(IRequestSend& requestSend, CommonData& commonData);

        virtual ~WHOperationQueue();

        void logOperations();

        void generateWHOperations(EngineOperationsListPointer operationEventPointer,
                                  std::vector<WHOperationPointer>& whOperations, WHEngineOperationsVisitor& visitor);

        /**
         * Generates and returns the next handle.
         */
        WHartHandle getNextHandle()
        {
            static WHartHandle currentHandle = 0;
            return ++currentHandle;
        }

        /**
         * Defines the type of a call back that will be invoked when all the operations have been confirmed.
         */
        typedef boost::function1<void, bool /* errorOccured */> OperationsCompletedHandler;

        typedef boost::function1<void, NE::Model::Operations::EngineOperationsListPointer> OperationsConfirmedHandler;

        /**
         * Add the operation to the queue and set the event to the operationEvent.
         */
        void addOperation(WHOperationPointer operation, uint32_t operationEvent,
                          OperationsCompletedHandler completeHandler,
                          OperationDependencyType::OperationDependency dependency = OperationDependencyType::None);

        /**
         * Add the list of operations to the queue and set the event to the operationEvent.
         * Returns true if any operations will be sent.
         */
        bool addOperations(const NE::Model::Operations::EngineOperationsListPointer& engineOperations, std::vector<WHOperationPointer>& operations, uint32_t& operationEventId,
                           std::string eventName, OperationsCompletedHandler completeHandler,
                           OperationsConfirmedHandler confirmHandler, bool stopOnError = true,
                           OperationDependencyType::OperationDependency dependency = OperationDependencyType::None);


        /**
         * Sends publishes and notifications.
         */
        std::set<uint32_t> sendPublishNotify();

        /**
         * Return a package with several commands for the next device address available in the queue.
         * Returns true if there is at least one command available in the queue.
         * If there is at least one command then a new handle is generated and set in all commands from the package.
         *
         * Sends the first MAX_PACKAGE_COMMANDS packages of commands into the field.
         * TODO: the number of packages that are sent will be determined based on the topology, the number
         * of unconfirmed packages, etc.
         */
        bool sendNextPackage(WHartHandle& handle, WHartAddress& destinationAddress, WHartCommandList& list);

        /**
         * Called when a confirm is received ....
         * For all the commands with the specified handle sets the operation status to confirmed.
         * Check all the events for the confirmed commands. If they have no unconfirmed operations then
         * all their corresponding operations are deleted from the operations queue and NetworkEngine is
         * notified that event has been completed.
         */
        void confirmPackage(WHartHandle& handle, WHartLocalStatus localStatus, const WHartCommandList& responses);

        /**
         *
         */
        void sendOperations();

        /**
         *
         */
        void markRemovedDevices(AddressSet& removedDevices);

        /**
         *
         */
        void CancelOperations(uint32_t operationEvent);

        /**
         * Updates the field devices with their new join priority.
         */
        void updateDevicesJoinPriority();

        void SetRemovedForWaitingOperations(AddressSet& removedDevices);


    public:

        /**
         * Set to true when the user sends a SIGUSR2 to the NM.
         */
        static bool saveTopoloyToDisk;

        /**
         *
         */
        static void HandlerSIGUSR2(int signal);

    private:

        /**
         *
         */
        void addOperationsImpl(std::vector<WHOperationPointer>& operations, uint32_t operationEvent,
                               OperationsCompletedHandler completeHandler, bool stopOnError);

        /**
         * Returns the current ASN (Absolute slot number) from the network formation.
         * A slot has 10 milliseconds. Used to set the time when a command become active.
         */
        WHartTime40 getASN();

        /**
         * For each event that has at least one command confirmed we check all of its commands.
         * If they are all confirmed then the event is notified that all the commands were confirmed
         * and remove all of operations associated to this commands form the queue.
         * We also check the timeout of the commands and remove them.
         * (event = EngineOperationsListPointer)
         */
        void checkCommands(std::set<uint32_t> confirmedOperationEvents);

        /**
         * Returns all the commands with the specified handle.
         */
        WHOperationsList getWHOperations(WHartHandle& handle);

        /**
         *
         */
        virtual void ProcessConfirm(WHartHandle requestHandle, const WHartLocalStatus& localStatus,
                                    const WHartCommandList& list);

        /**
         *
         */
        virtual void ProcessIndicate(WHartHandle handle, const WHartAddress& src, WHartPriority priority,
                                     WHartTransportType transportType, const WHartCommandList& list);

    private:

        struct OperationsDependencySort
        {
            public:
                typedef boost::shared_ptr<OperationsDependencySort> Ptr;

            public:
                LOG_DEF("h7.n.o.Operations")
                ;

                OperationsDependencySort()
                {
                    lastDependency = NE::Model::Operations::WaveDependency::FIRST;
                    eventId = 0;
                    isDone = false;
                    stopOnError = false;
                    hasError = false;
                }

                ~OperationsDependencySort()
                {
                }

                void Confirm();

                void GetOperations(std::vector<WHOperationPointer>& newOperations);

            public:

                NE::Model::Operations::WaveDependency::WaveDependencyEnum lastDependency;

                std::vector<WHOperationPointer> operations;

                OperationsCompletedHandler finalCompleteHandler;

                uint32_t eventId;

                bool isDone;

                bool stopOnError;

                bool hasError;
        };


        void ConfirmOperations(OperationsDependencySort::Ptr& operations, bool hasError);
        void ConfirmOperations2(OperationsCompletedHandler completeHandler, bool hasError);

        bool enqueueOperations(OperationsDependencySort::Ptr& operations);

        void SetSentIgnoredOnOperations(OperationsDependencySort::Ptr& operations);

        void StopSendingOperationsFromEvent(WHOperationPointer failedOperation, NE::Model::Operations::EngineOperationsListPointer& opList);

        /**
         * Returns the max package size depending on the kind of the destination address.
         */
        uint8_t getMaxPackageSize(WHartAddress& destinationAddress);

        /**
         * Returns the maximum number of bytes taken by a request or response for this kind of operation.
         */
        uint getOperationSize(WHOperationPointer whOperation);

        /**
         * Check the operation to see if it has dependencies which are not part of the commands package.
         * Returns true if the command can be added to the package.
         */
        bool existsOperationDependency(WHOperationsList& packageCommands, WHOperationPointer operation);

        bool existsOperationInQueue(IEngineOperationPointer operation);

        /**
         * For commands that modifies the device tables (like write, delete link, graph etc) the device responds with
         * a field that reflects the remaining number of possible resources that could be added. This information will
         * be set into the engine in device's history.
         */
        void updateDeviceMetadata(WHartAddress& address, uint16_t cmdId, void* command);

        /**
         * Updates the device history with the information from the parameters when a new packet is about to be sent.
         */
        void updateDeviceHistoryForSentPacket(WHartAddress& address, uint packageSize,
                                              WHOperationsList& packageCommands);

        /**
         * Confirm the last package in the device history.
         */
        void updateDeviceHistoryForConfirmPackage(WHartAddress& address);

        /**
         * Logs the current operation and the command generated from it.
         */
        void logCommand(WHOperationPointer wHOperation);

        /**
         * Forwards the success commands to GW.
         */
        void forwardCmdsToGw(WHartAddress destinationAddress, const WHartCommandList& responses, std::list<uint16_t>& successCmdIndexes);

        /**
         * Adds dependencies to existing operations.
         */
        void addDependencies(OperationsDependencySort::Ptr& operations, OperationDependencyType::OperationDependency dependency);

        /*
         * Adds dependencies to existing operations (for No Waves operation mode)
         */
        void addDependenciesNoWavesMode(const NE::Model::Operations::EngineOperationsListPointer& engineOperations, std::vector<WHOperationPointer>& operations, OperationDependencyType::OperationDependency dependency);


        NE::Model::Operations::EngineOperationsListPointer GenerateOperationsList(uint32_t eventId);

        void CallConfirmOps(uint32_t eventId, EngineOperationsListPointer& engineOperations);

    private:
        LogQueue logQueue;

        /**
         * The network manager start up time.
         */
        time_t managerStartTime;

        /**
         * Represents all the operations generated by the events in the engine.
         */
        WHOperationsList operationsQueue;

        /**
         *
         */
        typedef std::map<uint32_t, OperationsCompletedHandler> HandlersMap;
        HandlersMap completeHandlers;

        typedef std::map<uint32_t, bool> StopOnErrorMap;
        StopOnErrorMap stopOnErrorsMap;

        /**
         *
         */
        IRequestSend& requestSend;

        /**
         *
         */
        CommonData& commonData;

        /**
         *
         */
        DevicesJoinPriorityFlow devJoinPriorityFlow;

        OperationsResponseCodes responseCodes;

        /**
         * Represents the addresses of the devices to whom a package has been already sent.
         */
        std::vector<WHartAddress> alreadySentToAddresses;

        std::map<uint32_t, OperationsDependencySort::Ptr> operationDependencySorters;

        typedef std::map<uint32_t, OperationsConfirmedHandler> ConfirmHandlersMap;
        ConfirmHandlersMap confirmHandlers;

        typedef std::map<uint32_t, NE::Model::Operations::EngineOperationsListPointer> OperationsEventsMap;
        OperationsEventsMap operationsMap;

        std::map<WHartHandle, WHartAddress> waitingPacketConfirms;

        std::map<WHartHandle, WHOperationsList> waitingPackets;

        bool isSendingCommand;

        int currentEventIndex;


};

}

}
}

#endif /* WHOPERATIONQUEUE_H_ */
