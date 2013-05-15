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


#include "SaveRespErr.h"
#include <WHartHost/database/DBCommandsManager.h>
#include <WHartHost/gateway/GatewayIO.h>


namespace hart7 {
namespace hostapp {


//save db command error
void SaveRespErr::CommandFailed(DBCommand::ResponseStatus p_oErrorCode)
{
	std::string errorReason = FormatResponseCode(p_oErrorCode);
	try
	{
		LOG_DEBUG_APP("[DBCommand] mark Command=" << m_rDbCommand << " as failed!" << " ErrorCode=" << p_oErrorCode
		    << " ErrorReason=" << errorReason);
		m_rDbCmdManager.SetCommandFailed(m_rDbCommand, nlib::CurrentUniversalTime(), p_oErrorCode, errorReason);
	}
	catch (std::exception& ex)
	{
		LOG_ERROR_APP("[DBCommand] error on setting command as failed! Command=" << m_rDbCommand << " error="
		    << ex.what());
	}
}


// check errors
// wireless

bool SaveRespErr::IsErr_769(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[769-(ReadJoinStatus)]: hostError=" << p_nHostErrCode);
		return true;
	}
	//wh error codes
	switch(p_nWhErrCode)
	{
	case C769_NOO /*= RCS_N00_Success*/:
		break;
	case C769_E06 /*= RCS_E06_DeviceSpecificCommandError*/:
		LOG_ERROR_APP("[769-(ReadJoinStatus)]: RCS_E06_DeviceSpecificCommandError");
		return true;
	case C769_E32 /*= RCS_E32_Busy*/:
		LOG_ERROR_APP("[769-(ReadJoinStatus)]: RCS_E32_Busy");
		return true;
	default:
		LOG_ERROR_APP("[769-(ReadJoinStatus)]: invalid gateway error = " << p_nWhErrCode);
		return true;
	}

	return false;
}
bool SaveRespErr::IsErr_778(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[778-(ReadBatteryLife)]: hostError=" << p_nHostErrCode);
		return true;
	}
	//wh error codes
	switch(p_nWhErrCode)
	{
	case C778_N00 /*= RCS_N00_Success*/:
		break;
	case C778_E06 /*= RCS_E06_DeviceSpecificCommandError*/:
		LOG_ERROR_APP("[778-(ReadBatteryLife)]: RCS_E06_DeviceSpecificCommandError");
		return true;
	case C778_E32 /*= RCS_E32_Busy*/:
		LOG_ERROR_APP("[778-(ReadBatteryLife)]: RCS_E32_Busy");
		return true;
	default:
		LOG_ERROR_APP("[778-(ReadBatteryLife)]: invalid gateway error = " << p_nWhErrCode);
		return true;
	}

	return false;
}

bool SaveRespErr::IsErr_780(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[780-(NeighborHealthList)]: hostError=" << p_nHostErrCode);
		return true;
	}

	//wh error codes
	switch(p_nWhErrCode)
	{
	case C780_N00 /*= RCS_N00_Success*/:
		break;
	case C780_E02: /*= RCS_E02_InvalidSelection*/
		LOG_ERROR_APP("[780-(NeighborHealthList)]: RCS_E02_InvalidSelection");
 		return true;
	case C780_E05 /*= RCS_E05_TooFewDataBytesReceived*/:
		LOG_ERROR_APP("[780-(NeighborHealthList)]: RCS_E05_TooFewDataBytesReceived");
 		return true;
	case C780_W08: /*= RCM_W08_SetToNearestPossibleValue :*/
		LOG_WARN_APP("[780-(NeighborHealthList)]: RCM_W08_SetToNearestPossibleValue");
		break;
	default:
		LOG_ERROR_APP("[780-(NeighborHealthList)]: invalid gateway error = " << p_nWhErrCode);
		return true;
	}

	return false;
}

bool SaveRespErr::IsErr_783(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[783-(ReadSuperframeList)]: hostError=" << p_nHostErrCode);
		return true;
	}
	//wh error codes
	switch(p_nWhErrCode)
	{
	case C783_N00 /*= RCS_N00_Success*/:
		break;
	case C783_E05 /*= RCS_E05_TooFewDataBytesReceived*/:
		LOG_ERROR_APP("[783-(ReadSuperframeList)]: RCS_E05_TooFewDataBytesReceived");
 		return true;
	case C783_W08: /*= RCM_W08_SetToNearestPossibleValue :*/
		LOG_WARN_APP("[783-(ReadSuperframeList)]: RCM_W08_SetToNearestPossibleValue");
		break;
	default:
		LOG_ERROR_APP("[783-(ReadSuperframeList)]: invalid gateway error = " << p_nWhErrCode);
		return true;
	}

	return false;
}
bool SaveRespErr::IsErr_784(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[784-(ReadLinkList)]: hostError=" << p_nHostErrCode);
		return true;
	}
	//wh error codes
	switch(p_nWhErrCode)
	{
	case C784_N00 /*= RCS_N00_Success*/:
		break;
	case C784_E02: /*= RCS_E02_InvalidSelection*/
		LOG_ERROR_APP("[784-(ReadLinkList)]: RCS_E02_InvalidSelection");
		return true;
	case C784_E05 /*= RCS_E05_TooFewDataBytesReceived*/:
		LOG_ERROR_APP("[784-(ReadLinkList)]: RCS_E05_TooFewDataBytesReceived");
 		return true;
	case C784_W08: /*= RCM_W08_SetToNearestPossibleValue :*/
		LOG_WARN_APP("[784-(ReadLinkList)]: RCM_W08_SetToNearestPossibleValue");
		break;
	default:
		LOG_ERROR_APP("[784-(ReadLinkList)]: invalid gateway error = " << p_nWhErrCode);
		return true;
	}

	return false;
}

bool SaveRespErr::IsErr_785(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[785-(ReadGraphList)]: hostError=" << p_nHostErrCode);
		return true;
	}

	//wh error codes
	switch(p_nWhErrCode)
	{
	case C785_N00 /*= RCS_N00_Success*/:
		break;
	case C785_E05 /*= RCS_E05_TooFewDataBytesReceived*/:
		LOG_ERROR_APP("[785-(ReadGraphList)]: RCS_E05_TooFewDataBytesReceived");
 		return true;
	case C785_E02 /*= RCS_E02_InvalidSelection*/:
		LOG_ERROR_APP("[785-(ReadGraphList)]: RCS_E02_InvalidSelection");
 		return true;
	case C785_W08: /*= RCM_W08_SetToNearestPossibleValue :*/
		LOG_WARN_APP("[785-(ReadGraphList)]: RCM_W08_SetToNearestPossibleValue");
		break;
	default:
		LOG_ERROR_APP("[785-(ReadGraphList)]: invalid gateway error = " << p_nWhErrCode);
		return true;
	}

	return false;
}

