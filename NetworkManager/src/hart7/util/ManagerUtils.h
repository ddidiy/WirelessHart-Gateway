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
 * ManagerUtils.h
 *
 * Utility methods for Network Manager.
 *
 *  Created on: Dec 9, 2008
 *      Author: Radu Pop
 */

#ifndef MANAGERUTILS_H_
#define MANAGERUTILS_H_

#include <ApplicationLayer/Model/UniversalCommands.h>
#include <WHartStack/WHartStack.h>

#include <Common/NEAddress.h>
#include <Model/Capabilities.h>
#include <Model/NetworkEngine.h>

#include "../../NMSettingsLogic.h"
#include "../nmanager/nmodel/HartDevice.h"

#include "Model/Topology/SubnetTopology.h"

using namespace NE::Model;

namespace hart7 {

namespace util {

using hart7::stack::WHartAddress;

LOG_DEF("h7.u.ManagerUtils")
;

inline Uint16 getAddress16(Address32 address32)
{
    return (Uint16) ((address32 << 16) >> 16);
}

inline Address64 getAddress64FromUniqueId(WHartUniqueID uniqueID)
{
    Address64 address64;
    address64.value[0] = 0x00;
    address64.value[1] = 0x1B;
    address64.value[2] = 0x1E;

    for (int i = 0; i < 5; i++)
    {
        address64.value[i + 3] = uniqueID.bytes[i];
    }

    address64.createAddressString();
    return address64;
}

inline Address64 getAddress64FromUniqueId(const _device_address_t uniqueID)
{
    Address64 address64;
    address64.value[0] = 0x00;
    address64.value[1] = 0x1B;
    address64.value[2] = 0x1E;

    for (int i = 0; i < 5; i++)
    {
        address64.value[i + 3] = uniqueID[i];
    }

    address64.createAddressString();
    return address64;
}

inline void writeTopologyToFile(NE::Model::Topology::SubnetTopology& topology, std::string fileName)
{
    //TODO: check if this method can be removed
}

/**
 * Returns the Address64 from a wireless address.
 * @return the Address64 from a wireless address
 */
inline NE::Common::Address64 getAddress64(DevicesTable& deviceTable, const WHartAddress hartAddress)
{
    if (hartAddress.type == WHartAddress::whartaUniqueID)
    {
        return getAddress64FromUniqueId(hartAddress.address.uniqueID);
    }
    else if (hartAddress.type == WHartAddress::whartaNickname)
    {
        return deviceTable.getAddress64(hartAddress.address.nickname);
    }
    else if (hartAddress.type == WHartAddress::whartaProxy)
    {
        return getAddress64FromUniqueId(hartAddress.address.proxy.uniqueID);
    }
    else if (hartAddress.type == WHartAddress::whartaProxyShort)
    {
        return deviceTable.getAddress64(hartAddress.address.proxyShort.destNickname);
    }

    std::ostringstream stream;
    stream << "Invalid WHartAddress type : ";
    stream << hartAddress;
    throw NEException(stream.str());

}

inline NE::Model::Device& getDevice(DevicesTable& deviceTable, WHartAddress& hartAddress)
{
    LOG_DEBUG("getDevice adr : " << hartAddress);
    if (hartAddress.type == WHartAddress::whartaNickname)
    {
        return deviceTable.getDevice(hartAddress.address.nickname);
    }
    else if (hartAddress.type == WHartAddress::whartaUniqueID)
    {
        return deviceTable.getDevice(
                                     deviceTable.createAddress32(getAddress64FromUniqueId(hartAddress.address.uniqueID)));
    }
    else if (hartAddress.type == WHartAddress::whartaProxy)
    {
        return deviceTable.getDevice(
                                     deviceTable.createAddress32(
                                                                 getAddress64FromUniqueId(
                                                                                          hartAddress.address.proxy.uniqueID)));
    }
    else if (hartAddress.type == WHartAddress::whartaProxyShort)
    {
        return deviceTable.getDevice(hartAddress.address.proxyShort.destNickname);
    }

    std::ostringstream str;
    str << hartAddress;
    throw NE::Model::DeviceNotFoundException(str.str());
}

inline bool existsDevice(DevicesTable& deviceTable, WHartAddress& hartAddress)
{
    if (hartAddress.type == WHartAddress::whartaNickname)
    {
        return deviceTable.existsDevice(hartAddress.address.nickname);
    }
    else if (hartAddress.type == WHartAddress::whartaUniqueID)
    {
        return deviceTable.existsDevice(
                                        deviceTable.createAddress32(
                                                                    getAddress64FromUniqueId(
                                                                                             hartAddress.address.uniqueID)));
    }
    else if (hartAddress.type == WHartAddress::whartaProxy)
    {
        return deviceTable.existsDevice(
                                        deviceTable.createAddress32(
                                                                    getAddress64FromUniqueId(
                                                                                             hartAddress.address.proxy.uniqueID)));
    }
    else if (hartAddress.type == WHartAddress::whartaProxyShort)
    {
        return deviceTable.existsDevice(hartAddress.address.proxyShort.destNickname);
    }

    return false;
}

inline WHartUniqueID getUniqueIdFromAddress64(Address64 address64)
{
    WHartUniqueID uniqueID;

    for (int i = 0; i < 5; i++)
    {
        uniqueID.bytes[i] = address64.value[i + 3];
    }

    return uniqueID;
}

inline LinkType getStackLinkType(LinkTypes::LinkTypesEnum linkType)
{
    if (linkType == LinkTypes::BROADCAST)
    {
        return LinkType_Broadcast;
    }
    else if (linkType == LinkTypes::DISCOVERY)
    {
        return LinkType_Discovery;
    }
    else if (linkType == LinkTypes::JOIN)
    {
        return LinkType_Join;
    }
    else if (linkType == LinkTypes::NORMAL)
    {
        return LinkType_Normal;
    }

    throw NEException("Invalid LinkTypesEnum value.");
}

inline ServiceApplicationDomain getStackServiceApplicationDomain(
                                                                 ApplicationDomain::ApplicationDomainEnum applicationDomain)
{
    if (applicationDomain == ApplicationDomain::BLOCK_TRANSFFER)
    {
        return ServiceApplicationDomain_BlockTransfer;
    }
    else if (applicationDomain == ApplicationDomain::EVENT)
    {
        return ServiceApplicationDomain_Event;
    }
    else if (applicationDomain == ApplicationDomain::MAINTENANCE)
    {
        return ServiceApplicationDomain_Maintenance;
    }
    else if (applicationDomain == ApplicationDomain::PUBLISH)
    {
        return ServiceApplicationDomain_Publish;
    }

    throw NEException("Invalid ApplicationDomainEnum value.");
}

inline NE::Model::MetaDataAttributes getMetadataAttributes(const nmanager::HartDevice& device)
{
    NE::Model::MetaDataAttributes attr;
    if (device.isBackbone || device.isGateway)
    {
        attr.setTotalServices(259);
        attr.setTotalRoutes(259);
        attr.setTotalSourceRoutes(259);
        attr.setTotalGraphs(259);
        attr.setTotalGraphNeighbors(259);
        attr.setTotalSuperframes(19);
        attr.setTotalLinks(259);
    }
    else
    {
        attr.setTotalServices(69);
        attr.setTotalRoutes(39);
        attr.setTotalSourceRoutes(39);
        attr.setTotalGraphs(69);
        attr.setTotalGraphNeighbors(69);
        attr.setTotalSuperframes(19);
        attr.setTotalLinks(69);
    }

    return attr;
}

inline JoinProcessStatusMasks getJoinProcessStatus(Device& device, WaveDependency::WaveDependencyEnum wave)
{
    switch (wave)
    {
        case WaveDependency::MIN_DEPENDENCY:
        case WaveDependency::FIRST:
            return JoinProcessStatusMask_JoinRequested;
        case WaveDependency::SECOND:
            return JoinProcessStatusMask_Authenticated;
        case WaveDependency::THIRD:
        case WaveDependency::FOURTH:
        case WaveDependency::FIFTH:
            return JoinProcessStatusMask_NetworkJoined;
        case WaveDependency::SIXTH:
        case WaveDependency::SEVENTH:
        case WaveDependency::EIGTH:
        case WaveDependency::NINETH:
        case WaveDependency::TENTH:
            return JoinProcessStatusMask_JoinFailed;
        case WaveDependency::ELEVENTH:
        case WaveDependency::TWELFTH:
        case WaveDependency::MAX_DEPENDENCY:
            return JoinProcessStatusMask_NormalOperationCommencing;
    }

    return JoinProcessStatusMask_AdvertisementHeard;
}

inline JoinProcessStatusMasks getJoinProcessStatus(Device& device)
{
    switch (device.action)
    {
        case DeviceAction::NOT_SET:
        case DeviceAction::JOIN_REQUEST:
        case DeviceAction::JOIN_REQUEST_SUCCESS:
            return JoinProcessStatusMask_JoinRequested;
        case DeviceAction::ROUTER:
        case DeviceAction::ROUTER_SUCCESS:
        case DeviceAction::ROUTER_FAIL:
            return JoinProcessStatusMask_Authenticated;
        case DeviceAction::SERVICE_IN:
        case DeviceAction::SERVICE_IN_SUCCESS:
        case DeviceAction::SERVICE_OUT:
        case DeviceAction::SERVICE_OUT_SUCCESS:
            return JoinProcessStatusMask_NormalOperationCommencing;
        case DeviceAction::ATTACH_IN:
        case DeviceAction::ATTACH_IN_SUCCESS:
        case DeviceAction::ATTACH_OUT:
        case DeviceAction::ATTACH_OUT_SUCCESS:
            return JoinProcessStatusMask_NetworkJoined;
        case DeviceAction::DISCOVERY_IN:
        case DeviceAction::DISCOVERY_IN_SUCCESS:
        case DeviceAction::DISCOVERY_OUT:
        case DeviceAction::DISCOVERY_OUT_SUCCESS:
            return JoinProcessStatusMask_NormalOperationCommencing;
        case DeviceAction::AFFECTED_IN:
        case DeviceAction::AFFECTED_IN_SUCCESS:
        case DeviceAction::AFFECTED_OUT:
        case DeviceAction::AFFECTED_OUT_SUCCESS:
        case DeviceAction::CHECK_PRESENCE:
        case DeviceAction::CHECK_PRESENCE_SUCCESS:
            return JoinProcessStatusMask_NormalOperationCommencing;
        case DeviceAction::SERVICE_END:
        case DeviceAction::SERVICE_END_SUCCESS:
            return JoinProcessStatusMask_NormalOperationCommencing;
        case DeviceAction::JOIN_REQUEST_FAIL:
        case DeviceAction::SERVICE_IN_FAIL:
        case DeviceAction::SERVICE_OUT_FAIL:
        case DeviceAction::ATTACH_IN_FAIL:
        case DeviceAction::ATTACH_OUT_FAIL:
        case DeviceAction::DISCOVERY_IN_FAIL:
        case DeviceAction::DISCOVERY_OUT_FAIL:
        case DeviceAction::AFFECTED_IN_FAIL:
        case DeviceAction::AFFECTED_OUT_FAIL:
        case DeviceAction::SERVICE_END_FAIL:
        case DeviceAction::REMOVE:
        case DeviceAction::REMOVE_SUCCESS:
        case DeviceAction::REMOVE_FAIL:
        case DeviceAction::CHECK_PRESENCE_FAIL:
            return JoinProcessStatusMask_JoinFailed;
    }

    return JoinProcessStatusMask_AdvertisementHeard;
}

class ManagerUtils
{

