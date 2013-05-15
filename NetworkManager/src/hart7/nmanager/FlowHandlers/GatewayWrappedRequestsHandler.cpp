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

#include "GatewayWrappedRequestsHandler.h"

#include "Misc/Convert/Convert.h"
#include "Model/Device.h"
#include "Model/Tdma/TdmaTypes.h"
#include "../operations/WHOperationQueue.h"
#include "../../util/ManagerUtils.h"
#include "IDefaultCommandHandler.h"
#include <ApplicationLayer/ApplicationCommand.h>
#include <WHartStack/WHartNetworkData.h>
#include <hart7/util/NMLog.h>
#include <hart7/util/CommandsToString.h>
#include <time.h>

namespace hart7 {

namespace nmanager {

GatewayWrappedRequestsHandler::GatewayWrappedRequestsHandler(hart7::nmanager::CommonData& commonData_) :
    commonData(commonData_)
{
}

GatewayWrappedRequestsHandler::~GatewayWrappedRequestsHandler()
{
}

const static uint16_t SUBAPP_MAX_COMMANDS_COUNT = 25;
const static uint16_t SUBAPP_MAX_COMMANDS_BUFFER_SIZE = 10000;

void GatewayWrappedRequestsHandler::determineNoGraphs(Address32 address32, uint16_t& graphsNo, uint8_t& neighborsNo)
{
    std::set<Uint16> nodeGraphs;
    SubnetTopology& subnet = commonData.networkEngine.getSubnetTopology();
    Node& node = subnet.getNode(address32);

    EdgeList& edgeList = node.getOutBoundEdges();
    for (EdgeList::iterator itEdge = edgeList.begin(); itEdge != edgeList.end(); ++itEdge)
    {
        GraphNeighborMap& graphs = itEdge->getGraphs();
        for (GraphNeighborMap::iterator itGraph = graphs.begin(); itGraph != graphs.end(); ++itGraph)
        {
            nodeGraphs.insert(itGraph->first);
        }
    }

    graphsNo = (uint16_t) nodeGraphs.size();
    neighborsNo = (uint8_t) edgeList.size();
}

void GatewayWrappedRequestsHandler::ComposeCommand(uint16_t commandID, uint8_t responseCode, void* command,
                                                   C64765_NivisMetaCommand_Resp& resp)
{
    WHartCommand commandsArray[] = { { /*commands[0].*/commandID, responseCode, command } };
    WHartCommandList list = { 1, commandsArray };
    uint16_t writtenBytes = 0;
    commonData.stack.subapp.ComposePayload(resp.Command, sizeof(resp.Command) / sizeof(resp.Command[0]), writtenBytes,
                                           list, true);
    resp.CommandSize = (uint8_t) writtenBytes;
}

uint8_t GatewayWrappedRequestsHandler::Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                                              const C000_ReadUniqueIdentifier_Req& readUniqueIdentifier,
                                              C000_ReadUniqueIdentifier_Resp& resp)
{
    LOG_TRACE("C000_ReadUniqueIdentifier_Req");
    try
    {
        Address64 address64 = hart7::util::getAddress64FromUniqueId(DeviceUniqueId);
        Address32 address32 = commonData.networkEngine.getAddress32(address64);
        Device& device = commonData.networkEngine.getDevice(address32);

        resp.expandedDeviceType = device.capabilities.readUniqueIdentifier.expandedDeviceType;
        resp.minReqPreamblesNo = device.capabilities.readUniqueIdentifier.protocolMajorRevNo;
        resp.protocolMajorRevNo = device.capabilities.readUniqueIdentifier.protocolMajorRevNo;
        resp.deviceRevisionLevel = device.capabilities.readUniqueIdentifier.softwareRevisionLevel;
        resp.softwareRevisionLevel = device.capabilities.readUniqueIdentifier.hardwareRevisionLevel;
        resp.hardwareRevisionLevel = device.capabilities.readUniqueIdentifier.hardwareRevisionLevel;
        resp.physicalSignalingCode = device.capabilities.readUniqueIdentifier.physicalSignalingCode;
        resp.flags = device.capabilities.readUniqueIdentifier.flags;
        resp.deviceID = device.capabilities.readUniqueIdentifier.deviceID; // 24 bits
        resp.minRespPreamblesNo = device.capabilities.readUniqueIdentifier.maxNoOfDeviceVars;
        resp.maxNoOfDeviceVars = device.capabilities.readUniqueIdentifier.maxNoOfDeviceVars;
        resp.configChangeCounter = device.capabilities.readUniqueIdentifier.configChangeCounter;
        resp.extendedFieldDeviceStatus = device.capabilities.readUniqueIdentifier.extendedFieldDeviceStatus;
        resp.manufacturerIDCode = device.capabilities.readUniqueIdentifier.manufacturerIDCode;
        resp.privateLabelDistributorCode = device.capabilities.readUniqueIdentifier.privateLabelDistributorCode;
        resp.deviceProfile = device.capabilities.readUniqueIdentifier.deviceProfile;
    }
    catch (DeviceNotFoundException& ex)
    {
        LOG_ERROR(ex.what());
        return RCS_E32_Busy;
    }
    catch (...)
    {
        return RCS_E32_Busy;
    }

    return C000_N00;

}

uint8_t GatewayWrappedRequestsHandler::Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                                              const C013_ReadTagDescriptorDate_Req& readTagDescriptorDate,
                                              C013_ReadTagDescriptorDate_Resp& resp)
{
    LOG_WARN("C013_ReadTagDescriptorDate_Req : no data for the moment.");

    std::ostringstream stream;
    stream << "Handle";
    hart7::util::toStringC013(stream, &readTagDescriptorDate);
    LOG_TRACE(stream);

    std::strcpy(resp.tag, "Tag");
    std::strcpy(resp.descriptor, "Descriptor");
    resp.dateCode.day = 10;
    resp.dateCode.month = 11;
    resp.dateCode.year = 12;

    //    C013_E16 = RCS_E16_AccessRestricted;
    //    C013_E32 = RCS_E32_Busy;

    return C013_N00;
}

uint8_t GatewayWrappedRequestsHandler::Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                                              const C020_ReadLongTag_Req& readLongTagReq, C020_ReadLongTag_Resp& resp)
{
    LOG_TRACE("C020_ReadLongTag_Req");
    try
    {
        Address64 address64 = hart7::util::getAddress64FromUniqueId(DeviceUniqueId);
        Address32 address32 = commonData.networkEngine.getAddress32(address64);
        Device& device = commonData.networkEngine.getDevice(address32);

        std::memcpy(resp.longTag, device.capabilities.longTag, 32);

        return C020_N00;

    }
    catch (DeviceNotFoundException& ex)
    {
        LOG_ERROR(ex.what());
        return RCS_E32_Busy;
    }
    catch (...)
    {
        return RCS_E32_Busy;
    }

    return C000_N00;

}

uint8_t GatewayWrappedRequestsHandler::Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                                              const C769_ReadJoinStatus_Req& readJoinStatus,
                                              C769_ReadJoinStatus_Resp& resp)
{
    LOG_TRACE("C769_ReadJoinStatus_Req");

    try
    {
        Address64 address64 = hart7::util::getAddress64FromUniqueId(DeviceUniqueId);
        if (!commonData.networkEngine.existsDevice(address64))
        {
            LOG_TRACE("C769_ReadJoinStatus_Req device " << address64.toString() << " not joined yet!");
            return RCS_E32_Busy;
        }

        Address32 address32 = commonData.networkEngine.getAddress32(address64);
        Device& device = commonData.networkEngine.getDevice(address32);

        if (device.capabilities.isManager() || device.capabilities.isGateway())
        {
            resp.joinStatus = JoinProcessStatusMask_NormalOperationCommencing;
        }
        else
        {
            resp.joinStatus = hart7::util::getJoinProcessStatus(device);
        }

        // !!! this is a hack used only to inform the GW about the status of a device join flow;
        // only the join status is filled, the rest of the members will be 0;
        resp.wirelessMode = 0;
        resp.noOfAvailableNeighbors = 0;
        resp.noOfAdvertisingPacketsReceived = 0;
        resp.noOfJoinAttempts = 0;
        resp.joinRetryTimer.u32 = 0;
        resp.networkSearchTimer.u32 = 0;
    }
    catch (DeviceNotFoundException& ex)
    {
        LOG_ERROR(ex.what());
        return RCS_E32_Busy;
    }
    catch (...)
    {
        return RCS_E32_Busy;
    }

    return C769_NOO;
}

uint8_t GatewayWrappedRequestsHandler::Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                                              const C773_WriteNetworkId_Req& writeNetworkId,
                                              C773_WriteNetworkId_Resp& resp)
{
    LOG_WARN("C773_WriteNetworkId_Req");

    commonData.settings.NetworkID = writeNetworkId.m_unNetworkId;
    resp.m_unNetworkId = writeNetworkId.m_unNetworkId;

    return C773_NOO;
}

uint8_t GatewayWrappedRequestsHandler::Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                                              const C774_ReadNetworkId_Req& readNetworkId,
                                              C774_ReadNetworkId_Resp& resp)
{
    LOG_WARN("C774_ReadNetworkId_Req");

    resp.m_unNetworkId = commonData.settings.NetworkID;

    return C774_NOO;
}

uint8_t GatewayWrappedRequestsHandler::Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                                              const C775_WriteNetworkTag_Req& writeNetworkTag,
                                              C775_WriteNetworkTag_Resp& resp)
{
    LOG_WARN("C775_WriteNetworkTag_Req");

    commonData.settings.networkManagerTag = NE::Misc::Convert::array2string(writeNetworkTag.m_aNetworkTag, 32);

    return C775_N00;
}

uint8_t GatewayWrappedRequestsHandler::Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                                              const C776_ReadNetworkTag_Req& readNetworkTag,
                                              C776_ReadNetworkTag_Resp& resp)
{
    LOG_WARN("C776_ReadNetworkTag_Req : no data for the moment.");
    commonData.settings.networkManagerTag.c_str();

    return C776_N00;
}

uint8_t GatewayWrappedRequestsHandler::Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                                              const C778_ReadBatteryLife_Req& readBatteryLife,
                                              C778_ReadBatteryLife_Resp& resp)
{
    LOG_WARN("C778_ReadBatteryLife_Req : no data for the moment.");
    resp.m_unBatteryLifeRemaining = 0xFFFF;

    //    C778_E06 = RCS_E06_DeviceSpecificCommandError,
    //    C778_E32 = RCS_E32_Busy

    return C778_N00;
}

uint8_t GatewayWrappedRequestsHandler::Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                                              const C779_ReportDeviceHealth_Req& reportDeviceHealth,
                                              C779_ReportDeviceHealth_Resp& resp)
{
    LOG_TRACE("C779_ReportDeviceHealth_Resp");

    return C779_N00;
}

