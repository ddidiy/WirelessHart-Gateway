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

#include <WHartGateway/GwUtil.h>
#include <WHartGateway/GatewayConfig.h>
#include <WHartGateway/GatewayTypes.h>
#include <WHartStack/WHartStack.h>

namespace hart7 {
namespace gateway {

using namespace stack;

void FillGwIdentityResponse(C000_ReadUniqueIdentifier_Resp * p_pResp, GatewayConfig & config)
{
	memset(p_pResp,0, sizeof(C000_ReadUniqueIdentifier_Resp));

	// byte 0 = 254 - hardcoded
 	p_pResp->expandedDeviceType		= Gateway_ExpandedType();
	p_pResp->minReqPreamblesNo		= config.m_u8MinReqPreamblesNo;
	p_pResp->protocolMajorRevNo		= 7;
	p_pResp->deviceRevisionLevel		= config.m_u8DevRevisionLevel;
	p_pResp->softwareRevisionLevel		= config.m_u8SoftwRevisionLevel;

	// uint8_t
 	p_pResp->hardwareRevisionLevel		= config.m_u8HWRevisionLevel_PhysicalSignCode >>3; // most significant 5 bits (7-3)

 	// uint8_t
 	uint8_t mask = 7; // 00000111
 	p_pResp->physicalSignalingCode		= config.m_u8HWRevisionLevel_PhysicalSignCode & mask;	// least significant 3 bits (2-0)

	p_pResp->flags				= config.m_u8Flags;
	p_pResp->deviceID			= Gateway_DeviceID24();
	p_pResp->minRespPreamblesNo		= config.m_u8MinRespPreamblesNo;
	p_pResp->maxNoOfDeviceVars		= config.m_u8MaxNoOfDevicesVar;
	p_pResp->configChangeCounter		= config.m_u16ConfigChangeCounter;
	p_pResp->extendedFieldDeviceStatus	= config.m_u8FlagBits;

 	uint16_t manufacturerIDCode;
 	memcpy(&manufacturerIDCode, config.m_u8ManufactCode, sizeof(manufacturerIDCode));
 	p_pResp->manufacturerIDCode		= manufacturerIDCode;

 	uint16_t privateLabelDistributorCode;
 	memcpy(&privateLabelDistributorCode, config.m_u8ManufactLabel, sizeof(privateLabelDistributorCode));
 	p_pResp->privateLabelDistributorCode	= privateLabelDistributorCode;

 	p_pResp->deviceProfile			= DeviceProfileCodes_WIRELESSHART_GATEWAY;
}

} // namespace gateway
} // namespace hart7
