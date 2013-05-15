/*
 * Timer.h
 *
 *  Created on: Feb 4, 2009
 *      Author: Andy
 */

#ifndef TIMER_H_
#define TIMER_H_

#include <boost/shared_ptr.hpp>
//#include <boost/function.hpp>
#include <loki/Function.h>

#include <nlib/socket/ServiceSocket.h>

namespace nlib {
namespace timer {


class DeadlineTimer
{
public:

	typedef boost::weak_ptr<DeadlineTimer> WeakPtr;
	typedef boost::shared_ptr<DeadlineTimer> Ptr;

//	typedef boost::function1<void, uint32_t> ElapsedHandler;
	typedef Loki::Function<void(uint32_t)> ElapsedHandler;

	ElapsedHandler Elapsed;

private:

	DeadlineTimer(uint32_t expireTime);
	void TimeElapsed(uint32_t time);

	int expireTime;
	int timerLeft;
	friend class TimerFactory;
};

class TimerFactory
{
public:
	typedef boost::shared_ptr<TimerFactory> Ptr;
	TimerFactory(nlib::socket::ServiceSocket::Ptr service, uint32_t minPeriod = 100);
	DeadlineTimer::Ptr CreateDeadlineTimer(uint32_t expireTime);

private:

	void TimerElapsed(uint32_t time);

	nlib::socket::ServiceSocket::Ptr service;
	nlib::socket::Timer::Ptr timer;

	typedef std::vector<DeadlineTimer::WeakPtr> TimersList;
	TimersList timers;

	TimersList toAddTimers;

	struct timeval lastTimeCalled;

};

}
}

#endif /* TIMER_H_ */
