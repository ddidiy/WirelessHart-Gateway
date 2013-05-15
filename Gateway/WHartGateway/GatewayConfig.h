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

/*
 * GatewayConfig.h
 *
 *  Created on: Dec 15, 2008
 *      Author: nicu.dascalu
 */

#ifndef GATEWAYCONFIG_H_
#define GATEWAYCONFIG_H_

#include <string>
#include <WHartStack/WHartTypes.h>
#include <WHartStack/util/WHartCmdWrapper.h>

#include"GatewayTypes.h"
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

#include <Shared/Common.h>
#include <Shared/IniParser.h>
#include <WHartStack/util/WHartCmdWrapper.h>
#include <Shared/Config.h>
#include <Shared/Utils.h>



namespace hart7 {
namespace gateway {
using namespace stack;

/**
 * Gateway configuration.
 */

class GatewayConfig : CConfig
{
public:
	enum { DRM_TYPE_NONE = 0, DRM_TYPE_CACHE_BASED = 1, DRM_TYPE_IO_SYTEM = 2};
public:
	GatewayConfig();
	bool	Init();
	bool	RuntimeReload();

	bool	InitGwCmds();
	bool	ReadGWUniversalVariables();
	bool	ReadGWVariables();

	bool	IsSpecificBurstCmd(uint16_t cmdID);
	bool	IsSpecificReadCmd(uint16_t cmdID);
	bool	BurstSpecificCmds();
	bool	ReadSpecificCmds();

	bool	WriteCmdUniversalMessage (char * universalmessage);
	bool	WriteCmdTag (char * tag);
	bool	WriteCmdMasterDescriptor(char * masterDescriptor);
	bool	WriteCmdMasterDate(WHartDate masterDate);
	bool	WriteCmdFinalAssemblyNumber(uint32_t finalAssemblyNumber);
	bool	WriteCmdLongTag(char * longTag);
	bool	WriteDeviceStatus (uint8_t deviceStatus);
	bool 	WriteMinRespPreamblesNo(uint8_t p_u8MinRespPreamblesNo);
	bool 	WriteGwReqMaxRetryNo(uint8_t p_u8RetryNo);

	bool	m_bResetFlag;
	int		m_nNetworkManagerPort;
	char	m_szNetworkManagerHost[32];
	int		m_nNmClientListenPort;

	int		m_nListenAccessPointPort;
	int 	m_nAccessPointClientMinPort;
	int		m_nAccessPointClientMaxPort;

	int 	m_nHostApp_ListenPort;
	int		m_nHostApp_MinPort;
	int		m_nHostApp_MaxPort;

	int		m_nGwReqTimeout;

	/**
	 * Timeout for the request retries.
	 */
	int     m_nGwReqRetryTimeout; // minutes

	int		m_nGwDrmTimeout;
	int		m_nGwReqMaxRetryNo;
	int 	m_nLocalGwRetries;

	int		m_nMockMode;
	int		m_nSendIdInfoCmds;
	int		m_nDevicesRefreshInterval;

	int		m_nGwRequestService;
	int		m_nLogLevelStack;
	int		m_nLogLevelApp;

	int 	m_nGwBurstRespTimeout;
	int 	m_nGwRespTimeout;

	int 	m_nLogInternalStatusPeriod;
	int		m_nLogInternalRequestsStatisticsPeriod;

	int		m_nMaxCmdsPerAPDU;

	float	m_fBurstNotificationRate;
 	float	m_fEventNotificationRate;
	float	m_fDeviceStatusNotificationRate;
	float	m_fDeviceConfigurationNotificationRate;
	float	m_fTopologyNotificationRate;
	float	m_fScheduleNotificationRate;

	char	m_szAppPidFile[256];
	char	m_szModuleName[256];

	bool 	m_bNmBurstsCachingEnabled;
	bool	m_bUseSubdevPollingAddresses;

	bool	m_bDontAckC119WhenTime_minus1;
	bool	m_bSendDirectWiredDeviceBurst;
	bool	m_bBuildUnivCommandsCache;
	int		m_nDrmType;
	int		m_nDrmEntryLifetime;

	bool    m_bSendInvalidRequestToDevice;

public:

	int NetworkManagerPort()
	{
		return m_nNetworkManagerPort;
	}

	std::string NetworkManagerHost()
	{
		return m_szNetworkManagerHost;
	}

	int ListenAccessPointPort()
	{
		return m_nListenAccessPointPort;
	}

	int AccessPointClientMinPort()
	{
		return m_nAccessPointClientMinPort;
	}

	int AccessPointClientMaxPort()
	{
		return m_nAccessPointClientMaxPort;
	}

	int HostApp_ListenPort()
	{
		return m_nHostApp_ListenPort;
	}

	int HostApp_MinPort()
	{
		return m_nHostApp_MinPort;
	}

	int HostApp_MaxPort()
	{
		return m_nHostApp_MaxPort;
	}

	int GwReqTimeout()
	{
		return m_nGwReqTimeout;
	}