uint8_t GatewayWrappedRequestsHandler::Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                                              const C780_ReportNeighborHealthList_Req& reportNeighborHealthList,
                                              C780_ReportNeighborHealthList_Resp& resp)
{
    LOG_TRACE("C780_ReportNeighborHealthList_Req");

    try
    {
        Address64 address64 = hart7::util::getAddress64FromUniqueId(DeviceUniqueId);
        Address32 address32 = commonData.networkEngine.getAddress32(address64);
        Device& device = commonData.networkEngine.getDevice(address32);

        resp.m_ucNeighborTableIndex = device.capabilities.reportNeighborHealthList.m_ucNeighborTableIndex;
        resp.m_ucNoOfNeighborEntriesRead = device.capabilities.reportNeighborHealthList.m_ucNoOfNeighborEntriesRead;
        resp.m_ucTotalNoOfNeighbors = device.capabilities.reportNeighborHealthList.m_ucTotalNoOfNeighbors;

        for (int i = 0; i < resp.m_ucNoOfNeighborEntriesRead; ++i)
        {
            resp.m_aNeighbors[i].nicknameOfNeighbor
                        = device.capabilities.reportNeighborHealthList.m_aNeighbors[i].nicknameOfNeighbor;
            resp.m_aNeighbors[i].neighborFlags
                        = device.capabilities.reportNeighborHealthList.m_aNeighbors[i].neighborFlags;
            resp.m_aNeighbors[i].meanRSLSinceLastReport
                        = device.capabilities.reportNeighborHealthList.m_aNeighbors[i].meanRSLSinceLastReport;
            resp.m_aNeighbors[i].noOfPacketsTransmittedToThisNeighbor
                        = device.capabilities.reportNeighborHealthList.m_aNeighbors[i].noOfPacketsTransmittedToThisNeighbor;
            resp.m_aNeighbors[i].noOfFailedTransmits
                        = device.capabilities.reportNeighborHealthList.m_aNeighbors[i].noOfFailedTransmits;
            resp.m_aNeighbors[i].noOfPacketsReceivedFromThisNeighbor
                        = device.capabilities.reportNeighborHealthList.m_aNeighbors[i].noOfPacketsReceivedFromThisNeighbor;
        }
    }
    catch (DeviceNotFoundException& ex)
    {
        LOG_ERROR(ex.what());
        return RCS_E02_InvalidSelection;
    }
    catch (...)
    {
        return RCS_E02_InvalidSelection;
    }

    return C779_N00;
}

uint8_t GatewayWrappedRequestsHandler::Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                                              const C781_ReadDeviceNicknameAddress_Req& readDeviceNicknameAddress,
                                              C781_ReadDeviceNicknameAddress_Resp& resp)
{
    LOG_TRACE("C781_ReadDeviceNicknameAddress_Req");
    resp.Nickname = commonData.networkEngine.createAddress32(hart7::util::getAddress64FromUniqueId(DeviceUniqueId));

    return C781_N00;
}

uint8_t GatewayWrappedRequestsHandler::Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                                              const C782_ReadSessionEntries_Req& readSessionEntries,
                                              C782_ReadSessionEntries_Resp& resp)
{
    LOG_TRACE("C782_ReadSessionEntries_Req");

    hart7::stack::network::WHartNetworkData::SessionTable sessionTable = commonData.stack.network.sessionTable;

    uint8_t listSize = 0;
    int pos = 0;
    std::vector<uint16_t> deviceSessionIndexes;
    LOG_TRACE("sessionTable.size(): " << (int) sessionTable.size());
    hart7::stack::network::WHartNetworkData::SessionTable::iterator itSes = sessionTable.begin();
    for (; itSes != sessionTable.end(); ++itSes)
    {
        if (COMPARE_FIXED_ARRAY(itSes->peerUniqueID.bytes, DeviceUniqueId) == 0)
        {
            ++listSize;
            deviceSessionIndexes.push_back(pos);
        }

        ++pos;
    }

    if (listSize == 0)
    {
        resp.m_ucNoOfEntriesRead = 0;
        return C782_E02;
    }

    uint8_t maxSessionNo = (hart7::nmanager::operations::WHOperationQueue::MAX_PACKAGE_SIZE_AFTER_JOIN - 3 - 4) / 16;
    // if the client requested 5 elements but the size of the package allows only max 3 then 3 became the no of elements to read
    maxSessionNo = std::min(readSessionEntries.m_ucNoOfEntriesToRead, maxSessionNo);

    hart7::util::ManagerUtils::determineElementsToReturn(listSize, readSessionEntries.m_ucSessionIndex, maxSessionNo,
                                                         resp.m_ucSessionIndex, resp.m_ucNoOfEntriesRead);

    resp.m_unNoOfActiveSessions = listSize;

    for (int i = resp.m_ucSessionIndex; i < resp.m_ucSessionIndex + resp.m_ucNoOfEntriesRead; ++i)
    {
        // deviceSessionIndexes[i] - the i'th session for the device
        int pos = i - resp.m_ucSessionIndex;
        memcpy(resp.m_aSessions[pos].peerDeviceUniqueId, sessionTable[deviceSessionIndexes[i]].peerUniqueID.bytes, 5);
        resp.m_aSessions[pos].peerDeviceNickname = sessionTable[deviceSessionIndexes[i]].peerNickName;
        if (sessionTable[deviceSessionIndexes[i]].sessionKey.keyCode == WHartSessionKey::joinKeyed)
        {
            resp.m_aSessions[pos].sessionType = SessionTypeCode_Join;
        }
        else
        {
            resp.m_aSessions[pos].sessionType = SessionTypeCode_Unicast;
        }
        resp.m_aSessions[pos].peerDeviceNonceCounterVal = sessionTable[deviceSessionIndexes[i]].receiveNonceCounter;
        resp.m_aSessions[pos].theDeviceNonceCounterVal = sessionTable[deviceSessionIndexes[i]].transmitNonceCounter;
    }

    if ((readSessionEntries.m_ucSessionIndex != resp.m_ucSessionIndex) || (readSessionEntries.m_ucNoOfEntriesToRead
                != resp.m_ucNoOfEntriesRead))
    {
        return C782_W08;
    }

    return C782_NOO;
}

uint8_t GatewayWrappedRequestsHandler::Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                                              const C783_ReadSuperframeList_Req& readSuperframeList,
                                              C783_ReadSuperframeList_Resp& resp)
{
    LOG_TRACE("C783_ReadSuperframeList_Req");

    uint8_t maxSuperframesNo = std::min(readSuperframeList.m_ucNoOfEntriesToRead, (uint8_t) 9);
    hart7::util::ManagerUtils::determineElementsToReturn(
                                                         9, // there are 9 super frames
                                                         readSuperframeList.m_ucSuperframeIndex, maxSuperframesNo,
                                                         resp.m_ucSuperframeIndex, resp.m_ucNoOfEntriesRead);

    for (int i = resp.m_ucSuperframeIndex; i < resp.m_ucSuperframeIndex + resp.m_ucNoOfEntriesRead; ++i)
    {
        resp.m_aSuperframes[i].superframeId = i + 1;
        resp.m_aSuperframes[i].superframeModeFlags = SuperframeModeFlagsMask_Active;
        resp.m_aSuperframes[i].noOfSlotsInSuperframe = (uint16_t) (SuperframeLength::SLENGTH_250_MS
                    * std::pow((double) 2, i));
    }

    resp.m_ucNoOfActiveSuperframes = 9;

    if ((readSuperframeList.m_ucSuperframeIndex != resp.m_ucSuperframeIndex)
                || (readSuperframeList.m_ucNoOfEntriesToRead != resp.m_ucNoOfEntriesRead))
    {
        return C783_W08;
    }

    return C783_N00;
}

uint8_t GatewayWrappedRequestsHandler::Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                                              const C784_ReadLinkList_Req& readLinkList, C784_ReadLinkList_Resp& resp)
{
    LOG_TRACE("C784_ReadLinkList_Req");

    try
    {
        Address64 address64 = hart7::util::getAddress64FromUniqueId(DeviceUniqueId);
        Address32 address32 = commonData.networkEngine.getAddress32(address64);

        SubnetTdma& subnetTdma = commonData.networkEngine.getSubnetTdma();
        NodeTdma& nodeTdma = subnetTdma.getNodeTdma(address32);
        resp.m_unNoOfActiveLinks = nodeTdma.getTimeslotAllocationsArraySize();

        uint8_t maxLinksNo = (hart7::nmanager::operations::WHOperationQueue::MAX_PACKAGE_SIZE_AFTER_JOIN - 6 - 4) / 8;
        // if the client requested 5 elements but the size of the package allows only max 3 then 3 became the no of elements to read
        maxLinksNo = std::min(readLinkList.m_ucNoOfLinksToRead, maxLinksNo);

        hart7::util::ManagerUtils::determineElementsToReturn(resp.m_unNoOfActiveLinks, readLinkList.m_unLinkIndex,
                                                             maxLinksNo, resp.m_unLinkIndex, resp.m_ucNoOfLinksRead);
        int index = 0;

        // take into account only once a reception link (because it single reception enabled)
        if (nodeTdma.isUseSingleReception())
        {
            if (0 == resp.m_unLinkIndex) // if the links were requested from the 0 index

            {
                resp.m_aLinks[0].superframeId = (uint8_t) SuperframeId::SID_ALL;
                resp.m_aLinks[0].channelOffsetForThisLink = 0;
                resp.m_aLinks[0].slotNoForThisLink = 0;
                resp.m_aLinks[0].nicknameOfNeighborForThisLink = 0xFFFF;
                uint8_t linkOptions = 0;
                linkOptions |= LinkOptionFlagCodesMask_Receive;
                resp.m_aLinks[0].linkOptions = LinkOptionFlagCodesMask_Receive;
                resp.m_aLinks[0].linkType = LinkType_Normal;
            }

            index = 1;
        }

        bool responseReady = false;
        for (int i = 0; i < MAX_STAR_INDEX; ++i)
        {
            NE::Model::Tdma::TimeslotAllocations tas = nodeTdma.timeslotAllocationsArray[i];
            NE::Model::Tdma::TimeslotAllocations::iterator itTa = tas.begin();
            for (; itTa != tas.end(); ++itTa)
            {
                if (nodeTdma.isUseSingleReception())
                {
                    if (itTa->reception && !itTa->shared && !itTa->transmission)
                    {
                        if (itTa->linkType != LinkTypes::JOIN)
                        {
                            // the reception links with single reception are ignored
                            continue;
                        }
                    }
                }

                if (index > resp.m_unLinkIndex + resp.m_ucNoOfLinksRead)
                {
                    responseReady = true;
                    break;
                }

                if (index >= resp.m_unLinkIndex)
                {
                    resp.m_aLinks[index - resp.m_unLinkIndex].superframeId
                                = NE::Model::Tdma::getSuperframeId(itTa->publishPeriod);
                    resp.m_aLinks[index - resp.m_unLinkIndex].channelOffsetForThisLink = itTa->channel - 1;

                    //starIndex + timeslotIndex * MAX_STAR_INDEX;
                    resp.m_aLinks[index - resp.m_unLinkIndex].slotNoForThisLink = i + itTa->timeslotIndex
                                * MAX_STAR_INDEX;
                    resp.m_aLinks[index - resp.m_unLinkIndex].nicknameOfNeighborForThisLink
                                = hart7::util::getAddress16(itTa->peerAddress);

                    uint8_t linkOptions = 0;
                    if (itTa->transmission)
                    {
                        linkOptions |= LinkOptionFlagCodesMask_Transmit;
                    }
                    if (itTa->reception)
                    {
                        linkOptions |= LinkOptionFlagCodesMask_Receive;
                    }
                    if (itTa->shared)
                    {
                        linkOptions |= LinkOptionFlagCodesMask_Shared;
                    }

                    resp.m_aLinks[index - resp.m_unLinkIndex].linkOptions = linkOptions;
                    resp.m_aLinks[index - resp.m_unLinkIndex].linkType = hart7::util::getStackLinkType(itTa->linkType);
                }

                if (responseReady == true)
                {
                    break;
                }

                ++index;
            }
        }
    }
    catch (DeviceNotFoundException& ex)
    {
        resp.m_ucNoOfLinksRead = 0;
        resp.m_unLinkIndex = 0;
        LOG_ERROR(ex.what());
        return C784_E02;
    }
    catch (...)
    {
        resp.m_ucNoOfLinksRead = 0;
        resp.m_unLinkIndex = 0;
        return C784_E02;
    }

    if ((readLinkList.m_unLinkIndex != resp.m_unLinkIndex) || (readLinkList.m_ucNoOfLinksToRead
                != resp.m_ucNoOfLinksRead))
    {
        return C784_W08;
    }

    return C784_N00;
}

