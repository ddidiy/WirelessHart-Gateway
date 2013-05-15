/*
 * ServiceSocket.h
 *
 *  Created on: Nov 27, 2008
 *      Author: nicu.dascalu
 */

#ifndef NLIB_SERVICESOCKET_H_
#define NLIB_SERVICESOCKET_H_

#include "TcpSocket.h"
#include "UdpSocket.h"
#include "Timer.h"


#include <nlib/log.h>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace nlib {
namespace socket {

namespace detail {
struct RawService;
}

class ServiceSocket: boost::noncopyable
{
	LOG_DEF("nlib.socket.ServiceSocket");

public:
	typedef boost::shared_ptr<ServiceSocket> Ptr;

	/*
	 * Used to create instances of this class allocated dynamically.
	 */
	static Ptr NewInstance();

private:
	ServiceSocket();

public:
	~ServiceSocket();

	void Run();
	void Stop();

	/**
	 * For testing purpose
	 */
	void Test_RunOne();

	TcpAcceptor::Ptr CreateTcpAcceptor() const;

	TcpSocket::Ptr CreateTcpSocket() const;

	UdpSocket::Ptr CreateUdpSocket() const;

	Timer::Ptr CreateDeadLineTimer() const;

private:
	/** this object is shared by each TcpSocket or TcpAcceptor instance.*/
	const boost::shared_ptr<detail::RawService> service;
};

} // namespace socket
} // namespace nlib


#endif /* NLIB_SERVICESOCKET_H_ */
