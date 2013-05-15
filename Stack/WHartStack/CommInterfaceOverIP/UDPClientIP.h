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
 * UDPSessionIP.h
 *
 *  Created on: Nov 28, 2008
 *      Author: ovidiu.rauca
 */
#ifndef UDPSESSIONIP_H_
#define UDPSESSIONIP_H_

#include <WHartStack/CommInterfaceOverIP/SessionIP.h>

#include <nlib/log.h>
#include <nlib/datetime.h>

#include <nlib/socket/Address.h>
#include <nlib/socket/ServiceSocket.h>

#include <nlib/timer/Timer.h>

#include <map>

namespace hart7 {
namespace stack {
namespace transport {

class UDPClientIP;

class UDPClientSessionIP: public SessionIP
{
    LOG_DEF("h7.s.t.UDPSessionIP");
public:
	typedef boost::shared_ptr<UDPClientSessionIP> Ptr;
public:
	UDPClientSessionIP(UDPClientIP& client, nlib::socket::UdpSocket::Ptr socket);
	virtual ~UDPClientSessionIP();

	void Peer(const nlib::socket::Address& peer_)
	{
		this->peer = peer_;
	}

	//SessionIP
public:
    virtual void AcceptStream(std::ostream& stream);
    friend std::ostream& operator<<(std::ostream& output, UDPClientSessionIP& serverSession);
	boost::uint16_t SendMessage(MessageIP& req);

private:
	void ReceiveFromHandler(const nlib::socket::Address& from, const boost::uint8_t* bytes, std::size_t bytesSize);

private:
	UDPClientIP& client;

	nlib::socket::UdpSocket::Ptr socket;
	nlib::socket::Address peer;

	boost::uint16_t nextTransactionID; //transaction per session

	friend class UDPClientIP;
};
std::ostream& operator<<(std::ostream& output, UDPClientSessionIP& serverSession);

//client session
class UDPClientIP: public ClientIP
{
    LOG_DEF("h7.s.t.UDPClientSessionIP");
	static const int TIMER_INTERVAL; //seconds
	static const boost::uint32_t TIMEOUT_KEEPALIVE_RESPONSE;
	static const boost::uint32_t TIMEOUT_SESSIONINIT;
public:
	typedef boost::shared_ptr<UDPClientIP> Ptr;
public:
	UDPClientIP(const nlib::socket::Address& serverAddress, int commandPort,
	  const nlib::socket::ServiceSocket::Ptr& service, nlib::timer::TimerFactory::Ptr timerFactory);
	virtual ~UDPClientIP();

public:
	bool ProcessMessage(const nlib::socket::Address& from, const MessageIP& message);

	void	SetInactivityCloseTime (boost::uint32_t p_u32InactivTime) { inactivityCloseTime = p_u32InactivTime; }
	void	SetGranularityKeepAlive (boost::uint32_t p_u32GranularityKeepAlive) { granularityKeepAlive = p_u32GranularityKeepAlive; }

	//ClientIP
private:
	void Connect();
	void Close();
	void Reset();
	SessionIP::Ptr ActiveSession();

private:
	void Close(bool notify);
	void FireNewSessionSucceeded();
	void FireNewSessionFailed(const std::string& message);
	void FireConnectionClosed();
	void CheckConnection(int timePassed);

private:
	UDPClientSessionIP::Ptr currentSession;
	nlib::socket::ServiceSocket::Ptr service;

	nlib::socket::Address serverAddress;
	int commandPort;
	bool isConnected;

	nlib::timer::TimerFactory::Ptr timerFactory;

	boost::uint32_t inactivityCloseTime; //time used to close the session
	boost::uint32_t lastReceivedMessage;
	boost::uint32_t lastSendAutoKeepAlive;
	boost::uint32_t granularityKeepAlive;
	boost::uint32_t connectionTime;

	bool waitingForKeepAliveResponse;
	boost::uint32_t timeElapsedSinceLastKeepAlive;

	nlib::timer::DeadlineTimer::Ptr keepAliveTimer;
};

} // namespace transport
} // namespace stack
} // namespace hart7
#endif /* UDPSESSIONIP_H_ */
