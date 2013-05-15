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
 * CommandsToString.h
 *
 *  Created on: Nov 6, 2009
 *      Author: Radu Pop
 */

#ifndef COMMANDSTOSTRING_H_
#define COMMANDSTOSTRING_H_

#include "../nmanager/AllNetworkManagerCommands.h"
#include "Misc/Convert/Convert.h"
#include "Model/NetworkEngine.h"

using namespace NE::Misc::Convert;

namespace hart7 {

namespace util {

inline void toStringC000(std::ostringstream& stream, const C000_ReadUniqueIdentifier_Req * req)
{
    stream << "EmptyCommand_Req  {";
    stream << "}";
}
inline void toString(std::ostringstream& stream, const C000_ReadUniqueIdentifier_Resp * resp)
{
    stream << "C000_ReadUniqueIdentifier_Resp {";
    stream << "deviceID: " << std::hex << resp->deviceID;
    stream << ", expandedDeviceType: " << std::dec << (int) resp->expandedDeviceType;
    stream << ", configChangeCounter: " << std::dec << (int) resp->configChangeCounter;
    stream << ", manufacturerIDCode: " << std::dec << (int) resp->manufacturerIDCode;
    stream << ", privateLabelDistributorCode: " << std::dec << (int) resp->privateLabelDistributorCode;
    stream << ", minReqPreamblesNo: " << std::dec << (int) resp->minReqPreamblesNo;
    stream << ", protocolMajorRevNo: " << std::dec << (int) resp->protocolMajorRevNo;
    stream << ", deviceRevisionLevel: " << std::dec << (int) resp->deviceRevisionLevel;
    stream << ", softwareRevisionLevel: " << std::dec << (int) resp->softwareRevisionLevel;
    stream << ", hardwareRevisionLevel: " << std::dec << (int) resp->hardwareRevisionLevel;
    stream << ", physicalSignalingCode: " << std::dec << (int) resp->physicalSignalingCode;
    stream << ", flags: " << std::dec << (int) resp->flags;
    stream << ", minRespPreamblesNo: " << std::dec << (int) resp->minRespPreamblesNo;
    stream << ", maxNoOfDeviceVars: " << std::dec << (int) resp->maxNoOfDeviceVars;
    stream << ", extendedFieldDeviceStatus: " << std::dec << (int) resp->extendedFieldDeviceStatus;
    stream << ", deviceProfile: " << std::dec << (int) resp->deviceProfile;

    stream << "}";
}

inline void toStringC013(std::ostringstream& stream, const C013_ReadTagDescriptorDate_Req * req)
{
    stream << "C013_ReadTagDescriptorDate_Req  {";
    stream << "}";
}
inline void toString(std::ostringstream& stream, const C013_ReadTagDescriptorDate_Resp * resp)
{
    stream << "C013_ReadTagDescriptorDate_Resp {";
    stream << "tag: " << array2string((uint8_t*) resp->tag, 8);
    stream << ", descriptor: " << array2string((uint8_t*) resp->descriptor, 16);
    stream << ", dateCode: " << std::dec << (int) resp->dateCode.day;
    stream << " " << (int) resp->dateCode.month;
    stream << " " << (int) resp->dateCode.year;

    stream << "}";
}

inline void toStringC020(std::ostringstream& stream, const C020_ReadLongTag_Req * req)
{
    stream << "C020_ReadLongTag_Req  {";
    stream << "}";
}
inline void toString(std::ostringstream& stream, const C020_ReadLongTag_Resp * resp)
{
    stream << "C020_ReadLongTag_Resp {";
    stream << "longTag: " << array2string((uint8_t*) resp->longTag, 32);

    stream << "}";
}

inline void toStringC769(std::ostringstream& stream, const C769_ReadJoinStatus_Req * req)
{
    stream << "C769_ReadJoinStatus_Req  {";
    stream << "}";
}
inline void toString(std::ostringstream& stream, const C769_ReadJoinStatus_Resp * resp)
{
    stream << "C769_ReadJoinStatus_Resp {";
    stream << " wirelessMode=" << (int) resp->wirelessMode;
    stream << ", joinStatus=" << (int) resp->joinStatus;
    stream << ", noOfAvailableNeighbors=" << (int) resp->noOfAvailableNeighbors;
    stream << ", noOfAdvertisingPacketsReceived=" << (int) resp->noOfAdvertisingPacketsReceived;
    stream << ", noOfJoinAttempts=" << (int) resp->noOfJoinAttempts;
    stream << ", joinRetryTimer=" << (int) resp->joinRetryTimer.u32;
    stream << ", networkSearchTimer=" << (int) resp->networkSearchTimer.u32;

    stream << "}";
}

inline void toString(std::ostringstream& stream, const C770_RequestActiveAdvertise_Req * req)
{
    stream << "C770_RequestActiveAdvertise_Req  {";
    stream << " ShedTime=" << (int) req->m_tShedTime.u32;

    stream << "}";
}
inline void toString(std::ostringstream& stream, const C770_RequestActiveAdvertise_Resp * resp)
{
    stream << "C770_RequestActiveAdvertise_Resp {";
    stream << " ShedTime=" << (int) resp->m_tShedTime.u32;
    stream << ", AdvertisingPeriod=" << (int) resp->m_tAdvertisingPeriod.u32;
    stream << ", NoOfNeighborsAdvertising=" << (int) resp->m_ucNoOfNeighborsAdvertising;

    stream << "}";
}

inline void toStringC772(std::ostringstream& stream, const C772_ReadJoinModeConfiguration_Req * req)
{
    stream << "C772_ReadJoinModeConfiguration_Req  {";
    stream << "}";
}
inline void toString(std::ostringstream& stream, const C772_ReadJoinModeConfiguration_Resp * resp)
{
    stream << "C772_ReadJoinModeConfiguration_Resp {";
    stream << " JoinShedTime=" << (int) resp->m_tJoinShedTime.u32;
    stream << ", JoinMode=" << (int) resp->m_ucJoinMode;

    stream << "}";
}

inline void toStringC774(std::ostringstream& stream, const C774_ReadNetworkId_Req * req)
{
    stream << "C774_ReadNetworkId_Req  {";
    stream << "}";
}
inline void toString(std::ostringstream& stream, const C774_ReadNetworkId_Resp * resp)
{
    stream << "C774_ReadNetworkId_Resp {";
    stream << ", NetworkId=" << (int) resp->m_unNetworkId;

    stream << "}";
}

inline void toStringC776(std::ostringstream& stream, const C776_ReadNetworkTag_Req * req)
{
    stream << "C776_ReadNetworkTag_Req  {";
    stream << "}";
}
inline void toString(std::ostringstream& stream, const C776_ReadNetworkTag_Resp * resp)
{
    stream << "C776_ReadNetworkTag_Resp {";
    stream << "NetworkTag=" << array2string((uint8_t*) resp->m_aNetworkTag, 32);

    stream << "}";
}

inline void toStringC777(std::ostringstream& stream, const C777_ReadWirelessDeviceInformation_Req * req)
{
    stream << "C777_ReadWirelessDeviceInformation_Req  {";
    stream << "}";
}
inline void toString(std::ostringstream& stream, const C777_ReadWirelessDeviceInformation_Resp * resp)
{
    stream << "C777_ReadWirelessDeviceInformation_Resp {";
    stream << "PeakPacketsPerSecond=" << std::dec << (int) resp->m_fPeakPacketsPerSecond;
    stream << ", DurationAtPeakPacketLoadBeforePowerDrained=" << std::dec
                << (int) resp->m_tDurationAtPeakPacketLoadBeforePowerDrained.u32;
    stream << ", TimeToRecoverFromPowerDrain=" << std::dec << (int) resp->m_tTimeToRecoverFromPowerDrain.u32;
    stream << ", MinRequiredKeepAliveTime=" << std::dec << (int) resp->m_tMinRequiredKeepAliveTime.u32;
    stream << ", MaxNoOfNeighbors=" << std::dec << (int) resp->m_unMaxNoOfNeighbors;
    stream << ", MaxNoOfPacketBuffers=" << std::dec << (int) resp->m_unMaxNoOfPacketBuffers;
    stream << ", PowerSource=" << std::dec << (int) resp->m_ucPowerSource;
    stream << ", RSLThresholdGoodConnection=" << std::dec << (int) resp->m_cRSLThresholdGoodConnection;

    stream << "}";
}

inline void toStringC778(std::ostringstream& stream, const C778_ReadBatteryLife_Req * req)
{
    stream << "C778_ReadBatteryLife_Req  {";
    stream << "}";
}
inline void toString(std::ostringstream& stream, const C778_ReadBatteryLife_Resp * resp)
{
    stream << "C778_ReadBatteryLife_Resp {";
    stream << "BatteryLifeRemaining=" << std::dec << (int) resp->m_unBatteryLifeRemaining;

    stream << "}";
}

inline void toStringC779(std::ostringstream& stream, const C779_ReportDeviceHealth_Req * req)
{
    stream << "C779_ReportDeviceHealth_Req  {";
    stream << "}";
}
inline void toString(std::ostringstream& stream, const C779_ReportDeviceHealth_Resp * resp)
{
    stream << "C779_ReportDeviceHealth_Resp {";
    stream << "NoOfPacketsGeneratedByDevice=" << std::dec << (int) resp->m_unNoOfPacketsGeneratedByDevice;
    stream << ", NoOfPacketsTerminatedByDevice=" << std::dec << (int) resp->m_unNoOfPacketsTerminatedByDevice;
    stream << ", NoOfDataLinkLayerMICFailuresDetected=" << std::dec
                << (int) resp->m_ucNoOfDataLinkLayerMICFailuresDetected;
    stream << ", NoOfNetworkLayerMICFailuresDetected=" << std::dec
                << (int) resp->m_ucNoOfNetworkLayerMICFailuresDetected;
    stream << ", PowerStatus=" << std::dec << (int) resp->m_ucPowerStatus;
    stream << ", NoOfCRCErrorsDetected=" << std::dec << (int) resp->m_ucNoOfCRCErrorsDetected;
    stream << ", NoOfNonceCounterValuesNotReceived=" << std::dec << (int) resp->m_ucNoOfNonceCounterValuesNotReceived;

    stream << "}";
}

inline void toString(std::ostringstream& stream, const C780_ReportNeighborHealthList_Req * req)
{
    stream << "C780_ReportNeighborHealthList_Req  {";
    stream << " neighborsTableIndex=" << (int) req->m_ucNeighborTableIndex;
    stream << ", neighborsRead=" << (int) req->m_ucNeighborTableIndex;
    stream << "}";
}
inline void toString(std::ostringstream& stream, const C780_ReportNeighborHealthList_Resp* resp)
{
    stream << "C780_ReportNeighborHealthList_Resp {";
    stream << "totalNeighbors=" << (int) resp->m_ucTotalNoOfNeighbors;
    stream << ", neighborsTableIndex=" << (int) resp->m_ucNeighborTableIndex;
    stream << ", neighborsRead=" << (int) resp->m_ucNoOfNeighborEntriesRead;
    stream << ", Neighbors = {";

    for (int i = 0; i < resp->m_ucNoOfNeighborEntriesRead; i++)
    {
        stream << "[nicknameOfNeighbor=" << ToStr(resp->m_aNeighbors[i].nicknameOfNeighbor);
        stream << ", flags=" << std::dec << (int) resp->m_aNeighbors[i].neighborFlags;
        stream << ", meanRSL=" << (int) resp->m_aNeighbors[i].meanRSLSinceLastReport;
        stream << ", packetsTransmittedTo=" << (int) resp->m_aNeighbors[i].noOfPacketsTransmittedToThisNeighbor;
        stream << ", packetsFailedTo=" << (int) resp->m_aNeighbors[i].noOfFailedTransmits;
        stream << ", packetsReceivedFrom=" << (int) resp->m_aNeighbors[i].noOfPacketsReceivedFromThisNeighbor;
        stream << "] ";
    }

    stream << "}";
    stream << "}";
}

inline void toStringC781(std::ostringstream& stream, const C781_ReadDeviceNicknameAddress_Req * req)
{
    stream << "C781_ReadDeviceNicknameAddress_Req  {";
    stream << "}";
}
inline void toString(std::ostringstream& stream, const C781_ReadDeviceNicknameAddress_Resp * resp)
{
    stream << "C781_ReadDeviceNicknameAddress_Resp {";
    stream << "Nickname=" << ToStr(resp->Nickname);

    stream << "}";
}

inline void toString(std::ostringstream& stream, const C782_ReadSessionEntries_Req * req)
{
    stream << "C782_ReadSessionEntries_Req  {";
    stream << "SessionIndex=" << std::dec << (int) req->m_ucSessionIndex;
    stream << ", NoOfEntriesToRead=" << std::dec << (int) req->m_ucNoOfEntriesToRead;
    stream << "}";
}
inline void toString(std::ostringstream& stream, const C782_ReadSessionEntries_Resp * resp)
{
    stream << "C782_ReadSessionEntries_Resp {";
    stream << "SessionIndex=" << std::dec << (int) resp->m_ucSessionIndex;
    stream << ", NoOfEntriesRead=" << std::dec << (int) resp->m_ucNoOfEntriesRead;
    stream << ", NoOfActiveSessions=" << std::dec << (int) resp->m_unNoOfActiveSessions;
    stream << ", Sessions {";

    for (int i = 0; i < resp->m_ucNoOfEntriesRead; i++)
    {
        stream << " (peerDeviceNonceCounterVal=" << std::dec << (int) resp->m_aSessions[i].peerDeviceNonceCounterVal;
        stream << ", theDeviceNonceCounterVal=" << std::dec << (int) resp->m_aSessions[i].theDeviceNonceCounterVal;
        stream << ", peerDeviceNickname=" << ToStr(resp->m_aSessions[i].peerDeviceNickname);
        stream << ", peerDeviceUniqueId=" << array2string(resp->m_aSessions[i].peerDeviceUniqueId, 5);
        stream << ", sessionType=" << std::dec << (int) resp->m_aSessions[i].sessionType;
        stream << ") ";
    }

    stream << "}";
    stream << "}";
}

inline void toString(std::ostringstream& stream, const C783_ReadSuperframeList_Req * req)
{
    stream << "C783_ReadSuperframeList_Req  {";
    stream << "SuperframeIndex=" << std::dec << (int) req->m_ucSuperframeIndex;
    stream << ", NoOfEntriesToRead=" << std::dec << (int) req->m_ucNoOfEntriesToRead;
    stream << "}";
}
inline void toString(std::ostringstream& stream, const C783_ReadSuperframeList_Resp * resp)
{
    stream << "C783_ReadSuperframeList_Resp {";
    stream << "SuperframeIndex=" << std::dec << (int) resp->m_ucSuperframeIndex;
    stream << ", NoOfEntriesRead=" << std::dec << (int) resp->m_ucNoOfEntriesRead;
    stream << ", NoOfActiveSuperframes=" << std::dec << (int) resp->m_ucNoOfActiveSuperframes;
    stream << ", Superframes {";

    for (int i = 0; i < resp->m_ucNoOfEntriesRead; i++)
    {
        stream << " (superframeId=" << std::dec << (int) resp->m_aSuperframes[i].superframeId;
        stream << ", superframeModeFlags=" << std::dec << (int) resp->m_aSuperframes[i].superframeModeFlags;
        stream << ", noOfSlotsInSuperframe=" << std::dec << (int) resp->m_aSuperframes[i].noOfSlotsInSuperframe;
        stream << ") ";
    }

    stream << "}";
    stream << "}";
}

inline void toString(std::ostringstream& stream, const C784_ReadLinkList_Req * req)
{
    stream << "C784_ReadLinkList_Req  {";
    stream << "LinkIndex=" << std::dec << (int) req->m_unLinkIndex;
    stream << ", NoOfLinksToRead=" << std::dec << (int) req->m_ucNoOfLinksToRead;
    stream << "}";
}
inline void toString(std::ostringstream& stream, const C784_ReadLinkList_Resp * resp)
{
    stream << "C784_ReadLinkList_Resp {";
    stream << "LinkIndex=" << std::dec << (int) resp->m_unLinkIndex;
    stream << ", NoOfLinksRead=" << std::dec << (int) resp->m_ucNoOfLinksRead;
    stream << ", NoOfActiveLinks=" << std::dec << (int) resp->m_unNoOfActiveLinks;
    stream << ", Links {";

    for (int i = 0; i < resp->m_ucNoOfLinksRead; i++)
    {
        stream << " (superframeId=" << std::dec << (int) resp->m_aLinks[i].superframeId;
        stream << ", channelOffsetForThisLink=" << std::dec << (int) resp->m_aLinks[i].channelOffsetForThisLink;
        stream << ", slotNoForThisLink=" << std::dec << (int) resp->m_aLinks[i].slotNoForThisLink;
        stream << ", nicknameOfNeighborForThisLink=" << ToStr(resp->m_aLinks[i].nicknameOfNeighborForThisLink);
        stream << ", linkOptions=" << std::dec << (int) resp->m_aLinks[i].linkOptions;
        stream << ", linkType=" << std::dec << (int) resp->m_aLinks[i].linkType;
        stream << ") ";
    }

    stream << "}";
    stream << "}";
}

inline void toString(std::ostringstream& stream, const C785_ReadGraphList_Req * req)
{
    stream << "C785_ReadGraphList_Req  {";
    stream << "GraphListIndex=" << std::dec << (int) req->m_ucGraphListIndex;
    stream << "}";
}
inline void toString(std::ostringstream& stream, const C785_ReadGraphList_Resp * resp)
{
    stream << "C785_ReadGraphList_Resp {";
    stream << "GraphListIndex=" << std::dec << (int) resp->m_ucGraphListIndex;
    stream << ", TotalNoOfGraphs=" << std::dec << (int) resp->m_ucTotalNoOfGraphs;
    stream << ", GraphId=" << std::dec << (int) resp->m_unGraphId;
    stream << ", NoOfNeighbors=" << std::dec << (int) resp->m_ucNoOfNeighbors;
    stream << ", NicknameOfNeighbor {";

    for (int i = 0; i < resp->m_ucNoOfNeighbors; i++)
    {
        stream << " " << ToStr(resp->m_aNicknameOfNeighbor[i]);
    }

    stream << "}";
    stream << "}";
}

inline void toString(std::ostringstream& stream, const C786_ReadNeighborPropertyFlag_Req * req)
{
    stream << "C786_ReadNeighborPropertyFlag_Req  {";
    stream << "Nickname=" << ToStr(req->Nickname);
    stream << "}";
}
inline void toString(std::ostringstream& stream, const C786_ReadNeighborPropertyFlag_Resp * resp)
{
    stream << "C786_ReadNeighborPropertyFlag_Resp {";
    stream << "Nickname=" << ToStr(resp->Nickname);
    stream << ", NeighbourFlag=" << std::dec << (int) resp->NeighbourFlag;
    stream << "}";
}

inline void toString(std::ostringstream& stream, const C787_ReportNeighborSignalLevels_Req * req)
{
    stream << "C787_ReportNeighborSignalLevels_Req  {";
    stream << "NeighborTableIndex=" << std::dec << (int) req->m_ucNeighborTableIndex;
    stream << ", NoOfNeighborEntriesToRead=" << std::dec << (int) req->m_ucNoOfNeighborEntriesToRead;
    stream << "}";
}
inline void toString(std::ostringstream& stream, const C787_ReportNeighborSignalLevels_Resp * resp)
{
    stream << "C787_ReportNeighborSignalLevels_Resp {";
    stream << "NeighborTableIndex=" << std::dec << (int) resp->m_ucNeighborTableIndex;
    stream << ", NoOfNeighborEntriesRead=" << std::dec << (int) resp->m_ucNoOfNeighborEntriesRead;
    stream << ", TotalNoOfNeighbors=" << std::dec << (int) resp->m_ucTotalNoOfNeighbors;

    stream << ", m_aNeighbors = {";
    for (int i = 0; i < resp->m_ucNoOfNeighborEntriesRead; i++)
    {
        stream << "(RSLindB=" << std::dec << (int) resp->m_aNeighbors[i].RSLindB;
        stream << ", nickname=" << ToStr(resp->m_aNeighbors[i].nickname);
        stream << ") ";
    }

    stream << "}";
}

inline void toString788(std::ostringstream& stream, const C788_AlarmPathDown_Req * req)
{
    stream << "C788_AlarmPathDown_Req  {";
    stream << "}";
}
inline void toString(std::ostringstream& stream, const C788_AlarmPathDown_Resp * resp)
{
    stream << "C788_AlarmPathDown_Resp {";
    stream << "Nickname=" << ToStr(resp->Nickname);

    stream << "}";
}

inline void toStringC789(std::ostringstream& stream, const C789_AlarmSourceRouteFailed_Req * req)
{
    stream << "C789_AlarmSourceRouteFailed_Req  {";
    stream << "}";
}
inline void toString(std::ostringstream& stream, const C789_AlarmSourceRouteFailed_Resp * resp)
{
    stream << "C789_AlarmSourceRouteFailed_Resp {";
    stream << "NetworkLayerMICfromNPDUthatFailedRouting=" << std::dec
                << (int) resp->m_ulNetworkLayerMICfromNPDUthatFailedRouting;
    stream << ", NicknameOfUnreachableNeighbor=" << ToStr(resp->m_unNicknameOfUnreachableNeighbor);

    stream << "}";
}
inline void toStringC790(std::ostringstream& stream, const C790_AlarmGraphRouteFailed_Req * req)
{
    stream << "C790_AlarmGraphRouteFailed_Req  {";
    stream << "}";
}
inline void toString(std::ostringstream& stream, const C790_AlarmGraphRouteFailed_Resp * resp)
{
    stream << "C790_AlarmGraphRouteFailed_Resp {";
    stream << "GraphIdOfFailedRoute=" << std::dec << (int) resp->m_unGraphIdOfFailedRoute;

    stream << "}";
}

inline void toStringC791(std::ostringstream& stream, const C791_AlarmTransportLayerFailed_Req * req)
{
    stream << "C791_AlarmTransportLayerFailed_Req  {";
    stream << "}";
}
inline void toString(std::ostringstream& stream, const C791_AlarmTransportLayerFailed_Resp * resp)
{
    stream << "C791_AlarmTransportLayerFailed_Resp {";
    stream << "NicknameOfUnreachablePeer=" << ToStr(resp->m_unNicknameOfUnreachablePeer);

    stream << "}";
}

inline void toStringC794(std::ostringstream& stream, const C794_ReadUTCTime_Req * req)
{
    stream << "C794_ReadUTCTime_Req  {";
    stream << "}";
}
inline void toString(std::ostringstream& stream, const C794_ReadUTCTime_Resp * resp)
{
    stream << "C794_ReadUTCTime_Resp {";
    stream << "Date=" << std::dec << (int) resp->m_dDate.day << (int) resp->m_dDate.month << (int) resp->m_dDate.year;
    stream << ", Time=" << std::dec << (int) resp->m_tTime.u32;

    stream << "}";
}

inline void toString(std::ostringstream& stream, const C795_WriteTimerInterval_Req * req)
{
    stream << "C795_WriteTimerInterval_Req  {";
    stream << "TimerInterval=" << std::dec << (int) req->m_ulTimerInterval;
    stream << ", TimerType=" << std::dec << (int) req->m_ucTimerType;
    stream << "}";
}
inline void toString(std::ostringstream& stream, const C795_WriteTimerInterval_Resp * resp)
{
    stream << "C795_WriteTimerInterval_Resp {";
    stream << "TimerInterval=" << std::dec << (int) resp->m_ulTimerInterval;
    stream << ", TimerType=" << std::dec << (int) resp->m_ucTimerType;

    stream << "}";
}

inline void toString(std::ostringstream& stream, const C796_ReadTimerInterval_Req * req)
{
    stream << "C796_ReadTimerInterval_Req  {";
    stream << "TimerType=" << std::dec << (int) req->m_ucTimerType;
    stream << "}";
}
inline void toString(std::ostringstream& stream, const C796_ReadTimerInterval_Resp * resp)
{
    stream << "C796_ReadTimerInterval_Resp {";
    stream << "TimerInterval=" << std::dec << (int) resp->m_ulTimerInterval;
    stream << "TimerType=" << std::dec << (int) resp->m_ucTimerType;

    stream << "}";
}

inline void toString(std::ostringstream& stream, const C797_WriteRadioPower_Req * req)
{
    stream << "C797_WriteRadioPower_Req  {";
    stream << "outputPower=" << std::dec << (int) req->outputPower;
    stream << "}";
}
inline void toString(std::ostringstream& stream, const C797_WriteRadioPower_Resp * resp)
{
    stream << "C797_WriteRadioPower_Resp {";
    stream << "outputPower=" << std::dec << (int) resp->outputPower;

    stream << "}";
}

inline void toStringC798(std::ostringstream& stream, const C798_ReadRadioPower_Req * req)
{
    stream << "C798_ReadRadioPower_Req  {";
    stream << "}";
}
inline void toString(std::ostringstream& stream, const C798_ReadRadioPower_Resp * resp)
{
    stream << "C798_ReadRadioPower_Resp {";
    stream << "outputPower=" << std::dec << (int) resp->outputPower;

    stream << "}";
}

inline void toString(std::ostringstream& stream, const C799_RequestService_Req * req)
{
    stream << "C799_RequestService_Req  {";
    stream << "NicknameOfPeer=" << ToStr(req->m_unNicknameOfPeer);
    stream << ", Period=" << std::dec << (int) req->m_tPeriod.u32;
    stream << ", ServiceId=" << std::dec << (int) req->m_ucServiceId;
    stream << ", ServiceRequestFlags=" << std::dec << (int) req->m_ucServiceRequestFlags;
    stream << ", ServiceApplicationDomain=" << std::dec << (int) req->m_ucServiceApplicationDomain;

    stream << "}";
}
inline void toString(std::ostringstream& stream, const C799_RequestService_Resp * resp)
{
    stream << "C799_RequestService_Resp {";
    stream << "NicknameOfPeer=" << ToStr(resp->m_unNicknameOfPeer);
    stream << ", Period=" << std::dec << (int) resp->m_tPeriod.u32;
    stream << ", ServiceId=" << std::dec << (int) resp->m_ucServiceId;
    stream << ", RouteId=" << std::dec << (int) resp->m_ucRouteId;
    stream << ", ServiceRequestFlags=" << std::dec << (int) resp->m_ucServiceRequestFlags;
    stream << ", ServiceApplicationDomain=" << std::dec << (int) resp->m_ucServiceApplicationDomain;

    stream << "}";
}

inline void toString(std::ostringstream& stream, const C800_ReadServiceList_Req * req)
{
    stream << "C800_ReadServiceList_Req  {";
    stream << "ServiceIndex=" << std::dec << (int) req->m_ucServiceIndex;
    stream << ", NoOfEntriesToRead=" << std::dec << (int) req->m_ucNoOfEntriesToRead;
    stream << "}";
}
inline void toString(std::ostringstream& stream, const C800_ReadServiceList_Resp * resp)
{
    stream << "C800_ReadServiceList_Resp {";
    stream << "ServiceIndex=" << std::dec << (int) resp->m_ucServiceIndex;
    stream << ", NoOfEntriesRead=" << std::dec << (int) resp->m_ucNoOfEntriesRead;
    stream << ", NoOfActiveServices=" << std::dec << (int) resp->m_ucNoOfActiveServices;

    stream << ", m_aServices = {";
    for (int i = 0; i < resp->m_ucNoOfEntriesRead; i++)
    {
        stream << "(serviceRequestFlags=" << std::dec << (int) resp->m_aServices[i].serviceRequestFlags;
        stream << ", serviceApplicationDomain=" << std::dec << (int) resp->m_aServices[i].serviceApplicationDomain;
        stream << ", nicknameOfPeer=" << ToStr(resp->m_aServices[i].nicknameOfPeer);
        stream << ", period=" << std::dec << (int) resp->m_aServices[i].period.u32;
        stream << ", serviceId=" << std::dec << (int) resp->m_aServices[i].serviceId;
        stream << ", routeId=" << std::dec << (int) resp->m_aServices[i].routeId;
        stream << ") ";
    }

    stream << "}";
}

inline void toString(std::ostringstream& stream, const C801_DeleteService_Req * req)
{
    stream << "C801_DeleteService_Req  {";
    stream << "ServiceId=" << std::dec << (int) req->m_ucServiceId;
    stream << ", Reason=" << std::dec << (int) req->m_ucReason;
    stream << "}";
}
inline void toString(std::ostringstream& stream, const C801_DeleteService_Resp * resp)
{
    stream << "C801_DeleteService_Resp {";
    stream << "ServiceId=" << std::dec << (int) resp->m_ucServiceId;
    stream << ", Reason=" << std::dec << (int) resp->m_ucReason;
    stream << ", NoOfServiceEntriesRemaining=" << std::dec << (int) resp->m_ucNoOfServiceEntriesRemaining;

    stream << "}";
}

inline void toString(std::ostringstream& stream, const C802_ReadRouteList_Req * req)
{
    stream << "C802_ReadRouteList_Req  {";
    stream << "RouteIndex=" << std::dec << (int) req->m_ucRouteIndex;
    stream << ", NoOfEntriesToRead=" << std::dec << (int) req->m_ucNoOfEntriesToRead;
    stream << "}";
}
inline void toString(std::ostringstream& stream, const C802_ReadRouteList_Resp * resp)
{
    stream << "C802_ReadRouteList_Resp {";
    stream << "RouteIndex=" << std::dec << (int) resp->m_ucRouteIndex;
    stream << ", NoOfEntriesRead=" << std::dec << (int) resp->m_ucNoOfEntriesRead;
    stream << ", NoOfActiveRoutes=" << std::dec << (int) resp->m_ucNoOfActiveRoutes;
    stream << ", NoOfRoutesRemaining=" << std::dec << (int) resp->m_ucNoOfRoutesRemaining;

    stream << ", m_aRoutes = {";
    for (int i = 0; i < resp->m_ucNoOfEntriesRead; i++)
    {
        stream << "(routeId=" << std::dec << (int) resp->m_aRoutes[i].routeId;
        stream << ", sourceRouteAttached=" << std::dec << (int) resp->m_aRoutes[i].sourceRouteAttached;
        stream << ", destinationNickname=" << ToStr(resp->m_aRoutes[i].destinationNickname);
        stream << ", graphId=" << std::dec << (int) resp->m_aRoutes[i].graphId;
        stream << ") ";
    }

    stream << "}";
}

inline void toString(std::ostringstream& stream, const C803_ReadSourceRoute_Req* req)
{
    stream << "C803_ReadSourceRoute_Req  {";
    stream << "RouteId=" << std::dec << (int) req->m_ucRouteId;
    stream << "}";
}
inline void toString(std::ostringstream& stream, const C803_ReadSourceRoute_Resp * resp)
{
    stream << "C803_ReadSourceRoute_Resp {";
    stream << "RouteId=" << std::dec << (int) resp->m_ucRouteId;
    stream << ", NoOfHops=" << std::dec << (int) resp->m_ucNoOfHops;
    stream << ", HopNicknames = {";
    for (int i = 0; i < resp->m_ucNoOfHops; i++)
    {
        stream << " " << ToStr(resp->m_aHopNicknames[i]);
    }

    stream << " } }";
}

inline void toString(std::ostringstream& stream, const C805_WriteRadioCCAMode_Req * req)
{
    stream << "C805_WriteRadioCCAMode_Req  {";
    stream << "CCAMode=" << std::dec << (int) req->CCAMode;

    stream << "}";
}
inline void toString(std::ostringstream& stream, const C805_WriteRadioCCAMode_Resp * resp)
{
    stream << "C805_WriteRadioCCAMode_Resp {";
    stream << "CCAMode=" << std::dec << (int) resp->CCAMode;

    stream << "}";
}

inline void toStringC806(std::ostringstream& stream, const C806_ReadHandheldSuperframe_Req * req)
{
    stream << "C806_ReadHandheldSuperframe_Req  {";

    stream << "}";
}
inline void toString(std::ostringstream& stream, const C806_ReadHandheldSuperframe_Resp * resp)
{
    stream << "C806_ReadHandheldSuperframe_Resp {";
    stream << "SuperframeId=" << std::dec << (int) resp->m_ucSuperframeId;
    stream << ", SuperframeModeFlags=" << std::dec << (int) resp->m_ucSuperframeModeFlags;
    stream << ", NoOfSlotsInSuperframe=" << std::dec << (int) resp->m_unNoOfSlotsInSuperframe;

    stream << "}";
}

inline void toStringC808(std::ostringstream& stream, const C808_ReadTimeToLive_Req * req)
{
    stream << "C808_ReadTimeToLive_Req  {";

    stream << "}";
}
inline void toString(std::ostringstream& stream, const C808_ReadTimeToLive_Resp * resp)
{
    stream << "C808_ReadTimeToLive_Resp {";
    stream << "TimeToLive=" << std::dec << (int) resp->m_ucTimeToLive;

    stream << "}";
}

inline void toString(std::ostringstream& stream, const C809_WriteTimeToLive_Req * req)
{
    stream << "C809_WriteTimeToLive_Req  {";
    stream << "TimeToLive=" << std::dec << (int) req->m_ucTimeToLive;

    stream << "}";
}
inline void toString(std::ostringstream& stream, const C809_WriteTimeToLive_Resp * resp)
{
    stream << "C809_WriteTimeToLive_Resp {";
    stream << "TimeToLiveSet=" << std::dec << (int) resp->m_ucTimeToLiveSet;

    stream << "}";
}

inline void toString(std::ostringstream& stream, const C811_WriteJoinPriority_Req * req)
{
    stream << "C811_WriteJoinPriority_Req  {";
    stream << "JoinPriority=" << std::dec << (int) req->JoinPriority;

    stream << "}";
}
inline void toString(std::ostringstream& stream, const C811_WriteJoinPriority_Resp * resp)
{
    stream << "C811_WriteJoinPriority_Resp {";
    stream << "JoinPriority=" << std::dec << (int) resp->JoinPriority;

    stream << "}";
}

inline void toString(std::ostringstream& stream, const C813_WritePacketReceivePriority_Req * req)
{
    stream << "C813_WritePacketReceivePriority_Req  {";
    stream << "PacketRecPriority=" << std::dec << (int) req->PacketRecPriority;

    stream << "}";
}
inline void toString(std::ostringstream& stream, const C813_WritePacketReceivePriority_Resp * resp)
{
    stream << "C813_WritePacketReceivePriority_Resp {";
    stream << "PacketRecPriority=" << std::dec << (int) resp->PacketRecPriority;

    stream << "}";
}

inline void toString(std::ostringstream& stream, const C814_ReadDeviceListEntries_Req * req)
{
    stream << "C814_ReadDeviceListEntries_Req  {";
    stream << "DeviceListCode=" << std::dec << (int) req->m_ucDeviceListCode;
    stream << ", NoOfListEntriesToRead=" << std::dec << (int) req->m_ucNoOfListEntriesToRead;
    stream << ", StartingListIndex=" << std::dec << (int) req->m_unStartingListIndex;

    stream << "}";
}
inline void toString(std::ostringstream& stream, const C814_ReadDeviceListEntries_Resp * resp)
{
    stream << "C814_ReadDeviceListEntries_Resp {";
    stream << "DeviceListCode=" << std::dec << (int) resp->m_ucDeviceListCode;
    stream << ", NoOfListEntriesRead=" << std::dec << (int) resp->m_ucNoOfListEntriesRead;
    stream << ", StartingListIndex=" << std::dec << (int) resp->m_unStartingListIndex;
    stream << ", TotalNoOfEntriesInList=" << std::dec << (int) resp->m_unTotalNoOfEntriesInList;

    stream << ", m_aDeviceUniqueIds {";
    for (int i = 0; i < resp->m_ucNoOfListEntriesRead; i++)
    {
        stream << " " << array2string(resp->m_aDeviceUniqueIds[i], 5);
    }

    stream << "}";
    stream << "}";
}

inline void toStringC817(std::ostringstream& stream, const C817_ReadChannelBlacklist_Req* req)
{
    stream << "C817_ReadChannelBlacklist_Req  {";
    stream << "}";
}
inline void toString(std::ostringstream& stream, const C817_ReadChannelBlacklist_Resp* resp)
{
    stream << "C817_ReadChannelBlacklist_Req  {";
    stream << "NoOfBitsInCurrentChannelMapArray=0x" << ToStr(resp->m_ucNoOfBitsInCurrentChannelMapArray);
    stream << ", CurrentChannelMapArray=0x" << ToStr(resp->m_unCurrentChannelMapArray);
    stream << ", PendingChannelMapArray=" << std::dec << (int) resp->m_unPendingChannelMapArray;

    stream << "}";
    stream << "}";
}

inline void toString(std::ostringstream& stream, const C818_WriteChannelBlacklist_Req * req)
{
    stream << "C818_WriteChannelBlacklist_Req  {";
    stream << "NoOfBitsInNewChannelMapArray=" << std::dec << (int) req->m_ucNoOfBitsInNewChannelMapArray;
    stream << ", PendingChannelMapArray=0x" << ToStr(req->m_unPendingChannelMapArray);

    stream << "}";
}
inline void toString(std::ostringstream& stream, const C818_WriteChannelBlacklist_Resp * resp)
{
    stream << "C818_WriteChannelBlacklist_Resp {";
    stream << "NoOfBitsInNewChannelMapArray=" << std::dec << (int) resp->m_ucNoOfBitsInNewChannelMapArray;
    stream << ", PendingChannelMapArray=0x" << ToStr(resp->m_unPendingChannelMapArray);

    stream << "}";
    stream << "}";
}

inline void toString(std::ostringstream& stream, const C820_WriteBackOffExponent_Req * req)
{
    stream << "C820_WriteBackOffExponent_Req  {";
    stream << "MaxBackOffExp=" << std::dec << (int) req->MaxBackOffExp;

    stream << "}";
}
inline void toString(std::ostringstream& stream, const C820_WriteBackOffExponent_Resp * resp)
{
    stream << "C820_WriteBackOffExponent_Resp {";
    stream << "MaxBackOffExp=" << std::dec << (int) resp->MaxBackOffExp;

    stream << "}";
}

inline void toString(std::ostringstream& stream, const C823_RequestSession_Req * req)
{
    stream << "C823_RequestSession_Req  {";
    stream << "PeerDeviceNickname=" << ToStr(req->m_unPeerDeviceNickname);

    stream << "}";
}
inline void toString(std::ostringstream& stream, const C823_RequestSession_Resp * resp)
{
    stream << "C823_RequestSession_Resp {";
    stream << "KeyVal=" << array2string(resp->m_aKeyVal, 16);
    stream << ", PeerDeviceNonceCounterVal=" << std::dec << (int) resp->m_ulPeerDeviceNonceCounterVal;
    stream << ", PeerDeviceNickname=" << ToStr(resp->m_unPeerDeviceNickname);

    stream << "}";
}

inline void toString(std::ostringstream& stream, const C832_ReadNetworkDeviceIdentity_Req * req)
{
    stream << "C832_ReadNetworkDeviceIdentity_Req  {";
    stream << "DeviceUniqueID=" << array2string(req->DeviceUniqueID, 5);

    stream << "}";
}
inline void toString(std::ostringstream& stream, const C832_ReadNetworkDeviceIdentity_Resp * resp)
{
    stream << "C832_ReadNetworkDeviceIdentity_Resp {";
    stream << "DeviceUniqueID=" << array2string(resp->DeviceUniqueID, 5);
    stream << ", Nickname=" << ToStr(resp->Nickname);
    stream << ", LongTag=" << array2string((uint8_t*) resp->LongTag, 32);

    stream << "}";
}

inline void toString(std::ostringstream& stream, const C833_ReadNetworkDeviceNeighbourHealth_Req* req)
{
    stream << "C833_ReadNetworkDeviceNeighbourHealth_Req  {";
    stream << "UniqueID=" << array2string(req->UniqueID, 5);
    stream << ", NeighbourIndex=" << std::dec << (int) req->NeighbourIndex;
    stream << ", NeighbourEntriesToRead=" << (int) req->NeighbourEntriesToRead;
    stream << "}";
}
inline void toString(std::ostringstream& stream, const C833_ReadNetworkDeviceNeighbourHealth_Resp* resp)
{
    stream << "C833_ReadNetworkDeviceNeighbourHealth_Resp {";
    stream << "UniqueID=" << array2string(resp->UniqueID, 5);
    stream << ", NeighbourIndex=" << std::dec << (int) resp->NeighbourIndex;
    stream << ", NeighbourCount=" << (int) resp->NeighbourCount;

    stream << ", Neighbours {";
    for (int i = 0; i < resp->NeighbourCount; i++)
    {
        stream << "(" << ToStr(resp->Neighbours[i].NeighbourNickname);
        stream << " " << std::dec << (int) resp->Neighbours[i].NeighbourRSL;
        stream << " " << std::dec << (int) resp->Neighbours[i].TransmittedPacketCount;
        stream << " " << std::dec << (int) resp->Neighbours[i].TransmittedPacketWithNoACKCount;
        stream << " " << std::dec << (int) resp->Neighbours[i].ReceivedPacketCount;
        stream << ") ";
    }

    stream << "}";
    stream << "}";
}

inline void toString(std::ostringstream& stream, const C834_ReadNetworkTopologyInformation_Req* req)
{
    stream << "C834_ReadNetworkTopologyInformation_Req  {";
    stream << "UniqueID=" << array2string(req->DeviceLongAddress, 5);
    stream << ", GraphIndexNo=" << std::dec << (int) req->GraphIndexNo;

    stream << "}";
}

inline void toString(std::ostringstream& stream, const C834_ReadNetworkTopologyInformation_Resp* resp)
{
    stream << "C834_ReadNetworkTopologyInformation_Resp {";
    stream << "UniqueID=" << array2string(resp->DeviceLongAddress, 5);
    stream << ", GraphIndexNo=" << std::dec << (int) resp->GraphIndexNo;
    stream << ", TotalGraphsNo=" << std::dec << (int) resp->TotalGraphsNo;
    stream << ", IndexGraphId=" << std::dec << (int) resp->IndexGraphId;
    stream << ", NeighboursNo=" << std::dec << (int) resp->NeighboursNo;
    stream << ", Neighbours : ";
    for (int i = 0; i < resp->NeighboursNo; i++)
    {
        stream << " " << ToStr(resp->Neighbour[i]);
    }

    stream << "}";
}

inline void toString(std::ostringstream& stream, const C839_ChangeNotification_Req* req)
{
    stream << "C839_ChangeNotification_Req  {";
    stream << "DeviceAddress=" << array2string(req->DeviceAddress, 5);

    stream << "}";
}
inline void toString(std::ostringstream& stream, const C839_ChangeNotification_Resp* resp)
{
    stream << "C839_ChangeNotification_Resp {";
    stream << "DeviceAddress=" << array2string(resp->DeviceAddress, 5);
    stream << ", ChangeNotificationNo=" << std::dec << (int) resp->ChangeNotificationNo;
    stream << ", ChangeNotifications : ";
    for (int i = 0; i < resp->ChangeNotificationNo; i++)
    {
        stream << " " << std::dec << resp->ChangeNotifications[i];
    }

    stream << "}";
}

inline void toString(std::ostringstream& stream, const C840_ReadDeviceStatistics_Req* req)
{
    stream << "C840_ReadDeviceStatistics_Req  {";
    stream << "UniqueID=" << array2string(req->UniqueID, 5);

    stream << "}";
}
inline void toString(std::ostringstream& stream, const C840_ReadDeviceStatistics_Resp* resp)
{
    stream << "C840_ReadDeviceStatistics_Resp {";
    stream << "UniqueID=" << array2string(resp->UniqueID, 5);
    stream << ", ActiveGraphsNo=" << (int) resp->ActiveGraphsNo;
    stream << ", ActiveFramesNo=" << (int) resp->ActiveFramesNo;
    stream << ", ActiveLinksNo=" << (int) resp->ActiveLinksNo;
    stream << ", NeighboursNo=" << (int) resp->NeighboursNo;
    stream << ", AverageLatency=" << (int) resp->AverageLatency.u32;
    stream << ", JoinCount=" << (int) resp->JoinCount;
    stream << ", LastJoinTime=" << (int) resp->LastJoinTime.day;
    stream << " " << (int) resp->LastJoinTime.month;
    stream << " " << (int) resp->LastJoinTime.year;
    stream << ", LastJoinTimeToday=" << (int) resp->LastJoinTimeToday;

    stream << "}";
}

inline void toString(std::ostringstream& stream, const C960_DisconnectDevice_Req* req)
{
    stream << "C960_DisconnectDevice_Req  {";
    stream << "Reason=" << std::dec << (int) req->m_eReason;

    stream << "}";
}
inline void toString(std::ostringstream& stream, const C960_DisconnectDevice_Resp* resp)
{
    stream << "C960_DisconnectDevice_Resp {";
    stream << "Reason=" << (int) resp->m_eReason;

    stream << "}";
}

inline void toString(std::ostringstream& stream, const C961_WriteNetworkKey_Req* req)
{
    stream << "C961_WriteNetworkKey_Req  {";
    stream << "ExecutionTime=" << bytes2string(req->m_tExecutionTime);
    stream << ", KeyValue =" << array2string(req->m_aKeyValue, 16);
    stream << ", Truncated =" << (int) req->m_ucTruncated;
    stream << "}";
}
inline void toString(std::ostringstream& stream, const C961_WriteNetworkKey_Resp* resp)
{
    stream << "C961_WriteNetworkKey_Resp {";
    stream << "ExecutionTime=" << bytes2string(resp->m_tExecutionTime);
    stream << ", KeyValue =" << array2string(resp->m_aKeyValue, 16);
    stream << ", Truncated =" << (int) resp->m_ucTruncated;

    stream << "}";
}

inline void toString(std::ostringstream& stream, const C962_WriteDeviceNicknameAddress_Req* req)
{
    stream << "C962_WriteDeviceNicknameAddress_Req  {";
    stream << "Nickname=" << ToStr(req->m_unNickname);
    stream << "}";
}
inline void toString(std::ostringstream& stream, const C962_WriteDeviceNicknameAddress_Resp* resp)
{
    stream << "C962_WriteDeviceNicknameAddress_Resp {";
    stream << "Nickname=" << ToStr(resp->m_unNickname);

    stream << "}";
}

inline void toString(std::ostringstream& stream, const C963_WriteSession_Req* req)
{
    stream << "C963_WriteSession_Req  {";
    ;
    stream << "PeerNonceCounterValue=" << std::dec << (int) req->m_ulPeerNonceCounterValue;
    stream << ", PeerUniqueID=" << std::hex << array2string(req->m_aPeerUniqueID, 5);
    stream << ", PeerNickname=" << ToStr(req->m_unPeerNickname);
    stream << ", KeyValue=" << array2string(req->m_aKeyValue, 16);
    stream << ", SessionType=" << std::dec << (int) req->m_eSessionType;
    stream << ", Reserved=" << std::dec << (int) req->m_ucReserved;
    stream << ", Truncated=" << std::dec << (int) req->m_ucTruncated;
    stream << ", ExecutionTime=" << array2string(req->m_tExecutionTime, 5);

    stream << "}";
}
inline void toString(std::ostringstream& stream, const C963_WriteSession_Resp* resp)
{
    stream << "C963_WriteSession_Resp {";
    stream << "PeerNonceCounterValue=" << std::dec << (int) resp->m_ulPeerNonceCounterValue;
    stream << ", PeerDeviceID=" << std::hex << resp->m_ulPeerDeviceID;
    stream << ", PeerNickname=" << ToStr(resp->m_unPeerNickname);
    stream << ", PeerExpandedDeviceTypeCode=" << std::dec << (int) resp->m_unPeerExpandedDeviceTypeCode;
    stream << ", KeyValue=" << array2string(resp->m_aKeyValue, 16);
    stream << ", SessionType=" << std::dec << (int) resp->m_eSessionType;
    stream << ", RemainingSessionsNo=" << std::dec << (int) resp->m_ucRemainingSessionsNo;
    stream << ", Truncated=" << std::dec << (int) resp->m_ucTruncated;
    stream << ", ExecutionTime=" << array2string(resp->m_tExecutionTime, 5);

    stream << "}";
}

inline void toString(std::ostringstream& stream, const C964_DeleteSession_Req* req)
{
    stream << "C964_DeleteSession_Req  {";
    stream << "PeerNickname=" << ToStr(req->m_unPeerNickname);
    stream << ", SessionType=" << std::dec << (int) req->m_eSessionType;

    stream << "}";
}
inline void toString(std::ostringstream& stream, const C964_DeleteSession_Resp* resp)
{
    stream << "C964_DeleteSession_Resp {";
    stream << "PeerNickname=" << ToStr(resp->m_unPeerNickname);
    stream << ", SessionType=" << std::dec << (int) resp->m_eSessionType;
    stream << ", RemainingSessionsNo=" << std::dec << (int) resp->m_ucRemainingSessionsNo;

    stream << "}";
}

inline void toString(std::ostringstream& stream, const C965_WriteSuperframe_Req* req)
{
    stream << "C965_WriteSuperframe_Req  {";
    stream << "SuperframeSlotsNo=" << std::dec << (int) req->m_unSuperframeSlotsNo;
    stream << ", SuperframeID=" << std::dec << (int) req->m_ucSuperframeID;
    stream << ", SuperframeMode=" << std::dec << (int) req->m_ucSuperframeMode;
    stream << ", Reserved=" << std::dec << (int) req->m_ucReserved;
    stream << ", Truncated=" << std::dec << (int) req->m_ucTruncated;
    stream << ", ExecutionTime=" << array2string(req->m_tExecutionTime, 5);

    stream << "}";
}
inline void toString(std::ostringstream& stream, const C965_WriteSuperframe_Resp* resp)
{
    stream << "C965_WriteSuperframe_Resp {";
    stream << "SuperframeSlotsNo=" << std::dec << (int) resp->m_unSuperframeSlotsNo;
    stream << ", SuperframeID=" << std::dec << (int) resp->m_ucSuperframeID;
    stream << ", SuperframeMode=" << std::dec << (int) resp->m_ucSuperframeMode;
    stream << ", Truncated=" << std::dec << (int) resp->m_ucTruncated;
    stream << ", ExecutionTime=" << array2string(resp->m_tExecutionTime, 5);

    stream << "}";
}

inline void toString(std::ostringstream& stream, const C966_DeleteSuperframe_Req* req)
{
    stream << "C966_DeleteSuperframe_Req  {";
    stream << "SuperframeID" << std::dec << (int) req->m_ucSuperframeID;

    stream << "}";
}
inline void toString(std::ostringstream& stream, const C966_DeleteSuperframe_Resp* resp)
{
    stream << "C966_DeleteSuperframe_Resp {";
    stream << "SuperframeID" << std::dec << (int) resp->m_ucSuperframeID;
    stream << ", RemainingSuperframeEntriesNo" << std::dec << (int) resp->m_ucRemainingSuperframeEntriesNo;

    stream << "}";
}

inline void toString(std::ostringstream& stream, const C967_WriteLink_Req* req)
{
    stream << "C967_WriteLink_Req  {";
    stream << "SlotNumber=" << std::dec << (int) req->m_unSlotNumber;
    stream << ", NeighborNickname=" << ToStr(req->m_unNeighborNickname);
    stream << ", SuperframeID=" << std::dec << (int) req->m_ucSuperframeID;
    stream << ", ChannelOffset=" << std::dec << (int) req->m_ucChannelOffset;
    stream << ", LinkOptions=" << std::dec << (int) req->m_ucLinkOptions;
    stream << ", LinkType=" << std::dec << (int) req->m_eLinkType;

    stream << "}";
}
inline void toString(std::ostringstream& stream, const C967_WriteLink_Resp* resp)
{
    stream << "C967_WriteLink_Resp {";
    stream << "SlotNumber=" << std::dec << (int) resp->m_unSlotNumber;
    stream << ", NeighborNickname=" << ToStr(resp->m_unNeighborNickname);
    stream << ", SuperframeID=" << std::dec << (int) resp->m_ucSuperframeID;
    stream << ", ChannelOffset=" << std::dec << (int) resp->m_ucChannelOffset;
    stream << ", LinkOptions=" << std::dec << (int) resp->m_ucLinkOptions;
    stream << ", LinkType=" << std::dec << (int) resp->m_eLinkType;
    stream << ", RemainingLinksNo=" << std::dec << (int) resp->m_unRemainingLinksNo;

    stream << "}";
}

inline void toString(std::ostringstream& stream, const C968_DeleteLink_Req* req)
{
    stream << "C968_DeleteLink_Req  {";
    stream << "SlotNumber=" << std::dec << (int) req->m_unSlotNumber;
    stream << ", NeighborNickname=" << ToStr(req->m_unNeighborNickname);
    stream << ", SuperframeID=" << std::dec << (int) req->m_ucSuperframeID;

    stream << "}";
}
inline void toString(std::ostringstream& stream, const C968_DeleteLink_Resp* resp)
{
    stream << "C968_DeleteLink_Resp {";
    stream << "SlotNumber=" << std::dec << (int) resp->m_unSlotNumber;
    stream << ", NeighborNickname=" << ToStr(resp->m_unNeighborNickname);
    stream << ", RemainingLinksNo=" << std::dec << (int) resp->m_unRemainingLinksNo;
    stream << ", SuperframeID=" << std::dec << (int) resp->m_ucSuperframeID;

    stream << "}";
}

inline void toString(std::ostringstream& stream, const C969_WriteGraphNeighbourPair_Req* req)
{
    stream << "C969_WriteGraphNeighbourPair_Req  {";
    stream << "GraphID=" << std::dec << (int) req->m_unGraphID;
    stream << ", NeighborNickname=" << ToStr(req->m_unNeighborNickname);

    stream << "}";
}
inline void toString(std::ostringstream& stream, const C969_WriteGraphNeighbourPair_Resp* resp)
{
    stream << "C969_WriteGraphNeighbourPair_Resp {";
    stream << "GraphID=" << std::dec << (int) resp->m_unGraphID;
    stream << ", NeighborNickname=" << ToStr(resp->m_unNeighborNickname);
    stream << ", RemainingConnectionsNo=" << std::dec << (int) resp->m_ucRemainingConnectionsNo;

    stream << "}";
}

inline void toString(std::ostringstream& stream, const C970_DeleteGraphConnection_Req* req)
{
    stream << "C970_DeleteGraphConnection_Req  {";
    stream << "GraphID=" << std::dec << (int) req->m_unGraphID;
    stream << ", NeighborNickname=" << ToStr(req->m_unNeighborNickname);

    stream << "}";
}
inline void toString(std::ostringstream& stream, const C970_DeleteGraphConnection_Resp* resp)
{
    stream << "C970_DeleteGraphConnection_Resp {";
    stream << "GraphID=" << std::dec << (int) resp->m_unGraphID;
    stream << ", NeighborNickname=" << ToStr(resp->m_unNeighborNickname);
    stream << ", RemainingConnectionsNo=" << std::dec << (int) resp->m_ucRemainingConnectionsNo;

    stream << "}";
}

inline void toString(std::ostringstream& stream, const C971_WriteNeighbourPropertyFlag_Req* req)
{
    stream << "C971_WriteNeighbourPropertyFlag_Req  {";
    stream << " NeighborNickname=" << ToStr(req->m_unNeighborNickname);
    stream << ", NeighborFlags=" << std::dec << (int) req->m_ucNeighborFlags;

    stream << "}";
}
inline void toString(std::ostringstream& stream, const C971_WriteNeighbourPropertyFlag_Resp* resp)
{
    stream << "C971_WriteNeighbourPropertyFlag_Resp {";
    stream << " NeighborNickname=" << ToStr(resp->m_unNeighborNickname);
    stream << ", NeighborFlags=" << std::dec << (int) resp->m_ucNeighborFlags;

    stream << "}";
}

inline void toString(std::ostringstream& stream, const C972_SuspendDevices_Req* req)
{
    stream << "C972_SuspendDevices_Req  {";
    stream << "TimeToSuspend=" << bytes2string(req->m_tTimeToSuspend);
    stream << ", TimeToResume=" << bytes2string(req->m_tTimeToResume);

    stream << "}";
}
inline void toString(std::ostringstream& stream, const C972_SuspendDevices_Resp* resp)
{
    stream << "C972_SuspendDevices_Resp {";
    stream << "TimeToSuspend=" << bytes2string(resp->m_tTimeToSuspend);
    stream << ", TimeToResume=" << bytes2string(resp->m_tTimeToResume);

    stream << "}";
}

inline void toString(std::ostringstream& stream, const C973_WriteService_Req* req)
{
    stream << "C973_WriteService_Req  {";

    stream << "RequestFlags=" << std::dec << (int) req->m_ucRequestFlags;
    stream << ", ApplicationDomain=" << std::dec << (int) req->m_eApplicationDomain;
    stream << ", PeerNickname=" << ToStr(req->m_unPeerNickname);
    stream << ", Period=" << std::dec << (int) req->m_tPeriod.u32;
    stream << ", ServiceID=" << std::dec << (int) req->m_ucServiceID;
    stream << ", RouteID=" << std::dec << (int) req->m_ucRouteID;

    stream << "}";
}
inline void toString(std::ostringstream& stream, const C973_WriteService_Resp* resp)
{
    stream << "C973_WriteService_Resp {";
    stream << "RequestFlags=" << std::dec << (int) resp->m_ucRequestFlags;
    stream << ", ApplicationDomain=" << std::dec << (int) resp->m_eApplicationDomain;
    stream << ", PeerNickname=" << ToStr(resp->m_unPeerNickname);
    stream << ", Period=" << std::dec << (int) resp->m_tPeriod.u32;
    stream << ", ServiceID=" << std::dec << (int) resp->m_ucServiceID;
    stream << ", RouteID=" << std::dec << (int) resp->m_ucRouteID;
    stream << ", RemainingServicesNo=" << std::dec << (int) resp->m_ucRemainingServicesNo;

    stream << "}";
}

inline void toString(std::ostringstream& stream, const C974_WriteRoute_Req* req)
{
    stream << "C974_WriteRoute_Req  {";
    stream << "PeerNickname=" << ToStr(req->m_unPeerNickname);
    stream << ", GraphID=" << std::dec << (int) req->m_unGraphID;
    stream << ", RouteID=" << std::dec << (int) req->m_ucRouteID;

    stream << "}";
}
inline void toString(std::ostringstream& stream, const C974_WriteRoute_Resp* resp)
{
    stream << "C974_WriteRoute_Resp {";
    stream << "PeerNickname=" << ToStr(resp->m_unPeerNickname);
    stream << ", GraphID=" << std::dec << (int) resp->m_unGraphID;
    stream << ", RouteID=" << std::dec << (int) resp->m_ucRouteID;
    stream << ", RemainingRoutesNo=" << std::dec << (int) resp->m_ucRemainingRoutesNo;

    stream << "}";
}

inline void toString(std::ostringstream& stream, const C975_DeleteRoute_Req* req)
{
    stream << "C975_DeleteRoute_Req  {";
    stream << "RouteID=" << std::dec << (int) req->m_ucRouteID;

    stream << "}";
}
inline void toString(std::ostringstream& stream, const C975_DeleteRoute_Resp* resp)
{
    stream << "C975_DeleteRoute_Resp {";
    stream << "RouteID=" << std::dec << (int) resp->m_ucRouteID;
    stream << ", RemainingRoutesNo=" << std::dec << (int) resp->m_ucRemainingRoutesNo;

    stream << "}";
}

inline void toString(std::ostringstream& stream, const C976_WriteSourceRoute_Req* req)
{
    stream << "C976_WriteSourceRoute_Req  {";
    stream << "RouteID=" << std::dec << (int) req->m_ucRouteID;
    stream << ", HopsNo=" << std::dec << (int) req->m_ucHopsNo;
    stream << ", NicknameHopEntries {";
    for (int i = 0; i < req->m_ucHopsNo; ++i)
    {
        stream << " =" << ToStr(req->m_aNicknameHopEntries[i]);
    }

    stream << "}";
}
inline void toString(std::ostringstream& stream, const C976_WriteSourceRoute_Resp* resp)
{
    stream << "C976_WriteSourceRoute_Resp {";
    stream << "RouteID=" << std::dec << (int) resp->m_ucRouteID;
    stream << ", HopsNo=" << std::dec << (int) resp->m_ucHopsNo;
    stream << ", NicknameHopEntries {";
    for (int i = 0; i < resp->m_ucHopsNo; ++i)
    {
        stream << " =" << ToStr(resp->m_aNicknameHopEntries[i]);
    }
    stream << ", RemainingRoutesNo=" << std::dec << (int) resp->m_ucRemainingSourceRoutesNo;

    stream << "}";
}

inline void toString(std::ostringstream& stream, const C977_DeleteSourceRoute_Req* req)
{
    stream << "C977_DeleteSourceRoute_Req  {";
    stream << "RouteID=" << std::dec << (int) req->m_ucRouteID;

    stream << "}";
}
inline void toString(std::ostringstream& stream, const C977_DeleteSourceRoute_Resp* resp)
{
    stream << "C977_DeleteSourceRoute_Resp {";
    stream << "RouteID=" << std::dec << (int) resp->m_ucRouteID;

    stream << "}";
}

inline void toString(std::ostringstream& stream, const C64765_NivisMetaCommand_Req* req)
{
    stream << "C64765_NivisMetaCommand_Req  {";
    stream << "Nickname=" << ToStr(req->Nickname);
    stream << ", DeviceUniqueId=" << array2string(req->DeviceUniqueId, 5);
    stream << ", CommandSize=" << std::dec << (int) req->CommandSize;
    stream << ", Command=" << array2string(req->Command, req->CommandSize);

    stream << "}";
}

inline void toString(std::ostringstream& stream, const C64765_NivisMetaCommand_Resp* resp)
{
    stream << "C64765_NivisMetaCommand_Resp {";
    stream << "Nickname=" << ToStr(resp->Nickname);
    stream << ", DeviceUniqueId=" << array2string(resp->DeviceUniqueId, 5);
    stream << ", CommandSize=" << std::dec << (int) resp->CommandSize;
    stream << ", Command=" << array2string(resp->Command, resp->CommandSize);

    stream << "}";
}

}

}

#endif /* COMMANDSTOSTRING_H_ */
