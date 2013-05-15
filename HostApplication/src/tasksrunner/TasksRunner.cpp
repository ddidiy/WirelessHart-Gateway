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

#include <WHartHost/tasksrunner/TasksRunner.h>


#include <Shared/Utils.h>
#include <Shared/AnPaths.h>
#include <Shared/DurationWatcher.h>

#include <boost/bind.hpp> //for binding to function

namespace hart7 {
namespace hostapp {


//register
void TasksRunner::RegisterPeriodicTask(const Loki::Function<void(int)>& task)
{
	periodicTasks.push_back(task);
}

void TasksRunner::ClearPeriodicTasks()
{
	periodicTasks.clear();
}

//perform
void TasksRunner::RegisterCreateTimer(const Loki::Function<nlib::timer::DeadlineTimer::Ptr(int)>& timer)
{
	Timer = timer;
	tasksTimer = Timer(tasksPeriod);
	tasksTimer->Elapsed = boost::bind(&hostapp::TasksRunner::Run, this, _1);
}

void TasksRunner::Run(int periodTime/*ms*/)
{
    LOG_DEBUG_APP("[TaskRunner]: start running tasks... with time=" << periodTime);
	CDurationWatcher oDurationWatcher;
	int i=0;
	double dMin1, dMin5, dMin15, dTotal;
	if (GetProcessLoad (dMin1, dMin5, dMin15, dTotal, GET_PROCESS_LOAD_AT_ONE_MIN))
	{	char szTmp[2048];
		sprintf(szTmp, "ProcessLoad: 1/5/15 min: %.2f/%.2f/%.2f total: %.2f", dMin1, dMin5, dMin15, dTotal );
		LOG_INFO_APP("" << szTmp);
	}
	WATCH_DURATION(oDurationWatcher, 5000, 5000);
	for (PeriodicTasksList::iterator it = periodicTasks.begin(); it != periodicTasks.end(); ++it)
	{

		if(stopTasks)
		{
			LOG_INFO_APP("[TaskRunner]: no more tasks running!");
			return;
		}

		try
		{
			(*it)(periodTime);
			if(WATCH_DURATION_DEF(oDurationWatcher))
			{
				LOG_INFO_APP("Task index in list=" << i);	
			}
			i++;
		}
		catch(std::exception& ex)
		{
			LOG_ERROR_APP("[TaskRunner]: Run automatic task failed. error=" << ex.what());
		}
		catch(...)
		{
			LOG_ERROR_APP("[TaskRunner]: Run automatic task failed. unknown error!");
		}
	}

#ifdef HW_VR900
		TouchPidFile(NIVIS_TMP"MonitorHost.pid");
#endif

	LOG_DEBUG_APP("[TaskRunner]: stop running tasks.");
}

void TasksRunner::SetBurstCounterTaskPeriod(int period)
{
	if (Timer)
	{
		burstsCounterTimer = Timer(period);
		burstsCounterTimer->Elapsed = boost::bind(&hostapp::TasksRunner::RunBurstCounterTask, this, _1);
		return;
	}
	LOG_ERROR_APP("[TaskRunner]: no timer function for burst_counters task");
}

void TasksRunner::RegisterBurstCounterTask(const Loki::Function<void(int)>& task)
{
	PeriodicBurstCounterTask = task;
}

void TasksRunner::RunBurstCounterTask(int periodTime/*ms*/)
{
    LOG_DEBUG_APP("[TaskRunner]: start burst_counter task... with time=" << periodTime);
	CDurationWatcher oDurationWatcher;

	double dMin1, dMin5, dMin15, dTotal;
	if (GetProcessLoad (dMin1, dMin5, dMin15, dTotal, GET_PROCESS_LOAD_AT_ONE_MIN))
	{	char szTmp[2048];
		sprintf(szTmp, "ProcessLoad: 1/5/15 min: %.2f/%.2f/%.2f total: %.2f", dMin1, dMin5, dMin15, dTotal );
		LOG_INFO_APP("" << szTmp);
	}
	WATCH_DURATION(oDurationWatcher,5000,5000);
	try
	{
		if (PeriodicBurstCounterTask)
			PeriodicBurstCounterTask(periodTime);
		else
			LOG_WARN_APP("[TaskRunner]: no burst_counter task is registered");

		if(WATCH_DURATION_DEF(oDurationWatcher))
		{
		    LOG_DEBUG_APP("Task burst_counter.");
		}
	}
	catch(std::exception& ex)
	{
		LOG_ERROR_APP("[TaskRunner]: burst_counter task failed. error=" << ex.what());
	}
	catch(...)
	{
		LOG_ERROR_APP("[TaskRunner]: burst_counter task failed. unknown error!");
	}
	
	LOG_DEBUG_APP("[TaskRunner]: burst_counter task has finished.");
}


//stop_process
void TasksRunner::StopTasks()
{
	stopTasks = true;
}


} //namespace hostapp
} //namspace hart7
