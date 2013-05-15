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

#ifndef SESSION_IP_H
#define SESSION_IP_H

#include <WHartStack/CommInterfaceOverIP/MessageIP.h>

#include <loki/Function.h>
#include <boost/shared_ptr.hpp>


namespace hart7 {
namespace stack {
namespace transport {

/**
 * @see document with title: hcfmin58_torino2008.pdf
 */

/**
 * the client session interface.
 */
class SessionIP
{
public:
	typedef boost::shared_ptr<SessionIP> Ptr;

public:
	virtual ~SessionIP() {}

	virtual void AcceptStream(std::ostream& stream) = 0;

	/**
	 * Sends request.
	 * @throws error ..
	 */
	virtual boost::uint16_t SendMessage(MessageIP& req) = 0;

	/**
	 * Notify when receives a response/notification.
	 */
	Loki::Function<void(const MessageIP&)> ReceiveMessage;

	/**
	 * Notify when receives a response/notification after it was processed by the address detector.
	 */
	Loki::Function<void(const MessageIP&)> ReceiveProcessedMessage;

	friend std::ostream& operator<<(std::ostream& output, SessionIP& session);
};

std::ostream& operator<<(std::ostream& output, SessionIP& session);


/**
 * the client session interface.
 */
class ClientIP
{
public:
	typedef boost::shared_ptr<ClientIP> Ptr;

public:
	virtual ~ClientIP() {}

	/**
	 * Starts the connection.
	 */
	virtual void Connect() = 0;

	/**
	 * Closes the connection.
	 */
	virtual void Close() = 0;

	/**
	 * Notifies when connection established
	 */
	Loki::Function<void()> NewSessionSucceeded;

	/**
	 * Notifies when connection failed
	 */
	Loki::Function<void(const std::string&)> NewSessionFailed;

	//boost::function0<void> SessionClosed;
	Loki::Function<void()> SessionClosed;

	/**
	 * Returns the active session, once the session is established.
	 */
	virtual SessionIP::Ptr ActiveSession() = 0;
};

/**
 * the server ip.
 * Responsibilities: wait for new connection, and established sessions if is enabled.
 */
class ServerIP
{
public:
	typedef boost::shared_ptr<ServerIP> Ptr;

	virtual ~ServerIP() {}

	virtual void Listen() = 0;

	/**
	 * Notifies when a new client session received.
	 * @return bool if is allowed, false if not
	 */
	Loki::Function<bool(const std::string& /*host*/, const SessionIP::Ptr&)> EnableNewSession;

	/**
	 * Notifies when a new session established.
	 */
	//boost::function2<void, const std::string& /*host*/, const SessionIP::Ptr> NewSession;
	Loki::Function<void(const std::string& /*host*/, const SessionIP::Ptr)> NewSession;

	/**
	 * Notifies when an session closed.
	 */
	Loki::Function<void(const SessionIP::Ptr)> SessionClosed;

	/**
	 * Release any resources.
	 */
	virtual void Close() = 0;

};


} //namspace transport
} //namespace stack
} //namespace hart7

#endif /*SESSION_IP_H*/
