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

#ifndef DEVICEREADING_H_
#define DEVICEREADING_H_

#include <deque>
#include <sys/time.h> 

namespace hart7 {
namespace hostapp {

class DeviceReading
{
public:
	enum ReadingType
	{
		ReadValue = 0,
		BurstValue = 1,
	};

public:
	int				m_deviceID;
	int				m_channelNo;
	struct timeval	m_tv;
	unsigned char	m_valueStatus; 
	float			m_value;
	int 			m_nCommandID;
	bool			m_hasStatus;
	ReadingType		m_readingType;
};

class DeviceReadingKey
{
public:
	int				m_deviceID;
	int				m_channelNo;

DeviceReadingKey(int p_deviceID, int p_channelNo)
	: m_deviceID(p_deviceID), m_channelNo(p_channelNo)
	{ }

bool operator == (const DeviceReadingKey& other)  const
	{
		return (m_deviceID == other.m_deviceID) && (m_channelNo == other.m_channelNo);
	}

bool operator < (const DeviceReadingKey& other)  const
	{
		return (m_deviceID < other.m_deviceID) || (m_deviceID == other.m_deviceID && m_channelNo < other.m_channelNo);
	}
};

typedef std::map<DeviceReadingKey, DeviceReading> DeviceReadingsMapT;

} //namespace hostapp
} //namespace hart7


#endif /*DEVICEREADING_H_*/
