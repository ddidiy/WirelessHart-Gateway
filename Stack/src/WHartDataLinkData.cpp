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
 * WHartDataLink.cpp
 *
 *  Created on: Nov 27, 2008
 *      Author: nicu.dascalu
 */
#include <WHartStack/WHartDataLinkData.h>

#include <boost/bind.hpp>
#include <algorithm>

namespace hart7 {
namespace stack {
namespace datalink {

using namespace hart7::stack::transport;

WHartDataLinkData::WHartDataLinkData(const CommonData& commonData_) :
	commonData(commonData_)
{
	Reset();
}

void WHartDataLinkData::TransmitRequest(WHartHandle handle, const WHartDllAddress& dest, WHartPriority priority,
		WHartDllTimeout timeout, const WHartPayload& npdu)
{
	pendingRequests.push_back(QRequest());
	QRequest& qrequest = *pendingRequests.rbegin();

	SetQRequestDestinationPath(dest, qrequest);
	qrequest.handle = handle;
	qrequest.timeout = timeout;
	qrequest.priority = priority;
	qrequest.transactionID = 0xFFFF;
	qrequest.payload.clear();
	qrequest.payload.insert(qrequest.payload.end(), npdu.data, npdu.data + npdu.dataLen);

	SendRequest(qrequest);
}

void WHartDataLinkData::CreateNeighborSession(const WHartUniqueID& peerUniqueID, const WHartShortAddress peerNickname)
{
	NeighborLinkList::iterator link = neighborLinks.begin();
	for (; link != neighborLinks.end(); ++link)
	{
		if (link->uniqueID == peerUniqueID)
			break;
	}

	if (link == neighborLinks.end())
	{
		//not found, so create it
		neighborLinks.push_back(NeighborLink());
		link = neighborLinks.end();
		--link;
		link->uniqueID = peerUniqueID;
	}

	link->nickName = peerNickname;
}

void WHartDataLinkData::UpdateNeighborSessionNickname(transport::SessionIP::Ptr connection, WHartShortAddress nickname)
{
	NeighborLinkList::iterator link = GetNeighborForSession(connection);

	if (link != neighborLinks.end())
	{
		if (link->nickName == No_Nickname())
		{
			LOG_INFO("Neighbor session with address=" << WHartAddress(link->uniqueID) << " received nickname="
					<< WHartAddress(nickname));
			link->nickName = nickname;
			//[andy] - set the packetReceived back here (in case it is hijacked in GW to find the session identity.
			link->connection->ReceiveMessage = boost::bind(&WHartDataLinkData::PacketReceived, this, link->uniqueID, _1);
		}
	}
}

void WHartDataLinkData::SetNeighborSession(const WHartUniqueID& peerUniqueID, transport::SessionIP::Ptr connection)
{
	assert(connection && "Set neighbor session connection cannot be null!");

	for (NeighborLinkList::iterator link = neighborLinks.begin(); link != neighborLinks.end(); ++link)
	{
		if (link->uniqueID == peerUniqueID)
		{
			//register
			link->connection = connection;
			link->connection->ReceiveMessage = boost::bind(&WHartDataLinkData::PacketReceived, this, link->uniqueID, _1);
			return;
		}
	}
	LOG_WARN("SetNeighborSession: UniqueID:" << WHartAddress(peerUniqueID) << "not found!");
}

void WHartDataLinkData::ResetNeighborSession(transport::SessionIP::Ptr connection)
{
	NeighborLinkList::iterator link = GetNeighborForSession(connection);

	if (link != neighborLinks.end())
	{
		LOG_WARN("ServerSessionClosed: done!");
		if (link->IsActive())
		{
			if (link->connection)
			{
				link->connection->ReceiveMessage.clear();
			}
			link->connection.reset();
		}
	}
}

bool WHartDataLinkData::HasNeighborSession(transport::SessionIP::Ptr connection)
{
	return GetNeighborForSession(connection) != neighborLinks.end();
}

WHartDataLinkData::NeighborLinkList::iterator WHartDataLinkData::GetNeighborForSession(
		transport::SessionIP::Ptr connection)
{
	for (NeighborLinkList::iterator link = neighborLinks.begin(); link != neighborLinks.end(); ++link)
	{
		if (link->connection == connection)
		{
			return link;
		}
	}

	return neighborLinks.end();
}

void WHartDataLinkData::AddConnection(uint16_t graphID, uint16_t neighborNickname)
{
	Graph& graph = GetOrCreateGraph(graphID);

	if (graph.neighbours.end() == std::find(graph.neighbours.begin(), graph.neighbours.end(), neighborNickname))
	{
		graph.neighbours.push_back(neighborNickname);
	}
}

void WHartDataLinkData::DeleteConnection(uint16_t graphID, uint16_t neighborNickname)
{
	Graph& graph = GetOrCreateGraph(graphID);

	Graph::NeighborList::iterator found = std::find(graph.neighbours.begin(), graph.neighbours.end(), neighborNickname);
	if (found != graph.neighbours.end())
	{
		graph.neighbours.erase(found);
	}
}

Graph& WHartDataLinkData::GetOrCreateGraph(uint16_t graphId)
{
	GraphsMap::iterator graph = graphs.find(graphId);
	if (graph == graphs.end())
	{
		graphs.insert(std::make_pair(graphId, Graph()));
		graph = graphs.find(graphId);
	}

	return graph->second;
}

void WHartDataLinkData::SetQRequestDestinationPath(const WHartDllAddress& dest, QRequest& qrequest)
{
	if (dest.type == WHartDllAddress::whartdaGraph)
	{
		GraphsMap::iterator graph = graphs.find(dest.address.graphID);
		if (graph != graphs.end())
		{
			LOG_DEBUG("Sending on gID=" << std::hex << dest.address.graphID);
			for (Graph::NeighborList::iterator it = graph->second.neighbours.begin(); it != graph->second.neighbours.end(); ++it)
			{
//				LOG_DEBUG("Pushing neighbor=" << std::hex << *it);
				qrequest.neighbourSendList.PushNeighbor(WHartAddress(*it));
			}
		}
		else
		{
			LOG_WARN("Cannot find graphID=" << std::hex << dest.address.graphID);
		}
	}
	else if (dest.type == WHartDllAddress::whartdaNeighbour)
	{
		qrequest.neighbourSendList.PushNeighbor(WHartAddress(dest.address.neighbour));
	}
	else if (dest.type == WHartDllAddress::whartdaBroadcast)
	{
		//TODO
	}
	else if (dest.type == WHartDllAddress::whartdaProxy)
	{
		qrequest.neighbourSendList.PushNeighbor(WHartAddress(dest.address.proxy));
	}
}

void WHartDataLinkData::SendRequest(QRequest& qrequest)
{
	if (qrequest.neighbourSendList.HasMoreNeighbors())
	{
		WirelessNPDU_Request request;
		if (commonData.isJoined)
		{
			request.srcAddress = WHartAddress(commonData.myNickname);
		}
		else
		{
			request.srcAddress = WHartAddress(commonData.myUniqueID);
		}
		if (qrequest.transactionID == 0xFFFF)
			qrequest.transactionID = GetNextTransactionID();

		request.npdu = qrequest.payload;
		request.priority = qrequest.priority;
		request.transactionID = qrequest.transactionID;
		WHartAddress destination = qrequest.neighbourSendList.PopNextNeighbor();

		LOG_INFO("SendPacket: h=" << qrequest.handle << " tid=" << qrequest.transactionID << " to=" << destination);

		std::pair<bool, uint16_t> resp = SendPacket(destination, request);
		if (resp.first)
		{
			// reset timeout
			qrequest.timeRemaining = qrequest.timeout;
			qrequest.transactionID = request.transactionID;
		}
	}
	else
	{
		LOG_ERROR("no neighbors left that have active connections, so fail message");
		// no neighbors left that have active connections, so fail message
		upper->TransmitConfirm(qrequest.handle, WHartLocalStatus(WHartLocalStatus::whartlsError_Start));
	}

}

WHartDataLinkData::QRequestList::iterator WHartDataLinkData::GetPendingRequest(int transactionID)
{
	for (QRequestList::iterator it = pendingRequests.begin(); it != pendingRequests.end(); ++it)
	{
		if (it->transactionID == transactionID)
		{
			return it;
		}
	}
	return pendingRequests.end();
}

std::pair<bool, uint16_t> WHartDataLinkData::SendPacket(WHartAddress destination, transport::MessageIP& packet)
{
	NeighborLinkList::iterator link = neighborLinks.begin();
	for (; link != neighborLinks.end(); ++link)
	{
//		LOG_DEBUG("Iterating through sessions link->uniqueID=" << WHartAddress(link->uniqueID)
//				<< ", link->nickName=" << WHartAddress(link->nickName) <<", isactive="
//				<< (int)link->IsActive() << ", destination=" << destination);

		if (link->IsActive() //
				&& (destination == link->uniqueID || destination == link->nickName))
		{
			break;
		}
	}

	if (link != neighborLinks.end())
	{
//		if (packet.transactionID == 0xFFFF)
//			packet.transactionID = GetNextTransactionID();

		if (link->connection)
		{
			uint16_t transactionID = link->connection->SendMessage(packet);
			link->packetsTransmitted++;

			LOG_DEBUG("SendPacket: to=" << destination << " message=" << packet);
			return std::make_pair(true, transactionID);
		}
	}

	LOG_WARN("SendPacket: no connection found for dest=" << destination);
	return std::make_pair(false, 0); //no neighbor found
}

boost::uint16_t WHartDataLinkData::GetNextTransactionID()
{
	nextTransactionID = (boost::uint32_t) ((int) (nextTransactionID + 1) % 0xFFFF);

	//first 10 are reserver for other purpose: initiate, keep-alive, close sseion
	if (nextTransactionID < 10)
		nextTransactionID = 10;

	return nextTransactionID;
}

void WHartDataLinkData::PacketReceived(WHartUniqueID from, const MessageIP& packet)
{
	class DLLProcessor: public transport::IMessageIPVisitor
	{
		WHartDataLinkData& data;

	public:
		DLLProcessor(WHartDataLinkData& data_) :
			data(data_)
		{
		}

		void Do(const transport::MessageIP& packet)
		{
			const_cast<MessageIP&> (packet).Accept(*this);
		}

	private:
		void Visit(WirelessNPDU_Request& req)
		{
			//send ACK
			WirelessNPDU_ACK response;
			response.status = WHartLocalStatus::whartlsSuccess;
			response.transactionID = req.transactionID;

			data.SendPacket(req.srcAddress, response);

			WHartPayload payload;
			payload.dataLen = req.npdu.size();
			payload.data = req.npdu.c_str();

			WHartLocalStatus status(WHartLocalStatus::whartlsSuccess);
			switch (req.status)
			{
			case 0:
				break; // success
			default:
				status.status = WHartLocalStatus::whartlsError_Start;
			}
			WHartDllAddress address;
			switch (req.srcAddress.type)
			{
			case WHartAddress::whartaNickname:
				address.type = WHartDllAddress::whartdaNeighbour;
				address.address.neighbour = req.srcAddress.address.nickname;
				break;
			case WHartAddress::whartaUniqueID:
				address.type = WHartDllAddress::whartdaProxy;
				address.address.proxy = req.srcAddress.address.uniqueID;
				break;
			default:
				break;
			}

			data.upper->TransmitIndicate(status, req.priority, address, payload);
		}

		void Visit(WirelessNPDU_ACK& resp)
		{
			QRequestList::iterator pendingQRequest = data.GetPendingRequest(resp.transactionID);

			if (pendingQRequest != data.pendingRequests.end())
			{
				WHartLocalStatus status(WHartLocalStatus::whartlsSuccess);
				switch (resp.status)
				{
				case 0:
					break; // success
					//TODO check how errors are allocated, and check where to define them
					//			case 61: status = whartlsError_Start + 1; break;	// no buffers
					//			case 62: status = whartlsError_Start + 2; break;	// no alarm / event buffers
					//			case 63: status = whartlsError_Start + 3; break;	// priority too low
				default:
					break;
				}

				WHartHandle handle = pendingQRequest->handle;
				data.pendingRequests.erase(pendingQRequest);

				data.upper->TransmitConfirm(handle, status);
			}
			else
			{
				//TODO log message not found
				LOG_DEBUG("ACK for unknown Transaction ID!");
			}
		}
	};

	if (LOG_DEBUG_ENABLED())
	{
		LOG_DEBUG("PacketReceived: from=" << WHartAddress(from) << " message=" << packet);
	}
	else
	{
		LOG_INFO("PacketReceived: from=" << WHartAddress(from));
	}

	DLLProcessor(*this).Do(packet);
}

struct NoNeighborMessageEraser
{
	bool operator()(QRequest& qrequest)
	{
		return (!qrequest.neighbourSendList.HasMoreNeighbors());
	}
};

void WHartDataLinkData::TimeElapsed(int msecs)
{
	for (QRequestList::iterator it = pendingRequests.begin(); it != pendingRequests.end(); ++it)
	{
		it->timeRemaining -= msecs;
		if (it->timeRemaining <= 0)
		{
			SendRequest(*it);
		}
	}

	EraseNoNeighborPackets();
}

void WHartDataLinkData::EraseNoNeighborPackets()
{
	//erase all messages that have no more neighbors...
	pendingRequests.erase(std::remove_if(pendingRequests.begin(), pendingRequests.end(), NoNeighborMessageEraser()),
			pendingRequests.end());
}

void WHartDataLinkData::Reset()
{
	LOG_DEBUG("Resetting DLL.");
	graphs.clear();
	neighborLinks.clear();
	pendingRequests.clear();

	nextTransactionID = 10;
}

}// namespace datalink
}// namespace stack
}// namespace hart7

