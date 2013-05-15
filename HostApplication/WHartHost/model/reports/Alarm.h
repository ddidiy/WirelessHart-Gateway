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


#ifndef __ALARM_H_
#define __ALARM_H_


#include <Shared/MicroSec.h>


#include <WHartStack/WHartTypes.h>

#include <list>
#include <iostream>


namespace hart7 {
namespace hostapp {

struct Alarm
{
	Alarm( int p_nDeviceId, int p_nAlarmType, int PeerID_GraphId) :
		m_nDeviceId(p_nDeviceId), m_nAlarmType(p_nAlarmType), 
			m_PeerID_GraphId(PeerID_GraphId)
	{ hasMIC = true;
	  time = CMicroSec().GetElapsedTimeStr();
	}

	Alarm( int p_nDeviceId, int p_nAlarmType, int PeerID_GraphId, int MIC) :
	m_nDeviceId(p_nDeviceId), m_nAlarmType(p_nAlarmType), 
		m_PeerID_GraphId(PeerID_GraphId), m_MIC(MIC)
	{hasMIC = false;
	time = CMicroSec().GetElapsedTimeStr();
	}

	bool hasMIC;
	int m_nDeviceId;
	int m_nAlarmType;
	int m_PeerID_GraphId;
	int m_MIC;

	std::string time;

};

std::ostream& operator<< (std::ostream& p_rStream, const Alarm& p_rAlarm);

typedef std::list<Alarm> AlarmsListT;

}
}

#endif //__ALARM_H_
