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

#ifndef __STREAM_LINK_H
#define __STREAM_LINK_H

#include "Common.h"
#include "link.h"
#include "ServerSocket.h"
#include "UdpSocket.h"

/// @addtogroup libshared
/// @{

#define USEC_PER_SEC 1000000

#define LINK_SERIAL			0
#define LINK_TCP_CLIENT		1
#define LINK_TCP_SERVER		2
#define LINK_UDP			3

class CStreamLink
{
public:
	CStreamLink( bool p_bRawLog = true) { m_bRawLog = p_bRawLog; }
	virtual ~CStreamLink() {}

public:
	virtual int	OpenLink( const  char * p_sConn, unsigned int  p_nConnParam, unsigned int p_nConnParam2 = 0  ) = 0;
	virtual void	CloseLink () = 0;
	virtual int	IsLinkOpen () = 0;

	virtual int	HaveData(u_int p_nMicroSec = 10000) = 0;
	virtual int     GetMsgLen( u_int tout=1000 ) =0;
	virtual int	Read( u_char* p_pBuffer, u_int p_nLen, u_int p_nMicroSec = 10000) = 0;
	virtual int	Write( const u_char* p_pBuffer, u_int p_nLen ) = 0;

	virtual void	SetRawLog(bool p_bRawLog) { m_bRawLog = p_bRawLog; }

protected:
	bool	m_bRawLog;
};

class CSerialLink : public CStreamLink
{
public:
	CSerialLink(bool p_bRawLog = true) : CStreamLink(p_bRawLog) {}

public:
	int OpenLink( const  char * p_sConn, unsigned int  p_nConnParam, unsigned int /*p_nConnParam2*/ = 0 )
	{
		return m_oSerial.OpenLink(p_sConn,p_nConnParam);
	}

	void CloseLink ()
	{
		m_oSerial.CloseLink();
	}
	int GetMsgLen(u_int /*timeout*/=1000)
	{
		LOG("Function unavailable for SerialLink");
		//assert(false);
		return -1 ;
	}

	int IsLinkOpen ()
	{
		return m_oSerial.IsLinkOpen();
	}

	int HaveData(u_int p_nMicroSec = 10000)
	{
		return m_oSerial.haveData(p_nMicroSec / USEC_PER_SEC, p_nMicroSec % USEC_PER_SEC);
	}

	int Read( u_char* p_pBuffer, u_int p_nLen, u_int p_nMicroSec = 10000 )
	{
		return m_oSerial.readBlock(p_pBuffer, p_nLen, p_nMicroSec / USEC_PER_SEC, p_nMicroSec % USEC_PER_SEC );
	}

	int Write( const u_char* p_pBuffer, u_int p_nLen )
	{
		return m_oSerial.writeBlock(p_pBuffer, p_nLen);
	}

	void SetRawLog(bool p_bRawLog) { m_bRawLog = p_bRawLog; m_oSerial.SetLogRaw(m_bRawLog); }
private:
	CLink	m_oSerial;
};


class CTcpClientLink : public CStreamLink
{
public:
	CTcpClientLink(bool p_bRawLog = true) : CStreamLink(p_bRawLog) {}

public:
	int OpenLink ( const  char * p_sConn, unsigned int  p_nConnParam, unsigned int p_nConnParam2 = 120 )
	{
		if (!m_oSock.Create())
		{	return 0;
		}

		if(m_oSock.Connect( p_sConn, p_nConnParam, p_nConnParam2 ))
		{
			int keepalive = 1;
			setsockopt((int)m_oSock, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive));
			return true;
		}else{
			return false;
		}
	}
	int OpenLink ( unsigned long p_nConn, unsigned int  p_nConnParam, unsigned int p_nConnParam2 = 120 )
	{
		if (!m_oSock.Create())
		{	return 0;
		}

		return m_oSock.Connect( p_nConn, p_nConnParam, p_nConnParam2 );
	}

	void CloseLink ()
	{
		m_oSock.Close();
	}

	int IsLinkOpen ()
	{
		return m_oSock.IsValid();
	}

	int HaveData(u_int p_nMicroSec = 10000)
	{
		if (!checkConnection())
		{	return 0;
		}
		return m_oSock.CheckRecv(p_nMicroSec);
	}

	int Read( u_char* p_pBuffer, u_int p_nLen, u_int p_nMicroSec = 10000 )
	{
		if (!HaveData())
		{	return 0;
		}

		int nLen = p_nLen;
		if( !m_oSock.Recv((int)p_nMicroSec, (char*)p_pBuffer, nLen ) )
		{	return -1;
		}

		p_nLen = nLen;
		if (m_bRawLog && p_nLen > 0 )
		{	LOG_HEX( "TcpLink::Read: ", p_pBuffer, p_nLen );
		}
		return p_nLen;
	}
	int GetMsgLen(u_int /*timeout*/=1000)
	{
		LOG("Function unavailable for TCP");
		//assert(false);
		return -1 ;
	}
	int Write( const u_char* p_pBuffer, u_int p_nLen )
	{
		if (!checkConnection())
		{	return 0;
		}

		if (m_bRawLog)
		{	LOG_HEX( "TcpLink::Write: ", p_pBuffer, p_nLen );
		}

		return m_oSock.Send( (char*)p_pBuffer, p_nLen);
	}


protected:
	virtual int	checkConnection()
	{
		if (m_oSock.IsValid())
		{	return 1;
		}
		return m_oSock.Reconnect();
	}

