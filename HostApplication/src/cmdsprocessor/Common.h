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

#ifndef COMMON_H_
#define COMMON_H_

#include <stdint.h>
#include <list>

#include <WHartHost/model/PublisherInfo.h>
#include <WHartHost/model/Device.h>
#include <WHartHost/applicationcmds/AbstractAppCommand.h>
#include <WHartHost/applicationcmds/AppSetBurstConfigurationCmd.h>

namespace hart7 {
namespace hostapp {

class CommandsProcessor;

bool IsWHCmdNoValid(int cmdNo);

void* BuildReadValueCmdRequest(const PublishChannelSetT& p_rSet);

void* BuildBurstValueCmdRequest( const PublishChannelSetT& p_rSet,
								 PublishChannelSetT::const_iterator& p_oStart /*[in/out]*/);

void SendRoutesListRequests(const DevicePtr& device, int p_nIndex, int p_nSize, const AbstractAppCommandPtr& appCmd, CommandsProcessor* p_pProcessor);

/*true if request sent ; false if request not sent*/
bool AppSetBurstConfigurationCmd_SendReadBurstConfigCmd(AppSetBurstConfigurationCmd& p_rBurstConfigurationCmd, 
	const AbstractAppCommandPtr& appCmd, 
	CommandsProcessor* p_pProcessor);
/*true if request sent ; false if request not sent*/
bool AppSetBurstConfigurationCmd_SendTurnOffBurstMessagesCmd(AppSetBurstConfigurationCmd& p_rBurstConfigurationCmd, 
	const AbstractAppCommandPtr& appCmd, 
	CommandsProcessor* p_pProcessor);
/*true if request sent ; false if request not sent*/
bool AppSetBurstConfigurationCmd_SendGetSubDeviceCountCmd(AppSetBurstConfigurationCmd& p_rBurstConfigurationCmd,
    const AbstractAppCommandPtr& p_rAppCmd,
    CommandsProcessor* p_pProcessor);
/*true if request sent ; false if request not sent*/
bool AppSetBurstConfigurationCmd_SendGetSubDeviceIdCmd(AppSetBurstConfigurationCmd& p_rBurstConfigurationCmd,
    const AbstractAppCommandPtr& p_rAppCmd,
    CommandsProcessor* p_pProcessor);
/*true if request sent ; false if request not sent*/
bool AppSetBurstConfigurationCmd_SendConfigureBurstMessagesCmd(AppSetBurstConfigurationCmd& p_rBurstConfigurationCmd, 
	const AbstractAppCommandPtr& appCmd, 
	CommandsProcessor* p_pProcessor);
	
void AppSetBurstConfigurationCmd_Finished(AppSetBurstConfigurationCmd& p_rBurstConfigurationCmd);

void SendRequest009(const DeviceVariableCodes deviceVariables[], const int varsNo, const int p_nInnerDataHandler, const MAC& mac,
					const AbstractAppCommandPtr& p_rAppCmd, const CommandsProcessor* p_pProcessor);

void SendRequest074(const int p_nInnerDataHandler, const MAC& mac,
                    const AbstractAppCommandPtr& p_rAppCmd, const CommandsProcessor* p_pProcessor);

void SendRequest084(const uint16_t p_nSubDeviceIndex, const int p_nInnerDataHandler, const MAC& mac,
                    const AbstractAppCommandPtr& p_rAppCmd, const CommandsProcessor* p_pProcessor);

void SendRequest101(const uint8_t p_nBurstMessage, const int p_nInnerDataHandler, const MAC& mac,
                    const AbstractAppCommandPtr& p_rAppCmd, const CommandsProcessor* p_pProcessor);

void SendRequest105(const uint8_t p_nBurstMessage, const int p_nInnerDataHandler, const MAC& mac,
                    const AbstractAppCommandPtr& p_rAppCmd, const CommandsProcessor* p_pProcessor);




}
}
#endif //COMMON_H_
