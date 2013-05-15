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

#ifndef DEVICETABLE_H_
#define DEVICETABLE_H_

#include <string>
#include <vector>
#include <boost/unordered_map.hpp>

#include "Model/Device.h"
#include "Model/Operations/EngineOperations.h"
#include "Common/SettingsLogic.h"

namespace NE {
namespace Model {

/**
 * Represents the exception that is thrown when you try to get a Device by
 * an Address64 and DeviceTable doesn't contain a device with the given address.
 * @author Beniamin Tecar, Ioan-Vasile Pocol
 * @version 1.0
 */
struct DeviceNotFoundException: public NEException {
    public:
        DeviceNotFoundException(std::string address) :
            NEException((std::string) "The Device with the address " + address + " was not found") {
        }
};

typedef boost::unordered_map<Address32, Device> Devices32Container;
typedef std::map<Address64, Device> Devices64Container;
typedef std::map<Address64, Address32> AddressMapping64_32;
typedef std::map<Address32, Address64> AddressMapping32_64;

/**
 * @author Beniamin Tecar
 * @version 1.0
 */
class DevicesTable {
    LOG_DEF("I.M.DevicesTable");

    private:
        /* the list of deleted devices. The History of devices is saved. */
        Devices64Container deletedDevices;

        /* the list of current joined devices */
        Devices32Container devices;

        std::map<Address32, Uint32> nrOfJoinsPerDevice;

#ifdef JOIN_REASON
        typedef std::map<Address64, std::vector<Device::RejoinReason> > PendingJoinReasonsMap;
        PendingJoinReasonsMap pendingJoinReasons;
#endif
        /* the mapping between address64 and address32 */
        AddressMapping64_32 addressMapping;

        AddressMapping32_64 addressMapping32to64;

        typedef std::multimap<int, Device*> mapByJoinTime;
        mapByJoinTime devicesMap;

        std::map<std::string, Address32> deviceNicknameMap;

        bool generateNicknamesFromEUI64;

        std::string nicknamesFileName;

        Address64 gatewayAddress64;


    public:

        DevicesTable();

        virtual ~DevicesTable();

        std::map<Address32, Uint32> getNrOfJoinsPerDevice();

        /**
         *
         */
        Address32 createAddress32(Address64 address64);

        bool notExistsConfirmedDeviceOrOnEvaluation(Address32 address);

        /**
         * Returns true if there is a device with the specified address32.
         */
        bool existsDevice(Address32 address);

        /**
         * Returns true if there is a device with the specified address64.
         */
        bool existsDevice(const Address64& address64);

        /**
         * Returns true if existed join a device with the given EUID address
         */
        bool existsDeletedDevice(const Address64& address64);

        /**
         * Returns true if there is a confirmed device with the specified address.
         */
        bool existsConfirmedDevice(Address32 address);

        /**
         * Address64 lookup by address32
         */
        Address64 getAddress64(Address32 address32);

        /**
         * Address32 lookup by address64
         */
        Address32 getAddress32(const Address64& address64);

        /**
         * Returns the previous allocated address32
         */
        Address32 getDeletedAddress32(const Address64& address64);

        /**
         * Find a device by its 32 address. If the device is not found an exception is thrown.
         * @throws DeviceNotFoundException if a device with this address is not found.
         */
        Device& getDevice(Address32 address);

        /**
         * Return the list of active devices
         */
        Devices32Container& getDevices() {
            return devices;
        }

        /**
         * Find the status of a device by its 32 address. If the device is not found an exception is thrown.
         * @throws DeviceNotFoundException if a device with this address is not found.
         */
        DeviceStatus::DeviceStatus getDeviceStatus(Address32 address);

        /**
         * Find the device history by its address32.
         * @throws DeviceNotFoundException if a device with this address is not found.
         */
        DeviceHistory& getDeviceHistory(Address32 address32);

        /**
         * If the Address64 already exists the old history is recovered.
         */
        Device& addDevice(const Capabilities& capabilities, Address32 newAddress32, Address32 parentAddress32);

        /**
         */
        void incrementJoinNumber(Address32 joinedDeviceAddress);

        /**
         * Add the manager device to all provisioned subnets
         */
        Address32 addManager(Uint16 networkId, const Address64& managerAddress64, std::string& lTag);

        /**
         * Saves the devices history and delete the device from the list of active devices
         */
        void deleteDevice(Address32 address32);

        /**
         * Saves the devices history and delete the device from the list of active devices
         */
        void deleteDevice(const Address64 address64);

        /**
         * Set DELETED the status of the device
         */
        void setRemoveStatus(Address32 address);

        /**
         * A notification that a device has joined.
         * @param joinedDeviceAddress - the joined device address.
         */
        void confirmJoinedDevice(Address32 joinedDeviceAddress);

        /**
         * Commit the changes: change the current status or delete from model all not used entities
         */
        void commitChanges();

        /**
         * Roll back model (Remove entries with status=NEW).
         */
        void rollbackModel();

        /**
         * Returns the number of devices from the device table.
         */
        unsigned int size() {
            return devices.size();
        }

        /**
         * Clear the internal content of the table.
         */
        void clearTable();

        /**
         * Returns a string representation for the object.
         */
        void toString(std::ostringstream& stream);

        /**
         * Returns a user friendly string representation for the object, indented to facilitate the view.
         */
        void toIndentString(std::ostringstream& stream);

        /**
         * Returns a SHORT string representation for the object.
         * (the name and address of the device)
         */
        void toShortIndentString(std::ostringstream& stream);

        /**
         * Returns the number of join in progress.
         */
        Uint16 getNumberOfJoinInProgress(Uint32 parentAddress32, std::set<Address32>& addresses);

        /**
         * Returns the number of devices joined through the given device.
         */
        Uint16 getNumberOfChildren(Uint32 parentAddress32, std::set<Address32>& addresses);

        /**
        * Attempts to load device nicknames from file on disk.
        * This way one device will keep its nickname from one session to another.
        */
        void LoadDeviceNicknames (SettingsLogic& settingsLogic);

#ifdef JOIN_REASON
        void addDeviceJoinReason(Address64 longAddress, Uint8 joinReason[4]);
#endif

};

}
}

#endif /*DEVICETABLE_H_*/
