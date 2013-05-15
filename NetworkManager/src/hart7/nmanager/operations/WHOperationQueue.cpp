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
 * WHOperationQueue.cpp
 *
 *  Created on: Dec 16, 2008
 *      Author: Radu Pop
 */

#include "WHOperationQueue.h"
#include "OperationsDependency.h"
#include <boost/bind.hpp>
#include <time.h>
#include "WHOperationQueueLogger.h"
#include "../FlowHandlers/IDefaultCommandHandler.h"
#include "../../util/NMLog.h"
#include "../../util/ManagerUtils.h"
#include "SMState/SMStateLog.h"


class WHOperation;

namespace hart7 {

namespace nmanager {

namespace operations {

class ConfirmFromStackException: std::exception
{
};

WHOperationQueue::WHOperationQueue(IRequestSend& requestSend_, CommonData& commonData_) :
    requestSend(requestSend_), commonData(commonData_), devJoinPriorityFlow(commonData_),
                responseCodes(commonData.settings.handleAllReponseCodesAsError)
{
    managerStartTime = time(NULL);
    currentEventIndex = 1;
    isSendingCommand = false;
}

WHOperationQueue::~WHOperationQueue()
{
}

void WHOperationQueue::logCommand(WHOperationPointer wHOperation)
{
    if (wHOperation->getWHartCommand().commandID == CMDID_C64765_NivisMetaCommand)
    {
        // 64765 is sent as a response
        hart7::util::NMLog::logOperationResponse(wHOperation);
    }
    else
    {
        hart7::util::NMLog::logOperation(wHOperation);
    }
}

WHartTime40 WHOperationQueue::getASN()
{
    time_t seconds = (time(NULL) - managerStartTime) * 100;
    WHartTime40 wtime;
    wtime.u32 = seconds;

    return wtime;
}

void WHOperationQueue::addOperation(WHOperationPointer operation, uint32_t operationEvent,
                                    OperationsCompletedHandler completeHandler,
                                    OperationDependencyType::OperationDependency dependency)
{
    operation->setOperationEvent(operationEvent);
    this->operationsQueue.push_back(operation);

    completeHandlers.insert(std::make_pair(operationEvent, completeHandler));
}

void WHOperationQueue::addDependencies(OperationsDependencySort::Ptr& operations,
                                       OperationDependencyType::OperationDependency dependency)
{
    if (dependency == OperationDependencyType::None)
        return;


    for (std::vector<WHOperationPointer>::iterator it = operations->operations.begin(); it
                != operations->operations.end(); ++it)
    {
        for (std::map<uint32_t, OperationsDependencySort::Ptr>::iterator opDIt = operationDependencySorters.begin(); opDIt
                    != operationDependencySorters.end(); ++opDIt)
        {
            for (std::vector<WHOperationPointer>::iterator opIt = (opDIt->second)->operations.begin(); opIt
                        != (opDIt->second)->operations.end(); ++opIt)
            {
                if ((*opIt)->getOperationStatus() == OperationState::GENERATED)
                {
                    if ( dependency == OperationDependencyType::DependOnAll )
                    {
                        (*it)->getEngineOperation()->setOperationDependency((*opIt)->getEngineOperation(), true);
                    }
                    else if ( dependency == OperationDependencyType::DependOnSameAddress &&
                                ((*it)->getEngineOperation()->getOwner() == (*opIt)->getEngineOperation()->getOwner()) ){

                        (*it)->getEngineOperation()->setOperationDependency((*opIt)->getEngineOperation(), true);

                    } else if ( dependency == OperationDependencyType::DependOnCheckRemoved &&
                                (*opIt)->getOperationEventType() ==  NetworkEngineEventType::REMOVE_DEVICES ){

                        (*it)->getEngineOperation()->setOperationDependency((*opIt)->getEngineOperation(), true);

                    }
                }
            }
        }
    }
}

void WHOperationQueue::addDependenciesNoWavesMode(const NE::Model::Operations::EngineOperationsListPointer& engineOperations, std::vector<WHOperationPointer>& operations,
                                       OperationDependencyType::OperationDependency dependency)
{

    if (dependency == OperationDependencyType::None)
        return;

    Address32 joiningDeviceAddress = engineOperations->getRequesterAddress32();
    Device& joiningDevice = NE::Model::NetworkEngine::instance().getDevice(joiningDeviceAddress);
    Address32 parentDevice = joiningDevice.parent32;

    for (std::vector<WHOperationPointer>::iterator it = operations.begin(); it != operations.end(); ++it)
    {

        for (WHOperationsList::iterator opIt = operationsQueue.begin(); opIt != operationsQueue.end(); ++opIt)
        {
            //dependencies between join operations and all operations for the parent
            if ((*it)->getOperationEventType() ==  NetworkEngineEventType::JOIN_REQUEST
                        && (*opIt)->getOperationEventType() !=  NetworkEngineEventType::JOIN_REQUEST
                            && (*opIt)->getOperationEventType() !=  NetworkEngineEventType::REMOVE_DEVICES
                                && (*opIt)->getOperationStatus() == OperationState::GENERATED) {
                if ((*opIt)->getEngineOperation()->getOwner() == parentDevice
                            && parentDevice != joiningDevice.joinedBackbone32) {
                    (*opIt)->getEngineOperation()->setOperationDependency((*it)->getEngineOperation(), true);
                }
            }

            //dependencies between SetClkSrc ops
            if ((*it)->getEngineOperation()->getOperationType() ==  EngineOperationType::SET_CLOCK_SOURCE &&
                        (*opIt)->getEngineOperation()->getOperationType() ==  EngineOperationType::SET_CLOCK_SOURCE &&
                            (*opIt)->getEngineOperation()->getOwner() == (*it)->getEngineOperation()->getOwner() &&
                                ((boost::shared_ptr<SetClockSourceOperation>&)(*it))->neighborAddress == ((boost::shared_ptr<SetClockSourceOperation>&)(*opIt))->neighborAddress ){

                (*it)->getEngineOperation()->setOperationDependency((*opIt)->getEngineOperation(), true);
            }



                if ((*opIt)->getOperationStatus() == OperationState::GENERATED
                            || (*opIt)->getOperationStatus() == OperationState::SEND_ATTEMPT )
                {
                    if ( dependency == OperationDependencyType::DependOnAll )
                    {
                        try
                        {
                            (*it)->getEngineOperation()->setOperationDependency((*opIt)->getEngineOperation(), true);
                        }
                        catch (std::exception& ex)
                        {
                            LOG_WARN(ex.what());
                        }
                    }
                    else if ( dependency == OperationDependencyType::DependOnSameAddress &&
                                ((*it)->getEngineOperation()->getOwner() == (*opIt)->getEngineOperation()->getOwner()) ){
                        try
                        {
                            (*it)->getEngineOperation()->setOperationDependency((*opIt)->getEngineOperation(), true);
                        }
                        catch (std::exception& ex)
                        {
                            LOG_WARN(ex.what());
                        }
                    } else if ( dependency == OperationDependencyType::DependOnCheckRemoved &&
                                (*opIt)->getOperationEventType() ==  NetworkEngineEventType::REMOVE_DEVICES)
                    {
                        try
                        {
                            if (NetworkEngine::instance().getDevicesTable().getDevice((*opIt)->getEngineOperation()->getOwner()).isUdp())
                            {
                                (*it)->getEngineOperation()->setOperationDependency((*opIt)->getEngineOperation(), true);
                            }
                        }
                        catch (std::exception& ex)
                        {
                            LOG_WARN(ex.what());
                        }
                    }


                }
            }
        }
    }


bool WHOperationQueue::addOperations(const NE::Model::Operations::EngineOperationsListPointer& engineOperations, std::vector<WHOperationPointer>& operations, uint32_t& operationEventId,
                                     std::string eventName, OperationsCompletedHandler completeHandler, OperationsConfirmedHandler confirmHandler,
                                     bool stopOnError, OperationDependencyType::OperationDependency dependency)
{
    if (operations.size() == 0)
    {
        return false;
    }

    for (std::vector<WHOperationPointer>::iterator it = operations.begin(); it != operations.end(); ++it)
    {
        if ((*it)->getEngineOperation()->getDependency() >= NE::Model::Operations::WaveDependency::MAX_DEPENDENCY)
        {
            LOG_ERROR("Operation with wrong dependency: " << (*it)->toString());
            return false;
        }
    }


    currentEventIndex++;
    if (currentEventIndex == 0)
    {
        currentEventIndex = 1;
    }
    operationEventId = currentEventIndex;

    operationsMap[operationEventId] = engineOperations;

    logQueue.logNewEvent(currentEventIndex, eventName, operations);

    confirmHandlers.insert(std::make_pair(currentEventIndex, confirmHandler));

    if (commonData.settings.useWaves)
    {
        OperationsDependencySort::Ptr opDepSort(new OperationsDependencySort());
        opDepSort->lastDependency = NE::Model::Operations::WaveDependency::FIRST;
        opDepSort->operations = operations;
        opDepSort->finalCompleteHandler = completeHandler;
        opDepSort->eventId = currentEventIndex;
        opDepSort->isDone = false;
        opDepSort->stopOnError = stopOnError;
        opDepSort->hasError = false;

        addDependencies(opDepSort, dependency);

        SMState::SMStateLog::logOperations(eventName, *engineOperations, false);

        operationDependencySorters[operationEventId] = opDepSort;

        return enqueueOperations(opDepSort);
    }
    else
    {
        addDependenciesNoWavesMode( engineOperations, operations, dependency );

        SMState::SMStateLog::logOperations(eventName, *engineOperations, false);

        if (completeHandler) {
            addOperationsImpl(operations, operationEventId, boost::bind(&WHOperationQueue::ConfirmOperations2, this, completeHandler, _1), stopOnError);
        } else {
            addOperationsImpl(operations, operationEventId, NULL, stopOnError);
        }

        return true;
    }
}

bool WHOperationQueue::enqueueOperations(OperationsDependencySort::Ptr& operations)
{
    std::vector<WHOperationPointer> newOperations;
    operations->GetOperations(newOperations);

    if (newOperations.empty())
    {
        LOG_TRACE("New Operations Empty.");
        if (operations->lastDependency < NE::Model::Operations::WaveDependency::MAX_DEPENDENCY)
        {
            LOG_TRACE("New Operations Empty - Confirming.");
            operations->Confirm();
            return enqueueOperations(operations);
        }
        else
        {
            LOG_TRACE("Operations completed confirming.");
            logQueue.logConfirmEvent(operations->eventId, operations->hasError);
            if (operations->finalCompleteHandler)
            {
                operations->finalCompleteHandler(operations->hasError);
            }

            operationDependencySorters.erase(operations->eventId);

            return false;
        }
    }
    else
    {
        LOG_TRACE("enqueueOperations() : Sending operations with dependency=" << (int) operations->lastDependency << ".");

        // this will generate a copy of OperationsDependencySort - see if it bothers
        addOperationsImpl(newOperations, operations->eventId, boost::bind(&WHOperationQueue::ConfirmOperations, this,
                                                                          operations, _1), operations->stopOnError);
        return true;
    }
}

void WHOperationQueue::CancelOperations(uint32_t operationEvent)
{
    if (operationEvent == 0)
    {
        return;
    }

    ConfirmHandlersMap::iterator confIt = confirmHandlers.find(operationEvent);
    if (confIt != confirmHandlers.end())
    {
        NE::Model::Operations::EngineOperationsListPointer opList = GenerateOperationsList(operationEvent);
        for (WHOperationsList::iterator it = operationsQueue.begin(); it != operationsQueue.end(); ++it)
        {
            if ((*it)->getOperationEvent() == operationEvent)
            {
                if ((*it)->getOperationStatus() != OperationState::CONFIRMED)
                {
                    (*it)->setOperationStatus(OperationState::SENT_IGNORED);
                    opList->addOperation((*it)->getEngineOperation());
                    LOG_DEBUG("Attempting to confirm opeation id=" << (*it)->getEngineOperation()->getOperationId());
                }
            }
        }

        CallConfirmOps(operationEvent, opList);
        confirmHandlers.erase(confIt);
    }

    WHOperationsList::iterator itRemoved = std::remove_if(operationsQueue.begin(), operationsQueue.end(),
                                                          OperationsEventPredicate(operationEvent));

    if (itRemoved != operationsQueue.end())
    {
        operationsQueue.erase(itRemoved, operationsQueue.end());
    }


    HandlersMap::iterator it = completeHandlers.find(operationEvent);
    if (it != completeHandlers.end())
    {
        OperationsCompletedHandler tmp = it->second;

        completeHandlers.erase(it);

        // HACK: calling this with (it->second)(true) instead of (tmp)(true)
        // will generate a call cycle and finally a crash ..

        try
        {
            (tmp)(true);
        }
        catch (std::exception&)
        {
            LOG_DEBUG("Cancel operations failed when notifying.");
        }
    }
}

void WHOperationQueue::SetSentIgnoredOnOperations(OperationsDependencySort::Ptr& operations)
{
    for (std::vector<WHOperationPointer>::iterator it = operations->operations.begin(); it
                != operations->operations.end(); ++it)
    {
        if ((*it)->getEngineOperation()->getState() == NE::Model::Operations::OperationState::GENERATED)
        {
            (*it)->getEngineOperation()->setState(NE::Model::Operations::OperationState::SENT_IGNORED);
            (*it)->getEngineOperation()->setErrorCode(NE::Model::Operations::ErrorCode::ERROR);
        }
    }
}

void WHOperationQueue::ConfirmOperations(OperationsDependencySort::Ptr& operations, bool hasError)
{
    LOG_TRACE("Confirmed operations with dependency=" << (int) operations->lastDependency << ".");
    operations->Confirm();

    if (hasError)
    {
        operations->hasError = true;
        if (operations->stopOnError)
        {
            LOG_INFO("Error received on operations. Not sending the rest of the operations.");

            SetSentIgnoredOnOperations(operations);
            operations->lastDependency = NE::Model::Operations::WaveDependency::MAX_DEPENDENCY;
        }
    }

    if (enqueueOperations(operations))
    {
        sendOperations();
    }
}

void WHOperationQueue::OperationsDependencySort::Confirm()
{
    lastDependency = (NE::Model::Operations::WaveDependency::WaveDependencyEnum) ((int) lastDependency + 1);
}

void WHOperationQueue::OperationsDependencySort::GetOperations(std::vector<WHOperationPointer>& newOperations)
{
    for (std::vector<WHOperationPointer>::iterator it = operations.begin(); it != operations.end(); ++it)
    {
        if ((*it)->getEngineOperation()->getDependency() == lastDependency && (*it)->getOperationStatus()
                    == NE::Model::Operations::OperationState::GENERATED)
        {
            (*it)->setOperationEvent(eventId);
            newOperations.push_back(*it);
        }
    }
}

void WHOperationQueue::addOperationsImpl(std::vector<WHOperationPointer>& operations, uint32_t operationEvent,
                                         OperationsCompletedHandler completeHandler, bool stopOnError)
{
    //add the list of operations to the queue and set the event to the operationEvent
    for (uint i = 0; i < operations.size(); i++)
    {
        operations[i]->setOperationEvent(operationEvent);
        this->operationsQueue.push_back(operations[i]);
    }

    completeHandlers[operationEvent] = completeHandler;
    stopOnErrorsMap[operationEvent] = stopOnError;
}

void WHOperationQueue::ConfirmOperations2(OperationsCompletedHandler completeHandler, bool hasError)
{
    if (completeHandler) {
        completeHandler(hasError);
    } else {
        LOG_WARN("completeHandler is NULL!");
    }

}

void WHOperationQueue::sendOperations()
{
    WHartHandle handle;
    WHartAddress destinationAddress;
    WHartCommandList list;

    std::set<uint32_t> events = sendPublishNotify();

    while (sendNextPackage(handle, destinationAddress, list))
    {
        // do nothing
    }

    for (WHOperationsList::iterator it = operationsQueue.begin(); it != operationsQueue.end(); ++it)
    {
        if ((*it)->getOperationStatus() == NE::Model::Operations::OperationState::SEND_ATTEMPT)
        {
            (*it)->setOperationStatus(NE::Model::Operations::OperationState::GENERATED);
        }
    }

    checkCommands(events);
}

void WHOperationQueue::SetRemovedForWaitingOperations(AddressSet& removedDevices)
{
    for (std::map<uint32_t, OperationsDependencySort::Ptr>::iterator opDIt = operationDependencySorters.begin(); opDIt
                != operationDependencySorters.end(); ++opDIt)
    {
        for (std::vector<WHOperationPointer>::iterator opIt = (opDIt->second)->operations.begin(); opIt
                    != (opDIt->second)->operations.end(); ++opIt)
        {
            if (removedDevices.find((*opIt)->getEngineOperation()->getOwner()) != removedDevices.end())
            {
                if ((*opIt)->getOperationStatus() == NE::Model::Operations::OperationState::SENT) //already sent operations
                {
                    // do not change their status, as we do not know what the result is (they might be confirmed or not)
                }
                else if ((*opIt)->getOperationStatus() == NE::Model::Operations::OperationState::GENERATED)
                {
                    (*opIt)->setOperationStatus(NE::Model::Operations::OperationState::SENT_IGNORED);
                    (*opIt)->getEngineOperation()->setErrorCode(NE::Model::Operations::ErrorCode::ERROR);
                }
            }
        }
    }
}

void WHOperationQueue::markRemovedDevices(AddressSet& removedDevices)
{
    for (AddressSet::iterator it = removedDevices.begin(); it != removedDevices.end(); ++it)
    {
        LOG_DEBUG("Marking for remove device address " << ToStr((*it)));
    }

    std::map<uint32_t, EngineOperationsListPointer> engineOps;
    std::set<uint32_t> events;
    for (WHOperationsList::iterator it = operationsQueue.begin(); it != operationsQueue.end(); ++it)
    {
        if (removedDevices.find((*it)->getEngineOperation()->getOwner()) != removedDevices.end())
        {
            if ((*it)->getOperationStatus() == NE::Model::Operations::OperationState::GENERATED ||
                        (*it)->getOperationStatus() == NE::Model::Operations::OperationState::SENT) //already sent operations
            {
                (*it)->setOperationStatus(NE::Model::Operations::OperationState::SENT_IGNORED);
                (*it)->getEngineOperation()->setErrorCode(NE::Model::Operations::ErrorCode::ERROR);

                uint32_t eventId = (*it)->getOperationEvent();
                events.insert(eventId);

                std::map<uint32_t, EngineOperationsListPointer>::iterator engineOpIt = engineOps.find(eventId);
                if (engineOpIt == engineOps.end())
                {
                    engineOpIt = engineOps.insert(std::make_pair(eventId, GenerateOperationsList(eventId))).first;
                }

                if (engineOpIt != engineOps.end())
                {
                    engineOpIt->second->addOperation((*it)->getEngineOperation());
                }
            }
        }
    }
    // this is done for the waves mode
    SetRemovedForWaitingOperations(removedDevices);

    //[andy] - do not set all waiting to SI, wait for them to confirm. ESPECIALLY SetClockSources
    for (WHOperationsList::iterator it = operationsQueue.begin(); it != operationsQueue.end(); ++it)
    {
        std::set<uint32_t>::iterator setIt = events.find((*it)->getOperationEvent());
        if (setIt != events.end() && stopOnErrorsMap[*setIt] &&
                    (*it)->getOperationStatus() == NE::Model::Operations::OperationState::GENERATED)
            // sent ignored only if set should stop on error, if NOT confirmed
        {
            if ((*it)->getWHartCommand().commandID != CMDID_C971_WriteNeighbourPropertyFlag)    // do not set SetClockSource to SI
            {
                (*it)->setOperationStatus(NE::Model::Operations::OperationState::SENT_IGNORED);
                (*it)->getEngineOperation()->setErrorCode(NE::Model::Operations::ErrorCode::ERROR);

                std::map<uint32_t, EngineOperationsListPointer>::iterator engineOpIt = engineOps.find((*it)->getOperationEvent());
                if (engineOpIt != engineOps.end())
                {
                    engineOpIt->second->addOperation((*it)->getEngineOperation());
                }
            }
        }
    }
    // call confirm on the SI operations
    for (std::map<uint32_t, EngineOperationsListPointer>::iterator engineOpIt = engineOps.begin();
                engineOpIt != engineOps.end(); engineOpIt++)
    {
        if (engineOpIt->second->getSize() != 0)
        {
            LOG_DEBUG("Calling confirm on operations with SI from eventId=" << engineOpIt->first);
            CallConfirmOps(engineOpIt->first, engineOpIt->second);
        }
    }

    checkCommands(events);
}

uint WHOperationQueue::getOperationSize(WHOperationPointer whOperation)
{
    // TODO: the size can be taken from the C971_WriteNeighbourPropertyFlag_ReqSize ....
    // to the request size we have to add 3 bytes and to the response we have to add 4 bytes
    // (the command id 2 bytes, 1 byte the length ,1 byte response code (only for response))
    switch (whOperation->getWHartCommand().commandID)
    {
        case CMDID_C795_WriteTimerInterval:
            return 9; // request 5 + 3, response 5 + 4
        case CMDID_C811_WriteJoinPriority:
            return 5; // request 1 + 3, response 1 + 4
        case CMDID_C839_ChangeNotification:
            return 11; // request 8 + 3, response 5 + 4
        case CMDID_C973_WriteService:
            return 15; // request 10 + 3, response 11 + 4
        case CMDID_C801_DeleteService:
            return 7; // request 2 + 3, response 3 + 4
        case CMDID_C818_WriteChannelBlacklist:
            return 5; // request 1 + 2, response 1 + 2 + 2
        case CMDID_C974_WriteRoute:
            return 10; // request 5 + 3, response 6 + 4
        case CMDID_C976_WriteSourceRoute:
            // Byte  Format      Description
            // 0     Unsigned-8  Route ID
            // 1     Unsigned-8  Number of hops
            // 2-3   Unsigned-16 Nickname hop entry 0
            // 4-... Unsigned-16 Repeated for number of entries indicated in response byte 1
            return MAX_PACKAGE_SIZE_AFTER_JOIN;
        case CMDID_C975_DeleteRoute:
            return 6; // request 1 + 3, response 2 + 4
        case CMDID_C977_DeleteSourceRoute:
            return 5; // request 1 + 3, response 1 + 4
        case CMDID_C961_WriteNetworkKey:
            return 25; // request 21 + 3, response 21 + 4
        case CMDID_C962_WriteDeviceNicknameAddress:
            return 6; // request 2 + 3, response 2 + 4
        case CMDID_C963_WriteSession:
            return 38; // request 34 + 3, response 34 + 4
        case CMDID_C965_WriteSuperframe:
            return 14; // request 10 + 3, response 10 + 4
        case CMDID_C967_WriteLink:
            return 14; // request 8 + 3, response 10 + 4
        case CMDID_C968_DeleteLink:
            return 11; // request 5 + 3, response 7 + 4
        case CMDID_C969_WriteGraphNeighbourPair:
            return 9; // request 4 + 3, response 5 + 4
        case CMDID_C970_DeleteGraphConnection:
            return 9; // request 4 + 3, response 5 + 4
        case CMDID_C971_WriteNeighbourPropertyFlag:
            return 7; // request 3 + 3, response 3 + 4
        case CMDID_C777_ReadWirelessDeviceInformation:
            return 22; //request 0, response 1 + 4 + 4 + 4 + 1 + 4 + 2 + 2
        case CMDID_C64765_NivisMetaCommand:
            return MAX_PACKAGE_SIZE_AFTER_JOIN;
        default:
            LOG_ERROR("Command ID : " << (int) whOperation->getWHartCommand().commandID
                        << " not processed yet! Update getOperationSize() for this cmdId!");
            return MAX_PACKAGE_SIZE_AFTER_JOIN;
    }
}

uint8_t WHOperationQueue::getMaxPackageSize(WHartAddress& destinationAddress)
{
    if (destinationAddress.type == WHartAddress::whartaUniqueID)
    {
        return MAX_PACKAGE_SIZE_ON_JOIN_RESPONSE;
    }
    else if (destinationAddress.type == WHartAddress::whartaProxy || destinationAddress.type
                == WHartAddress::whartaProxyShort)
    {
        return MAX_PACKAGE_SIZE_ON_JOIN;
    }
    else if (destinationAddress.type == WHartAddress::whartaNickname)
    {
        return MAX_PACKAGE_SIZE_AFTER_JOIN;
    }

    LOG_ERROR("Invalid address type!");
    return MAX_PACKAGE_SIZE_ON_JOIN_RESPONSE;
}

bool IsPublishNotify(uint16_t commandId)
{
    switch (commandId)
    {
        case CMDID_C000_ReadUniqueIdentifier:
        case CMDID_C769_ReadJoinStatus:
        case CMDID_C832_ReadNetworkDeviceIdentity:
        case CMDID_C839_ChangeNotification:
            return true;
        case CMDID_C64765_NivisMetaCommand: // TODO if we'll have to send request also then it would not work
            return true;
        default:
            return false;
    }
}

std::set<uint32_t> WHOperationQueue::sendPublishNotify()
{
    std::set<uint32_t> events;
    if (this->operationsQueue.size() == 0)
    {
        return events;
    }

    WHOperationsList::iterator itOperation = operationsQueue.end();
    for (itOperation = operationsQueue.begin(); itOperation != operationsQueue.end(); ++itOperation)
    {
        if ((*itOperation)->getOperationStatus() == OperationState::GENERATED)
        {
            WHOperationPointer whOperation = *itOperation;
            WHartAddress destinationAddress = whOperation->getDestinationAddress();
            if (IsPublishNotify(whOperation->getWHartCommand().commandID))
            {
                WHOperationsList packageCommands;

                if (existsOperationDependency(packageCommands, whOperation))
                {
                    continue;
                }

                WHartCommand packCommandsArray[1];
                packCommandsArray[0] = whOperation->getWHartCommand();

                WHartCommandList list;
                list.count = 1;
                list.list = packCommandsArray;

                NE::Model::Services::Service& service = commonData.utils.GetServiceTo(destinationAddress);

                WHartHandle handle = requestSend.TransmitRequest(destinationAddress, stack::whartpCommand,
                                                                 (stack::WHartServiceID) service.getServiceId(),
                                                                 stack::wharttPublishNotify, list,
                                                                 commonData.utils.GetSessionType(destinationAddress),
                                                                 *this);

                whOperation->setOperationStatus(OperationState::CONFIRMED);

                packageCommands.push_back(whOperation);

                hart7::util::NMLog::logCommandResponse(whOperation->getWHartCommand().commandID, 0,
                                                       whOperation->getWHartCommand().command, destinationAddress);
                logQueue.logSendPacket(destinationAddress, handle, packageCommands);
                logQueue.logConfirmPacket(handle, 0, packageCommands, list);

                NE::Model::Operations::EngineOperationsListPointer opList = GenerateOperationsList(whOperation->getOperationEvent());
                opList->addOperation((*itOperation)->getEngineOperation());
                LOG_DEBUG("Attempting to confirm opeation id=" << (*itOperation)->getEngineOperation()->getOperationId());

                CallConfirmOps((*itOperation)->getOperationEvent(), opList);

                events.insert(whOperation->getOperationEvent());
            }
        }
    }

    return events;
}

bool WHOperationQueue::sendNextPackage(WHartHandle& handle, WHartAddress& destinationAddress, WHartCommandList& list)
{
    if (this->operationsQueue.size() == 0)
    {
        return false;
    }

    WHOperationsList::iterator itOperation = operationsQueue.end();
    WHartAddress address;
    WHOperationsList packageCommands;

    for (itOperation = operationsQueue.begin(); itOperation != operationsQueue.end(); ++itOperation)
    {
        if ((*itOperation)->getOperationStatus() == OperationState::GENERATED)
        {
            WHOperationPointer wHOperation = *itOperation;

            if (!requestSend.CanSendToAddress(wHOperation->getDestinationAddress())
                        || (existsOperationDependency(packageCommands, wHOperation)))
                continue;

            // get the first index of the address
            address = wHOperation->getDestinationAddress();
            break;
        }
    }

    if (itOperation == operationsQueue.end())
    {
        return false;
    }

    uint packageSize = 0;
    destinationAddress = address;

    bool addedCommand = false;
    do
    {
        addedCommand = false;
        // determine the first operations for the address
        for (WHOperationsList::iterator innerOperation = operationsQueue.begin(); innerOperation != operationsQueue.end(); ++innerOperation)
        {
            if ((*innerOperation)->getDestinationAddress() == address)
            {
                if ((*innerOperation)->getOperationStatus() == OperationState::GENERATED)
                {
                    WHOperationPointer wHOperation = *innerOperation;

                    // TODO in the nearest future we should optimize the packing of the commands
                    if (existsOperationDependency(packageCommands, wHOperation))
                    {
                        continue;
                    }

                    if (getOperationSize(wHOperation) + packageSize <= getMaxPackageSize(destinationAddress))
                    {
                        addedCommand = true;
                        packageSize += getOperationSize(wHOperation);
                        wHOperation->setOperationStatus(OperationState::SEND_ATTEMPT);
                        packageCommands.push_back(wHOperation);

                        logCommand(wHOperation);
                    }
                    else
                    {
                        // we reached the limit of the package;
                        addedCommand = false; // exit while
                        break;
                    }
                }
            }
        }
    } while (addedCommand); // loop until no operations are added.

    WHartCommand packCommandsArray[packageCommands.size()];
    int i = 0;
    for (WHOperationsList::iterator it = packageCommands.begin(); it != packageCommands.end(); it++, i++)
    {
        packCommandsArray[i] = (*it)->getWHartCommand();
    }

    list.count = packageCommands.size();
    list.list = packCommandsArray;

    // update the device history
    updateDeviceHistoryForSentPacket(address, packageSize, packageCommands);

    if (destinationAddress == NetworkManager_Nickname() || destinationAddress == NetworkManager_UniqueID())
    {
        // will be intercepted by ManagerStack, so besides address and commands, the rest don't really matter
        handle = requestSend.TransmitRequest(NetworkManager_Nickname(), stack::whartpCommand,
                                             (stack::WHartServiceID) 0, stack::wharttRequestUnicast, list,
                                             stack::WHartSessionKey::sessionKeyed, *this);

        std::list<uint16_t> successCmdIndexes;
        std::set<uint32_t> confirmedEngineOpList;
        int i = 0;
        for (WHOperationsList::iterator it = packageCommands.begin(); it != packageCommands.end(); ++it, ++i)
        {
            (*it)->setOperationStatus(OperationState::CONFIRMED);
            successCmdIndexes.push_back(i);

            // should only happen for the last command generated by an event
            if ((*it)->getEngineOperation()->getState() == OperationState::CONFIRMED)
            {
                confirmedEngineOpList.insert((*it)->getOperationEvent());
            }

            NE::Model::Operations::EngineOperationsListPointer opList = GenerateOperationsList((*it)->getOperationEvent());
            opList->addOperation((*it)->getEngineOperation());
            LOG_DEBUG("Attempting to confirm opeation id=" << (*it)->getEngineOperation()->getOperationId());

            CallConfirmOps((*it)->getOperationEvent(), opList);
        }

        forwardCmdsToGw(destinationAddress, list, successCmdIndexes);

        logQueue.logSendPacket(address, handle, packageCommands);
        logQueue.logConfirmPacket(handle, WHartLocalStatus::whartlsSuccess, packageCommands, list);

        checkCommands(confirmedEngineOpList);
    }
    else
    {
        try
        {
            isSendingCommand = true;
            NE::Model::Services::Service& service = commonData.utils.GetServiceTo(destinationAddress);

            handle = requestSend.TransmitRequest(destinationAddress, stack::whartpCommand,
                                                 (stack::WHartServiceID) service.getServiceId(),
                                                 stack::wharttRequestUnicast, list,
                                                 commonData.utils.GetSessionType(destinationAddress), *this);

            for (WHOperationsList::iterator it = packageCommands.begin(); it != packageCommands.end(); ++it)
            {
                (*it)->setHandle(handle);
                (*it)->setOperationStatus(OperationState::SENT);
            }

            waitingPacketConfirms[handle] = destinationAddress;
            waitingPackets[handle] = packageCommands;

            isSendingCommand = false;
            logQueue.logSendPacket(address, handle, packageCommands);
        }
        catch (ConfirmFromStackException&)
        {
            LOG_DEBUG("Cannot send to destination " << destinationAddress);
            isSendingCommand = false;
        }
        catch (...)
        {
            LOG_DEBUG("Cannot find service to destination=" << destinationAddress);

            std::set<uint32_t> confirmedEngineOpList;
            for (WHOperationsList::iterator it = packageCommands.begin(); it != packageCommands.end(); ++it)
            {
                (*it)->setOperationStatus(OperationState::CONFIRMED);
                (*it)->getEngineOperation()->setErrorCode(NE::Model::Operations::ErrorCode::ERROR);

                // should only happen for the last command generated by an event
                if ((*it)->getEngineOperation()->getState() == OperationState::CONFIRMED)
                {
                    confirmedEngineOpList.insert((*it)->getOperationEvent());
                }
            }

            isSendingCommand = false;

            checkCommands(confirmedEngineOpList);
        }
    }

    return true;
}


void WHOperationQueue::CallConfirmOps(uint32_t eventId, EngineOperationsListPointer& engineOperations)
{
    ConfirmHandlersMap::iterator confIt = confirmHandlers.find(eventId);
    if (confIt != confirmHandlers.end())
    {
        try
        {
            if (confIt->second)
                confIt->second(engineOperations);
        }
        catch (...)
        {
            LOG_WARN("An error occured while confirming operation.");
        }
    }
}

void WHOperationQueue::updateDeviceMetadata(WHartAddress& address, uint16_t cmdId, void* cmd)
{
    //LOG_TRACE("updateDeviceMetadata for address: " << address.ToString());

    try
    {
        if (!hart7::util::existsDevice(commonData.networkEngine.getDevicesTable(), address))
        {
            return;
        }

        NE::Model::Device& device = hart7::util::getDevice(commonData.networkEngine.getDevicesTable(), address);
        MetaDataAttributesPointer metaAttr = device.getMetaDataAttributes();
        uint8_t joinPriorityBefore = metaAttr->getJoinPriority();

        if (cmdId == CMDID_C777_ReadWirelessDeviceInformation)
        {
            LOG_DEBUG("MaxNoOfNeighbors = " << ((C777_ReadWirelessDeviceInformation_Resp*) cmd)->m_unMaxNoOfNeighbors );
            metaAttr->setMaxNeighbors(((C777_ReadWirelessDeviceInformation_Resp*) cmd)->m_unMaxNoOfNeighbors);
        }
        // update the services no
        else if (cmdId == CMDID_C973_WriteService)
        {
            metaAttr->updateTotalAndUsedServicesFromRemaining(((C973_WriteService_Resp*) cmd)->m_ucRemainingServicesNo);
        }
        else if (cmdId == CMDID_C801_DeleteService)
        {
            metaAttr->updateTotalAndUsedServicesFromRemaining(
                                                              ((C801_DeleteService_Resp*) cmd)->m_ucNoOfServiceEntriesRemaining);
        }
        // update the  routes no

        else if (cmdId == CMDID_C974_WriteRoute)
        {
            metaAttr->updateTotalAndUsedRoutesFromRemaining(((C974_WriteRoute_Resp*) cmd)->m_ucRemainingRoutesNo);
        }
        else if (cmdId == CMDID_C975_DeleteRoute)
        {
            metaAttr->updateTotalAndUsedRoutesFromRemaining(((C975_DeleteRoute_Resp*) cmd)->m_ucRemainingRoutesNo);
        }
        // update the source routes no

        else if (cmdId == CMDID_C976_WriteSourceRoute)
        {
            LOG_ERROR(CMDID_C976_WriteSourceRoute);
            metaAttr->updateTotalAndUsedSourceRoutesFromRemaining(
                                                                  ((C976_WriteSourceRoute_Resp*) cmd)->m_ucRemainingSourceRoutesNo);
        }
        // update graph neighbors(connections)  number

        else if (cmdId == CMDID_C969_WriteGraphNeighbourPair)
        {
            metaAttr->updateTotalAndUsedGraphsFromRemaining(
                                                            ((C969_WriteGraphNeighbourPair_Resp*) cmd)->m_ucRemainingConnectionsNo);
            metaAttr->updateTotalAndUsedGraphNeighborsFromRemaining(
                                                                    ((C969_WriteGraphNeighbourPair_Resp*) cmd)->m_ucRemainingConnectionsNo);
        }
        else if (cmdId == CMDID_C970_DeleteGraphConnection)
        {
            metaAttr->updateTotalAndUsedGraphsFromRemaining(
                                                            ((C970_DeleteGraphConnection_Resp*) cmd)->m_ucRemainingConnectionsNo);
            metaAttr->updateTotalAndUsedGraphNeighborsFromRemaining(
                                                                    ((C970_DeleteGraphConnection_Resp*) cmd)->m_ucRemainingConnectionsNo);

        }
        // update the graph no
        /**
         * usedGraphs not available yet see Doinel for details
         */
        // update superframes number

        else if (cmdId == CMDID_C965_WriteSuperframe)
        {
            metaAttr->updateTotalAndUsedSuperframesFromRemaining(
                                                                 ((C965_WriteSuperframe_Resp*) cmd)->m_ucRemainingSuperframesNo);
        }
        else if (cmdId == CMDID_C966_DeleteSuperframe)
        {
            metaAttr->updateTotalAndUsedSuperframesFromRemaining(
                                                                 ((C966_DeleteSuperframe_Resp*) cmd)->m_ucRemainingSuperframeEntriesNo);
        }
        // update links number

        else if (cmdId == CMDID_C967_WriteLink)
        {
            metaAttr->updateTotalAndUsedLinksFromRemaining(((C967_WriteLink_Resp*) cmd)->m_unRemainingLinksNo);
        }
        else if (cmdId == CMDID_C968_DeleteLink)
        {
            metaAttr->updateTotalAndUsedLinksFromRemaining(((C968_DeleteLink_Resp*) cmd)->m_unRemainingLinksNo);
        }

        if (joinPriorityBefore != metaAttr->getJoinPriority())
        {
            this->devJoinPriorityFlow.markJoinPriorityForDeviceAsChanged(device.address32, metaAttr->getJoinPriority());
        }
    }
    catch (DeviceNotFoundException& ex)
    {
        LOG_ERROR("updateDeviceHistory failed : " << ex.what());
    }

}

WHOperationQueue::WHOperationsList WHOperationQueue::getWHOperations(WHartHandle& handle)
{
    std::map<WHartHandle, WHOperationsList>::iterator it = waitingPackets.find(handle);
    if (it != waitingPackets.end())
    {
        return it->second;
    }

    return WHOperationsList();
}

bool WHOperationQueue::existsOperationInQueue(IEngineOperationPointer operation)
{
    for (WHOperationsList::iterator it = operationsQueue.begin(); it != operationsQueue.end(); ++it)
    {
        if (operation->getOperationId() == (*it)->getEngineOperation()->getOperationId())
        {
            return true;
        }
    }

    return false;
}

bool WHOperationQueue::existsOperationDependency(WHOperationsList& packageCommands, WHOperationPointer operation)
{
    if (commonData.settings.useWaves) return false;

    bool result = false;

    LOG_TRACE("existsOperationDependency() : " << operation->toString());
    IEngineOperationPointer engOp = operation->getEngineOperation();
    std::list<IEngineOperationPointer>& opDependencies = engOp->getOperationsDependencies();
    std::list<IEngineOperationPointer>::iterator itDep = opDependencies.begin();

    for (; itDep != opDependencies.end();)  //++ will be done at the end or when op->getState is CONFIRMED or SI
    {
        if ((*itDep)->getState() == OperationState::CONFIRMED || (*itDep)->getState() == OperationState::SENT_IGNORED /*||
                    !existsOperationInQueue(*itDep)*/)
        {
            // the dependency operation is confirmed or no longer will be sent ...
            // erase the confirmed dependency from the list
            itDep = opDependencies.erase(itDep);
        }
        else
        {
            if (!result)
            {
                // find the dependency in the package command
                WHOperationsList::iterator itPckCmd = packageCommands.begin();
                bool depInPck = false;
                for (; itPckCmd != packageCommands.end(); ++itPckCmd)
                {
                    if ((*itDep)->getOperationId() == (*itPckCmd)->getEngineOperation()->getOperationId())
                    {
                        depInPck = true;
                        break;
                    }
                }

                if (depInPck == false)
                {
                    // if the dependency is not confirmed and is not in package ... -> there is a dependency!
                    result = true;
                }
            }
            ++itDep;
        }
    }

    return result;
}

NE::Model::Operations::EngineOperationsListPointer WHOperationQueue::GenerateOperationsList(uint32_t eventId)
{
    NE::Model::Operations::EngineOperationsListPointer opList(new NE::Model::Operations::EngineOperations());
    OperationsEventsMap::iterator it = operationsMap.find(eventId);
    if (it != operationsMap.end())
    {
        opList.reset(new NE::Model::Operations::EngineOperations(*(it->second)));
        opList->operations.clear();
    }

    return opList;
}


void WHOperationQueue::confirmPackage(WHartHandle& handle, WHartLocalStatus localStatus,
                                      const WHartCommandList& responses)
{
    //[andy] - if it gets called because a package cannot be sent from the stack, throw exception
    // so the send method will get notified.
    if (isSendingCommand)
    {
        LOG_DEBUG("Confirm while sending command. Not sent from stack.");
        throw ConfirmFromStackException();
    }

    bool hasError = false;
    bool isTimeout = false;
    bool hasNoSpace = false;

    if (localStatus.status > hart7::stack::WHartLocalStatus::whartlsSuccess)
    {
        hasError = true;
        if (localStatus.status == hart7::stack::WHartLocalStatus::whartlsError_TL_Timeout)
        {
            isTimeout = true;
        }
    }

    // check all responses to check the status of the commands
    if (!hasError && (responses.count == 0))
    {
        LOG_ERROR("No responses in confirm!");
        return;
    }

    WHOperationsList whOperations = getWHOperations(handle);
    waitingPackets.erase(handle);

    if (whOperations.size() > 0)
    {
        updateDeviceHistoryForConfirmPackage(whOperations.front()->getDestinationAddress());
        logQueue.logConfirmPacket(handle, localStatus, whOperations, responses);
    }

    std::list<uint16_t> successCmdIndexes; // the indexes of the success operation
    std::set<uint32_t> confirmedOperationEvents;
    int i = 0;
    WHartAddress destinationAddress;

    for (WHOperationsList::iterator it = whOperations.begin(); it != whOperations.end(); it++, i++)
    {
        WHartCommand command;

        WHOperationPointer whOperation = *it;

        NE::Model::Operations::EngineOperationsListPointer opList = GenerateOperationsList(whOperation->getOperationEvent());

        if (hasError || hasNoSpace)
        {
            command.commandID = whOperation->getWHartCommand().commandID;
            command.responseCode = 0;
        }
        else
        {
            command = responses.list[i];
            hart7::util::NMLog::logCommandResponse((int) whOperation->getWHartCommand().commandID,
                                                   command.responseCode, command.command, destinationAddress);
        }

        if (command.responseCode == 0 && !isTimeout && !hasError)
        {
            updateDeviceMetadata((*it)->getDestinationAddress(), command.commandID, command.command);
        }

        LOG_INFO("response from " << whOperation->getDestinationAddress() << " to cmdId: "
                    << (int) command.commandID << ", responseCode: " << (int) command.responseCode);

        if (whOperation->getOperationStatus() == OperationState::CONFIRMED)
        {
            LOG_WARN("Operation " << (int) command.commandID << " already confirmed!");
        }
        else if (whOperation->getOperationStatus() == OperationState::SENT)
        {
            //LOG_TRACE("Operation " << (int) whOperation->getEngineOperation()->getOperationId() << " confirmed.");
            whOperation->setResponseCode(command.responseCode);

            // if payload too long
            if (command.responseCode == 60)
            {
                hasNoSpace = true;
            }

            if (hasNoSpace)
            {
                whOperation->setOperationStatus(OperationState::SENT_IGNORED);
                whOperation->getEngineOperation()->setErrorCode(NE::Model::Operations::ErrorCode::ERROR);
            }
            else
            {
                whOperation->setOperationStatus(OperationState::CONFIRMED);
                if (isTimeout)
                {
                    whOperation->getEngineOperation()->setErrorCode(NE::Model::Operations::ErrorCode::TIMEOUT);
                    StopSendingOperationsFromEvent(whOperation, opList);
                }
                else if (hasError || ((command.responseCode != 0)
                            && responseCodes.IsResponseCodeConsideredError(command.commandID, command.responseCode)))
                {
                    whOperation->getEngineOperation()->setErrorCode(NE::Model::Operations::ErrorCode::ERROR);
                    StopSendingOperationsFromEvent(whOperation, opList);
                }
                else
                {
                    whOperation->getEngineOperation()->setErrorCode(NE::Model::Operations::ErrorCode::SUCCESS);
                    if (command.commandID == CMDID_C799_RequestService //
                                || command.commandID == CMDID_C801_DeleteService //
                                || command.commandID == CMDID_C818_WriteChannelBlacklist //
                                || command.commandID == CMDID_C965_WriteSuperframe //
                                || command.commandID == CMDID_C966_DeleteSuperframe //
                                || command.commandID == CMDID_C967_WriteLink //
                                || command.commandID == CMDID_C968_DeleteLink //
                                || command.commandID == CMDID_C969_WriteGraphNeighbourPair //
                                || command.commandID == CMDID_C970_DeleteGraphConnection //
                                || command.commandID == CMDID_C971_WriteNeighbourPropertyFlag //
                                || command.commandID == CMDID_C973_WriteService //
                                || command.commandID == CMDID_C974_WriteRoute //
                                || command.commandID == CMDID_C975_DeleteRoute //
                                || command.commandID == CMDID_C976_WriteSourceRoute //
                                || command.commandID == CMDID_C977_DeleteSourceRoute //
                    )
                    {
                        destinationAddress = whOperation->getDestinationAddress();
                        successCmdIndexes.push_back(i);
                    }

                    if (command.responseCode != 0)
                    {
                        LOG_DEBUG("Response code = " << command.responseCode << " not considered an error.");
                        //                        command.responseCode = 0;
                    }
                }
            }
        }
        else if (whOperation->getOperationStatus() == OperationState::GENERATED)
        {
            LOG_ERROR("We received a response for something that we have not sent!");
            return;
        }

        confirmedOperationEvents.insert((*it)->getOperationEvent());

        opList->addOperation((*it)->getEngineOperation());
        LOG_DEBUG("Attempting to confirm opeation id=" << (*it)->getEngineOperation()->getOperationId());

        CallConfirmOps((*it)->getOperationEvent(), opList);
    }

    forwardCmdsToGw(destinationAddress, responses, successCmdIndexes);

    // check all the events that had at least one operation confirm by the last package
    checkCommands(confirmedOperationEvents);
}


void WHOperationQueue::forwardCmdsToGw(WHartAddress destinationAddress, const WHartCommandList& responses, std::list<
            uint16_t>& successCmdIndexes)
{
    int index = 0;
    std::list<uint16_t>::iterator it = successCmdIndexes.begin();
    for (; it != successCmdIndexes.end(); ++it)//, ++index)
    {
        std::deque<WHartCommandWrapper> responsesToFw;
        WHartCommand cmds[1]; //successCmdIndexes.size()];

        WHartCommandList successCommands;

        WHartCommand *command = responses.list + *it;
        responsesToFw.push_back(WHartCommandWrapper());
        WHartCommandWrapper& cmd = responsesToFw.back();

        C64765_NivisMetaCommand_Resp *resp;
        resp = (C64765_NivisMetaCommand_Resp *) cmd.commandBuffer.get();
        resp->Nickname = GATEWAY_ADDRESS;
        std::memcpy(
                    resp->DeviceUniqueId,
                    hart7::util::getUniqueIdFromAddress64(
                                                          hart7::util::getAddress64(
                                                                                    commonData.networkEngine.getDevicesTable(),
                                                                                    destinationAddress)).bytes, 5);

        uint16_t buffSize = 256;
        uint8_t commandBuff[buffSize];
        uint16_t writtenBytes;
        if (commonData.serializeResponse((uint16_t) command->commandID, commandBuff, buffSize, command->command,
                                         writtenBytes))
        {
            resp->CommandSize = writtenBytes;// + 4;
            memcpy(resp->Command, commandBuff, writtenBytes);
        }
        else
        {
            LOG_ERROR("Can not serialize " << (int) command->commandID);
            continue;
        }

        cmds[index].responseCode = 0;
        cmds[index].commandID = CMDID_C64765_NivisMetaCommand;
        cmds[index].command = resp;
        successCommands.count = 1;
        successCommands.list = cmds;

        NE::Model::Services::Service& service = commonData.utils.GetServiceTo(GATEWAY_ADDRESS);
        requestSend.TransmitRequest(GATEWAY_ADDRESS, stack::whartpCommand,
                                    (stack::WHartServiceID) service.getServiceId(), stack::wharttPublishNotify,
                                    successCommands, commonData.utils.GetSessionType(GATEWAY_ADDRESS), *this);

        hart7::util::NMLog::logCommandResponse(CMDID_C64765_NivisMetaCommand, 0, resp, WHartAddress(GATEWAY_ADDRESS));
    }
}

void WHOperationQueue::StopSendingOperationsFromEvent(WHOperationPointer failedOperation, NE::Model::Operations::EngineOperationsListPointer& opList)
{
    StopOnErrorMap::iterator stopOnErr = stopOnErrorsMap.find(failedOperation->getOperationEvent());
    if (stopOnErr != stopOnErrorsMap.end())
    {
        if (!(stopOnErr->second))
            return;
    }

    for (WHOperationsList::iterator it = operationsQueue.begin(); it != operationsQueue.end(); ++it)
    {
        if ((*it)->getOperationEvent() == failedOperation->getOperationEvent()
                    && (*it)->getEngineOperation()->getState() == OperationState::GENERATED)
        {
            if ((*it)->getWHartCommand().commandID != CMDID_C971_WriteNeighbourPropertyFlag)
            {
                (*it)->getEngineOperation()->setState(OperationState::SENT_IGNORED);
                (*it)->getEngineOperation()->setErrorCode(NE::Model::Operations::ErrorCode::ERROR);
            }
            opList->addOperation((*it)->getEngineOperation());

        }
    }
}

void WHOperationQueue::updateDeviceHistoryForConfirmPackage(WHartAddress& address)
{
    if (address.type == WHartAddress::whartaNickname)
    {
        try
        {
            if (commonData.networkEngine.existsDevice(address.address.nickname))
            {
                NE::Model::Device& device = commonData.networkEngine.getDevice(address.address.nickname);
                device.deviceHistory.confirmLastPackage();
            }
        }
        catch (DeviceNotFoundException& ex)
        {
            // do nothing : it could happen when GW is in join process
        }
    }
}

void WHOperationQueue::checkCommands(std::set<uint32_t> confirmedOperationEvents)
{
    for (std::set<uint32_t>::iterator it = confirmedOperationEvents.begin(); it != confirmedOperationEvents.end(); ++it)
    {
        LOG_TRACE("checkCommands() : Attempting to confirm event ID=" << *it);
    }

    // 1. determine all the unconfirmed events
    std::set<uint32_t> unconfirmedEventsSets;

    uint32_t unconfirmedOperationsNo = 0;
    WHOperationsList::iterator itWHartOperation = operationsQueue.begin();
    for (; itWHartOperation != operationsQueue.end(); ++itWHartOperation)
    {
        if (((*itWHartOperation)->getEngineOperation()->getState() != OperationState::CONFIRMED)
                    && ((*itWHartOperation)->getEngineOperation()->getState() != OperationState::SENT_IGNORED))
        {
            ++unconfirmedOperationsNo;
            LOG_TRACE("checkCommands() : Unconfirmed command: " << (*itWHartOperation)->toString());
            unconfirmedEventsSets.insert((*itWHartOperation)->getOperationEvent());
        }
    }

    std::ostringstream stream;
    stream << "checkCommands() : unconfirmed events no : ";
    stream << unconfirmedEventsSets.size();
    stream << ", unconfirmed operations no : ";
    stream << (int) unconfirmedOperationsNo;
    LOG_INFO(stream.str());

    // 2. check all the events from confirmedOperationEvents if they are in the unconfirmedEventsSets
    std::set<uint32_t>::iterator it;
    for (it = confirmedOperationEvents.begin(); it != confirmedOperationEvents.end(); ++it)
    {
        if (unconfirmedEventsSets.find((*it)) == unconfirmedEventsSets.end())
        {
            LOG_DEBUG("Confirming operation event with ID:" << (*it));

            bool hasError = false;
            for (WHOperationsList::iterator opQ = operationsQueue.begin(); opQ != operationsQueue.end(); opQ++)
            {
                if ((*opQ)->getOperationEvent() == *it)
                {
                    if ((*opQ)->getEngineOperation()->getErrorCode() != NE::Model::Operations::ErrorCode::SUCCESS)
                    {
                        hasError = true;
                    }
                    if ((*opQ)->getResponseCode() != 0
                                && responseCodes.IsResponseCodeConsideredError((*opQ)->getWHartCommand().commandID,
                                                                               (*opQ)->getResponseCode()))
                    {
                        hasError = true;
                    }
                    if ((*opQ)->getEngineOperation()->getState() == OperationState::SENT_IGNORED)
                    {
                        hasError = true;
                    }
                }
            }

            // (*it) represents an event that is confirmed => delete all it's operations
            WHOperationsList::iterator itRemoved = std::remove_if(operationsQueue.begin(), operationsQueue.end(),
                                                                  OperationsEventPredicate((*it)));
            operationsQueue.erase(itRemoved, operationsQueue.end());

            //if on waves, this will be called after the last set is confirmed. if not, this will be called when all the operations are
            //confirmed
            if (!commonData.settings.useWaves)
            {
                logQueue.logConfirmEvent(*it, hasError);
            }

            StopOnErrorMap::iterator stopOnErr = stopOnErrorsMap.find(*it);
            HandlersMap::iterator handlerIt = completeHandlers.find(*it);
            if (handlerIt != completeHandlers.end())
            {
                try
                {
                    OperationsCompletedHandler handler = handlerIt->second;
                    completeHandlers.erase(handlerIt);
                    if (stopOnErr != stopOnErrorsMap.end())
                    {
                        stopOnErrorsMap.erase(stopOnErr);
                    }

                    ConfirmHandlersMap::iterator confIt = confirmHandlers.find(*it);
                    if (confIt != confirmHandlers.end())
                    {
                        confirmHandlers.erase(confIt);
                    }

                    OperationsEventsMap::iterator opIt = operationsMap.find(*it);
                    if (opIt != operationsMap.end())
                    {
                        operationsMap.erase(opIt);
                    }

                    if (handler) {
                        // call handler
                        handler(hasError);
                    }
                }
                catch (std::exception& ex)
                {
                    LOG_ERROR("Exception occurred while invoking complete handler! ex=" << ex.what());
                }

            }
        }
    }
}

void WHOperationQueue::ProcessConfirm(WHartHandle requestHandle, const WHartLocalStatus& localStatus,
                                      const WHartCommandList& list)
{
    if (waitingPacketConfirms.find(requestHandle) != waitingPacketConfirms.end())
    {
        waitingPacketConfirms.erase(requestHandle);
    }

    confirmPackage(requestHandle, localStatus, list);

    // give a chance to other packets to be sent (they might get blocked when MAX_PACKAGE_SIZE is reached).
    sendOperations();
}

void WHOperationQueue::ProcessIndicate(WHartHandle handle, const WHartAddress& src, WHartPriority priority,
                                       WHartTransportType transportType, const WHartCommandList& list)
{
    LOG_WARN("Unexpected indicate received in WHOperationsQueue.");
}

void WHOperationQueue::updateDeviceHistoryForSentPacket(WHartAddress& address, uint packageSize,
                                                        WHOperationsList& packageCommands)
{
    if (address.type == WHartAddress::whartaNickname)
    {
        try
        {
            if (commonData.networkEngine.existsDevice(address.address.nickname))
            {
                NE::Model::Device& device = commonData.networkEngine.getDevice(address.address.nickname);
                device.deviceHistory.sentPackage(packageSize, packageCommands.size());
            }
        }
        catch (DeviceNotFoundException& ex)
        {
            // do nothing : it could happen when GW is in join process
        }
    }
}

void WHOperationQueue::updateDevicesJoinPriority()
{
    devJoinPriorityFlow.updateDevices(*this);
}

void WHOperationQueue::generateWHOperations(EngineOperationsListPointer operationEventPointer, std::vector<
            WHOperationPointer>& whOperations, WHEngineOperationsVisitor& visitor)
{
    OperationsDependency dependency(*operationEventPointer);
    dependency.ProcessDependencies();

    EngineOperationsVector & operations = operationEventPointer->getEngineOperations();

    NetworkEngineEventType::NetworkEngineEventTypeEnum operationEventType =
                                                            operationEventPointer->getNetworkEngineEventType();

    for (EngineOperationsVector::iterator it = operations.begin(); it != operations.end(); ++it)
    {

        std::ostringstream ostr;
        (*it)->toString(ostr);

        if ((*it)->getState() == NE::Model::Operations::OperationState::GENERATED)
        {
            if ((*it)->accept(visitor))
            {
                LOG_DEBUG("generateWHOperations() : generated operation: " << ostr.str());
                std::vector<WHOperationPointer> operations = visitor.getWHOperations();
                visitor.resetWHOperations();

                for (std::vector<WHOperationPointer>::iterator opIt = operations.begin(); opIt != operations.end(); ++opIt)
                {
                    (*opIt)->setEngineOperation(*it);
                    (*opIt)->setOperationEventType(operationEventType);
                    whOperations.push_back(*opIt);
                    LOG_DEBUG("Added command with id=" << whOperations.back()->getWHartCommand().commandID);
                }
            }
            else
            {
                LOG_DEBUG("Skipped operation: " << ostr.str());
                (*it)->setState(NE::Model::Operations::OperationState::CONFIRMED);
            }
        }
    }
}

void WHOperationQueue::logOperations()
{
    WHOperationsList::iterator itWHartOperation = operationsQueue.begin();
    LOG_ERROR("Logging all operations:");
    for (; itWHartOperation != operationsQueue.end(); ++itWHartOperation)
    {
        LOG_ERROR("Queue command: " << (*itWHartOperation)->toString());
    }
}


bool WHOperationQueue::saveTopoloyToDisk;

void WHOperationQueue::HandlerSIGUSR2(int signal)
{
    //WARNING: No logging should be made from Signal Handler. It may produce a deadlock with the NM process that write normal logging(ostring_stream use mutexes).
    pthread_t threadId = pthread_self();
    cout << getpid() << " : " << threadId << " : " << "HandlerSIGUSR2: Received USR2 signal." << endl;

    saveTopoloyToDisk = true;

    cout << getpid() << " : " << threadId << " : " << "HandlerSIGUSR2: handler exited" << endl;
}

}

}

}