    int GwReqRetryTimeout()
    {
        return m_nGwReqRetryTimeout;
    }

	int GwDrmTimeout()
	{
		return m_nGwDrmTimeout;
	}

	int GwBurstRespTimeout()
	{
		return m_nGwBurstRespTimeout;
	}

	int GwRespTimeout()
	{
		return m_nGwRespTimeout;
	}

	int GwReqMaxRetryNo()
	{
		return m_nGwReqMaxRetryNo;
	}

	int LogInternalStatusPeriod()
	{
		return m_nLogInternalStatusPeriod;
	}

	int LogInternalRequestsStatisticsPeriod()
	{
		return m_nLogInternalRequestsStatisticsPeriod;
	}

	float	BurstNotificationRate()
	{
		return m_fBurstNotificationRate;
	}

 	float	EventNotificationRate()
	{
		return m_fEventNotificationRate;
	}

	float	DeviceStatusNotificationRate()
	{
		return m_fDeviceStatusNotificationRate;
	}

	float	DeviceConfigurationNotificationRate()
	{
		return m_fDeviceConfigurationNotificationRate;
	}

	float	TopologyNotificationRate()
	{
		return m_fTopologyNotificationRate;
	}

	float	ScheduleNotificationRate()
	{
		return m_fScheduleNotificationRate;
	}

	bool NmBurstsCachingEnabled()
	{
		return m_bNmBurstsCachingEnabled;
	}

	bool UseSubdevPollingAddresses()
	{
		return m_bUseSubdevPollingAddresses;
	}

	CHartCmdWrapperList& GetDeviceInitCmds (const WHartUniqueID& m_rUniqueId)
	{
		TargetedCmdsUniqueIdMap::iterator it = m_oDeviceTargetedCmds.find(m_rUniqueId);

		if (it != m_oDeviceTargetedCmds.end())
		{
			return it->second;
		}

		return m_oDeviceInitCmds;
	}



public:
	char		m_szTag[9];
	char		m_szLongTag[33];
	char		m_szCmdUniversalMessage[33];

	char		m_szMasterDescriptor[17];
	WHartDate	m_stMasterDate;
	uint32_t	m_u32FinalAssemblyNumber;


	// [WH_GATEWAY] elements from config files

	uint8_t    m_u8AppJoinKey[16];

	uint32_t 	m_u32IpAddress;					/* gateway IP address */
	uint8_t 	m_u8DevRevisionLevel; 				/* Device Revision Level */
	uint8_t 	m_u8SoftwRevisionLevel;				/* Software Revision Level */
	uint8_t		m_u8HWRevisionLevel_PhysicalSignCode;		/* Hardware Revision Level - most significant 5 bits and Physical Signaling Code - lease significant 3 bits */
	uint8_t		m_u8Flags;					/* Flags */
	uint8_t 	m_u8FlagBits;					/* Extended Field Device Status */
	uint8_t 	m_u8ManufactCode[2];				/* Manufacturer Identification Code - Table 8  (range 1 - 24619) */
    uint8_t		m_u8ManufactLabel[2];				/* Private Label Distributor Code */

	uint16_t 	m_u16ConfigChangeCounter;			/* Configuration Change Counter	*/
	uint8_t 	m_u8MinReqPreamblesNo;				/* Mininum number of Preambles required from the Master to the Slave */
	uint8_t 	m_u8MinRespPreamblesNo;				/* Minimum number of Preambles to be sent with the response message */
	uint8_t 	m_u8MaxNoOfDevicesVar;				/* Maximum number of Device Variables */

	void DeviceStatus(uint8_t m_u8DeviceStatus);
	uint8_t DeviceStatus();

private:

    // TODO: check if device status must be in config
	uint8_t		m_u8DeviceStatus;				/* Device Status */
public:

	// variables used in command 048
	uint8_t 	m_u8deviceSpecificStatus1[6];
	uint8_t		m_u8deviceOperatingMode;
	uint8_t		m_u8standardizedStatus0;
	uint8_t		m_u8standardizedStatus1;
	uint8_t		m_u8analogChannelSaturatedCode;
	uint8_t		m_u8standardizedStatus2;
	uint8_t		m_u8standardizedStatus3;
	uint8_t		m_u8analogChannelFixedCode;
	uint8_t		m_u8deviceSpecificStatus2[11];

	// variable for NM session
	uint8_t		m_u8GranularityKeepAlive;

	CHartCmdWrapperList 	m_oDeviceInitCmds;
	typedef std::map< WHartUniqueID, CHartCmdWrapperList , CompareUniqueID> TargetedCmdsUniqueIdMap;
	TargetedCmdsUniqueIdMap m_oDeviceTargetedCmds;

	// List for Specific Burst Commands - for Gateway Caching
	CHartCmdWrapperList	m_oSpecificBurstCmds;

	// List for Specific Read Commands - for Gateway Caching
	CHartCmdWrapperList	m_oSpecificReadCmds;

};


} // namespace gateway
} // namespace hart7



#endif /* GATEWAYCONFIG_H_ */