uint8_t GatewayWrappedRequestsHandler::Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                                              const C785_ReadGraphList_Req& readGraphList,
                                              C785_ReadGraphList_Resp& resp)
{
    LOG_TRACE("C785_ReadGraphList_Req");

    try
    {
        Address64 address64 = hart7::util::getAddress64FromUniqueId(DeviceUniqueId);
        Address32 address32 = commonData.networkEngine.getAddress32(address64);
        SubnetTopology& subnet = commonData.networkEngine.getSubnetTopology();
        Node& node = subnet.getNode(address32);

        resp.m_ucGraphListIndex = readGraphList.m_ucGraphListIndex;

        // a graph index number is requested -> find the graph id
        Uint16 graphIdForGraphIndex = 0;
        Uint16 lastGraphId = 0; // keeps the last graph id so that in case the index is out of bound is changed to this one
        bool graphIdFound = false;

        EdgeList& edgeList = node.getOutBoundEdges();
        std::set<Uint16> nodeGraphs;
        int pos = 0;
        for (EdgeList::iterator itEdge = edgeList.begin(); itEdge != edgeList.end(); ++itEdge)
        {
            GraphNeighborMap& graphEdges = itEdge->getGraphs();
            for (GraphNeighborMap::iterator itGraph = graphEdges.begin(); itGraph != graphEdges.end(); ++itGraph)
            {
                if (subnet.existsPath(itGraph->first) && !subnet.getPath(itGraph->first).isSourcePath())
                {
                    // only if there is not already in the list of graph ids ...
                    if (nodeGraphs.find(itGraph->first) == nodeGraphs.end())
                    {
                        lastGraphId = itGraph->first;
                        if (pos == readGraphList.m_ucGraphListIndex)
                        {
                            graphIdForGraphIndex = itGraph->first;
                            graphIdFound = true;
                        }
                        nodeGraphs.insert(itGraph->first);

                        ++pos;
                    }
                }
            }
        }

        resp.m_ucTotalNoOfGraphs = nodeGraphs.size();

        if (lastGraphId == 0)
        {
            LOG_WARN("There is no graph for this address  " << ToStr(address32));
            resp.m_ucNoOfNeighbors = 0;
            return C785_E02;
        }

        if (graphIdFound == false)
        {
            LOG_WARN("GW requested a non-existing graph index " << (int) readGraphList.m_ucGraphListIndex);
            graphIdForGraphIndex = lastGraphId;
        }

        std::list<Uint16> graphNeighors;
        for (EdgeList::iterator itEdge = edgeList.begin(); itEdge != edgeList.end(); ++itEdge)
        {
            GraphNeighborMap& graphs = itEdge->getGraphs();
            for (GraphNeighborMap::iterator itGraph = graphs.begin(); itGraph != graphs.end(); ++itGraph)
            {
                if (itGraph->first == graphIdForGraphIndex)
                {
                    graphNeighors.push_back(hart7::util::getAddress16(itEdge->getDestination()));
                }
            }
        }

        resp.m_unGraphId = graphIdForGraphIndex;
        resp.m_ucNoOfNeighbors = graphNeighors.size();
        int i = 0;
        std::list<Uint16>::iterator itNeighbor = graphNeighors.begin();
        for (; itNeighbor != graphNeighors.end(); ++itNeighbor, ++i)
        {
            resp.m_aNicknameOfNeighbor[i] = *itNeighbor;
        }

        if (graphIdFound == false)
        {
            return C785_W08;
        }
    }
    catch (DeviceNotFoundException& ex)
    {
        LOG_ERROR(ex.what());
        resp.m_ucNoOfNeighbors = 0;
        return C785_E02;
    }
    catch (...)
    {
        resp.m_ucNoOfNeighbors = 0;
        return C785_E02;
    }

    return C785_N00;
}

uint8_t GatewayWrappedRequestsHandler::Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                                              const C786_ReadNeighborPropertyFlag_Req& readNeighborPropertyFlag,
                                              C786_ReadNeighborPropertyFlag_Resp& resp)
{
    LOG_TRACE("C786_ReadNeighborPropertyFlag_Req");

    resp.Nickname = readNeighborPropertyFlag.Nickname;

    try
    {
        Address64 address64 = hart7::util::getAddress64FromUniqueId(DeviceUniqueId);
        Address32 address32 = commonData.networkEngine.getAddress32(address64);

        if (!commonData.networkEngine.existsDevice(readNeighborPropertyFlag.Nickname))
        {
            return C786_E65; //RCM_E65_UnknownNickname
        }

        SubnetTopology& subnet = commonData.networkEngine.getSubnetTopology();
        Node& node = subnet.getNode(address32);

        EdgeList& edgeList = node.getOutBoundEdges();
        for (EdgeList::iterator itEdge = edgeList.begin(); itEdge != edgeList.end(); ++itEdge)
        {
            resp.NeighbourFlag = itEdge->isDestinationClockSource() ? 1 : 0;
        }
    }
    catch (DeviceNotFoundException& ex)
    {
        LOG_ERROR(ex.what());
        return C786_E65;
    }
    catch (...)
    {
        return C786_E65;
    }

    return C786_N00;
}

uint8_t GatewayWrappedRequestsHandler::Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                                              const C787_ReportNeighborSignalLevels_Req& reportNeighborSignalLevels,
                                              C787_ReportNeighborSignalLevels_Resp& resp)
{
    LOG_TRACE("C787_ReportNeighborSignalLevels_Req");

    try
    {
        Address64 address64 = hart7::util::getAddress64FromUniqueId(DeviceUniqueId);
        Address32 address32 = commonData.networkEngine.getAddress32(address64);
        Device& device = commonData.networkEngine.getDevice(address32);

        resp.m_ucNeighborTableIndex = device.capabilities.reportNeighborSignalLevels.m_ucNeighborTableIndex;
        resp.m_ucNoOfNeighborEntriesRead = device.capabilities.reportNeighborSignalLevels.m_ucNoOfNeighborEntriesRead;
        resp.m_ucTotalNoOfNeighbors = device.capabilities.reportNeighborSignalLevels.m_ucTotalNoOfNeighbors;

        for (int i = 0; i < resp.m_ucNoOfNeighborEntriesRead; ++i)
        {
            resp.m_aNeighbors[i].RSLindB = device.capabilities.reportNeighborSignalLevels.m_aNeighbors[i].RSLindB;
            resp.m_aNeighbors[i].nickname = device.capabilities.reportNeighborSignalLevels.m_aNeighbors[i].nickname;
        }
    }
    catch (DeviceNotFoundException& ex)
    {
        LOG_ERROR(ex.what());
        return RCS_E02_InvalidSelection;
    }
    catch (...)
    {
        return RCS_E02_InvalidSelection;
    }

    return C787_N00;
}

uint8_t GatewayWrappedRequestsHandler::Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                                              const C794_ReadUTCTime_Req& readUTCTime, C794_ReadUTCTime_Resp& resp)
{
    LOG_TRACE("Handle(C794_ReadUTCTime_Req)");

    // TODO get the data from somewhere

    return C794_N00;
}

uint8_t GatewayWrappedRequestsHandler::Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                                              const C800_ReadServiceList_Req& readServiceList,
                                              C800_ReadServiceList_Resp& resp)
{
    LOG_TRACE("Handle(C800_ReadServiceList_Req)");

    try
    {
        Address64 address64 = hart7::util::getAddress64FromUniqueId(DeviceUniqueId);
        Address32 address32 = commonData.networkEngine.getAddress32(address64);

        resp.m_ucNoOfEntriesRead = 0;

        NodeServiceMap& subnetServices = commonData.networkEngine.getSubnetServices().getNodeServiceMap();
        NodeServiceMap::iterator itServices = subnetServices.find(address32);

        if (itServices == subnetServices.end())
        {
            return C800_E06; // RCS_E06_DeviceSpecificCommandError
        }

        ServiceList& services = itServices->second.getServices();
        if (services.size() == 0)
        {
            return C800_E02; // RCS_E02_InvalidSelection
        }

        uint8_t maxServicesNo = (hart7::nmanager::operations::WHOperationQueue::MAX_PACKAGE_SIZE_AFTER_JOIN - 4 - 4)
                    / 10;
        // if the client requested 5 elements but the size of the package allows only max 3 then 3 became the no of elements to read
        maxServicesNo = std::min(readServiceList.m_ucNoOfEntriesToRead, maxServicesNo);

        hart7::util::ManagerUtils::determineElementsToReturn(services.size(), readServiceList.m_ucServiceIndex,
                                                             maxServicesNo, resp.m_ucServiceIndex,
                                                             resp.m_ucNoOfEntriesRead);

        resp.m_ucNoOfActiveServices = services.size();

        int index = 0;
        ServiceList::iterator itService = services.begin();
        for (; itService != services.end(); ++itService, ++index)
        {
            if (index > resp.m_ucServiceIndex + resp.m_ucNoOfEntriesRead)
            {
                break;
            }

            if (index >= resp.m_ucServiceIndex)
            {
                resp.m_aServices[index - resp.m_ucServiceIndex].nicknameOfPeer
                            = hart7::util::getAddress16(itService->peerAddress);
                resp.m_aServices[index - resp.m_ucServiceIndex].period.u32 = itService->period;
                resp.m_aServices[index - resp.m_ucServiceIndex].routeId = itService->routeId;
                resp.m_aServices[index - resp.m_ucServiceIndex].serviceId = itService->serviceId;

                uint8_t res = 0;
                res |= (itService->source) ? ServiceRequestFlagsMask_Source : 0;
                res |= (itService->sink) ? ServiceRequestFlagsMask_Sink : 0;
                res |= (itService->intermittent) ? ServiceRequestFlagsMask_Intermittent : 0;
                resp.m_aServices[index - resp.m_ucServiceIndex].serviceRequestFlags = res;

                resp.m_aServices[index - resp.m_ucServiceIndex].serviceApplicationDomain
                            = hart7::util::getStackServiceApplicationDomain(itService->applicationDomain);
            }
        }

        if ((readServiceList.m_ucServiceIndex != resp.m_ucServiceIndex) || (readServiceList.m_ucNoOfEntriesToRead
                    != resp.m_ucNoOfEntriesRead))
        {
            return C800_W08; // RCM_W08_SetToNearestPossibleValue
        }

    }
    catch (DeviceNotFoundException& ex)
    {
        LOG_ERROR(ex.what());
        resp.m_ucNoOfEntriesRead = 0;
        return C800_E02;
    }
    catch (...)
    {
        resp.m_ucNoOfEntriesRead = 0;
        return C800_E02;
    }

    return C800_NOO;
}

