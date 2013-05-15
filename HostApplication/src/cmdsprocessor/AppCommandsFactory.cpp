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


#include "AppCommandsFactory.h"

#include <WHartHost/applicationcmds/AppTopologyCommand.h>
#include <WHartHost/applicationcmds/AppSetBurstNotificationCmd.h>
#include <WHartHost/applicationcmds/AppUnsetBurstNotificationCmd.h>
#include <WHartHost/applicationcmds/AppSetBurstConfigurationCmd.h>
#include <WHartHost/applicationcmds/AppSetTopoNotificationCmd.h>
#include <WHartHost/applicationcmds/AppGeneralCommand.h>
#include <WHartHost/applicationcmds/AppReadValueCmd.h>
#include <WHartHost/applicationcmds/AppRoutesReportCmd.h>
#include <WHartHost/applicationcmds/AppServicesReportCmd.h>
#include <WHartHost/applicationcmds/AppDeviceHealthReportCmd.h>
#include <WHartHost/applicationcmds/AppSuperframesReportCmd.h>
#include <WHartHost/applicationcmds/AppDeviceScheduleLinkReportCmd.h>
#include <WHartHost/applicationcmds/AppNeighborHealthReportCmd.h>
#include <WHartHost/applicationcmds/AppDiscoveryBurstConfigCmd.h>

#include "CheckUsrCmdInputVal.h"

namespace hart7 {
namespace hostapp {


AbstractAppCommandPtr AppCommandsFactory::Create(DBCommand& command, DevicesManager& devices)
{
	LOG_DEBUG_APP("[AppCommand]: create for db command:" << command);

	switch ( command.commandCode )
	{
	case DBCommand::ccGetTopology:
		CheckUsrCmdInputVal(devices).TestUsrParams_ccGetTopology(command);
		return AbstractAppCommandPtr(new AppTopologyCommand());

	case DBCommand::ccNotifSubscribe:
	{
		DevicePtr dev;
		CheckUsrCmdInputVal(devices).TestUsrParams_ccNotifSubscribe(command, dev);
		return AbstractAppCommandPtr(new AppSetBurstNotificationCmd(dev->GetPublisherInfo().channelList, 
			dev->GetPublisherInfo().burstMessageList, dev->GetPublisherInfo().triggersList));
	}
	case DBCommand::ccNotifUnSubscribe:
		CheckUsrCmdInputVal(devices).TestUsrParams_ccNotifUnSubscribe(command);
		return AbstractAppCommandPtr(new AppUnsetBurstNotificationCmd());
	
	case DBCommand::ccTopologyNotify:
		CheckUsrCmdInputVal(devices).TestUsrParams_ccTopologyNotify(command);
		return AbstractAppCommandPtr(new AppSetTopoNotificationCmd());

	case DBCommand::ccGeneralCmd:
	{
		int retValueCmdNo;  /* cmdNo */
		std::string retValueDataBytes; /* dataBytes */ 
		bool bypassIOCache;
		CheckUsrCmdInputVal(devices).TestUsrParams_ccGeneralCmd(command, retValueCmdNo, retValueDataBytes, bypassIOCache);
		return AbstractAppCommandPtr(new AppGeneralCommand(retValueCmdNo, retValueDataBytes, command.commandID, command.deviceID, bypassIOCache));	
	}
	case DBCommand::ccReadValue:
		{
		/*compute the channel list for the req cmd*/
		PublishChannel channel;
		bool bypassIOCache;
		CheckUsrCmdInputVal(devices).TestUsrParams_ccReadValue(command, channel, bypassIOCache);
		PublishChannelSetT channelsSet;
		channelsSet.insert(channel);
		return AbstractAppCommandPtr(new AppReadValueCmd( channel.cmdNo, command.deviceID, DeviceReading::ReadValue, channelsSet, bypassIOCache));
		}
	case DBCommand::ccRoutesReport:
		return AbstractAppCommandPtr(new AppRoutesReportCmd(devices.RegisteredDevicesNo()));
	case DBCommand::ccServicesReport:
		return AbstractAppCommandPtr(new AppServicesReportCmd(devices.RegisteredDevicesNo()));
	case DBCommand::ccDeviceHealthReport:
	{
		std::list<std::pair<int, MAC> > devicesList;
		CheckUsrCmdInputVal(devices).TestUsrParams_ccDevHealthReport(command, devicesList);
		return AbstractAppCommandPtr(new AppDeviceHealthReportCmd(devicesList));
	}
	case DBCommand::ccSuperframesReport:
		return AbstractAppCommandPtr(new AppSuperframesReportCmd(devices.RegisteredDevicesNo()));
	case DBCommand::ccDeviceScheduleLinkReport:
	{
		std::list<std::pair<int,MAC> > devicesList;
		CheckUsrCmdInputVal(devices).TestUsrParams_ccDeviceScheduleLinkReport(command, devicesList);
		return AbstractAppCommandPtr(new AppDeviceScheduleLinksReportCmd(devicesList));
	}		
	case DBCommand::ccNeighborHealthReport:
	{
		std::list<std::pair<int, MAC> > devicesList;
		CheckUsrCmdInputVal(devices).TestUsrParams_ccNeighborHealthReport(command, devicesList);
		return AbstractAppCommandPtr(new AppNeighborHealthReportCmd(devicesList));
	}
	case DBCommand::ccAutodetectBurstsConfig:
	{
		DevicePtr dev;
		std::string pubConfFileName;
		CheckUsrCmdInputVal(devices).TestUsrParams_ccBurstConfiguration(command, dev, pubConfFileName);
		//for cancelling command
		dev->configBurstDBCmdID = command.commandID;
		return AbstractAppCommandPtr(new AppSetBurstConfigurationCmd(pubConfFileName, dev));
	}
    case DBCommand::ccReadBurstConfig:
    {
        DevicePtr dev;
        CheckUsrCmdInputVal(devices).TestUsrParams_ccReadBurstConfig(command, dev);
        MAC mac = dev->Mac();
        return AbstractAppCommandPtr(new AppDiscoveryBurstConfigCmd(mac));
    }
	default:
		break;
	}

	LOG_ERROR_APP("[AppCommand]: create with Unknown CommandCode=" << command.commandCode);
	THROW_EXCEPTION1(InvalidCommandException, "unknown command");
}

} // namespace hostapp
} // namespace hart7
