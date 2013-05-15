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

#include "NMLog.h"

#include "CommandsToString.h"
#include "Misc/Convert/Convert.h"
#include "Model/NetworkEngine.h"
#include "../nmanager/AllNetworkManagerCommands.h"

using namespace NE::Misc::Convert;

namespace hart7 {

namespace util {

struct HartCmdLog
{
        LOG_DEF("I.S.Cmds")
        ;

        static void logHartCommand(uint16_t commandId, bool isRequest, uint16_t errorCode, void* command,
                                   const WHartAddress& devAddress)
        {
            if (!(LOG_INFO_ENABLED()) || command == NULL)
            {
                return;
            }

            std::ostringstream stream;
            stream << devAddress << ": ";

            if (commandId == CMDID_C000_ReadUniqueIdentifier)
            {
                if (isRequest)
                {
                    hart7::util::toStringC000(stream, (C000_ReadUniqueIdentifier_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C000_ReadUniqueIdentifier_Resp*) command);
                }
            }
            else if (commandId == CMDID_C013_ReadTagDescriptorDate)
            {
                if (isRequest)
                {
                    hart7::util::toStringC013(stream, (C013_ReadTagDescriptorDate_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C013_ReadTagDescriptorDate_Resp*) command);
                }
            }
            else if (commandId == CMDID_C020_ReadLongTag)
            {
                if (isRequest)
                {
                    hart7::util::toStringC020(stream, (C020_ReadLongTag_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C020_ReadLongTag_Resp*) command);
                }
            }
            else if (commandId == CMDID_C769_ReadJoinStatus)
            {
                if (isRequest)
                {
                    hart7::util::toStringC769(stream, (C769_ReadJoinStatus_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C769_ReadJoinStatus_Resp*) command);
                }
            }
            else if (commandId == CMDID_C770_RequestActiveAdvertise)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C770_RequestActiveAdvertise_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C770_RequestActiveAdvertise_Resp*) command);
                }
            }
            else if (commandId == CMDID_C772_ReadJoinModeConfiguration)
            {
                if (isRequest)
                {
                    hart7::util::toStringC772(stream, (C772_ReadJoinModeConfiguration_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C772_ReadJoinModeConfiguration_Resp*) command);
                }
            }
            else if (commandId == CMDID_C774_ReadNetworkId)
            {
                if (isRequest)
                {
                    hart7::util::toStringC774(stream, (C774_ReadNetworkId_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C774_ReadNetworkId_Resp*) command);
                }
            }
            else if (commandId == CMDID_C776_ReadNetworkTag)
            {
                if (isRequest)
                {
                    hart7::util::toStringC776(stream, (C776_ReadNetworkTag_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C776_ReadNetworkTag_Resp*) command);
                }
            }
            else if (commandId == CMDID_C777_ReadWirelessDeviceInformation)
            {
                if (isRequest)
                {
                    hart7::util::toStringC777(stream, (C777_ReadWirelessDeviceInformation_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C777_ReadWirelessDeviceInformation_Resp*) command);
                }
            }
            else if (commandId == CMDID_C778_ReadBatteryLife)
            {
                if (isRequest)
                {
                    hart7::util::toStringC778(stream, (C778_ReadBatteryLife_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C778_ReadBatteryLife_Resp*) command);
                }
            }
            else if (commandId == CMDID_C779_ReportDeviceHealth)
            {
                if (isRequest)
                {
                    hart7::util::toStringC779(stream, (C779_ReportDeviceHealth_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C779_ReportDeviceHealth_Resp*) command);
                }
            }
            else if (commandId == CMDID_C780_ReportNeighborHealthList)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C780_ReportNeighborHealthList_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C780_ReportNeighborHealthList_Resp*) command);
                }
            }
            else if (commandId == CMDID_C781_ReadDeviceNicknameAddress)
            {
                if (isRequest)
                {
                    hart7::util::toStringC781(stream, (C781_ReadDeviceNicknameAddress_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C781_ReadDeviceNicknameAddress_Resp*) command);
                }
            }
            else if (commandId == CMDID_C782_ReadSessionEntries)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C782_ReadSessionEntries_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C782_ReadSessionEntries_Resp*) command);
                }
            }
            else if (commandId == CMDID_C783_ReadSuperframeList)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C783_ReadSuperframeList_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C783_ReadSuperframeList_Resp*) command);
                }
            }
            else if (commandId == CMDID_C784_ReadLinkList)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C784_ReadLinkList_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C784_ReadLinkList_Resp*) command);
                }
            }
            else if (commandId == CMDID_C785_ReadGraphList)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C785_ReadGraphList_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C785_ReadGraphList_Resp*) command);
                }
            }
            else if (commandId == CMDID_C786_ReadNeighborPropertyFlag)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C786_ReadNeighborPropertyFlag_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C786_ReadNeighborPropertyFlag_Resp*) command);
                }
            }
            else if (commandId == CMDID_C787_ReportNeighborSignalLevels)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C787_ReportNeighborSignalLevels_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C787_ReportNeighborSignalLevels_Resp*) command);
                }
            }
            else if (commandId == CMDID_C788_AlarmPathDown)
            {
                if (isRequest)
                {
                    hart7::util::toString788(stream, (C788_AlarmPathDown_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C788_AlarmPathDown_Resp*) command);
                }
            }
            else if (commandId == CMDID_C789_AlarmSourceRouteFailed)
            {
                if (isRequest)
                {
                    hart7::util::toStringC789(stream, (C789_AlarmSourceRouteFailed_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C789_AlarmSourceRouteFailed_Resp*) command);
                }
            }
            else if (commandId == CMDID_C790_AlarmGraphRouteFailed)
            {
                if (isRequest)
                {
                    hart7::util::toStringC790(stream, (C790_AlarmGraphRouteFailed_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C790_AlarmGraphRouteFailed_Resp*) command);
                }
            }
            else if (commandId == CMDID_C791_AlarmTransportLayerFailed)
            {
                if (isRequest)
                {
                    hart7::util::toStringC791(stream, (C791_AlarmTransportLayerFailed_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C791_AlarmTransportLayerFailed_Resp*) command);
                }
            }
            else if (commandId == CMDID_C794_ReadUTCTime)
            {
                if (isRequest)
                {
                    hart7::util::toStringC794(stream, (C794_ReadUTCTime_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C794_ReadUTCTime_Resp*) command);
                }
            }
            else if (commandId == CMDID_C795_WriteTimerInterval)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C795_WriteTimerInterval_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C795_WriteTimerInterval_Resp*) command);
                }
            }
            else if (commandId == CMDID_C796_ReadTimerInterval)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C796_ReadTimerInterval_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C796_ReadTimerInterval_Resp*) command);
                }
            }
            else if (commandId == CMDID_C797_WriteRadioPower)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C797_WriteRadioPower_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C797_WriteRadioPower_Resp*) command);
                }
            }
            else if (commandId == CMDID_C798_ReadRadioPower)
            {
                if (isRequest)
                {
                    hart7::util::toStringC798(stream, (C798_ReadRadioPower_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C798_ReadRadioPower_Resp*) command);
                }
            }
            else if (commandId == CMDID_C799_RequestService)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C799_RequestService_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C799_RequestService_Resp*) command);
                }
            }
            else if (commandId == CMDID_C800_ReadServiceList)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C800_ReadServiceList_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C800_ReadServiceList_Resp*) command);
                }
            }
            else if (commandId == CMDID_C801_DeleteService)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C801_DeleteService_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C801_DeleteService_Resp*) command);
                }
            }
            else if (commandId == CMDID_C802_ReadRouteList)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C802_ReadRouteList_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C802_ReadRouteList_Resp*) command);
                }
            }
            else if (commandId == CMDID_C803_ReadSourceRoute)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C803_ReadSourceRoute_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C803_ReadSourceRoute_Resp*) command);
                }
            }
            else if (commandId == CMDID_C805_WriteRadioCCAMode)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C805_WriteRadioCCAMode_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C805_WriteRadioCCAMode_Resp*) command);
                }
            }
            else if (commandId == CMDID_C806_ReadHandheldSuperframe)
            {
                if (isRequest)
                {
                    hart7::util::toStringC806(stream, (C806_ReadHandheldSuperframe_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C806_ReadHandheldSuperframe_Resp*) command);
                }
            }
            else if (commandId == CMDID_C808_ReadTimeToLive)
            {
                if (isRequest)
                {
                    hart7::util::toStringC808(stream, (C808_ReadTimeToLive_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C808_ReadTimeToLive_Resp*) command);
                }
            }
            else if (commandId == CMDID_C809_WriteTimeToLive)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C809_WriteTimeToLive_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C809_WriteTimeToLive_Resp*) command);
                }
            }
            else if (commandId == CMDID_C811_WriteJoinPriority)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C811_WriteJoinPriority_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C811_WriteJoinPriority_Resp*) command);
                }
            }
            else if (commandId == CMDID_C813_WritePacketReceivePriority)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C813_WritePacketReceivePriority_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C813_WritePacketReceivePriority_Resp*) command);
                }
            }
            else if (commandId == CMDID_C814_ReadDeviceListEntries)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C814_ReadDeviceListEntries_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C814_ReadDeviceListEntries_Resp*) command);
                }
            }
            else if (commandId == CMDID_C817_ReadChannelBlacklist)
            {
                if (isRequest)
                {
                    hart7::util::toStringC817(stream, (C817_ReadChannelBlacklist_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C817_ReadChannelBlacklist_Resp*) command);
                }
            }
            else if (commandId == CMDID_C818_WriteChannelBlacklist)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C818_WriteChannelBlacklist_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C818_WriteChannelBlacklist_Resp*) command);
                }
            }
            else if (commandId == CMDID_C820_WriteBackOffExponent)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C820_WriteBackOffExponent_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C820_WriteBackOffExponent_Resp*) command);
                }
            }
            else if (commandId == CMDID_C823_RequestSession)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C823_RequestSession_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C823_RequestSession_Resp*) command);
                }
            }
            else if (commandId == CMDID_C832_ReadNetworkDeviceIdentity)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C832_ReadNetworkDeviceIdentity_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C832_ReadNetworkDeviceIdentity_Resp*) command);
                }
            }
            else if (commandId == CMDID_C833_ReadNetworkDeviceNeighbourHealth)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C833_ReadNetworkDeviceNeighbourHealth_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C833_ReadNetworkDeviceNeighbourHealth_Resp*) command);
                }
            }
            else if (commandId == CMDID_C834_ReadNetworkTopologyInformation)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C834_ReadNetworkTopologyInformation_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C834_ReadNetworkTopologyInformation_Resp*) command);
                }
            }
            else if (commandId == CMDID_C839_ChangeNotification)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C839_ChangeNotification_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C839_ChangeNotification_Resp*) command);
                }
            }
            else if (commandId == CMDID_C840_ReadDeviceStatistics)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C840_ReadDeviceStatistics_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C840_ReadDeviceStatistics_Resp*) command);
                }
            }
            else if (commandId == CMDID_C960_DisconnectDevice)
            { // BEGIN WirelessNetworkManagerCommands
                if (isRequest)
                {
                    hart7::util::toString(stream, (C960_DisconnectDevice_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C960_DisconnectDevice_Resp*) command);
                }
            }
            else if (commandId == CMDID_C961_WriteNetworkKey)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C961_WriteNetworkKey_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C961_WriteNetworkKey_Resp*) command);
                }
            }
            else if (commandId == CMDID_C962_WriteDeviceNicknameAddress)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C962_WriteDeviceNicknameAddress_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C962_WriteDeviceNicknameAddress_Resp*) command);
                }
            }
            else if (commandId == CMDID_C963_WriteSession)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C963_WriteSession_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C963_WriteSession_Resp*) command);
                }
            }
            else if (commandId == CMDID_C964_DeleteSession)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C964_DeleteSession_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C964_DeleteSession_Resp*) command);
                }
            }
            else if (commandId == CMDID_C965_WriteSuperframe)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C965_WriteSuperframe_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C965_WriteSuperframe_Resp*) command);
                }
            }
            else if (commandId == CMDID_C966_DeleteSuperframe)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C966_DeleteSuperframe_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C966_DeleteSuperframe_Resp*) command);
                }
            }
            else if (commandId == CMDID_C967_WriteLink)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C967_WriteLink_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C967_WriteLink_Resp*) command);
                }
            }
            else if (commandId == CMDID_C968_DeleteLink)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C968_DeleteLink_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C968_DeleteLink_Resp*) command);
                }
            }
            else if (commandId == CMDID_C969_WriteGraphNeighbourPair)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C969_WriteGraphNeighbourPair_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C969_WriteGraphNeighbourPair_Resp*) command);
                }
            }
            else if (commandId == CMDID_C970_DeleteGraphConnection)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C970_DeleteGraphConnection_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C970_DeleteGraphConnection_Resp*) command);
                }
            }
            else if (commandId == CMDID_C971_WriteNeighbourPropertyFlag)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C971_WriteNeighbourPropertyFlag_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C971_WriteNeighbourPropertyFlag_Resp*) command);
                }
            }
            else if (commandId == CMDID_C972_SuspendDevices)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C972_SuspendDevices_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C972_SuspendDevices_Resp*) command);
                }
            }
            else if (commandId == CMDID_C973_WriteService)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C973_WriteService_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C973_WriteService_Resp*) command);
                }
            }
            else if (commandId == CMDID_C974_WriteRoute)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C974_WriteRoute_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C974_WriteRoute_Resp*) command);
                }
            }
            else if (commandId == CMDID_C975_DeleteRoute)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C975_DeleteRoute_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C975_DeleteRoute_Resp*) command);
                }
            }
            else if (commandId == CMDID_C976_WriteSourceRoute)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C976_WriteSourceRoute_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C976_WriteSourceRoute_Resp*) command);
                }
            }
            else if (commandId == CMDID_C977_DeleteSourceRoute)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C977_DeleteSourceRoute_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C977_DeleteSourceRoute_Resp*) command);
                }
            }
            else if (commandId == CMDID_C64765_NivisMetaCommand)
            {
                if (isRequest)
                {
                    hart7::util::toString(stream, (C64765_NivisMetaCommand_Req*) command);
                }
                else
                {
                    hart7::util::toString(stream, (C64765_NivisMetaCommand_Resp*) command);
                }
            }
            else
            {
                stream << "Unknown command id : " << commandId;
            }
            // END WirelessNetworkManagerCommands

            if (!isRequest)
            {
                stream << ", errorCode=" << std::dec << (int) errorCode;
            }

            LOG_INFO(stream.str());
        }

        static void logOperation(hart7::nmanager::operations::WHOperationPointer wHOperation)
        {
            if (!(LOG_INFO_ENABLED()))
                return;

            std::ostringstream str;
            wHOperation->getEngineOperation()->toString(str);
            LOG_INFO(str.str());
        }
};

