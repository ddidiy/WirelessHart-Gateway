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
 * UDPServerIP.cpp
 *
 *  Created on: Nov 28, 2008
 *      Author: ovidiu.rauca
 */

#include <WHartStack/CommInterfaceOverIP/UDPServerIP.h>

#include <nlib/detail/bytes_print.h>
#include <nlib/sformat.h>

#include <boost/bind.hpp>
#include <boost/assert.hpp>
#include <stdio.h>

namespace hart7 {
namespace stack {
namespace transport {

UDPServerSessionIP::UDPServerSessionIP(UDPServerIP& server_, nlib::socket::UdpSocket::Ptr socket_,
  const nlib::socket::Address& peer_) :
	server(server_), socket(socket_), peer(peer_)
{
	BOOST_ASSERT(socket != NULL);
	nextTransactionID = 0;
	socket->ReceivedFrom = boost::bind(&UDPServerSessionIP::ReceiveFromHandler, this, _1, _2, _3);
}

UDPServerSessionIP::~UDPServerSessionIP()
{
	if (socket)
	{
		socket->ReceivedFrom = 0;
		socket->Close();
		socket.reset();
	}
}

void UDPServerSessionIP::AcceptStream(std::ostream& stream)
{
    stream << *this;
}

 std::ostream& operator<<(std::ostream& output, UDPServerSessionIP& serverSession)
{
	 if (serverSession.socket)
	 {
		 output << "[Session Active from:" << serverSession.socket->ToString() << " connected to:" << serverSession.peer.ToString() << "]";
	 }
	 else
	 {
		 output << "[Closed session]";
	 }
    return output;
}

boost::uint16_t UDPServerSessionIP::SendMessage(MessageIP& message)
{
	if (message.transactionID == 0xFFFF)
	{
		//generate new transactionID
		nextTransactionID = (nextTransactionID + 1) % 0xFFFF;
		message.transactionID = nextTransactionID;
	}

//	LOG_INFO("SendMessage:"
//			<< " message=" << message
//			<< " on session=" << *this);

	MessageIPSerializer::BufferOut buffer;
	buffer.reserve(128);
	serializer.Serialize(message, buffer);

	if (socket)
	{
		socket->SendTo(peer, buffer.data(), buffer.size());
	}

	return message.transactionID;
}

void UDPServerSessionIP::ReceiveFromHandler(const nlib::socket::Address& from, const boost::uint8_t* bytes,
  std::size_t bytesSize)
{

	if (from != peer)
	{
		//ignore packet not expected from that destination
		LOG_WARN("Ignore the message received from:" << from.ToString());
		return;
	}

	try
	{
		MessageIP::Ptr message = unserializer.ParseBytesComplete(bytes, bytesSize);
		if (message && message->parsedStatus)
		{
//			LOG_INFO("ReceiveMessage:"
//					<< " on session=" << *this
//					<< " message=" << *message
//					<< " from=" << from.ToString());

			//forward first to server
			if (!server.ProcessMessage(*this, *message))
			{
				if (ReceiveMessage)
					ReceiveMessage(*message);
			}
		}
		else
		{
		    if (message)
		    {
		        LOG_ERROR("ReceiveMessage: bad message"
								<< " on session=" << *this
								<< " message=" << *message
								<< " from=" << from.ToString());
		    }
		    else
		    {
                LOG_ERROR("ReceiveMessage: bad message"
                                << " on session=" << *this
                                << " message=[NULL] from=" << from.ToString());
		    }
		}
	}
	catch (std::exception& ex)
	{
		LOG_ERROR("Parse bytes failed. error=" << ex.what()
				<< " bytes=<" << nlib::detail::BytesToString((uint8_t*) bytes, bytesSize) << ">"
				<< " from=" << from.ToString());
	}
	catch (...)
	{
		LOG_ERROR("Parse bytes failed. unknown error!"
				<< " bytes=<" << nlib::detail::BytesToString((uint8_t*) bytes, bytesSize) << ">"
				<< " from=" << from.ToString());
	}

}

/**************************************************************/

const int UDPServerIP::TIMER_INTERVAL = 2 * 1000; //seconds
const boost::uint32_t UDPServerIP::MAX_INACTIVE_CLOSE_TIME = 30; //seconds

std::ostream& operator<<(std::ostream& output, UDPServerIP::SessionMapping& sessionMapping)
{
    output << "[Session active=" << sessionMapping.IsActive() << " port=" << sessionMapping.port << " peer=" << sessionMapping.peer.ToString() << " lastTrafic=" << sessionMapping.lastTraffic << "]";

    return output;
}

UDPServerIP::UDPServerIP(const nlib::socket::ServiceSocket::Ptr& service_, nlib::timer::TimerFactory::Ptr timerFactory_,
		int listenPort, int minPort, int maxPort) :
	service(service_), timerFactory(timerFactory_)
{
	BOOST_ASSERT(service != NULL);

	listenSocket = service->CreateUdpSocket();
	BOOST_ASSERT(listenSocket != NULL);

	listenSocket->Port(listenPort);

	for (int i = minPort; i < maxPort; i++)
	{
		sessions.insert(SessionsMap::value_type(i, SessionMapping(i)));
	}

	currentTime = 0;
	inactiveCloseTime = MAX_INACTIVE_CLOSE_TIME * 1000;

	LOG_INFO("Server created on:" << listenSocket->ToString());
}

void UDPServerIP::Listen()
{
	LOG_INFO("Start listening on:" << listenSocket->ToString());
	listenSocket->ReceivedFrom = boost::bind(&UDPServerIP::HandleNewSession, this, _1, _2, _3);
	listenSocket->Listen();

	checkInactiveSessions = timerFactory->CreateDeadlineTimer(TIMER_INTERVAL);
	checkInactiveSessions->Elapsed = boost::bind(&UDPServerIP::CloseInactiveSessions, this, _1);
}

void UDPServerIP::Close()
{
	LOG_INFO("Closing UDPServer, sending close to all active clients ..");
	listenSocket->Close();
	checkInactiveSessions->Elapsed = 0;
	checkInactiveSessions.reset();

	//send close to all active clients
	SessionClose_Request close;
	close.status = 0;
	for (SessionsMap::iterator it = sessions.begin(); it != sessions.end(); it++)
	{
		if (it->second.IsActive())
		{
			it->second.session->SendMessage(close);
			it->second.session.reset();
		}
	}
}

void UDPServerIP::HandleNewSession(const nlib::socket::Address& from, const boost::uint8_t* bytes, std::size_t size)
{
	struct Processor: public IMessageIPVisitor
	{
		UDPServerIP& server;
		const nlib::socket::Address& from;

		Processor(UDPServerIP& server_, const nlib::socket::Address& from_) :
			server(server_), from(from_)
		{
		}

		void Do(const MessageIP& message)
		{
			const_cast<MessageIP&> (message).Accept(*this);
		}

		//IMessageIPVisitor
	private:
		void Visit(SessionInitiate_Request& req)
		{
			// LOG_INFO("HandleNewSession: SessionInitiate message=" << req << " handled.");
			SessionsMap::iterator activeSession = server.GetNextAvailable(from);
			if (activeSession == server.sessions.end())
			{
				LOG_WARN("HandleNewSession: No more sessions available!");
				SessionInitiate_Response initiateFailed;
				initiateFailed.status = SessionInitiate_Response::rcErr_AllAvailableSessionsInUse;
				initiateFailed.transactionID = req.transactionID;

				//SendResponse
				{
					MessageIPSerializer::BufferOut buffer;
					buffer.reserve(128);
					MessageIPSerializer serializer;
					serializer.Serialize(initiateFailed, buffer);
					server.listenSocket->SendTo(from, buffer.data(), buffer.size());
				}
				return;
			}
			if (activeSession->second.IsActive())
			{
				LOG_WARN("HandleNewSession:: Session already intializated, so closed automatically!");

				server.CloseSession(activeSession, 0);
				return;
			}

			nlib::socket::UdpSocket::Ptr udpSocket(server.service->CreateUdpSocket());
			try
			{
				udpSocket->Port(activeSession->first);
				udpSocket->Listen();
			}
			catch (std::exception& ex)
			{
				LOG_WARN("Exception occured while opening socket: ex=" << ex.what());
				return;
			}

			activeSession->second.peer = from;
			activeSession->second.session.reset(new UDPServerSessionIP(server, udpSocket, from));

			server.HasTraffic(activeSession);

			if (server.inactiveCloseTime < req.inactivityCloseTime)
			{
                LOG_INFO("HandleNewSession: SessionInitiate update - old server.inactiveCloseTime=" << server.inactiveCloseTime << ", new server.inactiveCloseTime=" << req.inactivityCloseTime);
				server.inactiveCloseTime = req.inactivityCloseTime;
			}

			{
				SessionInitiate_Response response;
				response.transactionID = req.transactionID;
				response.status = SessionInitiate_Response::rcSuccess;
				response.masterType = req.masterType;
				response.inactivityCloseTime = server.inactiveCloseTime;
				response.bypassIOCache = req.bypassIOCache;
				activeSession->second.session->SendMessage(response);
			}

			server.FireNewSession(from.Host(), activeSession->second.session);
			LOG_INFO("HandleNewSession: create session=" << *activeSession->second.session);
		}

		void Visit(SessionInitiate_Response& res)
		{
			LOG_WARN("HandleNewSession: Unexpected message=" << res);
		}

		void Visit(SessionClose_Request& req)
		{
			LOG_WARN("HandleNewSession: Unexpected message=" << req);
		}

		void Visit(SessionClose_Response& res)
		{
			LOG_WARN("HandleNewSession: Unexpected message=" << res);
		}

		void Visit(KeepAlive_Request& req)
		{
			LOG_WARN("HandleNewSession: Unexpected message=" << req);
		}

		void Visit(KeepAlive_Response& res)
		{
			LOG_WARN("HandleNewSession: Unexpected message=" << res);
		}

		void Visit(HARTWiredPDU_Request& req)
		{
			LOG_WARN("HandleNewSession: Unexpected message=" << req);
		}

		void Visit(HARTWiredPDU_Response& res)
		{
			LOG_WARN("HandleNewSession: Unexpected message=" << res);
		}

		void Visit(HARTWiredPDU_Notify& res)
		{
			LOG_WARN("HandleNewSession: Unexpected message=" << res);
		}

		void Visit(HARTDirectPDU_Request& req)
		{
			LOG_WARN("HandleNewSession: Unexpected message=" << req);
		}

		void Visit(HARTDirectPDU_Response& res)
		{
			LOG_WARN("HandleNewSession: Unexpected message=" << res);
		}

		void Visit(HARTDirectPDU_Notify& res)
		{
			LOG_WARN("HandleNewSession: Unexpected message=" << res);
		}

		void Visit(WirelessNPDU_Request& req)
		{
			LOG_WARN("HandleNewSession: Unexpected message=" << req);
		}

		void Visit(WirelessNPDU_ACK& res)
		{
			LOG_WARN("HandleNewSession: Unexpected message=" << res);
		}
	};

//	LOG_DEBUG("ReceiveFrom: from=" << from.ToString() << " bytes_count=" << size << " bytes="
//			<< nlib::detail::BytesToString(bytes, size));

	MessageIP::Ptr message = unserializer.ParseBytesComplete(bytes, size);
	if (message && message->parsedStatus)
	{
		Processor p(*this, from);
		p.Do(*message);
	}
	else
	{
	    if (message)
	    {
	        LOG_ERROR("ReceiveMessage: bad message=" << *message << " from=" << from.ToString());
	    }
	    else
	    {
            LOG_ERROR("ReceiveMessage: bad message=[NULL] from=" << from.ToString());
	    }
	}

}

void UDPServerIP::CloseInactiveSessions(int interval)
{
	currentTime += interval;

	for (SessionsMap::iterator it = sessions.begin(); it != sessions.end(); it++)
	{
		if (it->second.IsActive() //
		    && (currentTime - it->second.lastTraffic) > inactiveCloseTime)
		{
			LOG_WARN("Close inactive session=" << it->second << " ! currentTime=" << currentTime << ", lastTraffic=" << it->second.lastTraffic << ", inactiveCloseTime=" << inactiveCloseTime);
			CloseSession(it, 0);
		}
	}

	if (LOG_DEBUG_ENABLED())
	{
		//print session status
		if (currentTime % (60 * 1000) == 0)
		{
			LOG_DEBUG("Print sessions-begin");
			for (SessionsMap::iterator it = sessions.begin(); it != sessions.end(); it++)
			{
				LOG_DEBUG("   -Session=" << it->second);
			}
			LOG_DEBUG("Print sessions-end");
		}
	}
}

void UDPServerIP::CloseAllSessions()
{
	for (SessionsMap::iterator it = sessions.begin(); it != sessions.end(); it++)
	{
		if (it->second.IsActive())
		{
			LOG_WARN("Forcefully close session=" << it->second << " !");
			CloseSession(it, 0);
		}
	}
}

UDPServerIP::SessionsMap::iterator UDPServerIP::GetNextAvailable(const nlib::socket::Address& address)
{
	//select first active with same address
	for (SessionsMap::iterator it = sessions.begin(); it != sessions.end(); it++)
	{
		if (it->second.IsActive() && address == it->second.peer)
			return it;
	}

	//select first free
	for (SessionsMap::iterator it = sessions.begin(); it != sessions.end(); it++)
	{
		if (!it->second.IsActive())
			return it;
	}

	//no one available
	return sessions.end();
}

UDPServerIP::SessionsMap::iterator UDPServerIP::FindSesion(UDPServerSessionIP* session)
{
	for (SessionsMap::iterator it = sessions.begin(); it != sessions.end(); it++)
	{
		if (it->second.IsActive() && session == it->second.session.get())
			return it;
	}
	return sessions.end();
}

void UDPServerIP::HasTraffic(SessionsMap::iterator session)
{
	session->second.lastTraffic = currentTime;
}

void UDPServerIP::FireNewSession(const std::string& host, const SessionIP::Ptr& session)
{
	try
	{
		if (NewSession)
			NewSession(host, session);
	}
	catch (std::exception& ex)
	{
		LOG_ERROR("FireNewSession failed. error=" << ex.what());
	}
	catch (...)
	{
		LOG_ERROR("FireNewSession failed. unknown error!");
	}
}

bool UDPServerIP::FireEnableNewSession(const std::string& host, const SessionIP::Ptr& session)
{
	try
	{
		//LOG_DEBUG("Server: New session succeeded " << session->ToString());
		if (EnableNewSession)
			return EnableNewSession(host, session);

		return true; //by default if none is registered
	}
	catch (std::exception& ex)
	{
		LOG_ERROR("FireEnableNewSession failed. error=" << ex.what());
	}
	catch (...)
	{
		LOG_ERROR("FireEnableNewSession failed. unknown error!");
	}
	return false;
}

void UDPServerIP::FireSessionClosed(const SessionIP::Ptr& session)
{
	try
	{
		if (SessionClosed)
			SessionClosed(session);
	}
	catch (std::exception& ex)
	{
		LOG_ERROR("FireSessionClosed failed. error=" << ex.what());
	}
	catch (...)
	{
		LOG_ERROR("FireSessionClosed failed. unknown error!");
	}
}

bool UDPServerIP::ProcessMessage(UDPServerSessionIP& session_, const MessageIP& message)
{
	struct Processor: public IMessageIPVisitor
	{
		UDPServerIP& server;
		SessionsMap::iterator session;

		bool processed;

		Processor(UDPServerIP& server_, SessionsMap::iterator session_) :
			server(server_), session(session_)
		{
			processed = false;
		}

		bool Do(const MessageIP& message)
		{
			server.HasTraffic(session);

			const_cast<MessageIP&> (message).Accept(*this);
			return processed;
		}

		//IMessageIPVisitor
	private:
		void Visit(SessionInitiate_Request& req)
		{
			LOG_ERROR("SessionInitiate_Request not expected on active session! So close imediately session!");
			server.CloseSession(session, 0);
			processed = true;
		}

		void Visit(SessionInitiate_Response& res)
		{
			LOG_WARN("SessionInitiate_Response not expected on active session! Ignored!");
			processed = true;
		}

		void Visit(SessionClose_Request& req)
		{
			LOG_INFO("SessionClose_Request handled! Close session!");
			server.CloseSession(session, req.transactionID);
			processed = true;
		}

		void Visit(SessionClose_Response& res)
		{
			LOG_WARN("SessionClose_Response not expected on active session! Ignored!");
			processed = true;
		}

		void Visit(KeepAlive_Request& req)
		{
			LOG_INFO("KeepAlive_Request from "  << session->second << " handled!");
			KeepAlive_Response response;
			response.transactionID = req.transactionID;
			response.status = KeepAlive_Response::rcSuccess;
			if (session->second.session)
			{
				session->second.session->SendMessage(response);
			}
			processed = true;
		}

		void Visit(KeepAlive_Response& res)
		{
			LOG_WARN("KeepAlive_Response not expected on active session! Ignored!");
			processed = true;
		}

		void Visit(HARTWiredPDU_Request& req)
		{
		}

		void Visit(HARTWiredPDU_Response& res)
		{
		}

		void Visit(HARTWiredPDU_Notify& res)
		{
		}

		void Visit(HARTDirectPDU_Request& req)
		{
		}

		void Visit(HARTDirectPDU_Response& res)
		{
		}

		void Visit(HARTDirectPDU_Notify& res)
		{
		}

		void Visit(WirelessNPDU_Request& req)
		{
		}

		void Visit(WirelessNPDU_ACK& res)
		{
		}
	};

	SessionsMap::iterator activeSession = FindSesion(&session_);
	if (activeSession != sessions.end())
	{
		Processor p(*this, activeSession);
		return p.Do(message);
	}
	return false;
}

void UDPServerIP::CloseSession(SessionsMap::iterator activeSession, boost::uint32_t transactionID)
{
	LOG_WARN("Close Session=" << activeSession->second);
	if (!activeSession->second.IsActive())
	{
		LOG_WARN("Session not active.");
		return;
	}

	if (transactionID == 0)
	{
		SessionClose_Request close;
		close.status = 0;
		activeSession->second.session->SendMessage(close);
	}
	else
	{
		SessionClose_Response close;
		close.status = 0;
		close.transactionID = transactionID;
		activeSession->second.session->SendMessage(close);
	}
	FireSessionClosed(activeSession->second.session);
	activeSession->second.session.reset();
}

} // namespace transport
} // namespace stack
} // namespace hart7
