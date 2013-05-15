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

#ifndef WHART_STACK_H_
#define WHART_STACK_H_

#include <WHartStack/WHartTypes.h>
#include <cstring>


namespace hart7 {
namespace stack {

//b15 b14 bits used for type: 0 -> From App, 1 -> from NL, 2 -> from DLL
//b13 - tell if needs notif from DLL
//b12...b0 bits used for sequence
typedef uint16_t WHartHandle;

#define IS_WHARTHANDLE_APP(handle) ((handle & 0xC000) == 0)
#define IS_WHARTHANDLE_NL(handle) ((handle & 0xC000) == 0x4000)
#define IS_WHARTHANDLE_DLL(handle) ((handle & 0xC000) == 0x8000)

#define MAKE_WHARTHANDLE(type, seq)	(((type & 0x3) << 14) | (0x1FFF & seq))

const WHartUniqueID& Zero_UniqueID();

const WHartShortAddress NetworkManager_Nickname();
const WHartUniqueID& NetworkManager_UniqueID();

const WHartShortAddress Gateway_Nickname();
const WHartUniqueID& Gateway_UniqueID();
const uint32_t Gateway_DeviceID24();
const uint16_t Gateway_ExpandedType();

const WHartShortAddress Broadcast_Nickname();
const WHartShortAddress No_Nickname();

/**
 *
 */
struct WHartAddress
{
	enum AddressType
	{
		whartaNickname = 1,
		whartaUniqueID = 2,
		whartaProxy = 3,
        whartaProxyShort = 4
	};

	AddressType type;
	union
	{
		WHartShortAddress nickname;
		WHartUniqueID uniqueID;
		struct
		{
			WHartShortAddress nickname;
			WHartUniqueID uniqueID;
		} proxy;
		struct
		{
		    WHartShortAddress nickname;
		    WHartShortAddress destNickname;
		} proxyShort;
	} address;

	WHartAddress()
	{
		type = whartaNickname;
		address.nickname = No_Nickname();
	}

	WHartAddress(const WHartShortAddress& right)
	{
		type = whartaNickname;
		address.nickname = right;
	}

	WHartAddress(const WHartUniqueID& right)
	{
		type = whartaUniqueID;
		address.uniqueID = right;
	}

	WHartAddress(const uint8_t bytes[5])
	{
		type = whartaUniqueID;
		memcpy(address.uniqueID.bytes, bytes, 5);
	}

	WHartAddress(const WHartShortAddress& proxy, const WHartUniqueID& right)
	{
		type = whartaProxy;
		address.proxy.nickname = proxy;
		address.proxy.uniqueID = right;
	}

	void ToString(std::ostream& stream) const;

	friend std::ostream& operator<<(std::ostream& stream, const WHartAddress& address);

	bool operator==(const WHartAddress& right) const;

	bool operator<(const WHartAddress& right) const;
};

std::ostream& operator<<(std::ostream& stream, const WHartAddress& address);

enum WHartPriority
{
	whartpEvent = 0,
	whartpNormal = 1,
	whartpData = 2,
	whartpCommand = 3
};

typedef unsigned short WHartServiceID;

struct WHartLocalStatus
{
	enum Status
	{
		whartlsSuccess = 0,
		whartlsWarning = 1,
		whartlsError_Start = 10,
		whartlsError_TL_TableEntryAlreadyActive = 12,
		whartlsError_TL_DestinationHasNoNickname = 16,
		whartlsError_TL_Timeout = 24
	//start the error range
	};

	Status status;
	uint8_t deviceStatus;

	WHartLocalStatus(uint16_t status_ = whartlsSuccess, uint8_t deviceStatus_ = 0) :
		status((Status) status_), deviceStatus(deviceStatus_)
	{
	}
};

enum WHartTransportType
{
	wharttTransferRequest = 0,
	wharttTransferResponse = 1,
	wharttRequestUnicast = 2,
	wharttResponseUnicast = 3,
	wharttSearchBroadcast = 4,
	wharttPublishBroadcast = 5,
	wharttRequestBroadcast = 6,
	wharttResponseBroadcast = 7,
	wharttPublishNotify = 8
};

const char* ToString(WHartTransportType type);

struct WHartPayload
{
	const uint8_t* data;
	uint16_t dataLen;

	WHartPayload()
	{
		data = NULL;
		dataLen = 0;
	}

	WHartPayload(const uint8_t* data_, uint16_t dataLen_) :
		data(data_), dataLen(dataLen_)
	{
	}

	WHartPayload(const std::basic_string<uint8_t>& str_) :
		data(str_.data()), dataLen(str_.size())
	{
	}

