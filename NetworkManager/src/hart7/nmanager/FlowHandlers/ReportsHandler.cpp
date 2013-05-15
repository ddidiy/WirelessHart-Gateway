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
 * ReportsHandler.cpp
 *
 *  Created on: Oct 2, 2009
 *      Author: Andy
 */
#include "ReportsHandler.h"

using namespace hart7::stack;

void ReportsHandler::Handle(const WHartAddress& source, const C779_ReportDeviceHealth_Resp& deviceHealth)
{
    LOG_INFO("DeviceHealth: device=" << source << " packetsGenerated="
                << (int) deviceHealth.m_unNoOfPacketsGeneratedByDevice << " packetsTerminated="
                << (int) deviceHealth.m_unNoOfPacketsTerminatedByDevice << " dllMICFailures="
                << (int) deviceHealth.m_ucNoOfDataLinkLayerMICFailuresDetected << " netMICFailures="
                << (int) deviceHealth.m_ucNoOfNetworkLayerMICFailuresDetected << " powerStatus="
                << (int) deviceHealth.m_ucPowerStatus << " CRCErrors="
                << (int) deviceHealth.m_ucNoOfCRCErrorsDetected << " noncesNotReceived="
                << (int) deviceHealth.m_ucNoOfNonceCounterValuesNotReceived);

    commonData.reportDispatcher.dispatchReport779(source, deviceHealth);
}

void ReportsHandler::Handle(const WHartAddress& source, const C780_ReportNeighborHealthList_Resp& neighborHealth)
{
    for (int i = 0; i < neighborHealth.m_ucNoOfNeighborEntriesRead; i++)
    {
        try
        {
            commonData.networkEngine.addDiagnostics(
                                                    source.address.nickname,
                                                    neighborHealth.m_aNeighbors[i].nicknameOfNeighbor,
                                                    128 + neighborHealth.m_aNeighbors[i].meanRSLSinceLastReport,
                                                    neighborHealth.m_aNeighbors[i].noOfPacketsTransmittedToThisNeighbor,
                                                    neighborHealth.m_aNeighbors[i].noOfPacketsReceivedFromThisNeighbor,
                                                    neighborHealth.m_aNeighbors[i].noOfFailedTransmits);
        }
        catch (std::exception& ex)
        {
            LOG_INFO("Unable to add diagnostics for node: reason=" << ex.what());
        }
    }

    commonData.reportDispatcher.dispatchReport780(source, neighborHealth);
}

void ReportsHandler::Handle(const WHartAddress& source,
                            const C787_ReportNeighborSignalLevels_Resp& neighborSignalLevels)
{
    for (int i = 0; i < neighborSignalLevels.m_ucNoOfNeighborEntriesRead; i++)
    {
        try
        {
            // transforms rsl from (-128, 127) to (0, 255) values
            uint8_t rsl = 128 + neighborSignalLevels.m_aNeighbors[i].RSLindB;

            if (commonData.settings.checkEdgeVisibility == true)
            {

                if (!commonData.networkEngine.existsDevice(source.address.nickname)
                            || !commonData.networkEngine.existsDevice(neighborSignalLevels.m_aNeighbors[i].nickname))
                {
                    continue;
                }

                Device& srcDevice = commonData.networkEngine.getDevice(source.address.nickname);
                Device& dstDevice = commonData.networkEngine.getDevice(neighborSignalLevels.m_aNeighbors[i].nickname);
                if (srcDevice.status != NE::Model::DeviceStatus::OPERATIONAL //
                            || dstDevice.status != NE::Model::DeviceStatus::OPERATIONAL)
                {
                    continue;
                }

                // add this visible edge to the observation queue
                commonData.nodeVisibleVerifier.addVisibleNode(source.address.nickname,
                                                              neighborSignalLevels.m_aNeighbors[i].nickname, rsl);
            }
            else
            {
                // check if the reverse has already been received; if it has then mark the candidate in NewtorkEngine
                // and delete it from visible edges; Otherwise add the edge into NetworkEngine

                using namespace hart7::nmanager;

                VisibleEdge visiEdge(source.address.nickname, neighborSignalLevels.m_aNeighbors[i].nickname);
                VisibleEdge reEdge(neighborSignalLevels.m_aNeighbors[i].nickname, source.address.nickname);

                if (visibleEdges.find(reEdge) != visibleEdges.end())
                {
                    commonData.networkEngine.addVisible(source.address.nickname,
                                                        neighborSignalLevels.m_aNeighbors[i].nickname, rsl);
                    visibleEdges.erase(reEdge);
                }
                else
                {
                    // add it and wait for the reverse to come
                    visibleEdges.insert(visiEdge);
                }
            }
        }
        catch (std::exception& ex)
        {
            LOG_INFO("Unable to add visible node: reason=" << ex.what());
        }
    }

    commonData.reportDispatcher.dispatchReport787(source, neighborSignalLevels);
}
