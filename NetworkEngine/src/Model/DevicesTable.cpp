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

#include "DevicesTable.h"
#include <fstream>

using namespace NE::Common;
using namespace NE::Model;

#include "Model/NetworkEngine.h"
#include <set>

DevicesTable::DevicesTable() {
    generateNicknamesFromEUI64 = false;
    gatewayAddress64.loadString("00-1B-1E-F9-81-00-00-02");
}

DevicesTable::~DevicesTable() {
}

void DevicesTable::LoadDeviceNicknames (SettingsLogic& settingsLogic)
{
    generateNicknamesFromEUI64 = settingsLogic.generateNicknamesFromEUI64;
    nicknamesFileName = settingsLogic.nicknamesFileName;

    if (!generateNicknamesFromEUI64 && nicknamesFileName.size () > 0) {

        // read nicknames from disk
        std::ifstream nicknamesFile;
        nicknamesFile.open(nicknamesFileName.c_str(), std::ios::in);

        if (nicknamesFile.is_open()) {

            std::string line;
            Address64 address64;

            while (nicknamesFile.good()) {
                getline(nicknamesFile, line);
                if (line.size() > 24) {
                    size_t pos = line.find('\t');
                    if (pos != std::string::npos) {
                        try {
                            address64.loadString(line.substr(0, pos));
                            Address32 address32 = atoi(line.substr(pos + 1).c_str());
                            if (address32 != 0)
                                deviceNicknameMap[address64.address64String] = address32;
                        } catch (...) {
                            // do nothing
                        }
                    }
                }
            }

            nicknamesFile.close();

            // make sure the nicknames are properly allocated. In the eventuality that somebody messes up with the file on disk ..
            Address32 maxNickname = 0;
            std::set<Address32> tempNicknames;

            for (std::map<std::string, Address32>::iterator it = deviceNicknameMap.begin (); it != deviceNicknameMap.end (); ++it) {

                    if (it->second > maxNickname)
                        maxNickname = it->second;

                    if (tempNicknames.find (it->second) != tempNicknames.end ()) {

                        deviceNicknameMap.clear ();
                        break;

                    } else {
                        tempNicknames.insert(it->second);
                    }
            }

            if (maxNickname != deviceNicknameMap.size ())
                deviceNicknameMap.clear ();


        } else {
            LOG_ERROR("Could not find table of devices and nicknames on disk. New nicknames will be generated.");
        }
    }
}

std::map<Address32, Uint32> DevicesTable::getNrOfJoinsPerDevice() {
    return nrOfJoinsPerDevice;
}

Address32 DevicesTable::createAddress32(Address64 address64) {
    if (address64 == gatewayAddress64) {
        return GATEWAY_ADDRESS;
    } else {

        if (generateNicknamesFromEUI64) {
            return ((address64.value[6]) << 8) + address64.value[7];
        } else {

            std::map<std::string, Address32>::iterator it = deviceNicknameMap.find(address64.address64String);
            if (it == deviceNicknameMap.end()) {

                Address32 nickname = deviceNicknameMap.size() + 1;
                if (nickname == 0xF980) deviceNicknameMap["manager"] = nickname++;
                if (nickname == 0xF981) deviceNicknameMap["gateway"] = nickname++;
                deviceNicknameMap[address64.address64String] = nickname;

                // save nicknames on disk
                std::ofstream nicknamesFile;
                nicknamesFile.open(nicknamesFileName.c_str(), std::ios::trunc);

                if (nicknamesFile.is_open()) {
                    std::map<std::string, Address32>::iterator it;
                    for (it = deviceNicknameMap.begin(); it != deviceNicknameMap.end(); ++it)
                        nicknamesFile << it->first << "\t" << (unsigned int) it->second << std::endl;
                    nicknamesFile.close();
                } else {
                    LOG_ERROR("Could not save device nickname list on disk.");
                }

                return nickname;

            } else {
                return it->second;
            }
        }

    }
}

bool DevicesTable::existsDevice(Address32 address32) {
    Devices32Container::iterator it = devices.find(address32);
    return (it != devices.end() && !it->second.isDeletedStatus());
}

bool DevicesTable::existsDevice(const Address64& address64) {
    AddressMapping64_32::iterator it = addressMapping.find(address64);

    if (it != addressMapping.end()) {
        Address32 address = it->second;

        Device& device = getDevice(address);

        if (device.isDeletedStatus()) {
            LOG_DEBUG("existsDevice - address64=" << address64.toString() << " => false - status deleted");
            return false;
        }

        LOG_DEBUG("existsDevice - address64=" << address64.toString() << " => true, status=" << DeviceStatus::toString(
                    device.status));
        return true;
    } else {
        LOG_DEBUG("existsDevice - address64=" << address64.toString() << " => false");
        return false;
    }
}

