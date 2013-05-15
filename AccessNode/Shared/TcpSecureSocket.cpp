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
	
	purpose:	 handles tcp operations over a socket, in a secure manner
*********************************************************************/

#include <string.h>
#include <iostream>
#include <unistd.h>

#include "axtls/ssl.h"

#include "TcpSecureSocket.h"
#include "Common.h"

using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTcpSecureSocket::CTcpSecureSocket()
	:m_pSsl_ctx(0),
	 m_pSsl(0),
	 m_pInternalReadBuffer(0),
	 m_nRemainingBytesInBuffer(0)
{}


CTcpSecureSocket::~CTcpSecureSocket()
{
	ssl_ctx_free(m_pSsl_ctx);
}


/**
 * Ssl initialization function ; it creates the ssl context and loads in it the specified keys/certifs.
 * @param p_enumConnectionSide  tells on which side of the connection this socket is to be used 
 * @param p_strPrivateKeyFile path to private key file;
 * @param p_strCertfile  path to certificate 
 * @param p_strCACertfile path to certificate that was used to sign p_strCertfile
 * @param p_strKeyPasswd password used to encrypt the private key file; only aes128, aes256 supported
 * @return false for init error; true if ok
 */

bool CTcpSecureSocket::InitSSL(ConnectionSide p_enumConnectionSide, const char* p_strPrivateKeyFile, const char* p_strCertfile, const char* p_strCACertfile, const char* p_strKeyPasswd /*= 0*/)
{
	if ( (p_strPrivateKeyFile == 0) || (p_strCertfile == 0) || (p_strCACertfile == 0) )
	{
		NLOG_ERR("CTcpSecureSocket::InitSSL() - the given combination of ssl objects is invalid");
		return false;
	}

	m_enumConnectionSide = p_enumConnectionSide;

	if ( m_pSsl_ctx != 0 )
	{
		ssl_ctx_free(m_pSsl_ctx);
		m_pSsl_ctx = 0;
	}

	//don't use default private key and cert. + request certificate from the client
	uint32_t options = (p_enumConnectionSide == CLIENT_SIDE) ? (SSL_NO_DEFAULT_KEY) : (SSL_NO_DEFAULT_KEY | SSL_CLIENT_AUTHENTICATION);

	//init context
	if ((m_pSsl_ctx = ssl_ctx_new(options, (p_enumConnectionSide == CLIENT_SIDE) ? SSL_DEFAULT_CLNT_SESS : SSL_DEFAULT_SVR_SESS )) == 0)
	{
		NLOG_ERR("CTcpSecureSocket::InitSSL - Error at creating new server context");
		return false;
	}

	// load private key in context; 
	// if key is held encrypted, a password is needed to decrypt it.
	if ( ssl_obj_load(m_pSsl_ctx, SSL_OBJ_RSA_KEY, p_strPrivateKeyFile, p_strKeyPasswd ) != SSL_OK )
	{
		NLOG_ERR("CTcpSecureSocket::InitSSL - Error loading private key from file %s " ,  p_strPrivateKeyFile);
		return false;
	}

	//load certificate in context
	if ( ssl_obj_load(m_pSsl_ctx, SSL_OBJ_X509_CERT, p_strCertfile,  0) != SSL_OK )
	{
		NLOG_ERR("CTcpSecureSocket::InitSSL - Error loading certificate from file %s " , p_strCertfile);
		return false;
	}

	//load CA certificate in context
	if ( ssl_obj_load(m_pSsl_ctx, SSL_OBJ_X509_CACERT, p_strCACertfile, 0) != SSL_OK )
	{
		NLOG_ERR("CTcpSecureSocket::InitSSL - Error loading CA certificate from file %s " , p_strCACertfile);
		return false;
	}

	return true;
}


/**
 * SSL Send function
 * @param msg send buffer
 * @param len length to send
 * @param flags
 * @return error or success
 */
