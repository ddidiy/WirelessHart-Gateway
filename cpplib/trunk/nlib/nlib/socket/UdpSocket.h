/*
 * UdpSocket.h
 *
 *  Created on: Nov 28, 2008
 *      Author: ovidiu.rauca
 */

#ifndef NLIB_UDP_SOCKET_H_
#define NLIB_UDP_SOCKET_H_

#include "Address.h"

#include <nlib/log.h>

#include <boost/system/error_code.hpp> //for error codes
#include <boost/cstdint.hpp>
#include <boost/smart_ptr.hpp> //for scoped and shared ptr
#include <boost/noncopyable.hpp>

//#include <boost/function.hpp> //for callback
#include <loki/Function.h>

namespace nlib {
namespace socket {

namespace detail{
struct RawUdpSocket;
struct RawUdpEndPoint;
}


class UdpSocket: private boost::noncopyable
{
	LOG_DEF("nlib.socket.UdpSocket");
public:
	typedef boost::shared_ptr<UdpSocket> Ptr;

private:
	UdpSocket(detail::RawUdpSocket* socket);
public:
	virtual ~UdpSocket();

	void Port(const int port_);
	const int Port() const;

	void SendTo(const Address& to, const boost::uint8_t* bytes, std::size_t count); //sync send
//	boost::function3<void, const Address&/*from*/, const boost::uint8_t*, std::size_t> ReceivedFrom;
	typedef Loki::Function<void(const Address&/*from*/, const boost::uint8_t*, std::size_t)> ReceivedFromCallback;
	ReceivedFromCallback ReceivedFrom;

	void Listen(); //async read
	void Close();
	std::string ToString() const;

private:
	void ReceiveFromHandler(const boost::system::error_code& error, std::size_t bytes_transferred);

	const boost::scoped_ptr<detail::RawUdpSocket> socket;
	const boost::scoped_ptr<detail::RawUdpEndPoint> peerEndpoint; //const in order to decouple from asio endpoint

	char readBuffer[64 * 1024];
	int port;
	bool closed;
	friend class ServiceSocket;
};

}//namespace socket
}//namespace nlib

#endif /* UDPSOCKET_H_ */
