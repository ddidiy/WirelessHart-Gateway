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

/********************************************************************
	created:	10:10:2003
	file:		Socket.cpp
	author:		Claudiu Hobeanu, claudiu.hobeanu@nivis.com

	purpose:	 a wrapper class for bsd sockets
*********************************************************************/

#include <sys/time.h>
#include <arpa/inet.h>

#include "Socket.h"
#include "Common.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWrapSocket::CWrapSocket()
{
	m_socket = INVALID_SOCKET;
	m_nPort = 0;
}

CWrapSocket::~CWrapSocket()
{
	Close();
}

bool CWrapSocket::Create(int p_nType, int p_nFamily)
{
	Close();
	m_nSockType = p_nType;
	m_nSockFamily = p_nFamily;

	m_socket = socket(m_nSockFamily, m_nSockType, 0);

	if( m_socket == INVALID_SOCKET)
	{	LOG_ERR("CWrapSocket::Create()");
		return false;
	}
	LOG("Socket (fd %d) fam %d type %d open", m_socket, p_nFamily, p_nType);
	return true;
}

bool CWrapSocket::Bind(unsigned short p_nPort, const char* p_szLocalAddress, const char* p_szIfName )
{
	SetPort(p_nPort);
	int nBindErr = SOCKET_ERROR;

	char pBinAddress[16];//sizeof(sockaddr_in6.sin6_addr)];

	if (p_szLocalAddress && !inet_pton(m_nSockFamily,p_szLocalAddress,pBinAddress))
	{	
		LOG_ERR("CWrapSocket::Bind. inet_pton");
		return false;
	}

	if (p_szIfName)
	{
		
#ifndef HW_CYG
		//bind to if
		if (setsockopt (m_socket, SOL_SOCKET, SO_BINDTODEVICE, p_szIfName, strlen (p_szIfName)))
		{
			LOG_ERR("setsockopt");
			return false;
		}
#endif
	}
	if (m_nSockFamily == AF_INET)
	{	
		sockaddr_in	sAddr;	
		if (p_szLocalAddress)
		{	memcpy(&sAddr.sin_addr,pBinAddress,4);
		}
		else
		{	sAddr.sin_addr.s_addr	= htonl(INADDR_ANY);
		}	
		
		sAddr.sin_family		= m_nSockFamily;
		sAddr.sin_port			= m_nPort;

		nBindErr = bind(m_socket, (sockaddr *)&sAddr, sizeof(sAddr));
	}
	else if (m_nSockFamily == PF_INET6)
	{
		sockaddr_in6	sAddr;		 
		if (p_szLocalAddress)
		{	memcpy(&sAddr.sin6_addr,pBinAddress,16);
		}
		else
		{
			struct in6_addr foo=IN6ADDR_ANY_INIT;

			sAddr.sin6_addr = foo;
		}
		sAddr.sin6_family		= m_nSockFamily;
		sAddr.sin6_port			= m_nPort;

		nBindErr = bind(m_socket, (sockaddr *)&sAddr, sizeof(sAddr));
	}

	if (SOCKET_ERROR == nBindErr)
	{	LOG_ERR("CWrapSocket::Bind: port=%d", p_nPort);
		return false;
	}
	LOG("Socket (fd %d) bind(port %u)", m_socket, p_nPort);
	return true;
}

