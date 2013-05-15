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

#ifndef __DEVICESCHEDULELINK_H_
#define __DEVICESCHEDULELINK_H_


#include <Shared/MicroSec.h>


#include <WHartHost/model/MAC.h>
#include <list>
#include <iostream>


namespace hart7 {
namespace hostapp {


struct DeviceScheduleLink
{
	DeviceScheduleLink() { memset(this, 0, sizeof(*this)); }

	DeviceScheduleLink( int p_nSuperframeId,
		int p_nPeerId,
		int p_nSlotIndex,
		int p_nChannelOffset,
		int p_nTransmit,
		int p_nReceive,
		int p_nShared,
		int p_nLinkType) :
	m_nSuperframeId(p_nSuperframeId),
	m_nPeerId(p_nPeerId),
	m_nSlotIndex(p_nSlotIndex),
	m_nChannelOffset(p_nChannelOffset),
	m_nTransmit(p_nTransmit),
	m_nReceive(p_nReceive),
	m_nShared(p_nShared),
	m_nLinkType(p_nLinkType)
	{m_toDelete = false;
	time = CMicroSec().GetElapsedTimeStr();
	}

	DeviceScheduleLink( int p_nSuperframeId,
		int p_nPeerId,
		int p_nSlotIndex) :
		m_nSuperframeId(p_nSuperframeId),
		m_nPeerId(p_nPeerId),
		m_nSlotIndex(p_nSlotIndex)
	{m_toDelete = true;
	time = CMicroSec().GetElapsedTimeStr();
	}

	//
	bool m_toDelete;  //used in caching process


	int m_nSuperframeId;
	int m_nPeerId;
	int m_nSlotIndex;
	int m_nChannelOffset;
	int m_nTransmit;
	int m_nReceive;
	int m_nShared;
	int m_nLinkType;

	std::string time;
};

struct DevicesLinks
{

	DevicesLinks(int p_nDeviceId, std::list<DeviceScheduleLink>& linkList_):
		m_nDeviceId(p_nDeviceId),
		linkList(linkList_)
	{
	}

	DevicesLinks(int p_nDeviceId, DeviceScheduleLink link):m_nDeviceId(p_nDeviceId)
	{
		linkList.push_back(link);
	}

	DevicesLinks(int p_nDeviceId, MAC p_oDeviceMac, DeviceScheduleLink link):m_nDeviceId(p_nDeviceId)
	{
		m_oDeviceMac = p_oDeviceMac;
		linkList.push_back(link);
	}

	DevicesLinks(int p_nDeviceId, MAC p_oDeviceMac):m_nDeviceId(p_nDeviceId)
	{
		m_oDeviceMac = p_oDeviceMac;
	}

	int m_nDeviceId;
	MAC m_oDeviceMac;
	std::list<DeviceScheduleLink> linkList;
};

std::ostream& operator<< (std::ostream& p_rStream, const DeviceScheduleLink& p_rDevSchedLink);

typedef std::vector<DevicesLinks> DevicesScheduleLinks;
typedef std::list<DevicesLinks> DevicesScheduleLinksListT;

}
}

#endif //__DEVICESCHEDULELINK_H_