uint8_t GatewayWrappedRequestsHandler::Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                                              const C802_ReadRouteList_Req& readRouteList,
                                              C802_ReadRouteList_Resp& resp)
{

    LOG_TRACE("Handle(C802_ReadRouteList_Req)");

    try
    {
        Address64 address64 = hart7::util::getAddress64FromUniqueId(DeviceUniqueId);
        Address32 address32 = commonData.networkEngine.getAddress32(address64);

        resp.m_ucNoOfEntriesRead = 0;

        NodeServiceMap& subnetServices = commonData.networkEngine.getSubnetServices().getNodeServiceMap();
        NodeServiceMap::iterator itServices = subnetServices.find(address32);

        if (itServices == subnetServices.end())
        {
            return C802_E02; // RCS_E02_InvalidSelection
        }

        RouteList& routes = itServices->second.getRouteList();
        if (routes.size() == 0)
        {
            return C802_E02; // RCS_E02_InvalidSelection
        }

        uint8_t maxRoutesNo = (hart7::nmanager::operations::WHOperationQueue::MAX_PACKAGE_SIZE_AFTER_JOIN - 4 - 4) / 6;
        // if the client requested 5 elements but the size of the package allows only max 3 then 3 became the no of elements to read
        maxRoutesNo = std::min(readRouteList.m_ucNoOfEntriesToRead, maxRoutesNo);

        hart7::util::ManagerUtils::determineElementsToReturn(routes.size(), readRouteList.m_ucRouteIndex, maxRoutesNo,
                                                             resp.m_ucRouteIndex, resp.m_ucNoOfEntriesRead);

        resp.m_ucNoOfActiveRoutes = routes.size();
        resp.m_ucNoOfRoutesRemaining = routes.size() - (resp.m_ucRouteIndex + resp.m_ucNoOfEntriesRead);

        int index = 0;
        RouteList::iterator itRoute = routes.begin();
        for (; itRoute != routes.end(); ++itRoute, ++index)
        {
            if (index > resp.m_ucRouteIndex + resp.m_ucNoOfEntriesRead)
            {
                break;
            }

            if (index >= resp.m_ucRouteIndex)
            {
                resp.m_aRoutes[index - resp.m_ucRouteIndex].destinationNickname
                            = hart7::util::getAddress16(itRoute->getPeerAddress());
                resp.m_aRoutes[index - resp.m_ucRouteIndex].graphId = itRoute->getGraphId();
                resp.m_aRoutes[index - resp.m_ucRouteIndex].routeId = itRoute->getRouteId();
                resp.m_aRoutes[index - resp.m_ucRouteIndex].sourceRouteAttached = itRoute->isSourceRoute();
            }
        }

        if ((readRouteList.m_ucRouteIndex != resp.m_ucRouteIndex) || (readRouteList.m_ucNoOfEntriesToRead
                    != resp.m_ucNoOfEntriesRead))
        {
            return C802_W08; // RCM_W08_SetToNearestPossibleValue
        }

    }
    catch (DeviceNotFoundException& ex)
    {
        LOG_ERROR(ex.what());
        resp.m_ucNoOfEntriesRead = 0;
        return C802_E02;
    }
    catch (...)
    {
        resp.m_ucNoOfEntriesRead = 0;
        return C802_E02;
    }

    //    C802_E02 = RCS_E02_InvalidSelection,
    //    C802_E05 = RCS_E05_TooFewDataBytesReceived,
    //    C802_W08 = RCM_W08_SetToNearestPossibleValue

    return C802_NOO;
}

uint8_t GatewayWrappedRequestsHandler::Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                                              const C803_ReadSourceRoute_Req& readSourceRoute,
                                              C803_ReadSourceRoute_Resp& resp)
{

    LOG_TRACE("Handle(C803_ReadSourceRoute_Req)");

    try
    {
        Address64 address64 = hart7::util::getAddress64FromUniqueId(DeviceUniqueId);
        Address32 address32 = commonData.networkEngine.getAddress32(address64);

        resp.m_ucNoOfHops = 0;

        NodeServiceMap& subnetServices = commonData.networkEngine.getSubnetServices().getNodeServiceMap();
        NodeServiceMap::iterator itServices = subnetServices.find(address32);

        if (itServices == subnetServices.end())
        {
            return C803_E65;
        }

        RouteList& routes = itServices->second.getRouteList();
        if (routes.size() == 0)
        {
            return C803_E65;
        }

        int index = 0;
        RouteList::iterator itRoute = routes.begin();
        for (; itRoute != routes.end(); ++itRoute, ++index)
        {
            if (itRoute->getRouteId() == (address32 << 16) + readSourceRoute.m_ucRouteId)
            {
                resp.m_ucRouteId = readSourceRoute.m_ucRouteId;
                if (itRoute->isSourceRoute())
                {
                    std::list<Address32>& destinations = itRoute->getDestinations();
                    resp.m_ucNoOfHops = destinations.size();
                    int i = 0;
                    std::list<Address32>::iterator itList = destinations.begin();
                    for (; itList != destinations.end(); ++itList)
                    {
                        resp.m_aHopNicknames[i] = *itList;
                    }
                }
                else
                {
                    LOG_WARN("C803_ReadSourceRoute_Req for an id (" << (int)readSourceRoute.m_ucRouteId << ") that is not a source route id ");
                    return C803_E65;
                }

                break;

            }
        }

    }
    catch (DeviceNotFoundException& ex)
    {
        LOG_ERROR(ex.what());
        return C803_E65;
    }
    catch (...)
    {
        return C803_E65;
    }

    return C803_NOO;
}

uint8_t GatewayWrappedRequestsHandler::Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                                              const C814_ReadDeviceListEntries_Req& readDeviceListEntries,
                                              C814_ReadDeviceListEntries_Resp &resp)
{
    LOG_TRACE("Handle(C814_ReadDeviceListEntries_Req)");

    resp.m_ucDeviceListCode = readDeviceListEntries.m_ucDeviceListCode;

    if (resp.m_ucDeviceListCode != DeviceListCode_ActiveDeviceList) {
        resp.m_ucNoOfListEntriesRead = 0;
        LOG_WARN("NM responds for 814 command only to active device list request.");
        return C814_E02;
    }

    try
    {
        NodeMap& nodes = commonData.networkEngine.getSubnetTopology().getSubnetNodes();

        resp.m_unTotalNoOfEntriesInList = nodes.size();
        uint8_t maxDevicesNo = (hart7::nmanager::operations::WHOperationQueue::MAX_PACKAGE_SIZE_AFTER_JOIN - 6 - 4) / 6;
        maxDevicesNo = std::min(readDeviceListEntries.m_ucNoOfListEntriesToRead, maxDevicesNo);
        hart7::util::ManagerUtils::determineElementsToReturn(maxDevicesNo, readDeviceListEntries.m_unStartingListIndex,
                                                             maxDevicesNo, resp.m_unStartingListIndex,
                                                             resp.m_ucNoOfListEntriesRead);

        int index = 0;
        NodeMap::iterator itNode = nodes.begin();
        for (; itNode != nodes.end(); ++index, ++itNode)
        {
            if (itNode->second.getStatus() != Status::ACTIVE) {
                // returns only
                continue;
            }

            if (index > resp.m_unStartingListIndex + resp.m_ucNoOfListEntriesRead)
            {
                break;
            }

            if (index >= resp.m_unStartingListIndex)
            {
                WHartUniqueID uniqueId =
                            hart7::util::getUniqueIdFromAddress64(commonData.networkEngine.getAddress64(itNode->first));
                std::memcpy(resp.m_aDeviceUniqueIds[index - resp.m_unStartingListIndex], uniqueId.bytes, 5);
            }
        }
    }
    catch (DeviceNotFoundException& ex)
    {
        resp.m_ucNoOfListEntriesRead = 0;
        LOG_ERROR(ex.what());
        return C814_E02; // RCS_E02_InvalidSelection
    }
    catch (...)
    {
        resp.m_ucNoOfListEntriesRead = 0;
        return C814_E02; // RCS_E02_InvalidSelection
    }

    if ((resp.m_unStartingListIndex != readDeviceListEntries.m_ucNoOfListEntriesToRead)
                || (resp.m_ucNoOfListEntriesRead != readDeviceListEntries.m_ucNoOfListEntriesToRead))
    {
        // there were few neighbors read  (as requested)
        return C814_W08; // RCM_W08_SetToNearestPossibleValue
    }

    return C814_NOO;
}

uint8_t GatewayWrappedRequestsHandler::Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                                              const C817_ReadChannelBlacklist_Req& readChannelBlacklist,
                                              C817_ReadChannelBlacklist_Resp &resp)
{
    LOG_TRACE("Handle(C817_ReadChannelBlacklist_Req)");

    resp.m_ucNoOfBitsInCurrentChannelMapArray = 16;
    resp.m_unCurrentChannelMapArray = commonData.settings.channelMap;
    resp.m_unPendingChannelMapArray = commonData.settings.channelMap;

    return C817_NOO;
}

uint8_t GatewayWrappedRequestsHandler::Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                                              const C818_WriteChannelBlacklist_Req& writeChannelBlacklist,
                                              C818_WriteChannelBlacklist_Resp &resp)
{
    LOG_TRACE("Handle(C818_WriteChannelBlacklist_Req)");

    commonData.settings.pendingChannelMap = writeChannelBlacklist.m_unPendingChannelMapArray;

    resp.m_ucNoOfBitsInNewChannelMapArray = writeChannelBlacklist.m_ucNoOfBitsInNewChannelMapArray;
    resp.m_unPendingChannelMapArray = writeChannelBlacklist.m_unPendingChannelMapArray;

    return C818_NOO;
}

uint8_t GatewayWrappedRequestsHandler::Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                                              const C821_WriteNetworkAccessMode_Req& writeNetworkAccessMode,
                                              C821_WriteNetworkAccessMode_Resp &resp)
{
    LOG_TRACE("Handle(C821_WriteNetworkAccessMode_Req)");

    if (writeNetworkAccessMode.m_ucNetworkAccessModeCode > (Uint16) NetworkAccessModeCode_Lockdown)
    {
        return C821_E02;
    }

    commonData.settings.networkAccessMode = writeNetworkAccessMode.m_ucNetworkAccessModeCode;

    return RCS_N00_Success;
}

uint8_t GatewayWrappedRequestsHandler::Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                                              const C822_ReadNetworkAccessMode_Req& readNetworkAccessMode,
                                              C822_ReadNetworkAccessMode_Resp &resp)
{
    LOG_TRACE("Handle(C822_ReadNetworkAccessMode_Req)");

    resp.m_ucNetworkAccessModeCode = commonData.settings.networkAccessMode;

    return RCS_N00_Success;
}

