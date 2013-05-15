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


#include <WHartHost/model/reports/DeviceHealth.h>



namespace hart7 {
namespace hostapp {


std::ostream& operator<< (std::ostream& p_rStream, const DeviceHealth& p_rDevHealth)
{
	p_rStream << "deviceID=" << p_rDevHealth.m_nDeviceId << " powerStatus=" << p_rDevHealth.m_nPowerStatus 
		<< " generated=" << p_rDevHealth.m_nGenerated << " terminated=" << p_rDevHealth.m_nTerminated
		<< " dllFailures=" << p_rDevHealth.m_nDllFailures << " nlFailures=" << p_rDevHealth.m_nNlFailures
		<< " crcErrors=" << p_rDevHealth.m_nCrcErrors << " nonceLost=" << p_rDevHealth.m_nNonceLost;

	return p_rStream;
}




}
}

