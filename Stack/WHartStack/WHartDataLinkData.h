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
 * DataLinkLayer.h
 *
 *  Created on: Nov 17, 2008
 *      Author: ovidiu.rauca
 */

#ifndef WHART_DATALINKDATA_H_
#define WHART_DATALINKDATA_H_

#include <WHartStack/WHartStack.h>
#include <WHartStack/WHartCommonData.h>
#include <WHartStack/CommInterfaceOverIP/SessionIP.h>

#include <nlib/log.h>
#include <map>
#include <vector>
#include <list>
#include <string>


namespace hart7 {
namespace stack {
namespace datalink {

struct Graph
{
	typedef std::vector<WHartShortAddress> NeighborList;

	uint16_t graphID;
	std::vector<WHartShortAddress> neighbours;
};

struct NeighborLink
{
	WHartShortAddress nickName;
	WHartUniqueID uniqueID;

	uint16_t packetsTransmitted;
	uint16_t missedAckPackets;
	uint16_t packetsReceived;
	uint16_t broadcastReceived;

	transport::SessionIP::Ptr connection;

	bool IsActive() const
	{
		return connection;
	}
};

struct Packet
{
	typedef std::basic_string<uint8_t> Payload;

	uint16_t packetID;
	WHartDllAddress dest;
	WHartTime timestamp;
	WHartPriority priority;
	Payload npdu;
};

struct QNeighbourList
{
public:
	bool HasMoreNeighbors()
	{
		return neighbors.size() > 0;
	}

	WHartAddress PopNextNeighbor()
	{
		if (HasMoreNeighbors())
		{
			WHartAddress address = *neighbors.begin();
			neighbors.erase(neighbors.begin());

			return address;
		}

		return WHartAddress();
	}

	void PushNeighbor(WHartAddress neighbor)
	{
		neighbors.push_back(neighbor);
	}

private:
	std::vector<WHartAddress> neighbors;
};

struct QRequest
{
	// holds the time for which a message should be waited for, until a retry occurs
	WHartDllTimeout timeout;
	// holds the remaining time until the message should timeout
	WHartDllTimeout timeRemaining;

	// the handle of the message, used to signal upper layer when a response is received
	WHartHandle handle;
	// the priority of the message
	WHartPriority priority;

	// the transaction ID with which the response is mapped to the request
	int transactionID;
	// the NPDU of the message
	std::basic_string<uint8_t> payload;

	// the remaining neighbors list. A message should be sent to each neighbor, in turn, until one of them
	// successfully receives the message or no send neighbors are left
	QNeighbourList neighbourSendList;
};

/**
 *
 */
class WHartDataLinkData: public WHartDataLink
{
	LOG_DEF("h7.s.d.WHartDataLinkData");
public:
	typedef std::map<uint16_t, Graph> GraphsMap;
	typedef std::list<NeighborLink> NeighborLinkList;
	typedef std::vector<QRequest> QRequestList;

public:
	WHartDataLinkData(const CommonData& commonData);

	//WHartDataLink
private:
	void TransmitRequest(WHartHandle handle, const WHartDllAddress& dest, WHartPriority priority, WHartDllTimeout timout,
			const WHartPayload& npdu);

public:
	void Reset();

	// neighbor session management
public:
	/**
	 * Create or update peerNickName for a neighbor session
	 */
	void CreateNeighborSession(const WHartUniqueID& peerUniqueID, const WHartShortAddress peerNickname = No_Nickname());
	/*
	 * Set session
	 * Reset session if connection = NULL
	 */

	bool HasNeighborSession(transport::SessionIP::Ptr connection);

	void SetNeighborSession(const WHartUniqueID& peerUniqueID, transport::SessionIP::Ptr connection);

	void UpdateNeighborSessionNickname(transport::SessionIP::Ptr connection, WHartShortAddress nickname);

	void ResetNeighborSession(transport::SessionIP::Ptr connection);

	// graph management
public:
	void AddConnection(uint16_t graphID, uint16_t neighborNickname);
	void DeleteConnection(uint16_t graphID, uint16_t neighborNickname);

private:
	Graph& GetOrCreateGraph(uint16_t graphId);

	NeighborLinkList::iterator GetNeighborForSession(transport::SessionIP::Ptr connection);

	void EraseNoNeighborPackets();

	//send/receive packet
private:
	void SetQRequestDestinationPath(const WHartDllAddress& dest, QRequest& qrequest);
	void SendRequest(QRequest& qrequest);
	QRequestList::iterator GetPendingRequest(int transactionID);

	std::pair<bool, uint16_t> SendPacket(WHartAddress destination, transport::MessageIP& message);
	boost::uint16_t GetNextTransactionID();

	//called from outside
	void PacketReceived(WHartUniqueID from, const transport::MessageIP& packet);

public:	// Called from time to time, for timeout requests...
	void TimeElapsed(int msecs);

private:
	const CommonData& commonData;

	GraphsMap graphs;
	NeighborLinkList neighborLinks;
	QRequestList pendingRequests;

	boost::uint16_t nextTransactionID;
};

}// namespace datalink
}// namespace stack
}// namespace hart7

#endif /* WHART_DATALINKDATA_H_ */