bool CWrapSocket::Bind (const CNetAddressExt& p_rNetAddr)
{
	if (m_nSockFamily != p_rNetAddr.GetFamily())
	{
		LOG("");
		return false;
	}
	SetPort(p_rNetAddr.GetPort());

	int nBindErr = SOCKET_ERROR;

	if (p_rNetAddr.GetIfName())
	{

#ifndef HW_CYG
		//bind to if
		 if (setsockopt (m_socket, SOL_SOCKET, SO_BINDTODEVICE, p_rNetAddr.GetIfName(), strlen (p_rNetAddr.GetIfName())))
		 {
		     LOG_ERR("setsockopt");
		     return false;
		 }
#endif

	}

	if (m_nSockFamily == AF_INET)
	{	
		sockaddr_in	sAddr;	

		sAddr.sin_addr.s_addr	= p_rNetAddr.GetIPv4();
		sAddr.sin_family		= m_nSockFamily;
		sAddr.sin_port			= m_nPort;

		nBindErr = bind(m_socket, (sockaddr *)&sAddr, sizeof(sAddr));
	}
	else if (m_nSockFamily == PF_INET6)
	{
		sockaddr_in6	sAddr;		 
		
		memcpy(&sAddr.sin6_addr,p_rNetAddr.GetIPv6(),16);

		sAddr.sin6_family		= m_nSockFamily;
		sAddr.sin6_port			= m_nPort;

		nBindErr = bind(m_socket, (sockaddr *)&sAddr, sizeof(sAddr));
	}

	if (SOCKET_ERROR == nBindErr)
	{	LOG_ERR("CWrapSocket::Bind: port=%d", p_rNetAddr.GetPort());
		return false;
	}
	LOG("Socket (fd %d) bind(port %u)", m_socket, p_rNetAddr.GetPort());
	return true;
}


void CWrapSocket::Close()
{
	if( m_socket != INVALID_SOCKET )
	{	
		shutdown(m_socket, 2 );
		close(m_socket);
		LOG("Socket (fd %d) closed", m_socket);
		m_socket = INVALID_SOCKET;
	}
}


//extern int errno;
#include <errno.h>
bool CWrapSocket::CheckRecv(unsigned int p_nTimeout)
{
    if( !IsValid() )
    {   return false;
    }

	fd_set		readfds;
	struct timeval tmval;

	tmval.tv_usec = p_nTimeout%1000000;
	tmval.tv_sec = p_nTimeout/1000000;

	FD_ZERO(&readfds);
	FD_SET(m_socket, &readfds);

	int nSelected = ::select( m_socket +1, &readfds, NULL, NULL, &tmval);

	if ( SOCKET_ERROR == nSelected )
	{
		if (errno == EINTR )
		{	return false;
		}

		LOG_ERR("CWrapSocket::CheckRecv(fd %u)", m_socket);

		Close();
		return false;
	}

	if(nSelected == 0)
		return false;

	if( !FD_ISSET( m_socket, &readfds ) )
	{	return false;
	}

	return true;
}

/// Check if can send to socket. Time is in usec
/// @note usefull to detect in advance if a send operation would block
bool CWrapSocket::CanSend( void )
{	fd_set		writefds;
	struct timeval tmval;
	if( !IsValid() ) return false;
	tmval.tv_usec = 0;
	tmval.tv_sec = 0;
	FD_ZERO(&writefds);
	FD_SET(m_socket, &writefds);

	int nSelected = ::select( m_socket +1, NULL, &writefds , NULL, &tmval);
	if(	( SOCKET_ERROR == nSelected )			// socket error
	||	( 0 == nSelected)						// timeout
	||	( !FD_ISSET( m_socket, &writefds ) ) )	// cannot send: would block
	{
		LOG_ERR("CWrapSocket::CanSend(fd %u) select() returned %d", m_socket, nSelected);
		if (errno == EINTR )
		{	return false;
		}
		Close();
		return false;
	}
	
	return true;
}


unsigned long CWrapSocket::GetLocalIp()
{
	char szName[256];
	if( ::gethostname(szName, 256) )
	{	LOG_ERR( "CWrapSocket::GetLocalIp()");
		return INADDR_NONE;
	}

	LOG("GetLocalIp: hostname=%s", szName);
	struct hostent* pAddr = ::gethostbyname( szName );

	if (!pAddr)
	{	LOG_ERR( "CWrapSocket::GetLocalIp: gethostbyname ERROR");
		return INADDR_NONE;
	}

	return ((struct in_addr*)(pAddr->h_addr_list[0]))->s_addr ;
}
