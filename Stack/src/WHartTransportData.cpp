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
 * WHartTransport.c
 *
 *  Created on: Nov 19, 2008
 *      Author: Andy
 */

#include <WHartStack/WHartTransportData.h>
#include <WHartStack/util/Binarization.h>

#include <boost/bind.hpp>
#include <stdio.h>
#include <cassert>

namespace hart7 {
namespace stack {
namespace transport {

typedef enum
{
    TH_Mask_Acknowledged = 0x80, TH_Mask_Response = 0x40, TH_Mask_Broadcast = 0x20, TH_Mask_SequenceNumber = 0x1F
} HeaderByteMask;

static
bool IsBroadcastType(WHartTransportType type)
{
    switch (type)
    {
        case wharttTransferRequest:
        case wharttTransferResponse:
        case wharttRequestUnicast:
        case wharttResponseUnicast:
        case wharttPublishNotify:
            return false;
        case wharttSearchBroadcast:
        case wharttPublishBroadcast:
        case wharttRequestBroadcast:
        case wharttResponseBroadcast:
            return true;
        default:
            return true;
    }
}

static
bool IsResponseType(WHartTransportType type)
{
    switch (type)
    {
        case wharttTransferResponse:
        case wharttResponseUnicast:
        case wharttPublishBroadcast:
        case wharttPublishNotify:
        case wharttResponseBroadcast:
            return true;
        case wharttTransferRequest:
        case wharttRequestUnicast:
        case wharttSearchBroadcast:
        case wharttRequestBroadcast:
            return false;
        default:
            assert(false && "Unknown WHartTransportType !!!");
            return true;
    }
}

static WHartShortAddress GetProxyAddress(WHartAddress address)
{
    if (address.type == WHartAddress::whartaProxy)
    {
        return address.address.proxy.nickname;
    }
    else if (address.type == WHartAddress::whartaProxyShort)
    {
        return address.address.proxyShort.nickname;
    }

    return No_Nickname();
}

const boost::uint32_t WHartTransportData::TIMEOUT_TIME = 60 * 1000; //seconds

WHartTransportData::WHartTransportData(CommonData& commonData_) :
    commonData(commonData_)
{
    deviceStatus = 0;
    Reset();
}

WHartTransportData::~WHartTransportData()
{
}

bool WHartTransportData::CanSendToDestination(const WHartAddress& dest)
{

    if (dest.type == WHartAddress::whartaProxy || dest.type == WHartAddress::whartaProxyShort)
    {
        TableEntries::iterator proxy = FindTableEntry(GetProxyAddress(dest), true);

        if (proxy == tableEntriesMaster.end()){

        	LOG_INFO("No TableEntry coresponding to proxy found for destination = " << dest << ".");

        	return false;
        }

        if ((proxy->second.controlByte.isActive ||
        	 proxy->second.controlByte.isProxyActive ||
        	 proxy->second.haveTx) && proxy->second.hasProxyLimitation) {

        	return false;
        }
    }

    TableEntries::iterator tableEntry = FindTableEntry(dest, true);
	if (tableEntry == tableEntriesMaster.end()) {
		return true;
	}

	if (tableEntry->second.controlByte.isActive ||
		tableEntry->second.controlByte.isProxyActive ||
		tableEntry->second.haveTx) {

		return false;
	}

    return true;

}

void WHartTransportData::TransmitRequest(WHartHandle handle, const WHartAddress& dest, WHartPriority priority,
                                         WHartServiceID serviceID, WHartTransportType transportType,
                                         const WHartPayload& apdu, WHartSessionKey::SessionKeyCode sessionCode)
{
//    LOG_DEBUG("TransmitRequest handle=" << handle << ", dest=" << dest << ", serviceID=" << serviceID
//                << ", transportType=" << ToString(transportType) << ", TPDU=" << apdu << ", isAck="
//                << IsAcknowledgeableType(transportType));

    if (IsAcknowledgeableType(transportType))
    {
        //		assert((dest.type == WHartAddress::whartaNickname || dest.type == WHartAddress::whartaProxy) && "Expected nickname address.");
        //		const WHartShortAddress& destNickname = dest.type == WHartAddress::whartaProxy ? dest.address.proxy.nickname : dest.address.nickname;
//        LOG_DEBUG("Transmit request is acknowledgeable type...");

        TableEntries::iterator tableEntry = FindTableEntry(dest, true);
        if (tableEntry == tableEntriesMaster.end())
        {
            LOG_INFO("No entry for dest=" << dest << ". Creating new one...");

            tableEntry = AddTableEntry(dest, true);

            if (tableEntry == tableEntriesMaster.end())
            {
            	LOG_WARN ("Expected a nickname destination address but received instead address: " << dest);
				upper->TransmitConfirm(handle, WHartLocalStatus(WHartLocalStatus::whartlsError_TL_DestinationHasNoNickname),
						DeviceIndicatedStatus(), WHartPayload());
				return;
			}
        }

        if (tableEntry->second.controlByte.isActive || tableEntry->second.controlByte.isProxyActive)
        {
            //LOG_DEBUG("Table entry is already active or proxy active for dest=" << dest << " with packetHandle="
            //            << tableEntry->lastHandle);

            upper->TransmitConfirm(handle, WHartLocalStatus(WHartLocalStatus::whartlsError_TL_TableEntryAlreadyActive),
                                   DeviceIndicatedStatus(), WHartPayload());
            return; //table already in use
        }

        if (tableEntry->second.haveTx)
        {
            //LOG_DEBUG("Table entry is already in temporized TX for dest=" << dest << " with packetHandle="
            //            << tableEntry->lastHandle);

            upper->TransmitConfirm(handle, WHartLocalStatus(WHartLocalStatus::whartlsError_TL_TableEntryAlreadyActive),
                                   DeviceIndicatedStatus(), WHartPayload());
            return; //table already has packet
        }

        TableEntries::iterator proxyEntry = tableEntriesMaster.end();
        if ((dest.type == WHartAddress::whartaProxy) || (dest.type == WHartAddress::whartaProxyShort))
        {
            proxyEntry = FindTableEntry(GetProxyAddress(dest), true);
            if (proxyEntry == tableEntriesMaster.end() ||
            	((proxyEntry->second.controlByte.isActive ||
            	  proxyEntry->second.controlByte.isProxyActive ||
            	  proxyEntry->second.haveTx) && proxyEntry->second.hasProxyLimitation)
            	)
            {
                upper->TransmitConfirm(handle,
                                       WHartLocalStatus(WHartLocalStatus::whartlsError_TL_TableEntryAlreadyActive),
                                       DeviceIndicatedStatus(), WHartPayload());
                return; //table already has packet
            }
        }

        uint8_t sequenceNumber = ((tableEntry->second.controlByte.sequenceNumber + 1) & 0x1F);

        //create TPDU
        uint8_t buffer[128];
        BinaryStream stream;
        STREAM_INIT(&stream, buffer, sizeof(buffer));

        uint8_t transportByte = 0x00;
        transportByte |= (uint8_t) TH_Mask_Acknowledged;
        transportByte |= (uint8_t)(IsBroadcastType(transportType) ? TH_Mask_Broadcast : 0);
        transportByte |= (uint8_t)(sequenceNumber & TH_Mask_SequenceNumber);

        STREAM_WRITE_UINT8(&stream, transportByte);
        STREAM_WRITE_UINT8(&stream, deviceStatus);
        //TODO replace status
        STREAM_WRITE_UINT8(&stream, 0); // device extended status
        STREAM_WRITE_BYTES(&stream, apdu.data, apdu.dataLen);

//        LOG_DEBUG("Updating transport table with id=" << dest << "...");
        WHartPayload tpdu(buffer, stream.nextByte - buffer);

        tableEntry->second.controlByte.isActive = 1;
        tableEntry->second.controlByte.isMaster = 1;
        tableEntry->second.controlByte.isBroadcast = IsBroadcastType(transportType);
        tableEntry->second.controlByte.sequenceNumber = sequenceNumber;
        tableEntry->second.retryCount = maxRetryCount;
        tableEntry->second.timeOutSent = TIMEOUT_TIME;
        tableEntry->second.lastServiceID = serviceID;
        tableEntry->second.lastPriority = priority;
        tableEntry->second.lastHandle = handle;
        tableEntry->second.lastSessionCode = sessionCode;
        tableEntry->second.lastTPDU.assign(tpdu.data, tpdu.dataLen);
        tableEntry->second.lastDestination = dest;

        // [andy] - set proxy entry active to avoid more packets sent to the same proxy
        if (proxyEntry != tableEntriesMaster.end() && proxyEntry->second.hasProxyLimitation)
        {
            proxyEntry->second.controlByte.isProxyActive = 1;
            proxyEntry->second.timeOutSent = (maxRetryCount + 1) * TIMEOUT_TIME; // max timeout
//            LOG_DEBUG("Set proxy for device" << dest << "active with timeout = " << (maxRetryCount + 1) * TIMEOUT_TIME <<".");
        }

        // insert handle into tracked handles
        //        TrackedHandle trHandle = { dest, priority, true };
        //        trackedHandles.insert(std::make_pair(handle, trHandle));

        if (tableEntry->second.secondsUntilNextTransmit > 0)
        {
//            LOG_DEBUG("Temporizing packet to destination " << dest);
            tableEntry->second.haveTx = true;
            tableEntry->second.controlByte.isActive = 0;
            tableEntry->second.controlByte.isProxyActive = 0;
        }
        else
        {
//            LOG_DEBUG("Forwarding TPDU= " << tpdu << " to lower layer...");
            lower->TransmitRequest(handle, dest, priority, serviceID, tpdu, sessionCode);
            SetTableEntrySent(tableEntry);
        }
    }
    else
    {
//        LOG_DEBUG("Transmit request is not acknowledgeable type...");
        //create TPDU
        uint8_t buffer[128];
        BinaryStream stream;
        STREAM_INIT(&stream, buffer, sizeof(buffer));

        uint8_t transportByte = 0x00;

        transportByte |= IsBroadcastType(transportType) ? TH_Mask_Broadcast : 0;
        transportByte |= handle & TH_Mask_SequenceNumber;

        transportByte |= IsResponseType(transportType) ? TH_Mask_Response : 0;

        //TODO:[andy] - duplicated serialization code... move in one single place

        STREAM_WRITE_UINT8(&stream, transportByte);
        STREAM_WRITE_UINT8(&stream, deviceStatus);
        STREAM_WRITE_UINT8(&stream, 0); // device extended status
        STREAM_WRITE_BYTES(&stream, apdu.data, apdu.dataLen);

        // insert handle into tracked handles
        //        TrackedHandle trHandle = { dest, priority, false };
        //        trackedHandles.insert(std::make_pair(handle, trHandle));

        lower->TransmitRequest(handle, dest, priority, serviceID, WHartPayload(buffer, stream.nextByte - buffer),
                               sessionCode);
    }
}

void WHartTransportData::TransmitConfirm(WHartHandle handle, const WHartLocalStatus& localStatus)
{
//    LOG_DEBUG("TransmitConfirm");
    TrackedHandles::iterator trackedHandle = FindTrackedHandle(handle);
    if (trackedHandle == trackedHandles.end())
    {
        LOG_DEBUG("Cannot find tracked handle need for TransmitConfirm in transport layer!");
        return;
    }

    if (!trackedHandle->second.isAckService)
    {
        // not have table entry, so forward upper
        upper->TransmitConfirm(handle, localStatus, DeviceIndicatedStatus(), WHartPayload());
        return;
    }

    //	assert(trackedHandle->second.destAddress.type == WHartAddress::whartaNickname && "Expected nickname address.");

    const WHartAddress& address = trackedHandle->second.destAddress;

    TableEntries::iterator tableEntry = FindTableEntry(address, true);

    if (tableEntry != tableEntriesMaster.end())
    {
        if (localStatus.status >= WHartLocalStatus::whartlsError_Start)
        {
            tableEntry->second.controlByte.isActive = 0;
            //LOG_DEBUG("Set device at address " << address << " inactive.");

            if (tableEntry->second.lastDestination.type == WHartAddress::whartaProxy || tableEntry->second.lastDestination.type
                        == WHartAddress::whartaProxyShort)
            {
                TableEntries::iterator proxy = FindTableEntry(GetProxyAddress(tableEntry->second.lastDestination), true);
                if (proxy != tableEntriesMaster.end())
                {
                    proxy->second.controlByte.isProxyActive = 0;
                    //LOG_DEBUG("Set proxy for device " << address
                    //    << " inactive. Proxy device last dest: " << proxy->lastDestination <<".");
                }
            }
            upper->TransmitConfirm(handle, localStatus, DeviceIndicatedStatus(), WHartPayload());
        }
    } else {
    	LOG_DEBUG("Could not find table entry for device: " << address << " during TransmitConfirm in transport layer.");
    }
}

void WHartTransportData::TransmitIndicate(WHartHandle handle, const WHartAddress& srcAddress, WHartPriority priority,
                                          const WHartPayload& tpdu, WHartSessionKey::SessionKeyCode sessionCode)
{
//    LOG_DEBUG("TransmitIndicate: handle=" << handle << ", srcAddress=" << srcAddress << ", priority="
//                << priority << ", TPDU=" << tpdu);

    BinaryStream stream;
    STREAM_INIT(&stream, tpdu.data, tpdu.dataLen);

    uint8_t transportByte;
    STREAM_READ_UINT8(&stream, &transportByte);
    uint8_t peerDeviceStatus;
    STREAM_READ_UINT8(&stream, &peerDeviceStatus);
    uint8_t peerExtendedDeviceStatus;
    STREAM_READ_UINT8(&stream, &peerExtendedDeviceStatus);

    DeviceIndicatedStatus status = { peerDeviceStatus, peerExtendedDeviceStatus };

    bool isAcknowlededService = (transportByte & TH_Mask_Acknowledged) != 0;
    bool isResponse = (transportByte & TH_Mask_Response) != 0;
    uint8_t sequenceNumber = (transportByte & TH_Mask_SequenceNumber);

//    LOG_DEBUG("TransmitIndicate: is Acknowledged=" << isAcknowlededService << ", isResponse=" << isResponse
//                << ", sequenceNumber=" << (int) sequenceNumber);

    if (!isAcknowlededService)
    {
        //NOT ACKNOWLEDGED SERVICE
        WHartTransportType transportType = isResponse ? wharttPublishNotify : wharttRequestUnicast;

        upper->TransmitIndicate(handle, srcAddress, priority, status, transportType,
                                WHartPayload(stream.nextByte, stream.remainingBytes), sessionCode);
        return;
    }

    if (!isResponse)
    {
        TableEntries::iterator tableEntry = FindTableEntry(srcAddress, false);
        if (tableEntry == tableEntriesSlave.end())
        {
            tableEntry = AddTableEntry(srcAddress, false);

            if (tableEntry == tableEntriesSlave.end()) {

            	LOG_WARN ("Expected a nickname source address but received instead address: " << srcAddress);
            	return;
            }

            tableEntry->second.controlByte.sequenceNumber = (sequenceNumber - 1) & 0x1F;
        }

        //IS ACKNOWLEDGED REQUEST
        if (tableEntry->second.controlByte.sequenceNumber == sequenceNumber)
        {
        	LOG_DEBUG("Equal SN, resending response.");
            //resend response
            lower->TransmitRequest(handle, srcAddress, priority, tableEntry->second.lastServiceID,
                                   WHartPayload(tableEntry->second.lastTPDU.data(), tableEntry->second.lastTPDU.size()), sessionCode);
        }
        else if (tableEntry->second.controlByte.sequenceNumber == ((sequenceNumber - 1) & 0x1F))
        {
            // is first time, or new request
            TrackedHandle trackedHandle;
            trackedHandle.destAddress = srcAddress;
            trackedHandle.priority = priority;
            trackedHandle.isAckService = true;
            trackedHandles[handle] = trackedHandle;

            tableEntry->second.controlByte.sequenceNumber = sequenceNumber;

//            LOG_DEBUG("TransmitIndicate to upper");
            upper->TransmitIndicate(handle, srcAddress, priority, status, wharttRequestUnicast,
                                    WHartPayload(stream.nextByte, stream.remainingBytes), sessionCode);
        }
        else
        {
            LOG_WARN("Wrong sequence number received. Current=" << (int) tableEntry->second.controlByte.sequenceNumber
                        << ", received=" << (int) sequenceNumber << " in packet from " << srcAddress);
            return;
        }
    }
    else
    {
        TableEntries::iterator tableEntry = FindTableEntry(srcAddress, true);
        if (tableEntry == tableEntriesMaster.end())
        {
            LOG_WARN("Cannot find table entry for srcAddress=" << srcAddress << ". Skipping packet...");
            return;
        }
        //is response, and should be transformed in TransmitConfirm
        if (tableEntry->second.controlByte.isActive)
        {
            if (tableEntry->second.controlByte.sequenceNumber == sequenceNumber)
            {
//                LOG_DEBUG("TransmitConfirm call to upper.");
                tableEntry->second.controlByte.isActive = 0;

                if (tableEntry->second.lastDestination.type == WHartAddress::whartaProxy || tableEntry->second.lastDestination.type
                            == WHartAddress::whartaProxyShort)
                {
                    TableEntries::iterator proxy = FindTableEntry(GetProxyAddress(tableEntry->second.lastDestination), true);
                    if (proxy != tableEntriesMaster.end() && proxy->second.hasProxyLimitation)
                    {
                        proxy->second.controlByte.isProxyActive = 0;
//                        LOG_DEBUG("Set proxy for device " << srcAddress << " inactive. Proxy last dest: " << proxy->lastDestination <<".");
                    }
                }

                upper->TransmitConfirm(tableEntry->second.lastHandle, WHartLocalStatus(), status,
                                       WHartPayload(stream.nextByte, stream.remainingBytes));
            }
            else
            {
                LOG_WARN("TransmitIndicate: Expected seqNumber=" << (int) tableEntry->second.controlByte.sequenceNumber
                            << " Actual seqNumber=" << (int) sequenceNumber << " in packet from " << srcAddress);
            }
        }
        else
        {
            LOG_WARN("Received a response on an inactive table entry (probably duplicate). Skipping...");
        }
    }
}

void WHartTransportData::TransmitResponse(WHartHandle handle, WHartServiceID serviceID, const WHartPayload& pdu,
                                          WHartSessionKey::SessionKeyCode sessionCode)
{
    TrackedHandles::iterator trackedHandleIt = FindTrackedHandle(handle);
    if (trackedHandleIt == trackedHandles.end())
    {
        LOG_ERROR("TransmitResponse: Unmatched handle=" << handle);
        upper->TransmitConfirm(handle, WHartLocalStatus(WHartLocalStatus::whartlsError_Start + 14),
                               DeviceIndicatedStatus(), WHartPayload());
        return;
    }

    TrackedHandle trackedHandle = trackedHandleIt->second;
    trackedHandles.erase(handle);
    //    if (trackedHandle->second.isAckService)
    {
        //is acknowledged
        //		assert(trackedHandle->second.destAddress.type == WHartAddress::whartaNickname && "Expected nickname address.");

        const WHartAddress& address = trackedHandle.destAddress;
        TableEntries::iterator tableEntry = FindTableEntry(address, false);
        if (tableEntry == tableEntriesSlave.end())
        {
            tableEntry = AddTableEntry(address, false);

            if (tableEntry == tableEntriesSlave.end()){

            	LOG_WARN ("Expected a nickname destination address but received instead address: " << address);
            	upper->TransmitConfirm(handle, WHartLocalStatus(WHartLocalStatus::whartlsError_TL_DestinationHasNoNickname),
            			DeviceIndicatedStatus(), WHartPayload());
            	return;
            }
        }

        uint8_t buffer[128];
        BinaryStream stream;
        STREAM_INIT(&stream, buffer, sizeof(buffer));

        uint8_t transportByte = (uint8_t)(TH_Mask_Response | TH_Mask_Acknowledged
                    | tableEntry->second.controlByte.sequenceNumber);
        STREAM_WRITE_UINT8(&stream, transportByte);
        STREAM_WRITE_UINT8(&stream, deviceStatus);
        STREAM_WRITE_UINT8(&stream, 0); // device extended status

        STREAM_WRITE_BYTES(&stream, pdu.data, pdu.dataLen);

        //        tableEntry->controlByte.isActive = 1;
        tableEntry->second.controlByte.isMaster = 0;
        tableEntry->second.retryCount = 0;
        tableEntry->second.timeOutSent = TIMEOUT_TIME;
        tableEntry->second.lastTPDU.clear();
        tableEntry->second.lastTPDU.append(buffer, stream.nextByte - &buffer[0]);
        tableEntry->second.lastServiceID = serviceID;

        lower->TransmitRequest(handle, trackedHandle.destAddress, trackedHandle.priority, serviceID,
                               WHartPayload(tableEntry->second.lastTPDU), sessionCode);
    }
    //    else
    //    {
    //not acknowledged
    //        uint8_t buffer[128];
    //        BinaryStream stream;
    //        STREAM_INIT(&stream, buffer, sizeof(buffer));
    //
    //        uint8_t transportByte = (uint8_t)(TH_Mask_Response | (handle & 0x1F));
    //        STREAM_WRITE_UINT8(&stream, transportByte);
    //        STREAM_WRITE_UINT8(&stream, deviceStatus);
    //        STREAM_WRITE_UINT8(&stream, 0); // device extended status
    //        STREAM_WRITE_BYTES(&stream, pdu.data, pdu.dataLen);
    //
    //        lower->TransmitRequest(handle, trackedHandle->second.destAddress, trackedHandle->second.priority, serviceID,
    //                               WHartPayload(buffer, stream.nextByte - buffer), sessionCode);
    //    }
}

WHartTransportData::TableEntries::iterator WHartTransportData::AddTableEntry(const WHartAddress& peerAddress, bool isMaster)
{
	if (peerAddress.type != WHartAddress::whartaNickname) {

		if (isMaster) return tableEntriesMaster.end();
		else return tableEntriesSlave.end();
	}

    bool isNewEntry = false;

    TableEntries::iterator entry = FindTableEntry(peerAddress, isMaster);
    if (isMaster && entry == tableEntriesMaster.end()) {

		tableEntriesMaster.insert(std::pair<WHartShortAddress, TableEntry>(
				peerAddress.address.nickname, TableEntry()));
		entry = tableEntriesMaster.find(peerAddress.address.nickname);
		isNewEntry = true;
		LOG_DEBUG("Add Master TableEntry for=" << peerAddress);

	} else if (!isMaster && entry == tableEntriesSlave.end()) {

		tableEntriesSlave.insert(std::pair<WHartShortAddress, TableEntry>(
				peerAddress.address.nickname, TableEntry()));
		entry = tableEntriesSlave.find(peerAddress.address.nickname);
		isNewEntry = true;
		LOG_DEBUG("Add Slave TableEntry for=" << peerAddress);

	} else {

		if (entry->second.controlByte.isActive || entry->second.haveTx) {
			TimeoutOperation(entry); // just in case
		}
	}

    entry->second.controlByte.isMaster = isMaster;
    entry->second.controlByte.isActive = 0;
    entry->second.controlByte.isProxyActive = 0;
    entry->second.controlByte.isBroadcast = 0; // todo
    entry->second.controlByte.sequenceNumber = 0;
    entry->second.packetCounter = 0;
    entry->second.hasProxyLimitation = true;

    return entry;
}

WHartTransportData::TableEntries::iterator WHartTransportData::AddTableEntryWithUniqueId(WHartUniqueID uniqueID,
		WHartShortAddress nickname, bool isMaster, bool hasProxyLimitation)
{
	LOG_DEBUG("Creating transport table addresses: uniqueID=" << WHartAddress(uniqueID) << " and nick="
	                << WHartAddress(nickname));

    bool isNewEntry = false;
    WHartAddress peerAddress(nickname);

    TableEntries::iterator entry = FindTableEntry(nickname, isMaster);
    if (isMaster && entry == tableEntriesMaster.end())
    {

		tableEntriesMaster.insert(std::pair<WHartShortAddress, TableEntry>(
				nickname, TableEntry()));
		entry = tableEntriesMaster.find(nickname);
		isNewEntry = true;
		LOG_DEBUG("Add Master TableEntry for=" << peerAddress);

	} else if (!isMaster && entry == tableEntriesSlave.end()) {

		tableEntriesSlave.insert(std::pair<WHartShortAddress, TableEntry>(
				nickname, TableEntry()));
		entry = tableEntriesSlave.find(nickname);
		isNewEntry = true;
		LOG_DEBUG("Add Slave TableEntry for=" << peerAddress);

	} else {

		if (entry->second.controlByte.isActive || entry->second.haveTx) {
			TimeoutOperation(entry); // just in case
		}
	}

    entry->second.peerNickName = nickname;
    entry->second.peerUniqueID = uniqueID;
    entry->second.controlByte.isMaster = isMaster;
    entry->second.controlByte.isActive = 0;
    entry->second.controlByte.isProxyActive = 0;
    entry->second.controlByte.isBroadcast = 0; // todo
    entry->second.controlByte.sequenceNumber = 0;
    entry->second.packetCounter = 0;
    entry->second.hasProxyLimitation = hasProxyLimitation;

    if (isNewEntry) uniqueIdToNicknames[HashableUniqueID(uniqueID)] = nickname;

    return entry;
}

bool WHartTransportData::RemoveTableEntry(const WHartAddress& peerAddress, bool isMaster)
{
    TableEntries::iterator entry = FindTableEntry(peerAddress, isMaster);
    if ((isMaster && entry != tableEntriesMaster.end()) || (!isMaster && entry != tableEntriesSlave.end()))
    {
        if (entry->second.controlByte.isActive || entry->second.haveTx)
        {
            TimeoutOperation(entry); // just in case
        }

        if (isMaster) tableEntriesMaster.erase(entry);
        else tableEntriesSlave.erase(entry);

        UniqueIdToNicknameMap::iterator it = uniqueIdToNicknames.find(HashableUniqueID(peerAddress.address.uniqueID));
        if (it != uniqueIdToNicknames.end ())
        {
        	uniqueIdToNicknames.erase(it);
        }

        std::ostringstream stream;
        stream << "Removed entry for " << peerAddress;
        logTableEntries(stream.str());

        return true;
    }

    return false;
}

void WHartTransportData::FlushRequest(WHartHandle handle)
{
    //	TransportTableEntry *entry;
    //
    //	entry = GetTableEntryByHandle(_this, handle);
    //	if (entry)
    //	{
    //		entry->ControlByte &= ~Mask_Active; // mark as not active so it will not be retransmitted
    //	}
}

void WHartTransportData::FlushConfirm(WHartHandle handle, WHartLocalStatus localStatus)
{
}

void WHartTransportData::CheckTimePassed(TableEntries::iterator itEntry, int timeIntervalmsecs)
{
	if (itEntry->second.controlByte.isMaster && itEntry->second.secondsUntilNextTransmit > 0)
	        {
		itEntry->second.secondsUntilNextTransmit -= timeIntervalmsecs;
		if (itEntry->second.secondsUntilNextTransmit <= 0) {
			itEntry->second.secondsUntilNextTransmit = 0;
		}
	}
	if (itEntry->second.controlByte.isActive
			&& itEntry->second.controlByte.isMaster) {
		itEntry->second.timeOutSent -= timeIntervalmsecs;
		if (itEntry->second.timeOutSent <= 0) {
			//timeout request
			if (itEntry->second.retryCount > 0) // we should retry
			{
				LOG_INFO("Packet with handle=" << itEntry->second.lastHandle
						<< " to destination="
						<< itEntry->second.lastDestination
						<< " timeouted. Resending (remaining="
						<< (int) itEntry->second.retryCount << ").");
				//CHECK: how to get the destination ???
				//					const WHartAddress addressDest(itEntry->peerUniqueID);
				WHartPayload payload(itEntry->second.lastTPDU);
				lower->TransmitRequest(itEntry->second.lastHandle,
						itEntry->second.lastDestination,
						itEntry->second.lastPriority,
						itEntry->second.lastServiceID, payload,
						itEntry->second.lastSessionCode);

				itEntry->second.timeOutSent = TIMEOUT_TIME;
				itEntry->second.retryCount--;
			} else {
				TimeoutOperation(itEntry);
			}
		}
	} else if (itEntry->second.haveTx && itEntry->second.controlByte.isMaster
			&& itEntry->second.secondsUntilNextTransmit == 0) {
		LOG_DEBUG("Temporized packet is being sent...");
		WHartPayload payload(itEntry->second.lastTPDU);
		lower->TransmitRequest(itEntry->second.lastHandle,
				itEntry->second.lastDestination, itEntry->second.lastPriority,
				itEntry->second.lastServiceID, payload,
				itEntry->second.lastSessionCode);
		SetTableEntrySent(itEntry);
	}
}

void WHartTransportData::TimePassed(int timeIntervalmsecs)
{
    for (TableEntries::iterator itEntry = tableEntriesMaster.begin(); itEntry != tableEntriesMaster.end(); itEntry++)
    {
    	CheckTimePassed(itEntry, timeIntervalmsecs);
    }

    for (TableEntries::iterator itEntry = tableEntriesSlave.begin(); itEntry != tableEntriesSlave.end(); itEntry++)
    {
		CheckTimePassed(itEntry, timeIntervalmsecs);
	}
}

void WHartTransportData::SetTableEntrySent(TableEntries::iterator itEntry)
{
    itEntry->second.secondsUntilNextTransmit = lower->GetServicePeriod(itEntry->second.lastServiceID, itEntry->second.lastDestination);
    itEntry->second.haveTx = false;
  	itEntry->second.controlByte.isActive = true;

    LOG_DEBUG("Set TableEntry SecToTx=" << itEntry->second.secondsUntilNextTransmit);
}

void WHartTransportData::TimeoutOperation(TableEntries::iterator itEntry)
{
    WHartPayload payload(itEntry->second.lastTPDU);
    LOG_ERROR("Packet with handle=" << itEntry->second.lastHandle << " to destination=" << itEntry->second.lastDestination
                << " timeouted, remaining=0. Dropping...");

    //drop the packet
    itEntry->second.controlByte.isActive = 0;
    itEntry->second.haveTx = 0;
    if (itEntry->second.lastDestination.type == WHartAddress::whartaProxy ||
    	itEntry->second.lastDestination.type == WHartAddress::whartaProxyShort)
    {
        TableEntries::iterator proxy = FindTableEntry(GetProxyAddress(itEntry->second.lastDestination), true);
        if ((proxy != tableEntriesMaster.end() || proxy != tableEntriesSlave.end()) && proxy->second.hasProxyLimitation)
        {
            proxy->second.controlByte.isProxyActive = 0;
        }
    }

    upper->TransmitConfirm(itEntry->second.lastHandle, WHartLocalStatus(WHartLocalStatus::whartlsError_TL_Timeout),
                           DeviceIndicatedStatus(), payload);
}

WHartTransportData::TableEntries::iterator WHartTransportData::FindTableEntry(const WHartAddress& peer, bool isMaster)
{

	TableEntries::iterator it;
	if (isMaster) it = tableEntriesMaster.end();
	else it = tableEntriesSlave.end();

	if (peer.type == WHartAddress::whartaNickname){

		if (isMaster) it = tableEntriesMaster.find(peer.address.nickname);
		else it = tableEntriesSlave.find(peer.address.nickname);

	} else if (peer.type == WHartAddress::whartaProxyShort) {

		if (isMaster) it = tableEntriesMaster.find(peer.address.proxyShort.destNickname);
		else it = tableEntriesSlave.find(peer.address.proxyShort.destNickname);

	} else if (peer.type == WHartAddress::whartaUniqueID) {

		UniqueIdToNicknameMap::iterator mapit = uniqueIdToNicknames.find(HashableUniqueID(peer.address.uniqueID));
		if (mapit != uniqueIdToNicknames.end ())
		{
			WHartShortAddress nickname  =  mapit->second;
			if (isMaster) it = tableEntriesMaster.find(nickname);
			else it = tableEntriesSlave.find(nickname);
		}

	} else if (peer.type == WHartAddress::whartaProxy) {

		 UniqueIdToNicknameMap::iterator mapit = uniqueIdToNicknames.find(HashableUniqueID(peer.address.proxy.uniqueID));
		 if (mapit != uniqueIdToNicknames.end())
		 {
			WHartShortAddress nickname = mapit->second;
			if (isMaster) it = tableEntriesMaster.find(nickname);
			else it = tableEntriesSlave.find(nickname);
		 }

	}

    return it;
}


WHartTransportData::TrackedHandles::iterator WHartTransportData::FindTrackedHandle(WHartHandle handle)
{
    return trackedHandles.find(handle);
}

void WHartTransportData::Reset()
{
    maxRetryCount = 3;
    trackedHandles.clear();
    tableEntriesMaster.clear();
    tableEntriesSlave.clear();

    commonData.isJoined = false;
}

void WHartTransportData::logTableEntries(std::string reason)
{
/*
    std::ostringstream stream;
    stream << "TableEntries : (" << reason << ") {";

    TableEntries::iterator it = tableEntriesMaster.begin();
    for (; it != tableEntriesMaster.end(); ++it)
    {
        stream << std::endl;
        it->second.toString(stream);
    }

    it = tableEntriesSlave.begin();
    for (; it != tableEntriesSlave.end(); ++it)
    {
		stream << std::endl;
		it->second.toString(stream);
	}

    stream << std::endl;
    stream << "}";

    static struct LogTransportTableEntries
    {
        public:
            LOG_DEF("LogTransportTableEntries");

            void LogTable(std::ostringstream& stream)
            {
                LOG_INFO(stream.str());
            }
    } logTransport;

    logTransport.LogTable(stream);
 */
}

std::string  WHartTransportData::HashableUniqueID(const WHartUniqueID& uniqueId)
{
	char buff[32];
	memset(buff, 0, 32);

	sprintf(buff,"%x%x%x%x%x", uniqueId.bytes[0], uniqueId.bytes[1], uniqueId.bytes[2], uniqueId.bytes[3], uniqueId.bytes[4]);

	return std::string(buff);
}

} // namespace transport
} // namespace stack
} // namespace hart7
