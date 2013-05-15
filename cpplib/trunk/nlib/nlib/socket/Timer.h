/*
 * Timer.h
 *
 *  Created on: Dec 4, 2008
 *      Author: ovidiu.rauca
 */

#ifndef SOCKET_TIMER_H_
#define SOCKET_TIMER_H_


#include <nlib/log.h>
#include <nlib/datetime.h>

#include <boost/system/error_code.hpp> //for error codes
#include <boost/smart_ptr.hpp> //for scoped and shared ptr
#include <boost/noncopyable.hpp>

//#include <boost/function.hpp> //for callback
#include <loki/Function.h>

namespace nlib {
namespace socket {

namespace detail {
struct RawDeadlineTimer;
}


/**
 * A simple timer wrapper over the Asio DeadlineTimer.
 */
class Timer: private boost::noncopyable
{
	LOG_DEF("nlib.socket.Timer");
public:
	typedef boost::shared_ptr<Timer> Ptr;

private:
	Timer(detail::RawDeadlineTimer* timer);

public:
	~Timer();

	/**
	 * Starts the timer. the callback @Elapsed will ve called with interval
	 * @interval in milliseconds.
	 */
	void Start(int interval);

	/**
	 * Stops the timer.
	 */
	void Stop();

	/**
	 * the callback
	 */
//	boost::function1<void, int> Elapsed;
	  typedef Loki::Function<void(int)> ElapsedCallback;
	  ElapsedCallback Elapsed;

private:
	void ElapsedHandler(int interval);

private:
	const boost::scoped_ptr<detail::RawDeadlineTimer> timer;
	/** protects the timer resource to be started more than once */
	bool started;

	friend class ServiceSocket;
};

} //namespace socket {
} //namespace nlib

#endif /* SOCKET_TIMER_H_ */
