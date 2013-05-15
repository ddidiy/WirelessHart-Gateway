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

#ifndef _WATCDOG_H_
#define _WATCDOG_H_

#include <stdexcept>
#define WTD "WTD:"


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
class CWatchdog {
public:
	class exception : public std::runtime_error
	{
	public:
		exception(const char*p) : std::runtime_error(p) { }
	};

	CWatchdog()
		: m_lPingInterval(0)
		{
		}
	~CWatchdog() {}

public:
	enum Type {
		WTD_MAX6371,
		WTD_INTERNAL,
		WTD_ALL=0xFF
	};

public:
	virtual int	Open(const void*param)=0;
	virtual int	Close()=0;
	virtual int	Ping()=0;
	const long	GetPingInterval() const { return m_lPingInterval ; }

protected:
	long	m_lPingInterval ;///< If no ping is done in m_lPingInterval seconds, the watchdog will reboot the device
};
#endif	// _WATCDOG_H_
