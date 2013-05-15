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

#ifndef _AP_CONFIG_H_
#define _AP_CONFIG_H_

// $Id: Cfg.h,v 1.12.4.1 2013/05/15 19:19:17 owlman Exp $

//////////////////////////////////////////////////////////////////////////////
/// @author	Marius Negreanu
/// @date	Mon Dec  8 13:09:52 EET 2008
/// @brief	AccessPoint Configuration Interface.
//////////////////////////////////////////////////////////////////////////////

#include "Shared/Config.h"
#include "Shared/h.h"


//////////////////////////////////////////////////////////////////////////////
/// @class	CWHAccessPointCfg
/// @ingroup AccessPoint
/// @brief	AccessPoint Configuration Interface.
//////////////////////////////////////////////////////////////////////////////
class CWHAccessPointCfg : CConfig
{
public:
	CWHAccessPointCfg();
	virtual ~CWHAccessPointCfg();
public:
	int	Init();
	bool	 RawLog()const	{ return m_bRawLog; }

public:
	char	 m_szSM_IPv4[16] ;	///< Security Manager IPv4 address
	uint16_t m_nSM_Port ;		///< Security Manager UDP Port

	char	 m_szGW_IPv4[16] ;	///< Security Manager IPv4 address
	uint16_t m_nGW_Port ;		///< Gateway UDP Port

	char	 m_szTtyDev[256] ;	///< AccessPoint Serial Link Device
	int	m_nTtyBauds ;			///< AccessPoint Serial Link Bauds

	int	 m_nUdpTimeout ;		///< Gateway Expiration Timeot( miliseconds )
	int	 m_nTtyTimeout;			///< Transceiver Expiration Timeout( miliseconds )

	char	 m_szIPv4[16] ;		///< AccessPoint IPv4 Address
	uint16_t m_nPort ;			///< AccessPoint UDP Port

	bool	m_bRawLog ;			///< To use or not to use RawLog.
	bool	m_bSendDiscOnReset; /// send disconnect packages on transciever reset

	int		m_nTrPowerID;
	int 	m_nTrMaxInactivity;

	uint16_t m_nNetworkId ;
	uint8_t  m_JoinKey[16] ;
	uint8_t	 m_EUI64[8] ;
	uint8_t	 m_LongTag[32] ;

private:
	bool getAddrTupple( const char* module, const char* opt, char* ipv4, uint16_t& port );
};


#endif // _AP_CONFIG_H_
