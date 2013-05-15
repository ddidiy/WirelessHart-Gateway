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

#ifndef TCP_SESSION_IP_H
#define TCP_SESSION_IP_H

#include <WHartStack/CommInterfaceOverIP/SessionIP.h>
#include <WHartStack/CommInterfaceOverIP/MessageIPSerializer.h>

#include <nlib/log.h>
#include <nlib/socket/TcpSocket.h>
#include <nlib/socket/ServiceSocket.h>
#include <loki/Function.h>
#include <list>

namespace hart7 {
namespace stack {
namespace transport {

class TCPSessionIP: public SessionIP
{
    LOG_DEF("h7.s.t.TCPSessionIP");
public:
	TCPSessionIP(nlib::socket::TcpSocket::Ptr socket);

	typedef Loki::Function<void()> ReceiveDisconnectCallback;
	ReceiveDisconnectCallback ReceiveDisconnect;

    virtual void AcceptStream(std::ostream& stream);

	//SessionIP
private:
	bool IsConnected() const;
	std::string ToString() const;
	boost::uint16_t SendMessage(MessageIP& message);
	void TimeElapsed(int msecsElapsed);
	void Close();

	void HandleSocketConnected(bool connected);
	void FireReceiveDisconnect();

private:
	void MessageReceived(const MessageIP& message);

	MessageIPUnserializer unserializer;
	MessageIPSerializer serializer;
	nlib::socket::TcpSocket::Ptr socket;

	boost::uint16_t nextTransactionID;

	friend class TCPServerIP;
};

class TCPClientIP: public ClientIP
{
    LOG_DEF("h7.s.t.TCPClientSessionIP");
public:
	TCPClientIP(const nlib::socket::ServiceSocket::Ptr& service);

	//ClientSessionIP
private:
	void Connect();
	void Close();
	SessionIP::Ptr ActiveSession();

	void HandleSocketConnected(bool connected);
	void HandleInitiateResponse(const MessageIP& message);
	void FireNewSessionSucceeded();
	void FireNewSessionFailed(const std::string& message);

private:
	nlib::socket::ServiceSocket::Ptr service;
	nlib::socket::TcpSocket::Ptr socket;
	TCPSessionIP::Ptr pendingSession;
};

class TCPServerIP: public ServerIP
{
    LOG_DEF("h7.s.t.TCPServerIP");
public:
	TCPServerIP(const nlib::socket::ServiceSocket::Ptr& service);

	//ServerIP
private:
	void Listen();
	void Close();

	void NewConnection(nlib::socket::TcpSocket::Ptr socket);
	void HandleInitiateRequest(const SessionIP::Ptr& session, const std::string& host, const MessageIP& message);

	bool FireEnableNewSession(const std::string& host, const TCPSessionIP::Ptr& session);
	void FireNewSession(const std::string& host, const TCPSessionIP::Ptr& session);

private:
	const nlib::socket::ServiceSocket::Ptr service;
	nlib::socket::TcpAcceptor::Ptr acceptor;

	typedef std::list<SessionIP::Ptr> Sessions;
	Sessions pendingSesions;
	Sessions establishedSesions;
};

} // namespace transport
} // namespace stack
} // namespace hart7

#endif /*TCP_SESSION_IP_H*/