protected:
	CTcpSocket	m_oSock;
};


class CTcpServerLink : public CTcpClientLink
{
public:
	CTcpServerLink(bool p_bRawLog = true) : CTcpClientLink(p_bRawLog) {}

public:
	int OpenLink ( const  char * /*p_sConn*/, unsigned int  p_nConnParam, unsigned int /*p_nConnParam2*/ = 0 )
	{	m_nServPort = p_nConnParam;
		return m_oServSock.StartServer(m_nServPort);
	}

	void CloseLink ()
	{
		if (m_oSock.IsValid())
		{	m_oSock.Close();
		}
		if (m_oServSock.IsValid())
		{
			m_oServSock.Close();
		}
	}

	int IsLinkOpen ()
	{
		return m_oServSock.IsValid() || m_oSock.IsValid();
	}
protected:
	virtual int	checkConnection()
	{
		if (m_oSock.IsValid())
		{	return 1;
		}

		if (!m_oServSock.IsValid())
		{	m_oServSock.StartServer(m_nServPort);
		}

		if (!m_oServSock.CheckRecv())
		{	return 0;
		}

		int ret = m_oServSock.Accept(&m_oSock);
		m_oServSock.Close();
		return ret;
	}

protected:
	CServerSocket		m_oServSock;
	unsigned short		m_nServPort;
};


class CUdpLink : public CStreamLink
{
public:
	CUdpLink(bool p_bRawLog = true)
		: CStreamLink(p_bRawLog)
		, m_nPktRemotePort(0)
	{}

public:
	int OpenLink ( const  char * p_sConn, unsigned int  p_nConnParam, unsigned int p_nConnParam2 )
	{
		m_nRemotePort = p_nConnParam;
		m_nLocalPort = p_nConnParam2;
		strcpy(m_szIP,p_sConn);
		m_bUseConfigDest = ((m_nRemotePort > 0) && m_szIP[0]);
		LOG("m_bUseConfigDest=%d", m_bUseConfigDest);
		return checkConnection();
	}

	void CloseLink ()
	{
		m_oSock.Close();
	}

	int IsLinkOpen ()
	{
		return m_oSock.IsValid();
	}

	int HaveData(u_int p_nMicroSec = 10000)
	{
		if (!checkConnection())
		{	return 0;
		}
		return m_oSock.CheckRecv(p_nMicroSec);
	}
	/**
	 * @brief Returns the size of the waiting UDP kernel message.
	 * @param [in] p_nMicroSec =10000. select() timeout in microseconds.
	 * Doesn't work on kernels < 2.6.9(?) */
	int GetMsgLen(u_int p_nMicroSec = 10000)
	{
		if ( !HaveData(p_nMicroSec) )
			return -1 ;

		return m_oSock.GetMsgLen( ) ;
	}

	int Read( u_char* p_pBuffer, u_int p_nLen, u_int p_nMicroSec = 10000 )
	{
		if (!checkConnection())
		{	return 0;
		}

		if (!HaveData(p_nMicroSec))
		{	return 0;
		}

		char szIp[32];
		size_t nLen = p_nLen;
		int nPort=0 ;
		if( !m_oSock.RecvFrom( (char*)p_pBuffer, &nLen, 0, szIp, &nPort ) )
		{	return -1;
		}
		m_nPktRemotePort = nPort ;
		if (!m_bUseConfigDest)
		{	strcpy(m_szIP,szIp);
			m_nRemotePort = m_nPktRemotePort;
			LOG("CUdpLink.Read:  <%s> <%d>", m_szIP, m_nPktRemotePort );
		}

		p_nLen = nLen;
		if (m_bRawLog && p_nLen > 0 )
		{	LOG_HEX( "UdpLink::Read:  ", p_pBuffer, p_nLen );
		}
		return p_nLen;
	}

	int Write( const u_char* p_pBuffer, u_int p_nLen )
	{
		if (!checkConnection())
		{	return 0;
		}

		if (m_bRawLog)
		{	LOG_HEX( "UdpLink::Write: ", p_pBuffer, p_nLen );
		}

		if (m_nRemotePort<=0 || !m_szIP[0])
		{	LOG("CUdpLink.Write: dest not set");
			return 0;
		}

		return m_oSock.SendTo (m_szIP, m_nRemotePort, (char*)p_pBuffer, p_nLen);
	}
	int PktRemotePort(void) const {
		return m_nPktRemotePort ;
	}
	int RemotePort(void) const {
		return m_nRemotePort ;
	}
	void RemotePort(int p_nRemotePort) {
		m_nRemotePort = p_nRemotePort ;
	}

protected:
	virtual int	checkConnection()
	{
		if (m_oSock.IsValid())
		{	return 1;
		}
		if (!m_oSock.Create())
		{	return 0;
		}
		if (m_nLocalPort<=0)
		{
				LOG("ERR: checkConnection failed: m_nLocalPort[%d]", m_nLocalPort);
				return 0;
		}

		return m_oSock.Bind(m_nLocalPort);
	}

protected:
	CUdpSocket	m_oSock;
	bool		m_bUseConfigDest;
	int	m_nPktRemotePort; /// extracted from recvfrom
	unsigned int	m_nRemotePort; /// parameter of OpenLink
	unsigned int	m_nLocalPort;
	char		m_szIP[32];
};
/// @}
#endif //__STREAM_LINK_H
