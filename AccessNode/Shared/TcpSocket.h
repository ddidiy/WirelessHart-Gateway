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
	file:		TcpSocket.h
	author:		Claudiu Hobeanu, claudiu.hobeanu@nivis.com
	
	purpose:	 handles tcp operations over a socket
*********************************************************************/



#if !defined(AFX_TCPSOCKET_H__53338826_F81C_4BAF_B9E3_732EAC7A1C12__INCLUDED_)
#define AFX_TCPSOCKET_H__53338826_F81C_4BAF_B9E3_732EAC7A1C12__INCLUDED_

#include "Socket.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/// @addtogroup libshared
/// @{

class  CTcpSocket : public CWrapSocket  
{
public:
	CTcpSocket();
	virtual ~CTcpSocket();
	virtual bool Create(int p_nType = SOCK_STREAM, int p_nFamily = AF_INET) {	return CWrapSocket::Create( p_nType, p_nFamily ); }

	virtual bool Connect( unsigned long p_nIP, unsigned short p_nPort, unsigned short p_nTimeout = 120 );
	virtual bool Connect( const char* p_szHost, unsigned short p_nPort, unsigned short p_nTimeout = 120 );

	bool Reconnect();

	virtual bool Send( const void* msg, int len, int flags = 0  ) ; 
	virtual bool Recv( void* msg, int& len, int flags = 0 )  ;
	virtual bool Recv( int p_nTimeout, void* msg, int& p_nLen, int flags = 0) ;

	bool RecvBlock( int p_nTimeout, void* msg, int p_nLen, int flags = 0) ;

	void SetIp( unsigned long p_nIP ) { m_nIP = p_nIP; }
	unsigned long GetIp() { return m_nIP;}

	const char * GetIPString( void ) { in_addr sIP; sIP.s_addr = m_nIP; return inet_ntoa( sIP ); }


protected:
	unsigned long	m_nIP;	//net order
	unsigned short	m_nTimeout;	//connect timeout, for use at reconnect time
};

/// @}
#endif // !defined(AFX_TCPSOCKET_H__53338826_F81C_4BAF_B9E3_732EAC7A1C12__INCLUDED_)
