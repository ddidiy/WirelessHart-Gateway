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

#ifndef __SERVICE_H_
#define __SERVICE_H_


#include <Shared/MicroSec.h>


#include <WHartStack/WHartTypes.h>

#include <list>
#include <iostream>


namespace hart7 {
namespace hostapp {

struct Service
{
	Service( int		p_nServiceId,
			 int 		p_nDeviceId,
			 int 		p_nPeerId,
			 uint8_t	p_nApplicationDomain,
			 uint8_t 	p_nSourceFlag,
			 uint8_t 	p_nSinkFlag,
			 uint8_t 	p_nIntermittentFlag,
			 int 		p_nPeriod,
			 uint8_t 	p_nRouteId) :
		m_nServiceId(p_nServiceId),
		m_nDeviceId(p_nDeviceId),
		m_nPeerId(p_nPeerId),
		m_nApplicationDomain(p_nApplicationDomain),
		m_nSourceFlag(p_nSourceFlag),
		m_nSinkFlag(p_nSinkFlag),
		m_nIntermittentFlag(p_nIntermittentFlag),
		m_nPeriod(p_nPeriod),
		m_nRouteId(p_nRouteId)
		{m_toDelete = false;
		time = CMicroSec().GetElapsedTimeStr();}

	Service( int		p_nServiceId,
			int 		p_nDeviceId) :
			m_nServiceId(p_nServiceId),
			m_nDeviceId(p_nDeviceId)
		{m_toDelete = true;
		time = CMicroSec().GetElapsedTimeStr();}

	//
	bool m_toDelete;  //used in caching process
	

	int m_nServiceId;
	int m_nDeviceId;
	int m_nPeerId;
	uint8_t m_nApplicationDomain;
	uint8_t m_nSourceFlag;
	uint8_t m_nSinkFlag;
	uint8_t m_nIntermittentFlag;
	int m_nPeriod;
	uint8_t m_nRouteId;

	std::string time;
};

typedef std::list<Service> ServicesListT;

std::ostream& operator<< (std::ostream& p_rStream, const Service& p_rSignal);


}
}

#endif //__SERVICE_H_
