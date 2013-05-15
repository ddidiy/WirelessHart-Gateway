/*
 * asio_socket.cpp
 *
 *  Created on: Nov 27, 2008
 *      Author: nicu.dascalu
 */

//WARNING!!!!: [andy] must include this header before all others that include boost for ASIO to work
#ifdef _WIN32_WINNT
#	define __USE_W32_SOCKETS
#	include <winsock2.h> //hack [nicu.dascalu] - should be included before asio
#endif

#include <cassert>

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>


#include <nlib/exception.h>
#include <nlib/detail/bytes_print.h>
#include <nlib/socket/ServiceSocket.h>
#include <nlib/socket/TcpSocket.h>


namespace nlib {
namespace socket {
namespace detail {

struct RawService : private boost::noncopyable
{
	boost::asio::io_service ioservice;
};

struct RawTcpSocket : private boost::noncopyable
{
public:
	RawTcpSocket(boost::shared_ptr<RawService> service_) :
		service(service_), socket(service_->ioservice)
	{
	}

	/** assures that the socket has a valid service object. */
	const boost::shared_ptr<RawService> service;
	boost::asio::ip::tcp::socket socket;
};

struct RawTcpAcceptor : private boost::noncopyable
{
	RawTcpAcceptor(boost::shared_ptr<RawService> service_) :
		service(service_), acceptor(service_->ioservice)
	{
	}

	/** assures that the socket has a valid service object. */
	const boost::shared_ptr<RawService> service;
	boost::asio::ip::tcp::acceptor acceptor;
};

struct RawUdpSocket : private boost::noncopyable
{
	RawUdpSocket(boost::shared_ptr<RawService> service_) :
		service(service_), socket(service_->ioservice)
	{
	}

	/** assures that the socket has a valid service object. */
	const boost::shared_ptr<RawService> service;
	boost::asio::ip::udp::socket socket;
};

struct RawUdpEndPoint : private boost::noncopyable
{
	RawUdpEndPoint()
	{
	}

	boost::asio::ip::udp::endpoint endpoint;
};

struct RawDeadlineTimer : private boost::noncopyable
{
	RawDeadlineTimer(boost::shared_ptr<socket::detail::RawService> service_) :
		service(service_), timer(service_->ioservice)
	{
	}