bool DevicesTable::existsDeletedDevice(const Address64& address64) {
    return deletedDevices.find(address64) != deletedDevices.end();
}

bool DevicesTable::existsConfirmedDevice(Address32 address32) {
    return getDevice(address32).isJoinConfirmed();
}

bool DevicesTable::notExistsConfirmedDeviceOrOnEvaluation(Address32 address)
{
    Devices32Container::iterator it = devices.find(address);
    if (it != devices.end())
    {
        return (!it->second.isJoinConfirmed() || it->second.isOnEvaluation());
    }
    return false;
}


Address64 DevicesTable::getAddress64(Address32 address32) {
    AddressMapping32_64::iterator foundMapping = addressMapping32to64.find(address32);
    if (foundMapping != addressMapping32to64.end()) {
        return foundMapping->second;
    } else {
        std::ostringstream stream;
        stream << "Device " << std::hex << address32 << " not found!";
        LOG_ERROR(stream.str());
        throw DeviceNotFoundException(stream.str());
    }
}

Address32 DevicesTable::getAddress32(const Address64& address64) {
    AddressMapping64_32::iterator foundMapping = addressMapping.find(address64);
    if (foundMapping != addressMapping.end()) {
        return foundMapping->second;
    } else {
        std::ostringstream stream;
        stream << "Device " << address64.toString() << " not found!";
        LOG_ERROR(stream.str());
        throw DeviceNotFoundException(address64.toString());
    }
}

Address32 DevicesTable::getDeletedAddress32(const Address64& address64) {
    Devices64Container::iterator foundDevice = deletedDevices.find(address64);
    if (foundDevice != deletedDevices.end()) {
        return foundDevice->second.address32;
    } else {
        std::ostringstream stream;
        stream << "Deleted device " << address64.toString() << " not found!";
        LOG_ERROR(stream.str());
        throw DeviceNotFoundException(address64.toString());
    }
}

void DevicesTable::confirmJoinedDevice(Address32 address) {
    Device& device = getDevice(address);

    if (device.status == DeviceStatus::DELETED) {
        LOG_ERROR("confirmJoinedDevice - status deleted for device: " << ToStr(address));
        return;
    }

    if (address == GATEWAY_ADDRESS) {
        device.status = DeviceStatus::OPERATIONAL;
    } else {
        device.status = DeviceStatus::QUARANTINE;
    }

    device.setAction(DeviceAction::ROUTER_SUCCESS);
}

Device& DevicesTable::getDevice(Address32 address32) {
    Devices32Container::iterator foundDevice = devices.find(address32);
    if (foundDevice != devices.end()) {
        return foundDevice->second;
    } else {
        LOG_ERROR("Device " << ToStr(address32) << " not found!");

        std::ostringstream streamAddress;
        streamAddress << ToStr(address32);
        throw DeviceNotFoundException(streamAddress.str());
    }
}

DeviceStatus::DeviceStatus DevicesTable::getDeviceStatus(Address32 address32) {
    return getDevice(address32).status;
}

DeviceHistory& DevicesTable::getDeviceHistory(Address32 address32) {
    return getDevice(address32).deviceHistory;
}

