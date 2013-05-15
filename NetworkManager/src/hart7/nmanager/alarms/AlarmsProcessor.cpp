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
 * AlarmsProcessor.cpp
 *
 *  Created on: Jun 14, 2010
 *      Author: radu pop, andrei petrut
 */

#include "AlarmsProcessor.h"
#include "NodeAlarmVerifier.h"
#include "SMState/SMStateLog.h"

namespace hart7 {

namespace nmanager {

AlarmsProcessor::AlarmsProcessor(CommonData& commonData_, operations::WHOperationQueue& operationsQueue_) :
    commonData(commonData_), operationsQueue(operationsQueue_)
{
}

AlarmsProcessor::~AlarmsProcessor()
{
}

void AlarmsProcessor::newAlarm788(uint16_t src, C788_AlarmPathDown_Resp& alarmPathDown)
{
    nivisMetaOperations.reset(new EngineOperations());
    nivisMetaWhOperations.clear();

    nivisMetaOperations->reasonOfOperations = "Forward alarm 788 to GW";
    Address64 address64 = commonData.networkEngine.getAddress64(src);

    uint16_t buffSize = 256;
    uint8_t command[buffSize];
    uint16_t writtenBytes;
    if (commonData.serializeResponse<C788_AlarmPathDown_Resp> ((uint16_t) CMDID_C788_AlarmPathDown, command, buffSize,
                                                               alarmPathDown, writtenBytes))
    {

        IEngineOperationPointer
                    operation(new NivisCustom64765Operation(GATEWAY_ADDRESS, (uint16_t) CMDID_C788_AlarmPathDown,
                                                            (uint16_t) GATEWAY_ADDRESS,
                                                            hart7::util::getUniqueIdFromAddress64(address64).bytes, //
                                                            2 + 4, command));
        operation->setDependency(WaveDependency::FIRST);
        nivisMetaOperations->addOperation(operation);

        sendOperations();
    }
    else
    {
        LOG_ERROR("Can not serialize CMDID_C788_AlarmPathDown");
    }
}

void AlarmsProcessor::newAlarm789(uint16_t src, C789_AlarmSourceRouteFailed_Resp & alarmSourceRouteFailed)
{
    nivisMetaOperations.reset(new EngineOperations());
    nivisMetaWhOperations.clear();
    nivisMetaOperations->reasonOfOperations = "Forward alarm 789 to GW";
    Address64 address64 = commonData.networkEngine.getAddress64(src);
    uint16_t buffSize = 256;
    uint8_t command[buffSize];
    uint16_t writtenBytes;
    if (commonData.serializeResponse<C789_AlarmSourceRouteFailed_Resp> ((uint16_t) CMDID_C789_AlarmSourceRouteFailed,
                                                                        command, buffSize, alarmSourceRouteFailed,
                                                                        writtenBytes))
    {
        IEngineOperationPointer
                    operation(new NivisCustom64765Operation(GATEWAY_ADDRESS,
                                                            (uint16_t) CMDID_C789_AlarmSourceRouteFailed,
                                                            (uint16_t) GATEWAY_ADDRESS,
                                                            hart7::util::getUniqueIdFromAddress64(address64).bytes, //
                                                            6 + 4, command));
        operation->setDependency(WaveDependency::FIRST);
        nivisMetaOperations->addOperation(operation);

        sendOperations();
    }
    else
    {
        LOG_ERROR("Can not serialize CMDID_C789_AlarmSourceRouteFailed");
    }
}

void AlarmsProcessor::newAlarm790(uint16_t src, C790_AlarmGraphRouteFailed_Resp & alarmGraphRouteFailed)
{
    nivisMetaOperations.reset(new EngineOperations());
    nivisMetaWhOperations.clear();
    nivisMetaOperations->reasonOfOperations = "Forward alarm 790 to GW";
    Address64 address64 = commonData.networkEngine.getAddress64(src);
    uint16_t buffSize = 256;
    uint8_t command[buffSize];
    uint16_t writtenBytes;
    if (commonData.serializeResponse<C790_AlarmGraphRouteFailed_Resp> ((uint16_t) CMDID_C790_AlarmGraphRouteFailed,
                                                                       command, buffSize, alarmGraphRouteFailed,
                                                                       writtenBytes))
    {
        IEngineOperationPointer
                    operation(new NivisCustom64765Operation(GATEWAY_ADDRESS,
                                                            (uint16_t) CMDID_C790_AlarmGraphRouteFailed,
                                                            GATEWAY_ADDRESS,
                                                            hart7::util::getUniqueIdFromAddress64(address64).bytes, //
                                                            2 + 4, command));
        operation->setDependency(WaveDependency::FIRST);
        nivisMetaOperations->addOperation(operation);

        sendOperations();
    }
    else
    {
        LOG_ERROR("Can not serialize CMDID_C790_AlarmGraphRouteFailed");
    }
}

void AlarmsProcessor::newAlarm791(uint16_t src, C791_AlarmTransportLayerFailed_Resp & alarmTransportLayerFailed)
{
    nivisMetaOperations.reset(new EngineOperations());
    nivisMetaWhOperations.clear();

    nivisMetaOperations->reasonOfOperations = "Forward alarm 791 to GW";
    Address64 address64 = commonData.networkEngine.getAddress64(src);

    uint16_t buffSize = 256;
    uint8_t command[buffSize];
    uint16_t writtenBytes;
    if (commonData.serializeResponse<C791_AlarmTransportLayerFailed_Resp> (
                                                                           (uint16_t) CMDID_C791_AlarmTransportLayerFailed,
                                                                           command, buffSize,
                                                                           alarmTransportLayerFailed, writtenBytes))
    {
        IEngineOperationPointer
                    operation(new NivisCustom64765Operation(GATEWAY_ADDRESS,
                                                            (uint16_t) CMDID_C791_AlarmTransportLayerFailed,
                                                            GATEWAY_ADDRESS,
                                                            hart7::util::getUniqueIdFromAddress64(address64).bytes, 2
                                                                        + 4, command));
        operation->setDependency(WaveDependency::FIRST);
        nivisMetaOperations->addOperation(operation);

        sendOperations();
    }
    else
    {
        LOG_ERROR("Can not serialize CMDID_C791_AlarmTransportLayerFailed");
    }
}

void AlarmsProcessor::sendOperations()
{
    operations::WHEngineOperationsVisitor visitor(nivisMetaOperations, commonData);
    operationsQueue.generateWHOperations(nivisMetaOperations, nivisMetaWhOperations, visitor);

    SMState::SMStateLog::logOperations(nivisMetaOperations->reasonOfOperations, *nivisMetaOperations);

    uint32_t opEvent;
    if (operationsQueue.addOperations(nivisMetaOperations, nivisMetaWhOperations, opEvent, nivisMetaOperations->reasonOfOperations, NULL, NULL))
    {
        operationsQueue.sendOperations();
    }
}

std::string AlarmsProcessor::toStringAlarmListener()
{
    return "AlarmsProcessor";
}

}

}