	const boost::shared_ptr<socket::detail::RawService> service;
	boost::asio::deadline_timer timer;
};

} // namespace detail


/*--------------------------------------
 *--------------ServiceSocket----------*/

ServiceSocket::Ptr ServiceSocket::NewInstance()
{
	return Ptr(new ServiceSocket());
}

ServiceSocket::ServiceSocket() :
	service(new detail::RawService())
{
}

ServiceSocket::~ServiceSocket()
{
}

void ServiceSocket::Run()
{
	service->ioservice.run();
}

void ServiceSocket::Test_RunOne()
{
	service->ioservice.run_one();
}


void ServiceSocket::Stop()
{
	service->ioservice.stop();
}

TcpAcceptor::Ptr ServiceSocket::CreateTcpAcceptor() const
{
	return TcpAcceptor::Ptr(new TcpAcceptor(new detail::RawTcpAcceptor(service)));
}

TcpSocket::Ptr ServiceSocket::CreateTcpSocket() const
{
	return TcpSocket::Ptr(new TcpSocket(new detail::RawTcpSocket(service)));
}

UdpSocket::Ptr ServiceSocket::CreateUdpSocket() const
{
	return UdpSocket::Ptr(new UdpSocket(new detail::RawUdpSocket(service)));
}

Timer::Ptr ServiceSocket::CreateDeadLineTimer() const
{
	return Timer::Ptr(new Timer(new detail::RawDeadlineTimer(service)));
}

//const std::string& ToString() const { return toString; }


/*--------------------------------------
 * ---------------UdpSocket-------------*/
UdpSocket::UdpSocket(detail::RawUdpSocket* socket_) :
	socket(socket_), peerEndpoint(new detail::RawUdpEndPoint())
{
	assert(socket_ && "socket should not be NULL");
	closed = true;
}

UdpSocket::~UdpSocket()
{
}

void UdpSocket::Port(const int port_)
{
	port = port_;
	if (socket->socket.is_open())
	{
		THROW_EXCEPTION1(nlib::Exception, "Socket already opened!");
	}
	boost::asio::ip::udp::endpoint endpoint(boost::asio::ip::udp::v4(), port);
	socket->socket.open(endpoint.protocol());
	socket->socket.bind(endpoint);
	closed = false;
	LOG_DEBUG("Socket=" << ToString() << "open and bind.");
}

const int UdpSocket::Port() const
{
	return port;
}

void UdpSocket::SendTo(const Address& to, const boost::uint8_t* bytes, std::size_t count) //sync
{
	LOG_INFO("SendTo to=" << to.ToString() << " count=" << count);// << " bytes=" << nlib::detail::BytesToString(bytes,
			//count));

	boost::asio::ip::udp::endpoint endpoint(boost::asio::ip::address::from_string(to.Host()), to.Port());
	if (socket)
	{
		socket->socket.send_to(boost::asio::buffer((const char*) bytes, count), endpoint);
	}
}

void UdpSocket::Listen()
{
	if (!closed && socket)
	{
		socket->socket.async_receive_from(boost::asio::buffer(readBuffer, sizeof(readBuffer)), peerEndpoint->endpoint,
				boost::bind(&UdpSocket::ReceiveFromHandler, this, boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred));
	}
}

void UdpSocket::Close()
{
	closed = true;
	if (socket)
	{
		socket->socket.close();
	}
	LOG_DEBUG("Close " << ToString());
}

std::string UdpSocket::ToString() const
{
	return boost::str(boost::format("[Port:%1%]") % port);
}

void UdpSocket::ReceiveFromHandler(const boost::system::error_code& error, std::size_t bytes_transferred)
{
	if (!peerEndpoint || !socket || closed)
	{
		LOG_DEBUG("Socket already closed. Skipping...");
		return;
	}

	if (!error)
	{
		Address peer(peerEndpoint->endpoint.address().to_string(), peerEndpoint->endpoint.port());

		LOG_INFO("ReceiveFromHandler: Receive on socket=" << ToString() << " from=" << peer.ToString() << " count="
				<< bytes_transferred << " bytes=" << nlib::detail::BytesToString((boost::uint8_t*) readBuffer,
				bytes_transferred));

		try
		{
			if (ReceivedFrom)
				ReceivedFrom(peer, (boost::uint8_t*) readBuffer, bytes_transferred);
			else
				LOG_WARN("No observer!");
		}
		catch (std::exception& ex)
		{
			LOG_ERROR("HandleRead: Socket=" << ToString() << " failed notifying ReceiveBytes! error=" << ex.what());
		}
		catch (...)
		{
			LOG_ERROR("HandleRead: Socket=" << ToString() << " failed notifying ReceiveBytes! unknow error.");
		}

		if (!closed)
		{
			Listen(); //back to listen
		}
	}
	else if (error != boost::asio::error::operation_aborted)
	{
		LOG_WARN("ReceiveFromHandler: Socket=" << ToString() << "failed to received! error=" << error);
	}
	else // exit requested
	{
		LOG_INFO("ReceiveFromHandler: Socket=" << ToString() << " exit requested!");
	}

}



/*--------------------------------------
 *--------------TcpSocket--------------*/

TcpSocket::TcpSocket(detail::RawTcpSocket* socket_)
: socket(socket_)
{
	port = 0;
	FireConnected(false);
}

TcpSocket::~TcpSocket()
{
	if (isConnected)
	{
		Close();
	}
}

void TcpSocket::AsyncConnect()
{
	boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(host), port);
	socket->socket.async_connect(endpoint, boost::bind(&TcpSocket::HandleConnect, this, boost::asio::placeholders::error));
}

void TcpSocket::SyncSend(const boost::uint8_t* bytes, std::size_t count)
{
	LOG_DEBUG("Sending " << count << " bytes to socket...");
	boost::asio::write(socket->socket, boost::asio::buffer((const char*)bytes, count));
}

void TcpSocket::Flush()
{
	//TODO [nicu.dascalu] - implement this
}

void TcpSocket::HandleConnect(const boost::system::error_code& error)
{
	if (!error)
	{
		LOG_INFO("HandleConnect: connection succeeded.");
		FireConnected(true);
		AsyncRead();
	}
	else
	{
		LOG_INFO("HandleConnect: connection failed! error_code=" << error);
		FireConnected(false);
	}
}

void TcpSocket::Close()
{
	socket->socket.close(); // should fire closed automatically from HandleRead...
}

void TcpSocket::AsyncRead()
{
	boost::asio::async_read(socket->socket, boost::asio::buffer(readBuffer, sizeof(readBuffer)),
	boost::asio::transfer_at_least(1), boost::bind(&TcpSocket::HandleRead, this, boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}

void TcpSocket::HandleRead(const boost::system::error_code& error, std::size_t bytes_transferred)
{
	if (!error)
	{
		LOG_DEBUG("HandleRead: received " << bytes_transferred << " bytes.");
		try
		{
			Received((boost::uint8_t*)readBuffer, bytes_transferred);
		}
		catch (std::exception& ex)
		{
			LOG_ERROR("HandleRead: failed notifying ReceiveBytes! error=" << ex.what());
		}
		catch (...)
		{
			LOG_ERROR("HandleRead: failed notifying ReceiveBytes! unknow error.");
		}

		AsyncRead();
	}
	else if (error != boost::asio::error::operation_aborted)
	{
		LOG_ERROR("HandleRead: connection lost! error=" << error);
		FireConnected(false);
	}
	else // exit requested

	{
		LOG_INFO("HandleRead: exit requested!");
	}
}

void TcpSocket::FireConnected(bool status)
{
	try
	{
		isConnected = status;

		if (isConnected)
		{
			toString = boost::str(boost::format("[Local=%1%:%2%, Remote=%3%:%4%]")
			% socket->socket.local_endpoint().address().to_string() % socket->socket.local_endpoint().port()
			% socket->socket.remote_endpoint().address().to_string() % socket->socket.remote_endpoint().port());
		}
		else
		{
			toString = "[Disconnected]";
		}

		if (Connected)
		Connected(status);
	}
	catch (std::exception& ex)
	{
		LOG_ERROR("FireConnect: failed notifying Connected! error=" << ex.what());
	}
	catch (...)
	{
		LOG_ERROR("FireConnect: failed notifying Connected! unknown error!");
	}
}
void TcpSocket::Host(const std::string& host_)
{
	host = host_;
}

std::string TcpSocket::Host()
{
	return host;
}

void TcpSocket::Port(int port_)
{
	port = port_;
}

int TcpSocket::Port()
{
	return port;
}


TcpAcceptor::TcpAcceptor(detail::RawTcpAcceptor* acceptor_)
: acceptor(acceptor_)
{
	port = 0;
	isListening = false;
}
TcpAcceptor::~TcpAcceptor()
{
	Close();
}

void TcpAcceptor::Listen()
{
	boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);
	acceptor->acceptor.open(endpoint.protocol());
	acceptor->acceptor.bind(endpoint);
	acceptor->acceptor.listen();

	AsyncAccept();
}

void TcpAcceptor::Close()
{
	if (isListening)
	{
		acceptor->acceptor.close(); // should set isListening to false in HandleAccept
	}
}

bool TcpAcceptor::IsListening() const
{
	return isListening;
}

void TcpAcceptor::AsyncAccept()
{
	newSocket.reset(new TcpSocket(new detail::RawTcpSocket(acceptor->service)));
	acceptor->acceptor.async_accept(newSocket->socket->socket, boost::bind(&TcpAcceptor::HandleAccept, this,
			boost::asio::placeholders::error));

}

void TcpAcceptor::HandleAccept(const boost::system::error_code& error)
{
	if (!error)
	{
		LOG_DEBUG("HandleAccept: receive connection from: " << newSocket->ToString());
		try
		{
			NewConnection(newSocket);
		}
		catch (std::exception& ex)
		{
			LOG_ERROR("HandleAccept: failed notifying NewConnection! error=" << ex.what());
		}
		catch (...)
		{
			LOG_ERROR("HandleAccept: failed notifying NewConnection! unknow error.");
		}

		AsyncAccept();
	}
	else if (error != boost::asio::error::operation_aborted)
	{
		LOG_ERROR("HandleAccept: new connection failed! error=" << error);
		AsyncAccept();
	}
	else // exit requested

	{
		isListening = false;
		LOG_INFO("HandleAccept: exit requested!");
	}
}

void TcpAcceptor::Port(int port_)
{
	port = port_;
}

/*-----------------------------------------------
 *--------------------Timer----------------------*/
Timer::Timer(detail::RawDeadlineTimer* timer_) :
	timer(timer_)
{
	started = false;
}

Timer::~Timer()
{
	Stop();
}

void Timer::Start(int interval)
{
	if (started)
		return; //already started

	LOG_DEBUG("Starting timer...");
	started = true;

	timer->timer.expires_from_now(boost::posix_time::milliseconds(interval));
	timer->timer.async_wait(boost::bind(&Timer::ElapsedHandler, this, interval));
}

void Timer::Stop()
{
	if (started)
	{
		LOG_DEBUG("Stopping timer...");
		started = false;
		timer->timer.cancel();
	}
}

void Timer::ElapsedHandler(int interval)
{
	if (started)
	{
		// call here in case the Elapsed takes a long time to process.
		timer->timer.expires_from_now(boost::posix_time::milliseconds(interval));
		try
		{
			if (Elapsed)
				Elapsed(interval);
		}
		catch (std::exception& ex)
		{
			LOG_ERROR("Fire elapse failed! error=" << ex.what());
		}
		catch (...)
		{
			LOG_ERROR("Fire elapse failed! unknown error.");
		}
		if (started)
		{
			timer->timer.async_wait(boost::bind(&Timer::ElapsedHandler, this, interval));
		}
	}
}


} // namespace socket
} // namespace nlib
