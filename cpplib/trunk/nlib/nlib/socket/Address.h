/*
 * Address.h
 *
 *  Created on: Dec 5, 2008
 *      Author: nicu.dascalu
 */

#ifndef NLIB_ADDRESS_SOCKET_H_
#define NLIB_ADDRESS_SOCKET_H_

//#include "detail/Asio.h"
#include <string>

namespace nlib {
namespace socket {

struct Address
{
private:
	//boost::scoped_ptr<detail::RawUdpEndPoint> address;
	std::string host;
	int port;

public:
	Address()
	{
		host = "127.0.0.1";
		port = 0;
	}

	Address(const std::string& host_, int port_)
	: host(host_), port(port_)
	{
	}
	const Address& operator=(const Address& rigth);

	const std::string& Host() const
	{
		return host;
	}
	void Host(const std::string& host_)
	{
		host = host_;
	}

	const int Port() const
	{
		return port;
	}
	void Port(const int port_)
	{
		port = port_;
	}

	std::string ToString() const;

	friend bool operator<(const Address& left, const Address& rigth);
};

bool operator<(const Address& left, const Address& rigth);
bool operator==(const Address& left, const Address& rigth);
bool operator!=(const Address& left, const Address& rigth);

} //namespace socket
} //namespace nlib


#endif /* NLIB_ADDRESS_SOCKET_H_ */