uint8_t GatewayWrappedRequestsHandler::Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                                              const C832_ReadNetworkDeviceIdentity_Req& readNetworkDeviceIdentity,
                                              C832_ReadNetworkDeviceIdentity_Resp &resp)
{
    LOG_TRACE("Handle(C832_ReadNetworkDeviceIdentity_Req)");

    try
    {
        memcpy(resp.DeviceUniqueID, readNetworkDeviceIdentity.DeviceUniqueID, 5);
        Address64 address64 = hart7::util::getAddress64FromUniqueId(readNetworkDeviceIdentity.DeviceUniqueID);
        Address32 address32 = commonData.networkEngine.getAddress32(address64);
        Device& device = commonData.networkEngine.getDevice(address32);

        if (!commonData.networkEngine.existsDevice(address32))
        {
            return C832_ResponseCode_InvalidSelection;
        }

        device.deviceRequested832 = true;

        resp.Nickname = (uint16_t) address32;
        memcpy(resp.LongTag, device.capabilities.longTag, 32);
    }
    catch (DeviceNotFoundException& ex)
    {
        LOG_ERROR(ex.what());
        return C832_ResponseCode_InvalidSelection; //   Error   Invalid selection
    }
    catch (NE::Common::NEException& nex)
    {
        LOG_ERROR(nex.what());
        return C832_ResponseCode_InvalidSelection; //   Error   Invalid selection
    }
    catch (...)
    {
        return C832_ResponseCode_InvalidSelection; //   Error   Invalid selection
    }

    return C832_ResponseCode_Success;
}

uint8_t GatewayWrappedRequestsHandler::Handle(
                                              WHartHandle handle,
                                              const _device_address_t& DeviceUniqueId,
                                              const C833_ReadNetworkDeviceNeighbourHealth_Req& readNetworkDeviceNeighbourHealth,
                                              C833_ReadNetworkDeviceNeighbourHealth_Resp &resp)
{
    LOG_TRACE("Handle(C833_ReadNetworkDeviceNeighbourHealth_Req)");

    int pos = 0;

    try
    {
        memcpy(resp.UniqueID, readNetworkDeviceNeighbourHealth.UniqueID, 5);
        Address64 address64 = hart7::util::getAddress64FromUniqueId(readNetworkDeviceNeighbourHealth.UniqueID);
        Address32 address32 = commonData.networkEngine.getAddress32(address64);
        EdgeList& edgeList = commonData.networkEngine.getSubnetTopology().getNode(address32).getOutBoundEdges();

        // GW requests NeighbourEntriesToRead of reads from NeighbourIndex position => we have to make sure
        // there is at least one neighbor to be read
        if (readNetworkDeviceNeighbourHealth.NeighbourIndex + 1 > (uint8_t) edgeList.size())
        {
            // GW requested neighbors from an indexed outside the size of the list
            resp.NeighbourCount = 0;
            return C833_E65; //  RCM_E65_InvalidNeighborTableIndex
        }

        resp.NeighbourIndex = readNetworkDeviceNeighbourHealth.NeighbourIndex;

        // radu: edgeList.size() contains also the number of the edges not active !
        EdgeList::iterator itEdge = edgeList.begin();
        uint16_t noActiveLinks = 0;
        for (; itEdge != edgeList.end(); ++itEdge)
        {
            if (itEdge->getStatus() != Status::DELETED)
            {
                ++noActiveLinks;
            }
        }

        resp.NeighbourCount = std::min(readNetworkDeviceNeighbourHealth.NeighbourEntriesToRead,
                                       (uint8_t) (noActiveLinks - readNetworkDeviceNeighbourHealth.NeighbourIndex));
        //TODO:[andy] - find the max number of everything that can be returned, and replace hardcodings here
        resp.NeighbourCount = std::min((int)resp.NeighbourCount, 6); // 6 hardcoded max neighbours from stack

        int i = 0;
        itEdge = edgeList.begin();
        for (; itEdge != edgeList.end(); ++itEdge)
        {
            if (itEdge->getStatus() == Status::DELETED)
            {
                continue;
            }

            pos = i++ - readNetworkDeviceNeighbourHealth.NeighbourIndex;

            if (pos < 0)
            {
                continue; // we pass by the first neighbors until the required index
            }

            if (pos >= resp.NeighbourCount)
            {
                break; // we get to the wanted number of neighbors
            }

            resp.Neighbours[pos].NeighbourNickname = (Uint16) (itEdge->getDestination());
            resp.Neighbours[pos].NeighbourRSL = itEdge->getLastRsl();
            resp.Neighbours[pos].TransmittedPacketCount = itEdge->getTotalSent();
            resp.Neighbours[pos].TransmittedPacketWithNoACKCount = itEdge->getTotalFailed();
            resp.Neighbours[pos].ReceivedPacketCount = itEdge->getTotalReceived();
        }
    }
    catch (DeviceNotFoundException& ex)
    {
        resp.NeighbourCount = 0;
        resp.NeighbourIndex = 0;
        LOG_ERROR(ex.what());
        return C833_ResponseCode_InvalidSelection;
    }
    catch (NE::Common::NEException& nex)
    {
        resp.NeighbourCount = 0;
        resp.NeighbourIndex = 0;
        LOG_ERROR(nex.what());
        return C833_ResponseCode_InvalidSelection;
    }
    catch (...)
    {
        resp.NeighbourCount = 0;
        resp.NeighbourIndex = 0;
        return C833_ResponseCode_InvalidSelection;
    }

    if (resp.NeighbourCount != readNetworkDeviceNeighbourHealth.NeighbourEntriesToRead)
    {
        // there were few neighbors read  (as requested)
        return C833_W08; // Warning     Set to nearest value
    }

    return C833_ResponseCode_Success;
}

uint8_t GatewayWrappedRequestsHandler::Handle(
                                              WHartHandle handle,
                                              const _device_address_t& DeviceUniqueId,
                                              const C834_ReadNetworkTopologyInformation_Req& readNetworkTopologyInformation,
                                              C834_ReadNetworkTopologyInformation_Resp &resp)
{
    LOG_TRACE("Handle(C834_ReadNetworkTopologyInformation_Req)");

    try
    {
        memcpy(resp.DeviceLongAddress, readNetworkTopologyInformation.DeviceLongAddress, 5);

        Address64 address64 = hart7::util::getAddress64FromUniqueId(readNetworkTopologyInformation.DeviceLongAddress);
        Address32 address32 = commonData.networkEngine.getAddress32(address64);
        SubnetTopology& subnet = commonData.networkEngine.getSubnetTopology();
        Node& node = subnet.getNode(address32);

        resp.GraphIndexNo = readNetworkTopologyInformation.GraphIndexNo;

        if (readNetworkTopologyInformation.GraphIndexNo != 0)
        {
            LOG_DEBUG("The GW requested an index number different than 0!");
        }

        // a graph index number is requested -> have to find the graph id
        Uint16 graphIdForGraphIndex = 0;
        bool graphIdFound = false;

        EdgeList& edgeList = node.getOutBoundEdges();
        std::set<Uint16> nodeGraphs;
        int pos = 0;
        for (EdgeList::iterator itEdge = edgeList.begin(); itEdge != edgeList.end(); ++itEdge)
        {
            GraphNeighborMap& graphs = itEdge->getGraphs();
            for (GraphNeighborMap::iterator itGraph = graphs.begin(); itGraph != graphs.end(); ++itGraph)
            {
                if (subnet.existsPath(itGraph->first) && !subnet.getPath(itGraph->first).isSourcePath())
                {
                    if (pos == readNetworkTopologyInformation.GraphIndexNo)
                    {
                        graphIdForGraphIndex = itGraph->first;
                        graphIdFound = true;
                    }
                    nodeGraphs.insert(itGraph->first);

                    ++pos;
                }
            }
        }

        resp.TotalGraphsNo = nodeGraphs.size();
        if (graphIdFound == false)
        {
            LOG_WARN("GW requested a non-existing graph index " << (int) readNetworkTopologyInformation.GraphIndexNo);
            resp.NeighboursNo = 0;
            return C834_E65;
        }

        std::list<Uint16> graphNeighbors;
        for (EdgeList::iterator itEdge = edgeList.begin(); itEdge != edgeList.end(); ++itEdge)
        {
            if (itEdge->getStatus() != Status::ACTIVE)
            {
                continue;
            }

            GraphNeighborMap& graphs = itEdge->getGraphs();
            for (GraphNeighborMap::iterator itGraph = graphs.begin(); itGraph != graphs.end(); ++itGraph)
            {
                if (itGraph->first == graphIdForGraphIndex)
                {
                    graphNeighbors.push_back(hart7::util::getAddress16(itEdge->getDestination()));
                }
            }
        }

        resp.IndexGraphId = graphIdForGraphIndex;
        resp.NeighboursNo = graphNeighbors.size();
        int i = 0;
        std::list<Uint16>::iterator itNeighbor = graphNeighbors.begin();
        for (; itNeighbor != graphNeighbors.end(); ++itNeighbor, ++i)
        {
            resp.Neighbour[i] = *itNeighbor;
        }
    }
    catch (DeviceNotFoundException& ex)
    {
        LOG_ERROR(ex.what());
        resp.NeighboursNo = 0;
        return C834_ResponseCode_InvalidSelection;
    }
    catch (NE::Common::NEException& nex)
    {
        resp.NeighboursNo = 0;
        LOG_ERROR(nex.what());
        return C834_ResponseCode_InvalidSelection;
    }
    catch (...)
    {
        resp.NeighboursNo = 0;
        return C834_ResponseCode_InvalidSelection;
    }

    return C834_ResponseCode_Success;
}

uint8_t GatewayWrappedRequestsHandler::Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                                              const C840_ReadDeviceStatistics_Req& readDeviceStatistics,
                                              C840_ReadDeviceStatistics_Resp &resp)
{
    LOG_TRACE("Handle(C840_ReadDeviceStatistics_Req) ");
    memcpy(resp.UniqueID, readDeviceStatistics.UniqueID, 5);

    Address64 address64 = hart7::util::getAddress64FromUniqueId(readDeviceStatistics.UniqueID);

    try
    {
        Address32 address32 = commonData.networkEngine.getAddress32(address64);
        NE::Model::Device& device = commonData.networkEngine.getDevice(address32);

        determineNoGraphs(address32, resp.ActiveGraphsNo, resp.NeighboursNo);

        resp.ActiveFramesNo = 9; // indeed ... hard coded

        SubnetTdma& subnetTdma = commonData.networkEngine.getSubnetTdma();

        if (commonData.networkEngine.existsDevice(address32) && subnetTdma.existsNode(address32))
        {
            NodeTdma& nodeTdma = subnetTdma.getNodeTdma(address32);
            resp.ActiveLinksNo = nodeTdma.getTimeslotAllocationsArraySize();

            _time_t wtime;
            wtime.u32 = time(NULL);
            resp.AverageLatency = wtime; //TODO de luat de undeva
            resp.JoinCount = commonData.networkEngine.getDevicesTable().getNrOfJoinsPerDevice()[address32];

            WHartDate date;
            tm * ptm;
            ptm = gmtime(&device.deviceHistory.lastJoinTime);
            date.day = ptm->tm_mday;
            date.month = ptm->tm_mday;
            date.year = ptm->tm_year;
            resp.LastJoinTime = date;

            resp.LastJoinTimeToday = 0;
        }
        else
        {
            return RCS_E02_InvalidSelection;
        }
    }
    catch (DeviceNotFoundException& ex)
    {
        LOG_ERROR(ex.what());
        return RCS_E02_InvalidSelection;
    }
    catch (...)
    {
        return RCS_E02_InvalidSelection;
    }

    return C840_ResponseCode_Success;
}

