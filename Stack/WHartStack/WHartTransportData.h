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
 * WHartTransport.h
 *
 *  Created on: Nov 19, 2008
 *      Author: andrei.petrut
 */

#ifndef WHART_TRANSPORT_DATA_H
#define WHART_TRANSPORT_DATA_H


#include <WHartStack/WHartStack.h>
#include <WHartStack/WHartCommonData.h>

#include <nlib/log.h>
#include <nlib/socket/Timer.h>

#include <iomanip>
#include <map>
#include <string>

#include <boost/unordered_map.hpp>

namespace hart7 {
namespace stack {
namespace transport {

/**
 *
 */
class TableEntry
{
    public:

        WHartShortAddress peerNickName;
        WHartUniqueID peerUniqueID;
        struct
        {
                uint8_t isActive;
                uint8_t isProxyActive;
                uint8_t isMaster;
                uint8_t isBroadcast;
                uint8_t sequenceNumber;
        } controlByte;

        uint8_t packetCounter;

        std::basic_string<uint8_t> lastTPDU;
        uint8_t retryCount;
        uint32_t timeOutSent; //timeout

        WHartAddress lastDestination;
        WHartServiceID lastServiceID;
        WHartPriority lastPriority;
        WHartSessionKey::SessionKeyCode lastSessionCode;
        WHartHandle lastHandle;

        bool hasProxyLimitation;
        bool haveTx;
        int secondsUntilNextTransmit;

    public:

        TableEntry()
        {
            controlByte.isActive = 0;
            controlByte.isProxyActive = 0;
            controlByte.isMaster = 0;
            controlByte.isBroadcast = 0;
            controlByte.sequenceNumber = 0;
            haveTx = false;
            secondsUntilNextTransmit = 0;
            hasProxyLimitation = 0;
        }

        void toString(std::ostringstream& stream)
        {
            stream << "TableEntry{";
            stream << "peerNickName = " << std::hex << std::setw(8) << std::setfill('0') << (int) peerNickName;
            stream << ", peerUniqueID = " << peerUniqueID;
            stream << ", controlByte = {" << std::dec;
            stream << "isActive = " << (int) controlByte.isActive;
            stream << ", isProxyActive = " << (int) controlByte.isProxyActive;
            stream << ", isMaster = " << (int) controlByte.isMaster;
            stream << ", isBroadcast = " << (int) controlByte.isBroadcast;
            stream << ", sequenceNumber = " << (int) controlByte.sequenceNumber;
            stream << "}";

            stream << ", packetCounter = " << (int) packetCounter;
            stream << ", lastServiceID = " << lastServiceID;
            stream << ", lastPriority = " << lastPriority;
            stream << ", lastSessionCode = " << lastSessionCode;
            stream << ", lastHandle = " << lastHandle;
            stream << ", haveTx = " << haveTx;
            stream << ", secToTx = " << secondsUntilNextTransmit;
            stream << "}";
        }
};

/**
 *
 */
class TrackedHandle
{
    public:
        WHartAddress destAddress;
        WHartPriority priority;
        bool isAckService;
};

/**
 *
 */
class WHartTransportData: public WHartTransport
{
    public:
        static
        bool IsAcknowledgeableType(WHartTransportType type)
        {
            switch (type)
            {
                case wharttTransferRequest:
                case wharttTransferResponse:
                case wharttSearchBroadcast:
                case wharttPublishBroadcast:
                case wharttPublishNotify:
                    return false;
                case wharttRequestUnicast:
                case wharttResponseUnicast:
                case wharttRequestBroadcast:
                case wharttResponseBroadcast:
                    return true;
                default:
                    return false;
            }
        }

        LOG_DEF("h7.s.t.WHartTransportData");
        static const int TIMER_INTERVAL; //seconds
        static const boost::uint32_t TIMEOUT_TIME; //seconds
    public:
        typedef std::map<WHartHandle, TrackedHandle> TrackedHandles;
        typedef boost::unordered_map<WHartShortAddress, TableEntry> TableEntries;
        typedef boost::unordered_map<std::string, WHartShortAddress> UniqueIdToNicknameMap;

    public:
        WHartTransportData(CommonData& commonData);
        ~WHartTransportData();

        // WHartTransport_
    public:
        void TransmitRequest(WHartHandle handle, const WHartAddress& dest, WHartPriority priority,
                             WHartServiceID serviceID, WHartTransportType transportType, const WHartPayload& apdu,
                             WHartSessionKey::SessionKeyCode sessionCode);
        void TransmitConfirm(WHartHandle requestHandle, const WHartLocalStatus& localStatus);
        void TransmitIndicate(WHartHandle handle, const WHartAddress& src, WHartPriority priority,
                              const WHartPayload& tpdu, WHartSessionKey::SessionKeyCode sessionCode);
        void TransmitResponse(WHartHandle indicatedHandle, WHartServiceID serviceID, const WHartPayload& tpdu,
                              WHartSessionKey::SessionKeyCode sessionCode);

        bool CanSendToDestination(const WHartAddress& dest);

        void FlushRequest(WHartHandle handle);
        void FlushConfirm(WHartHandle handle, WHartLocalStatus localStatus);

    public:
        void Reset();
        void TimePassed(int timeInterval);

        bool RemoveTableEntry(const WHartAddress& peerAddress, bool isMaster);
        TableEntries::iterator AddTableEntry(const WHartAddress& peerAddress, bool isMaster);
        TableEntries::iterator AddTableEntryWithUniqueId(WHartUniqueID uniqueID, WHartShortAddress nickname, bool isMaster, bool hasProxyLimitation);

        /**
         * Prints the transport layer table entries to a log.
         */
        void logTableEntries(std::string reason);

    private:
        TableEntries::iterator FindTableEntry(const WHartAddress& address, bool isMaster);
        TrackedHandles::iterator FindTrackedHandle(WHartHandle handle);
        void TimeoutOperation(TableEntries::iterator itEntry);
        void CheckTimePassed(TableEntries::iterator itEntry, int timeIntervalmsecs);
        void SetTableEntrySent(TableEntries::iterator itEntry);
        std::string HashableUniqueID(const WHartUniqueID& uniqueID);

    private:
        CommonData& commonData;

        TrackedHandles trackedHandles;
        TableEntries tableEntriesMaster;
        TableEntries tableEntriesSlave;
        UniqueIdToNicknameMap uniqueIdToNicknames;
        uint8_t deviceStatus;

        uint8_t maxRetryCount;
};

} // namespace transport
} // namespace stack
} // namespace hart7

#endif /* WHART_TRANSPORT_DATA_H */
