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

// CVersaNode.cpp: implementation of the CVersaNode class.

#include <arpa/inet.h>
#include <string.h>
#include <cstdio>

#include "VersaNode.h"
#include "../Shared/Common.h"

#define ERR_MSG_BUFF_LEN	128

#define SEND_RETRIES		10
#define	FILE_HDR_LEN		12

const unsigned  char c_PROMPT[]		= {'>'};							// prompt char
const unsigned  char c_VALIDATION[]	= {'o', 'k', 'o', 'k'};				// validation string
const unsigned  char c_START[]		= {0, 0, '>'};						// start string
      unsigned	char g_CONFIRM[]	= {0, 0, '>'};						// confirm string - modified at exec time
const unsigned  char c_DELETE[]		= {(char)0xFF, (char)0xFF, '>'};	// confirm flash erase string


CVersaNode::CVersaNode()
{

}

CVersaNode::~CVersaNode()
{

}


void CVersaNode::CRC(unsigned long * p_pulData, size_t & p_runLen)
{
	if (p_pulData && p_runLen)
	{
		unsigned long * pCrc = p_pulData+p_runLen;
		*pCrc = 0;		
		for (size_t i = 0; i < p_runLen; i++)
			*pCrc ^= p_pulData[i];
//		*pCrc = htonl(*pCrc);
		p_runLen++;
	}
}

void CVersaNode::Log(const unsigned char * p_szInfo)
{
	if ( p_szInfo )
	{
		LOG((char*)p_szInfo);
	}
}

bool CVersaNode::Write2RS232(const unsigned char * p_puchData, size_t p_unLen)
{
	return bool(m_coLink.writeBlock(p_puchData, p_unLen));
}

bool CVersaNode::init(unsigned char p_uchDestination)
{
	// build del pk
	m_stPk.m_ushPkNo = htons(0xFFFF);
	m_stPk.m_ulAddr = (p_uchDestination | 0x80) << 24;
	m_stPk.m_ulAddr = htonl(m_stPk.m_ulAddr);
	size_t nLen = 8;
	m_stPk.m_ushLen = htons(nLen);
	nLen >>= 2;
	CRC((unsigned long *)&m_stPk, nLen);
	
	// clear device's flash with a command
	if (!Write2RS232((const unsigned char *)&m_stPk, ntohs(m_stPk.m_ushLen) + CRC_LEN))
		return false;

	// set first addr
	m_stPk.m_ulAddr = htonl(p_uchDestination << 24);

	// wait confirmation
	if (!ExpectFromRS232(c_DELETE, sizeof(c_DELETE), START_TIMEOUT<<1))
		return false;

	return true;
}

