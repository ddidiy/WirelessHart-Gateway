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

#ifndef APPDISCOVERYBURSTCONFIGCMD_H_
#define APPDISCOVERYBURSTCONFIGCMD_H_


#include <WHartHost/applicationcmds/AbstractAppCommand.h>
#include <vector>

namespace hart7 {
namespace hostapp {

/**
 * @brief Represents a topology command from application perspective
 * Command Pattern
 */
class IAppCommandVisitor;
class AppDiscoveryBurstConfigCmd;

typedef boost::shared_ptr<AppDiscoveryBurstConfigCmd> AppDiscoveryBurstConfigCmdPtr;

class AppDiscoveryBurstConfigCmd: public AbstractAppCommand
{

public:
	virtual bool Accept(IAppCommandVisitor& visitor);

protected:
	void DumpToStream(std::ostream& p_rStream) const;

public:
	MAC            m_oDeviceMAC;
	PublisherInfo  m_oPublisherInfo;
    unsigned char  m_nCurrentBurst;
    unsigned short m_nCurrentCmdNo;
    double         m_nCurrentUpdatePeriod;
    double         m_nCurrentMaxUpdatePeriod;
    unsigned char  m_nTotalBurstCount;

public:
	WHartUniqueID GetDeviceAddress() { return m_oDeviceMAC.Address(); }

	AppDiscoveryBurstConfigCmd(MAC& device) :
	    m_oDeviceMAC(device),
	    m_oPublisherInfo(),
	    m_nCurrentBurst(0),
	    m_nCurrentCmdNo(0),
	    m_nCurrentUpdatePeriod(0),
	    m_nCurrentMaxUpdatePeriod(0),
	    m_nTotalBurstCount(0)
	{ }

};

}
}

#endif
