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
	file:		ServerSocket.cpp
	author:		Claudiu Hobeanu, claudiu.hobeanu@nivis.com
	
	purpose:	 handles tcp server operations over a socket
*********************************************************************/
 
//////////////////////////////////////

//#include "stdafx.h"
#include "ServerSocket.h"
#include "Common.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CServerSocket::CServerSocket()
{

}

CServerSocket::~CServerSocket()
{

}

bool CServerSocket::Listen(int p_nNoConn)
{
	if( listen(m_socket, p_nNoConn) == SOCKET_ERROR )
	{	LOG_ERR("CServerSocket::Listen()");
		return false;
	}
	
	return true;
}

bool CServerSocket::Accept(CTcpSocket *p_sock)
{
	sockaddr_in sinFrom = {0,0,{0},{0}};
	//memset((void *) &sinFrom, 0, sizeof(sinFrom));
	socklen_t nAddrLen = sizeof(sinFrom);
	int keepalive = 1;

	int sAcc = accept( m_socket, (sockaddr *)&sinFrom, &nAddrLen );

	if( sAcc < 0 )
	{	LOG_ERR( "CServerSocket::Accept:" );
		return false;
	}

	p_sock->Attach(sAcc);
	p_sock->SetIp(sinFrom.sin_addr.s_addr);
	p_sock->SetPort(sinFrom.sin_port);
	setsockopt(sAcc, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive));

	return true;
}

bool CServerSocket::StartServer(unsigned short p_nPort, int nListenParam )
{
	if (!Create( SOCK_STREAM))
	{	return false;
	}
	int optval = 1;
	optval = setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
	if (!Bind(p_nPort)) 
	{	return false;
	}
	if( !Listen(nListenParam) )
	{	return false;
	}

	LOG("Server started on port %d", p_nPort );
	return true;
}
