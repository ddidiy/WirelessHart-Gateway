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

#ifndef APPSETBURSTCONFIGURATIONCMD_H_
#define APPSETBURSTCONFIGURATIONCMD_H_


#include <string>
#include <list>

#include <WHartHost/model/Device.h>
#include <WHartHost/applicationcmds/AbstractAppCommand.h>


namespace hart7 {
namespace hostapp {

/**
 * @brief Represents a set burst configure command from application perspective
 * Command Pattern
 */
class IAppCommandVisitor;

class AppSetBurstConfigurationCmd: public AbstractAppCommand
{

public:
	enum SetBurstConfigurationState
	{
		SET_BURSTCONF_READ_CONFIG_STATE,
		SET_BURSTCONF_TURNOFF_BURSTMESSAGES_STATE,
		SET_BURSTCONF_GET_SUBDEVICEINDEX_STATE,
		SET_BURSTCONF_CONFIGURE_BURSTMESSAGES_STATE
	};

public:
	AppSetBurstConfigurationCmd(const std::string& p_strPubConfigFile, DevicePtr p_oDev);

public:
	virtual bool Accept(IAppCommandVisitor& visitor);

protected:
	void DumpToStream(std::ostream& p_rStream) const;



public:
	const std::string& GetPubConfigFileName();
	void SetState(SetBurstConfigurationState p_enumState);
	SetBurstConfigurationState GetState();
	DevicePtr GetDevice();

public:
	std::list<BurstMessage>& GetBurstMessagesToConfigure();
	void RegisterBurstMessageConfigurationStatus(BurstState p_enumBurstState);
	int GetNbOfUnconfiguredBurstMessages();

private:
	SetBurstConfigurationState m_enumState;
	std::string m_strPubConfigFile;

	DevicePtr m_oDev;
	std::list<BurstMessage> m_oBurstMessagesToConfigure;
	int m_nUnconfiguredBurstMessages;

public:
	uint16_t   m_nSubDevicesCount;
	uint16_t   m_nCurrentSubDeviceIndex;  // keeps tracking of cmd084 for each subDeviceIndex

private:
	void ComputeBurstMessagesToConfigureList();

};

}
}

#endif //APPSETBURSTCONFIGURATIONCMD_H_