bool SaveRespErr::IsErr_787(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[787-(ReadNeighborSignalLevel)]: hostError=" << p_nHostErrCode);
		return true;
	}
	//wh error codes
	switch(p_nWhErrCode)
	{
	case C787_N00 /*= RCS_N00_Success*/:
		break;
	case C787_E05 /*= RCS_E05_TooFewDataBytesReceived*/:
		LOG_ERROR_APP("[787-(ReadNeighborSignalLevel)]: RCS_E05_TooFewDataBytesReceived");
		return true;
	case C787_W08: /*= RCM_W08_SetToNearestPossibleValue :*/
		LOG_WARN_APP("[787-(ReadNeighborSignalLevel)]: RCM_W08_SetToNearestPossibleValue");
		break;
	default:
		LOG_ERROR_APP("[787-(ReadNeighborSignalLevel)]: invalid gateway error = " << p_nWhErrCode);
		return true;
	}

	return false;
}
bool SaveRespErr::IsErr_779(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[779-(ReadDeviceHealth)]: hostError=" << p_nHostErrCode);
		return true;
	}

	//wh error codes
	switch(p_nWhErrCode)
	{
	case C779_N00 /*= RCS_N00_Success*/:
		break;
	default:
		LOG_ERROR_APP("[779-(ReadDeviceHealth)]: invalid gateway error = " << p_nWhErrCode);
		return true;
	}

	return false;
}
bool SaveRespErr::IsErr_788(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[788-(AlarmPathDown)]: hostError=" << p_nHostErrCode);
		return true;
	}

	//wh error codes
	switch(p_nWhErrCode)
	{
	case C788_N00 /*= RCS_N00_Success*/:
		break;
	case C788_E16: // RCS_E16_AccessRestricted,
		LOG_ERROR_APP("[788-(AlarmPathDown)]: RCS_E16_AccessRestricted");	
	default:
		LOG_ERROR_APP("[788-(AlarmPathDown)]: invalid gateway error = " << p_nWhErrCode);
		return true;
	}
	return false;
}
bool SaveRespErr::IsErr_789(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[789-(SourceRouteFailed)]: hostError=" << p_nHostErrCode);
		return true;
	}

	//wh error codes
	switch(p_nWhErrCode)
	{
	case C789_NOO /*= RCS_N00_Success*/:
		break;
	case C789_E16: // RCS_E16_AccessRestricted,
		LOG_ERROR_APP("[789-(SourceRouteFailed)]: RCS_E16_AccessRestricted");	
	default:
		LOG_ERROR_APP("[789-(SourceRouteFailed)]: invalid gateway error = " << p_nWhErrCode);
		return true;
	}
	return false;
}
bool SaveRespErr::IsErr_790(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[790-(GraphRouteFailed)]: hostError=" << p_nHostErrCode);
		return true;
	}

	//wh error codes
	switch(p_nWhErrCode)
	{
	case C790_NOO /*= RCS_N00_Success*/:
		break;
	case C790_E16: // RCS_E16_AccessRestricted,
		LOG_ERROR_APP("[790-(GraphRouteFailed)]: RCS_E16_AccessRestricted");	
	default:
		LOG_ERROR_APP("[790-(GraphRouteFailed)]: invalid gateway error = " << p_nWhErrCode);
		return true;
	}
	return false;
}
bool SaveRespErr::IsErr_791(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[791-(TransportLayerFailed)]: hostError=" << p_nHostErrCode);
		return true;
	}

	//wh error codes
	switch(p_nWhErrCode)
	{
	case C791_NOO /*= RCS_N00_Success*/:
		break;
	case C791_E16: // RCS_E16_AccessRestricted,
		LOG_ERROR_APP("[791-(TransportLayerFailed)]: RCS_E16_AccessRestricted");	
	default:
		LOG_ERROR_APP("[791-(TransportLayerFailed)]: invalid gateway error = " << p_nWhErrCode);
		return true;
	}
	return false;
}
bool SaveRespErr::IsErr_800(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[800-(ReadServicesList)]: hostError=" << p_nHostErrCode);
		return true;
	}

	//wh error codes
	switch(p_nWhErrCode)
	{
	case C800_NOO /*= RCS_N00_Success*/:
		break;
	case C800_E05 /*= RCS_E05_TooFewDataBytesReceived*/:
		LOG_ERROR_APP("[800-(ReadServicesList)]: RCS_E05_TooFewDataBytesReceived");
 		return true;
	case C800_E06 /*= RCS_E06_DeviceSpecificCommandError*/:
		LOG_ERROR_APP("[800-(ReadServicesList)]: RCS_E06_DeviceSpecificCommandError");
		return true;
	case C800_W08: /*= RCM_W08_SetToNearestPossibleValue :*/
		LOG_WARN_APP("[800-(ReadServicesList)]: RCM_W08_SetToNearestPossibleValue");
		break;
	default:
		LOG_ERROR_APP("[800-(ReadServicesList)]: invalid gateway error = " << p_nWhErrCode);
		return true;
	}

	return false;
}
bool SaveRespErr::IsErr_801(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[802-(ReadRouteList)]: hostError=" << p_nHostErrCode);
		return true;
	}
	switch(p_nWhErrCode)
	{
	case C801_NOO:/*RCS_N00_Success*/
		break;
	case C801_E05:/*RCS_E05_TooFewDataBytesReceived*/
		LOG_ERROR_APP("[801-(DeleteService)]: RCS_E05_TooFewDataBytesReceived");
		return true;
	case C801_E16:/*RCS_E16_AccessRestricted*/
		LOG_ERROR_APP("[801-(DeleteService)]: RCS_E16_AccessRestricted");
		return true;
	case C801_E65:/*RCM_E65_EntryNotFound*/
		LOG_ERROR_APP("[801-(DeleteService)]: RCM_E65_EntryNotFound");
		return true;
	case C801_E66:/*RCM_E66_InvalidReasonCode*/
		LOG_ERROR_APP("[801-(DeleteService)]: RCM_E66_InvalidReasonCode");
		return true;
	case C801_E67:/*RCM_E67_ReasonCodeRejected*/
		LOG_ERROR_APP("[801-(DeleteService)]: RCM_E67_ReasonCodeRejected");
		return true;
	case C801_E68:/*RCM_E68_DeleteNotAllowed*/
		LOG_ERROR_APP("[801-(DeleteService)]: RCM_E68_DeleteNotAllowed");
		return true;
	default:
		LOG_ERROR_APP("[801-(DeleteService)]: invalid gateway error = " << p_nWhErrCode);
		return true;
	}

	return false;
}
bool SaveRespErr::IsErr_802(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[802-(ReadRouteList)]: hostError=" << p_nHostErrCode);
		return true;
	}

	//wh error codes
	switch(p_nWhErrCode)
	{
	case C802_NOO: /*= RCS_N00_Success*/
		break;
	case C802_E02: /*= RCS_E02_InvalidSelection*/
		LOG_ERROR_APP("[802-(ReadRouteList)]: RCS_E02_InvalidSelection");
		return true;
	case C802_E05: /*= RCS_E05_TooFewDataBytesReceived*/
		LOG_ERROR_APP("[802-(ReadRouteList)]: RCS_E05_TooFewDataBytesReceived");
		return true;
	case C802_W08: /*= RCM_W08_SetToNearestPossibleValue (maximum no. of routes to be requested)*/
		LOG_WARN_APP("[802-(ReadRouteList)]: RCM_W08_SetToNearestPossibleValue");
		break;
	default:
		LOG_ERROR_APP("[802-(ReadRouteList)]: invalid gateway error = " << p_nWhErrCode);
		return true;
	}

	return false;
}
bool SaveRespErr::IsErr_803(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[803-(ReadRouteList)]: hostError=" << p_nHostErrCode);
		return true;
	}

	//wh error codes
	switch(p_nWhErrCode)
	{
	case C803_NOO: /*= RCS_N00_Success*/
		break;
	case C803_E05: /*= RCS_E05_TooFewDataBytesReceived*/
		LOG_ERROR_APP("[803-(ReadRouteList)]: RCS_E05_TooFewDataBytesReceived");
		return true;
	case C803_E65: /*= RCM_E65_EntryNotFound*/
		LOG_ERROR_APP("[803-(ReadRouteList)]: RCM_E65_EntryNotFound");
		return true;
	default:
		LOG_ERROR_APP("[803-(ReadRouteList)]: invalid gateway error = " << p_nWhErrCode);
		return true;
	}

	return false;
}
bool SaveRespErr::IsErr_814(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[814-(ReadDeviceListEntries)]: hostError=" << p_nHostErrCode);
		return true;
	}

	//wh error codes
	switch(p_nWhErrCode)
	{
	case C814_NOO: /*= RCS_N00_Success*/
		break;
	case C814_E02: /*= RCS_E02_InvalidSelection*/
		LOG_ERROR_APP("[814-(ReadDeviceListEntries)]: RCS_E02_InvalidSelection");
		return true;
	case C814_E05: /*= RCS_E05_TooFewDataBytesReceived*/
		LOG_ERROR_APP("[814-(ReadDeviceListEntries)]: RCS_E05_TooFewDataBytesReceived");
		return true;
	case C814_W08: /*= RCM_W08_SetToNearestPossibleValue (maximum no. of IDs to be requested)*/
		LOG_WARN_APP("[814-(ReadDeviceListEntries)]: RCM_W08_SetToNearestPossibleValue");
		break;
	default:
		LOG_ERROR_APP("[814-(ReadDeviceListEntries)]: invalid gateway error = " << p_nWhErrCode);
		return true;
	}

	return false;
}
bool SaveRespErr::IsErr_818(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[818-(WriteChannelBlackList)]: hostError=" << p_nHostErrCode);
		return true;
	}

	//wh error codes
	switch(p_nWhErrCode)
	{
	case C818_NOO /*= RCS_N00_Success*/:
		break;
	case C818_E03 /*= RCS_E03_PassedParameterTooLarge*/:
		LOG_ERROR_APP("[818-(WriteChannelBlackList)]: RCS_E03_PassedParameterTooLarge");
		return true;
	case C818_E05 /*= RCS_E05_TooFewDataBytesReceived*/:
		LOG_ERROR_APP("[818-(WriteChannelBlackList)]: RCS_E05_TooFewDataBytesReceived");
		return true;
	case C818_E06 /*= RCS_E06_DeviceSpecificCommandError*/:
		LOG_ERROR_APP("[818-(WriteChannelBlackList)]: RCS_E06_DeviceSpecificCommandError");
		return true;
	case C818_E16 /*= RCS_E16_AccessRestricted*/:
		LOG_ERROR_APP("[818-(WriteChannelBlackList)]: RCS_E16_AccessRestricted");
		return true;
	case C818_E65 /*= RCM_E65_IllegalFrequencyChannelBits*/:
		LOG_ERROR_APP("[818-(WriteChannelBlackList)]: RCM_E65_IllegalFrequencyChannelBits");
		return true;
	default:
		LOG_ERROR_APP("[818-(WriteChannelBlackList)]: invalid gateway error =" << p_nWhErrCode);	
		return true;
	}

	return false;
}
bool SaveRespErr::IsErr_832(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[832-(ReadNetDeviceID)]: hostError=" << p_nHostErrCode);
		return true;
	}

	//wh error codes
	switch(p_nWhErrCode)
	{
	case C832_ResponseCode_Success /*= 0*/:
		break;
	case C832_ResponseCode_InvalidSelection /*= 2*/:
		LOG_ERROR_APP("[832-(ReadNetDeviceID)]: C832_ResponseCode_InvalidSelection");
		return true;
	case C832_ResponseCode_TooFewBytes /*= 5*/:
		LOG_ERROR_APP("[832-(ReadNetDeviceID)]: C832_ResponseCode_TooFewBytes");
		return true;
	default:
		LOG_ERROR_APP("[832-(ReadNetDeviceID)]: invalid gateway error =" << p_nWhErrCode);	
		return true;
	}

	return false;
}
bool SaveRespErr::IsErr_833(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[833-(ReadDevice'sNeighbHealth)]: hostError=" << p_nHostErrCode);
		return true;
	}
	
	//wh error codes
	switch(p_nWhErrCode)
	{
	case C833_ResponseCode_Success /*= 0*/:
		break;
	case C833_ResponseCode_InvalidSelection /*= 2*/:
		LOG_ERROR_APP("[833-(ReadDevice'sNeighbHealth)]: C833_ResponseCode_InvalidSelection");	
		return true;
	case C833_ResponseCode_TooFewBytes /*= 5*/:
		LOG_ERROR_APP("[833-(ReadDevice'sNeighbHealth)]: C833_ResponseCode_InvalidSelection");	
		return true;
	case C833_W08 /*= RCM_W08_SetToNearestPossibleValue*/:
		LOG_WARN_APP("[833-(ReadDevice'sNeighbHealth)]: RCM_W08_SetToNearestPossibleValue");
		break;
	case C833_E65 /*= RCM_E65_InvalidNeighborTableIndex*/:
		LOG_ERROR_APP("[833-(ReadDevice'sNeighbHealth)]: RCM_E65_InvalidNeighborTableIndex");	
		return true;
	default:
		LOG_ERROR_APP("[833-(ReadDevice'sNeighbHealth)]: invalid gateway error =" << p_nWhErrCode);	
		return true;
	}

	return false;
}
bool SaveRespErr::IsErr_834(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[834-(ReadNetTopologyInfo)]: hostError=" << p_nHostErrCode);
		return true;
	}
	
	//wh error codes
	switch(p_nWhErrCode)
	{
	case C834_ResponseCode_Success /*= 0*/:
		break;
	case C834_ResponseCode_InvalidSelection /*= 2*/:
		LOG_ERROR_APP("[834-(ReadNetTopologyInfo)]: C834_ResponseCode_InvalidSelection");	
		return true;
	case C834_ResponseCode_TooFewBytes /*= 5*/:
		LOG_ERROR_APP("[834-(ReadNetTopologyInfo)]: C834_ResponseCode_TooFewBytes");	
		return true;
	case C834_E65 /*= RCM_E65_EntryNotFound*/:
		LOG_ERROR_APP("[834-(ReadNetTopologyInfo)]: RCM_E65_EntryNotFound");	
		return true;
	default:
		LOG_ERROR_APP("[834-(ReadNetTopologyInfo)]: invalid gateway error =" << p_nWhErrCode);	
		return true;
	}

	return false;
}
bool SaveRespErr::IsErr_837(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[837-(WriteUpdateNotifcation)]: hostError=" << p_nHostErrCode);
		return true;
	}
	
	//wh error codes
	switch(p_nWhErrCode)
	{
	case C837_NOO:// RCS_N00_Success,
		break;
	case C837_E05:// RCS_E05_TooFewDataBytesReceived,
		LOG_ERROR_APP("[837-(WriteUpdateNotifcation)]: RCS_E05_TooFewDataBytesReceived");	
		return true;
	case C837_E16:// RCS_E16_AccessRestricted,
		LOG_ERROR_APP("[837-(WriteUpdateNotifcation)]: RCS_E16_AccessRestricted");	
		return true;
	case C837_E65:// RCM_E65_UnknownUID,
		LOG_ERROR_APP("[837-(WriteUpdateNotifcation)]: RCM_E65_UnknownUID");	
		return true;
	case C837_E66:// RCM_E66_UnknownNotificationFlag
		LOG_ERROR_APP("[837-(WriteUpdateNotifcation)]: RCM_E66_UnknownNotificationFlag");	
		return true;
	default:
		LOG_ERROR_APP("[837-(WriteUpdateNotifcation)]: invalid gateway error =" << p_nWhErrCode);	
		return true;
	}

	return false;
}
bool SaveRespErr::IsErr_839(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[839-(ChangeNotifcation)]: hostError=" << p_nHostErrCode);
		return true;
	}
	
	//wh error codes
	switch(p_nWhErrCode)
	{
	case C839_N00: // RCS_N00_Success
		break;
	case C839_E05: // RCS_E05_TooFewDataBytesReceived
		LOG_ERROR_APP("[839-(ChangeNotifcation)]: RCS_E05_TooFewDataBytesReceived");	
		return true;
	case C839_E65: // RCM_E65_UnknownUID
		LOG_ERROR_APP("[839-(ChangeNotifcation)]: RCM_E65_UnknownUID");	
		return true;
	default:
		LOG_ERROR_APP("[839-(ChangeNotifcation)]: invalid gateway error =" << p_nWhErrCode);	
		return true;
	}

	return false;
}
bool SaveRespErr::IsErr_840(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[840-(ReadDeviceStatistics)]: hostError=" << p_nHostErrCode);
		return true;
	}

	//wh error codes
	switch(p_nWhErrCode)
	{
	case C840_ResponseCode_Success: // RCS_N00_Success
		break;
	case C840_E02: //RCS_E02_InvalidSelection
		LOG_ERROR_APP("[840-(ReadDeviceStatistics)]: RCS_E02_InvalidSelection");	
		return true;
	case C840_E05: // RCS_E05_TooFewDataBytesReceived
		LOG_ERROR_APP("[840-(ReadDeviceStatistics)]: RCS_E05_TooFewDataBytesReceived");
		return true;
	default:
		LOG_ERROR_APP("[840-(ReadDeviceStatistics)]: invalid gateway error =" << p_nWhErrCode);			
		return true;
	}

	return false;
}
bool SaveRespErr::IsErr_965(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[965-(WriteSuperFrame)]: hostError=" << p_nHostErrCode);
		return true;
	}
	
	//wh error codes
	switch(p_nWhErrCode)
	{
	case C965_NOO /*= RCS_N00_Success*/:
		break;
	case C965_E05 /*= RCS_E05_TooFewDataBytesReceived*/:
		LOG_ERROR_APP("[965-(WriteSuperFrame)]: RCS_E05_TooFewDataBytesReceived");	
		return true;
	case C965_E16/* = RCS_E16_AccessRestricted*/:
		LOG_ERROR_APP("[965-(WriteSuperFrame)]: RCS_E16_AccessRestricted");	
		return true;
	case C965_E65 /*= RCM_E65_NoMoreEntriesAvailable*/:
		LOG_ERROR_APP("[965-(WriteSuperFrame)]: RCM_E65_NoMoreEntriesAvailable");	
		return true;
	case C965_E66 /*= RCM_E66_InvalidExecutionTime*/:
		LOG_ERROR_APP("[965-(WriteSuperFrame)]: RCM_E66_InvalidExecutionTime");	
		return true;
	case C965_E67 /*= RCM_E67_InvalidNoOfSlots*/:
		LOG_ERROR_APP("[965-(WriteSuperFrame)]: RCM_E67_InvalidNoOfSlots");	
		return true;
	case C965_E68 /*= RCM_E68_InvalidSuperframeMode*/:
		LOG_ERROR_APP("[965-(WriteSuperFrame)]: RCM_E68_InvalidSuperframeMode");	
		return true;
	default:
		LOG_ERROR_APP("[965-(WriteSuperFrame)]: invalid gateway error =" << p_nWhErrCode);	
		return true;
	}

	return false;
}
bool SaveRespErr::IsErr_966(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[966-(DeleteSuperFrame)]: hostError=" << p_nHostErrCode);
		return true;
	}

	//wh error codes
	switch(p_nWhErrCode)
	{
	case C966_NOO /*= RCS_N00_Success*/:
		break;
	case C966_E05 /*= RCS_E05_TooFewDataBytesReceived*/:
		LOG_ERROR_APP("[966-(DeleteSuperFrame)]: RCS_E05_TooFewDataBytesReceived");	
		return true;
	case C966_E16 /*= RCS_E16_AccessRestricted*/:
		LOG_ERROR_APP("[966-(DeleteSuperFrame)]: RCS_E16_AccessRestricted");	
		return true;
	case C966_E65 /*= RCM_E65_EntryNotFound*/:
		LOG_ERROR_APP("[966-(DeleteSuperFrame)]: RCM_E65_EntryNotFound");	
		return true;
	default:
		LOG_ERROR_APP("[966-(DeleteSuperFrame)]: invalid gateway error =" << p_nWhErrCode);	
		return true;
	}

	return false;
}
bool SaveRespErr::IsErr_967(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[967-(WriteLink)]: hostError=" << p_nHostErrCode);
		return true;
	}

	//wh error codes
	switch(p_nWhErrCode)
	{
	case C967_NOO /*= RCS_N00_Success*/:
		break;
	case C967_E05 /*= RCS_E05_TooFewDataBytesReceived*/:
		LOG_ERROR_APP("[967-(WriteLink)]: RCS_E05_TooFewDataBytesReceived");	
		return true;
	case C967_E16 /*= RCS_E16_AccessRestricted*/:
		LOG_ERROR_APP("[967-(WriteLink)]: RCS_E05_TooFewDataBytesReceived");	
		return true;
	case C967_E65 /* = RCM_E65_NoMoreEntriesAvailable*/:
		LOG_ERROR_APP("[967-(WriteLink)]: RCM_E65_NoMoreEntriesAvailable");	
		return true;
	case C967_E66 /*= RCM_E66_LinkAlreadyExists*/:
		LOG_ERROR_APP("[967-(WriteLink)]: RCM_E66_LinkAlreadyExists");	
		return true;
	case C967_E67 /*= RCM_E67_UnknownSuperframeID*/:
		LOG_ERROR_APP("[967-(WriteLink)]: RCM_E67_UnknownSuperframeID");	
		return true;
	case C967_E68 /*= RCM_E68_InvalidSlotNumber*/:
		LOG_ERROR_APP("[967-(WriteLink)]: RCM_E68_InvalidSlotNumber");	
		return true;
	case C967_E69 /*= RCM_E69_InvalidLinkOptions*/:
		LOG_ERROR_APP("[967-(WriteLink)]: RCM_E69_InvalidLinkOptions");	
		return true;
	case C967_E70 /*= RCM_E70_InvalidChannelOffset*/:
		LOG_ERROR_APP("[967-(WriteLink)]: RCM_E70_InvalidChannelOffset");	
		return true;
	case C967_E71 /*= RCM_E71_InvalidLinkType*/:
		LOG_ERROR_APP("[967-(WriteLink)]: RCM_E71_InvalidLinkType");	
		return true;
	case C967_E72 /*= RCM_E72_NoMoreNeighborsAvailable*/:
		LOG_ERROR_APP("[967-(WriteLink)]: RCM_E72_NoMoreNeighborsAvailable");	
		return true;
	default:
		LOG_ERROR_APP("[967-(WriteLink)]: invalid gateway error =" << p_nWhErrCode);	
		return true;
	}

	return false;
}
bool SaveRespErr::IsErr_968(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[968-(DeleteLink)]: hostError=" << p_nHostErrCode);
		return true;
	}

	//wh error codes
	switch(p_nWhErrCode)
	{
	case C968_NOO /*= RCS_N00_Success*/:
		break;
	case C968_E05 /*= RCS_E05_TooFewDataBytesReceived*/:
		LOG_ERROR_APP("[968-(DeleteLink)]: RCS_E05_TooFewDataBytesReceived");	
		return true;
	case C968_E16 /*= RCS_E16_AccessRestricted*/:
		LOG_ERROR_APP("[968-(DeleteLink)]: RCS_E16_AccessRestricted");	
		return true;
	case C968_E65 /*= RCM_E65_LinkNotFound*/:
		LOG_ERROR_APP("[968-(DeleteLink)]: RCM_E65_LinkNotFound");	
		return true;
	default:
		LOG_ERROR_APP("[968-(DeleteLink)]: invalid gateway error =" << p_nWhErrCode);	
		return true;
	}

	return false;
}
bool SaveRespErr::IsErr_969(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[969-(WriteGraph)]: hostError=" << p_nHostErrCode);
		return true;
	}

	//wh error codes
	switch(p_nWhErrCode)
	{
	case C969_NOO /*= RCS_N00_Success*/:
		break;
	case C969_E05 /*= RCS_E05_TooFewDataBytesReceived*/:
		LOG_ERROR_APP("[969-(WriteGraph)]: RCS_E05_TooFewDataBytesReceived");	
		return true;
	case C969_E16 /*= RCS_E16_AccessRestricted*/:
		LOG_ERROR_APP("[969-(WriteGraph)]: RCS_E16_AccessRestricted");	
		return true;
	case C969_E65 /*= RCM_E65_NoMoreEntriesAvailable*/:
		LOG_ERROR_APP("[969-(WriteGraph)]: RCM_E65_NoMoreEntriesAvailable");	
		return true;
	case C969_E66 /*= RCM_E66_UnknownNickname*/:
		LOG_ERROR_APP("[969-(WriteGraph)]: RCM_E66_UnknownNickname");	
		return true;
	case C969_E67 /*= RCM_E67_CantAddEdgeWhenSuperframeUsedAsGraphEquiv*/:
		LOG_ERROR_APP("[969-(WriteGraph)]: RCM_E67_CantAddEdgeWhenSuperframeUsedAsGraphEquiv");	
		return true;
	default:
		LOG_ERROR_APP("[969-(WriteGraph)]: invalid gateway error =" << p_nWhErrCode);	
		return true;
	}

	return false;
}
bool SaveRespErr::IsErr_970(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[970-(DeleteGraph)]: hostError=" << p_nHostErrCode);
		return true;
	}

	//wh error codes
	switch(p_nWhErrCode)
	{
	case C970_NOO /*= RCS_N00_Success*/:
		break;
	case C970_E05 /*= RCS_E05_TooFewDataBytesReceived*/:
		LOG_ERROR_APP("[970-(DeleteGraph)]: RCS_E05_TooFewDataBytesReceived");	
		return true;
	case C970_E16 /*= RCS_E16_AccessRestricted*/:
		LOG_ERROR_APP("[970-(DeleteGraph)]: RCS_E16_AccessRestricted");	
		return true;
	case C970_E65 /*= RCM_E65_EntryNotFound*/:
		LOG_ERROR_APP("[970-(DeleteGraph)]: RCM_E65_EntryNotFound");	
		return true;
	default:
		LOG_ERROR_APP("[970-(DeleteGraph)]: invalid gateway error =" << p_nWhErrCode);	
		return true;
	}
	return false;
}
bool SaveRespErr::IsErr_971(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[971-(WriteNeighbor)]: hostError=" << p_nHostErrCode);
		return true;
	}

	//wh error codes
	switch(p_nWhErrCode)
	{
	case C971_NOO /*= RCS_N00_Success*/:
		break;
	case C971_E05 /*= RCS_E05_TooFewDataBytesReceived*/:
		LOG_ERROR_APP("[971-(WriteNeighbor)]: RCS_E05_TooFewDataBytesReceived");	
		return true;
	case C971_E16 /*= RCS_E16_AccessRestricted*/:
		LOG_ERROR_APP("[971-(WriteNeighbor)]: RCS_E16_AccessRestricted");	
		return true;
	case C971_E65 /*= RCM_E65_UnknownNickname*/:
		LOG_ERROR_APP("[971-(WriteNeighbor)]: RCM_E65_UnknownNickname");	
		return true;
	case C971_E66 /*= RCM_E66_InvalidNeighborProperty*/:
		LOG_ERROR_APP("[971-(WriteNeighbor)]: RCM_E66_InvalidNeighborProperty");	
		return true;
	default:
		LOG_ERROR_APP("[971-(WriteNeighbor)]: invalid gateway error =" << p_nWhErrCode);
		return true;
	}
	return false;
}
bool SaveRespErr::IsErr_973(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[973-(Write Service)]: hostError=" << p_nHostErrCode);
		return true;
	}
	switch(p_nWhErrCode)
	{
	case C973_NOO:/**/
		break;
	case C973_E05: /*RCS_E05_TooFewDataBytesReceived*/
		LOG_ERROR_APP("[973-(Write Service)]: RCS_E05_TooFewDataBytesReceived");	
		return true;
	case C973_E16: /*RCS_E16_AccessRestricted*/
		LOG_ERROR_APP("[973-(Write Service)]: RCS_E16_AccessRestrictedd");	
		return true;
	case C973_E65: /*RCM_E65_NoMoreEntriesAvailable*/
		LOG_ERROR_APP("[973-(Write Service)]: RCM_E65_NoMoreEntriesAvailable");	
		return true;
	case C973_E66: /*RCM_E66_InvalidServiceID*/
		LOG_ERROR_APP("[973-(Write Service)]: RCM_E66_InvalidServiceID");	
		return true;
	case C973_E67: /*RCM_E67_OpenTransactionPending*/
		LOG_ERROR_APP("[973-(Write Service)]: RCM_E67_OpenTransactionPending");	
		return true;
	case C973_E68: /*RCM_E68_InvalidApplicationDomain*/
		LOG_ERROR_APP("[973-(Write Service)]: RCM_E68_InvalidApplicationDomain");	
		return true;
	case C973_E69: /*RCM_E69_UnknownCorrespNickname*/
		LOG_ERROR_APP("[973-(Write Service)]: RCM_E69_UnknownCorrespNickname");	
		return true;
	case C973_E70: /*RCM_E70_InvalidRouteID*/
		LOG_ERROR_APP("[973-(Write Service)]: RCM_E70_InvalidRouteID");	
		return true;
	case C973_E71: /*RCM_E71_CorrespNicknameAndRouteCorrespMismatch*/
		LOG_ERROR_APP("[973-(Write Service)]: RCM_E71_CorrespNicknameAndRouteCorrespMismatch");	
		return true;
	default:
		LOG_ERROR_APP("[973-(Write Service)]: invalid gateway error =" << p_nWhErrCode);	
		return true;
	}

	return false;
}
bool SaveRespErr::IsErr_974(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[974-(Write Route)]: hostError=" << p_nHostErrCode);
		return true;
	}
	switch(p_nWhErrCode)
	{
	case C974_NOO:/**/
		break;
	case C974_E05 /*= RCS_E05_TooFewDataBytesReceived*/:
		LOG_ERROR_APP("[974-(Write Route)]: RCS_E05_TooFewDataBytesReceived");	
		return true;
	case C974_E16 /*= RCS_E16_AccessRestricted*/:
		LOG_ERROR_APP("[974-(Write Route)]: RCS_E16_AccessRestricted");	
		return true;
	case C974_E65 /*= RCM_E65_NoMoreEntriesAvailable*/:
		LOG_ERROR_APP("[974-(Write Route)]: RCM_E65_NoMoreEntriesAvailable");	
		return true;
	case C974_E66 /*= RCM_E66_InvalidNickname*/:
		LOG_ERROR_APP("[974-(Write Route)]: RCM_E66_InvalidNickname");	
		return true;
	case C974_E67 /*= RCM_E67_InvalidGraphID*/:
		LOG_ERROR_APP("[974-(Write Route)]: RCM_E67_InvalidGraphID");	
		return true;
	default:
		LOG_ERROR_APP("[974-(Write Route)]: invalid gateway error =" << p_nWhErrCode);	
		return true;
	}

	return false;
}
bool SaveRespErr::IsErr_975(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[975-(Delete Route)]: hostError=" << p_nHostErrCode);
		return true;
	}
	switch(p_nWhErrCode)
	{
	case C975_NOO:/**/
		break;
	case C975_E05 /*= RCS_E05_TooFewDataBytesReceived*/:
		LOG_ERROR_APP("[975-(Delete Route)]: RCS_E05_TooFewDataBytesReceived");	
		return true;
	case C975_E16 /*= RCS_E16_AccessRestricted*/:
		LOG_ERROR_APP("[975-(Delete Route)]: RCS_E16_AccessRestricted");	
		return true;
	case C975_E65 /*= RCM_E65_InvalidRouteID*/:
		LOG_ERROR_APP("[975-(Delete Route)]: RCM_E65_InvalidRouteID");	
		return true;
	default:
		LOG_ERROR_APP("[975-(Delete Route)]: invalid gateway error =" << p_nWhErrCode);	
		return true;
	}

	return false;
}
bool SaveRespErr::IsErr_976(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[976-(Write Source Route)]: hostError=" << p_nHostErrCode);
		return true;
	}
	switch(p_nWhErrCode)
	{
	case C976_NOO:/**/
		break;
	case C976_E05 /*= RCS_E05_TooFewDataBytesReceived*/:
		LOG_ERROR_APP("[976-(Write Source Route)]: RCS_E05_TooFewDataBytesReceived");	
		return true;
	case C976_W08 /*= RCM_W08_BroadcastAddrDeletedInResp*/:
		LOG_ERROR_APP("[976-(Write Source Route)]: RCM_W08_BroadcastAddrDeletedInResp");	
		return true;
	case C976_E16 /*= RCS_E16_AccessRestricted*/:
		LOG_ERROR_APP("[976-(Write Source Route)]: RCS_E16_AccessRestricted");	
		return true;
	case C976_E65 /*= RCM_E65_NoMoreEntriesAvailable*/:
		LOG_ERROR_APP("[976-(Write Source Route)]: RCM_E65_NoMoreEntriesAvailable");	
		return true;
	case C976_E66 /*= RCM_E66_InvalidRouteID*/:
		LOG_ERROR_APP("[976-(Write Source Route)]: RCM_E66_InvalidRouteID");	
		return true;
	case C976_E67 /*= RCM_E67_InvalidNickname*/:
		LOG_ERROR_APP("[976-(Write Source Route)]: RCM_E67_InvalidNickname");	
		return true;
	case C976_E68 /*= RCM_E68_InvalidNoOfHops*/:
		LOG_ERROR_APP("[976-(Write Source Route)]: RCM_E68_InvalidNoOfHops");	
		return true;
	default:
		LOG_ERROR_APP("[976-(Write Source Route)]: invalid gateway error =" << p_nWhErrCode);	
		return true;
	}

	return false;
}
bool SaveRespErr::IsErr_977(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[977-(Delete Source Route)]: hostError=" << p_nHostErrCode);
		return true;
	}
	switch(p_nWhErrCode)
	{
	case C977_NOO:/**/
		break;
	case C977_E05 /*= RCS_E05_TooFewDataBytesReceived*/:
		LOG_ERROR_APP("[977-(Delete Source Route)]: RCS_E05_TooFewDataBytesReceived");	
		return true;
	case C977_E16 /*= RCS_E16_AccessRestricted*/:
		LOG_ERROR_APP("[977-(Delete Source Route)]: RCS_E16_AccessRestricted");	
		return true;
	case C977_E66 /*= RCM_E66_InvalidRouteID*/:
		LOG_ERROR_APP("[977-(Delete Source Route)]: RCM_E66_InvalidRouteID");	
		return true;
	default:
		LOG_ERROR_APP("[977-(Delete Source Route)]: invalid gateway error =" << p_nWhErrCode);	
		return true;
	}

	return false;
}

