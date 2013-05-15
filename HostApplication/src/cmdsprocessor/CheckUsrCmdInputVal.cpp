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



#include "CheckUsrCmdInputVal.h"
#include "Common.h"



namespace hart7 {
namespace hostapp {


//devices
DevicePtr CheckUsrCmdInputVal::GetRegisteredDevice(int deviceID)
{
	DevicePtr device = devices.FindDevice(deviceID);
	if (!device)
	{
		LOG_ERROR_APP("[DBParamsCheck]: DeviceID:" << deviceID <<" not found!");
		THROW_EXCEPTION1(DeviceNotFoundException, deviceID);
	}
	if (device->Status() != Device::dsRegistered)
	{
		LOG_DEBUG_APP("[DBParamsCheck]: Device:" << device->Mac() << " not registered");
		THROW_EXCEPTION1(DeviceNodeRegisteredException, device);
	}
	return device;
}

//command's parameters
bool CheckUsrCmdInputVal::GetOptionalParameterValue(DBCommandParameter::ParameterCode parameterCode, 
											const DBCommand::ParametersList& list, int &val)
{
	for (DBCommand::ParametersList::const_iterator it = list.begin(); it != list.end(); it++)
	{
		if (it->parameterCode == parameterCode)
		{
			val = atoi(it->parameterValue.c_str());
			return true;
		}
	}
	return false;
}
int CheckUsrCmdInputVal::GetParameterValueAsInt(DBCommandParameter::ParameterCode parameterCode, 
										   const DBCommand::ParametersList& list,
										   const std::string& parameterName)
{
	for (DBCommand::ParametersList::const_iterator it = list.begin(); it != list.end(); it++)
	{
		if (it->parameterCode == parameterCode)
		{
			return atoi(it->parameterValue.c_str());
		}
	}
	THROW_EXCEPTION1(InvalidCommandException, parameterName + " Parameter not found!");
}
std::string CheckUsrCmdInputVal::GetParameterValueAsString(DBCommandParameter::ParameterCode parameterCode,
														   const DBCommand::ParametersList& list, 
														   const std::string& parameterName)
{
	for (DBCommand::ParametersList::const_iterator it = list.begin(); it != list.end(); it++)
	{
		if (it->parameterCode == parameterCode)
		{
			return it->parameterValue;
		}
	}
	THROW_EXCEPTION1(InvalidCommandException, parameterName + " Parameter not found!");
}
void CheckUsrCmdInputVal::GetParamValuesAsInt(DBCommandParameter::ParameterCode parameterCode, 
						 const DBCommand::ParametersList& allParameterslist, 
						 std::list<int>& list /*[in/out]*/)
{
	for (DBCommand::ParametersList::const_iterator it = allParameterslist.begin(); it != allParameterslist.end(); it++)
	{
		if (it->parameterCode == parameterCode)
		{
			list.push_back(atoi(it->parameterValue.c_str()));
		}
	}
}


void CheckUsrCmdInputVal::TestUsrParams_ccGetTopology(DBCommand& command/*[in]*/)
{
	DevicePtr device = devices.FindDevice(command.deviceID);
	if (!device)
	{
		LOG_ERROR_APP("[DBParamsCheck]: Topology with no gw:" << device->Mac());
		THROW_EXCEPTION1(DeviceNotFoundException, command.deviceID);
	}
	if (device->Type() != Device::dtGateway)
	{
		LOG_ERROR_APP("[DBParamsCheck]: Topology is not sent for gw!");
		THROW_EXCEPTION1(InvalidCommandException, "Topology is not sent for gw!");
	}
}
void CheckUsrCmdInputVal::TestUsrParams_ccNotifSubscribe(DBCommand& command/*[in]*/, DevicePtr &dev/*[out]*/)
{
	dev = GetRegisteredDevice(command.deviceID);
}
void CheckUsrCmdInputVal::TestUsrParams_ccNotifUnSubscribe(DBCommand& command/*[in]*/)
{
	//just checking
	DevicePtr device = devices.FindDevice(command.deviceID);
	if (!device)
	{
		LOG_ERROR_APP("[DBParamsCheck-Unsubscribe]: DeviceID:" << command.deviceID <<" not found!");
		THROW_EXCEPTION1(DeviceNotFoundException, command.deviceID);
	}
}
void CheckUsrCmdInputVal::TestUsrParams_ccBurstConfiguration(DBCommand& command/*[in]*/, DevicePtr &dev/*[out]*/, std::string& p_rPubConfFileName)
{
	dev = GetRegisteredDevice(command.deviceID);
	p_rPubConfFileName = GetParameterValueAsString(DBCommandParameter::ConfigureBurst_PubConfFile, command.parameters, "pub conf file name");
}
void CheckUsrCmdInputVal::TestUsrParams_ccTopologyNotify(DBCommand& command/*[in]*/)
{
}
void CheckUsrCmdInputVal::TestUsrParams_ccGeneralCmd(DBCommand& command/*[in]*/, 
									  int &ValueCmdNo/*[out]*/, std::string &ValueDataBytes/*[out]*/,
									  bool &bypassIOCache)
{
	DevicePtr device = GetRegisteredDevice(command.deviceID);
	ValueCmdNo = GetParameterValueAsInt(DBCommandParameter::GeneralCommand_CmdNo, command.parameters, "Command No." );
	ValueDataBytes = GetParameterValueAsString(DBCommandParameter::GeneralCommand_DataBytes, command.parameters, "Data bytes");

	if(ValueCmdNo < 0)
	{
		LOG_ERROR_APP("Invalid command value");
		THROW_EXCEPTION1(InvalidCommandException, "Invalid command value");
	}

	if(ValueDataBytes.size() % 2 != 0)
	{
		LOG_ERROR_APP("Invalid data bytes value");
		THROW_EXCEPTION1(InvalidCommandException, "Invalid data bytes value");
	}
	
	

	for(int i = 0; i< (int)ValueDataBytes.size(); i++)
	{
		if(!isxdigit(ValueDataBytes[i]))
			{
			LOG_ERROR_APP("Invalid data bytes value");
			THROW_EXCEPTION1(InvalidCommandException, "Invalid data bytes value");
			}
	}

	bypassIOCache = false;
	int bypass = 0;
	if (GetOptionalParameterValue(DBCommandParameter::BypassIOCache,command.parameters,bypass))
		bypassIOCache = (bypass != 0);

}
void CheckUsrCmdInputVal::TestUsrParams_ccReadValue(DBCommand& command/*[in]*/, 
										  PublishChannel & channel/*[out]*/,
										  bool &bypassIOCache)
{
	DevicePtr dev = GetRegisteredDevice(command.deviceID);
	if ( dev->GetPublisherInfo().channelList.size() == 0 )
	{	LOG_ERROR_APP("[DBParamsCheck]: ReadValue -> No channels found");
		THROW_EXCEPTION1(InvalidCommandException, "No channels found");
	}
	
	int channelID = (int) GetParameterValueAsInt(DBCommandParameter::ReadValue_ChannelID,command.parameters, "ReadValueChannelID" );

	PublishChannelSetT::iterator it = FindChannel(dev->GetPublisherInfo().channelList, channelID);

	if (it == dev->GetPublisherInfo().channelList.end())
	{	LOG_ERROR_APP("[DBParamsCheck]: ReadValue -> WHCommand number not found in channels");
		THROW_EXCEPTION1(InvalidCommandException, "WHCommand number not found in channels");
	}

	channel = *it;
	if (!IsWHCmdNoValid(channel.cmdNo))
	{
		LOG_ERROR_APP("[DBParamsCheck]: ReadValue -> invalid WHCommand number");
		THROW_EXCEPTION1(InvalidCommandException, "invalid WHCommand number");
	}

	bypassIOCache = false;
	int bypass = 0;
	if (GetOptionalParameterValue(DBCommandParameter::BypassIOCache,command.parameters,bypass))
		bypassIOCache = (bypass != 0);
}
void CheckUsrCmdInputVal::TestUsrParams_ccDevHealthReport(DBCommand& command/*[in]*/, std::list<std::pair<int, MAC> > &list/*[in/out]*/)
{
	std::list<int> deviceIDs;
	GetParamValuesAsInt(DBCommandParameter::DevHealthReport_DevID, command.parameters, deviceIDs);

	if (deviceIDs.size() == 0)
	{
		DevicesManager::const_iterator_by_nick devIt = devices.BeginRegisteredByNick ();
		
		if (devIt == devices.EndRegisteredByNick())
		{
			LOG_ERROR_APP("[DBParamsCheck]: DevHealthReport -> no registered devices found");
			THROW_EXCEPTION1(InvalidCommandException, "no registered devices found");
		}

		for ( ; devIt != devices.EndRegisteredByNick(); ++devIt)
			list.push_back(std::pair<int, MAC> (devIt->second->id, devIt->second->Mac()));

		return;
	}

	for(std::list<int>::const_iterator i = deviceIDs.begin(); i != deviceIDs.end(); ++i)
		list.push_back(std::pair<int, MAC> (*i, GetRegisteredDevice(*i)->Mac()));

}

void CheckUsrCmdInputVal::TestUsrParams_ccNeighborHealthReport(DBCommand& command/*[in]*/, std::list<std::pair<int, MAC> >  &list/*[in/out]*/)
{
	std::list<int> deviceIDs;
	GetParamValuesAsInt(DBCommandParameter::NeighbourHealthReport_DevID, command.parameters, deviceIDs);

	if (deviceIDs.size() == 0)
	{
		DevicesManager::const_iterator_by_nick devIt = devices.BeginRegisteredByNick ();

		if (devIt == devices.EndRegisteredByNick())
		{
			LOG_ERROR_APP("[DBParamsCheck]: ScheduleReport -> no registered devices found");
			THROW_EXCEPTION1(InvalidCommandException, "no registered devices found");
		}

		for ( ; devIt != devices.EndRegisteredByNick(); ++devIt)
			list.push_back(std::pair<int, MAC> (devIt->second->id, devIt->second->Mac()));

		return;
	}

	for(std::list<int>::const_iterator i = deviceIDs.begin(); i != deviceIDs.end(); ++i)
		list.push_back(std::pair<int, MAC> (*i, GetRegisteredDevice(*i)->Mac()));
}

void CheckUsrCmdInputVal::TestUsrParams_ccDeviceScheduleLinkReport(DBCommand& command/*[in]*/, std::list<std::pair<int, MAC> > &list/*[in/out]*/)
{
	std::list<int> deviceIDs;
	GetParamValuesAsInt(DBCommandParameter::ScheduleReport_DevID, command.parameters, deviceIDs);

	if (deviceIDs.size() == 0)
	{
		DevicesManager::const_iterator_by_nick devIt = devices.BeginRegisteredByNick ();

		if (devIt == devices.EndRegisteredByNick())
		{
			LOG_ERROR_APP("[DBParamsCheck]: ScheduleReport -> no registered devices found");
			THROW_EXCEPTION1(InvalidCommandException, "no registered devices found");
		}

		for ( ; devIt != devices.EndRegisteredByNick(); ++devIt)
			list.push_back(std::pair<int, MAC> (devIt->second->id, devIt->second->Mac()));

		return;
	}

	for(std::list<int>::const_iterator i = deviceIDs.begin(); i != deviceIDs.end(); ++i)
		list.push_back(std::pair<int, MAC> (*i, GetRegisteredDevice(*i)->Mac()));
}

void CheckUsrCmdInputVal::TestUsrParams_ccReadBurstConfig(DBCommand& command/*[in]*/, DevicePtr &dev/*[out]*/)
{
    dev = GetRegisteredDevice(command.deviceID);
    if (!dev)
    {
        dev->IssueDiscoveryBurstConfigCmd = hart7::hostapp::AUTODETECT_NONE;
        LOG_ERROR_APP("[CheckUsrCmdInputVal]: ReadBurstConfig -> No device");
        THROW_EXCEPTION1(InvalidCommandException, "No device found");
    }
}

}
}
