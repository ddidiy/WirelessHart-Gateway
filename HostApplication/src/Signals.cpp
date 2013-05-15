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


#include <WHartHost/Signals.h>

#ifndef _WINDOWS
#include <signal.h>
#endif

using namespace std;

namespace hart7 {
namespace util {


Signals::HandlersMMap Signals::registeredHandles;

Signals::Signals()
{
}

Signals::~Signals()
{
}

//wait
void Signals::Wait(int signal)
{
#ifndef _WINDOWS
	sigset_t signalSet;

	sigemptyset(&signalSet);
	sigaddset(&signalSet, signal);

	LOG_DEBUG_APP("[Signal] waiting for signal=" << signal << "...");

	int receivedSignal = 0;
	sigwait(&signalSet, &receivedSignal);

	LOG_INFO_APP("[Signal] received signal=" << receivedSignal);
#endif
}

void Signals::Wait(SignalsList signals)
{
#ifndef _WINDOWS
	sigset_t signalsSet;
	sigemptyset(&signalsSet);

	for (SignalsList::iterator itSignal = signals.begin(); itSignal != signals.end(); itSignal++)
	{
		LOG_DEBUG_APP("[Signal] waiting for signal=" << *itSignal << "...");
		sigaddset(&signalsSet, *itSignal);
	}

	int receivedSignal = 0;
	sigwait(&signalsSet, &receivedSignal);

	LOG_INFO_APP("[Signal] received signal=" << receivedSignal);
#endif
}

//ignore
void Signals::Ignore(int signal)
{
#ifndef _WINDOWS
#ifndef MIPS
	sigignore(signal);
	LOG_INFO_APP("[Signal] signal=" << signal << " will be ignored!!!");
#endif
#endif
}

//register
//void Signals::RegisterSignalHandler(int signal, const boost::function0<void>& handler)
void Signals::RegisterSignalHandler(int signal, Loki::Function<void(void)> handler)
{
#ifndef _WINDOWS
	struct sigaction act;
	act.sa_handler = signal_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0; //SA_SIGINFO
	if(sigaction(signal, &act, 0) ==0)
	{
		registeredHandles.insert(pair<int, Loki::Function<void(void)> >(signal, handler));
	}
	else
		LOG_ERROR_APP("[Signal] cannot register handler for signal: " << signal);		
#endif
}

void Signals::signal_handler(int signum)
{
	pair<HandlersMMap::const_iterator,HandlersMMap::const_iterator> ret = registeredHandles.equal_range(signum);
	for (HandlersMMap::const_iterator it = ret.first; it != ret.second; ++it)
	{
		it->second();
	}
}


} //namespace util
} //namespace hart7
