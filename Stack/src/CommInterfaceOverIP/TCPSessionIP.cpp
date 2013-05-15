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

#include <WHartStack/CommInterfaceOverIP/TCPSessionIP.h>

#include <boost/bind.hpp>

namespace hart7 {
namespace stack {
namespace transport {

TCPSessionIP::TCPSessionIP(nlib::socket::TcpSocket::Ptr socket_) :
	socket(socket_), nextTransactionID(0)
{
	socket->Connected = boost::bind(&TCPSessionIP::HandleSocketConnected, this, _1);
	socket->Received = boost::bind(&MessageIPUnserializer::ParseBytes, &unserializer, _1, _2);

	unserializer.MessageUnserialized = boost::bind(&TCPSessionIP::MessageReceived, this, _1);
}

void TCPSessionIP::AcceptStream(std::ostream& stream)
{
    stream << *this;
}
bool TCPSessionIP::IsConnected() const
{
	return socket.get() != 0;
}

std::string TCPSessionIP::ToString() const
{
	if (IsConnected())
	{
		return socket->ToString();
	}
	else
	{
		return "[Not Connected]";
	}
}

void TCPSessionIP::AcceptStream(std::ostream& stream)
{
    if (IsConnected())
    {
        stream << socket;
    }
    else
    {
        stream << "[Not Connected]";
    }
}


boost::uint16_t TCPSessionIP::SendMessage(MessageIP& message)
{
	LOG_DEBUG("Sending message...");
	MessageIPSerializer::BufferOut buffer;
	message.transactionID = nextTransactionID++;
	serializer.Serialize(message, buffer);

	socket->SyncSend(buffer.data(), buffer.size());
	socket->Flush();

	return message.transactionID;
}

void TCPSessionIP::TimeElapsed(int msecsElapsed)
{

}

void TCPSessionIP::MessageReceived(const MessageIP& message)
{
	LOG_DEBUG("Message received in session...");
	if (ReceiveMessage)
		ReceiveMessage(message);
}

void TCPSessionIP::Close()
{
	//from specs Note:
	//	For TCP, the session is closed whenever the socket for this session is closed for any reason.
	socket->Close();
	socket.reset();
}

void TCPSessionIP::HandleSocketConnected(bool connected)
{
	if (connected)
	{
		LOG_WARN("HandleSocketConnected: should not be called with connected=true!");
		return;
	}

	FireReceiveDisconnect();
	socket->Close();
	socket.reset();
}

void TCPSessionIP::FireReceiveDisconnect()
{
	try
	{
		if (ReceiveDisconnect)
			ReceiveDisconnect();
	} catch (std::exception& ex)
	{
		LOG_ERROR("FireReceiveDisconnect failed. error=" << ex.what());
	} catch (...)
	{
		LOG_ERROR("FireReceiveDisconnect failed. unknown error!");
	}
}

TCPClientIP::TCPClientIP(const nlib::socket::ServiceSocket::Ptr& service_) :
	service(service_)
{
	socket = service->CreateTcpSocket();
}

void TCPClientIP::Connect()
{
	if (!socket.get())
	{
		LOG_WARN("Connect: already connected!");
		return;
	}

	socket->Connected = boost::bind(&TCPClientIP::HandleSocketConnected, this, _1);
	socket->AsyncConnect();
}

void TCPClientIP::Close()
{

}

SessionIP::Ptr TCPClientIP::ActiveSession()
{
	return SessionIP::Ptr();
}

void TCPClientIP::HandleSocketConnected(bool connected)
{
	if (connected)
	{
		LOG_DEBUG("Client connected...");
		pendingSession.reset(new TCPSessionIP(socket));
		pendingSession->ReceiveMessage = boost::bind(&TCPClientIP::HandleInitiateResponse, this, _1);

		SessionInitiate_Request initiate;
		initiate.masterType = 0;
		initiate.inactivityCloseTime = 0;

		pendingSession->SendMessage(initiate);
	}
	else
	{
		FireNewSessionFailed("socket connection failed.");
	}
}

void TCPClientIP::HandleInitiateResponse(const MessageIP& message)
{
	if (message.parsedStatus)
	{
		const SessionInitiate_Response* response = dynamic_cast<const SessionInitiate_Response*> (&message);
		if (response && response->status == SessionInitiate_Response::rcSuccess)
		{
			FireNewSessionSucceeded();
			//			pendingSession.reset();
		}
		else
		{
			//			pendingSession->Close();
			pendingSession.reset();
			FireNewSessionFailed("initiate response failed or unexpected response.");
		}
	}
	else
	{
		//		pendingSession->Close();
		pendingSession.reset();
		FireNewSessionFailed("parser failed.");
	}
}

void TCPClientIP::FireNewSessionSucceeded()
{
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

void TCPClientIP::FireNewSessionFailed(const std::string& message)
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

TCPServerIP::TCPServerIP(const nlib::socket::ServiceSocket::Ptr& service_) :
	service(service_)
{
	BOOST_ASSERT(service != NULL);
	acceptor = service->CreateTcpAcceptor();
	BOOST_ASSERT(acceptor != NULL);
}

void TCPServerIP::Listen()
{
	if (!acceptor.get())
	{
		LOG_WARN("Server already closed!");
		return;
	}
	acceptor->NewConnection = boost::bind(&TCPServerIP::NewConnection, this, _1);
	acceptor->Listen();
}

void TCPServerIP::Close()
{
	if (acceptor.get())
	{
		acceptor->Close();
		acceptor.reset();
	}
}

void TCPServerIP::NewConnection(nlib::socket::TcpSocket::Ptr socket)
{
	LOG_DEBUG("Server received new connection...");
	SessionIP::Ptr session(new TCPSessionIP(socket));
	pendingSesions.push_back(session);

	session->ReceiveMessage = boost::bind(&TCPServerIP::HandleInitiateRequest, this, session, socket->Host(), _1);
}

void TCPServerIP::HandleInitiateRequest(const SessionIP::Ptr& session, const std::string& host,
		const MessageIP& message)
{
	//session->ReceiveMessage.reset();
	LOG_DEBUG("Received message on server side...");
	if (message.parsedStatus)
	{
		const SessionInitiate_Request* request = dynamic_cast<const SessionInitiate_Request*> (&message);
		if (request && FireEnableNewSession(host, session))
		{
			SessionInitiate_Response response;
			response.transactionID = request->transactionID;
			response.status = SessionInitiate_Response::rcSuccess;
			session->SendMessage(response);

			pendingSesions.remove(session);
			establishedSesions.push_back(session);

			FireNewSession(host, session);
		}
		else
		{
			SessionInitiate_Response response;
			response.transactionID = request->transactionID;
			response.status = SessionInitiate_Response::rcErr_AllAvailableSessionsInUse;

			session->SendMessage(response);
			pendingSesions.remove(session);
			//			session->Close();
		}
	}
	else
	{
		pendingSesions.remove(session);
		//		session->Close();
	}
}

bool TCPServerIP::FireEnableNewSession(const std::string& host, const TCPSessionIP::Ptr& session)
{
	try
	{
		if (EnableNewSession)
			return EnableNewSession(host, session);

		return true; //by default if none is registered
	} catch (std::exception& ex)
	{
		LOG_ERROR("FireEnableNewSession failed. error=" << ex.what());
	} catch (...)
	{
		LOG_ERROR("FireEnableNewSession failed. unknown error!");
	}
	return false;
}

void TCPServerIP::FireNewSession(const std::string& host, const TCPSessionIP::Ptr& session)
{
	try
	{
		if (NewSession)
			NewSession(host, session);
	} catch (std::exception& ex)
	{
		LOG_ERROR("FireNewSession failed. error=" << ex.what());
	} catch (...)
	{
		LOG_ERROR("FireNewSession failed. unknown error!");
	}
}

} // namespace transport
} // namespace stack
} // namespace hart7