uint8_t GatewayWrappedRequestsHandler::Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                                              const C842_WriteDeviceSchedulingFlags_Req& writeDeviceSchedulingFlags,
                                              C842_WriteDeviceSchedulingFlags_Resp &resp)
{
    LOG_TRACE("Handle(C842_WriteDeviceSchedulingFlags_Req) ");

    try
    {
        memcpy(resp.UniqueID, writeDeviceSchedulingFlags.UniqueID, 5);

        Address64 address64 = hart7::util::getAddress64FromUniqueId(writeDeviceSchedulingFlags.UniqueID);
        Address32 address32 = commonData.networkEngine.getAddress32(address64);
        NE::Model::Device& device = commonData.networkEngine.getDevice(address32);

        if ((writeDeviceSchedulingFlags.SchedulingFlags & 0xF8) > 0)
        {
            return RCM_E09_InvalidPropertyFlag;
        }

        device.deviceSchedulingFlags = writeDeviceSchedulingFlags.SchedulingFlags;
    }
    catch (DeviceNotFoundException& ex)
    {
        LOG_WARN(ex.what());
        return RCM_E65_UnknownUID;
    }
    catch (...)
    {
        return RCM_E65_UnknownUID;
    }

    return RCS_N00_Success;
}

uint8_t GatewayWrappedRequestsHandler::Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                                              const C843_ReadDeviceSchedulingFlags_Req& readDeviceSchedulingFlags,
                                              C843_ReadDeviceSchedulingFlags_Resp &resp)
{
    LOG_TRACE("Handle(C843_ReadDeviceSchedulingFlags_Req) ");

    try
    {
        memcpy(resp.UniqueID, readDeviceSchedulingFlags.UniqueID, 5);

        Address64 address64 = hart7::util::getAddress64FromUniqueId(readDeviceSchedulingFlags.UniqueID);
        Address32 address32 = commonData.networkEngine.getAddress32(address64);
        NE::Model::Device& device = commonData.networkEngine.getDevice(address32);

        resp.SchedulingFlags = device.deviceSchedulingFlags;
    }
    catch (DeviceNotFoundException& ex)
    {
        LOG_WARN(ex.what());
        return RCM_E65_UnknownUID;
    }
    catch (...)
    {
        return RCM_E65_UnknownUID;
    }

    return RCS_N00_Success;
}

uint8_t GatewayWrappedRequestsHandler::Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                                              const C844_ReadNetworkConstraints_Req& readNetworkConstraints,
                                              C844_ReadNetworkConstraints_Resp &resp)
{
    LOG_TRACE("Handle(C844_ReadNetworkConstraints_Req) ");

    resp.NetworkFlags = commonData.settings.networkOptimizationFlags;
    resp.ReqRespPairMessagesPerTenSeconds = commonData.settings.requestResponseMessagesPer10Seconds;

    return RCS_N00_Success;
}

uint8_t GatewayWrappedRequestsHandler::Handle(WHartHandle handle, const _device_address_t& DeviceUniqueId,
                                              const C845_WriteNetworkConstraints_Req& writeNetworkConstraints,
                                              C845_WriteNetworkConstraints_Resp &resp)
{
    LOG_TRACE("Handle(C845_WriteNetworkConstraints_Req) ");

    if ((writeNetworkConstraints.NetworkFlags & 0xFC) > 0)
    {
        return RCS_E02_InvalidSelection;
    }

    if (writeNetworkConstraints.ReqRespPairMessagesPerTenSeconds < 1)
    {
        return RCS_E02_InvalidSelection;
    }

    commonData.settings.networkOptimizationFlags = writeNetworkConstraints.NetworkFlags;
    commonData.settings.requestResponseMessagesPer10Seconds = writeNetworkConstraints.ReqRespPairMessagesPerTenSeconds;

    return RCS_N00_Success;
}

