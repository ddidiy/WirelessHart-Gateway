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

#ifndef _DISCOVERYCONFIG_H_
#define _DISCOVERYCONFIG_H_
//////////////////////////////////////////////////////////////////////////////
/// @author	Catrina Mihailescu
/// @date	Mon Oct  19 14:16:52 EET 2009
/// @brief	Discovery Process Configuration. 
//////////////////////////////////////////////////////////////////////////////

#include "../../Shared/Config.h"
#include "../../Shared/h.h"

#define	GATEWAY_VERSION			1
#define GATEWAY_RESERVED_BYTE 		0
#define COMMAND0_FIRSTBYTE 		254
#define COMMAND0_REVISION_NO		7
#define COMMAND0_DEVICEID		0x000002
#define COMMAND0_DEVICE_PROFILE		132



//////////////////////////////////////////////////////////////////////////////
/// @class	CWHDiscoveryCfg
/// @ingroup 	Discovery
/// @brief	Discovery Process Configuration.
//////////////////////////////////////////////////////////////////////////////

// this structure is passed to sendTo() as a discovery response message.

struct discResponse 
{
	uint8_t		m_pGUID[16];			/* GUID sent in the request */
	uint8_t 	m_u8Version; 			/* protocol version number - static*/
	uint8_t 	m_u8Reserved;			/* reserved for future expansion*/
	uint16_t 	m_u16ByteCount;			/* byte count for the entire packet content */
	uint32_t 	m_u32IpAddress;			/* gateway IP address */
	uint16_t 	m_u16Port;			/* port number used by the gateway in communication with the client */

}__attribute__((packed));


// this structure is passed to sendTo() as the command 0 discovery response

struct disc0Response
{
	uint8_t 	m_u8FirstByte;			/* "254" */
	uint16_t	m_u16ExtendedDevType;		/* Expanded Device Type - default value */
	uint8_t		m_u8MinNoPreambles;		/* Minimum number of preambles from Master-Slave */
	uint8_t 	m_u8PreamblesNo;		/* number of preambles */
	uint8_t		m_u8WHRevisionNo;		/* HART Protocol Major Revision Number */
	uint8_t 	m_u8DevRevisionLevel; 		/* Device Revision Level */
	uint8_t 	m_u8SoftwRevisionLevel;		/* Software Revision Level */
	
	uint8_t		m_u8HWRevisionLevel_PhysicalSignCode;		/* Hardware Revision Level - most significant 5 bits and Physical Signaling Code - lease significant 3 bits */
	uint8_t		m_u8Flags;			/* Flags */
	uint8_t		m_DeviceID[3];			/* Device ID - different for every device */
	uint8_t 	m_u8MinPreamblesNo;		/* minimum no of preambles to be sent with the response */
	uint8_t 	m_u8MaxDeviceVarNo;		/* Maximum Number of Device Variables */
	uint16_t 	m_u16ConfigChangeCounter;	/* Configuration Change Counter */
	
	uint8_t 	m_u8FlagBits;			/* Extended Field Device Status */

	uint8_t 	m_u8ManufactCode[2];		/* Manufacturer Identification Code - Table 8  (range 1 - 24619) */
	uint8_t		m_u8ManufactLabel[2];		/* Private Label Distributor Code	*/
	uint8_t		m_u8DeviceProfile;		/* DeviceProfile */

}__attribute__((packed));


// this structure is passed to sendTo() as the command 0 discovery response

struct disc20Response
{
	char 		m_pLongTag[32];			/* Long Tag * - 32 bytes */	

}__attribute__((packed));



class CWHDiscoveryCfg : CConfig
{
public:
	CWHDiscoveryCfg();
	virtual ~CWHDiscoveryCfg();

public: 

	int Init (const char* p_szModuleName);
	bool ReadDiscoveryVars();
	int m_nLocalPort;
	char m_szType[4];

	discResponse 	response_msg;
	disc0Response 	response0_msg;
	disc20Response 	response20_msg;

	
};


#endif
