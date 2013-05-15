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

#ifndef _MISC_IMPL_H_
#define _MISC_IMPL_H_

#include <stdint.h>
#include <vector>
#include <arpa/inet.h> // inet_ntop

#include "../../../Shared/IniParser.h" /* MAX_LINE_LEN */

static const int MultiplyDeBruijnBitPosition[32] =
{
		0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
		31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
};

//////////////////////////////////////////////////////////////////////
/// @class CValidator
//////////////////////////////////////////////////////////////////////
class CValidator {
public:
	// return 0 if p_szIp is invalid
	// return uint32_t of p_szIp
	uint32_t Ip(const char* p_szIp, uint32_t p_uiMask);
	uint32_t Mask( const char *p_szMask );
	uint32_t Gateway( const char *p_szGw, uint32_t mask, uint32_t ip32 );
};

//////////////////////////////////////////////////////////////////////
/// @class CMiscImpl
//////////////////////////////////////////////////////////////////////
class CMiscImpl
{
	int m_nFd;
public:
	struct StatusSummary
	{
		uint16_t totalDevices;
		uint16_t onlineDevices;
		uint16_t offlineDevices;
		uint32_t alerts;
		uint16_t devicesNotCommissioned;
		time_t lastAlertReportTimestamp;
		time_t lastAuditReportTimestamp;
		uint8_t memoryUsagePercent ;
		const char*  batteryCharger ;
	};
	struct GatewayNetworkInfo
	{
		char	ip[MAX_LINE_LEN];
		char	mask[MAX_LINE_LEN];
		char	defgw[MAX_LINE_LEN];
		bool	bUpdateMAC;
		char	mac0[MAX_LINE_LEN];
		char	mac1[MAX_LINE_LEN];
		bool	bDhcpEnabled;
		char*	nameservers[256];
	};

	struct GprsProviderInfo
	{
		char	m_szMCC[16];
		char	m_szMNC[16];
		char	m_szProviderTag	[MAX_LINE_LEN];
		char	m_szCountry	[MAX_LINE_LEN];
		char	m_szAPN[MAX_LINE_LEN];
		char	m_szDialNo[MAX_LINE_LEN];
		char	m_szUser[MAX_LINE_LEN];
		char	m_szPass[MAX_LINE_LEN];
	};

	#define AN_ID_LEN 9

	// ER Protocol (Discovery) {

	#define ERPROTO_MAC_LEN			6
	#define ERPROTO_MSG_VER			1
	#define ERPROTO_BROADCAST_ADDR	"255.255.255.255"
	#define ERPROTO_CRC_LEN			sizeof(unsigned short)
	#define ERPROTO_BUFF_LEN		128
	#define ERPROTO_CRC_TABLE_LEN	256
	#define ERPROTO_PORT			10000
	#define ERPROTO_SEARCH_TIMEOUT 	1000

	enum  ERProto_MsgType {
		ERPROTO_MSG_REQ = 1,
		ERPROTO_MSG_RSP,
	};

	struct ERProtoPkHeader {
		uint8_t m_u8_Version;
		uint8_t m_u8_Type;

		ERProtoPkHeader();

	} __attribute__((packed));

	struct ERProtoReqPk {
		ERProtoPkHeader m_oHdr;
		uint16_t m_u16_CRC;

		ERProtoReqPk();

	} __attribute__((packed));

	struct ERProtoRspPk {
		ERProtoPkHeader m_oHdr;
		uint32_t m_u32_ANID;
		uint8_t m_aMAC[ERPROTO_MAC_LEN];
		uint32_t m_u32_IP;
		uint32_t m_u32_Mask;
		uint32_t m_u32_DHCPIP;
		uint32_t m_u32_DHCPMask;
		uint32_t m_u32_KnownIP;
		uint32_t m_u32_KnownMask;
		uint32_t m_u32_Gateway;
		uint16_t m_u16_CRC;

		void NTOH();

	} __attribute__((packed));

	// ER Protocol (Discovery) }

	struct DiscoveryInfo
	{
		char m_szAnID[AN_ID_LEN];
		char m_szStaticIP[INET_ADDRSTRLEN];
		char m_szDynamicIP[INET_ADDRSTRLEN];
		DiscoveryInfo(	uint32_t p_u32_anID,
			uint32_t p_u32_staticIP,
			uint32_t p_u32_dynamicIP);
	};


public:
	char*getVersion(void);
	bool fileUpload(const char *p_szCmd, const char*p_kszCmdParam, char *& p_szCmdRsp) ;
	bool fileDownload(const char *p_szCmd, const char*p_kszCmdParam, char *& p_szCmdRsp) ;

	bool getGatewayNetworkInfo( GatewayNetworkInfo& p_rGatewayNetworkInfo );
	bool setGatewayNetworkInfo( GatewayNetworkInfo& p_rGatewayNetworkInfo, int*status );
	// return the ntp servers
	bool getNtpServers( std::vector<char*>& servers ) ;
	// set an array of ntp servers
	bool setNtpServers( const std::vector<const char*>& servers ) ;

	bool restartApp( const char * p_szAppName, const char * p_szAppParams );
    bool softwareReset( );
	bool hardReset( );
	bool applyConfigChanges( const char * p_szModule, const char * p_szSignal ) ;
	bool discoverRouters( std::vector<DiscoveryInfo>& routerList ) ;
	bool execCmd(const char *p_szCmd, char *& p_szCmdRsp);

private:
	bool lockUpgradeFile(void);
	bool unlockUpgradeFile(void);

	// ER Protocol (Discovery) {
	unsigned short static ERProtoCRC(unsigned char*, unsigned int);
	// ER Protocol (Discovery) }

} ;
#endif	/* _MISC_IMPL_H_ */
