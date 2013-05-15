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

#include "../../Shared/StreamLink.h"
#include "../../Shared/IniParser.h"
#include "../../Shared/Common.h"
#include "../../Shared/h.h"
#include "../../Shared/Config.h"
#include "../../Shared/app.h"



CDiscoveryApp g_stApp;


CDiscoveryApp::CDiscoveryApp()
: CApp( "discovery" )
{
}

CDiscoveryApp::~CDiscoveryApp()
{
}

void CDiscoveryApp::Close()
{
	CApp::Close();
}

int CDiscoveryApp::Init( const char * p_szInterface )
{
	char szBuffer[ 64 ];
	
	sprintf( szBuffer, NIVIS_TMP"%s.log", p_szInterface);
	
	//g_stLog.OpenStdout();

	if(!CApp::Init(szBuffer))
	{ 
		return 0;
	}

	if(!m_WHDisc.Init(p_szInterface))
	{	
		return 0;
	}

 	if(!m_WHDisc.ReadDiscoveryVars())
 	{
 		return 0;
 	}

	srand(time(NULL));
	
	LOG( "Init DONE");
	
	return 1;    //all went OK

}


int CDiscoveryApp::Run( void )
{
	//g_stLog.SetMaxSize( 200 * 1024 );	// it is too late here...
	return udpServer( );
}

unsigned int  CDiscoveryApp::getANId()
{
	return  m_stCfg.m_nAnId;
}

unsigned char*& CDiscoveryApp::incrBuffer(unsigned char *& p, unsigned offset)
{
	p = p + offset;
	return  p;
}


// returns -1,0 -- error
//		length of data in output buffer 
int CDiscoveryApp::processDataBIN(unsigned char *p_pBuffIn, int p_nLenIn,unsigned char *p_pBuffOut, int p_nLenOutMax, const struct sockaddr_in& p_stClientAddr)
{
	// match of the GUID in the discovery request vs. GUID of the device
	int m_nLenOut;
	LOG("DELETEME -- process data BIN");
	if(sizeof(m_WHDisc.response_msg.m_pGUID)==p_nLenIn)
		{
			LOG("Sizeof input GUID = Sizeof gateway GUID");
			
		}
	else	
		{
			LOG("Sizeof input GUID: %d, expected: %d",p_nLenIn, sizeof(m_WHDisc.response_msg.m_pGUID));
			return -1;
		}

	
	if(memcmp((unsigned char *) m_WHDisc.response_msg.m_pGUID, p_pBuffIn, sizeof(m_WHDisc.response_msg.m_pGUID) ) != 0)
		{		
			LOG("WRONG GUID");	
			return -1;	
			
		}

	LOG("CORRECT GUID");

	// send a discovery response message as the GUID matches

	memcpy(p_pBuffOut, &m_WHDisc.response_msg, sizeof(m_WHDisc.response_msg));
	
	memcpy(p_pBuffOut + sizeof(m_WHDisc.response_msg), &m_WHDisc.response0_msg, sizeof(m_WHDisc.response0_msg));
	memcpy (p_pBuffOut + sizeof(m_WHDisc.response_msg) + sizeof(m_WHDisc.response0_msg), &m_WHDisc.response20_msg, sizeof(m_WHDisc.response20_msg));

	m_nLenOut = sizeof(m_WHDisc.response_msg)+ sizeof(m_WHDisc.response0_msg)+ sizeof(m_WHDisc.response20_msg);
	return m_nLenOut;//size data in out buffer
	
	

}

// returns -1,0 -- error
//		length of data in output buffer 
int CDiscoveryApp::processDataXML(unsigned char *p_pBuffIn, int p_nLenIn,unsigned char *p_pBuffOut, int p_nLenOutMax, const struct sockaddr_in& p_stClientAddr)
{
	// match of the GUID in the discovery request vs. GUID of the device
	int m_nLenOut;
	LOG("DELETEME -- process data XML");
	if(sizeof(m_WHDisc.response_msg.m_pGUID)==p_nLenIn)
		{
			LOG("Sizeof input GUID = Sizeof gateway GUID");
			
		}
	else	
		{
			LOG("Sizeof input GUID: %d, expected: %d",p_nLenIn, sizeof(m_WHDisc.response_msg.m_pGUID));
			return -1;
		}

	
	if(memcmp((unsigned char *) m_WHDisc.response_msg.m_pGUID, p_pBuffIn, sizeof(m_WHDisc.response_msg.m_pGUID) ) != 0)
		{		
			LOG("WRONG GUID");	
			return -1;	
			
		}

	LOG("CORRECT GUID");

	// send a discovery response message as the GUID matches
	
	memcpy(p_pBuffOut,  m_WHDisc.response_msg.m_pGUID, sizeof( m_WHDisc.response_msg.m_pGUID));
	memcpy(p_pBuffOut + sizeof(m_WHDisc.response_msg.m_pGUID), &m_WHDisc.response_msg.m_u32IpAddress , sizeof(m_WHDisc.response_msg.m_u32IpAddress));
	memcpy(p_pBuffOut + sizeof(m_WHDisc.response_msg.m_pGUID)+ sizeof(m_WHDisc.response_msg.m_u32IpAddress), &m_WHDisc.response_msg.m_u16Port, sizeof(m_WHDisc.response_msg.m_u16Port));
	
	m_nLenOut = sizeof(m_WHDisc.response_msg.m_pGUID)+ sizeof(m_WHDisc.response_msg.m_u32IpAddress) + sizeof(m_WHDisc.response_msg.m_u16Port);

	return m_nLenOut; 
}