// universal
bool SaveRespErr::IsErr_000(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[000-(ReadUniqueId)]: hostError=" << p_nHostErrCode);
		return true;
	}

	//wh error codes
	switch(p_nWhErrCode)
	{
	case C000_N00: /*= RCS_N00_Success*/
		break;
	default:
		LOG_ERROR_APP("[000-(ReadUniqueId)]: invalid gateway error =" << p_nWhErrCode);
		return true;
	}

	return false;
}
bool SaveRespErr::IsErr_001(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[001-(ReadPrimaryValue)]: hostError=" << p_nHostErrCode);
		return true;
	}

	//wh error codes
	switch(p_nWhErrCode)
	{
	case C001_N00: // = RCS_N00_Success,
		break;
	case C001_E06: // = RCS_E06_DeviceSpecificCommandError,
		LOG_ERROR_APP("[001-(ReadPrimaryValue)]: RCS_E06_DeviceSpecificCommandError");	
		return true;
	case C001_W08: // = RCM_W08_UpdateFailure,
		LOG_ERROR_APP("[001-(ReadPrimaryValue)]: RCM_W08_UpdateFailure");	
		return true;
	case C001_E16: // = RCS_E16_AccessRestricted
		LOG_ERROR_APP("[001-(ReadPrimaryValue)]: RCS_E16_AccessRestricted");	
		return true;
	default:
		LOG_ERROR_APP("[001-(ReadPrimaryValue)]: invalid gateway error =" << p_nWhErrCode);
		return true;
	}

	return false;
}
bool SaveRespErr::IsErr_002(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[002-(ReadLoopCurrent)]: hostError=" << p_nHostErrCode);
		return true;
	}

	//wh error codes
	switch(p_nWhErrCode)
	{
	case C002_N00:// = RCS_N00_Success,
		break;
	case C002_E06:// = RCS_E06_DeviceSpecificCommandError,
		LOG_ERROR_APP("[002-(ReadLoopCurrent)]: RCS_E06_DeviceSpecificCommandError");	
		return true;
	case C002_W08:// = RCM_W08_UpdateFailure,
		LOG_ERROR_APP("[002-(ReadLoopCurrent)]: RCM_W08_UpdateFailure");	
		return true;
	case C002_E16:// = RCS_E16_AccessRestricted
		LOG_ERROR_APP("[002-(ReadLoopCurrent)]: RCS_E16_AccessRestricted");	
		return true;
	default:
		LOG_ERROR_APP("[002-(ReadLoopCurrent)]: invalid gateway error =" << p_nWhErrCode);
		return true;

	}
	return false;
}
bool SaveRespErr::IsErr_003(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[003-(ReadDynamicVar)]: hostError=" << p_nHostErrCode);
		return true;
	}

	//wh error codes
	switch(p_nWhErrCode)
	{
	case C003_N00:// = RCS_N00_Success,
		break;
	case C003_E06:// = RCS_E06_DeviceSpecificCommandError,
		LOG_ERROR_APP("[003-(ReadDynamicVar)]: RCS_E06_DeviceSpecificCommandError");	
		return true;
	case C003_W08:// = RCM_W08_UpdateFailure,
		LOG_ERROR_APP("[003-(ReadDynamicVar)]: RCM_W08_UpdateFailure");	
		return true;
	case C003_E16:// = RCS_E16_AccessRestricted
		LOG_ERROR_APP("[003-(ReadDynamicVar)]: RCS_E16_AccessRestricted");	
		return true;
	default:
		LOG_ERROR_APP("[003-(ReadDynamicVar)]: invalid gateway error =" << p_nWhErrCode);
		return true;
	}

	return false;

}
bool SaveRespErr::IsErr_009(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[009-(ReadDeviceVariablesWithStatus)]: hostError=" << p_nHostErrCode);
		return true;
	}
	//wh error codes
	switch(p_nWhErrCode)
	{
	case C009_N00:// = RCS_N00_Success,
		break;
	case C009_E02:// = RCS_E02_InvalidSelection,
		LOG_ERROR_APP("[009-(ReadDeviceVarWithStatus)]: RCS_E02_InvalidSelection");	
		return true;
	case C009_E05:// = RCS_E05_TooFewDataBytesReceived,
		LOG_ERROR_APP("[009-(ReadDeviceVarWithStatus)]: RCS_E05_TooFewDataBytesReceived");	
		return true;
	case C009_E06:// = RCS_E06_DeviceSpecificCommandError,
		LOG_ERROR_APP("[009-(ReadDeviceVarWithStatus)]: RCS_E06_DeviceSpecificCommandError");	
		return true;
	case C009_W08:// = RCM_W08_UpdateFailure,
		LOG_ERROR_APP("[009-(ReadDeviceVarWithStatus)]: RCM_W08_UpdateFailure");	
		return true;
	case C009_W14:// = RCM_W14_DynamicVarsReturnedForDeviceVars,
		LOG_ERROR_APP("[009-(ReadDeviceVarWithStatus)]: RCM_W14_DynamicVarsReturnedForDeviceVars");	
		break;
	case C009_E16:// = RCS_E16_AccessRestricted,
		LOG_ERROR_APP("[009-(ReadDeviceVarWithStatus)]: RCS_E16_AccessRestricted");	
		return true;
	case C009_W30:// = RCM_W30_CommandResponseTruncated
		LOG_ERROR_APP("[009-(ReadDeviceVarWithStatus)]: RCM_W30_CommandResponseTruncated");	
		return true;
	default:
		LOG_ERROR_APP("[001-(ReadDeviceVarWithStatus)]: invalid gateway error =" << p_nWhErrCode);	
		return true;
	}

	return false;
}

