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
 * GatewayWrappedRequestsHandler.h
 *
 *  Created on: Nov 6, 2009
 *      Author: Radu Pop
 */

#ifndef GATEWAYWRAPPEDREQUESTSHANDLER_H_
#define GATEWAYWRAPPEDREQUESTSHANDLER_H_

#include "../AllNetworkManagerCommands.h"
#include "../CommonData.h"
#include <nlib/log.h>
#include <WHartStack/WHartStack.h>

namespace hart7 {

namespace nmanager {

/**
 * Handles 64765 requests from the GW - wrapped requests.
 * Provides data about devices that is known by the Network Manager (links, graphs, etc).
 */
class GatewayWrappedRequestsHandler
{
        LOG_DEF("h7.s.GatewayWrappedRequestsHandler")
        ;

    private:

        hart7::nmanager::CommonData& commonData;

    public:

        GatewayWrappedRequestsHandler(hart7::nmanager::CommonData& commonData_);

        virtual ~GatewayWrappedRequestsHandler();

        /**
         * Handles a wrapped request from the GW.
         */
        uint8_t Handle(WHartHandle handle, const C64765_NivisMetaCommand_Req & nivisMetaCommand,
                       C64765_NivisMetaCommand_Resp *resp);

    private:

        void
                    ComposeCommand(uint16_t commandID, uint8_t responseCode, void* command,
                                   C64765_NivisMetaCommand_Resp& resp);

        /**
         * Determines the number of graphs for the given node address.
         */
        void determineNoGraphs(Address32 address32, uint16_t& graphsNo, uint8_t& neighborsNo);

    public:

        uint8_t Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                       const C000_ReadUniqueIdentifier_Req& readUniqueIdentifierReq,
                       C000_ReadUniqueIdentifier_Resp& resp);

        uint8_t Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                       const C013_ReadTagDescriptorDate_Req& readTagDescriptorDateReq,
                       C013_ReadTagDescriptorDate_Resp &resp);

        uint8_t Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                       const C020_ReadLongTag_Req& readLongTagReq, C020_ReadLongTag_Resp& resp);

        uint8_t Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                       const C769_ReadJoinStatus_Req& readJoinStatus, C769_ReadJoinStatus_Resp& resp);

        uint8_t Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                       const C773_WriteNetworkId_Req& writeNetworkId, C773_WriteNetworkId_Resp& resp);

        uint8_t Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                       const C774_ReadNetworkId_Req& readNetworkId, C774_ReadNetworkId_Resp& resp);

        uint8_t Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                       const C775_WriteNetworkTag_Req& writeNetworkTag, C775_WriteNetworkTag_Resp& resp);

        uint8_t Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                       const C776_ReadNetworkTag_Req& readNetworkTag, C776_ReadNetworkTag_Resp& resp);

        uint8_t Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                       const C778_ReadBatteryLife_Req& readBatteryLife, C778_ReadBatteryLife_Resp& resp);

        uint8_t Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                       const C779_ReportDeviceHealth_Req& reportDeviceHealth, C779_ReportDeviceHealth_Resp& resp);

        uint8_t Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                       const C780_ReportNeighborHealthList_Req& reportNeighborHealthList,
                       C780_ReportNeighborHealthList_Resp& resp);

        uint8_t Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                       const C781_ReadDeviceNicknameAddress_Req& readDeviceNicknameAddress,
                       C781_ReadDeviceNicknameAddress_Resp& resp);

        uint8_t Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                       const C782_ReadSessionEntries_Req& readSessionEntries, C782_ReadSessionEntries_Resp& resp);

        uint8_t Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                       const C783_ReadSuperframeList_Req& readSuperframeList, C783_ReadSuperframeList_Resp& resp);

        uint8_t Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                       const C784_ReadLinkList_Req& readLinkList, C784_ReadLinkList_Resp& resp);

        uint8_t Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                       const C785_ReadGraphList_Req& readGraphList, C785_ReadGraphList_Resp& resp);

        uint8_t Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                       const C786_ReadNeighborPropertyFlag_Req& readNeighborPropertyFlag,
                       C786_ReadNeighborPropertyFlag_Resp& resp);

        uint8_t Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                       const C787_ReportNeighborSignalLevels_Req& reportNeighborSignalLevels,
                       C787_ReportNeighborSignalLevels_Resp& resp);

        uint8_t Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                       const C794_ReadUTCTime_Req& readUTCTime, C794_ReadUTCTime_Resp& resp);

        uint8_t Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                       const C800_ReadServiceList_Req& readServiceList, C800_ReadServiceList_Resp& resp);

        uint8_t Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                       const C802_ReadRouteList_Req& readRouteList, C802_ReadRouteList_Resp& resp);

        uint8_t Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                       const C803_ReadSourceRoute_Req& readSourceRoute, C803_ReadSourceRoute_Resp& resp);

        uint8_t Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                       const C814_ReadDeviceListEntries_Req& readDeviceListEntries,
                       C814_ReadDeviceListEntries_Resp &resp);

        uint8_t Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                       const C817_ReadChannelBlacklist_Req& readChannelBlacklist, C817_ReadChannelBlacklist_Resp &resp);

        uint8_t Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                       const C818_WriteChannelBlacklist_Req& writeChannelBlacklist,
                       C818_WriteChannelBlacklist_Resp &resp);

        uint8_t Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                       const C821_WriteNetworkAccessMode_Req& writeNetworkAccessMode,
                       C821_WriteNetworkAccessMode_Resp &resp);

        uint8_t Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                       const C822_ReadNetworkAccessMode_Req& readNetworkAccessMode,
                       C822_ReadNetworkAccessMode_Resp &resp);

        uint8_t Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                       const C832_ReadNetworkDeviceIdentity_Req& readNetworkDeviceIdentity,
                       C832_ReadNetworkDeviceIdentity_Resp &resp);

        uint8_t Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                       const C833_ReadNetworkDeviceNeighbourHealth_Req& readNetworkDeviceNeighbourHealth,
                       C833_ReadNetworkDeviceNeighbourHealth_Resp &resp);

        uint8_t Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                       const C834_ReadNetworkTopologyInformation_Req& readNetworkTopologyInformation,
                       C834_ReadNetworkTopologyInformation_Resp &resp);

        uint8_t Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                       const C840_ReadDeviceStatistics_Req& readDeviceStatistics, C840_ReadDeviceStatistics_Resp &resp);

        uint8_t Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                       const C842_WriteDeviceSchedulingFlags_Req& writeDeviceSchedulingFlags,
                       C842_WriteDeviceSchedulingFlags_Resp &resp);

        uint8_t Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                       const C843_ReadDeviceSchedulingFlags_Req& readDeviceSchedulingFlags,
                       C843_ReadDeviceSchedulingFlags_Resp &resp);

        uint8_t Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                       const C844_ReadNetworkConstraints_Req& readNetworkConstraints,
                       C844_ReadNetworkConstraints_Resp &resp);

        uint8_t Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                       const C845_WriteNetworkConstraints_Req& writeNetworkConstraints,
                       C845_WriteNetworkConstraints_Resp &resp);
};

}
}

#endif /* GATEWAYWRAPPEDREQUESTSHANDLER_H_ */