	void ToString(std::ostream& stream) const;

	friend std::ostream& operator<<(std::ostream& output, const WHartPayload& payload);
};

std::ostream& operator<<(std::ostream& output, const WHartPayload& payload);

struct WHartSessionKey
{
	enum SessionKeyCode
	{
		sessionKeyed = 0, joinKeyed = 1, handheldKeyed = 2
	};

	bool isBroadcast;
	SessionKeyCode keyCode;

	uint8_t key[16];
};

struct WHartDllAddress
{
	enum AddressType
	{
		whartdaGraph = 0, whartdaNeighbour = 1, whartdaBroadcast = 2, whartdaProxy = 3
	};

	AddressType type;
	union
	{
		uint16_t graphID;
		WHartShortAddress neighbour;
		WHartUniqueID proxy;
	} address;

	WHartDllAddress()
	{
		type = whartdaGraph;
		address.graphID = 0;
	}

	WHartDllAddress(uint16_t graphID, bool)
	{
		type = whartdaGraph;
		address.graphID = graphID;
	}

	WHartDllAddress(WHartShortAddress neighbour)
	{
		type = whartdaNeighbour;
		address.neighbour = neighbour;
	}

	WHartDllAddress(WHartUniqueID proxy)
	{
		type = whartdaProxy;
		address.proxy = proxy;
	}

	void ToString(std::ostream& output) const;
	friend std::ostream& operator<<(std::ostream& stream, const WHartDllAddress& address);
};

std::ostream& operator<<(std::ostream& stream, const WHartDllAddress& address);

typedef uint16_t WHartDllTimeout;

struct WHartCommand
{
	uint16_t commandID;
	uint8_t responseCode;  //valid only for responses
	void* command;
};

struct WHartCommandList
{
	uint8_t count;
	WHartCommand* list;
};

//forward declaration
struct WHartApplication;
struct WHartSubApplication;
struct WHartTransport;
struct WHartNetwork;
struct WHartDataLink;

struct WHartApplication
{
	virtual ~WHartApplication()
	{
	}

	// on source, called by lower level
	virtual void TransmitConfirm(WHartHandle requestHandle, const WHartLocalStatus& localStatus, DeviceIndicatedStatus status,
			const WHartCommandList& list) = 0;

	// on destiantion, called by lower level
	virtual void TransmitIndicate(WHartHandle handle, const WHartAddress& src, WHartPriority priority, DeviceIndicatedStatus status,
			WHartTransportType transportType, const WHartCommandList& list) = 0;
};

/**
 * HART Subapplication Layer
 * Responsabilities:
 * 	- serialize/unserialize requsts/responses
 * 	- process stack commands : For Network, DLL, etc
 */
struct WHartSubApplication
{
	virtual ~WHartSubApplication()
	{
	}

	// on source, called by upper layer
	virtual WHartHandle TransmitRequest(const WHartAddress& dest, WHartPriority priority, WHartServiceID serviceID,
			WHartTransportType transportType, const WHartCommandList& list, WHartSessionKey::SessionKeyCode sessionCode) = 0;

	// on source, called by lower level
	virtual void
			TransmitConfirm(WHartHandle requestHandle, const WHartLocalStatus& localStatus, DeviceIndicatedStatus status, const WHartPayload& apdu) = 0;

	// on destiantion, called by lower level
	virtual void TransmitIndicate(WHartHandle handle, const WHartAddress& src, WHartPriority priority, DeviceIndicatedStatus status,
			WHartTransportType transportType, const WHartPayload& apdu, WHartSessionKey::SessionKeyCode sessionCode) = 0;

	virtual void TransmitResponse(WHartHandle indicatedHandle, WHartServiceID serviceID, const WHartCommandList& list,
			WHartSessionKey::SessionKeyCode sessionCode) = 0;

	WHartApplication* upper; //access to upper layer for notify, app cmds
	WHartTransport* lower; //access to lower to use.
};

/**
 * HART Transport Layer interface.
 * Responsabilities:
 * 	- handles un-acknanowledged: when received Confirm from TL forwards to upper
 *  - acknowledeged service: waits for response, make some retries and notify AL after than with success/failure.
 *  - handles block data transfer (not implemented)
 */
struct WHartTransport
{
	virtual ~WHartTransport()
	{
	}

	//primitives

	// on source, called by upper layer
	virtual void TransmitRequest(WHartHandle handle, const WHartAddress& dest, WHartPriority priority,
			WHartServiceID serviceID, WHartTransportType transportType, const WHartPayload& apdu,
			WHartSessionKey::SessionKeyCode sessionCode) = 0;

