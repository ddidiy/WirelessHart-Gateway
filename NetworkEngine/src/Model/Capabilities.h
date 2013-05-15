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

#ifndef CAPABILITIES_H_
#define CAPABILITIES_H_

#include "Common/NETypes.h"
#include "Common/NEAddress.h"

#include <ApplicationLayer/Model/DataLinkLayerCommands.h>
#include <ApplicationLayer/Model/UniversalCommands.h>
#include <ApplicationLayer/Model/WirelessApplicationCommands.h>

using namespace NE::Common;
using namespace NE::Misc::Marshall;

namespace NE {
namespace Model {
namespace DeviceType {

/**
 * Bit 0 - IO
 * Bit 1 - router
 * Bit 2 - backbone router
 * Bit 3 - gateway
 * Bit 4 - system manager
 * Bit 5 - security manager
 * Bit 6 - system time source
 * Bit 7 - provisioning device
 */
enum DeviceTypeEnum {
    NOT_SET = 0, IN_OUT = 1, // if device has IO role (has a sensor attached to it)
    ROUTER = 2,
    BACKBONE = 4,
    GATEWAY = 8,
    MANAGER = 16,
    SECURITY_MANAGER = 32,
    TIME_SOURCE = 64,
    PROVISIONING = 128
};

} // namespace DeviceType


class Capabilities {

    public:

        Address64 euidAddress;
        Uint16 dllSubnetId;
        Uint16 deviceType; //to verify the role of device use the provided methods "isXXX"
        std::string tagName;
        Uint8 softwareMajorVersion;
        Uint8 softwareMinorVersion;
        // TODO : radu : it should be VisibleString ... something about UDO ...
        Uint8 softwareRevision;

        C000_ReadUniqueIdentifier_Resp readUniqueIdentifier;
        Uint8 longTag[32];
        C787_ReportNeighborSignalLevels_Resp neighborSignalLevels;

        // keeps the last reports received from the device
        C779_ReportDeviceHealth_Resp reportDeviceHealth;
        C780_ReportNeighborHealthList_Resp reportNeighborHealthList;
        C787_ReportNeighborSignalLevels_Resp reportNeighborSignalLevels;

        //------------------------------
        //Capabilities from dlmo.DeviceCapability
        //------------------------------------

        /**
         * capacity of the queue that is available for forwarding operations.
         * default 0xFFFF
         */
        Uint16 queueCapacity;

        /**
         * Nominal clock accuracy of this device, in ppm)
         */
        Uint16 clockAcurracy;

        /**
         * Map of radio channels supported by the device
         * default 0xFFFF
         */
        Uint16 channelMap;

        /**
         * Number of advertisements per minute.
         * default 0x7fff - continuous
         */
        Uint16 advertiseRate;

        /**
         * Number of seconds per hour operating the receiver.
         */
        Uint16 receiveCapacity;

        /**
         * Number of DSDUs forwarded per minute.
         */
        Uint16 forwardRate;

        /**
         * Time to turn around an ACK/NACK.
         */
        Uint16 ackTurnaround;

        /**
         * Memory capacity for dlmo11a.NeighborDiag.
         */
        Uint16 neighborDiagnosticsCapacity;

        /**
         * Radios maximum transmit power level in dBm.
         */
        Uint8 radioOutputPower;

        /**
         * Minimum dB reference for ED reporting.
         */
        Uint8 zeroED;

        /**
         * Maximum dB reference for ED reporting.
         */
        Uint8 maxED;

        /**
         * DeviceCapability.Options indicates optional features that the device supports:
         * Bit 0 = 1 indicates that group codes are supported in dlm011a.Neighbor.
         * Bit 1 = 1 indicates that graph extensions are supported in dlm011a.Neighbor.
         * Bit 2 = 1 indicates that the device is capable of receiving duocast or n-cast acknowledgments.
         * Bit 3 = 1 indicates that the device is capable of supporting dlmo11a.Superframe.SfType=1.
         * This may be needed in some regions for regulatory compliance.
         * Bits 4-7 are reserved and shall be set to 0.
         *
         */
        Uint8 options; //TODO add some methods to verify dif options available

        /**
         * Used for DeviceList.
         * Read from DMO attribute 6.
         * Max length: 16 characters.
         */
        VisibleString vendorID;

        /**
         * Used for DeviceList.
         * Read from DMO attribute 7.
         * Max length: 16 characters.
         */
        VisibleString modelID;

        /**
         * Used for DeviceList.
         * Read from DMO attribute 22.
         * Max length: 16 characters.
         */
        VisibleString softwareRevisionInfo;

        /**
         * Initialize to default values (0 to numbers, "" to strings).
         */
        Capabilities();

        /**
         * Returns true if the device type indicates a BACKBONE device type.
         */
        bool isBackbone() const;

        /**
         * Returns true if the device type indicates a FIELD_DEVICE device type.
         */
        bool isFieldDevice() const;

        /**
         * Returns true if the device type indicates a FIELD_DEVICE or ROUTER device type.
         */
        bool isDevice() const;

        /**
         * Returns true if the device type indicates a GATEWAY device type.
         */
        bool isGateway() const;

        /**
         * Returns true if the device type indicates a ROUTING device type.
         */
        bool isRouting() const;

        /**
         * Returns true if the device type indicates a SYSTEM_MANAGER device type.
         */
        bool isManager() const;

        /**
         * Returns true if the device type indicates a SECURITY_MANAGER device type.
         */
        bool isSecurityManager() const;

        /**
         * Returns true if the device type indicates a TIME_SOURCE device type.
         */
        bool isTimeSource() const;

        /**
         * Returns true if the device type indicates a PROVISIONING device type.
         */
        bool isProvisioning() const;

        std::string getDeviceTypeDescription();

    public:

        /**
         * Returns a string representation of these capabilities.
         */
        void toString(std::ostringstream& stream);

        /**
         * Returns a string representation of these capabilities
         * (for the table output format).
         */
        void toTableIndentString(std::ostringstream& stream);

        std::string getRoleAsString();

};
}
}

#endif /*CAPABILITIES_H_*/
