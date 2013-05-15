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

//////////////////////////////////////////////////////////////////////////////
/// @author	Catrina Mihailescu
/// @date	Wed Oct  21 15:24:30 EET 2009
/// @brief	Discovery Process Configuration. 
//////////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/file.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/wait.h>

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <net/if.h>

#include <time.h>
 
#include "DiscoveryApp.h"
#include "DiscoveryConfig.h"
#define FS_TREE_REL
#include "../../Shared/StreamLink.h"
#include "../../Shared/IniParser.h"
#include "../../Shared/Common.h"
#include "../../Shared/h.h"
#include "../../Shared/Config.h"
#include "../../Shared/app.h"

#define DISCOVERY_FILE_PATH	 NIVIS_PROFILE"gw_info_univ.ini"


CWHDiscoveryCfg::CWHDiscoveryCfg()
{
	memset( this, 0, sizeof(*this) );
}

CWHDiscoveryCfg::~CWHDiscoveryCfg()
{
	;
}

const uint16_t Discovery_ExpandedType()
{
	return  0xF981;
}


// read from DISCOVERY_FILE_PATH file

bool CWHDiscoveryCfg::ReadDiscoveryVars()
{
	LOG("*******DISCOVERY VARIABLES INIT*********");
	CIniParser discovery_parser;

	if(!discovery_parser.Load(DISCOVERY_FILE_PATH))
	{
		LOG("****NO DISCOVERY INIT FILE*****");
		return false;
	}


	// read variables for C000
	response0_msg.m_u8FirstByte = COMMAND0_FIRSTBYTE;

	response0_msg.m_u16ExtendedDevType = Discovery_ExpandedType();
	
	int min_req_preambles_no;
	if (!(discovery_parser.GetVar("WH_GATEWAY","MIN_REQ_PREAMBLES_NO", &min_req_preambles_no)))
		min_req_preambles_no = 0;	
	response0_msg.m_u8MinNoPreambles = (uint8_t) min_req_preambles_no;

	response0_msg.m_u8WHRevisionNo = COMMAND0_REVISION_NO;

	int device_revision_level;
	if (!discovery_parser.GetVar("WH_GATEWAY","DEVICE_REVISION_LEVEL", &device_revision_level))
		device_revision_level = 0;
	response0_msg.m_u8DevRevisionLevel = (uint8_t) device_revision_level;

	int sw_revision_no;
	if (!discovery_parser.GetVar("WH_GATEWAY","SOFTWARE_REVISION_LEVEL", &sw_revision_no))
		sw_revision_no = 0;
	response0_msg.m_u8SoftwRevisionLevel = (uint8_t)  sw_revision_no;

	int hwRevisionLevel_physicalSignalingMode;
	if (!discovery_parser.GetVar("WH_GATEWAY","HWREVLEVEL_PHYSIGNALMODE", &hwRevisionLevel_physicalSignalingMode))
		hwRevisionLevel_physicalSignalingMode = 0;
	response0_msg.m_u8HWRevisionLevel_PhysicalSignCode = (uint8_t) hwRevisionLevel_physicalSignalingMode;

	if (!(discovery_parser.GetVar("WH_GATEWAY","FLAGS", &response0_msg.m_u8Flags, 1) == 1))
		response0_msg.m_u8Flags = 0;

	response0_msg.m_DeviceID[0] = COMMAND0_DEVICEID >> 16 & 0xFFFF;
	response0_msg.m_DeviceID[1] = COMMAND0_DEVICEID >> 8 & 0xFFFF;
	response0_msg.m_DeviceID[2] = COMMAND0_DEVICEID  & 0xFFFF;

	int min_resp_preambles_no;
	if (!(discovery_parser.GetVar("WH_GATEWAY","MIN_RESP_PREAMBLES_NO", &min_resp_preambles_no)))
		min_resp_preambles_no = 0; 
	response0_msg.m_u8MinPreamblesNo = (uint8_t) min_resp_preambles_no;

	int max_no_of_devices_var;
	if (!(discovery_parser.GetVar("WH_GATEWAY","MAX_NO_OF_DEVICES_VARS", &max_no_of_devices_var)))
		max_no_of_devices_var = 0; 
	response0_msg.m_u8MaxDeviceVarNo = (uint8_t) max_no_of_devices_var;

	int configChangeCounter;
	if (!(discovery_parser.GetVar("WH_GATEWAY","CHANGE_COUNTER", &configChangeCounter)))
		configChangeCounter = 0;
	response0_msg.m_u16ConfigChangeCounter = (uint16_t) configChangeCounter;

	int field_device_status;
	if (!discovery_parser.GetVar("WH_GATEWAY","EXTENDED_STATUS", &field_device_status))
		field_device_status = 0;
	response0_msg.m_u8FlagBits = (uint8_t)  field_device_status;

	if (!(discovery_parser.GetVar("WH_GATEWAY","MANUFACT_ID_CODE", response0_msg.m_u8ManufactCode, sizeof(response0_msg.m_u8ManufactCode)) == sizeof(response0_msg.m_u8ManufactCode)))
		memset(response0_msg.m_u8ManufactCode, 0, sizeof(response0_msg.m_u8ManufactCode));

	if (!(discovery_parser.GetVar("WH_GATEWAY","PRIVATE_LABEL_CODE", response0_msg.m_u8ManufactLabel, sizeof(response0_msg.m_u8ManufactLabel)) == sizeof(response0_msg.m_u8ManufactLabel)))
		memset(response0_msg.m_u8ManufactLabel, 0, sizeof(response0_msg.m_u8ManufactLabel));

	response0_msg.m_u8DeviceProfile = COMMAND0_DEVICE_PROFILE;

	// read variables for C020
	
	char long_tag[33];
	memset(long_tag, 0, sizeof(long_tag));
	if (!(discovery_parser.GetVar("WH_GATEWAY","LONG_TAG", long_tag, sizeof(long_tag)) >0))
		memset(long_tag, 0, sizeof(long_tag));
	memcpy(response20_msg.m_pLongTag, long_tag, 32);

	return true;
}

