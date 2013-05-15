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

#ifndef SIGNALS_H_
#define SIGNALS_H_

#include <nlib/log.h>

#include <loki/Function.h>

#include <map>
#include <vector>

#include <signal.h> //used to define SIG_ identifiers


namespace hart7 {
namespace util {

typedef std::vector<int> SignalsList;

class Signals
{

public:
	Signals();
	virtual ~Signals();

//wait
public:
	void Wait(int signal);
	void Wait(SignalsList signals);	
	
//ignore
public:
	void Ignore(int signal);	
	
//register
public:
	void RegisterSignalHandler(int signal, Loki::Function<void(void)> handler);
private:
	static void signal_handler(int signum);

//
private:
	typedef std::multimap<int, Loki::Function<void(void)> > HandlersMMap;
	static HandlersMMap registeredHandles;
};

}
}

#endif /*SIGNALS_H_*/
