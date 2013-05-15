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


#ifndef APPSETNOTIFICATIONBITMASK_H_
#define APPSETNOTIFICATIONBITMASK_H_


#include <WHartHost/applicationcmds/AbstractAppCommand.h>
#include <WHartHost/model/MAC.h>


namespace hart7 {
namespace hostapp {

class AppSetNotificationBitMask: public AbstractAppCommand
{

public:
	AppSetNotificationBitMask(const MAC& p_rDeviceMac, unsigned short notifBitMask);
	virtual bool Accept(IAppCommandVisitor& visitor);

protected:
	void DumpToStream(std::ostream& p_rStream) const;

public:
	MAC				m_devMac;
	unsigned short	m_notificationBitMask;

};


}
}


#endif

