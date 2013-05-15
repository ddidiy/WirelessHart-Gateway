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

#include <WHartHost/applicationcmds/AppSetBurstConfigurationCmd.h>
#include <WHartHost/applicationcmds/IAppCommandVisitor.h>


namespace hart7 {
namespace hostapp {

AppSetBurstConfigurationCmd::AppSetBurstConfigurationCmd(const std::string& p_strPubConfigFile, DevicePtr p_oDev) :
				m_enumState(SET_BURSTCONF_READ_CONFIG_STATE),
				m_strPubConfigFile(p_strPubConfigFile),
				m_oDev(p_oDev),
				m_nUnconfiguredBurstMessages(0)
{	
	ComputeBurstMessagesToConfigureList();
}

bool AppSetBurstConfigurationCmd::Accept(IAppCommandVisitor& visitor)
{
	visitor.Visit(*this);
	return true;
}

void AppSetBurstConfigurationCmd::DumpToStream(std::ostream& p_rStream) const
{
	p_rStream << "AppSetBurstConfigurationCmd[CommandID=" << dbCommand.commandID << " Status=" << (int)dbCommand.commandStatus << "]";
}

void AppSetBurstConfigurationCmd::RegisterBurstMessageConfigurationStatus(BurstState p_enumBurstState)
{
	if (m_enumState == SET_BURSTCONF_TURNOFF_BURSTMESSAGES_STATE)
	{
		m_nUnconfiguredBurstMessages += ((p_enumBurstState != BURST_STATE_OFF && p_enumBurstState != BURST_STATE_ERROR) ? 1 : 0);
	}
	else if (m_enumState == SET_BURSTCONF_CONFIGURE_BURSTMESSAGES_STATE)
	{	
		m_nUnconfiguredBurstMessages += ((p_enumBurstState != BURST_STATE_SET && p_enumBurstState != BURST_STATE_ERROR) ? 1 : 0);
	}
}

void AppSetBurstConfigurationCmd::ComputeBurstMessagesToConfigureList()
{
	PublisherInfo& pubInfo = m_oDev->GetPublisherInfo();

	m_oBurstMessagesToConfigure.clear();

	for (BurstMessageSetT::iterator bmIt = pubInfo.burstMessageList.begin() ; bmIt != pubInfo.burstMessageList.end() ; ++bmIt)
	{	PublisherInfo::BurstMessageStateMap::const_iterator statesMapIt = pubInfo.burstMessagesState.find(bmIt->burstMessage);
		//don't send configuration cmds for current burst message if it is already configured or if a fatal error was previously encountered
		if ( (statesMapIt != pubInfo.burstMessagesState.end()) && 
			 (statesMapIt->second == BURST_STATE_SET || statesMapIt->second == BURST_STATE_ERROR) )
		{	continue;
		}
		m_oBurstMessagesToConfigure.push_back(*bmIt);
	}
}

//
const std::string& AppSetBurstConfigurationCmd::GetPubConfigFileName()
{ 
	return m_strPubConfigFile; 
}

void AppSetBurstConfigurationCmd::SetState(SetBurstConfigurationState p_enumState) 
{
	m_nUnconfiguredBurstMessages = 0;
	m_enumState = p_enumState;
}

AppSetBurstConfigurationCmd::SetBurstConfigurationState AppSetBurstConfigurationCmd::AppSetBurstConfigurationCmd::GetState()
{ 
	return m_enumState; 
}

DevicePtr AppSetBurstConfigurationCmd::GetDevice()
{ 
	return m_oDev; 
}

//
std::list<BurstMessage>& AppSetBurstConfigurationCmd::GetBurstMessagesToConfigure()
{	
	return m_oBurstMessagesToConfigure;
}

int AppSetBurstConfigurationCmd::GetNbOfUnconfiguredBurstMessages() 
{
	return m_nUnconfiguredBurstMessages;
}

}
}

