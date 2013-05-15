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
 * ReportsProcessor.cpp
 *
 *  Created on: Jun 14, 2010
 *      Author: radu
 */

#include "ReportsProcessor.h"
#include "SMState/SMStateLog.h"

namespace hart7 {

namespace nmanager {

ReportsProcessor::ReportsProcessor(CommonData& commonData_, operations::WHOperationQueue& operationsQueue_) :
    commonData(commonData_), operationsQueue(operationsQueue_)
{

}

ReportsProcessor::~ReportsProcessor()
{
}

void ReportsProcessor::newReport779(const WHartAddress& source, const C779_ReportDeviceHealth_Resp& reportDeviceHealth)
{
    try
    {
        if (source.type != source.whartaNickname)
        {
            return;
        }

        nivisMetaOperations.reset(new EngineOperations());
        nivisMetaWhOperations.clear();

        nivisMetaOperations->reasonOfOperations = "Forward report 779 to GW";
        Address64 address64 = commonData.networkEngine.getAddress64(source.address.nickname);

        uint16_t buffSize = 256;
        uint8_t command[buffSize];
        uint16_t writtenBytes;
        if (commonData.serializeResponse<C779_ReportDeviceHealth_Resp> ((uint16_t) CMDID_C779_ReportDeviceHealth,
                                                                        command, buffSize, reportDeviceHealth,
                                                                        writtenBytes))
        {
            IEngineOperationPointer
                        operation(new NivisCustom64765Operation(GATEWAY_ADDRESS,
                                                                (uint16_t) CMDID_C779_ReportDeviceHealth,
                                                                (uint16_t) GATEWAY_ADDRESS,
                                                                hart7::util::getUniqueIdFromAddress64(address64).bytes, //
                                                                9 + 4, command));
            operation->setDependency(WaveDependency::FIRST);
            nivisMetaOperations->addOperation(operation);

            sendOperations();
        }
        else
        {
            LOG_ERROR("Can not serialize CMDID_C779_ReportDeviceHealth");
        }
    }
    catch (...)
    {
        LOG_ERROR("newReport779() device not found " << source.address.nickname);
    }
}

void ReportsProcessor::newReport780(const WHartAddress& source,
                                    const C780_ReportNeighborHealthList_Resp& neighborHealthList)
{
    try
    {
        if (source.type != source.whartaNickname)
        {
            return;
        }
        nivisMetaOperations.reset(new EngineOperations());
        nivisMetaWhOperations.clear();

        nivisMetaOperations->reasonOfOperations = "Forward report 780 to GW";
        Address64 address64 = commonData.networkEngine.getAddress64(source.address.nickname);

        uint16_t buffSize = 256;
        uint8_t command[buffSize];
        uint16_t writtenBytes;
        if (commonData.serializeResponse<C780_ReportNeighborHealthList_Resp> (
                                                                              (uint16_t) CMDID_C780_ReportNeighborHealthList,
                                                                              command, buffSize, neighborHealthList,
                                                                              writtenBytes))
        {

            IEngineOperationPointer
                        operation(
                                  new NivisCustom64765Operation(
                                                                GATEWAY_ADDRESS,//
                                                                (uint16_t) CMDID_C780_ReportNeighborHealthList,//
                                                                GATEWAY_ADDRESS,//
                                                                hart7::util::getUniqueIdFromAddress64(address64).bytes, //
                                                                ((3
                                                                            + 10
                                                                                        * neighborHealthList.m_ucNoOfNeighborEntriesRead)
                                                                            + 4), //
                                                                command));
            operation->setDependency(WaveDependency::FIRST);
            nivisMetaOperations->addOperation(operation);

            sendOperations();
        }
    }
    catch (...)
    {
        LOG_ERROR("newReport780() device not found " << source.address.nickname);
    }
}

void ReportsProcessor::newReport787(const WHartAddress& source,
                                    const C787_ReportNeighborSignalLevels_Resp& neighborSignalLevels)
{
    try
    {
        if (source.type != source.whartaNickname)
        {
            return;
        }

        nivisMetaOperations.reset(new EngineOperations());
        nivisMetaWhOperations.clear();

        nivisMetaOperations->reasonOfOperations = "Forward report 787 to GW";
        Address64 address64 = commonData.networkEngine.getAddress64(source.address.nickname);

        uint16_t buffSize = 256;
        uint8_t command[buffSize];
        uint16_t writtenBytes;
        if (commonData.serializeResponse<C787_ReportNeighborSignalLevels_Resp> (
                                                                                (uint16_t) CMDID_C787_ReportNeighborSignalLevels,
                                                                                command, buffSize,
                                                                                neighborSignalLevels, writtenBytes))
        {

            IEngineOperationPointer
                        operation(
                                  new NivisCustom64765Operation(
                                                                GATEWAY_ADDRESS,
                                                                (uint16_t) CMDID_C787_ReportNeighborSignalLevels,
                                                                GATEWAY_ADDRESS,
                                                                hart7::util::getUniqueIdFromAddress64(address64).bytes, //
                                                                (3
                                                                            + 3
                                                                                        * neighborSignalLevels.m_ucNoOfNeighborEntriesRead)
                                                                            + 4, command));
            operation->setDependency(WaveDependency::FIRST);
            nivisMetaOperations->addOperation(operation);

            sendOperations();
        }
    }
    catch (...)
    {
        LOG_ERROR("newReport787() device not found " << source.address.nickname);
    }
}

void ReportsProcessor::sendOperations()
{
    operations::WHEngineOperationsVisitor visitor(nivisMetaOperations, commonData);
    operationsQueue.generateWHOperations(nivisMetaOperations, nivisMetaWhOperations, visitor);

    uint32_t opEvent;
    if (operationsQueue.addOperations(nivisMetaOperations, nivisMetaWhOperations, opEvent, nivisMetaOperations->reasonOfOperations, NULL, NULL))
    {
        operationsQueue.sendOperations();
    }
}

std::string ReportsProcessor::toStringReportListener()
{
    return "ReportsProcessor";
}

}

}