bool SaveRespErr::IsErr_020(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[020-(WriteLongTag)]: hostError=" << p_nHostErrCode);
		return true;
	}
	//wh error codes
	/*
	C020_N00 = RCS_N00_Success,
	C020_E16 = RCS_E16_AccessRestricted,
	C020_E32 = RCS_E32_Busy
	*/
	switch(p_nWhErrCode)
	{
	case C020_N00:// = RCS_N00_Success
		break;
	case C020_E16:// = RCS_E16_AccessRestricted
		LOG_ERROR_APP("[020-(WriteLongTag)]: RCS_E16_AccessRestricted");
		break;
	case C020_E32:// = RCS_E32_Busy
		LOG_ERROR_APP("[020-(WriteLongTag)]: RCS_E32_Busy");
		break;
	default:
		return true;
	}
	return false;
}

bool SaveRespErr::IsErr_033(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[033-(ReadDeviceVariables)]: hostError=" << p_nHostErrCode);
		return true;
	}

	//wh error codes
	switch(p_nWhErrCode)
	{
	case C033_NOO:// = RCS_N00_Success,
		break;
	case C033_E02:// = RCS_E02_InvalidSelection,
		LOG_ERROR_APP("[033-(ReadDeviceVariables)]: RCS_E02_InvalidSelection");	
		return true;
	case C033_E05:// = RCS_E05_TooFewDataBytesReceived,
		LOG_ERROR_APP("[033-(ReadDeviceVariables)]: RCS_E05_TooFewDataBytesReceived");	
		return true;
	case C033_E06:// = RCS_E06_DeviceSpecificCommandError,
		LOG_ERROR_APP("[033-(ReadDeviceVariables)]: RCS_E06_DeviceSpecificCommandError");	
		return true;
	case C033_W08:// = RCM_W08_UpdateFailure,
		LOG_ERROR_APP("[033-(ReadDeviceVariables)]: RCM_W08_UpdateFailure");	
		return true;
	case C033_E16:// = RCS_E16_AccessRestricted
		LOG_ERROR_APP("[033-(ReadDeviceVariables)]: RCS_E16_AccessRestricted");	
		return true;
	default:
		LOG_ERROR_APP("[033-(ReadDeviceVariables)]: invalid gateway error =" << p_nWhErrCode);	
		return true;
	}

	return false;
}

bool SaveRespErr::IsErr_074(int p_nHostErrCode, int p_nWhErrCode)
{
    //host error codes
    if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
    {
        LOG_ERROR_APP("[074-(ReadIOSystemCapabilities)]: hostError=" << p_nHostErrCode);
        return true;
    }
    //wh error codes
    switch(p_nWhErrCode)
    {
        case C074_NOO /*= RCS_N00_Success*/:
            break;
        case C074_E06 /*= RCS_E06_DeviceSpecificCommandError*/:
            LOG_ERROR_APP("[074-(ReadIOSystemCapabilities)]: RCS_E06_DeviceSpecificCommandError");
            return true;
        case C074_E16 /*= RCS_E16_AccessRestricted*/:
            LOG_ERROR_APP("[074-(ReadIOSystemCapabilities)]: RCS_E16_AccessRestricted");
            return true;
        case C074_E32 /*= RCS_E32_Busy*/:
            LOG_ERROR_APP("[074-(ReadIOSystemCapabilities)]: RCS_E32_Busy");
            return true;
        default:
            LOG_ERROR_APP("[074-(ReadIOSystemCapabilities)]: invalid gateway error = " << p_nWhErrCode);
            return true;
    }
    return false;
}

bool SaveRespErr::IsErr_084(int p_nHostErrCode, int p_nWhErrCode)
{
    //host error codes
    if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
    {
        LOG_ERROR_APP("[084-(ReadSubDeviceIdentitySummary)]: hostError=" << p_nHostErrCode);
        return true;
    }
    //wh error codes
    switch(p_nWhErrCode)
    {
        case C084_NOO /*= RCS_N00_Success*/:
            break;
        case C084_E02 /*= RCS_E02_InvalidSelection*/:
            LOG_ERROR_APP("[084-(ReadSubDeviceIdentitySummary)]: RCS_E02_InvalidSelection");
            return true;
        case C084_E05 /*= RCS_E05_TooFewDataBytesReceived*/:
            LOG_ERROR_APP("[084-(ReadSubDeviceIdentitySummary)]: RCS_E05_TooFewDataBytesReceived");
            return true;
        case C084_E06 /*= RCS_E06_DeviceSpecificCommandError*/:
            LOG_ERROR_APP("[084-(ReadSubDeviceIdentitySummary)]: RCS_E06_DeviceSpecificCommandError");
            return true;
        default:
            LOG_ERROR_APP("[084-(ReadSubDeviceIdentitySummary)]: invalid gateway error = " << p_nWhErrCode);
            return true;
    }
    return false;
}

bool SaveRespErr::IsErr_101(int p_nHostErrCode, int p_nWhErrCode)
{
    //host error codes
    if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
    {
        LOG_ERROR_APP("[101-(ReadSubDeviceToBurstMessageMap)]: hostError=" << p_nHostErrCode);
        return true;
    }
    //wh error codes
    switch(p_nWhErrCode)
    {
        case C101_NOO /*= RCS_N00_Success*/:
            break;
        case C101_E02 /*= RCS_E02_InvalidSelection*/:
            LOG_ERROR_APP("[101-(ReadSubDeviceToBurstMessageMap)]: RCS_E02_InvalidSelection");
            return true;
        case C101_E05 /*= RCS_E05_TooFewDataBytesReceived*/:
            LOG_ERROR_APP("[101-(ReadSubDeviceToBurstMessageMap)]: RCS_E05_TooFewDataBytesReceived");
            return true;
        case C101_E06 /*= RCS_E06_DeviceSpecificCommandError*/:
            LOG_ERROR_APP("[101-(ReadSubDeviceToBurstMessageMap)]: RCS_E06_DeviceSpecificCommandError");
            return true;
        default:
            LOG_ERROR_APP("[101-(ReadSubDeviceToBurstMessageMap)]: invalid gateway error = " << p_nWhErrCode);
            return true;
    }

    return false;
}