int CWHDiscoveryCfg::Init(const char* p_szModuleName)
{
	if (!CConfig::Init(p_szModuleName))
	{
		LOG("GROUP NOT FOUND");
		return 0;
	}
	
	// get GUID from config.ini
	if (!(GetVar("GUID", response_msg.m_pGUID, sizeof(response_msg.m_pGUID)) == sizeof(response_msg.m_pGUID)))
	{
		LOG("GUID NOT FOUND IN config.ini FILE");
		return 0;
	}
	
	// get LOCAL_PORT from config.ini
	if(!GetVar("LOCAL_PORT", &m_nLocalPort))
	{
		LOG("LOCAL_PORT NOT FOUND IN config.ini FILE");	
		return 0;
	}
	LOG("LOCAL PORT from config.ini: %d",m_nLocalPort );
	
	//get TYPE from config.ini
	char type[4];
	if (!GetVar("TYPE",type , sizeof(type)))
	{
		LOG("TYPE NOT FOUND in config.ini file");
		return 0;
	}
	LOG("TYPE from config.ini: %s",type );
	memcpy(m_szType, type, strlen(type));

	// set Protocol Version No
	response_msg.m_u8Version = GATEWAY_VERSION;

	// set Reserved Byte
	response_msg.m_u8Reserved = GATEWAY_RESERVED_BYTE;
	
	// set Byte Count
	response_msg.m_u16ByteCount = sizeof(response_msg) + sizeof(response0_msg) + sizeof(response20_msg);
	
	// get IP/Port from config.ini
	net_address stNetAddr;
	if(!CIniParser::GetVar("WH_GATEWAY", "GATEWAY", &stNetAddr))
	{	
		return 0;
	}

	response_msg.m_u16Port = stNetAddr.m_dwPortNo;
	response_msg.m_u32IpAddress = stNetAddr.m_nIP;

	return 1;
}





