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

#ifndef TASKSRUNNER_H_
#define TASKSRUNNER_H_

#include <nlib/log.h>

#include <list>

//#include <boost/function.hpp>
#include <loki/Function.h>

#include <nlib/timer/Timer.h>


namespace hart7 {
namespace hostapp {

/*
 * 
 */
class TasksRunner
{
	LOG_DEF("hart7.hostapp.TasksRunner");

//init
public:
	TasksRunner(int period) :
	    tasksPeriod(period),
	    stopTasks(false)
	{ }

//register
public:
	void RegisterPeriodicTask(const Loki::Function<void(int)>& task);
	void ClearPeriodicTasks();

//perform
public:
	void RegisterCreateTimer(const Loki::Function<nlib::timer::DeadlineTimer::Ptr(int)>& timer);
	void Run(int periodTime/*ms*/);

//publishers
public:
	void SetBurstCounterTaskPeriod(int period/**/);
	void RegisterBurstCounterTask(const Loki::Function<void(int)>& task);

private:
	void RunBurstCounterTask(int periodTime/*ms*/);

//stop_process
public:
	void StopTasks();

private:
	typedef std::list<Loki::Function<void(int)> > PeriodicTasksList;
	PeriodicTasksList periodicTasks;
	nlib::timer::DeadlineTimer::Ptr	tasksTimer;

	int tasksPeriod;
	bool stopTasks;

	Loki::Function<void(int)> PeriodicBurstCounterTask;
	nlib::timer::DeadlineTimer::Ptr	burstsCounterTimer;

	Loki::Function<nlib::timer::DeadlineTimer::Ptr(int)> Timer;
};

} //namespace hostapp
} //namespace hart7
#endif 