bool SaveRespErr::IsErr_102(int p_nHostErrCode, int p_nWhErrCode)
{
    //host error codes
    if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
    {
        LOG_ERROR_APP("[102-(WriteBurstPeriod)]: hostError=" << p_nHostErrCode);
        return true;
    }
    //wh error codes
    switch(p_nWhErrCode)
    {
        case C102_NOO /*= RCS_N00_Success*/:
            break;
        case C102_E02 /*= RCS_E02_InvalidSelection*/:
            LOG_ERROR_APP("[102-(MapSubDeviceToBurstMessage)]: RCS_E02_InvalidSelection");
            return true;
        case C102_E05 /*= RCS_E05_TooFewDataBytesReceived*/:
            LOG_ERROR_APP("[102-(MapSubDeviceToBurstMessage)]: RCS_E05_TooFewDataBytesReceived");
            return true;
        case C102_E06 /*= RCS_E06_DeviceSpecificCommandError*/:
            LOG_ERROR_APP("[102-(MapSubDeviceToBurstMessage)]: RCS_E06_DeviceSpecificCommandError");
            return true;
        case C102_E07 /*= RCS_E07_InWriteProtectMode*/:
            LOG_ERROR_APP("[102-(MapSubDeviceToBurstMessage)]: RCS_E07_InWriteProtectMode");
            return true;
        case C102_E09 /*= RCM_E09_InvalidSubDeviceIndex*/:
            LOG_ERROR_APP("[102-(MapSubDeviceToBurstMessage)]: RCM_E09_InvalidSubDeviceIndex");
            return true;
        case C102_E16 /*= RCS_E16_AccessRestricted*/:
            LOG_ERROR_APP("[102-(MapSubDeviceToBurstMessage)]: RCS_E16_AccessRestricted");
            return true;
        default:
            LOG_ERROR_APP("[102-(MapSubDeviceToBurstMessage)]: invalid gateway error = " << p_nWhErrCode);
            return true;
    }

    return false;
}

bool SaveRespErr::IsErr_103(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[103-(WriteBurstPeriod)]: hostError=" << p_nHostErrCode);
		return true;
	}
	//wh error codes
	switch(p_nWhErrCode)
	{
		case C103_NOO /*= RCS_N00_Success*/:
			break;
		case C103_E05 /*= RCS_E05_TooFewDataBytesReceived*/:
			LOG_ERROR_APP("[103-(WriteBurstPeriod)]: RCS_E05_TooFewDataBytesReceived");
			return true;
		case C103_E06 /*= RCS_E06_DeviceSpecificCommandError*/:
			LOG_ERROR_APP("[103-(WriteBurstPeriod)]: RCS_E06_DeviceSpecificCommandError");
			return true;
		case C103_E07 /*= RCS_E07_InWriteProtectMode*/:
			LOG_ERROR_APP("[103-(WriteBurstPeriod)]: RCS_E07_InWriteProtectMode");
			return true;
		case C103_W08 /*= RCM_W08_UpdateTimesAdjusted*/:
			LOG_WARN_APP("[103-(WriteBurstPeriod)]: RCM_W08_UpdateTimesAdjusted");
			break;
		case C103_E09 /*= RCM_E09_InvalidBurstMessage*/:
			LOG_ERROR_APP("[103-(WriteBurstPeriod)]: RCM_E09_InvalidBurstMessage");
			return true;
		case C103_E16 /*= RCS_E16_AccessRestricted*/:
			LOG_ERROR_APP("[103-(WriteBurstPeriod)]: RCS_E16_AccessRestricted");
			return true;
		case C103_E32 /*= RCS_E32_Busy*/:
			LOG_ERROR_APP("[103-(WriteBurstPeriod)]: RCS_E32_Busy");
			return true;
		case C103_E33 /*= RCS_E33_DelayedResponseInitiated*/:
			LOG_ERROR_APP("[103-(WriteBurstPeriod)]: RCS_E33_DelayedResponseInitiated");
			return true;
		case C103_E34 /*= RCS_E34_DelayedResponseRunning*/:
			LOG_ERROR_APP("[103-(WriteBurstPeriod)]: RCS_E34_DelayedResponseRunning");
			return true;
		case C103_E35 /*= RCS_E35_DelayedResponseDead*/:
			LOG_ERROR_APP("[103-(WriteBurstPeriod)]: RCS_E35_DelayedResponseDead");
			return true;
		case C103_E36 /*= RCS_E36_DelayedResponseConflict*/:
			LOG_ERROR_APP("[103-(WriteBurstPeriod)]: RCS_E36_DelayedResponseConflict");
			return true;
		default:
			LOG_ERROR_APP("[103-(WriteBurstPeriod)]: invalid gateway error = " << p_nWhErrCode);
			return true;
	}

	return false;
}

bool SaveRespErr::IsErr_104(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[104-(WriteBurstTrigger)]: hostError=" << p_nHostErrCode);
		return true;
	}
	//wh error codes
	switch(p_nWhErrCode)
	{
		case C104_NOO /*= RCS_N00_Success*/:
			break;
		case C104_E02 /*= RCS_E02_InvalidSelection*/:
			LOG_ERROR_APP("[104-(WriteBurstTrigger)]: RCS_E02_InvalidSelection");
			return true;
		case C104_E03 /*= RCS_E03_PassedParameterTooLarge,*/:
			LOG_ERROR_APP("[104-(WriteBurstTrigger)]: RCS_E03_PassedParameterTooLarge");
			return true;
		case C104_E04 /*= RCS_E04_PassedParameterTooSmall*/:
			LOG_ERROR_APP("[104-(WriteBurstTrigger)]: RCS_E04_PassedParameterTooSmall");
			return true;
		case C104_E05 /*= RCS_E05_TooFewDataBytesReceived*/:
			LOG_ERROR_APP("[104-(WriteBurstTrigger)]: RCS_E05_TooFewDataBytesReceived");
			return true;
		case C104_E06 /*= RCS_E06_DeviceSpecificCommandError*/:
			LOG_ERROR_APP("[104-(WriteBurstTrigger)]: RCS_E06_DeviceSpecificCommandError");
			return true;
		case C104_E07 /*= RCS_E07_InWriteProtectMode*/:
			LOG_ERROR_APP("[104-(WriteBurstTrigger)]: RCS_E07_InWriteProtectMode");
			return true;
		case C104_E09 /*= RCM_E09_InvalidBurstMessage*/:
			LOG_ERROR_APP("[104-(WriteBurstTrigger)]: RCM_E09_InvalidBurstMessage");
			return true;
		case C104_E11 /*= RCM_E11_InvalidDeviceVariableClassification*/:
			LOG_ERROR_APP("[104-(WriteBurstTrigger)]: RCM_E11_InvalidDeviceVariableClassification");
			return true;
		case C104_E12 /*= RCM_E12_InvalidUnitsCode*/:
			LOG_ERROR_APP("[104-(WriteBurstTrigger)]: RCM_E12_InvalidUnitsCode");
			return true;
		case C104_E13 /*= RCM_E13_InvalidBurstTriggerModeSelectionCode*/:
			LOG_ERROR_APP("[104-(WriteBurstTrigger)]: RCM_E13_InvalidBurstTriggerModeSelectionCode");
			return true;
		case C104_E16 /*= RCS_E16_AccessRestricted*/:
			LOG_ERROR_APP("[104-(WriteBurstTrigger)]: RCS_E16_AccessRestricted");
			return true;
		case C104_E32 /*= RCS_E32_Busy*/:
			LOG_ERROR_APP("[104-(WriteBurstTrigger)]: RCS_E32_Busy");
			return true;
		case C104_E33 /*= RCS_E33_DelayedResponseInitiated*/:
			LOG_ERROR_APP("[104-(WriteBurstTrigger)]: RCS_E33_DelayedResponseInitiated");
			return true;
		case C104_E34 /*= RCS_E34_DelayedResponseRunning*/:
			LOG_ERROR_APP("[104-(WriteBurstTrigger)]: RCS_E34_DelayedResponseRunning");
			return true;
		case C104_E35 /*= RCS_E35_DelayedResponseDead*/:
			LOG_ERROR_APP("[104-(WriteBurstTrigger)]: RCS_E35_DelayedResponseDead");
			return true;
		case C104_E36 /*= RCS_E36_DelayedResponseConflict*/:
			LOG_ERROR_APP("[104-(WriteBurstTrigger)]: RCS_E36_DelayedResponseConflict");
			return true;
		default:
			LOG_ERROR_APP("[104-(WriteBurstTrigger)]: invalid gateway error = " << p_nWhErrCode);
			return true;
	}

	return false;
}

bool SaveRespErr::IsErr_105(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[105-(ReadBurstModeConfiguration)]: hostError=" << p_nHostErrCode);
		return true;
	}
	//wh error codes
	switch(p_nWhErrCode)
	{
	case C105_NOO /*= RCS_N00_Success*/:
		break;
	case C105_E06 /*= RCS_E06_DeviceSpecificCommandError*/:
		LOG_ERROR_APP("[105-(ReadBurstModeConfiguration)]: RCS_E06_DeviceSpecificCommandError");
		return true;
	case C105_E09 /*= RCM_E09_InvalidBurstMessage*/:
		LOG_ERROR_APP("[105-(ReadBurstModeConfiguration)]: RCM_E09_InvalidBurstMessage");
		return true;
	case C105_E32 /*= RCS_E32_Busy*/:
		LOG_ERROR_APP("[105-(ReadBurstModeConfiguration)]: RCS_E32_Busy");
		return true;
	default:
		LOG_ERROR_APP("[105-(ReadBurstModeConfiguration)]: invalid gateway error = " << p_nWhErrCode);
		return true;
	}

	return false;
}

bool SaveRespErr::IsErr_107(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[107-(WriteBurstDeviceVariables)]: hostError=" << p_nHostErrCode);
		return true;
	}
	//wh error codes
	switch(p_nWhErrCode)
	{
	case C107_NOO /*= RCS_N00_Success*/:
		break;
	case C107_E02 /*= RCS_E02_InvalidSelection*/:
		LOG_ERROR_APP("[107-(WriteBurstDeviceVariables)]: RCS_E02_InvalidSelection");
		return true;
	case C107_E05 /*= RCS_E05_TooFewDataBytesReceived*/:
		LOG_ERROR_APP("[107-(WriteBurstDeviceVariables)]: RCS_E05_TooFewDataBytesReceived");
		return true;
	case C107_E06 /*= RCS_E06_DeviceSpecificCommandError*/:
		LOG_ERROR_APP("[107-(WriteBurstDeviceVariables)]: RCS_E06_DeviceSpecificCommandError");
		return true;
	case C107_E07 /*= RCS_E07_InWriteProtectMode*/:
		LOG_ERROR_APP("[107-(WriteBurstDeviceVariables)]: RCS_E07_InWriteProtectMode");
		return true;
	case C107_W08 /*= RCM_W08_BurstConditionConflict*/:
		LOG_WARN_APP("[107-(WriteBurstDeviceVariables)]: RCM_W08_BurstConditionConflict");
		break;
	case C107_E09 /*= RCM_E09_InvalidBurstMessage*/:
		LOG_ERROR_APP("[107-(WriteBurstDeviceVariables)]: RCM_E09_InvalidBurstMessage");
		return true;
	default:
		LOG_ERROR_APP("[107-(WriteBurstDeviceVariables)]: invalid gateway error = " << p_nWhErrCode);
		return true;
	}

	return false;
}

bool SaveRespErr::IsErr_108(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[108-(WriteBurstModeCommandNumber)]: hostError=" << p_nHostErrCode);
		return true;
	}
	//wh error codes
	switch(p_nWhErrCode)
	{
	case C108_NOO /*= RCS_N00_Success*/:
		break;
	case C108_E02 /*= RCS_E02_InvalidSelection*/:
		LOG_ERROR_APP("[108-(WriteBurstModeCommandNumber)]: RCS_E02_InvalidSelection");
		return true;
	case C108_E05 /*= RCS_E05_TooFewDataBytesReceived*/:
		LOG_ERROR_APP("[108-(WriteBurstModeCommandNumber)]: RCS_E05_TooFewDataBytesReceived");
		return true;
	case C108_E06 /*= RCS_E06_DeviceSpecificCommandError*/:
		LOG_ERROR_APP("[108-(WriteBurstModeCommandNumber)]: RCS_E06_DeviceSpecificCommandError");
		return true;
	case C108_E07 /*= RCS_E07_InWriteProtectMode*/:
		LOG_ERROR_APP("[108-(WriteBurstModeCommandNumber)]: RCS_E07_InWriteProtectMode");
		return true;
	case C108_W08 /*= RCM_W08_BurstConditionConflict*/:
		LOG_WARN_APP("[108-(WriteBurstModeCommandNumber)]: RCM_W08_BurstConditionConflict");
		break;
	case C108_E09 /*= RCM_E09_InvalidBurstMessage*/:
		LOG_ERROR_APP("[108-(WriteBurstModeCommandNumber)]: RCM_E09_InvalidBurstMessage");
		return true;
	default:
		LOG_ERROR_APP("[108-(WriteBurstModeCommandNumber)]: invalid gateway error = " << p_nWhErrCode);
		return true;
	}

	return false;
}