Device& DevicesTable::addDevice(const Capabilities& capabilities, Address32 newAddress32, Address32 parentAddress32) {

    incrementJoinNumber(newAddress32);

    if (!existsDevice(parentAddress32) || !getDevice(parentAddress32).isJoinConfirmed()) {
        std::ostringstream stream;
        stream << "Illegal join for device " << capabilities.euidAddress.toString();
        stream << " to parent " << ToStr(parentAddress32);
        stream << ". Parent not joined yet.";
        LOG_ERROR(stream.str());
        throw NEException(stream.str());
    }

    const Address64& newAddress64 = capabilities.euidAddress;

    Address32 joinedBackbone32 = 0;

    if (capabilities.isFieldDevice()) {
        joinedBackbone32 = getDevice(parentAddress32).joinedBackbone32;
    } else {
        joinedBackbone32 = newAddress32;
    }

    LOG_INFO("newAddress64=" << newAddress64.toString() << " parentAddress32=" << ToStr(parentAddress32));

    if (existsDevice(newAddress32)) {
        //if (getDevice(newAddress32).isRemoveStatus() || getDevice(newAddress32).isDeletedStatus()) {
        Device& device = devices[newAddress32];
        device.setCapabilities(capabilities);

    } else if (existsDeletedDevice(newAddress64)) {
        Device& device = deletedDevices[newAddress64];

        device.address32 = newAddress32;
        device.setCapabilities(capabilities);

        devices[newAddress32] = device;
        deletedDevices.erase(newAddress64);
    } else {
        Device newDevice(capabilities);
        newDevice.address32 = newAddress32;

#ifdef JOIN_REASON
        PendingJoinReasonsMap::iterator jr = pendingJoinReasons.find(newAddress64);
        if (jr != pendingJoinReasons.end()) {
            for (std::vector<Device::RejoinReason>::iterator jrIt = jr->second.begin(); jrIt != jr->second.end(); jrIt++) {
                newDevice.setJoinReason(jrIt->joinReason);
            }
        }
        pendingJoinReasons.erase(newAddress64);
#endif

        devices[newAddress32] = newDevice;
    }

    Device& device = devices[newAddress32];
    for (mapByJoinTime::iterator it = devicesMap.begin(); it != devicesMap.end(); ++it)
    {
        if (it->second->address32 == device.address32)
        {
            devicesMap.erase(it);
            break;
        }
    }

    device.status = DeviceStatus::JOIN_REQUEST_RECEIVED;
    device.joinedBackbone32 = joinedBackbone32;
    device.deviceHistory.setLastJoinTime(time(NULL));
    device.parent32 = parentAddress32;
    device.getMetaDataAttributes()->resetAttributes();

    devicesMap.insert(std::make_pair(device.deviceHistory.lastJoinTime, &device));

    addressMapping[newAddress64] = newAddress32;
    addressMapping32to64[newAddress32] = newAddress64;

    return device;
}

void DevicesTable::incrementJoinNumber(Address32 joinedDeviceAddress) {
    std::map<Address32, Uint32>::iterator it = nrOfJoinsPerDevice.find(joinedDeviceAddress);
    if (it != nrOfJoinsPerDevice.end()) {
        it->second += 1;
    } else {
        nrOfJoinsPerDevice.insert(std::make_pair(joinedDeviceAddress, (Uint32) 1));
    }
}