	// on source, called by lower level
	virtual void TransmitConfirm(WHartHandle requestHandle, const WHartLocalStatus& localStatus) = 0;

	// on destination, called by the lower layer
	virtual void TransmitIndicate(WHartHandle handle, const WHartAddress& src, WHartPriority priority,
			const WHartPayload& tpdu, WHartSessionKey::SessionKeyCode sessionCode) = 0;
	// on destination, called by the lower level
	virtual void TransmitResponse(WHartHandle indicatedHandle, WHartServiceID serviceID, const WHartPayload& tpdu,
			WHartSessionKey::SessionKeyCode sessionCode) = 0;

	virtual void FlushRequest(WHartHandle requestHandle)
	{
	}
	virtual void FlushConfirm(WHartHandle, WHartLocalStatus)
	{
	}

	WHartSubApplication* upper; //access to upper layer for notify.
	WHartNetwork* lower; //access to lower to use.
};

/**
 * HART Network Layer
 * Responsabilities:
 * 	- routes packetes to next hop after validate TTL (and decrement it) & ASN
 *  - authenticate packets for us, and notify TL with decripted npdu
 *  - notify TL about the successfully/failed send packets
 */
struct WHartNetwork
{
	enum WHartNetworkLocalMgm
	{
		whartnlmReset,
		whartnlWriteSessionKey,
		whartnlDelSession,
		whartnlAddRoute,
		whartnlDelRoute,
		whartnlDefaultRoute,
		whartnlReadPDUTimeout,
		whartnlWritePDUTimeout,
		whartnlReadTTL,
		whartnlWriteTTL
	};

	virtual ~WHartNetwork()
	{
	}

	//primitives

	// on source, called by upper layer
	virtual void TransmitRequest(WHartHandle handle, const WHartAddress& dest, WHartPriority priority,
			WHartServiceID serviceID, const WHartPayload& apdu, WHartSessionKey::SessionKeyCode sessionCode) = 0;
	// on source, called by lower level
	virtual void TransmitConfirm(WHartHandle requestHandle, const WHartLocalStatus& localSTatus) = 0;

	// on destination, called by the lower layer
	virtual void TransmitIndicate(const WHartLocalStatus& localStatus, WHartPriority priority,
			const WHartDllAddress& src, WHartPayload& npdu) = 0;

	// used by transport layer for temporization
	virtual int GetServicePeriod(WHartServiceID serviceID, const WHartAddress& dest) = 0;

	WHartTransport* upper; //access to upper layer for notify.
	WHartDataLink* lower; //access to lower to use.
};

/**
 * HART DataLink
 * Responsabilities:
 * 	- todo
 */
struct WHartDataLink
{
	enum DataLinkLocalManagement
	{
		dlllmRESET,
		dlllmDISCONNECT,
		dlllmRE_JOIN,
		dlllmWRITE_SUPERFRAME,
		dlllmDELETE_SUPERFRAME,
		dlllmADD_LINK,
		dlllmDELETE_LINK,
		dlllmADD_CONNECTION,
		dlllmDELETE_CONNECTION,
		dlllmREAD_NETWORKID,
		dlllmWRITE_NETWORKID,
		dlllmWRITE_NETWORK_KEY,
		dlllmREAD_TIMEOUT_PERIODS,
		dlllmWRITE_TIMEOUT_PERIOD,
		dlllmREAD_CAPACITIES,
		dlllmREAD_PRIORITY_THRESHOLD,
		dlllmWRITE_PRIORITY_THRESHOLD,
		dlllmREAD_JOIN_PRIORITY,
		dlllmWRITE_JOIN_PRIORITY,
		dlllmREAD_PROMISCUOUS_MODE,
		dlllmWRITE_PROMISCUOUS_MODE,
		dlllmREAD_MAX_BACK_OFF_EXPONENT,
		dlllmWRITE_MAX_BACK_OFF_EXPONENT
	};

	virtual ~WHartDataLink()
	{
	}

	virtual void TransmitRequest(WHartHandle handle, const WHartDllAddress& dest, WHartPriority priority,
			WHartDllTimeout timout, const WHartPayload& dpdu) = 0;

	//void FlushRequest(WHartHandle);
	//void FlushConfirm(WHartHandle, WHartLocalStatus);

	//TODO [nicu.dascalu] - add if need it, local management

	WHartNetwork* upper; //access to upper layer for notify.
};

} //namespace stack
} //namespace hart7

#endif /* WHART_STACK_H_ */