bool SaveRespErr::IsErr_109(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		LOG_ERROR_APP("[109-(BurstModeControl)]: hostError=" << p_nHostErrCode);
		return true;
	}
	//wh error codes
	switch(p_nWhErrCode)
	{
	case C109_NOO /*= RCS_N00_Success*/:
		break;
	case C109_E02 /*= RCS_E02_InvalidSelection*/:
		LOG_ERROR_APP("[109-(BurstModeControl)]: RCS_E02_InvalidSelection");
		return true;
	case C109_E05 /*= RCS_E05_TooFewDataBytesReceived*/:
		LOG_ERROR_APP("[109-(BurstModeControl)]: RCS_E05_TooFewDataBytesReceived");
		return true;
	case C109_E06 /*= RCS_E06_DeviceSpecificCommandError*/:
		LOG_ERROR_APP("[109-(BurstModeControl)]: RCS_E06_DeviceSpecificCommandError");
		return true;
	case C109_E07 /*= RCS_E07_InWriteProtectMode*/:
		LOG_ERROR_APP("[109-(BurstModeControl)]: RCS_E07_InWriteProtectMode");
		return true;
	case C109_W08 /*= RCM_W08_UpdatePeriodIncreased*/:
		LOG_WARN_APP("[109-(BurstModeControl)]: RCM_W08_UpdatePeriodIncreased");
		break;
	case C109_E09 /*= RCM_E09_InsufficientBandwidth*/:
		LOG_ERROR_APP("[109-(BurstModeControl)]: RCM_E09_InsufficientBandwidth");
		return true;
	case C109_E16 /*= RCS_E16_AccessRestricted*/:
		LOG_ERROR_APP("[109-(BurstModeControl)]: RCS_E16_AccessRestricted");
		return true;
	case C109_E32 /*= RCS_E32_Busy*/:
		LOG_ERROR_APP("[109-(BurstModeControl)]: RCS_E32_Busy");
		return true;
	case C109_E33 /*= RCS_E33_DelayedResponseInitiated*/:
		LOG_ERROR_APP("[109-(BurstModeControl)]: RCS_E33_DelayedResponseInitiated");
		return true;
	case C109_E34 /*= RCS_E34_DelayedResponseRunning*/:
		LOG_ERROR_APP("[109-(BurstModeControl)]: RCS_E34_DelayedResponseRunning");
		return true;
	case C109_E35 /*= RCS_E35_DelayedResponseDead*/:
		LOG_ERROR_APP("[109-(BurstModeControl)]: RCS_E35_DelayedResponseDead");
		return true;
	case C109_E36 /*= RCS_E36_DelayedResponseConflict*/:
		LOG_ERROR_APP("[109-(BurstModeControl)]: RCS_E36_DelayedResponseConflict");
		return true;
	default:
		LOG_ERROR_APP("[109-(BurstModeControl)]: invalid gateway error = " << p_nWhErrCode);
		return true;
	}

	return false;	
}

bool SaveRespErr::IsErr_111(int p_nHostErrCode, int p_nWhErrCode)
{
    return true;
}

bool SaveRespErr::IsErr_112(int p_nHostErrCode, int p_nWhErrCode)
{
    return true;
}

bool SaveRespErr::IsErr_178(int p_nHostErrCode, int p_nWhErrCode)
{
    //host error codes
    if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
    {
        LOG_ERROR_APP("[178-(PublishedDynamicData)]: hostError=" << p_nHostErrCode);
        return true;
    }

    //wh error codes
    switch(p_nWhErrCode)
    {
    case C178_NOO:// = RCS_N00_Success,
        break;
    case C178_E16:// = RCS_E16_AccessRestricted
        LOG_ERROR_APP("[178-(PublishedDynamicData)]: RCS_E16_AccessRestricted");
        return true;
    case C178_E32:// = RCS_E32_Busy
        LOG_ERROR_APP("[178-(PublishedDynamicData)]: RCS_E32_Busy");
        return true;
    default:
        LOG_ERROR_APP("[178-(PublishedDynamicData)]: invalid gateway error =" << p_nWhErrCode);
        return true;
    }
    return false;
}

// save errors
// wireless
void SaveRespErr::SaveErr_778(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		CommandFailed((DBCommand::ResponseStatus)((int)DBCommand::rsFailure_HostReceptionErrorBase + p_nHostErrCode));
		return;
	}
	//wh error codes
	switch(p_nWhErrCode)
	{
	case C778_N00 /*= RCS_N00_Success*/:
		assert(false);
	case C778_E06 /*= RCS_E06_DeviceSpecificCommandError*/:
		CommandFailed((DBCommand::ResponseStatus)C778_E06);
		break;
	case C778_E32 /*= RCS_E32_Busy*/:
		CommandFailed((DBCommand::ResponseStatus)C778_E32);
		break;
	default:
		CommandFailed(DBCommand::rsFailure_GatewayInvalidFailureCode);
		break;
	}

}
void SaveRespErr::SaveErr_779(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		CommandFailed((DBCommand::ResponseStatus)((int)DBCommand::rsFailure_HostReceptionErrorBase + p_nHostErrCode));
		return;
	}
	//wh error codes
	switch(p_nWhErrCode)
	{
	case C779_N00 /*= RCS_N00_Success*/:
		assert(false);
	default:
		CommandFailed(DBCommand::rsFailure_GatewayInvalidFailureCode);
		break;
	}
}

void SaveRespErr::SaveErr_785(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		CommandFailed((DBCommand::ResponseStatus)((int)DBCommand::rsFailure_HostReceptionErrorBase + p_nHostErrCode));
		return;
	}
	//wh error codes
	switch(p_nWhErrCode)
	{
    case C785_W08 /*= RCM_W08_SetToNearestPossibleValue*/:
	case C785_N00 /*= RCS_N00_Success*/:
		assert(false);
    case C785_E02 /*= RCS_E02_InvalidSelection*/:
		CommandFailed((DBCommand::ResponseStatus)C785_E02);
		break;
    case C785_E05 /*= RCS_E05_TooFewDataBytesReceived,*/:
		CommandFailed((DBCommand::ResponseStatus)C785_E05);
		break;
	default:
		CommandFailed(DBCommand::rsFailure_GatewayInvalidFailureCode);
		break;
	}
}
void SaveRespErr::SaveErr_802(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		CommandFailed((DBCommand::ResponseStatus)((int)DBCommand::rsFailure_HostReceptionErrorBase + p_nHostErrCode));
		return;
	}

	//wh error codes
	switch(p_nWhErrCode)
	{
	case C802_NOO: /*= RCS_N00_Success*/
	case C802_W08: /*= RCM_W08_SetToNearestPossibleValue (maximum no. of routes to be requested)*/
		assert(false);
	case C802_E02: /*= RCS_E02_InvalidSelection*/
		CommandFailed((DBCommand::ResponseStatus)C802_E02);
		break;
	case C802_E05: /*= RCS_E05_TooFewDataBytesReceived*/
		CommandFailed((DBCommand::ResponseStatus)C802_E05);
		break;
	default:
		CommandFailed(DBCommand::rsFailure_GatewayInvalidFailureCode);
		break;
	}
}
void SaveRespErr::SaveErr_803(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		CommandFailed((DBCommand::ResponseStatus)((int)DBCommand::rsFailure_HostReceptionErrorBase + p_nHostErrCode));
		return;
	}

	//wh error codes
	switch(p_nWhErrCode)
	{
	case C803_NOO: /*= RCS_N00_Success*/
		assert(false);
	case C803_E05: /*= RCS_E05_TooFewDataBytesReceived*/
		CommandFailed((DBCommand::ResponseStatus)C803_E05);
		break;
	case C803_E65: /*= RCM_E65_EntryNotFound*/
		CommandFailed((DBCommand::ResponseStatus)C803_E65);
		break;
	default:
		CommandFailed(DBCommand::rsFailure_GatewayInvalidFailureCode);
		break;
	}
}
void SaveRespErr::SaveErr_814(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		CommandFailed((DBCommand::ResponseStatus)((int)DBCommand::rsFailure_HostReceptionErrorBase + p_nHostErrCode));
		return;
	}

	//wh error codes
	switch(p_nWhErrCode)
	{
	case C814_NOO /*= RCS_N00_Success*/:
		assert(false);
	case C814_E02 /*= RCS_E02_InvalidSelection*/:
		CommandFailed((DBCommand::ResponseStatus)C814_E02);
		break;
	case C814_E05 /*= RCS_E05_TooFewDataBytesReceived*/:
		CommandFailed((DBCommand::ResponseStatus)C814_E05);
		break;
	case C814_W08 /*= RCM_W08_SetToNearestPossibleValue (maximum no. of IDs to be requested)*/:
		assert(false);
	default:
		CommandFailed(DBCommand::rsFailure_GatewayInvalidFailureCode);
		break;
	}
}
void SaveRespErr::SaveErr_832(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		CommandFailed((DBCommand::ResponseStatus)((int)DBCommand::rsFailure_HostReceptionErrorBase + p_nHostErrCode));
		return;
	}

	//wh error codes
	switch(p_nWhErrCode)
	{
	case C832_ResponseCode_Success /*= 0*/:
		assert(false);
	case C832_ResponseCode_InvalidSelection /*= 2*/:
		CommandFailed((DBCommand::ResponseStatus)C832_ResponseCode_InvalidSelection);
		break;
	case C832_ResponseCode_TooFewBytes /*= 5*/:
		CommandFailed((DBCommand::ResponseStatus)C832_ResponseCode_TooFewBytes);
		break;
	default:
		CommandFailed(DBCommand::rsFailure_GatewayInvalidFailureCode);
		break;
	}
}
void SaveRespErr::SaveErr_833(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		CommandFailed((DBCommand::ResponseStatus)((int)DBCommand::rsFailure_HostReceptionErrorBase + p_nHostErrCode));
		return;
	}
	
	//wh error codes
	switch(p_nWhErrCode)
	{
	case C833_ResponseCode_Success /*= 0*/:
		assert(false);
	case C833_ResponseCode_InvalidSelection /*= 2*/:
		CommandFailed((DBCommand::ResponseStatus)C833_ResponseCode_InvalidSelection);
		break;
	case C833_ResponseCode_TooFewBytes /*= 5*/:
		CommandFailed((DBCommand::ResponseStatus)C833_ResponseCode_TooFewBytes);
		break;
	case C833_W08 /*= RCM_W08_SetToNearestPossibleValue*/:
		assert(false);
	case C833_E65 /*= RCM_E65_InvalidNeighborTableIndex*/:
		CommandFailed((DBCommand::ResponseStatus)C833_E65);
		break;
	default:
		CommandFailed(DBCommand::rsFailure_GatewayInvalidFailureCode);
		break;
	}
}
void SaveRespErr::SaveErr_834(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		CommandFailed((DBCommand::ResponseStatus)((int)DBCommand::rsFailure_HostReceptionErrorBase + p_nHostErrCode));
		return;
	}
	
	//wh error codes
	switch(p_nWhErrCode)
	{
	case C834_ResponseCode_Success /*= 0*/:
		assert(false);
	case C834_ResponseCode_InvalidSelection /*= 2*/:
		CommandFailed((DBCommand::ResponseStatus)C834_ResponseCode_InvalidSelection);
		break;
	case C834_ResponseCode_TooFewBytes /*= 5*/:
		CommandFailed((DBCommand::ResponseStatus)C834_ResponseCode_TooFewBytes);
		break;
	case C834_E65 /*= RCM_E65_EntryNotFound*/:
		CommandFailed((DBCommand::ResponseStatus)C834_E65);
		break;
	default:
		CommandFailed(DBCommand::rsFailure_GatewayInvalidFailureCode);
		break;
	}

}
void SaveRespErr::SaveErr_837(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		CommandFailed((DBCommand::ResponseStatus)((int)DBCommand::rsFailure_HostReceptionErrorBase + p_nHostErrCode));
		return;
	}
	
	//wh error codes
	switch(p_nWhErrCode)
	{
	case C837_NOO:// RCS_N00_Success,
		assert(false);
	case C837_E05:// RCS_E05_TooFewDataBytesReceived,
		CommandFailed((DBCommand::ResponseStatus)C837_E05);
		break;
	case C837_E16:// RCS_E16_AccessRestricted,
		CommandFailed((DBCommand::ResponseStatus)C837_E16);
		break;
	case C837_E65:// RCM_E65_UnknownUID,
		CommandFailed((DBCommand::ResponseStatus)C837_E65);
		break;
	case C837_E66:// RCM_E66_UnknownNotificationFlag
		CommandFailed((DBCommand::ResponseStatus)C837_E66);
		break;
	default:
		CommandFailed(DBCommand::rsFailure_GatewayInvalidFailureCode);
		break;
	}
}
void SaveRespErr::SaveErr_839(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		CommandFailed((DBCommand::ResponseStatus)((int)DBCommand::rsFailure_HostReceptionErrorBase + p_nHostErrCode));
		return;
	}
	
	//wh error codes
	switch(p_nWhErrCode)
	{
	case C839_N00: // RCS_N00_Success
		assert(false);
	case C839_E05: // RCS_E05_TooFewDataBytesReceived
		CommandFailed((DBCommand::ResponseStatus)C839_E05);
		break;
	case C839_E65: // RCM_E65_UnknownUID
		CommandFailed((DBCommand::ResponseStatus)C839_E65);
		break;
	default:
		CommandFailed(DBCommand::rsFailure_GatewayInvalidFailureCode);
		break;
	}
}
void SaveRespErr::SaveErr_840(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		CommandFailed((DBCommand::ResponseStatus)((int)DBCommand::rsFailure_HostReceptionErrorBase + p_nHostErrCode));
		return;
	}

	//wh error codes
	switch(p_nWhErrCode)
	{
	case C840_ResponseCode_Success: // RCS_N00_Success
		assert(false);
	case C840_E02: //RCS_E02_InvalidSelection
		CommandFailed((DBCommand::ResponseStatus)C840_E02);	
		break;
	case C840_E05: // RCS_E05_TooFewDataBytesReceived
		CommandFailed((DBCommand::ResponseStatus)C840_E05);	
		break;
	default:
		CommandFailed((DBCommand::ResponseStatus)C840_E02);
		break;
	}
}
// universal
void SaveRespErr::SaveErr_000(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		CommandFailed((DBCommand::ResponseStatus)((int)DBCommand::rsFailure_HostReceptionErrorBase + p_nHostErrCode));
		return;
	}

	//wh error codes
	switch(p_nWhErrCode)
	{
	case C000_N00: /*= RCS_N00_Success*/
		assert(false);
	default:
		CommandFailed(DBCommand::rsFailure_GatewayInvalidFailureCode);
		break;
	}
}
void SaveRespErr::SaveErr_001(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		CommandFailed((DBCommand::ResponseStatus)((int)DBCommand::rsFailure_HostReceptionErrorBase + p_nHostErrCode));
		return;
	}

	//wh error codes
	switch(p_nWhErrCode)
	{
	case C001_N00: // = RCS_N00_Success,
		assert(false);
	case C001_E06: // = RCS_E06_DeviceSpecificCommandError,
		CommandFailed((DBCommand::ResponseStatus)C001_E06);
		break;
	case C001_W08: // = RCM_W08_UpdateFailure,
		CommandFailed((DBCommand::ResponseStatus)C001_W08);
		break;
	case C001_E16: // = RCS_E16_AccessRestricted
		CommandFailed((DBCommand::ResponseStatus)C001_E16);
		break;
	default:
		CommandFailed(DBCommand::rsFailure_GatewayInvalidFailureCode);
		break;
	}
}
void SaveRespErr::SaveErr_002(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		CommandFailed((DBCommand::ResponseStatus)((int)DBCommand::rsFailure_HostReceptionErrorBase + p_nHostErrCode));
		return;
	}

	//wh error codes
	switch(p_nWhErrCode)
	{
	case C002_N00:// = RCS_N00_Success,
		break;
	case C002_E06:// = RCS_E06_DeviceSpecificCommandError,
		CommandFailed((DBCommand::ResponseStatus)C002_E06);
		return;
	case C002_W08:// = RCM_W08_UpdateFailure,
		CommandFailed((DBCommand::ResponseStatus)C002_W08);	
		return;
	case C002_E16:// = RCS_E16_AccessRestricted
		CommandFailed((DBCommand::ResponseStatus)C002_E16);
		return;
	default:
		CommandFailed(DBCommand::rsFailure_GatewayInvalidFailureCode);
		return;

	}
}
void SaveRespErr::SaveErr_003(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		CommandFailed((DBCommand::ResponseStatus)((int)DBCommand::rsFailure_HostReceptionErrorBase + p_nHostErrCode));
		return;
	}

	//wh error codes
	switch(p_nWhErrCode)
	{
	case C003_N00:// = RCS_N00_Success,
		break;
	case C003_E06:// = RCS_E06_DeviceSpecificCommandError,
		CommandFailed((DBCommand::ResponseStatus)C003_E06);		
		return;
	case C003_W08:// = RCM_W08_UpdateFailure,
		CommandFailed((DBCommand::ResponseStatus)C003_W08);	
		return;
	case C003_E16:// = RCS_E16_AccessRestricted
		CommandFailed((DBCommand::ResponseStatus)C003_E16);	
		return;
	default:
		CommandFailed(DBCommand::rsFailure_GatewayInvalidFailureCode);
		return;
	}
}
void SaveRespErr::SaveErr_009(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		CommandFailed((DBCommand::ResponseStatus)((int)DBCommand::rsFailure_HostReceptionErrorBase + p_nHostErrCode));
		return;
	}
	//wh error codes
	switch(p_nWhErrCode)
	{
	case C009_N00:// = RCS_N00_Success,
		assert(false);
	case C009_E02:// = RCS_E02_InvalidSelection,
		CommandFailed((DBCommand::ResponseStatus)C009_E02);
		break;
	case C009_E05:// = RCS_E05_TooFewDataBytesReceived,
		CommandFailed((DBCommand::ResponseStatus)C009_E05);
		break;
	case C009_E06:// = RCS_E06_DeviceSpecificCommandError,
		CommandFailed((DBCommand::ResponseStatus)C009_E06);
		break;
	case C009_W08:// = RCM_W08_UpdateFailure,
		CommandFailed((DBCommand::ResponseStatus)C009_W08);
		break;
	case C009_E16:// = RCS_E16_AccessRestricted,
		CommandFailed((DBCommand::ResponseStatus)C009_E16);
		break;
	case C009_W30:// = RCM_W30_CommandResponseTruncated
		CommandFailed((DBCommand::ResponseStatus)C009_W30);
		break;
	default:
		CommandFailed(DBCommand::rsFailure_GatewayInvalidFailureCode);
		break;
	}
}

