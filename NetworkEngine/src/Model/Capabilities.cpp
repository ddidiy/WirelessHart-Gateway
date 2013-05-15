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
 * Capabilities.cpp
 *
 *  Created on: May 22, 2008
 *      Author: catalin.pop
 */
#include "Capabilities.h"
#include <iomanip>

namespace NE {
namespace Model {

using namespace NE::Common;
using namespace NE::Misc::Marshall;

Capabilities::Capabilities() :
    euidAddress(), dllSubnetId(0), deviceType(DeviceType::NOT_SET), tagName(), softwareMajorVersion(0),
                softwareMinorVersion(0), queueCapacity(0xFFFF), clockAcurracy(0), channelMap(0xFFFF), advertiseRate(
                            0x7fff), receiveCapacity(0), forwardRate(0), ackTurnaround(0), neighborDiagnosticsCapacity(
                            0), radioOutputPower(0), zeroED(0), maxED(0), options(0) {

    // initialize the reports
    // C779_ReportDeviceHealth_Resp
    reportDeviceHealth.m_unNoOfPacketsGeneratedByDevice = 0;
    reportDeviceHealth.m_unNoOfPacketsTerminatedByDevice = 0;
    reportDeviceHealth.m_ucNoOfDataLinkLayerMICFailuresDetected = 0;
    reportDeviceHealth.m_ucNoOfNetworkLayerMICFailuresDetected = 0;
    reportDeviceHealth.m_ucPowerStatus = DevicePowerStatus_Nominal; // DevicePowerStatus - CommonTable58
    reportDeviceHealth.m_ucNoOfCRCErrorsDetected = 0;
    reportDeviceHealth.m_ucNoOfNonceCounterValuesNotReceived = 0;

    // C780_ReportNeighborHealthList_Resp
    reportNeighborHealthList.m_ucNeighborTableIndex = 0;
    reportNeighborHealthList.m_ucNoOfNeighborEntriesRead = 0;
    reportNeighborHealthList.m_ucTotalNoOfNeighbors = 0;

    //C787_ReportNeighborSignalLevels_Resp
    reportNeighborSignalLevels.m_ucNeighborTableIndex = 0;
    reportNeighborSignalLevels.m_ucNoOfNeighborEntriesRead = 0;
    reportNeighborSignalLevels.m_ucTotalNoOfNeighbors = 0;
}

bool Capabilities::isBackbone() const {
    return deviceType & NE::Model::DeviceType::BACKBONE;
}

bool Capabilities::isRouting() const {
    return deviceType & NE::Model::DeviceType::ROUTER;
}

bool Capabilities::isFieldDevice() const {
    return deviceType & NE::Model::DeviceType::IN_OUT;
}

bool Capabilities::isDevice() const {
    return isFieldDevice() || isRouting();
}

bool Capabilities::isGateway() const {
    return deviceType & NE::Model::DeviceType::GATEWAY;
}

bool Capabilities::isManager() const {
    return deviceType & NE::Model::DeviceType::MANAGER;
}

bool Capabilities::isSecurityManager() const {
    return deviceType & NE::Model::DeviceType::SECURITY_MANAGER;
}

bool Capabilities::isTimeSource() const {
    return deviceType & NE::Model::DeviceType::TIME_SOURCE;
}

bool Capabilities::isProvisioning() const {
    return deviceType & NE::Model::DeviceType::PROVISIONING;
}

std::string Capabilities::getRoleAsString() {
    if (isManager()) {
        return "Manager";
    }
    if (isSecurityManager()) {
        return "SecMan";
    } else if (isBackbone()) {
        return "BR";
    } else if (isGateway()) {
        return "GW";
    } else if (isRouting()) {
        return "Rout";
    } else if (isFieldDevice()) {
        return "FD";
    } else if (isFieldDevice()) {
        return "FD";
    } else if (isDevice()) {
        return "Dev";
    } else if (isTimeSource()) {
        return "TimSrc";
    } else if (isProvisioning()) {
        return "Prov";
    }

    return "Unknown role";
}

std::string Capabilities::getDeviceTypeDescription() {
    if (isBackbone()) {
        return "BACKBONE";
    } else if (isManager()) {
        return "MANAGER";
    } else if (isGateway()) {
        return "GATEWAY";
    } else if (isRouting()) {
        if (isFieldDevice()) {
            return "ROUTER-IO";
        }
        return "ROUTER";
    } else if (isFieldDevice()) {
        return "IN_OUT";
    } else if (isProvisioning()) {
        return "PROVISIONING";
    } else if (isSecurityManager()) {
        return "SECURITY_MANAGER";
    } else if (isTimeSource()) {
        return "TIME_SOURCE";
    }

    return "N/A";
}

void Capabilities::toString(std::ostringstream& stream) {
    stream << "Capabilities {";
    stream << "euidAddr=" << euidAddress.toString();
    stream << ", dllSubId=" << std::hex << (int) dllSubnetId << std::dec;
    stream << ", devType=" << std::setw(17) << getDeviceTypeDescription();
    stream << "}";
}

void Capabilities::toTableIndentString(std::ostringstream& stream) {
    stream << std::setw(20) << euidAddress.toString();
}
}
}
