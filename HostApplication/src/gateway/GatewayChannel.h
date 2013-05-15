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
 * GatewayChannel.h
 *
 */

#ifndef GATEWAYCHANNEL_H_
#define GATEWAYCHANNEL_H_


#include <WHartStack/CommInterfaceOverIP/UDPClientIP.h>

#include <nlib/log.h>
#include <loki/Function.h>


namespace hart7 {
namespace gateway {


/*
 * connection management
 */
class GatewayChannel
{
	LOG_DEF("hart7.gateway.GatewayChannel");

public:
	GatewayChannel();
	virtual ~GatewayChannel();


//functionality
public:
	void Start(const std::string &hostIP, int hostPort, int localPort);
	void Run();
	void RunOnce();
	void Stop();

//send msg
public:
	uint16_t SendMessage(stack::transport::MessageIP& message);

//receive msg
public:
	Loki::Function<void(const stack::transport::MessageIP&)>			m_messageReceived;
private:
	void MessageReceived(const stack::transport::MessageIP& message)
	{
		m_messageReceived(message);
	}

/* 
 * Observer Pattern
 */
public:
	std::vector<Loki::Function<void(void)> >						m_gwDisconnected;
	std::vector<Loki::Function<void(const std::string&, int)> >		m_gwConnected;
	nlib::timer::DeadlineTimer::Ptr GetTimer(int periodTime/*ms*/)
	{
		return m_timerFactory->CreateDeadlineTimer(periodTime);
	}

//connection
private:
	void SessionSucceeded();
	void SessionFailed(const std::string& reason);
	void SessionClosed();
private:
	void ChangeState(bool open);

//
private:
	stack::transport::ClientIP::Ptr		m_client;
	stack::transport::SessionIP::Ptr	m_connection;
	nlib::socket::ServiceSocket::Ptr	m_service;
	nlib::timer::TimerFactory::Ptr		m_timerFactory;

private:
	std::string m_host;
	int			m_port;

private:
	bool m_isOpen;

};

}
}

#endif /* GATEWAYCHANNEL_H_ */
