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
 * UDPServerIP.h
 *
 *  Created on: Nov 28, 2008
 *      Author: ovidiu.rauca
 */

#ifndef UDPSERVERIP_H_
#define UDPSERVERIP_H_

#include <WHartStack/CommInterfaceOverIP/SessionIP.h>
#include <WHartStack/CommInterfaceOverIP/MessageIPSerializer.h>

#include <nlib/log.h>
#include <nlib/socket/ServiceSocket.h>
#include <nlib/timer/Timer.h>
#include <nlib/datetime.h>
#include <map>

namespace hart7 {
namespace stack {
namespace transport {

class UDPServerIP;

class UDPServerSessionIP: public SessionIP
{
    LOG_DEF("h7.s.t.UDPServerSessionIP");

public:
	UDPServerSessionIP(UDPServerIP& server, nlib::socket::UdpSocket::Ptr socket, const nlib::socket::Address& peer);
	virtual ~UDPServerSessionIP();

	//SessionIP
    virtual void AcceptStream(std::ostream& stream);
	boost::uint16_t SendMessage(MessageIP& req);

	friend std::ostream& operator<<(std::ostream& output, UDPServerSessionIP& serverSession);

private:
	void ReceiveFromHandler(const nlib::socket::Address& from, const boost::uint8_t* bytes, std::size_t bytesSize);

private:
	UDPServerIP& server;
	MessageIPUnserializer unserializer;
	MessageIPSerializer serializer;

	nlib::socket::UdpSocket::Ptr socket;
	nlib::socket::Address peer;

	boost::uint16_t nextTransactionID; //transaction per session
};
std::ostream& operator<<(std::ostream& output, UDPServerSessionIP& serverSession);

//server sessions
class UDPServerIP: public ServerIP
{
    LOG_DEF("h7.s.t.UDPServerIP");

	static const int TIMER_INTERVAL; //seconds
	static const boost::uint32_t MAX_INACTIVE_CLOSE_TIME; //seconds

public:
	struct SessionMapping
	{
		const int port;
		nlib::socket::Address peer;
		boost::shared_ptr<UDPServerSessionIP> session;
		boost::uint32_t lastTraffic;

		SessionMapping(int port_) : port(port_)
		{
			lastTraffic = 0;
		}

		bool IsActive() const
		{
			return session;
		}

	    friend std::ostream& operator<<(std::ostream& output, SessionMapping& sessionMapping);
	};
	typedef std::map<int, SessionMapping> SessionsMap;

public:
	UDPServerIP(const nlib::socket::ServiceSocket::Ptr& service, nlib::timer::TimerFactory::Ptr timerFactory, int listenPort, int minPort, int maxPort);

	//ServerIP
public:
	void Listen();
	void Close();

public:
	bool FireEnableNewSession(const std::string& host, const SessionIP::Ptr& session);
	void FireNewSession(const std::string& host, const SessionIP::Ptr& session);
	void FireSessionClosed(const SessionIP::Ptr& session);
	bool ProcessMessage(UDPServerSessionIP& session, const MessageIP& message);
	void CloseAllSessions();

private:
	SessionsMap::iterator GetNextAvailable(const nlib::socket::Address& address);
	SessionsMap::iterator FindSesion(UDPServerSessionIP* session);

	void HasTraffic(SessionsMap::iterator session);
	void CloseSession(SessionsMap::iterator session, boost::uint32_t transactionID);

	void HandleNewSession(const nlib::socket::Address& from, const boost::uint8_t* bytes, std::size_t size);
	void CloseInactiveSessions(int interval);

public:
	// the configured close time in milliseconds
	boost::uint32_t inactiveCloseTime;

	MessageIPUnserializer unserializer;
	const nlib::socket::ServiceSocket::Ptr service;
	nlib::timer::TimerFactory::Ptr timerFactory;

	nlib::socket::UdpSocket::Ptr listenSocket;
	SessionsMap sessions;

	nlib::timer::DeadlineTimer::Ptr checkInactiveSessions;

	// holds the current time in milliseconds since the server started
	boost::uint32_t currentTime;
};

std::ostream& operator<<(std::ostream& output, UDPServerIP::SessionMapping& sessionMapping);

} // namespace transport
} // namespace stack
} // namespace hart7

#endif /* UDPSERVERIP_H_ */