bool CTcpSecureSocket::Send(const void *msg, int len, int flags)
{
	if (m_pSsl == 0)
	{
		NLOG_ERR("CTcpSecureSocket::Send() - no ssl connection");
		return false;
	}

	if (msg == 0)
	{
		NLOG_ERR("CTcpSecureSocket::Send() - invalid send buffer");
		return false;
	}

	if (len == 0)
	{
		NLOG_WARN("Sending 0 bytes");
		return true;
	}

	int pos = 0;
	for( pos = 0; pos < len;)
	{
		int nRet = ssl_write( m_pSsl, (uint8_t*)msg + pos, len - pos );
		if( nRet < 0 )
		{	
			NLOG_ERR("CTcpSecureSocket::Send() - error on ssl_write");
			Close();
			return false;
		}
		pos += nRet;
	}

	return true;
}


/**
 * SSL Receive function
 * @param receive buffer 
 * @param len input : how much to receive ; output : how much was actually received
 * @param flags 
 * @return false for error ; true for success
 * @remarks : axtls secures using a block cipher => encr and decr on fixed block sizes
 *			  if haven't received at least a full block, it cannot perform decryption and return our data
 *			  so, Recv returns false and outputs len = 0 in this case
 * 			  if behaviour not ok, uncomment the "do while" to wait until a full block has arrived
 */
bool CTcpSecureSocket::Recv(void *msg, int &len, int flags)
{
	if (m_pSsl == 0)
	{
		NLOG_ERR("CTcpSecureSocket::Recv() - no ssl connection");
		len = 0;
		return false;
	}
	
	if (msg == 0)
	{
		NLOG_ERR("CTcpSecureSocket::Recv() - recv buffer = 0");
		len = 0;
		return false;
	}

	if (len == 0)
	{
		NLOG_WARN("Receiving 0 bytes");
		return true;
	}

	//if no bytes in buffer, fill the buffer
	//ssl_read reads data into an internal read buffer whose address will be stored in the output parameter
	if (m_nRemainingBytesInBuffer == 0)
	{
		//do {		
			m_nRemainingBytesInBuffer = ssl_read(m_pSsl, (&m_pInternalReadBuffer));
		//} while (m_nRemainingBytesInBuffer == SSL_OK && m_pSsl->got_bytes < m_pSsl->need_bytes);

		//error
		if (m_nRemainingBytesInBuffer < 0)
		{
			NLOG_ERR("CTcpSecureSocket::Recv - error on ssl_read");
			Close();
			len = 0;
			return false;
		}
		//either handshk not finished or we don't have a full block for decryption yet
		else if (m_nRemainingBytesInBuffer == 0)
		{
			NLOG_DBG("CTcpSecureSocket::Recv - ssl_read returns 0");
			len = 0;
			return false;
		}
	}

	//process the bytes in the buffer
	if ( m_nRemainingBytesInBuffer <= len )
	{
		//copy the data from the internal buffer to our buffer
		memcpy(msg, m_pInternalReadBuffer, m_nRemainingBytesInBuffer);	
		len = m_nRemainingBytesInBuffer;
		m_nRemainingBytesInBuffer = 0; //all bytes were read
		m_pInternalReadBuffer = 0;
	}
	else
	{
		//copy the data from the internal buffer to our buffer
		memcpy(msg, m_pInternalReadBuffer, len);
		m_nRemainingBytesInBuffer -= len;
		m_pInternalReadBuffer += len;
	}

	return true;
}

/**
 * Checks if there's interesting data on socket
 * @param p_nTimeout 
 * @return true if data to recv; false if handshk not done yet or no data to recv
 */
bool CTcpSecureSocket::CheckRecv(unsigned int  p_nTimeout) 
{
	if (m_pSsl == 0)
	{
		NLOG_ERR("CTcpSecureSocket::CheckRecv() - no ssl connection");
		return false;
	}

	/* Cannot use select to check for data on socket until the handshake completes
	 * While handshake in progress we have uninteresting data on socket (ssl handshake packets)*/
	if ( ssl_handshake_status(m_pSsl) != SSL_OK )
	{
		int len;
		len = ssl_read(m_pSsl,0); //re-initialize handshake status flag
		return false;
	}

	/* Recv reads as much as possible and returns only the required len ; 
	 * The rest is kept internally and used on subsequent Recv calls
	 * Therefore, if data is available in buffer, CheckRecv should return true */
	if ( m_nRemainingBytesInBuffer > 0)
	{
		return true;
	}
	
	return CTcpSocket::CheckRecv(p_nTimeout);
}


