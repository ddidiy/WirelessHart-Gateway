#ifndef NLIB_TCP_SOCKET_H
#define NLIB_TCP_SOCKET_H

#include <nlib/log.h>

#include <boost/smart_ptr.hpp> //for scoped and shared ptr
#include <boost/system/error_code.hpp> //for error codes
#include <boost/cstdint.hpp>
#include <boost/noncopyable.hpp>

//#include <boost/function.hpp> //for callback
#include <loki/Function.h>

namespace nlib {
namespace socket {

namespace detail {
struct RawTcpSocket;
struct RawTcpAcceptor;
}


class TcpAcceptor;

class TcpSocket : private boost::noncopyable
{
	LOG_DEF("nlib.socket.TcpSocket");
public:
	typedef boost::shared_ptr<TcpSocket> Ptr;

private:
	TcpSocket(detail::RawTcpSocket* socket);

public:
	~TcpSocket();

	const std::string& ToString() const
	{
		return toString;
	}

	void Host(const std::string& host_);
	std::string Host();
	void Port(int port_);
	int Port();

	void AsyncConnect();
	//boost::function1<void, bool> Connected;
	typedef Loki::Function<void(bool)> ConnectedCallback;
	ConnectedCallback Connected;


	bool IsConnected() const
	{
		return isConnected;
	}

	void SyncSend(const boost::uint8_t* bytes, std::size_t count);
	void Flush();

	//boost::function2<void, const boost::uint8_t*, std::size_t> Received;
	typedef Loki::Function<void(const boost::uint8_t*, std::size_t)> ReceivedCallback;
	ReceivedCallback Received;

	void Close();

private:
	void HandleConnect(const boost::system::error_code& error);

	void AsyncRead();
	void HandleRead(const boost::system::error_code& error, std::size_t bytes_transferred);

	void FireConnected(bool connectedStatus);

	std::string host;
	int port;

	const boost::scoped_ptr<detail::RawTcpSocket> socket;

	char readBuffer[64 * 1024];
	bool isConnected;

	std::string toString;

	friend class TcpAcceptor;
	friend class ServiceSocket;
};

/**
 *
 */
class TcpAcceptor : private boost::noncopyable
{
	LOG_DEF("nlib.socket.TcpAcceptor")
	;

public:
	typedef boost::shared_ptr<TcpAcceptor> Ptr;

private:
	TcpAcceptor(detail::RawTcpAcceptor* acceptor);

public:
	~TcpAcceptor();

	void Port(int port);

	void Listen();
	void Close();
	bool IsListening() const;

	//boost::function1<void, TcpSocket::Ptr> NewConnection;
	typedef Loki::Function<void(TcpSocket::Ptr)> NewConnectionCallback;
	NewConnectionCallback NewConnection;

private:
	void AsyncAccept();
	void HandleAccept(const boost::system::error_code& error);

private:
	int port;
	bool isListening;
	const boost::scoped_ptr<detail::RawTcpAcceptor> acceptor;

	TcpSocket::Ptr newSocket;

	friend class ServiceSocket;
};

} // namespace socket
} // namespace nlib

#endif /*NLIB_TCP_SOCKET_H*/