int CDiscoveryApp::udpServer( void )
{
	/* Variable and structure definitions. */
	
	int sd, rc;
	struct sockaddr_in serveraddr, clientaddr;
	int clientaddrlen = sizeof(clientaddr);
	int serveraddrlen = sizeof(serveraddr);    
	

	unsigned char buffer[1000]={0};
	unsigned char *bufptr = buffer;
	
	int buflen = sizeof(buffer);

	if((sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		LOG ("UDP server - socket() error %s", strerror(errno));
		return -1;
	}

	memset(&serveraddr, 0x00, serveraddrlen);
	serveraddr.sin_family = AF_INET;

	serveraddr.sin_port = htons(m_WHDisc.m_nLocalPort);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if((rc = bind(sd, (struct sockaddr *)&serveraddr, serveraddrlen)) < 0)
	{
		LOG ("UDP server - bind() error %s", strerror(errno));
		close(sd);
		return -1;
	}
	
	LOG("Using IP %s and port %d, bind to device [%s]", inet_ntoa(serveraddr.sin_addr), m_WHDisc.m_nLocalPort, ETH0);
	
	int optval=1;
	int optlen=sizeof(optval);
	
	if ( setsockopt(sd , SOL_SOCKET, SO_BROADCAST, (char *)&optval,optlen) == SOCKET_ERROR )
	{
		LOG( "AllowBroadcast() %s", strerror(errno));
		close(sd);
		return -1;
	}
	   
	if ( setsockopt(sd , SOL_SOCKET, SO_BINDTODEVICE, ETH0, sizeof(ETH0)) == SOCKET_ERROR )
	{
		LOG( "setsockopt(SO_BINDTODEVICE)  %s", strerror(errno));
		close(sd);
		return -1;
	}
	
	LOG("UDP server - Listening...");
	while( !IsStop() )	
	{
		memset(bufptr, 0, buflen);
		
		while( !checkRecv( sd, 1000000 ) ) // wait 1 second - long enough to keep the processor loat at a low level
		{	//stay in loop waiting a message. This is necessary in order to check for IsStop
			if( IsStop() )
			{
				LOG("udpServer: STOP requested, terminate");
				close(sd);
				return 1;
			}
		}
		
		if((rc = recvfrom(sd, bufptr, buflen, 0, (struct sockaddr *)&clientaddr, (socklen_t *) &clientaddrlen))<0)
		{
			LOG("UDP Server - recvfrom() error %s", strerror(errno));
			continue;
		}
		
		LOG("Incomming message from %s:%d", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port) );
		

		unsigned char pBuffOut[1024];
		int nLenMax = sizeof(pBuffOut);
		int nLenOut = 0;

		if (strcmp(m_WHDisc.m_szType, "bin") == 0)
			nLenOut = processDataBIN(bufptr, rc, pBuffOut, nLenMax, clientaddr);

		else if (strcmp(m_WHDisc.m_szType, "xml") == 0)
			nLenOut = processDataXML(bufptr, rc, pBuffOut, nLenMax, clientaddr);

		else 
		{	
			LOG ("Unknown Processing TYPE for Discovery Procedure!");
			return 0;
		}
		if( nLenOut <= 0 )
		{
			continue;
		}
		
		clientaddr.sin_family = AF_INET;
		clientaddr.sin_port = htons(30000);
		clientaddr.sin_addr.s_addr=INADDR_BROADCAST;

		usleep((rand()%10)*1000);
		LOG("Sendback DISCOVERY_REPLY nLen=%d",nLenOut);

		if((rc = sendto(sd, (const void*)pBuffOut, nLenOut, 0, (struct sockaddr *)&clientaddr, clientaddrlen))<0)
		{
			LOG("ERROR sendto() %s", strerror(errno));
			continue;
		}
	} 
	close(sd);
	return 1;
}


//p_nTimeout is in uSec (10^-6 sec)
bool CDiscoveryApp::checkRecv(int p_nSocket, int p_nTimeout)
{
	fd_set		readfds;
	struct timeval tmval;

	tmval.tv_usec = p_nTimeout%1000000;
	tmval.tv_sec = p_nTimeout/1000000;

	FD_ZERO(&readfds);
	FD_SET(p_nSocket, &readfds);

	int nSelected = ::select( p_nSocket +1, &readfds, NULL, NULL, &tmval);

	if ( SOCKET_ERROR == nSelected )
	{
		LOG_ERR("CDiscoveryApp::checkRecv");
		return false;
	}

	if( (nSelected == 0) || (!FD_ISSET( p_nSocket, &readfds ) ) )
	{	return false;
	}

	return true;
}



