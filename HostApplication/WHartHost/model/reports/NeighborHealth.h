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

#ifndef __NEIGHBORHEALTH_H_
#define __NEIGHBORHEALTH_H_


#include <Shared/MicroSec.h>


#include <list>
#include <vector>

#include <WHartHost/model/MAC.h>


namespace hart7 {
namespace hostapp {

struct NeighborHealth
{
	NeighborHealth(){}
	const static int	g_nNoClockSource = 0;

	NeighborHealth(	int p_nPeerID,
		int p_nClockSource,
		int p_nRSL,
		int p_nTransmissions,
		int p_nFailedTransmissions,
		int p_nReceptions) :
	m_nPeerID(p_nPeerID),
	m_nClockSource(p_nClockSource),
	m_nRSL(p_nRSL),
	m_nTransmissions(p_nTransmissions),
	m_nFailedTransmissions(p_nFailedTransmissions),
	m_nReceptions(p_nReceptions)
	{time = CMicroSec().GetElapsedTimeStr();}
	
	int m_nPeerID;
	int m_nClockSource;
	int m_nRSL;
	int m_nTransmissions;
	int m_nFailedTransmissions;
	int m_nReceptions;

	std::string time;
};

struct DeviceNeighborsHealth
{
	DeviceNeighborsHealth(int p_nDeviceID, const MAC& p_rMac):
		m_nDeviceID(p_nDeviceID),
		m_oMac(p_rMac)
	{}

	DeviceNeighborsHealth(int p_nDeviceID):
	m_nDeviceID(p_nDeviceID)
	{}

	int m_nDeviceID;
	MAC m_oMac;
	std::list<NeighborHealth> m_oNeighborsList;
};

std::ostream& operator<< (std::ostream& p_rStream, const NeighborHealth& p_rNeighbHealth);

typedef std::vector<DeviceNeighborsHealth> DevicesNeighborsHealth;

typedef std::list<DeviceNeighborsHealth> DevicesNeighborsHealthListT;


}
}

#endif //__NEIGHBORHEALTH_H_
