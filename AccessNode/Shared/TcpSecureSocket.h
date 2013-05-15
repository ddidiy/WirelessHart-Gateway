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
	created:	27:11:2009
	file:		TcpSecureSocket.h
	author:		Gabriel Ghervase, gabriel.ghervase@nivis.com
	
	purpose:	handles tcp operations over a socket, in a secure manner
*********************************************************************/

#ifndef _TCPSECURESOCKET_H__
#define _TCPSECURESOCKET_H__

#include "Socket.h"
#include "TcpSocket.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>

/// @addtogroup libshared
/// @{

struct _SSL_CTX;
typedef struct _SSL_CTX SSL_CTX;

struct _SSL;
typedef struct _SSL SSL;

class  CTcpSecureSocket : public CTcpSocket
{
public:
	enum ConnectionSide {CLIENT_SIDE, SERVER_SIDE};

public:
	using CTcpSocket::Connect;
	using CTcpSocket::Recv;

	CTcpSecureSocket();
	~CTcpSecureSocket();

	bool InitSSL(ConnectionSide p_enumConnectionSide, const char* p_strPrivateKeyFile, const char* p_strCertfile, const char* p_strCACertfile, const char* p_strKeyPasswd = 0);

	void Close();

	bool Connect( unsigned long p_nIP, unsigned short p_nPort, unsigned short p_nTimeout = 120 );

	void Attach(int p_nSock);

	bool Send( const void* msg, int len, int flags = 0  ) ; 
	bool Recv( void* msg, int& len, int flags = 0 )  ;
	bool CheckRecv( unsigned int  p_nTimeout) ;
	bool BlockUntilSSLServerHandshakeCompletes() ;

private:
	bool establishConnectionToSSLServer();
	bool establishConnectionToSSLClient();

private:
	ConnectionSide m_enumConnectionSide;	
	SSL_CTX* m_pSsl_ctx;
	SSL* m_pSsl;
	uint8_t* m_pInternalReadBuffer;
	int m_nRemainingBytesInBuffer;
};

/// @}
#endif // _TCPSECURESOCKET_H__
