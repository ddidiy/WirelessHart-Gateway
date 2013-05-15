/*
 * timer.cpp
 *
 *  Created on: Feb 4, 2009
 *      Author: Andy
 */

#include <nlib/timer/Timer.h>
#include <boost/bind.hpp>

namespace nlib {
namespace timer {


DeadlineTimer::DeadlineTimer(uint32_t expireTime_)
	: expireTime(expireTime_)
{
	timerLeft = expireTime;
}


void DeadlineTimer::TimeElapsed(uint32_t time)
{
	timerLeft -= time;

	while (timerLeft <= 0)
	{
		if (Elapsed)
			Elapsed(expireTime);

		timerLeft += expireTime;
	}
}

TimerFactory::TimerFactory(nlib::socket::ServiceSocket::Ptr service_, uint32_t minPeriod)
	: service(service_)
{

	gettimeofday( &lastTimeCalled, NULL) ;
	timer = service->CreateDeadLineTimer();
	timer->Elapsed = boost::bind(&TimerFactory::TimerElapsed, this, _1);
	timer->Start(minPeriod);
}

DeadlineTimer::Ptr TimerFactory::CreateDeadlineTimer(uint32_t expireTime)
{
	DeadlineTimer::Ptr timer(new DeadlineTimer(expireTime));

	DeadlineTimer::WeakPtr weakTimer(timer);
	toAddTimers.push_back(weakTimer);

	return timer;
}

struct EmptyPointerEraser
{
	bool operator()(DeadlineTimer::WeakPtr& ptr)
	{
		DeadlineTimer::Ptr strongPtr = ptr.lock();
		return !strongPtr;
	}
};

void TimerFactory::TimerElapsed(uint32_t time)
{
	struct timeval tv;
	gettimeofday( &tv, NULL) ;

	if ((lastTimeCalled.tv_sec > tv.tv_sec) || (lastTimeCalled.tv_sec == tv.tv_sec && lastTimeCalled.tv_usec > tv.tv_usec))
	{
		// we are somehow in the past, might happen with ntp or user might change it
		lastTimeCalled = tv;
		return;
	}

	int deltaT = (tv.tv_sec - lastTimeCalled.tv_sec) * 1000 + (tv.tv_usec - lastTimeCalled.tv_usec ) / 1000;

	lastTimeCalled = tv;

	while (deltaT > 0)
	{
		if (deltaT < time)
		{
			time = deltaT;
		}

		for (TimersList::iterator it = timers.begin(); it != timers.end(); ++it)
		{
	//		try
	//		{
				DeadlineTimer::Ptr strongTimer = it->lock();
				if (strongTimer)
				{
					strongTimer->TimeElapsed(time);
				}
	//		}
	//		catch (std::exception& )
	//		{
	//			//TODO log
	//		}
		}

		deltaT -= time;
}

	//erase empty pointers
	timers.erase(std::remove_if(timers.begin(), timers.end(), EmptyPointerEraser()),
			timers.end());

	//add new timers
	for (TimersList::iterator it = toAddTimers.begin(); it != toAddTimers.end(); ++it)
	{
		timers.push_back(*it);
	}

	toAddTimers.clear();
}

}
}
