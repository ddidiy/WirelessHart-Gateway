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
 * GatewayChannel.cpp
 *
 *  
 */


#include "GatewayChannel.h"

#include <WHartHost/gateway/GatewayExceptions.h>

#include <nlib/socket/Address.h>
#include <boost/bind.hpp>

#include <iomanip>
#include <netinet/in.h>


namespace hart7 {
namespace gateway {


GatewayChannel::GatewayChannel()
{
	m_service = nlib::socket::ServiceSocket::NewInstance();
	m_timerFactory.reset(new nlib::timer::TimerFactory(m_service, 500));
}

GatewayChannel::~GatewayChannel()
{
}


//functionality
void GatewayChannel::Start(const std::string &hostIP, int hostPort, int localPort)
{
	m_client.reset(new hart7::stack::transport::UDPClientIP(nlib::socket::Address(hostIP, hostPort), localPort, m_service, m_timerFactory));
	m_client->NewSessionSucceeded = boost::bind(&GatewayChannel::SessionSucceeded, this);
	m_client->NewSessionFailed = boost::bind(&GatewayChannel::SessionFailed, this, _1);
	m_client->SessionClosed = boost::bind(&GatewayChannel::SessionClosed, this);
	m_client->Connect();
	
	LOG_DEBUG_APP("[GWChannel]: Connecting to gateway...");
}
void GatewayChannel::Run()
{
	m_service->Run();
}
void GatewayChannel::RunOnce()
{
	m_service->Test_RunOne();
}
void GatewayChannel::Stop()
{
	m_service->Stop();
}

//send msg
uint16_t GatewayChannel::SendMessage(stack::transport::MessageIP& message)
{
	uint16_t transactionID;
	
	if (m_connection != NULL)
	{
		try
		{
			transactionID = m_connection->SendMessage(message);
		}
		catch(std::exception& e)
		{
			THROW_EXCEPTION1(GWChannelException, "[GWChannel]: failed to send message!");
		}

		return transactionID;
	}
	
	THROW_EXCEPTION1(GWChannelException, "[GWChannel]: failed to send message!");
}


//connection
void GatewayChannel::SessionSucceeded()
{
	LOG_INFO_APP("[GWChannel]: Session succeeded!");
	m_connection = m_client->ActiveSession();
	assert(m_connection != NULL);
	m_connection->ReceiveMessage = boost::bind(&GatewayChannel::MessageReceived, this, _1);

	// notify of connected event
	ChangeState(true);

}
void GatewayChannel::SessionFailed(const std::string& reason)
{
	LOG_ERROR_APP("[GWChannel]: Session failed! Reason=" << reason);
	m_client->Connect();

	// notify of disconnected event
	ChangeState(false);

}
void GatewayChannel::SessionClosed()
{
	LOG_WARN_APP("[GWChannel]: Session closed. reconnecting to gateway...");
	m_client->Connect();
	
	// notify of disconnected event
	ChangeState(false);
}
void GatewayChannel::ChangeState(bool open)
{
	if (m_isOpen != open)
	{
		m_isOpen = open;
		if (!m_isOpen)
		{
			LOG_WARN_APP("[GWChannel]: Channel closed!");
			// notify of disconnected event
			for (std::vector<Loki::Function<void(void)> >::iterator it = m_gwDisconnected.begin(); 
						it != m_gwDisconnected.end(); ++it)
			{
				try
				{
					(*it)();
				}
				catch (std::exception& ex)
				{
					LOG_ERROR_APP("[GWChannel]: Fire event (when channel closed) failed! error=" << ex.what());
				}
				catch (...)
				{
					LOG_ERROR_APP("[GWChannel]: Fire event (when channel closed) failed! unknown error!");
				}
			}
			return;
		}
		

		LOG_INFO_APP("[GWChannel]: Channel opened!");
		for (std::vector<Loki::Function<void(const std::string&, int)> >::iterator it = m_gwConnected.begin(); 
					it != m_gwConnected.end(); ++it)
		{
			try
			{
				(*it)(m_host, m_port);
			}
			catch (std::exception& ex)
			{
				LOG_ERROR_APP("[GWChannel]: Fire event (when channel opened) failed! error=" << ex.what());
			}
			catch (...)
			{
				LOG_ERROR_APP("[GWChannel]: Fire event (when channel opened) failed! unknown error!");
			}
		}
	}
}

}
}