Address32 DevicesTable::addManager(Uint16 networkId, const Address64& managerAddress64, std::string& lTag) {
    Address32 managerAddress16 = MANAGER_ADDRESS;

    LOG_INFO("Adding Manager: addres16=" << ToStr(managerAddress16) << ", addreess64=" << managerAddress64.toString());

    Capabilities capabilities;
    capabilities.euidAddress = managerAddress64;
    capabilities.deviceType = DeviceType::MANAGER;

    for (uint8_t i = 0; i < lTag.length(); i++) {
        capabilities.longTag[i] = lTag[i];
    }
    capabilities.longTag[lTag.length()] = '\0';

    {
        //Table 1. Expanded Device Type Codes
        capabilities.readUniqueIdentifier.expandedDeviceType = ExpandedDeviceTypeCode_WirelessHART_Network_Manager;

        capabilities.readUniqueIdentifier.minReqPreamblesNo = 2;
        capabilities.readUniqueIdentifier.protocolMajorRevNo = 7;
        capabilities.readUniqueIdentifier.deviceRevisionLevel = 0;
        capabilities.readUniqueIdentifier.softwareRevisionLevel = 0;
        capabilities.readUniqueIdentifier.hardwareRevisionLevel = 0;

        //Table 10. Physical Signaling Codes
        //    PhysicalSignalingCodes_BELL202CURRENT = 0,
        //    PhysicalSignalingCodes_BELL202VOLTAGE = 1,
        //    PhysicalSignalingCodes_RS485 = 2,
        //    PhysicalSignalingCodes_RS232 = 3,
        //    PhysicalSignalingCodes_WIRELESS = 4,
        //    PhysicalSignalingCodes_SPECIAL
        capabilities.readUniqueIdentifier.physicalSignalingCode = PhysicalSignalingCodes_BELL202CURRENT;

        //Table 11. Flag Assignments
        //    FlagAssignmentsMask_MultiSensorFieldDevice = 0x01,
        //    FlagAssignmentsMask_EEPromControl = 0x02,
        //    FlagAssignmentsMask_ProtocolBridge = 0x04,
        //    FlagAssignmentsMask_O0PSKMoldulation = 0x08,
        //    FlagAssignmentsMask_C8pskCapable = 0x40,
        //    FlagAssignmentsMask_C8pskInMultiDropOnly = 0x80
        capabilities.readUniqueIdentifier.flags = FlagAssignmentsMask_MultiSensorFieldDevice;

        capabilities.readUniqueIdentifier.deviceID = 1; // 24 bits
        capabilities.readUniqueIdentifier.minRespPreamblesNo = 0;
        capabilities.readUniqueIdentifier.maxNoOfDeviceVars = 0;
        capabilities.readUniqueIdentifier.configChangeCounter = 0;

        // table 17
        //    ExtendedDeviceStatusCodesMask_MaintenanceRequired = 0x01,
        //    ExtendedDeviceStatusCodesMask_DeviceVariableAlert = 0x02,
        //    ExtendedDeviceStatusCodesMask_CriticalPowerFailure = 0x04
        capabilities.readUniqueIdentifier.extendedFieldDeviceStatus = ExtendedDeviceStatusCodesMask_MaintenanceRequired;

        // Table 8
        capabilities.readUniqueIdentifier.manufacturerIDCode = ManufacturerIdentificationCodes_ACROMAG;

        // Table 8
        capabilities.readUniqueIdentifier.privateLabelDistributorCode = ManufacturerIdentificationCodes_ACROMAG;

        //Table 57. Device Profile Code
        //    DeviceProfileCodes_HART_PROCESS_AUTOMATION_DEVICE = 1,
        //    DeviceProfileCodes_HART_DISCRETE_DEVICE = 2,
        //    DeviceProfileCodes_HYBRID_PROCAUTOMATION_AND_DISCRETE = 3,
        //    DeviceProfileCodes_IO_SYSTEM = 4,
        //    DeviceProfileCodes_WIRELESSHART_PROCESS_AUTOMATION_DEVICE = 129,
        //    DeviceProfileCodes_WIRELESSHART_DISCRETE_DEVICE = 130,
        //    DeviceProfileCodes_WIRELESSHART_HYBRID_PROCAUTOMATION_AND_DISCRETE = 131,
        //    DeviceProfileCodes_WIRELESSHART_GATEWAY = 132,
        //    DeviceProfileCodes_WIRELESSHART_PROCESS_ADAPTER = 141,
        //    DeviceProfileCodes_WIRELESSHART_DISCRETE_ADAPTER = 142,
        //    DeviceProfileCodes_WIRELESSHART_HANDHELD_MAINTENANCE_TOOL = 144
        capabilities.readUniqueIdentifier.deviceProfile = DeviceProfileCodes_HART_PROCESS_AUTOMATION_DEVICE;
    }

    Device newDevice(capabilities);
    newDevice.status = DeviceStatus::OPERATIONAL;
    newDevice.address32 = managerAddress16;

    newDevice.deviceHistory.setLastJoinTime(time(NULL));

    devices[newDevice.address32] = newDevice;
    devicesMap.insert(std::make_pair(newDevice.deviceHistory.lastJoinTime, &devices[newDevice.address32]));

    addressMapping[managerAddress64] = newDevice.address32;
    addressMapping32to64[newDevice.address32] = managerAddress64;

    return newDevice.address32;
}

void DevicesTable::setRemoveStatus(Address32 address) {
    LOG_DEBUG("setRemoveStatus address=" << ToStr(address));
    Devices32Container::iterator it = devices.find(address);

    if (it != devices.end()) {
        //TODO: ivp - delete the device from the list?
        it->second.status = DeviceStatus::DELETED;
        if (it->second.capabilities.isGateway()) {
            it->second.deviceRequested832 = true;
        } else {
            it->second.deviceRequested832 = false;
        }
        LOG_DEBUG("setRemoveStatus - done");
    }
}

void DevicesTable::deleteDevice(Address32 address32) {
    if (existsDevice(address32)) {
        Device& device = devices[address32];
        Address64 address64 = device.address64;
        deletedDevices[address64] = devices[address32];
        for (mapByJoinTime::iterator it =  devicesMap.lower_bound(device.deviceHistory.lastJoinTime);
            it != devicesMap.upper_bound(device.deviceHistory.lastJoinTime); it++)
        {
            if (it->second == &device)
            {
                devicesMap.erase(it);
                break;
            }
        }
        devices.erase(address32);   //DO NOT USE device PAST THIS POINT
        addressMapping.erase(address64);
        addressMapping32to64.erase(address32);

    } else {
        std::ostringstream stream;
        stream << "Device " << ToStr(address32) << " not found!";
        LOG_ERROR(stream.str());
        throw DeviceNotFoundException(stream.str());
    }
}