bool CVersaNode::MainTask( const unsigned char * p_szSerialPort, unsigned unSerPortSpeed, unsigned char p_uchDestination, 
						   const char * p_szFile, unsigned long p_ulTimeout, bool p_bLogData/*=false*/, int p_nTransNo, int p_nTimeOff)
{
	
	if (p_nTransNo>0)
	{
		systemf_to(60, "devmem 0xF00008%01d0 32 0x00000000", p_nTransNo);

		sleep(p_nTimeOff);
	}

	if (!m_coLink.openSerialLink((char *) p_szSerialPort, unSerPortSpeed ))
	{
		LOG_ERR("ERROR: Cannot open the specified RS232");
		return false;
	}
	m_coLink.SetLogRaw(p_bLogData);

	if (p_nTransNo)
	{
		systemf_to(60, "devmem 0xF00008%01d0 32 0x00000024", p_nTransNo);
	}

	// open file
	LOG("p_szFile=%s", p_szFile);
	FILE * pFile = fopen( p_szFile, "r" );
	if ( !pFile )
	{
		perror("ERROR: Cannot open the specified file\n");
		return false;
	}

	// exclude file header
	char achHdr[FILE_HDR_LEN+1];
	if ( FILE_HDR_LEN >= fread(achHdr, sizeof(char), FILE_HDR_LEN+1, pFile) )
	{
		LOG_ERR("ERROR: Invalid file format");
		fclose(pFile);
		return false;	
	}
	fseek(pFile, FILE_HDR_LEN, SEEK_SET); 

#ifndef _DEBUG
	// wait prompt
	int nPTimeout = time(NULL)+ PROMPT_TIMEOUT;
	while ( !ExpectFromRS232(c_PROMPT, sizeof(c_PROMPT), PROMPT_TIMEOUT) && nPTimeout > time(NULL))
	;
	
	if(nPTimeout < time(NULL) )
	{
	    	LOG("WARNING: Prompt not received");
		
		fclose(pFile);
		return false;
	}

	// send validation string
	if ( !Write2RS232(c_VALIDATION, sizeof(c_VALIDATION)) )
	{
		fclose(pFile);
		return false;
	}

	// wait start
	if ( !ExpectFromRS232(c_START, sizeof(c_START), START_TIMEOUT) )
	{
		LOG_ERR("ERROR: Start not received");
		fclose(pFile);
		return false;
	}
#endif // not _DEBUG

	// init
	if ( !init(p_uchDestination) )
	{
		LOG_ERR("ERROR: Flash erase not performed");
#ifndef _DEBUG
		fclose(pFile);
		return false;
#endif // not _DEBUG
	}
	
	// log header
	LOG_DTIME();
	LOG_IN("INFORMATION: File header -");
	for ( int i = 0; i < FILE_HDR_LEN; i++ )
		LOG_IN( " %02X", achHdr[i] );
	LOG_IN("\n");
	
	// upload chunks
	do {
		char szLog[64];
		// build crt pk
		if ( !getNextPkFromFile(pFile) )
		{
			// calc total len
			long nSize = ftell(pFile)-FILE_HDR_LEN;
			unsigned short nPad = nSize % sizeof(unsigned long);
			if (nPad) 
				nSize += sizeof(unsigned long)-nPad;			
			// log upload results at the end
			sprintf(szLog, "Uploaded %lu bytes into %u packet", nSize, ntohs(m_stPk.m_ushPkNo));	// min 0 bytes uploaded - condition already verified
			if (nSize > DATA_MAX_LEN)	// add a 's' - we have packets
				strcat(szLog, "s");
			Log((unsigned char *)szLog);
			break;
		}

		// prepare wait pk
		memcpy(g_CONFIRM, &m_stPk.m_ushPkNo, sizeof(m_stPk.m_ushPkNo));
		// prepare context logs
		sprintf(szLog, "Packet %u retry", ntohs(m_stPk.m_ushPkNo));
		sprintf(szLog+24, "Packet %u sent with %u bytes", ntohs(m_stPk.m_ushPkNo), ntohs(m_stPk.m_ushLen)-8);
		unsigned int nRetries = SEND_RETRIES;
		do
		{
			// send pk or retry
			if ( !Write2RS232((const unsigned char *)&m_stPk, ntohs(m_stPk.m_ushLen) + CRC_LEN) )
			{
				fclose(pFile);
				return false;
			}
			// log operation
			if ( nRetries < SEND_RETRIES )	Log((unsigned char *)szLog);
			else							Log((unsigned char *)szLog+24);
		
			// wait				
			if ( !ExpectFromRS232(g_CONFIRM, sizeof(g_CONFIRM), p_ulTimeout) )
			{
				if ( !nRetries )
				{
					LOG_ERR("ERROR: Device seems to be disconnected from RS232");
#ifdef _DEBUG
					break;
#else
					fclose(pFile);
					return false;
#endif // _DEBUG
				}
			}
			else	// next pk
				break;
		}
		while ( nRetries-- );
	} 
	while (true);
	
	fclose(pFile);

	if (p_nTransNo>0)
	{
		systemf_to(60, "devmem 0xF00008%01d0 32 0x00000000", p_nTransNo);

		sleep(p_nTimeOff);
	}

	if (!m_coLink.openSerialLink((char *) p_szSerialPort, unSerPortSpeed ))
	{
		LOG_ERR("ERROR: Cannot open the specified RS232");
		return false;
	}
	m_coLink.SetLogRaw(p_bLogData);

	if (p_nTransNo)
	{
		systemf_to(60, "devmem 0xF00008%01d0 32 0x00000024", p_nTransNo);
	}
	
	return true;
}

bool CVersaNode::ExpectFromRS232(const unsigned char * p_puchWait, unsigned char p_uchWaitLen, unsigned long p_ulTimeout) 
{
	if (p_puchWait && p_uchWaitLen )
	{
		unsigned int nRead, nLastRead = 0;
		do 
		{
		
			nRead = m_coLink.readBlock(m_achBuffer+nLastRead, p_uchWaitLen-nLastRead, 1);
			//if (nRead)
			//	LOG_ERR("nRead=%u m_achBuffer+nLastRead=%c", nRead, m_achBuffer+nLastRead );
			nRead += nLastRead;		// total read
			if (nRead >= p_uchWaitLen)
				return !memcmp(m_achBuffer, p_puchWait, p_uchWaitLen);
		}
		while ( p_ulTimeout-- );
	}
	return false;
}

bool CVersaNode::getNextPkFromFile(FILE * p_pFile)
{
	if (p_pFile)
	{
		// clear data
		memset(m_stPk.m_uchData, 0xFF, DATA_MAX_LEN);

		// read from file
		if ( feof(p_pFile) )
			return false;
		size_t nLen = fread(m_stPk.m_uchData, sizeof(unsigned char), DATA_MAX_LEN, p_pFile);
		
		// check for data
		if (!nLen)
			return false;
		
		// calc offset
		unsigned long ulCrtDst = htonl(m_stPk.m_ulAddr);
		m_stPk.m_ulAddr = htonl( ((ulCrtDst&0xFF000000) | (0x00FFFFFF&ulCrtDst)) + (ntohs(m_stPk.m_ushLen)-8) );
		
		// pk no
		m_stPk.m_ushPkNo = ntohs(m_stPk.m_ushPkNo)+1;
 		if ( !m_stPk.m_ushPkNo )
			m_stPk.m_ushPkNo++;
		m_stPk.m_ushPkNo = htons(m_stPk.m_ushPkNo);

		// pad data
		unsigned short nPad = nLen%sizeof(sizeof(unsigned long));
		if (nPad)
			nLen += sizeof(unsigned long)-nPad;
		nLen += 8;	// add hdr len
		m_stPk.m_ushLen = htons(nLen);

		// crc
		nLen >>= 2;
		CRC((unsigned long *)&m_stPk, nLen);

		return true;
	}
	return false;
}