/**
 * Establishes an ssl connection using the given id and port
 * @return error or success
 */
bool CTcpSecureSocket::Connect(unsigned long p_nIP, unsigned short p_nPort, unsigned short p_nTimeout)
{
	if (m_pSsl_ctx == 0)
	{
		NLOG_ERR("CTcpSecureSocket::Connect() - no ssl context");
		return false;
	}

	bool retvalue = CTcpSocket::Connect(p_nIP, p_nPort, p_nTimeout);

	if (!retvalue) return retvalue;

	if (m_enumConnectionSide == CLIENT_SIDE)
	{
		//establish a new ssl session with a server
		retvalue =  retvalue && establishConnectionToSSLServer();
	}
	//else: for server side, the ssl session is established after obtaining the communication socket from accept (which is set using Attach)

	return retvalue;
}

/**
 * Attaches a new socket connection; Establishes a new ssl connection on the given connection
 * @param p_nSock - socket to attach
 */
void CTcpSecureSocket::Attach(int p_nSock)
{
	if (m_pSsl_ctx == 0)
	{
		NLOG_ERR("CTcpSecureSocket::Attach() - no ssl context");
		return;
	}

	ssl_free(m_pSsl);
	m_pSsl = 0;

	CTcpSocket::Attach(p_nSock);	

	if (m_enumConnectionSide == CLIENT_SIDE)
	{
		establishConnectionToSSLServer();
	}	
	else
	{
		establishConnectionToSSLClient();
	}
}


void CTcpSecureSocket::Close()
{
	ssl_free(m_pSsl);
	m_pSsl = 0;
	m_nRemainingBytesInBuffer = 0;
	m_pInternalReadBuffer = 0;
	CTcpSocket::Close();
}


/**
 * Called by the client to setup ssl connection with server
 * @return error or succes
 * @remarks blocks until handshake is done
 */
bool CTcpSecureSocket::establishConnectionToSSLServer()
{
	if (m_enumConnectionSide != CLIENT_SIDE)
	{
		NLOG_ERR("establishConnectionToSSLServer - m_enumConnectionSide != CLIENT_SIDE ");
		Close();
		return false;
	}

	//blocking call
	m_pSsl =  ssl_client_new(m_pSsl_ctx, m_socket,NULL,0);	

	int retval = ssl_handshake_status(m_pSsl);

	if ( retval != SSL_OK)
	{
		NLOG_ERR("establishConnectionToSSLServer - client SSL Handshake Failed with code %d", retval);
		Close();
		return false;
	}

	return true;
}


/**
 * Called by server to establich connection with a client
 * @return error or success
 * @remarks non blocking - should check for handshake completion either with CheckRecv or with BlockUntilSSLServerHandshakeCompletes
 */
bool CTcpSecureSocket::establishConnectionToSSLClient()
{
	if (m_enumConnectionSide != SERVER_SIDE)
	{
		NLOG_ERR("establishConnectionToSSLClient - m_enumConnectionSide != SERVER_SIDE ");
		Close();
		return false;
	}

	//non-blocking; check if handshake is finished and ok when using m_pSsl
	m_pSsl =  ssl_server_new(m_pSsl_ctx, m_socket);

	return true;
}


/**
 * Waits until the server's handshake with the client is finished
 * @return error or success
 */
bool CTcpSecureSocket::BlockUntilSSLServerHandshakeCompletes()
{
	if (m_pSsl == 0)
	{
		NLOG_ERR("CTcpSecureSocket::BlockUntilSSLServerHandshakeCompletes() - no ssl connection");
		return false;
	}

	while ( ssl_handshake_status(m_pSsl) != SSL_OK )
	{
		int len;
		len = ssl_read(m_pSsl,0);  //re-initialize handshake status flag
	}
	
	return true;
}

