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
 * AllNMParsersComposers.h
 *
 *  Created on: Nov 6, 2009
 *      Author: andrei.petrut
 */

#ifndef ALLNMPARSERSCOMPOSERS_H_
#define ALLNMPARSERSCOMPOSERS_H_

#include "AllNetworkManagerCommands.h"

/**
 * All the parsers and the composers for all the commands that the Network Manager will use.
 */

namespace hart7 {

namespace nmanager {

/*
 * uint16_t cmdId;
 *	ParseFunction fnParser;
 *	ExecuteFunction fnExecute;
 *	ComposeFunction fnComposer;
 */
//N,0
//N,20
//N,787
//961,961
//964,962
//963,962
static ParseExecuteComposerEntry g_composeReqParseResp[] =
{   //WARN these need to be in order for the find method to work.
	{ CMDID_C000_ReadUniqueIdentifier, (ParseFunction) Parse_C000_ReadUniqueIdentifier_Resp, NULL, NULL },
	{ CMDID_C020_ReadLongTag, (ParseFunction) Parse_C020_ReadLongTag_Resp, NULL, NULL },
    { CMDID_C773_WriteNetworkId, (ParseFunction)Parse_C773_WriteNetworkId_Resp, NULL, (ComposeFunction) Compose_C773_WriteNetworkId_Req},
    { CMDID_C777_ReadWirelessDeviceInformation, (ParseFunction)Parse_C777_ReadWirelessDeviceInformation_Resp, NULL, (ComposeFunction) Compose_C777_ReadWirelessDeviceInformation_Req},
    { CMDID_C779_ReportDeviceHealth, (ParseFunction)Parse_C779_ReportDeviceHealth_Resp, NULL, (ComposeFunction) Compose_C779_ReportDeviceHealth_Req},
	{ CMDID_C780_ReportNeighborHealthList, (ParseFunction)Parse_C780_ReportNeighborHealthList_Resp, NULL, (ComposeFunction) Compose_C780_ReportNeighborHealthList_Req},
	{ CMDID_C784_ReadLinkList, (ParseFunction)Parse_C784_ReadLinkList_Resp, NULL, (ComposeFunction) Compose_C784_ReadLinkList_Req},
	{ CMDID_C787_ReportNeighborSignalLevels, (ParseFunction)Parse_C787_ReportNeighborSignalLevels_Resp, NULL, (ComposeFunction) Compose_C787_ReportNeighborSignalLevels_Req},
	{ CMDID_C788_AlarmPathDown, (ParseFunction)Parse_C788_AlarmPathDown_Resp, NULL, (ComposeFunction) NULL},
	{ CMDID_C789_AlarmSourceRouteFailed, (ParseFunction)Parse_C789_AlarmSourceRouteFailed_Resp, NULL, (ComposeFunction) NULL},
	{ CMDID_C790_AlarmGraphRouteFailed, (ParseFunction)Parse_C790_AlarmGraphRouteFailed_Resp, NULL, (ComposeFunction) NULL},
	{ CMDID_C791_AlarmTransportLayerFailed, (ParseFunction)Parse_C791_AlarmTransportLayerFailed_Resp, NULL, (ComposeFunction) NULL},
	{ CMDID_C795_WriteTimerInterval, (ParseFunction)Parse_C795_WriteTimerInterval_Resp, NULL, (ComposeFunction) Compose_C795_WriteTimerInterval_Req},
	{ CMDID_C801_DeleteService, (ParseFunction)Parse_C801_DeleteService_Resp, NULL, (ComposeFunction) Compose_C801_DeleteService_Req},
	{ CMDID_C811_WriteJoinPriority, (ParseFunction)Parse_C811_WriteJoinPriority_Resp, NULL, (ComposeFunction) Compose_C811_WriteJoinPriority_Req},
	{ CMDID_C818_WriteChannelBlacklist, (ParseFunction)Parse_C818_WriteChannelBlacklist_Resp, NULL, (ComposeFunction) Compose_C818_WriteChannelBlacklist_Req},
	{ CMDID_C839_ChangeNotification, NULL, NULL, (ComposeFunction) Compose_C839_ChangeNotification_Resp},
	{ CMDID_C961_WriteNetworkKey, (ParseFunction) Parse_C961_WriteNetworkKey_Resp, NULL, (ComposeFunction) Compose_C961_WriteNetworkKey_Req},
	{ CMDID_C962_WriteDeviceNicknameAddress, (ParseFunction) Parse_C962_WriteDeviceNicknameAddress_Resp, NULL, (ComposeFunction) Compose_C962_WriteDeviceNicknameAddress_Req},
	{ CMDID_C963_WriteSession, (ParseFunction) Parse_C963_WriteSession_Resp, NULL, (ComposeFunction) Compose_C963_WriteSession_Req},
	{ CMDID_C965_WriteSuperframe, (ParseFunction)Parse_C965_WriteSuperframe_Resp, NULL, (ComposeFunction) Compose_C965_WriteSuperframe_Req},
	{ CMDID_C967_WriteLink, (ParseFunction)Parse_C967_WriteLink_Resp, NULL, (ComposeFunction) Compose_C967_WriteLink_Req},
	{ CMDID_C968_DeleteLink, (ParseFunction)Parse_C968_DeleteLink_Resp, NULL, (ComposeFunction) Compose_C968_DeleteLink_Req},
	{ CMDID_C969_WriteGraphNeighbourPair, (ParseFunction)Parse_C969_WriteGraphNeighbourPair_Resp, NULL, (ComposeFunction) Compose_C969_WriteGraphNeighbourPair_Req},
	{ CMDID_C970_DeleteGraphConnection, (ParseFunction)Parse_C970_DeleteGraphConnection_Resp, NULL, (ComposeFunction) Compose_C970_DeleteGraphConnection_Req},
	{ CMDID_C971_WriteNeighbourPropertyFlag, (ParseFunction)Parse_C971_WriteNeighbourPropertyFlag_Resp, NULL, (ComposeFunction) Compose_C971_WriteNeighbourPropertyFlag_Req},
	{ CMDID_C973_WriteService, (ParseFunction)Parse_C973_WriteService_Resp, NULL, (ComposeFunction) Compose_C973_WriteService_Req},
	{ CMDID_C974_WriteRoute, (ParseFunction)Parse_C974_WriteRoute_Resp, NULL, (ComposeFunction) Compose_C974_WriteRoute_Req},
	{ CMDID_C975_DeleteRoute, (ParseFunction)Parse_C975_DeleteRoute_Resp, NULL, (ComposeFunction) Compose_C975_DeleteRoute_Req},
	{ CMDID_C976_WriteSourceRoute, (ParseFunction)Parse_C976_WriteSourceRoute_Resp, NULL, (ComposeFunction) Compose_C976_WriteSourceRoute_Req},
	{ CMDID_C977_DeleteSourceRoute, (ParseFunction)Parse_C977_DeleteSourceRoute_Resp, NULL, (ComposeFunction) Compose_C977_DeleteSourceRoute_Req},
	{ CMDID_C64765_NivisMetaCommand, NULL, NULL, (ComposeFunction) Compose_C64765_NivisMetaCommand_Resp}
};


static ParseExecuteComposerEntry g_parseReqComposeResp[] = {
        { CMDID_C000_ReadUniqueIdentifier, (ParseFunction)Parse_C000_ReadUniqueIdentifier_Req, NULL, (ComposeFunction)Compose_C000_ReadUniqueIdentifier_Resp },
        { CMDID_C013_ReadTagDescriptorDate, (ParseFunction)Parse_C013_ReadTagDescriptorDate_Req, NULL, (ComposeFunction) Compose_C013_ReadTagDescriptorDate_Resp},
        { CMDID_C020_ReadLongTag, (ParseFunction)Parse_C020_ReadLongTag_Req, NULL, (ComposeFunction) Compose_C020_ReadLongTag_Resp},
        { CMDID_C769_ReadJoinStatus, (ParseFunction)Parse_C769_ReadJoinStatus_Req, NULL, (ComposeFunction) Compose_C769_ReadJoinStatus_Resp},
        { CMDID_C773_WriteNetworkId, (ParseFunction)Parse_C773_WriteNetworkId_Req, NULL, (ComposeFunction) Compose_C773_WriteNetworkId_Resp},
        { CMDID_C774_ReadNetworkId, (ParseFunction)Parse_C774_ReadNetworkId_Req, NULL, (ComposeFunction) Compose_C774_ReadNetworkId_Resp},
        { CMDID_C775_WriteNetworkTag, (ParseFunction)Parse_C775_WriteNetworkTag_Req, NULL, (ComposeFunction) Compose_C775_WriteNetworkTag_Resp},
        { CMDID_C776_ReadNetworkTag, (ParseFunction)Parse_C776_ReadNetworkTag_Req, NULL, (ComposeFunction) Compose_C776_ReadNetworkTag_Resp},
        { CMDID_C776_ReadNetworkTag, (ParseFunction)Parse_C776_ReadNetworkTag_Req, NULL, (ComposeFunction) Compose_C776_ReadNetworkTag_Resp},
        { CMDID_C778_ReadBatteryLife, (ParseFunction)Parse_C778_ReadBatteryLife_Req, NULL, (ComposeFunction) Compose_C778_ReadBatteryLife_Resp},
		{ CMDID_C779_ReportDeviceHealth, (ParseFunction)Parse_C779_ReportDeviceHealth_Req, NULL, (ComposeFunction) Compose_C779_ReportDeviceHealth_Resp},
		{ CMDID_C780_ReportNeighborHealthList, (ParseFunction)Parse_C780_ReportNeighborHealthList_Req, NULL, (ComposeFunction) Compose_C780_ReportNeighborHealthList_Resp},
		{ CMDID_C781_ReadDeviceNicknameAddress, (ParseFunction)Parse_C781_ReadDeviceNicknameAddress_Req, NULL, (ComposeFunction) Compose_C781_ReadDeviceNicknameAddress_Resp},
        { CMDID_C782_ReadSessionEntries, (ParseFunction)Parse_C782_ReadSessionEntries_Req, NULL, (ComposeFunction) Compose_C782_ReadSessionEntries_Resp},
        { CMDID_C783_ReadSuperframeList, (ParseFunction)Parse_C783_ReadSuperframeList_Req, NULL, (ComposeFunction) Compose_C783_ReadSuperframeList_Resp},
        { CMDID_C784_ReadLinkList, (ParseFunction)Parse_C784_ReadLinkList_Req, NULL, (ComposeFunction) Compose_C784_ReadLinkList_Resp},
        { CMDID_C785_ReadGraphList, (ParseFunction)Parse_C785_ReadGraphList_Req, NULL, (ComposeFunction) Compose_C785_ReadGraphList_Resp},
        { CMDID_C786_ReadNeighborPropertyFlag, (ParseFunction)Parse_C786_ReadNeighborPropertyFlag_Req, NULL, (ComposeFunction) Compose_C786_ReadNeighborPropertyFlag_Resp},
		{ CMDID_C787_ReportNeighborSignalLevels, (ParseFunction)Parse_C787_ReportNeighborSignalLevels_Req, NULL, (ComposeFunction) Compose_C787_ReportNeighborSignalLevels_Resp},
		{ CMDID_C788_AlarmPathDown, (ParseFunction)Parse_C788_AlarmPathDown_Req, NULL, (ComposeFunction) Compose_C788_AlarmPathDown_Resp},
		{ CMDID_C789_AlarmSourceRouteFailed, (ParseFunction)Parse_C789_AlarmSourceRouteFailed_Req, NULL, (ComposeFunction) Compose_C789_AlarmSourceRouteFailed_Resp},
		{ CMDID_C790_AlarmGraphRouteFailed, (ParseFunction)Parse_C790_AlarmGraphRouteFailed_Req, NULL, (ComposeFunction) Compose_C790_AlarmGraphRouteFailed_Resp},
		{ CMDID_C791_AlarmTransportLayerFailed, (ParseFunction)Parse_C791_AlarmTransportLayerFailed_Req, NULL, (ComposeFunction) Compose_C791_AlarmTransportLayerFailed_Resp},
		{ CMDID_C794_ReadUTCTime, (ParseFunction)Parse_C794_ReadUTCTime_Req, NULL, (ComposeFunction) Compose_C794_ReadUTCTime_Resp},
		{ CMDID_C799_RequestService, (ParseFunction)Parse_C799_RequestService_Req, NULL, (ComposeFunction) Compose_C799_RequestService_Resp},
	    { CMDID_C800_ReadServiceList, (ParseFunction)Parse_C800_ReadServiceList_Req, NULL, (ComposeFunction) Compose_C800_ReadServiceList_Resp},
	    { CMDID_C801_DeleteService, (ParseFunction)Parse_C801_DeleteService_Req, NULL, (ComposeFunction) Compose_C801_DeleteService_Resp},
	    { CMDID_C802_ReadRouteList, (ParseFunction)Parse_C802_ReadRouteList_Req, NULL, (ComposeFunction) Compose_C802_ReadRouteList_Resp},
	    { CMDID_C803_ReadSourceRoute, (ParseFunction)Parse_C803_ReadSourceRoute_Req, NULL, (ComposeFunction) Compose_C803_ReadSourceRoute_Resp},
	    { CMDID_C814_ReadDeviceListEntries, (ParseFunction)Parse_C814_ReadDeviceListEntries_Req, NULL, (ComposeFunction) Compose_C814_ReadDeviceListEntries_Resp},
	    { CMDID_C817_ReadChannelBlacklist, (ParseFunction)Parse_C817_ReadChannelBlacklist_Req, NULL, (ComposeFunction) Compose_C817_ReadChannelBlacklist_Resp},
	    { CMDID_C818_WriteChannelBlacklist, (ParseFunction)Parse_C818_WriteChannelBlacklist_Req, NULL, (ComposeFunction) Compose_C818_WriteChannelBlacklist_Resp},
	    { CMDID_C821_WriteNetworkAccessMode, (ParseFunction)Parse_C821_WriteNetworkAccessMode_Req, NULL, (ComposeFunction) Compose_C821_WriteNetworkAccessMode_Resp},
	    { CMDID_C822_ReadNetworkAccessMode, (ParseFunction)Parse_C822_ReadNetworkAccessMode_Req, NULL, (ComposeFunction) Compose_C822_ReadNetworkAccessMode_Resp},
		{ CMDID_C832_ReadNetworkDeviceIdentity, (ParseFunction)Parse_C832_ReadNetworkDeviceIdentity_Req, NULL, (ComposeFunction) Compose_C832_ReadNetworkDeviceIdentity_Resp},
		{ CMDID_C833_ReadNetworkDeviceNeighbourHealth, (ParseFunction)Parse_C833_ReadNetworkDeviceNeighbourHealth_Req, NULL, (ComposeFunction) Compose_C833_ReadNetworkDeviceNeighbourHealth_Resp},
		{ CMDID_C834_ReadNetworkTopologyInformation, (ParseFunction)Parse_C834_ReadNetworkTopologyInformation_Req, NULL, (ComposeFunction) Compose_C834_ReadNetworkTopologyInformation_Resp},
        { CMDID_C839_ChangeNotification, (ParseFunction)Parse_C839_ChangeNotification_Req, NULL, (ComposeFunction) Compose_C839_ChangeNotification_Resp},
        { CMDID_C840_ReadDeviceStatistics, (ParseFunction)Parse_C840_ReadDeviceStatistics_Req, NULL, (ComposeFunction) Compose_C840_ReadDeviceStatistics_Resp},
        { CMDID_C842_WriteDeviceSchedulingFlags, (ParseFunction)Parse_C842_WriteDeviceSchedulingFlags_Req, NULL, (ComposeFunction) Compose_C842_WriteDeviceSchedulingFlags_Resp},
        { CMDID_C965_WriteSuperframe, (ParseFunction)Parse_C965_WriteSuperframe_Req, NULL, (ComposeFunction) Compose_C965_WriteSuperframe_Resp},
        { CMDID_C966_DeleteSuperframe, (ParseFunction)Parse_C966_DeleteSuperframe_Req, NULL, (ComposeFunction) Compose_C966_DeleteSuperframe_Resp},
        { CMDID_C967_WriteLink, (ParseFunction)Parse_C967_WriteLink_Req, NULL, (ComposeFunction) Compose_C967_WriteLink_Resp},
        { CMDID_C968_DeleteLink, (ParseFunction)Parse_C968_DeleteLink_Req, NULL, (ComposeFunction) Compose_C968_DeleteLink_Resp},
        { CMDID_C969_WriteGraphNeighbourPair, (ParseFunction)Parse_C969_WriteGraphNeighbourPair_Req, NULL, (ComposeFunction) Compose_C969_WriteGraphNeighbourPair_Resp},
        { CMDID_C970_DeleteGraphConnection, (ParseFunction)Parse_C970_DeleteGraphConnection_Req, NULL, (ComposeFunction) Compose_C970_DeleteGraphConnection_Resp},
        { CMDID_C971_WriteNeighbourPropertyFlag, (ParseFunction)Parse_C971_WriteNeighbourPropertyFlag_Req, NULL, (ComposeFunction) Compose_C971_WriteNeighbourPropertyFlag_Resp},
        { CMDID_C973_WriteService, (ParseFunction)Parse_C973_WriteService_Req, NULL, (ComposeFunction) Compose_C973_WriteService_Resp},
        { CMDID_C974_WriteRoute, (ParseFunction)Parse_C974_WriteRoute_Req, NULL, (ComposeFunction) Compose_C974_WriteRoute_Resp},
        { CMDID_C975_DeleteRoute, (ParseFunction)Parse_C975_DeleteRoute_Req, NULL, (ComposeFunction) Compose_C975_DeleteRoute_Resp},
        { CMDID_C976_WriteSourceRoute, (ParseFunction)Parse_C976_WriteSourceRoute_Req, NULL, (ComposeFunction) Compose_C976_WriteSourceRoute_Resp},
        { CMDID_C977_DeleteSourceRoute, (ParseFunction)Parse_C977_DeleteSourceRoute_Req, NULL, (ComposeFunction) Compose_C977_DeleteSourceRoute_Resp},
        { CMDID_C64765_NivisMetaCommand, (ParseFunction)Parse_C64765_NivisMetaCommand_Req, NULL, (ComposeFunction) Compose_C64765_NivisMetaCommand_Resp}

/*TODO: fill the list with the appropriate (ParseFunction/NULL/ComposeFunction)*/
};

} // namespace nmanager
} // namespace hart7


#endif /* ALLNMPARSERSCOMPOSERS_H_ */