uint8_t GatewayWrappedRequestsHandler::Handle(WHartHandle handle, const C64765_NivisMetaCommand_Req & nivisMetaCommand,
                                              C64765_NivisMetaCommand_Resp* resp)
{
    std::ostringstream stream;
    stream << "Handle(C64765_NivisMetaCommand) : ";
    hart7::util::toString(stream, &nivisMetaCommand);
    LOG_DEBUG(stream.str());

    resp->CommandSize = 0;
    resp->Nickname = nivisMetaCommand.Nickname;
    memcpy(resp->DeviceUniqueId, nivisMetaCommand.DeviceUniqueId, 5);

    // parse the inside command
    uint8_t commandsDataBuffer[SUBAPP_MAX_COMMANDS_BUFFER_SIZE];
    WHartCommand commands[SUBAPP_MAX_COMMANDS_COUNT];

    WHartCommandList requests = { 0, commands };
    hart7::stack::WHartPayload apdu(nivisMetaCommand.Command, nivisMetaCommand.CommandSize);
    if (!commonData.stack.subapp.ParsePayload(apdu, commandsDataBuffer, SUBAPP_MAX_COMMANDS_BUFFER_SIZE, requests,
                                              false))
    {
        LOG_ERROR("Handle(C64765_NivisMetaCommand_Req) : Something went wrong while parsing the request ...");
        resp->CommandSize = 0;
        return C64765_ResponseCode_Error_Parsing_Inside_Command;
    }


    WHartAddress deviceAddress(nivisMetaCommand.DeviceUniqueId);

    // we should have only one command in the commands array ...
    try
    {
        uint8_t responseCode = 0;

        switch (commands[0].commandID)
        {
            case CMDID_C000_ReadUniqueIdentifier:
            {
                hart7::util::NMLog::logCommand(CMDID_C000_ReadUniqueIdentifier,
                                               (C000_ReadUniqueIdentifier_Req*) commands[0].command, deviceAddress);
                C000_ReadUniqueIdentifier_Resp readUniqueIdent;

                responseCode = Handle(handle, nivisMetaCommand.DeviceUniqueId,
                                      *((C000_ReadUniqueIdentifier_Req*) commands[0].command), readUniqueIdent);
                hart7::util::NMLog::logCommandResponse(CMDID_C000_ReadUniqueIdentifier, responseCode, &readUniqueIdent,
                                                       deviceAddress);

                ComposeCommand(commands[0].commandID, responseCode, &readUniqueIdent, *resp);

                break;
            }
            case CMDID_C013_ReadTagDescriptorDate:
            {
                hart7::util::NMLog::logCommand(CMDID_C013_ReadTagDescriptorDate,
                                               (C013_ReadTagDescriptorDate_Req*) commands[0].command, deviceAddress);
                C013_ReadTagDescriptorDate_Resp readUniqueIdent;

                responseCode = Handle(handle, nivisMetaCommand.DeviceUniqueId,
                                      *((C013_ReadTagDescriptorDate_Req*) commands[0].command), readUniqueIdent);
                hart7::util::NMLog::logCommandResponse(CMDID_C013_ReadTagDescriptorDate, responseCode,
                                                       &readUniqueIdent, deviceAddress);

                ComposeCommand(commands[0].commandID, responseCode, &readUniqueIdent, *resp);

                break;
            }
            case CMDID_C020_ReadLongTag:
            {
                hart7::util::NMLog::logCommand(CMDID_C020_ReadLongTag, (C020_ReadLongTag_Req*) commands[0].command,
                                               deviceAddress);
                C020_ReadLongTag_Resp readUniqueIdent;

                responseCode = Handle(handle, nivisMetaCommand.DeviceUniqueId,
                                      *((C020_ReadLongTag_Req*) commands[0].command), readUniqueIdent);
                hart7::util::NMLog::logCommandResponse(CMDID_C020_ReadLongTag, responseCode, &readUniqueIdent,
                                                       deviceAddress);

                ComposeCommand(commands[0].commandID, responseCode, &readUniqueIdent, *resp);

                break;
            }
            case CMDID_C769_ReadJoinStatus:
            {
                hart7::util::NMLog::logCommand(CMDID_C769_ReadJoinStatus,
                                               (C769_ReadJoinStatus_Req*) commands[0].command, deviceAddress);
                C769_ReadJoinStatus_Resp readJoinStatus;

                responseCode = Handle(handle, nivisMetaCommand.DeviceUniqueId,
                                      *((C769_ReadJoinStatus_Req*) commands[0].command), readJoinStatus);
                hart7::util::NMLog::logCommandResponse(CMDID_C769_ReadJoinStatus, responseCode, &readJoinStatus,
                                                       deviceAddress);

                ComposeCommand(commands[0].commandID, responseCode, &readJoinStatus, *resp);

                break;
            }
            case CMDID_C773_WriteNetworkId:
            {
                hart7::util::NMLog::logCommand(CMDID_C773_WriteNetworkId,
                                               (C773_WriteNetworkId_Req*) commands[0].command, deviceAddress);
                C773_WriteNetworkId_Resp readJoinStatus;

                responseCode = Handle(handle, nivisMetaCommand.DeviceUniqueId,
                                      *((C773_WriteNetworkId_Req*) commands[0].command), readJoinStatus);
                hart7::util::NMLog::logCommandResponse(CMDID_C773_WriteNetworkId, responseCode, &readJoinStatus,
                                                       deviceAddress);

                ComposeCommand(commands[0].commandID, responseCode, &readJoinStatus, *resp);

                break;
            }
            case CMDID_C774_ReadNetworkId:
            {
                hart7::util::NMLog::logCommand(CMDID_C774_ReadNetworkId, (C774_ReadNetworkId_Req*) commands[0].command,
                                               deviceAddress);
                C774_ReadNetworkId_Resp readJoinStatus;

                responseCode = Handle(handle, nivisMetaCommand.DeviceUniqueId,
                                      *((C774_ReadNetworkId_Req*) commands[0].command), readJoinStatus);
                hart7::util::NMLog::logCommandResponse(CMDID_C774_ReadNetworkId, responseCode, &readJoinStatus,
                                                       deviceAddress);

                ComposeCommand(commands[0].commandID, responseCode, &readJoinStatus, *resp);

                break;
            }
            case CMDID_C775_WriteNetworkTag:
            {
                hart7::util::NMLog::logCommand(CMDID_C775_WriteNetworkTag,
                                               (C775_WriteNetworkTag_Req*) commands[0].command, deviceAddress);
                C775_WriteNetworkTag_Resp readJoinStatus;

                responseCode = Handle(handle, nivisMetaCommand.DeviceUniqueId,
                                      *((C775_WriteNetworkTag_Req*) commands[0].command), readJoinStatus);
                hart7::util::NMLog::logCommandResponse(CMDID_C775_WriteNetworkTag, responseCode, &readJoinStatus,
                                                       deviceAddress);

                ComposeCommand(commands[0].commandID, responseCode, &readJoinStatus, *resp);

                break;
            }
            case CMDID_C776_ReadNetworkTag:
            {
                hart7::util::NMLog::logCommand(CMDID_C776_ReadNetworkTag,
                                               (C776_ReadNetworkTag_Req*) commands[0].command, deviceAddress);
                C776_ReadNetworkTag_Resp readNetworkTag;

                responseCode = Handle(handle, nivisMetaCommand.DeviceUniqueId,
                                      *((C776_ReadNetworkTag_Req*) commands[0].command), readNetworkTag);
                hart7::util::NMLog::logCommandResponse(CMDID_C776_ReadNetworkTag, responseCode, &readNetworkTag,
                                                       deviceAddress);

                ComposeCommand(commands[0].commandID, responseCode, &readNetworkTag, *resp);

                break;
            }
            case CMDID_C778_ReadBatteryLife:
            {
                hart7::util::NMLog::logCommand(CMDID_C778_ReadBatteryLife,
                                               (C778_ReadBatteryLife_Req*) commands[0].command, deviceAddress);
                C778_ReadBatteryLife_Resp readBatteryLife;

                responseCode = Handle(handle, nivisMetaCommand.DeviceUniqueId,
                                      *((C778_ReadBatteryLife_Req*) commands[0].command), readBatteryLife);
                hart7::util::NMLog::logCommandResponse(CMDID_C778_ReadBatteryLife, responseCode, &readBatteryLife,
                                                       deviceAddress);

                ComposeCommand(commands[0].commandID, responseCode, &readBatteryLife, *resp);

                break;
            }
            case CMDID_C779_ReportDeviceHealth:
            {
                hart7::util::NMLog::logCommand(CMDID_C779_ReportDeviceHealth,
                                               (C779_ReportDeviceHealth_Req*) commands[0].command, deviceAddress);
                C779_ReportDeviceHealth_Resp reportNeighborSignalLevels;
                responseCode
                            = Handle(handle, nivisMetaCommand.DeviceUniqueId,
                                     *((C779_ReportDeviceHealth_Req*) commands[0].command), reportNeighborSignalLevels);
                hart7::util::NMLog::logCommandResponse(CMDID_C779_ReportDeviceHealth, responseCode,
                                                       &reportNeighborSignalLevels, deviceAddress);
                ComposeCommand(commands[0].commandID, responseCode, &reportNeighborSignalLevels, *resp);

                break;
            }
            case CMDID_C780_ReportNeighborHealthList:
            {
                hart7::util::NMLog::logCommand(CMDID_C780_ReportNeighborHealthList,
                                               (C780_ReportNeighborHealthList_Req*) commands[0].command, deviceAddress);
                C780_ReportNeighborHealthList_Resp reportNeighborSignalLevels;
                responseCode = Handle(handle, nivisMetaCommand.DeviceUniqueId,
                                      *((C780_ReportNeighborHealthList_Req*) commands[0].command),
                                      reportNeighborSignalLevels);
                hart7::util::NMLog::logCommandResponse(CMDID_C780_ReportNeighborHealthList, responseCode,
                                                       &reportNeighborSignalLevels, deviceAddress);
                ComposeCommand(commands[0].commandID, responseCode, &reportNeighborSignalLevels, *resp);

                break;
            }

            case CMDID_C781_ReadDeviceNicknameAddress:
            {
                hart7::util::NMLog::logCommand(CMDID_C781_ReadDeviceNicknameAddress,
                                               (C781_ReadDeviceNicknameAddress_Req*) commands[0].command, deviceAddress);
                C781_ReadDeviceNicknameAddress_Resp readDeviceNicknameAddress;

                responseCode = Handle(handle, nivisMetaCommand.DeviceUniqueId,
                                      *((C781_ReadDeviceNicknameAddress_Req*) commands[0].command),
                                      readDeviceNicknameAddress);
                hart7::util::NMLog::logCommandResponse(CMDID_C781_ReadDeviceNicknameAddress, responseCode,
                                                       &readDeviceNicknameAddress, deviceAddress);

                ComposeCommand(commands[0].commandID, responseCode, &readDeviceNicknameAddress, *resp);

                break;
            }
            case CMDID_C782_ReadSessionEntries:
            {
                hart7::util::NMLog::logCommand(CMDID_C782_ReadSessionEntries,
                                               (C782_ReadSessionEntries_Req*) commands[0].command, deviceAddress);
                C782_ReadSessionEntries_Resp readSessionEntries;
                responseCode = Handle(handle, nivisMetaCommand.DeviceUniqueId,
                                      *((C782_ReadSessionEntries_Req*) commands[0].command), readSessionEntries);
                hart7::util::NMLog::logCommandResponse(CMDID_C782_ReadSessionEntries, responseCode,
                                                       &readSessionEntries, deviceAddress);
                ComposeCommand(commands[0].commandID, responseCode, &readSessionEntries, *resp);

                break;
            }
            case CMDID_C783_ReadSuperframeList:
            {
                hart7::util::NMLog::logCommand(CMDID_C783_ReadSuperframeList,
                                               (C783_ReadSuperframeList_Req*) commands[0].command, deviceAddress);
                C783_ReadSuperframeList_Resp readSuperframeList;
                responseCode = Handle(handle, nivisMetaCommand.DeviceUniqueId,
                                      *((C783_ReadSuperframeList_Req*) commands[0].command), readSuperframeList);
                hart7::util::NMLog::logCommandResponse(CMDID_C783_ReadSuperframeList, responseCode,
                                                       &readSuperframeList, deviceAddress);
                ComposeCommand(commands[0].commandID, responseCode, &readSuperframeList, *resp);

                break;
            }
            case CMDID_C784_ReadLinkList:
            {
                hart7::util::NMLog::logCommand(CMDID_C784_ReadLinkList, (C784_ReadLinkList_Req*) commands[0].command,
                                               deviceAddress);
                C784_ReadLinkList_Resp readLinkList;
                responseCode = Handle(handle, nivisMetaCommand.DeviceUniqueId,
                                      *((C784_ReadLinkList_Req*) commands[0].command), readLinkList);
                hart7::util::NMLog::logCommandResponse(CMDID_C784_ReadLinkList, responseCode, &readLinkList,
                                                       deviceAddress);
                ComposeCommand(commands[0].commandID, responseCode, &readLinkList, *resp);

                break;
            }
            case CMDID_C785_ReadGraphList:
            {
                hart7::util::NMLog::logCommand(CMDID_C785_ReadGraphList, (C785_ReadGraphList_Req*) commands[0].command,
                                               deviceAddress);
                C785_ReadGraphList_Resp readGraphList;
                responseCode = Handle(handle, nivisMetaCommand.DeviceUniqueId,
                                      *((C785_ReadGraphList_Req*) commands[0].command), readGraphList);
                hart7::util::NMLog::logCommandResponse(CMDID_C785_ReadGraphList, responseCode, &readGraphList,
                                                       deviceAddress);
                ComposeCommand(commands[0].commandID, responseCode, &readGraphList, *resp);

                break;
            }
            case CMDID_C786_ReadNeighborPropertyFlag:
            {
                hart7::util::NMLog::logCommand(CMDID_C786_ReadNeighborPropertyFlag,
                                               (C786_ReadNeighborPropertyFlag_Req*) commands[0].command, deviceAddress);
                C786_ReadNeighborPropertyFlag_Resp readNeighborPropertyFlag;
                responseCode = Handle(handle, nivisMetaCommand.DeviceUniqueId,
                                      *((C786_ReadNeighborPropertyFlag_Req*) commands[0].command),
                                      readNeighborPropertyFlag);
                hart7::util::NMLog::logCommandResponse(CMDID_C786_ReadNeighborPropertyFlag, responseCode,
                                                       &readNeighborPropertyFlag, deviceAddress);
                ComposeCommand(commands[0].commandID, responseCode, &readNeighborPropertyFlag, *resp);

                break;
            }
            case CMDID_C787_ReportNeighborSignalLevels:
            {
                hart7::util::NMLog::logCommand(CMDID_C787_ReportNeighborSignalLevels,
                                               (C787_ReportNeighborSignalLevels_Req*) commands[0].command,
                                               deviceAddress);
                C787_ReportNeighborSignalLevels_Resp reportNeighborSignalLevels;
                responseCode = Handle(handle, nivisMetaCommand.DeviceUniqueId,
                                      *((C787_ReportNeighborSignalLevels_Req*) commands[0].command),
                                      reportNeighborSignalLevels);
                hart7::util::NMLog::logCommandResponse(CMDID_C787_ReportNeighborSignalLevels, responseCode,
                                                       &reportNeighborSignalLevels, deviceAddress);
                ComposeCommand(commands[0].commandID, responseCode, &reportNeighborSignalLevels, *resp);

                break;
            }
            case CMDID_C794_ReadUTCTime:
            {
                hart7::util::NMLog::logCommand(CMDID_C794_ReadUTCTime, (C794_ReadUTCTime_Req*) commands[0].command,
                                               deviceAddress);
                C794_ReadUTCTime_Resp reportNeighborSignalLevels;
                responseCode = Handle(handle, nivisMetaCommand.DeviceUniqueId,
                                      *((C794_ReadUTCTime_Req*) commands[0].command), reportNeighborSignalLevels);
                hart7::util::NMLog::logCommandResponse(CMDID_C794_ReadUTCTime, responseCode,
                                                       &reportNeighborSignalLevels, deviceAddress);
                ComposeCommand(commands[0].commandID, responseCode, &reportNeighborSignalLevels, *resp);

                break;
            }
            case CMDID_C800_ReadServiceList:
            {
                hart7::util::NMLog::logCommand(CMDID_C800_ReadServiceList,
                                               (C800_ReadServiceList_Req*) commands[0].command, deviceAddress);
                C800_ReadServiceList_Resp readServiceList;
                responseCode = Handle(handle, nivisMetaCommand.DeviceUniqueId,
                                      *((C800_ReadServiceList_Req*) commands[0].command), readServiceList);
                hart7::util::NMLog::logCommandResponse(CMDID_C800_ReadServiceList, responseCode, &readServiceList,
                                                       deviceAddress);
                ComposeCommand(commands[0].commandID, responseCode, &readServiceList, *resp);

                break;
            }
            case CMDID_C802_ReadRouteList:
            {
                hart7::util::NMLog::logCommand(CMDID_C802_ReadRouteList, (C802_ReadRouteList_Req*) commands[0].command,
                                               deviceAddress);
                C802_ReadRouteList_Resp readRouteList;
                responseCode = Handle(handle, nivisMetaCommand.DeviceUniqueId,
                                      *((C802_ReadRouteList_Req*) commands[0].command), readRouteList);
                hart7::util::NMLog::logCommandResponse(CMDID_C802_ReadRouteList, responseCode, &readRouteList,
                                                       deviceAddress);
                ComposeCommand(commands[0].commandID, responseCode, &readRouteList, *resp);

                break;
            }
            case CMDID_C803_ReadSourceRoute:
            {
                hart7::util::NMLog::logCommand(CMDID_C803_ReadSourceRoute,
                                               (C803_ReadSourceRoute_Req*) commands[0].command, deviceAddress);
                C803_ReadSourceRoute_Resp readSourceRoute;
                responseCode = Handle(handle, nivisMetaCommand.DeviceUniqueId,
                                      *((C803_ReadSourceRoute_Req*) commands[0].command), readSourceRoute);
                hart7::util::NMLog::logCommandResponse(CMDID_C803_ReadSourceRoute, responseCode, &readSourceRoute,
                                                       deviceAddress);
                ComposeCommand(commands[0].commandID, responseCode, &readSourceRoute, *resp);

                break;
            }
            case CMDID_C814_ReadDeviceListEntries:
            {
                hart7::util::NMLog::logCommand(CMDID_C814_ReadDeviceListEntries,
                                               (C814_ReadDeviceListEntries_Req*) commands[0].command, deviceAddress);
                C814_ReadDeviceListEntries_Resp readSourceRoute;
                responseCode = Handle(handle, nivisMetaCommand.DeviceUniqueId,
                                      *((C814_ReadDeviceListEntries_Req*) commands[0].command), readSourceRoute);
                hart7::util::NMLog::logCommandResponse(CMDID_C814_ReadDeviceListEntries, responseCode,
                                                       &readSourceRoute, deviceAddress);
                ComposeCommand(commands[0].commandID, responseCode, &readSourceRoute, *resp);

                break;
            }
            case CMDID_C817_ReadChannelBlacklist:
            {
                hart7::util::NMLog::logCommand(CMDID_C817_ReadChannelBlacklist,
                                               (C817_ReadChannelBlacklist_Req*) commands[0].command, deviceAddress);
                C817_ReadChannelBlacklist_Resp readSourceRoute;
                responseCode = Handle(handle, nivisMetaCommand.DeviceUniqueId,
                                      *((C817_ReadChannelBlacklist_Req*) commands[0].command), readSourceRoute);
                hart7::util::NMLog::logCommandResponse(CMDID_C817_ReadChannelBlacklist, responseCode, &readSourceRoute,
                                                       deviceAddress);
                ComposeCommand(commands[0].commandID, responseCode, &readSourceRoute, *resp);

                break;
            }
            case CMDID_C818_WriteChannelBlacklist:
            {
                hart7::util::NMLog::logCommand(CMDID_C818_WriteChannelBlacklist,
                                               (C818_WriteChannelBlacklist_Req*) commands[0].command, deviceAddress);
                C818_WriteChannelBlacklist_Resp readSourceRoute;
                responseCode = Handle(handle, nivisMetaCommand.DeviceUniqueId,
                                      *((C818_WriteChannelBlacklist_Req*) commands[0].command), readSourceRoute);
                hart7::util::NMLog::logCommandResponse(CMDID_C818_WriteChannelBlacklist, responseCode,
                                                       &readSourceRoute, deviceAddress);
                ComposeCommand(commands[0].commandID, responseCode, &readSourceRoute, *resp);

                break;
            }
            case CMDID_C821_WriteNetworkAccessMode:
            {
                hart7::util::NMLog::logCommand(CMDID_C821_WriteNetworkAccessMode,
                                               (C821_WriteNetworkAccessMode_Req*) commands[0].command, deviceAddress);
                C821_WriteNetworkAccessMode_Resp readSourceRoute;
                responseCode = Handle(handle, nivisMetaCommand.DeviceUniqueId,
                                      *((C821_WriteNetworkAccessMode_Req*) commands[0].command), readSourceRoute);
                hart7::util::NMLog::logCommandResponse(CMDID_C821_WriteNetworkAccessMode, responseCode,
                                                       &readSourceRoute, deviceAddress);
                ComposeCommand(commands[0].commandID, responseCode, &readSourceRoute, *resp);

                break;
            }
            case CMDID_C822_ReadNetworkAccessMode:
            {
                hart7::util::NMLog::logCommand(CMDID_C822_ReadNetworkAccessMode,
                                               (C822_ReadNetworkAccessMode_Req*) commands[0].command, deviceAddress);
                C822_ReadNetworkAccessMode_Resp readSourceRoute;
                responseCode = Handle(handle, nivisMetaCommand.DeviceUniqueId,
                                      *((C822_ReadNetworkAccessMode_Req*) commands[0].command), readSourceRoute);
                hart7::util::NMLog::logCommandResponse(CMDID_C822_ReadNetworkAccessMode, responseCode,
                                                       &readSourceRoute, deviceAddress);
                ComposeCommand(commands[0].commandID, responseCode, &readSourceRoute, *resp);

                break;
            }
            case CMDID_C832_ReadNetworkDeviceIdentity:
            {
                hart7::util::NMLog::logCommand(CMDID_C832_ReadNetworkDeviceIdentity,
                                               (C832_ReadNetworkDeviceIdentity_Req*) commands[0].command, deviceAddress);
                C832_ReadNetworkDeviceIdentity_Resp readSourceRoute;
                responseCode = Handle(handle, nivisMetaCommand.DeviceUniqueId,
                                      *((C832_ReadNetworkDeviceIdentity_Req*) commands[0].command), readSourceRoute);
                hart7::util::NMLog::logCommandResponse(CMDID_C832_ReadNetworkDeviceIdentity, responseCode,
                                                       &readSourceRoute, deviceAddress);
                ComposeCommand(commands[0].commandID, responseCode, &readSourceRoute, *resp);

                break;
            }
            case CMDID_C833_ReadNetworkDeviceNeighbourHealth:
            {
                hart7::util::NMLog::logCommand(CMDID_C833_ReadNetworkDeviceNeighbourHealth,
                                               (C833_ReadNetworkDeviceNeighbourHealth_Req*) commands[0].command,
                                               deviceAddress);
                C833_ReadNetworkDeviceNeighbourHealth_Resp readNetworkDeviceNeighbourHealth;
                responseCode = Handle(handle, nivisMetaCommand.DeviceUniqueId,
                                      *((C833_ReadNetworkDeviceNeighbourHealth_Req*) commands[0].command),
                                      readNetworkDeviceNeighbourHealth);
                hart7::util::NMLog::logCommandResponse(CMDID_C833_ReadNetworkDeviceNeighbourHealth, responseCode,
                                                       &readNetworkDeviceNeighbourHealth, deviceAddress);
                ComposeCommand(commands[0].commandID, responseCode, &readNetworkDeviceNeighbourHealth, *resp);

                break;
            }
            case CMDID_C834_ReadNetworkTopologyInformation:
            {
                hart7::util::NMLog::logCommand(CMDID_C834_ReadNetworkTopologyInformation,
                                               (C834_ReadNetworkTopologyInformation_Req*) commands[0].command,
                                               deviceAddress);
                C834_ReadNetworkTopologyInformation_Resp readNetworkTopologyInformation;
                responseCode = Handle(handle, nivisMetaCommand.DeviceUniqueId,
                                      *((C834_ReadNetworkTopologyInformation_Req*) commands[0].command),
                                      readNetworkTopologyInformation);
                hart7::util::NMLog::logCommandResponse(CMDID_C834_ReadNetworkTopologyInformation, responseCode,
                                                       &readNetworkTopologyInformation, deviceAddress);
                ComposeCommand(commands[0].commandID, responseCode, &readNetworkTopologyInformation, *resp);

                break;
            }
            case CMDID_C840_ReadDeviceStatistics:
            {
                hart7::util::NMLog::logCommand(CMDID_C840_ReadDeviceStatistics,
                                               (C840_ReadDeviceStatistics_Req*) commands[0].command, deviceAddress);
                C840_ReadDeviceStatistics_Resp readDeviceStatistics;
                responseCode = Handle(handle, nivisMetaCommand.DeviceUniqueId,
                                      *((C840_ReadDeviceStatistics_Req*) commands[0].command), readDeviceStatistics);
                hart7::util::NMLog::logCommandResponse(CMDID_C840_ReadDeviceStatistics, responseCode,
                                                       &readDeviceStatistics, deviceAddress);
                ComposeCommand(commands[0].commandID, responseCode, &readDeviceStatistics, *resp);

                break;
            }
            case CMDID_C843_ReadDeviceSchedulingFlags:
            {
                hart7::util::NMLog::logCommand(CMDID_C843_ReadDeviceSchedulingFlags,
                                               (C843_ReadDeviceSchedulingFlags_Req*) commands[0].command, deviceAddress);
                C843_ReadDeviceSchedulingFlags_Resp readDeviceStatistics;
                responseCode = Handle(handle, nivisMetaCommand.DeviceUniqueId,
                                      *((C843_ReadDeviceSchedulingFlags_Req*) commands[0].command),
                                      readDeviceStatistics);
                hart7::util::NMLog::logCommandResponse(CMDID_C843_ReadDeviceSchedulingFlags, responseCode,
                                                       &readDeviceStatistics, deviceAddress);
                ComposeCommand(commands[0].commandID, responseCode, &readDeviceStatistics, *resp);

                break;
            }
            case CMDID_C844_ReadNetworkConstraints:
            {
                hart7::util::NMLog::logCommand(CMDID_C844_ReadNetworkConstraints,
                                               (C844_ReadNetworkConstraints_Req*) commands[0].command, deviceAddress);
                C844_ReadNetworkConstraints_Resp readDeviceStatistics;
                responseCode = Handle(handle, nivisMetaCommand.DeviceUniqueId,
                                      *((C844_ReadNetworkConstraints_Req*) commands[0].command), readDeviceStatistics);
                hart7::util::NMLog::logCommandResponse(CMDID_C844_ReadNetworkConstraints, responseCode,
                                                       &readDeviceStatistics, deviceAddress);
                ComposeCommand(commands[0].commandID, responseCode, &readDeviceStatistics, *resp);

                break;
            }
            case CMDID_C845_WriteNetworkConstraints:
            {
                hart7::util::NMLog::logCommand(CMDID_C845_WriteNetworkConstraints,
                                               (C845_WriteNetworkConstraints_Req*) commands[0].command, deviceAddress);
                C845_WriteNetworkConstraints_Resp readDeviceStatistics;
                responseCode = Handle(handle, nivisMetaCommand.DeviceUniqueId,
                                      *((C845_WriteNetworkConstraints_Req*) commands[0].command), readDeviceStatistics);
                hart7::util::NMLog::logCommandResponse(CMDID_C845_WriteNetworkConstraints, responseCode,
                                                       &readDeviceStatistics, deviceAddress);
                ComposeCommand(commands[0].commandID, responseCode, &readDeviceStatistics, *resp);

                break;
            }

            default:
            {
                LOG_WARN("Handle(C64765_NivisMetaCommand_Req) for Command " << (int)commands[0].commandID << " is not processed by NM.");
                return C64765_ResponseCode_Command_Not_Treated;
            }
        }

    }
    catch (DeviceNotFoundException& ex)
    {
        LOG_ERROR(ex.what());
        return RCS_E02_InvalidSelection;
    }
    catch (...)
    {
        return RCS_E02_InvalidSelection;
    }

    return C64765_ResponseCode_Success;
}

}
}
