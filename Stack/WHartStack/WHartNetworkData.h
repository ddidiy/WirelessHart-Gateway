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
 * WHartNetwork.h
 *
 *  Created on: Dec 11, 2008
 *      Author: nicu.dascalu
 */

#ifndef WHART_NETWORK_DATA_H
#define WHART_NETWORK_DATA_H

#include <WHartStack/WHartStack.h>
#include <WHartStack/WHartCommonData.h>
#include <ApplicationLayer/Model/WirelessNetworkManagerCommands.h>
#include <ApplicationLayer/Model/NetworkLayerCommands.h>
#include <WHartStack/util/Binarization.h>
#include <boost/function.hpp>

#include <nlib/log.h>
#include <vector>
#include <set>

namespace hart7 {
namespace stack {
namespace network {

struct Session
{
	uint8_t sessionID;
	WHartUniqueID peerUniqueID;
	WHartShortAddress peerNickName;

	WHartSessionKey sessionKey;
	uint32_t receiveNonceCounter;
	uint32_t nonceCounterHistory;
	uint32_t transmitNonceCounter;
};

struct Service
{
	typedef int ServiceFlags; //for compile only
	typedef int ServiceDomain;

	uint8_t serviceID;
	WHartShortAddress peerNickname;
	ServiceFlags serviceFlags;
	ServiceDomain serviceDomain;
	WHartTime servicePeriod;
	uint8_t routeID;
};


struct Route
{
	enum DestinationTypes
	{
		dtRegular = 0,
		dtSourceRoute = 1
	};

	enum RouteTypes
	{
		rtMaintenance = 0x1,
		rtPublish = 0x2,
		rtBlockTransfer = 0x4,
		rtEvent = 0x8
	};

	uint8_t routeID;
	RouteTypes routeType;

	DestinationTypes destinationType;
	std::vector<WHartShortAddress> sourceRoutePath;

	uint16_t graphID;
	WHartShortAddress peerNickname;

	uint16_t numPktsTransmitted;
	WHartTime lastTransmitTime;
	uint16_t numPktsReceived;
};


class WHartNetworkData: public WHartNetwork
{
	LOG_DEF("h7.s.n.WHartNetworkData");

public:
	typedef std::vector<Session> SessionTable;
	typedef std::vector<Route> RouteTable;
	typedef std::vector<Service> ServiceTable;
	typedef std::set<WHartHandle> RoutedHandleTable;

public:
	WHartNetworkData(CommonData& common);
	~WHartNetworkData();

	//WHartNetwork
public:
	void TransmitRequest(WHartHandle handle, const WHartAddress& dest, WHartPriority priority, WHartServiceID serviceID,
			const WHartPayload& tpdu, WHartSessionKey::SessionKeyCode sessionCode);
	void TransmitRequest_Broadcast(WHartHandle handle, WHartPriority priority, const WHartPayload& tpdu);
	void TransmitRequest_Unicast(WHartHandle handle, const WHartAddress& dest, WHartPriority priority,
			WHartServiceID serviceID, const WHartPayload& tpdu, WHartSessionKey::SessionKeyCode sessionCode);
	void TransmitRequest_Join(const WHartAddress& through);

	void TransmitConfirm(WHartHandle requestHandle, const WHartLocalStatus& localStatus);
	void TransmitIndicate(const WHartLocalStatus& localStatus, WHartPriority priority, const WHartDllAddress& src,
			WHartPayload& npdu);

	// used by transport layer for temporization
	virtual int GetServicePeriod(WHartServiceID serviceID, const WHartAddress& dest);

	void Reset();
private:
	void WriteNetworkHeader(Route& route, const WHartAddress& dest, const WHartAddress& src, uint8_t control, uint8_t ttl, uint16_t asn, BinaryStream& stream);

	WHartDllAddress GetDllDestinationAddress(Route& route);
public:

	// local management
public:
	void WriteService(C973_WriteService_Req* req, C973_WriteService_Resp* res);
    void DeleteService(C801_DeleteService_Req* req, C801_DeleteService_Resp* resp);

	void WriteSession(const C963_WriteSession_Req* req, C963_WriteSession_Resp* res);
	void DeleteSession(C964_DeleteSession_Req* req, C964_DeleteSession_Resp* res);
	void DeleteSession(const WHartAddress& destination, WHartSessionKey::SessionKeyCode sessionType);

	void WriteRoute(C974_WriteRoute_Req* req, C974_WriteRoute_Resp* res);
	void DeleteRoute(C975_DeleteRoute_Req* req, C975_DeleteRoute_Resp* res);
	int ReadRouteList(C802_ReadRouteList_Req* req, C802_ReadRouteList_Resp* resp);

	void WriteSourceRoute(C976_WriteSourceRoute_Req* req, C976_WriteSourceRoute_Resp* resp);
	void DeleteSourceRoute(C977_DeleteSourceRoute_Req* req, C977_DeleteSourceRoute_Resp* resp);
	int ReadSourceRoute(C803_ReadSourceRoute_Req* req, C803_ReadSourceRoute_Resp* resp);

	void SetSession(C963_WriteSession_Req* req);

private:
	ServiceTable::iterator FindService(WHartServiceID serviceID, const WHartAddress& dest);

    SessionTable::iterator FindSession(const WHartAddress& peer, WHartSessionKey::SessionKeyCode keyCode);
	RouteTable::iterator FindRoute(uint8_t routeID);
	RouteTable::iterator GetRoute(WHartServiceID serviceID, const WHartAddress& dest, WHartLocalStatus& error);

	WHartHandle NewRoutedHandle();
	bool IsRoutedHandle(WHartHandle handle);
	void DeleteRoutedHandle(WHartHandle handle);

public:
	CommonData& commonData;

	SessionTable sessionTable;
	RouteTable routeTable;
	ServiceTable serviceTable;

	RoutedHandleTable routedHandlersTable;
	WHartHandle nextRoutedHandler;

	uint8_t defaultRouteID;

	uint16_t PDUTimeout;
	uint8_t TTL;

	//called when join flow is detected
    boost::function1<void, WHartAddress&> OnJoinCallback;
};

}// namespace network
}// namespace stack
}// namespace hart7


#endif /* WHART_NETWORK_H */