    public:

        /**
         * From a list of size <code>listSize</code>, form the index <code>requestedIndex</code> a number of
         * <code>requestedCount</code> elements is requested.
         * Determine the <code>returnedIndex</code> and <code>returnedCount</code> so that they will be inside the limits
         * of the list. If the <code>requestedIndex</code> is outside the boundaries of the list set the requested*
         * so that the last element from the list is returned.
         */
        static void determineElementsToReturn(uint16_t listSize, uint8_t requestedIndex, uint8_t requestedCount,
                                              uint8_t& returnedIndex, uint8_t& returnedCount);

        static void determineElementsToReturn(uint16_t listSize, uint16_t requestedIndex, uint8_t requestedCount,
                                              uint16_t& returnedIndex, uint8_t& returnedCount);
};

/**
 * Returns an instance of NetworkEngine's Capabilities created from the
 * C000_ReadUniqueIdentifier_Resp, C020_ReadLongTag_Resp and .
 */
inline Capabilities getCapabilities(const WHartAddress& hartAddress, //
                                    C000_ReadUniqueIdentifier_Resp* readUnique, //
                                    C020_ReadLongTag_Resp* longTag, //
                                    C787_ReportNeighborSignalLevels_Resp* neighbors, //
                                    uint16_t networkID, NMSettingsLogic& settingsLogic, DevicesTable& deviceTable)
{
    Capabilities capabilities;

    capabilities.euidAddress = getAddress64(deviceTable, hartAddress);
    capabilities.dllSubnetId = networkID;

    capabilities.readUniqueIdentifier = *readUnique;
    capabilities.neighborSignalLevels = *neighbors;
    memcpy(capabilities.longTag, longTag->longTag, 32);

    bool isBackbone = false;
    for (std::vector<Address64>::iterator it = settingsLogic.backbones.begin(); it != settingsLogic.backbones.end(); ++it)
    {
        if (capabilities.euidAddress == *it)
        {
            isBackbone = true;
            break;
        }
    }
    if (isBackbone)
    {
        capabilities.deviceType = DeviceType::BACKBONE;
    }
    else if ((hartAddress == hart7::stack::Gateway_Nickname()) || (hartAddress == hart7::stack::Gateway_UniqueID()))
    {
        capabilities.deviceType = DeviceType::GATEWAY;
    }
    else
    {
        capabilities.deviceType = DeviceType::ROUTER;
    }
    return capabilities;
}

}

}

#endif /* MANAGERUTILS_H_ */
