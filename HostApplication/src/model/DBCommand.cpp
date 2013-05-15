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

#include <sstream>
#include <WHartHost/model/DBCommand.h>
#include <ApplicationLayer/Model/CommonResponseCode.h>


namespace hart7 {
namespace hostapp {

DBCommandParameter::DBCommandParameter()
{
}
DBCommandParameter::DBCommandParameter(ParameterCode code, const std::string& value)
{
	parameterCode = code;
	parameterValue = value;
}

DBCommand::DBCommand()
{
	commandID = NO_COMMAND_ID;
	deviceID = NO_DEVICE_ID;
	commandCode = (CommandCode)NO_COMMAND_CODE;
	commandStatus = (CommandStatus)NO_COMMAND_STATUS;
	errorCode = rsNoStatus;
	generatedType = cgtManual; //manual by default
}

//to_string
const std::string DBCommand::ToString() const
{
	std::ostringstream str;
	DumpToStream(str);
	return str.str();
}
void DBCommand::DumpToStream(std::ostream& p_rStream) const
{
	p_rStream << "[CommandID=" << commandID << " CommandCode=" << commandCode << " CommandStatus=" << commandStatus << " DeviceID=" << deviceID << "]";
}


bool GetErrorCodeDescription(int responseCode, std::string& description)
{
	struct Map
	{
		int code;
		const char* description;
	};
	static Map
	    mapCodes[] = {
	        
			{ DBCommand::rsFailure_InvalidCommand, "invalid command"},
			{ DBCommand::rsFailure_InvalidDevice, "invalid device"},
	        { DBCommand::rsFailure_DeviceNotRegistered, "device not registered" },
	        { DBCommand::rsFailure_GatewayInvalidFailureCode, "gateway failure code" },
	        { DBCommand::rsFailure_HostSerializationError, "serialization error" },
	        { DBCommand::rsFailure_HostSendChannelError, "channel error when sending message" },
			{ DBCommand::rsFailure_HostSendTrackingError, "messsage tracking error when sending message" },
	        { DBCommand::rsFailure_InternalError, "internal error" },
			{ DBCommand::rsFailure_HostGWCmdAlreadySentError, "gateway: already wireless hart command sent for device" },
			{ DBCommand::rsFailure_CommandWasCancelled, "db command was cancelled" },

			{ /*GatewayIO::HostTimedOut =*/ -1 + DBCommand::rsFailure_HostReceptionErrorBase, "host timed-out" },
			{ /*GatewayIO::HostDisconnected =*/ -2 + DBCommand::rsFailure_HostReceptionErrorBase, "host disconnected" },
			{ /*GatewayIO::HostUnserializationFail =*/ -3 + DBCommand::rsFailure_HostReceptionErrorBase, "host unserialization error" },

			
			{ RCS_N00_Success + DBCommand::rsFailure_GatewayReponseCodeBase, "success" },
			{ RCS_E01_Undefined1 + DBCommand::rsFailure_GatewayReponseCodeBase, "undefined" },
			{ RCS_E02_InvalidSelection + DBCommand::rsFailure_GatewayReponseCodeBase, "invalid selection" },
			{ RCS_E03_PassedParameterTooLarge + DBCommand::rsFailure_GatewayReponseCodeBase, "passed parameter too large" },
			{ RCS_E04_PassedParameterTooSmall + DBCommand::rsFailure_GatewayReponseCodeBase, "passed parameter too small" },
			{ RCS_E05_TooFewDataBytesReceived + DBCommand::rsFailure_GatewayReponseCodeBase, "too few data bytes received" },
			{ RCS_E06_DeviceSpecificCommandError + DBCommand::rsFailure_GatewayReponseCodeBase, "device specific command error" },
			{ RCS_E07_InWriteProtectMode + DBCommand::rsFailure_GatewayReponseCodeBase, "in write protect mode" },

			{ RCM_E09_LowerRangeValueTooHigh + DBCommand::rsFailure_GatewayReponseCodeBase, "whart error 9, see the log for details" },

			{ RCM_E10_LowerRangeValueTooLow + DBCommand::rsFailure_GatewayReponseCodeBase, "whart error 10, see the log for details" },

			{ RCM_E11_UpperRangeValueTooHigh + DBCommand::rsFailure_GatewayReponseCodeBase, "whart error 11, see the log for details" },

			{ RCM_E12_UpperRangeValueTooLow + DBCommand::rsFailure_GatewayReponseCodeBase, "whart error 12, see the log for details" },
			
			{ RCM_E13_InvalidTransferFunctionCode + DBCommand::rsFailure_GatewayReponseCodeBase, "whart error 13" },
			
			{ RCM_W14_DynamicVarsReturnedForDeviceVars + DBCommand::rsFailure_GatewayReponseCodeBase, "whart error 14"},

			{ RCM_E15_InvalidAnalogChannelCodeNumber + DBCommand::rsFailure_GatewayReponseCodeBase, "invalid analog channel code" },

			{ RCS_E16_AccessRestricted + DBCommand::rsFailure_GatewayReponseCodeBase, "access restricted" },
			{ RCS_E17_InvalidDeviceVariableIndex + DBCommand::rsFailure_GatewayReponseCodeBase, "invaid device variable index" },
			{ RCS_E18_InvalidUnitsCode + DBCommand::rsFailure_GatewayReponseCodeBase, "invalid units code" },
			{ RCS_E19_DeviceVariableIndexNotAllowed + DBCommand::rsFailure_GatewayReponseCodeBase, "device variable index not allowed" },
			{ RCS_E20_InvalidExtendedCommandNumber + DBCommand::rsFailure_GatewayReponseCodeBase, "invalid extended command number" },
			{ RCS_E21_InIOCardNumber + DBCommand::rsFailure_GatewayReponseCodeBase, "in IOCrad number" },
			{ RCS_E22_InChannelNumber + DBCommand::rsFailure_GatewayReponseCodeBase, "in channel number" },
			{ RCS_E23_SubdeviceResponseTooLong + DBCommand::rsFailure_GatewayReponseCodeBase, "subdevice reponse too long" },

			{ RCM_E28_InvalidRangeUnitsCode + DBCommand::rsFailure_GatewayReponseCodeBase, "inavlid range units code" },
			{ RCM_E29_InvalidSpan + DBCommand::rsFailure_GatewayReponseCodeBase, "invalid span" },
			{ RCS_E32_Busy + DBCommand::rsFailure_GatewayReponseCodeBase, "busy" },
			{ RCS_E33_DelayedResponseInitiated + DBCommand::rsFailure_GatewayReponseCodeBase, "delayed response initiated" },
			{ RCS_E34_DelayedResponseRunning + DBCommand::rsFailure_GatewayReponseCodeBase, "delayed response running" },
			{ RCS_E35_DelayedResponseDead + DBCommand::rsFailure_GatewayReponseCodeBase, "delayed response dead" },
			{ RCS_E36_DelayedResponseConflict + DBCommand::rsFailure_GatewayReponseCodeBase, "delayed response conflict" },

			// reserved for future: E37-E59
			{ RCS_E37_ReservedForFuture37 + DBCommand::rsFailure_GatewayReponseCodeBase, "reserved for future 37" },
			{ RCS_E59_ReservedForFuture59 + DBCommand::rsFailure_GatewayReponseCodeBase, "reserved for future 59" },

			{ RCS_E60_PayloadTooLong + DBCommand::rsFailure_GatewayReponseCodeBase, "payoad too long" },
			{ RCS_E61_NoBuffersAvailable + DBCommand::rsFailure_GatewayReponseCodeBase, "no buffers available" },
			{ RCS_E62_NoAlarmEventBuffersAvailable + DBCommand::rsFailure_GatewayReponseCodeBase, "no alalrms event buffers available" },
			{ RCS_E63_PriorityTooLow + DBCommand::rsFailure_GatewayReponseCodeBase, "priority too low" },
			{ RCS_E64_CommandNotImplemented + DBCommand::rsFailure_GatewayReponseCodeBase, "commad not implemented" },

			{ RCM_E65_Declined + DBCommand::rsFailure_GatewayReponseCodeBase, "whart error 65, see the log for details" },
			
			{ RCM_E66_InvalidNumberOfSlots + DBCommand::rsFailure_GatewayReponseCodeBase, "whart error 66, see the log for details" },
			
			{ RCM_E67_InvalidGraphID  + DBCommand::rsFailure_GatewayReponseCodeBase, "whart error 67, see the log for details" },
			
			{ RCM_E68_UnknownNickname + DBCommand::rsFailure_GatewayReponseCodeBase, "whart error 68, see the log for details" },
			
			{ RCM_E69_InvalidLinkOptions + DBCommand::rsFailure_GatewayReponseCodeBase, "whart error 69, see the log for details" },
			
			{ RCM_E70_InvalidChannelOffset + DBCommand::rsFailure_GatewayReponseCodeBase, "whart error 70, see the log for details" },
			
			{ RCM_E71_CorrespNicknameAndRouteCorrespMismatch + DBCommand::rsFailure_GatewayReponseCodeBase, "whart error 71, see the log for details" },

			{ RCM_E72_NoMoreNeighborsAvailable + DBCommand::rsFailure_GatewayReponseCodeBase, "whart error 72, see the log for details" },

	      };

	//look-up for description into tables
	for (unsigned int i = 0; i < sizeof(mapCodes) / sizeof(mapCodes[0]); i++)
	{
		if (mapCodes[i].code == responseCode)
		{
			description = mapCodes[i].description;
			return true;
		}
	}

	return false; //not found description
}

std::string FormatResponseCode(int responseCode)
{
	std::string errorDescription;
	if (!GetErrorCodeDescription(responseCode, errorDescription))
	{
		errorDescription = "unknown application error";
	}

	//app error
	return boost::str(boost::format("App(%1%)-%2%") % (int)responseCode % errorDescription);
}


std::ostream& operator<< (std::ostream& p_rStream, const DBCommand& p_rDBCommand)
{	p_rDBCommand.DumpToStream(p_rStream);
	return p_rStream;
}


} //namespace hostapp
} //namespace hart7
