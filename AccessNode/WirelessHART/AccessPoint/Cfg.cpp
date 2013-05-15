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

// $Id: Cfg.cpp,v 1.19.4.1 2013/05/15 19:19:17 owlman Exp $

//////////////////////////////////////////////////////////////////////////////
/// @author	Marius Negreanu
/// @date	Thu Dec 18 13:40:22 2008 UTC
//////////////////////////////////////////////////////////////////////////////

#include <ctype.h>
#include <netinet/in.h>
#include <termios.h>

#include "Cfg.h"

CWHAccessPointCfg::CWHAccessPointCfg()
{
	memset( this, 0, sizeof(*this) ); // no virtual functions present so we memset(this,0)
}

CWHAccessPointCfg::~CWHAccessPointCfg()
{
	;
}


#define READ_BUF_SIZE 256

int CWHAccessPointCfg::Init()
{
	/// @todo check for potential conflicts between  this CIniParser::Load
	/// and the CIniParser::Load which occurs in CConfig::Init
	CConfig::Init("ACCESS_POINT", INI_FILE);
	if( !CIniParser::Load(INI_FILE) )
		return 0;

	//if ( ! getAddrTupple( "MANAGER", "MANAGER", m_szSM_IPv4, m_nSM_Port ) )
	//{
	//	ERR("Error at [MANAGER].MANAGER");
	//	return 0 ;
	//}

	if ( ! getAddrTupple( "WH_GATEWAY", "GATEWAY", m_szGW_IPv4, m_nGW_Port ) )
	{
		ERR("Error at [WH_GATEWAY].GATEWAY");
		return 0 ;
	}
	LOG("GW addr: %s:%u", m_szGW_IPv4, m_nGW_Port );

	if ( ! CIniParser::GetVar("ACCESS_POINT", "TTY_DEV", m_szTtyDev, READ_BUF_SIZE) )
	{
		ERR("Error at [ACCESS_POINT].TTY_DEV");
		return 0 ;
	}
	if ( ! CConfig::GetVarAsBaudRate("TTY_BAUDS", &m_nTtyBauds ) )
	{
		ERR("Error at [ACCESS_POINT].TTY_BAUDS");
		return 0 ;
	}
	if ( ! CConfig::GetVar("UDP_TIMEOUT", &m_nUdpTimeout ) )
	{
		ERR("Error at [ACCESS_POINT].UDP_TIMEOUT");
		return 0 ;
	}
	if ( ! CConfig::GetVar("TTY_TIMEOUT", &m_nTtyTimeout ) )
	{
		ERR("Error at [ACCESS_POINT].TTY_TIMEOUT");
		return 0 ;
	}
	if ( ! getAddrTupple( "ACCESS_POINT", "ACCESS_POINT", m_szIPv4, m_nPort ) )
	{
		ERR("Error at [ACCESS_POINT].ACCESS_POINT");
		return 0 ;
	}
	LOG("AP addr: %s:%u", m_szIPv4, m_nPort );

	if ( !CConfig::GetVar("NETWORK_ID", (uint8_t*)&m_nNetworkId , sizeof(m_nNetworkId)) )
	{
		ERR("Error at [ACCESS_POINT].NETWORK_ID");
		return 0;
	}

	if( !CConfig::GetVar("AppJoinKey", m_JoinKey, sizeof(m_JoinKey)))
	{
		ERR("Error at [ACCESS_POINT].AppJoinKey.");
		return 0;
	}
	LOG_HEX("AppJoinKey: ", m_JoinKey, 16);

	char buf[256];
	if( !CIniParser::GetVar("ACCESS_POINT", "AP_EUI64", buf, 256))
	{
		ERR("Error at [ACCESS_POINT].AP_EUI64.");
		return 0;
	}
	unsigned int tempBuf[8];

	sscanf( buf, "%x-%x-%x-%x-%x-%x-%x-%x", tempBuf, tempBuf + 1, tempBuf + 2, tempBuf + 3,
			tempBuf + 4, tempBuf + 5, tempBuf + 6, tempBuf + 7);
	for (int i = 0; i < 8; i++) { m_EUI64[i] = (uint8_t)tempBuf[i]; }

	LOG_HEX("AP_EUI64: ", m_EUI64, 8);

	char temp[256];

	if (!CIniParser::GetVar("ACCESS_POINT", "AP_TAG", temp, 256))
	{
		ERR("Error at [ACCESS_POINT].AP_TAG.");
		return 0;
	}

	memcpy(m_LongTag, temp, 32);

	LOG_HEX("AP_TAG: ", m_LongTag, 32);

	READ_DEFAULT_VARIABLE_YES_NO( "RAW_LOG", m_bRawLog, "NO" );
	READ_DEFAULT_VARIABLE_YES_NO( "SEND_DISCONNECT_ON_RESET", m_bSendDiscOnReset, "YES" );
	READ_DEFAULT_VARIABLE_INT("TR_POWER_ID", m_nTrPowerID, 1);
	READ_DEFAULT_VARIABLE_INT("TR_MAX_INACTIVITY", m_nTrMaxInactivity, 30);

	return 1;
}

//////////////////////////////////////////////////////////////////////////////
///// @brief Read a tuple like 10.0.0.1,2001
///// @retval 0 error
///// @retval !0 success
///// @param [in] module CConfig Module.
///// @param [in] option CConfig Option.
///// @param [out] ipv4  The IP address that was read.
///// @param [out] port  The port that was read.  
////////////////////////////////////////////////////////////////////////////////
bool CWHAccessPointCfg::getAddrTupple( const char *module, const char *option, char* ipv4, uint16_t& port )
{
	#define ADDR_TUPLE_FIELDS 5
	char buf[READ_BUF_SIZE] ;
	int a[ADDR_TUPLE_FIELDS];

	int rv = CIniParser::GetVar(module, option, buf, READ_BUF_SIZE );
	if ( ! rv ) { ERR("Error at %s:%s", module, option); return 0 ; }

	rv = sscanf( buf, "%u.%u.%u.%u %*[:,] %u", a, a+1, a+2, a+3, a+4 );
	if ( rv != ADDR_TUPLE_FIELDS ) {
		ERR("Invalid tuple.");
		return false ;
	}
	port = a[ADDR_TUPLE_FIELDS-1] ;

	if ( ipv4 ) snprintf( ipv4, 15, "%u.%u.%u.%u", a[0],a[1], a[2], a[3] ) ;
	return  true;
}
