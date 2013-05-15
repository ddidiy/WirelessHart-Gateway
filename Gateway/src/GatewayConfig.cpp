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

#include <WHartGateway/GatewayConfig.h>
#include <sys/file.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <Shared/Utils.h>

#define FILE_PATH 			NIVIS_PROFILE"gw_ini_cmds.ini"
#define FILE_PATH_CONFIGINI 		NIVIS_PROFILE"config.ini"
#define FILE_PATH_GW_INFO_UNIV  	NIVIS_PROFILE"gw_info_univ.ini"
extern void EnableLog(int level);

namespace hart7 {
namespace gateway {

GatewayConfig::GatewayConfig()
{
    memset(m_szLongTag, 0, sizeof(m_szLongTag));
    memset(m_szTag, 0, sizeof(m_szTag));
    memset(m_szCmdUniversalMessage, 0, sizeof(m_szCmdUniversalMessage));
    m_bResetFlag = false;
    m_bNmBurstsCachingEnabled = true;
    m_bUseSubdevPollingAddresses = true;

    m_bSendInvalidRequestToDevice = false;
}

bool GatewayConfig::Init()
{
    LOG("-------------------GATEWAY CONFIG INIT--------------------");

    if (!ReadGWVariables())
    {
        return false;
    }

    if (!RuntimeReload())
    {
        return false;
    }
    InitGwCmds();

    return true;
}

// read from gw_ini_cmds.ini file
// initialising for target = all and when = dev_init
bool GatewayConfig::InitGwCmds()
{
    WHartUniqueID devUniqueID;
    m_oDeviceInitCmds.clear();
    m_oDeviceTargetedCmds.clear();
    CIniParser gw_commands_parser;

    if (!gw_commands_parser.Load(FILE_PATH))
    {
        return 0;
    }

    for (int i = 0; gw_commands_parser.FindGroup("command", i); i++)
    {
        char target[255];
        char when[255];
        int target_len;
        int cmd_id;
        unsigned char cmd_data[256];
        int cmd_len;
        uint8_t output[5];

        if (gw_commands_parser.GetVar(NULL, "when", when, sizeof(when)) <= 0)
            continue;

        if (strcmp(when, "dev_init") != 0)
        {
            continue;
        }

        if (gw_commands_parser.GetVar(NULL, "target", target, sizeof(target)) <= 0)
            continue;

        if (!gw_commands_parser.GetVar(NULL, "cmd_id", &cmd_id))
            continue;

        if ((cmd_len = gw_commands_parser.GetVar(NULL, "cmd_data", cmd_data, sizeof(cmd_data))) < 0)
            continue;

        if (strcmp(target, "all") == 0)
        {
            // new element in Cmds List
            CHartCmdWrapper::Ptr new_element(new CHartCmdWrapper);

            new_element->LoadRaw((uint16_t) cmd_id, cmd_len, cmd_data);
            m_oDeviceInitCmds.push_back(new_element);
        }
        else
        {
            target_len = Hex2Bin(target, (char *) output);

            if (target_len != sizeof(devUniqueID.bytes))
            {
                LOG_INFO("target=" << target << " inavlid len=" << target_len);
                continue;
            }

            memcpy(devUniqueID.bytes, output, target_len);

            CHartCmdWrapper::Ptr new_element(new CHartCmdWrapper);
            new_element->LoadRaw((uint16_t) cmd_id, cmd_len, cmd_data);

            m_oDeviceTargetedCmds[devUniqueID].push_back(new_element);
        }
        LOG(" target=%s cmdId=%d  data=%s", target, cmd_id, GetHex(cmd_data, cmd_len, ' '));

    }

    return true;
}

// write CmdUniversalMessage to config_universal.ini file

bool GatewayConfig::WriteCmdUniversalMessage(char * universalmessage)
{
    CIniParser gwVarConfig;
    if (!gwVarConfig.Load(FILE_PATH_GW_INFO_UNIV, "r+"))
    {
        LOG("------- NO CONFIG.INI FILE -------");
        return 0;
    }

    if (!gwVarConfig.FindGroup("WH_GATEWAY", true))
    {
        return 0;
    }

    if (!gwVarConfig.SetVar("WH_GATEWAY", "CMD_UNIVERSAL_MESSAGE", universalmessage))
    {
        return 0;
    }

    return 1;
}

// write szTag to config_universal.ini file

bool GatewayConfig::WriteCmdTag(char * tag)
{
    CIniParser gwVarConfig;
    if (!gwVarConfig.Load(FILE_PATH_GW_INFO_UNIV, "r+"))
    {
        LOG("-------NO CONFIG.INI FILE -------");
        return 0;
    }

    if (!gwVarConfig.FindGroup("WH_GATEWAY", true))
    {
        return 0;
    }

    else
    {
        if (!gwVarConfig.SetVar("WH_GATEWAY", "TAG", tag))
            return 0;

    }
    return 1;

}

// write m_szMasterDescriptor to config_universal.ini file

bool GatewayConfig::WriteCmdMasterDescriptor(char * masterDescriptor)
{
    CIniParser gwVarConfig;
    if (!gwVarConfig.Load(FILE_PATH_GW_INFO_UNIV, "r+"))
    {
        LOG("------- NO CONFIG.INI FILE -------");
        return 0;
    }

    if (!gwVarConfig.FindGroup("WH_GATEWAY", true))
    {
        return 0;
    }

    else
    {
        if (!gwVarConfig.SetVar("WH_GATEWAY", "MASTER_DESCRIPTOR", masterDescriptor))
            return 0;

    }
    return 1;

}

// write m_stMasterDate to config _universal.ini file

bool GatewayConfig::WriteCmdMasterDate(WHartDate masterDate)
{
    CIniParser gwVarConfig;
    if (!gwVarConfig.Load(FILE_PATH_GW_INFO_UNIV, "r+"))
    {
        LOG("-------NO CONFIG.INI FILE -------");
        return 0;
    }

    if (!gwVarConfig.FindGroup("WH_GATEWAY", true))
    {
        return 0;
    }

    else
    {
        int date_day;
        int date_month;
        int date_year;

        date_day = (int) masterDate.day;
        date_month = (int) masterDate.month;
        date_year = (int) masterDate.year;

        if (!gwVarConfig.SetVar("WH_GATEWAY", "MASTER_DATE_DAY", date_day))
            return 0;

        if (!gwVarConfig.SetVar("WH_GATEWAY", "MASTER_DATE_MONTH", date_month))
            return 0;

        if (!gwVarConfig.SetVar("WH_GATEWAY", "MASTER_DATE_YEAR", date_year))
            return 0;
    }
    return 1;

}

// write Command Final Assembly Number to config_universal.ini file
bool GatewayConfig::WriteCmdFinalAssemblyNumber(uint32_t finalAssemblyNumber)
{
    CIniParser gwVarConfig;
    if (!gwVarConfig.Load(FILE_PATH_GW_INFO_UNIV, "r+"))
    {
        LOG("------- NO CONFIG.INI FILE -------");
        return 0;
    }

    if (!gwVarConfig.FindGroup("WH_GATEWAY", true))
    {
        return 0;
    }

    else
    {
        int finAssemblyNumber;
        finAssemblyNumber = (int) finalAssemblyNumber;
        if (!gwVarConfig.SetVar("WH_GATEWAY", "FINAL_ASSEMBLY_NUMBER", finAssemblyNumber))
            return 0;

    }
    return 1;

}

bool GatewayConfig::WriteMinRespPreamblesNo(uint8_t p_u8MinRespPreamblesNo)
{
    CIniParser gwVarConfig;
    if (!gwVarConfig.Load(FILE_PATH_GW_INFO_UNIV, "r+"))
    {
        LOG("------- NO CONFIG.INI FILE -------");
        return 0;
    }

    if (!gwVarConfig.FindGroup("WH_GATEWAY", true))
    {
        return 0;
    }

    int minRespPreamblesNo = p_u8MinRespPreamblesNo;

    if (!gwVarConfig.SetVar("WH_GATEWAY", "MIN_RESP_PREAMBLES_NO", minRespPreamblesNo))
    {
        return 0;
    }

    return 1;
}

// write Command Long Tag to config_universal.ini file

bool GatewayConfig::WriteCmdLongTag(char * longTag)
{

    CIniParser gwVarConfig;
    if (!gwVarConfig.Load(FILE_PATH_GW_INFO_UNIV, "r+"))
    {
        LOG("------- NO CONFIG.INI FILE -------");
        return 0;
    }

    if (!gwVarConfig.FindGroup("WH_GATEWAY", true))
    {
        return 0;
    }

    else
    {
        if (!gwVarConfig.SetVar("WH_GATEWAY", "LONG_TAG", longTag))
            return 0;

    }
    return 1;

}

// write Device Status to config_universal.ini file
// TODO: Beni - save Device Status on C038_ResetConfigurationChangedFlag
bool GatewayConfig::WriteDeviceStatus(uint8_t deviceStatus)
{
    CIniParser gwVarConfig;
    if (!gwVarConfig.Load(FILE_PATH_GW_INFO_UNIV, "r+"))
    {
        LOG("------- NO CONFIG.INI FILE -------");
        return 0;
    }

    if (!gwVarConfig.SetVar("WH_GATEWAY", "DEVICE_STATUS", deviceStatus))
        return 0;

    m_u8DeviceStatus = deviceStatus;

    return 1;
}

void GatewayConfig::DeviceStatus(uint8_t u8DeviceStatus)
{
    LOG_INFO("OLD u8DeviceStatus=" << (int)m_u8DeviceStatus << ", NEW u8DeviceStatus=" << (int)u8DeviceStatus);

    m_u8DeviceStatus = u8DeviceStatus;
}

uint8_t GatewayConfig::DeviceStatus()
{
    // LOG_DEBUG("GET DeviceStatus - CURRENT DeviceStatus=" << (int)m_u8DeviceStatus);
    return m_u8DeviceStatus;
}

bool GatewayConfig::WriteGwReqMaxRetryNo(uint8_t p_u8RetryNo)
{
    CIniParser gwVarConfig;
    if (!gwVarConfig.Load(FILE_PATH_CONFIGINI, "r+"))
    {
        LOG("------- NO CONFIG.INI FILE -------");
        return 0;
    }

    if (!gwVarConfig.FindGroup("WH_GATEWAY", true))
    {
        return 0;
    }

    if (!gwVarConfig.SetVar("WH_GATEWAY", "GW_REQ_MAX_RETRY_NO", p_u8RetryNo, 0, true))
    {
        return 0;
    }
    return 1;
}

// read variables from config_universal.ini file

bool GatewayConfig::ReadGWUniversalVariables()
{
    CIniParser gwVarConfig;

    if (!gwVarConfig.Load(FILE_PATH_GW_INFO_UNIV))
    {
        LOG("------- NO CONFIG.INI FILE -------");
        return 0;
    }

    if (!gwVarConfig.FindGroup("WH_GATEWAY", true))
    {
        return 0;
    }

    // variables for C000

    int min_req_preambles_no;
    if (!(gwVarConfig.GetVar("WH_GATEWAY", "MIN_REQ_PREAMBLES_NO", &min_req_preambles_no)))
        min_req_preambles_no = 0; // default value
    m_u8MinReqPreamblesNo = (uint8_t) min_req_preambles_no;

    int device_revision_level;
    if (!gwVarConfig.GetVar("WH_GATEWAY", "DEVICE_REVISION_LEVEL", &device_revision_level))
        device_revision_level = 0; // default value
    m_u8DevRevisionLevel = (uint8_t) device_revision_level;

    int sw_revision_no;
    if (!gwVarConfig.GetVar("WH_GATEWAY", "SOFTWARE_REVISION_LEVEL", &sw_revision_no))
        sw_revision_no = 0; // default value

    m_u8SoftwRevisionLevel = (uint8_t) sw_revision_no;

    int hwRevisionLevel_physicalSignalingMode;
    if (!gwVarConfig.GetVar("WH_GATEWAY", "HWREVLEVEL_PHYSIGNALMODE", &hwRevisionLevel_physicalSignalingMode))
        hwRevisionLevel_physicalSignalingMode = 0; // default value

    m_u8HWRevisionLevel_PhysicalSignCode = (uint8_t) hwRevisionLevel_physicalSignalingMode;

    if (!(gwVarConfig.GetVar("WH_GATEWAY", "FLAGS", &m_u8Flags, 1) == 1))
        m_u8HWRevisionLevel_PhysicalSignCode = 0; // default value


    int min_resp_preambles_no;
    if (!(gwVarConfig.GetVar("WH_GATEWAY", "MIN_RESP_PREAMBLES_NO", &min_resp_preambles_no)))
        min_resp_preambles_no = 0; // default value
    m_u8MinRespPreamblesNo = (uint8_t) min_resp_preambles_no;

    int max_no_of_devices_var;
    if (!(gwVarConfig.GetVar("WH_GATEWAY", "MAX_NO_OF_DEVICES_VARS", &max_no_of_devices_var)))
        max_no_of_devices_var = 0; // default value
    m_u8MaxNoOfDevicesVar = (uint8_t) max_no_of_devices_var;

    int configChangeCounter;
    if (!(gwVarConfig.GetVar("WH_GATEWAY", "CHANGE_COUNTER", &configChangeCounter)))
        configChangeCounter = 0; // default value
    m_u16ConfigChangeCounter = (uint16_t) configChangeCounter;

    int field_device_status;
    if (!gwVarConfig.GetVar("WH_GATEWAY", "EXTENDED_STATUS", &field_device_status))
        field_device_status = 0; // default value

    m_u8FlagBits = (uint8_t) field_device_status;

    if (!(gwVarConfig.GetVar("WH_GATEWAY", "MANUFACT_ID_CODE", m_u8ManufactCode, sizeof(m_u8ManufactCode))
                == sizeof(m_u8ManufactCode)))
        memset(m_u8ManufactCode, 0, sizeof(m_u8ManufactCode)); // default value

    if (!(gwVarConfig.GetVar("WH_GATEWAY", "PRIVATE_LABEL_CODE", m_u8ManufactLabel, sizeof(m_u8ManufactLabel))
                == sizeof(m_u8ManufactLabel)))
        memset(m_u8ManufactLabel, 0, sizeof(m_u8ManufactLabel)); // default value
    // end of variables for C000

    // variables for C020 and C022
    char long_tag[33];
    if (!(gwVarConfig.GetVar("WH_GATEWAY", "LONG_TAG", long_tag, sizeof(long_tag)) > 0))
        memset(long_tag, 0, sizeof(long_tag)); //default value
    memcpy(m_szLongTag, long_tag, sizeof(m_szLongTag));

    // variables for C016 and C019
    int finalAssemblyNumber;
    if (!gwVarConfig.GetVar("WH_GATEWAY", "FINAL_ASSEMBLY_NUMBER", &finalAssemblyNumber))
        finalAssemblyNumber = 0; //default value
    memcpy(&m_u32FinalAssemblyNumber, &finalAssemblyNumber, sizeof(m_u32FinalAssemblyNumber));

    // variables for commands 013 and 018
    char tag[9];
    if (!(gwVarConfig.GetVar("WH_GATEWAY", "TAG", tag, sizeof(tag)) > 0))
        memset(tag, 0, sizeof(tag));
    memcpy(m_szTag, tag, sizeof(m_szTag));

    if (!(gwVarConfig.GetVar("WH_GATEWAY", "MASTER_DESCRIPTOR", m_szMasterDescriptor, sizeof(m_szMasterDescriptor)) > 0))
        memset(m_szMasterDescriptor, 0, sizeof(m_szMasterDescriptor)); // default value

    int master_date_day;
    int master_date_month;
    int master_date_year;

    if (!gwVarConfig.GetVar("WH_GATEWAY", "MASTER_DATE_DAY", &master_date_day))
        master_date_day = 0; // default value

    if (!gwVarConfig.GetVar("WH_GATEWAY", "MASTER_DATE_MONTH", &master_date_month))
        master_date_month = 0; // default value

    if (!gwVarConfig.GetVar("WH_GATEWAY", "MASTER_DATE_YEAR", &master_date_year))
        master_date_year = 0; // default value

    m_stMasterDate.day = (uint8_t) master_date_day;
    m_stMasterDate.month = (uint8_t) master_date_month;
    m_stMasterDate.year = (uint8_t) master_date_year;

    // variables for C012/C017
    if (!(gwVarConfig.GetVar("WH_GATEWAY", "CMD_UNIVERSAL_MESSAGE", m_szCmdUniversalMessage,
                             sizeof(m_szCmdUniversalMessage)) > 0))
        memset(m_szCmdUniversalMessage, 0, sizeof(m_szCmdUniversalMessage)); // default value

    // variables for C048

    if (!(gwVarConfig.GetVar("WH_GATEWAY", "DEVICE_SPECIFIC_STATUS1", m_u8deviceSpecificStatus1,
                             sizeof(m_u8deviceSpecificStatus1)) == sizeof(m_u8deviceSpecificStatus1)))
        memset(m_u8deviceSpecificStatus1, 0, sizeof(m_u8deviceSpecificStatus1)); // default value

    int deviceOperatingMode;
    if (!gwVarConfig.GetVar("WH_GATEWAY", "DEVICE_OPERATING_MODE", &deviceOperatingMode))
        deviceOperatingMode = 0; // default value
    m_u8deviceOperatingMode = (uint8_t) deviceOperatingMode;

    int standardizedStatus0;
    if (!gwVarConfig.GetVar("WH_GATEWAY", "STANDARDIZED_STATUS0", &standardizedStatus0))
        standardizedStatus0 = 0; // default value
    m_u8standardizedStatus0 = (uint8_t) standardizedStatus0;

    int standardizedStatus1;
    if (!gwVarConfig.GetVar("WH_GATEWAY", "STANDARDIZED_STATUS1", &standardizedStatus1))
        standardizedStatus1 = 0; // default value
    m_u8standardizedStatus1 = (uint8_t) standardizedStatus1;

    int analogChannelSaturatedCode;
    if (!gwVarConfig.GetVar("WH_GATEWAY", "ANALOG_CHANNEL_SATURATED_CODE", &analogChannelSaturatedCode))
        analogChannelSaturatedCode = 0; // default value
    m_u8analogChannelSaturatedCode = (uint8_t) analogChannelSaturatedCode;

    int standardizedStatus2;
    if (!gwVarConfig.GetVar("WH_GATEWAY", "STANDARDIZED_STATUS2", &standardizedStatus2))
        standardizedStatus2 = 0; // default value
    m_u8standardizedStatus2 = (uint8_t) standardizedStatus2;

    int standardizedStatus3;
    if (!gwVarConfig.GetVar("WH_GATEWAY", "STANDARDIZED_STATUS3", &standardizedStatus3))
        standardizedStatus3 = 0; // default value
    m_u8standardizedStatus3 = (uint8_t) standardizedStatus3;

    int analogChannelFixedCode;
    if (!gwVarConfig.GetVar("WH_GATEWAY", "ANALOG_CHANNEL_FIXED_CODE", &analogChannelFixedCode))
        analogChannelFixedCode = 0; // default value
    m_u8analogChannelFixedCode = (uint8_t) analogChannelFixedCode;

    if (!(gwVarConfig.GetVar("WH_GATEWAY", "DEVICE_SPECIFIC_STATUS2", m_u8deviceSpecificStatus2,
                             sizeof(m_u8deviceSpecificStatus2)) == sizeof(m_u8deviceSpecificStatus2)))
        memset(m_u8deviceSpecificStatus2, 0, sizeof(m_u8deviceSpecificStatus2));

    // variable used for C038
    // TODO: Beni - save Device Status on C038_ResetConfigurationChangedFlag
    m_u8DeviceStatus = 0;
    if (!(gwVarConfig.GetVar("WH_GATEWAY", "DEVICE_STATUS", &m_u8DeviceStatus, sizeof(m_u8DeviceStatus))))
        m_u8DeviceStatus = 0; // default value

    return true;
}

// read variables from config.ini file
bool GatewayConfig::ReadGWVariables()
{
    // **********************CIniParser************************//

    CIniParser gwVarConfig;
    net_address stNetAddr;

    if (!gwVarConfig.Load(FILE_PATH_CONFIGINI))
    {
        LOG("------- NO CONFIG.INI FILE -------");
        return 0;
    }

    if (!gwVarConfig.GetVar("NETWORK_MANAGER", "NETWORK_MANAGER", &stNetAddr))
        return 0;

    m_nNetworkManagerPort = ntohs(stNetAddr.m_dwPortNo);
    strcpy(m_szNetworkManagerHost, inet_ntoa(*(in_addr*) &stNetAddr.m_nIP));

    // ****************CConfig**********************************//

    if (!CConfig::Init("WH_GATEWAY", FILE_PATH_CONFIGINI ))
    {
        LOG("-------------NO CONFIG.INI FILE--------------");
        return 0;
    }

    READ_MANDATORY_VARIABLE("GATEWAY", stNetAddr);

    m_nListenAccessPointPort = ntohs(stNetAddr.m_dwPortNo);
    m_u32IpAddress = stNetAddr.m_nIP;

    READ_MANDATORY_VARIABLE("NM_CLIENT_LISTEN_PORT",m_nNmClientListenPort );

    //READ_MANDATORY_VARIABLE ("LISTEN_AP_PORT", m_nListenAccessPointPort);

    READ_MANDATORY_VARIABLE ("AP_CLIENT_MIN_PORT", m_nAccessPointClientMinPort);

    READ_MANDATORY_VARIABLE ("AP_CLIENT_MAX_PORT", m_nAccessPointClientMaxPort);

    READ_MANDATORY_VARIABLE ("HOSTAPP_LISTEN_PORT", m_nHostApp_ListenPort);

    READ_MANDATORY_VARIABLE ("HOSTAPP_MIN_PORT", m_nHostApp_MinPort);

    READ_MANDATORY_VARIABLE("HOSTAPP_MAX_PORT", m_nHostApp_MaxPort);

    READ_DEFAULT_VARIABLE_INT("GW_REQUEST_SERVICE",m_nGwRequestService, 0);

    READ_DEFAULT_VARIABLE_FLOAT("BURST_MODE_NOTIFICATION_RATE", m_fBurstNotificationRate, 0.25);
    m_fBurstNotificationRate *= 1000; //miliseconds
    READ_DEFAULT_VARIABLE_FLOAT("EVENT_NOTIFICATION_RATE", m_fEventNotificationRate, 1);
    m_fEventNotificationRate *= 1000; //miliseconds
    READ_DEFAULT_VARIABLE_FLOAT("DEVICE_STATUS_NOTIFICATION_RATE", m_fDeviceStatusNotificationRate, 5);
    m_fDeviceStatusNotificationRate *= 1000; //miliseconds
    READ_DEFAULT_VARIABLE_FLOAT("DEVICE_CONFIGURATION_NOTIFICATION_RATE", m_fDeviceConfigurationNotificationRate, 5);
    m_fDeviceConfigurationNotificationRate *= 1000; //miliseconds
    READ_DEFAULT_VARIABLE_FLOAT("TOPOLOGY_NOTIFICATION_RATE", m_fTopologyNotificationRate, 5);
    m_fTopologyNotificationRate *= 1000; //miliseconds
    READ_DEFAULT_VARIABLE_FLOAT("SCHEDULE_NOTIFICATION_RATE", m_fScheduleNotificationRate, 5);
    m_fScheduleNotificationRate *= 1000; //miliseconds

    READ_DEFAULT_VARIABLE_INT("GW_MOCK_MODE",m_nMockMode, 0);
    READ_DEFAULT_VARIABLE_INT("SEND_ID_INFO_CMDS",m_nSendIdInfoCmds, 1);

    return 1;
}

bool GatewayConfig::RuntimeReload()
{
    READ_MANDATORY_VARIABLE_STRING("AppJoinKey", m_u8AppJoinKey);

    READ_MANDATORY_VARIABLE("GW_REQ_TIMEOUT", m_nGwReqTimeout);
    m_nGwReqTimeout *= sysconf(_SC_CLK_TCK);

    READ_DEFAULT_VARIABLE_INT("GW_REQ_RETRY_TIMEOUT", m_nGwReqRetryTimeout, 15);
    m_nGwReqRetryTimeout = m_nGwReqRetryTimeout * 60 * sysconf(_SC_CLK_TCK); // from minutes to clock ticks

    READ_DEFAULT_VARIABLE_INT("LOG_INTERNAL_STATUS_PERIOD", m_nLogInternalStatusPeriod, 0);
    READ_DEFAULT_VARIABLE_INT("LOG_INTERNAL_REQUEST_STATISTICS_PERIOD", m_nLogInternalRequestsStatisticsPeriod, 0);

    READ_MANDATORY_VARIABLE("CACHE_BURST_RESP_TIMEOUT", m_nGwBurstRespTimeout);
    READ_MANDATORY_VARIABLE("CACHE_READ_RESP_TIMEOUT", m_nGwRespTimeout);

    READ_MANDATORY_VARIABLE("GW_DRM_TIMEOUT", m_nGwDrmTimeout);
    m_nGwDrmTimeout *= sysconf(_SC_CLK_TCK);

    READ_MANDATORY_VARIABLE("GW_REQ_MAX_RETRY_NO", m_nGwReqMaxRetryNo);

    READ_DEFAULT_VARIABLE_INT("LOCAL_GW_RETRIES", m_nLocalGwRetries, 3);

    READ_DEFAULT_VARIABLE_INT("DEVICES_REFRESH_INTERVAL", m_nDevicesRefreshInterval, 5);

    READ_DEFAULT_VARIABLE_INT("LOG_LEVEL_STACK", m_nLogLevelStack, 4); //4-debug, 3-info
    READ_DEFAULT_VARIABLE_INT("LOG_LEVEL_APP", m_nLogLevelApp, 4); //4-debug, 3-info

    int granularity_keepalive;
    READ_DEFAULT_VARIABLE_INT("GRANULARITY_KEEPALIVE", granularity_keepalive, 60);
    m_u8GranularityKeepAlive = (uint8_t) granularity_keepalive;

    READ_DEFAULT_VARIABLE_INT("MAX_CMDS_PER_APDU", m_nMaxCmdsPerAPDU, 5);

    READ_DEFAULT_VARIABLE_YES_NO("NM_BURSTS_CACHING_ENABLED", m_bNmBurstsCachingEnabled, "YES");

    READ_DEFAULT_VARIABLE_YES_NO("USE_SUBDEV_POLLING_ADDRESSES", m_bUseSubdevPollingAddresses, "YES");

    READ_DEFAULT_VARIABLE_YES_NO("DONT_ACK_C119_WHEN_TIME_MINUS_1", m_bDontAckC119WhenTime_minus1, "NO");

    READ_DEFAULT_VARIABLE_YES_NO("SEND_DIRECT_WIRED_DEVICE_BURST", m_bSendDirectWiredDeviceBurst, "NO");
    READ_DEFAULT_VARIABLE_YES_NO("BUILD_UNIV_CMDS_CACHE", m_bBuildUnivCommandsCache, "NO");

    READ_DEFAULT_VARIABLE_INT("DRM_TYPE", m_nDrmType, GatewayConfig::DRM_TYPE_CACHE_BASED);

    // READ_DEFAULT_VARIABLE_STRING("GATEWAY_TAG", m_szTag, "GwTag" );

    READ_DEFAULT_VARIABLE_INT("DRM_ENTRY_LIFETIME", m_nDrmEntryLifetime, 1200);

    READ_DEFAULT_VARIABLE_YES_NO("SEND_INVALID_REQUEST_TO_DEVICE", m_bSendInvalidRequestToDevice, "NO");


    EnableLog(m_nLogLevelStack);//4-debug, 3-info
    EnableLog_APP(m_nLogLevelApp);//4-debug, 3-info

    LOG_INFO_APP("GatewayConfig DONE logLevel stack=" << m_nLogLevelStack << " app=" << m_nLogLevelApp);

    ReadSpecificCmds();

    BurstSpecificCmds();

    ReadGWUniversalVariables();

    return true;
}

bool GatewayConfig::IsSpecificBurstCmd(uint16_t p_u16CmdId)
{

    // specific burst cmds list
    CHartCmdWrapperList::iterator itSpecificBurstCmds = m_oSpecificBurstCmds.begin();

    for (; itSpecificBurstCmds != m_oSpecificBurstCmds.end(); itSpecificBurstCmds++)
    {
        CHartCmdWrapper::Ptr pSpecificCmds = *itSpecificBurstCmds;

        if (pSpecificCmds->GetCmdId() == p_u16CmdId)
        {
            return true;
        }

    }
    return false;
}

bool GatewayConfig::IsSpecificReadCmd(uint16_t p_u16CmdId)
{
    // specific read cmds list
    CHartCmdWrapperList::iterator itSpecificReadCmds = m_oSpecificReadCmds.begin();

    for (; itSpecificReadCmds != m_oSpecificReadCmds.end(); itSpecificReadCmds++)
    {
        CHartCmdWrapper::Ptr pSpecificCmds = *itSpecificReadCmds;

        if (pSpecificCmds->GetCmdId() == p_u16CmdId)
        {
            return true;
        }

    }
    return false;
}

// Specific Burst cmds for Gateway Caching
bool GatewayConfig::BurstSpecificCmds()
{
    m_oSpecificBurstCmds.clear();

    CIniParser specificCmdsConfig;
    CIniParser::valuesList burstSpecificCmds;
    if (!specificCmdsConfig.Load(FILE_PATH_CONFIGINI))
    {
        LOG("------- NO CONFIG.INI FILE -------");
        return 0;
    }

    if (!specificCmdsConfig.GetVar("WH_GATEWAY", "BURST_SPECIFIC_CMDS", &burstSpecificCmds))
    {
        return 0;
    }

    CIniParser::valuesList::iterator it = burstSpecificCmds.begin();

    for (; it != burstSpecificCmds.end(); it++)
    {
        CHartCmdWrapper::Ptr cmdTemp(new CHartCmdWrapper);

        cmdTemp->LoadRaw((*it), 0, NULL, 0);
        m_oSpecificBurstCmds.push_back(cmdTemp);

    }

    return 1;
}

//Specific Read Cmds for Gateway Caching
bool GatewayConfig::ReadSpecificCmds()
{
    m_oSpecificReadCmds.clear();
    // **********************CIniParser************************//

    CIniParser specificCmdsConfig;
    CIniParser::valuesList readSpecificCmds;
    if (!specificCmdsConfig.Load(FILE_PATH_CONFIGINI))
    {
        LOG("------- NO CONFIG.INI FILE -------");
        return false;
    }

    if (!specificCmdsConfig.GetVar("WH_GATEWAY", "READ_SPECIFIC_CMDS", &readSpecificCmds))
    {
        return false;
    }

    CIniParser::valuesList::iterator it = readSpecificCmds.begin();

    for (; it != readSpecificCmds.end(); it++)
    {
        CHartCmdWrapper::Ptr cmdTemp(new CHartCmdWrapper);

        cmdTemp->LoadRaw((*it), 0, NULL, 0);
        m_oSpecificReadCmds.push_back(cmdTemp);

        LOG("ReadSpecificCmds: vmdId=%d", (*it));
    }

    return 1;

}
} // namespace gateway
} // namespace hart7
