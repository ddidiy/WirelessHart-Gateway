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

#ifndef __SUPERFRAME_H_
#define __SUPERFRAME_H_


#include <Shared/MicroSec.h>

#include <WHartStack/WHartTypes.h>

#include <list>
#include <iostream>


namespace hart7 {
namespace hostapp {

struct Superframe
{
	Superframe( int p_nSuperframeId, 
		int p_nDeviceId,
		int p_nNumberOfTimeSlots,
		int p_nActive,
		int p_nHandheldSuperframe) :
		m_nSuperframeId(p_nSuperframeId),
		m_nDeviceId(p_nDeviceId),
		m_nNumberOfTimeSlots(p_nNumberOfTimeSlots),
		m_nActive(p_nActive),
		m_nHandheldSuperframe(p_nHandheldSuperframe)
		{m_toDelete = false;
		time = CMicroSec().GetElapsedTimeStr();}

	Superframe( int p_nSuperframeId, 
				int p_nDeviceId) :
			m_nSuperframeId(p_nSuperframeId),
			m_nDeviceId(p_nDeviceId)
		{m_toDelete = true;
		time = CMicroSec().GetElapsedTimeStr();}


	//
	bool m_toDelete;  //used in caching process

	uint8_t m_nSuperframeId;
	int m_nDeviceId;
	int m_nNumberOfTimeSlots;
	int m_nActive;
	int m_nHandheldSuperframe;

	std::string time;
};

typedef std::list<Superframe> SuperframesListT;

std::ostream& operator<< (std::ostream& p_rStream, const Superframe& p_rSuperframe);

}
}

#endif //__SUPERFRAME_H_