void SaveRespErr::SaveErr_020(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if(p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		CommandFailed((DBCommand::ResponseStatus)((int)DBCommand::rsFailure_HostReceptionErrorBase+p_nHostErrCode));
		return;
	}
	//wh error codes
	switch(p_nWhErrCode)
	{
	case C020_N00:// = RCS_N00_Success
		assert(false);
	case C020_E16:// = RCS_E16_AccessRestricted
		CommandFailed((DBCommand::ResponseStatus)C020_E16);
		break;
	case C020_E32:// = RCS_E32_Busy
		CommandFailed((DBCommand::ResponseStatus)C020_E32);
		break;
	default:
		CommandFailed(DBCommand::rsFailure_GatewayInvalidFailureCode);
		break;
	}
}


void SaveRespErr::SaveErr_033(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		CommandFailed((DBCommand::ResponseStatus)((int)DBCommand::rsFailure_HostReceptionErrorBase + p_nHostErrCode));
		return;
	}

	//wh error codes
	switch(p_nWhErrCode)
	{
	case C033_NOO:// = RCS_N00_Success,
		assert(false);
	case C033_E02:// = RCS_E02_InvalidSelection,
		CommandFailed((DBCommand::ResponseStatus)C033_E02);
		break;
	case C033_E05:// = RCS_E05_TooFewDataBytesReceived,
		CommandFailed((DBCommand::ResponseStatus)C033_E05);
		break;
	case C033_E06:// = RCS_E06_DeviceSpecificCommandError,
		CommandFailed((DBCommand::ResponseStatus)C033_E06);
		break;
	case C033_W08:// = RCM_W08_UpdateFailure,
		CommandFailed((DBCommand::ResponseStatus)C033_W08);
		break;
	case C033_E16:// = RCS_E16_AccessRestricted
		CommandFailed((DBCommand::ResponseStatus)C033_E16);
		break;
	default:
		CommandFailed(DBCommand::rsFailure_GatewayInvalidFailureCode);
		break;
	}

}

void SaveRespErr::SaveErr_074(int p_nHostErrCode, int p_nWhErrCode)
{
    //host error codes
    if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
    {
        CommandFailed((DBCommand::ResponseStatus)((int)DBCommand::rsFailure_HostReceptionErrorBase + p_nHostErrCode));
        return;
    }
    //wh error codes
    switch(p_nWhErrCode)
    {
    case C074_NOO /*= RCS_N00_Success*/:
        break;
    case C074_E06 /*= RCS_E06_DeviceSpecificCommandError*/:
        CommandFailed((DBCommand::ResponseStatus)C084_E06);
        break;
    case C074_E16 /*= RCS_E16_AccessRestricted*/:
        CommandFailed((DBCommand::ResponseStatus)C074_E16);
        break;
    case C074_E32 /*= RCS_E32_Busy*/:
        CommandFailed((DBCommand::ResponseStatus)C074_E32);
        break;
    default:
        CommandFailed(DBCommand::rsFailure_GatewayInvalidFailureCode);
        break;
    }
}

void SaveRespErr::SaveErr_084(int p_nHostErrCode, int p_nWhErrCode)
{
    //host error codes
    if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
    {
        CommandFailed((DBCommand::ResponseStatus)((int)DBCommand::rsFailure_HostReceptionErrorBase + p_nHostErrCode));
        return;
    }
    //wh error codes
    switch(p_nWhErrCode)
    {
    case C084_NOO /*= RCS_N00_Success*/:
        break;
    case C084_E02 /*= RCS_E02_InvalidSelection*/:
        CommandFailed((DBCommand::ResponseStatus)C084_E02);
        break;
    case C084_E05 /*= RCS_E05_TooFewDataBytesReceived*/:
        CommandFailed((DBCommand::ResponseStatus)C084_E05);
        break;
    case C084_E06 /*= RCS_E06_DeviceSpecificCommandError*/:
        CommandFailed((DBCommand::ResponseStatus)C084_E06);
        break;
    default:
        CommandFailed(DBCommand::rsFailure_GatewayInvalidFailureCode);
        break;
    }
}

void SaveRespErr::SaveErr_101(int p_nHostErrCode, int p_nWhErrCode)
{
    //host error codes
    if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
    {
        CommandFailed((DBCommand::ResponseStatus)((int)DBCommand::rsFailure_HostReceptionErrorBase + p_nHostErrCode));
        return;
    }
    //wh error codes
    switch(p_nWhErrCode)
    {
    case C101_NOO /*= RCS_N00_Success*/:
        break;
    case C101_E02 /*= RCS_E02_InvalidSelection*/:
        CommandFailed((DBCommand::ResponseStatus)C101_E02);
        break;
    case C101_E05 /*= RCS_E05_TooFewDataBytesReceived*/:
        CommandFailed((DBCommand::ResponseStatus)C101_E05);
        break;
    case C101_E06 /*= RCS_E06_DeviceSpecificCommandError*/:
        CommandFailed((DBCommand::ResponseStatus)C101_E06);
        break;
    default:
        CommandFailed(DBCommand::rsFailure_GatewayInvalidFailureCode);
        break;
    }
}

void SaveRespErr::SaveErr_103(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		CommandFailed((DBCommand::ResponseStatus)((int)DBCommand::rsFailure_HostReceptionErrorBase + p_nHostErrCode));
		return;
	}
	//wh error codes
	switch(p_nWhErrCode)
	{
	case C103_W08 /*= RCM_W08_UpdateTimesAdjusted*/:
	case C103_NOO /*= RCS_N00_Success*/:
		break;
	case C103_E05 /*= RCS_E05_TooFewDataBytesReceived*/:
		CommandFailed((DBCommand::ResponseStatus)C103_E05);
		break;
	case C103_E06 /*= RCS_E06_DeviceSpecificCommandError*/:
		CommandFailed((DBCommand::ResponseStatus)C103_E06);
		break;
	case C103_E07 /*= RCS_E07_InWriteProtectMode*/:
		CommandFailed((DBCommand::ResponseStatus)C103_E07);
		break;
	case C103_E09 /*= RCM_E09_InvalidBurstMessage*/:
		CommandFailed((DBCommand::ResponseStatus)C103_E09);
		break;
	case C103_E16 /*= RCS_E16_AccessRestricted*/:
		CommandFailed((DBCommand::ResponseStatus)C103_E16);
		break;
	case C103_E32 /*= RCS_E32_Busy*/:
		CommandFailed((DBCommand::ResponseStatus)C103_E32);
		break;
	case C103_E33 /*= RCS_E33_DelayedResponseInitiated*/:
		CommandFailed((DBCommand::ResponseStatus)C103_E33);
		break;
	case C103_E34 /*= RCS_E34_DelayedResponseRunning*/:
		CommandFailed((DBCommand::ResponseStatus)C103_E34);
		break;
	case C103_E35 /*= RCS_E35_DelayedResponseDead*/:
		CommandFailed((DBCommand::ResponseStatus)C103_E35);
		break;
	case C103_E36 /*= RCS_E36_DelayedResponseConflict*/:
		CommandFailed((DBCommand::ResponseStatus)C103_E36);
		break;
	default:
		CommandFailed(DBCommand::rsFailure_GatewayInvalidFailureCode);
		break;
	}
}

