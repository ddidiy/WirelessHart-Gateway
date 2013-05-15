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
 * UDPSessionIP.cpp
 *
 *  Created on: Nov 28, 2008
 *      Author: ovidiu.rauca
 */

#include <WHartStack/CommInterfaceOverIP/UDPClientIP.h>
#include <WHartStack/CommInterfaceOverIP/MessageIPSerializer.h>

#include <nlib/detail/bytes_print.h>
#include <nlib/sformat.h>

#include <boost/bind.hpp>
#include <boost/assert.hpp>

#include <stdio.h>

namespace hart7 {
namespace stack {
namespace transport {

UDPClientSessionIP::UDPClientSessionIP(UDPClientIP& client_, nlib::socket::UdpSocket::Ptr socket_) :
	client(client_), socket(socket_)
{
	BOOST_ASSERT(socket != NULL);
	nextTransactionID = 0;

	socket->ReceivedFrom = boost::bind(&UDPClientSessionIP::ReceiveFromHandler, this, _1, _2, _3);
//	socket->ReceivedFrom = nlib::socket::UdpSocket::ReceivedFromCallback(this, &UDPClientSessionIP::ReceiveFromHandler);
}

UDPClientSessionIP::~UDPClientSessionIP()
{
	if (socket)
		socket->Close();
	socket.reset();
}

void UDPClientSessionIP::AcceptStream(std::ostream& stream)
{
    stream << *this;
}

std::ostream& operator<<(std::ostream& output, UDPClientSessionIP& session)
{
    output << "[Session from=" << session.socket->ToString() << " to=" << session.peer.ToString() << "]";

    return output;
}

boost::uint16_t UDPClientSessionIP::SendMessage(MessageIP& message)
{
	if (message.transactionID == 0xFFFF)
	{
		nextTransactionID = (nextTransactionID + 1) % 0xFFFF;
		message.transactionID = nextTransactionID;
	}

	MessageIPSerializer::BufferOut buffer;
	buffer.reserve(128);
	MessageIPSerializer().Serialize(message, buffer);

	socket->SendTo(peer, buffer.data(), buffer.size());

	return message.transactionID;
}

void UDPClientSessionIP::ReceiveFromHandler(const nlib::socket::Address& from, const boost::uint8_t* bytes,
		std::size_t bytesSize)
{
	LOG_DEBUG("ReceiveFromHandler session=" << *this << " from=" << from.ToString() << " size=" << bytesSize);
//			<< " bytes=" << nlib::detail::BytesToString(bytes, bytesSize));

	//TODO investigate how to reject packets from other sources.
	if (true)//from == peer)
	{
		MessageIP::Ptr message = MessageIPUnserializer().ParseBytesComplete(bytes, bytesSize);
		if (!message || !message->parsedStatus)
		{
			LOG_ERROR("Failed to parse bytes! size=" << bytesSize << " bytes=" << nlib::detail::BytesToString(bytes, bytesSize));
			return;
		}

//		LOG_INFO("ReceiveFromHandler session=" << *this << " from=" << from.ToString() << " message="
//				<< *message);

		try
		{
			//first we forward to client to handle some messages
			if (!client.ProcessMessage(from, *message))
			{
				if (ReceiveMessage)
					ReceiveMessage(*message);
			}
		} catch (std::exception& ex)
		{
			LOG_ERROR("Failed to process message=" << *message << ". error=" << ex.what());
		} catch (...)
		{
			LOG_ERROR("Failed to process message=" << *message << ". unknown error!");
		}
	}
	else
	{
		LOG_WARN("Packet recevied on session=" << *this << " comes from a different IP(" << from.ToString() << "). Ignoring.");
	}
}

/**************************************************************/

const int UDPClientIP::TIMER_INTERVAL = 1 * 1000; //seconds
const boost::uint32_t UDPClientIP::TIMEOUT_KEEPALIVE_RESPONSE = 15 * 1000;
const boost::uint32_t UDPClientIP::TIMEOUT_SESSIONINIT = 5 * 1000;

UDPClientIP::UDPClientIP(const nlib::socket::Address& serverAddress_, int commandPort_,
		const nlib::socket::ServiceSocket::Ptr& service_, nlib::timer::TimerFactory::Ptr timerFactory_) :
	service(service_), serverAddress(serverAddress_), commandPort(commandPort_), timerFactory(timerFactory_)
{
	inactivityCloseTime = 60 * 1000; //60's
	granularityKeepAlive = 30 * 1000; //10's

	Reset();
}

UDPClientIP::~UDPClientIP()
{
	LOG_WARN("Destroying the UDPClient session ..");
	Close();
}

void UDPClientIP::Reset()
{
	lastReceivedMessage = 0;
	lastSendAutoKeepAlive = 0;
	timeElapsedSinceLastKeepAlive = 0;
	isConnected = false;
	connectionTime = 0;
	waitingForKeepAliveResponse = false;
}

void UDPClientIP::Connect()
{
	if (isConnected)
	{
		LOG_WARN("Already connected! session=" << *currentSession);
		return;
	}

	Reset();

	LOG_INFO("Client initiate session on port=" << commandPort << " with server=" << serverAddress.ToString());

	if (!service)
	{
        LOG_ERROR("Unable to create Udp Socket! service=NULL");
        return;
	}

	// create session
	nlib::socket::UdpSocket::Ptr socket = service->CreateUdpSocket();
	if (!socket)
	{
	    LOG_ERROR("Fail to create Udp Socket! port=" << commandPort << ", server=" << serverAddress.ToString());
        return;
	}

	socket->Port(commandPort);
	currentSession.reset(new UDPClientSessionIP(*this, socket));
	currentSession->Peer(serverAddress);
	socket->Listen();

	waitingForKeepAliveResponse = false;
	connectionTime = 0;

	//create timer
	keepAliveTimer = timerFactory->CreateDeadlineTimer(TIMER_INTERVAL);
	keepAliveTimer->Elapsed = boost::bind(&UDPClientIP::CheckConnection, this, _1);

	//send initiate session
	SessionInitiate_Request initiate;
	initiate.transactionID = 0;
	initiate.masterType = 0;
	initiate.inactivityCloseTime = inactivityCloseTime;
	currentSession->SendMessage(initiate);
}

void UDPClientIP::Close()
{
    LOG_INFO("Client initiate close on port=" << commandPort << " with server=" << serverAddress.ToString());

	if (isConnected)
	{
		SessionClose_Request req;
		req.transactionID = 1;
		currentSession->SendMessage(req);
	}

	Close(true);
}

void UDPClientIP::Close(bool notify)
{
    LOG_WARN("Client initiate close on port=" << commandPort << " with server=" << serverAddress.ToString() << ", notify=" << notify);

	isConnected = false;

	if (keepAliveTimer)
	{
		keepAliveTimer->Elapsed = 0;
	}
	keepAliveTimer.reset();

	LOG_INFO("Disconnected!!!");

	if (currentSession && currentSession->socket)
		currentSession->socket->Close();
	currentSession.reset();

	if (notify)
		FireConnectionClosed();
}

SessionIP::Ptr UDPClientIP::ActiveSession()
{
	return isConnected ? currentSession : SessionIP::Ptr();
}

void UDPClientIP::CheckConnection(int timePassed)
{

	lastReceivedMessage += timePassed;
	lastSendAutoKeepAlive += timePassed;
	if (!isConnected)
	{
		connectionTime += timePassed;
	}

	if (lastReceivedMessage >= granularityKeepAlive && !waitingForKeepAliveResponse)//lastSendAutoKeepAlive >= granularityKeepAlive)
	{
		LOG_INFO("Send Keep alive request...");
		KeepAlive_Request req;
		req.transactionID = 2;
		currentSession->SendMessage(req);

		lastSendAutoKeepAlive = 0; //now
		timeElapsedSinceLastKeepAlive = 0;
		waitingForKeepAliveResponse = true;
	}
	else if (waitingForKeepAliveResponse)	// do not enter this branch if we have just set waitingForKARespose to true
	{
		if (timeElapsedSinceLastKeepAlive >= TIMEOUT_KEEPALIVE_RESPONSE)
		{
			waitingForKeepAliveResponse = false;
			LOG_WARN("Keep alive request timeout expired! Closing the session...");
			Close();
			return;
		}

		timeElapsedSinceLastKeepAlive += timePassed;
	}

	if (!isConnected && (connectionTime > TIMEOUT_SESSIONINIT))
	{
		LOG_WARN("Connection not established in the specified time. Closing the session...");
		Close();
		return;
	}

	if (lastReceivedMessage > inactivityCloseTime)
	{
		//TODO:also close
		lastReceivedMessage = 0;
	}
}

bool UDPClientIP::ProcessMessage(const nlib::socket::Address& from, const MessageIP& message)
{
	struct Processor: public IMessageIPVisitor
	{
		UDPClientIP& client;
		const nlib::socket::Address& from;
		bool processed;
		bool resetTimer;

		Processor(UDPClientIP& client_, const nlib::socket::Address& from_) :
			client(client_), from(from_)
		{
			processed = false; resetTimer = false;
		}

		bool Do(const MessageIP& message)
		{
			const_cast<MessageIP&> (message).Accept(*this);
			return processed;
		}

		//IMessageIPVisitor
	private:
		void Visit(SessionInitiate_Request& req)
		{
			LOG_WARN("SessionInitiate request not expected...");
			processed = true;
			resetTimer = true;
		}

		void Visit(SessionInitiate_Response& res)
		{
			resetTimer = true;
			if (!client.isConnected)
			{
				if (res.status == SessionInitiate_Response::rcSuccess)
				{
					client.currentSession->Peer(from);
					client.isConnected = true;
					client.inactivityCloseTime = res.inactivityCloseTime;
					client.FireNewSessionSucceeded();
				}
				else
				{
					client.Close(false);
					client.FireNewSessionFailed("SessionInitiate response failed or unexpected response.");
				}
			}
			else
			{
				LOG_WARN("SessionInitiate response not expected...");
			}
			processed = true;
		}

		void Visit(SessionClose_Request& req)
		{
			resetTimer = true;
			LOG_DEBUG("Session close requested...");
			SessionClose_Response resp;
			resp.transactionID = req.transactionID;

			client.currentSession->SendMessage(req);

			client.Close(true);
			processed = true;
		}

		void Visit(SessionClose_Response& res)
		{
			resetTimer = true;
			LOG_DEBUG("Session close responded...");
			processed = true;
		}

		void Visit(KeepAlive_Request& req)
		{
			// not expected
			resetTimer = true;
			processed = true;
		}

		void Visit(KeepAlive_Response& res)
		{
			resetTimer = true;
			if (client.waitingForKeepAliveResponse)
			{
				LOG_DEBUG("Keep alive responded... transactionID=" << res.transactionID);
				client.waitingForKeepAliveResponse = false;
				client.timeElapsedSinceLastKeepAlive = 0;
			}
			processed = true;
		}

		void Visit(HARTWiredPDU_Request& req)
		{
			resetTimer = true;
		}

		void Visit(HARTWiredPDU_Response& res)
		{
			resetTimer = true;
		}

		void Visit(HARTWiredPDU_Notify& res)
		{
			resetTimer = true;
		}

		void Visit(HARTDirectPDU_Request& req)
		{
			resetTimer = true;
		}

		void Visit(HARTDirectPDU_Response& res)
		{
			resetTimer = true;
		}

		void Visit(HARTDirectPDU_Notify& res)
		{
			resetTimer = true;
		}

		void Visit(WirelessNPDU_Request& req)
		{
			resetTimer = true;
		}

		void Visit(WirelessNPDU_ACK& res)
		{
			resetTimer = true;
		}
	};

	//  SesionInitate_Response comes from another server port
	//	if (from != currentSession->peer)
	//	{
	//		//ignore packet not expected from that destination
	//		LOG_WARN("HandleMessage session=" << currentSession->ToString()
	//				<< " from=" << from.ToString() << " unexpected source! Ignore message."
	//				<< " message=" << message.ToString());
	//		return true;
	//	}

	Processor p(*this, from);
	bool result = p.Do(message);
	if (p.resetTimer)
	{
		lastReceivedMessage = 0;
	}

	return result;
}

void UDPClientIP::FireNewSessionSucceeded()
{
	LOG_INFO("Client: New session succeeded " << *currentSession);

	try
	{
		if (NewSessionSucceeded)
			NewSessionSucceeded();
	} catch (std::exception& ex)
	{
		LOG_ERROR("FireNewSessionSucceeded failed. error=" << ex.what());
	} catch (...)
	{
		LOG_ERROR("FireNewSessionSucceeded failed. unknown error!");
	}
}

void UDPClientIP::FireNewSessionFailed(const std::string& message)
{
	try
	{
		if (NewSessionFailed)
			NewSessionFailed(message);
	} catch (std::exception& ex)
	{
		LOG_ERROR("FireNewSessionFailed failed. error=" << ex.what());
	} catch (...)
	{
		LOG_ERROR("FireNewSessionFailed failed. unknown error!");
	}
}

void UDPClientIP::FireConnectionClosed()
{
	try
	{
		if (SessionClosed)
			SessionClosed();
	} catch (std::exception& ex)
	{
		LOG_ERROR("FireConnectionClosed failed. error=" << ex.what());
	} catch (...)
	{
		LOG_ERROR("FireConnectionClosed failed. unknown error!");
	}
}

/********************************************************************/

} // namespace transport
} // namespace stack
} // namespace hart7
