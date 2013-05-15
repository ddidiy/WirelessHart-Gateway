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

#ifndef DBCOMMAND_H_
#define DBCOMMAND_H_

#include <string>
#include <vector>
#include <iostream>

#include <nlib/datetime.h>
#include <boost/format.hpp>


#include <WHartHost/model/PublisherInfo.h>

namespace hart7 {
namespace hostapp {



class DBCommand;
typedef std::vector<DBCommand> DBCommandsList;

class DBCommandParameter
{
public:
	enum ParameterCode
	{
		ReadValue_ChannelID = 10,
		
		ScheduleReport_DevID = 61,
		NeighbourHealthReport_DevID = 62,
		DevHealthReport_DevID = 63,

		GeneralCommand_CmdNo = 92,
		GeneralCommand_DataBytes = 93,

		ConfigureBurst_PubConfFile = 1200,
		BypassIOCache = 1300
	};

	ParameterCode parameterCode;
	std::string parameterValue;

	DBCommandParameter();
	DBCommandParameter(ParameterCode code, const std::string& value);
};


class DBCommand
{
public:
	typedef std::vector<DBCommandParameter> ParametersList;

	enum CommandCode
	{
		ccGetTopology = 0,
		ccReadValue = 1,
		ccNotifSubscribe = 3,
		ccNotifUnSubscribe = 4,
		ccGeneralCmd = 11,

		ccReadBurstConfig = 105,

		ccTopologyNotify = 9,
			
		ccRoutesReport = 120,
		ccServicesReport = 121,
		ccSuperframesReport = 122,
		ccDeviceScheduleLinkReport = 123,
		ccNeighborHealthReport = 124,
		ccDeviceHealthReport = 125,
		ccAutodetectBurstsConfig = 126
	};
	
	enum ResponseStatus
	{
		rsSuccess = 1,
		rsNoStatus = 0,
		rsFailure_InvalidCommand = -1,
		rsFailure_InvalidDevice = -2,
		rsFailure_DeviceNotRegistered = -3,
		rsFailure_GatewayInvalidFailureCode = -4,
		rsFailure_HostSerializationError = -5,
		rsFailure_HostSendChannelError = -6,
		rsFailure_HostSendTrackingError = -7,
		rsFailure_InternalError = -8,
		rsFailure_HostGWCmdAlreadySentError = -9,
		rsFailure_CommandWasCancelled = -10,
		
		rsFailure_HostReceptionErrorBase = -200,  //see GatewayIO.h
		rsFailure_GatewayReponseCodeBase = 0,	  //see CommonResponseCode.h 
	};

	enum CommandStatus
	{
		csNew = 0,
		csSent = 1,
		csResponded = 2,
		csFailed = 3
	};

	enum CommandGeneratedType
	{
		cgtManual = 0,
		cgtAutomatic = 1
	};

	static const int NO_COMMAND_ID = -1;
	static const int NO_DEVICE_ID = -1;
	static const int NO_COMMAND_CODE = -1;
	static const int NO_COMMAND_STATUS = -1;

public:
	DBCommand();

//to_string
public:
	const std::string ToString() const;
	friend std::ostream& operator<< (std::ostream& p_rStream, const DBCommand& p_rDBCommand);
protected:
	void DumpToStream(std::ostream& p_rStream) const;

public:
	int commandID;
	int deviceID;
	CommandCode commandCode;
	CommandStatus commandStatus;

	nlib::DateTime timePosted;
	nlib::DateTime timeResponded;
	ResponseStatus errorCode;
	std::string errorReason;

	std::string response;

	ParametersList parameters;

	CommandGeneratedType generatedType;
	


};

std::string FormatResponseCode(int responseCode);
std::ostream& operator<< (std::ostream& p_rStream, const DBCommand& p_rDBCommand);


} // namespace hostapp
} // namespace hart7

#endif /*COMMAND_H_*/
