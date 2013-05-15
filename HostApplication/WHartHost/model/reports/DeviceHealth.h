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

#ifndef __DEVICEHEALTH_H_
#define __DEVICEHEALTH_H_


#include <Shared/MicroSec.h>


#include <WHartHost/model/MAC.h>


#include <vector>
#include <list>
#include <iostream>


namespace hart7 {
namespace hostapp {


struct DeviceHealth
{

	DeviceHealth(){}
	DeviceHealth( int p_nDeviceId,
		const MAC &p_oDeviceMac,
		int p_nPowerStatus,
		int p_nGenerated,
		int p_nTerminated,
		int p_nDllFailures,
		int p_nNlFailures,
		int p_nCrcErrors,
		int p_nNonceLost) :
		m_nDeviceId(p_nDeviceId),
		m_oDeviceMac(p_oDeviceMac),
		m_nPowerStatus(p_nPowerStatus),
		m_nGenerated(p_nGenerated),
		m_nTerminated(p_nTerminated),
		m_nDllFailures(p_nDllFailures),
		m_nNlFailures(p_nNlFailures),
		m_nCrcErrors(p_nCrcErrors),
		m_nNonceLost(p_nNonceLost)
	{time = CMicroSec().GetElapsedTimeStr();}

	int m_nDeviceId;
	MAC	m_oDeviceMac;
	int m_nPowerStatus;
	int m_nGenerated;
	int m_nTerminated;
	int m_nDllFailures;
	int m_nNlFailures;
	int m_nCrcErrors;
	int m_nNonceLost;

	std::string time;
};

std::ostream& operator<< (std::ostream& p_rStream, const DeviceHealth& p_rDevHealth);


typedef std::vector<DeviceHealth> DeviceHealthList;

typedef std::list<DeviceHealth> DevicesHealthListT;
}
}

#endif
