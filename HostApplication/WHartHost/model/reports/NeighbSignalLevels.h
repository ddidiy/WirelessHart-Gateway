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



#ifndef __NEIGHBSIGNALLEVELS_H_
#define __NEIGHBSIGNALLEVELS_H_


#include <list>
#include <vector>

#include <WHartHost/model/MAC.h>
#include <iostream>


namespace hart7 {
namespace hostapp {


struct NeighbourSignalLevel
{
	NeighbourSignalLevel(int neighbDevID_, int signalLevel_):neighDevID(neighbDevID_), signalLevel(signalLevel_){}
	int neighDevID;
	int signalLevel;
};

struct NeighbourSignalLevels
{
	NeighbourSignalLevels(int deviceID_, const std::list<NeighbourSignalLevel>&	neighbors_):
			deviceID(deviceID_),neighbors(neighbors_){}
	NeighbourSignalLevels(int deviceID_, const MAC &mac_):deviceID(deviceID_),devMac(mac_){}
	int deviceID;
	MAC devMac;
	std::list<NeighbourSignalLevel>	neighbors;
};

std::ostream& operator<< (std::ostream& p_rStream, const NeighbourSignalLevel& p_rNeighbSignalLev);

typedef std::list<NeighbourSignalLevels> NeighbourSignalLevelsListT;

}
}

#endif