struct HartDeviceHistoryLog
{
        LOG_DEF("NMState.DeviceHistory")
        ;

        static void logHistory()
        {
            using namespace NE::Model;

            if (!(LOG_DEBUG_ENABLED()))
                return;

            std::ostringstream stream;
            stream << "DevicesHistory {";
            stream << std::endl;
            stream << std::setw(8) << "Device";
            DeviceHistory::toHeaderString(stream);

            DevicesTable& devicesTable = NetworkEngine::instance().getDevicesTable();
            Devices32Container& devices = devicesTable.getDevices();

            // sort the devices after their last join time
            typedef std::multimap<int, Device*> mapByJoinTime;
            mapByJoinTime devicesMap;
            for (Devices32Container::iterator it = devices.begin(); it != devices.end(); ++it)
            {
                devicesMap.insert(std::make_pair(it->second.deviceHistory.lastJoinTime, &(it->second)));
            }

            for (mapByJoinTime::iterator itDev = devicesMap.begin(); itDev != devicesMap.end(); ++itDev)
            {
                stream << std::endl;
                stream << NE::Common::ToStr(itDev->second->address32, 8);
                itDev->second->deviceHistory.toString(stream);
            }

            stream << std::endl;
            stream << "}";

            LOG_DEBUG(stream.str());
        }

};

NMLog::NMLog()
{

}

NMLog::~NMLog()
{
}

void NMLog::logCommand(uint16_t commandId, void* command, const WHartAddress& address)
{
    HartCmdLog::logHartCommand(commandId, true, 0, command, address);
}

void NMLog::logCommandResponse(uint16_t commandId, uint16_t errorCode, void* command, const WHartAddress& address)
{
    HartCmdLog::logHartCommand(commandId, false, errorCode, command, address);
}

void NMLog::logOperation(hart7::nmanager::operations::WHOperationPointer wHOperation)
{
    HartCmdLog::logOperation(wHOperation);

    logCommand((int) wHOperation->getWHartCommand().commandID, wHOperation->getWHartCommand().command,
               wHOperation->getDestinationAddress());
}

void NMLog::logOperationResponse(hart7::nmanager::operations::WHOperationPointer wHOperation)
{
    HartCmdLog::logOperation(wHOperation);

    logCommandResponse((int) wHOperation->getWHartCommand().commandID, 0, wHOperation->getWHartCommand().command,
                       wHOperation->getDestinationAddress());
}

void NMLog::logDeviceHistory()
{
    HartDeviceHistoryLog::logHistory();
}

}

}
