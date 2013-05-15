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

// VersaNode.h: interface for the CVersaNode class.
#ifndef _VERSA_NODE_H_
#define _VERSA_NODE_H_


#include "../Shared/link.h"
#include <cstdio>


#define BAUDRATE			B115200 /* CBR_115200 */
#define ONE_SEC_TIMEOUT		1 						
#define START_TIMEOUT		(10*ONE_SEC_TIMEOUT)
#define PROMPT_TIMEOUT      (60*ONE_SEC_TIMEOUT)  

#define DATA_MAX_LEN		256
#define READ_BUFFER_LEN		128
#define CRC_LEN				sizeof(unsigned long)



#pragma pack(1)
typedef struct {
	unsigned short m_ushLen;
	unsigned short m_ushPkNo;
	unsigned long  m_ulAddr;
	unsigned char  m_uchData[DATA_MAX_LEN+CRC_LEN];
} GENERAL_PK;
#pragma pack()

class CVersaNode
{
public:
	CVersaNode();
	virtual ~CVersaNode();
	
	bool Write2RS232(const unsigned  char * p_puchData, size_t p_unLen);
	bool ExpectFromRS232(const unsigned char * p_puchWait, unsigned char p_uchWaitLen, unsigned long p_ulTimeout);	
	bool MainTask( const unsigned char * p_szSerialPort, unsigned unSerPortSpeed, unsigned char p_uchDestination,
								  const char * p_szFile, unsigned long p_ulTimeout, bool p_bLogData=false, int p_nTransNo = -1, int p_nTimeOff = 10);

	static void CRC(unsigned long * p_pulData, size_t & p_runLen);
	static void Log(const unsigned char * p_szInfo);


protected:
	unsigned char m_achBuffer[READ_BUFFER_LEN];	// RS232 buffer
	GENERAL_PK  m_stPk;							// packet to send through serial
	CLink		m_coLink;						// serial object
	bool init(unsigned char p_uchDestination);
	bool getNextPkFromFile(FILE * p_pFile);

};

#endif