void DevicesTable::deleteDevice(const Address64 address64) {
    deleteDevice(getAddress32(address64));
}

Uint16 DevicesTable::getNumberOfJoinInProgress(Uint32 parentAddress32, std::set<Address32>& addresses) {
    Uint16 noJoins = 0;
    Devices32Container::iterator it = devices.begin();
    for (; it != devices.end(); ++it) {
        if (it->second.parent32 == parentAddress32) {
            if (it->second.status == DeviceStatus::QUARANTINE) {
                ++noJoins;
                addresses.insert(it->first);
            }
        }
    }

    return noJoins;
}

Uint16 DevicesTable::getNumberOfChildren(Uint32 parentAddress32, std::set<Address32>& addresses) {
    Uint16 noJoins = 0;
    Devices32Container::iterator it = devices.begin();
    for (; it != devices.end(); ++it) {
        if (it->second.parent32 == parentAddress32 && it->second.status == DeviceStatus::OPERATIONAL) {
            ++noJoins;
            addresses.insert(it->first);
        }
    }

    return noJoins;
}

#ifdef JOIN_REASON
void DevicesTable::addDeviceJoinReason(Address64 longAddress, Uint8 joinReason[4])
{
    Address32 nickname = createAddress32(longAddress);

    if (existsDevice(nickname)) {
        Device& device = getDevice(nickname);
        device.setJoinReason(joinReason);
    } else if (existsDeletedDevice(longAddress)) {
        Device& delDevice = deletedDevices[longAddress];
        delDevice.setJoinReason(joinReason);
    } else {
        pendingJoinReasons[longAddress].push_back(Device::RejoinReason());
        memcpy(pendingJoinReasons[longAddress].back().joinReason, joinReason, 4);
    }

}
#endif

void DevicesTable::clearTable() {
    deletedDevices.clear();
    devices.clear();
    devicesMap.clear();
    addressMapping.clear();
    addressMapping32to64.clear();
}

void DevicesTable::toString(std::ostringstream& stream) {
    stream << "DeviceTable {";
    stream << "size=" << devices.size() << ", ";

    int i = 0;
    for (Devices32Container::iterator it = devices.begin(); it != devices.end(); ++it) {
        stream << " Device[" << (int) i++ << "]: address32=" << ToStr(it->first) << ", ";
        it->second.toString(stream);
    }
    stream << "}";
}

void DevicesTable::toIndentString(std::ostringstream& stream) {

    Uint16 operationalsNo = 0; // used to count the operational devices
    Devices32Container::iterator it = devices.begin();
    for (; it != devices.end(); ++it) {
        if (it->second.isOperational()) {
                ++operationalsNo;
         }
    }

    stream << "DeviceTable {" << "size=" << devices.size();
    stream << ", operational=" << (int) operationalsNo;
    stream << std::endl;
    stream << "  ";
    stream << std::setw(5) << "Adr";
    stream << std::setw(5) << "Prnt";
    stream << std::setw(24) << "EUI64";
    stream << std::setw(13) << "status";
    stream << std::setw(3) << "J#";
    stream << std::setw(3) << "JP";
    stream << std::setw(16) << "lastJoinTime";
    stream << std::setw(21) << "Res";
#ifdef JOIN_REASON
    stream << std::setw(13 * MAX_REJOINREASON_SIZE) << "JoinReason";
#endif
    stream << "  last action";
    stream << std::endl;


    int i = 0;
    for (mapByJoinTime::iterator it = devicesMap.begin(); it != devicesMap.end(); ++it) {
        stream << std::setw(2) << std::dec << (int) ++i;
        it->second->toTableIndentString(stream, nrOfJoinsPerDevice[it->second->address32]);
        stream << std::endl;
    }
    stream << "}";
    stream << std::endl;
}

void DevicesTable::toShortIndentString(std::ostringstream& stream) {
    stream << "DeviceTable {";
    stream << "size=" << devices.size() << ", ";
    stream << std::endl;
    int i = 1;
    for (Devices32Container::iterator it = devices.begin(); it != devices.end(); ++it) {
        stream << "  [" << (int) i++ << "]: " << it->second.toShortIndentString();
        stream << std::endl;
    }
    stream << "}";
    stream << std::endl;

}

