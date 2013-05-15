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
	file:		TcpSocket.cpp
	author:		Claudiu Hobeanu, claudiu.hobeanu@nivis.com

	purpose:	 handles tcp operations over a socket
*********************************************************************/

#include <string.h>

#include "TcpSocket.h"
#include "Common.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTcpSocket::CTcpSocket()
: m_nTimeout(120)
{
}

CTcpSocket::~CTcpSocket()
{
}


bool CTcpSocket::Send(const void *msg, int len, int flags)
{
	int pos = 0;
	for( pos = 0; pos < len;)
	{
		if( !CanSend() ) /// check if send() would block
			return false;

		int nRet = send( m_socket, (char*)msg + pos, len - pos, flags);
		if( nRet == SOCKET_ERROR )
		{	LOG_ERR("CTcpSocket::Send(fd %d)", m_socket);
			Close();
			return false;
		}
		pos += nRet;
	}
	return true;
}

bool CTcpSocket::Recv(void *msg, int &len, int flags)
{
	if (len == 0)
	{	LOG( "CTcpSocket::Recv(fd %d) -- WARNING input len == 0", m_socket );
		return false;
	}

	int nRet = recv( m_socket,	msg, len, flags );

	if( nRet == SOCKET_ERROR )
	{	LOG_ERR("CTcpSocket::Recv: recv(fd %d)", m_socket);
		Close();
		return false;
	}

	if( nRet == 0 )
	{	LOG( "CTcpSocket::Recv(fd %d): Session ended", m_socket);
		Close();
		return false;
	}

	len = nRet;

	return true;
}



bool CTcpSocket::Recv(int p_nTimeout, void *msg, int &p_nLen, int flags)
{
	if( !CheckRecv(p_nTimeout) )
	{	return false;
	}

	return Recv( msg, p_nLen, flags );
}

bool CTcpSocket::RecvBlock( int p_nTimeout, void* msg, int p_nLen, int flags )
{
	int nCrtLen = 0;
	for( ;nCrtLen < p_nLen; )
	{
		if ( !CheckRecv( p_nTimeout ))
		{
			if ( 0 < nCrtLen )
			{	LOG( "ERROR RecvBlock::CheckRecv FAILED ( %d / %d )", nCrtLen, p_nLen );
			}

			return false;
		}

		int nLeft = p_nLen - nCrtLen;
		if ( !Recv( (char*)msg + nCrtLen, nLeft, flags ) )
		{
			if ( 0 < nCrtLen )
			{	LOG( "ERROR RecvBlock::Recv FAILED ( %d / %d )", nCrtLen, p_nLen );
			}

			return false;
		}

		nCrtLen += nLeft;
	}

	return true;
}



bool CTcpSocket::Connect(const char *p_szHost, unsigned short p_nPort, unsigned short p_nTimeout)
{
	struct hostent *server;

	server = gethostbyname( p_szHost );
	if( server == NULL ) {
		LOG( "ERROR No such host : [%s]",  p_szHost);
		return false;
	}

	return Connect( (*(struct in_addr *)server->h_addr_list[0]).s_addr, htons(p_nPort), p_nTimeout );
}

bool CTcpSocket::Connect(unsigned long p_nIP, unsigned short p_nPort, unsigned short p_nTimeout)
{
	m_nPort = p_nPort;
	m_nIP =  p_nIP ;
	m_nTimeout = p_nTimeout;

	sockaddr_in	saRemote;
	memset( &saRemote, 0, sizeof( saRemote ) );

	saRemote.sin_family			= AF_INET;
	saRemote.sin_port			= m_nPort;
	saRemote.sin_addr.s_addr	= p_nIP;

	if(connect_to(m_socket, (struct sockaddr *)&saRemote, sizeof(saRemote), p_nTimeout ) == SOCKET_ERROR)
	{	in_addr sIP;
		sIP.s_addr = p_nIP;
		LOG_ERR("CTcpSocket::Connect: (%s:%d)", inet_ntoa(sIP), ntohs(m_nPort));
		Close();
		return false;
	}
	return true;
}

bool CTcpSocket::Reconnect()
{
	Close();
	if( !Create() )
		return false;

	return Connect(m_nIP, m_nPort, m_nTimeout );
}