void SaveRespErr::SaveErr_104(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		CommandFailed((DBCommand::ResponseStatus)((int)DBCommand::rsFailure_HostReceptionErrorBase + p_nHostErrCode));
		return;
	}
	//wh error codes
	switch(p_nWhErrCode)
	{
	case C104_NOO /*= RCS_N00_Success*/:
		break;
	case C104_E02 /*= RCS_E02_InvalidSelection*/:
		CommandFailed((DBCommand::ResponseStatus)C104_E02);
		break;
	case C104_E03 /*= RCS_E03_PassedParameterTooLarge*/:
		CommandFailed((DBCommand::ResponseStatus)C104_E03);
		break;
	case C104_E04 /*= RCS_E04_PassedParameterTooSmall*/:
		CommandFailed((DBCommand::ResponseStatus)C104_E04);
		break;
	case C104_E05 /*= RCS_E05_TooFewDataBytesReceived*/:
		CommandFailed((DBCommand::ResponseStatus)C104_E05);
		break;
	case C104_E06 /*= RCS_E06_DeviceSpecificCommandError*/:
		CommandFailed((DBCommand::ResponseStatus)C104_E06);
		break;
	case C104_E07 /*= RCS_E07_InWriteProtectMode*/:
		CommandFailed((DBCommand::ResponseStatus)C104_E07);
		break;
	case C104_E09 /*= RCM_E09_InvalidBurstMessage*/:
		CommandFailed((DBCommand::ResponseStatus)C104_E09);
		break;
	case C104_E11 /*= RCM_E11_InvalidDeviceVariableClassification*/:
		CommandFailed((DBCommand::ResponseStatus)C104_E11);
		break;
	case C104_E12 /*= RCM_E12_InvalidUnitsCode*/:
		CommandFailed((DBCommand::ResponseStatus)C104_E12);
		break;
	case C104_E13 /*= RCM_E13_InvalidBurstTriggerModeSelectionCode*/:
		CommandFailed((DBCommand::ResponseStatus)C104_E13);
		break;
	case C104_E16 /*= RCS_E16_AccessRestricted*/:
		CommandFailed((DBCommand::ResponseStatus)C104_E16);
		break;
	case C104_E32 /*= RCS_E32_Busy*/:
		CommandFailed((DBCommand::ResponseStatus)C104_E32);
		break;
	case C104_E33 /*= RCS_E33_DelayedResponseInitiated*/:
		CommandFailed((DBCommand::ResponseStatus)C104_E33);
		break;
	case C104_E34 /*= RCS_E34_DelayedResponseRunning*/:
		CommandFailed((DBCommand::ResponseStatus)C104_E34);
		break;
	case C104_E35 /*= RCS_E35_DelayedResponseDead*/:
		CommandFailed((DBCommand::ResponseStatus)C104_E35);
		break;
	case C104_E36 /*= RCS_E36_DelayedResponseConflict*/:
		CommandFailed((DBCommand::ResponseStatus)C104_E36);
		break;
	default:
		CommandFailed(DBCommand::rsFailure_GatewayInvalidFailureCode);
		break;
	}
}

void SaveRespErr::SaveErr_105(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		CommandFailed((DBCommand::ResponseStatus)((int)DBCommand::rsFailure_HostReceptionErrorBase + p_nHostErrCode));
		return;
	}
	//wh error codes
	switch(p_nWhErrCode)
	{
	case C105_NOO /*= RCS_N00_Success*/:
		assert(false);
	case C105_E06 /*= RCS_E06_DeviceSpecificCommandError*/:
		CommandFailed((DBCommand::ResponseStatus)C105_E06);
		break;
	case C105_E09 /*= RCM_E09_InvalidBurstMessage*/:
		CommandFailed((DBCommand::ResponseStatus)C105_E09);
		break;
	case C105_E32 /*= RCS_E32_Busy*/:
		CommandFailed((DBCommand::ResponseStatus)C105_E32);
		break;
	default:
		CommandFailed(DBCommand::rsFailure_GatewayInvalidFailureCode);
		break;
	}
}

void SaveRespErr::SaveErr_107(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		CommandFailed((DBCommand::ResponseStatus)((int)DBCommand::rsFailure_HostReceptionErrorBase + p_nHostErrCode));
		return;
	}
	//wh error codes
	switch(p_nWhErrCode)
	{
	case C107_W08 /*= RCM_W08_BurstConditionConflict*/:
	case C107_NOO /*= RCS_N00_Success*/:
		break;
	case C107_E02 /*= RCS_E02_InvalidSelection*/:
		CommandFailed((DBCommand::ResponseStatus)C107_E02);
		break;
	case C107_E05 /*= RCS_E05_TooFewDataBytesReceived*/:
		CommandFailed((DBCommand::ResponseStatus)C107_E05);
		break;
	case C107_E06 /*= RCS_E06_DeviceSpecificCommandError*/:
		CommandFailed((DBCommand::ResponseStatus)C107_E06);
		break;
	case C107_E07 /*= RCS_E07_InWriteProtectMode*/:
		CommandFailed((DBCommand::ResponseStatus)C107_E07);
		break;
	case C107_E09 /*= RCM_E09_InvalidBurstMessage*/:
		CommandFailed((DBCommand::ResponseStatus)C107_E09);
		break;
	default:
		CommandFailed(DBCommand::rsFailure_GatewayInvalidFailureCode);
		break;
	}
}

void SaveRespErr::SaveErr_108(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		CommandFailed((DBCommand::ResponseStatus)((int)DBCommand::rsFailure_HostReceptionErrorBase + p_nHostErrCode));
		return;
	}
	//wh error codes
	switch(p_nWhErrCode)
	{
	case C108_W08 /*= RCM_W08_BurstConditionConflict*/:
	case C108_NOO /*= RCS_N00_Success*/:
		break;
	case C108_E02 /*= RCS_E02_InvalidSelection*/:
		CommandFailed((DBCommand::ResponseStatus)C108_E02);
		break;
	case C108_E05 /*= RCS_E05_TooFewDataBytesReceived*/:
		CommandFailed((DBCommand::ResponseStatus)C108_E05);
		break;
	case C108_E06 /*= RCS_E06_DeviceSpecificCommandError*/:
		CommandFailed((DBCommand::ResponseStatus)C108_E06);
		break;
	case C108_E07 /*= RCS_E07_InWriteProtectMode*/:
		CommandFailed((DBCommand::ResponseStatus)C108_E07);
		break;
	case C108_E09 /*= RCM_E09_InvalidBurstMessage*/:
		CommandFailed((DBCommand::ResponseStatus)C108_E09);
		break;
	default:
		CommandFailed(DBCommand::rsFailure_GatewayInvalidFailureCode);
		break;
	}
}

void SaveRespErr::SaveErr_109(int p_nHostErrCode, int p_nWhErrCode)
{
	//host error codes
	if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
	{
		CommandFailed((DBCommand::ResponseStatus)((int)DBCommand::rsFailure_HostReceptionErrorBase + p_nHostErrCode));
		return;
	}
	//wh error codes
	switch(p_nWhErrCode)
	{
	case C109_W08 /*= RCM_W08_UpdatePeriodIncreased*/:
	case C109_NOO /*= RCS_N00_Success*/:
		break;
	case C109_E02 /*= RCS_E02_InvalidSelection*/:
		CommandFailed((DBCommand::ResponseStatus)C109_E02);
		break;
	case C109_E05 /*= RCS_E05_TooFewDataBytesReceived*/:
		CommandFailed((DBCommand::ResponseStatus)C109_E05);
		break;
	case C109_E06 /*= RCS_E06_DeviceSpecificCommandError*/:
		CommandFailed((DBCommand::ResponseStatus)C109_E06);
		break;
	case C109_E07 /*= RCS_E07_InWriteProtectMode*/:  
		CommandFailed((DBCommand::ResponseStatus)C109_E07);
		break;
	case C109_E09 /*= RCM_E09_InsufficientBandwidth*/:
		CommandFailed((DBCommand::ResponseStatus)C109_E09);
		break;
	case C109_E16 /*= RCS_E16_AccessRestricted*/:
		CommandFailed((DBCommand::ResponseStatus)C109_E16);
		break;
	case C109_E32 /*= RCS_E32_Busy*/:
		CommandFailed((DBCommand::ResponseStatus)C109_E32);
		break;
	case C109_E33 /*= RCS_E33_DelayedResponseInitiated*/:
		CommandFailed((DBCommand::ResponseStatus)C109_E33);
		break;
	case C109_E34 /*= RCS_E34_DelayedResponseRunning*/:
		CommandFailed((DBCommand::ResponseStatus)C109_E34);
		break;
	case C109_E35 /*= RCS_E35_DelayedResponseDead*/:
		CommandFailed((DBCommand::ResponseStatus)C109_E35);
		break;
	case C109_E36 /*= RCS_E36_DelayedResponseConflict*/:
		CommandFailed((DBCommand::ResponseStatus)C109_E36);
		break;
	default:
		CommandFailed(DBCommand::rsFailure_GatewayInvalidFailureCode);
		break;
	}
}

void SaveRespErr::SaveErr_111(int p_nHostErrCode, int p_nWhErrCode)
{
}

void SaveRespErr::SaveErr_112(int p_nHostErrCode, int p_nWhErrCode)
{
}

void SaveRespErr::SaveErr_178(int p_nHostErrCode, int p_nWhErrCode)
{
    //host error codes
    if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
    {
        CommandFailed((DBCommand::ResponseStatus)((int)DBCommand::rsFailure_HostReceptionErrorBase + p_nHostErrCode));
        return;
    }

    //wh error codes
    switch(p_nWhErrCode)
    {
    case C178_NOO:// = RCS_N00_Success,
        assert(false);
    case C178_E16:// = RCS_E16_AccessRestricted
        CommandFailed((DBCommand::ResponseStatus)C178_E16);
        break;
    case C178_E32:// = RCS_E32_Busy
        CommandFailed((DBCommand::ResponseStatus)C178_E32);
        break;
    default:
        CommandFailed(DBCommand::rsFailure_GatewayInvalidFailureCode);
        break;
    }
}

std::string SaveRespErr::GetErrText(int command, int p_nHostErrCode, int p_nWhErrCode)
{
    //host error codes
    if (p_nHostErrCode != gateway::GatewayIO::HostSuccess)
        return "HostError: " + p_nHostErrCode;

    //wh error codes
    switch(p_nWhErrCode)
    {
    case RCS_N00_Success: // 103, 104, 105, 107, 108, 109
        return "Success";
    case RCS_E02_InvalidSelection: // 102, 104, 107, 108, 109
        return "E02_InvalidSelection";
    case RCS_E03_PassedParameterTooLarge: // 104
        return "E03_PassedParameterTooLarge";
    case RCS_E04_PassedParameterTooSmall: // 104
        return "E04_PassedParameterTooSmall";
    case RCS_E05_TooFewDataBytesReceived: // 102, 103, 104, 107, 108, 109
        return "E05_TooFewDataBytesReceived";
    case RCS_E06_DeviceSpecificCommandError: // 102, 103, 104, 105, 107, 108, 109
        return "E06_DeviceSpecificCommandError";
    case RCS_E07_InWriteProtectMode: // 103, 104, 107, 108, 109
        return "E07_InWriteProtectMode";
    case RCM_W08_UpdateTimesAdjusted: // 103, 107, 108, 109
        switch (command) {
            case CMDID_C103_WriteBurstPeriod:
                return "W08_UpdateTimesAdjusted";
            case CMDID_C107_WriteBurstDeviceVariables:
            case CMDID_C108_WriteBurstModeCommandNumber:
                return "W08_BurstConditionConflict";
            case CMDID_C109_BurstModeControl: //
                return "W08_UpdatePeriodIncreased";
            default:
                return "W08";
        }
    case RCM_E09_InvalidSubDeviceIndex: // 102, 103, 104, 105, 107, 108, 109
        switch (command) {
            case CMDID_C102_MapSubDeviceToBurstMessage:
                return "E09_InvalidSubDeviceIndex";
            case CMDID_C103_WriteBurstPeriod:
            case CMDID_C104_WriteBurstTrigger:
            case CMDID_C105_ReadBurstModeConfiguration:
            case CMDID_C107_WriteBurstDeviceVariables:
            case CMDID_C108_WriteBurstModeCommandNumber:
                return "E09_InvalidBurstMessage";
            case CMDID_C109_BurstModeControl: //
                return "E09_InsufficientBandwidth";
            default:
                return "E09";
        }
    case RCM_E11_InvalidDeviceVariableClassification: // 104
        return "E11_InvalidDeviceVariableClassification";
    case RCM_E12_InvalidUnitsCode: // 104
        return "E12_InvalidUnitsCode";
    case RCM_E13_InvalidBurstTriggerModeSelectionCode: // 104
        return "E13_InvalidBurstTriggerModeSelectionCode";
    case RCS_E16_AccessRestricted: // 102, 103, 104, 109
        return "E16_AccessRestricted";
    case RCS_E32_Busy: // 103, 104, 105, 109
        return "E32_Busy";
    case RCS_E33_DelayedResponseInitiated: // 103, 104, 109
        return "E33_DelayedResponseInitiated";
    case RCS_E34_DelayedResponseRunning: // 103, 104, 109
        return "E34_DelayedResponseRunning";
    case RCS_E35_DelayedResponseDead: // 103, 104, 109
        return "E35_DelayedResponseDead";
    case RCS_E36_DelayedResponseConflict: // 103, 104, 109
        return "E36_DelayedResponseConflict";
    default:
        return "Error: " + DBCommand::rsFailure_GatewayInvalidFailureCode;
    }
}

}
}
