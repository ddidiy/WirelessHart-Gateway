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

#ifndef NOTIFICATIONDEVINFO_H_
#define NOTIFICATIONDEVINFO_H_


#include <WHartHost/model/Device.h>
#include <WHartHost/model/reports/NeighborHealth.h>
#include <WHartHost/model/reports/TopologyReport.h>


namespace hart7 {
namespace hostapp {

class NotificationDevInfo
{
public:
	NotificationDevInfo():DeviceType(Device::dtRoutingDeviceNode){}

public:

	struct Neighbour
	{
		NickName		DeviceNickName;
		NeighborHealth	Health;

	};
	typedef std::vector<Neighbour> NeighborsListT;

	MAC				DeviceMAC;
	NickName		DeviceNickName;
	boost::uint8_t	DeviceType;
	std::string		LongTag;
	unsigned short  notifBitMask;
	int				softwareRev;
	int				deviceCode;
	
	NeighborsListT NeighborsList;
	TopologyReport::Device::GraphsListT GraphsList;
};

}
}


#endif
