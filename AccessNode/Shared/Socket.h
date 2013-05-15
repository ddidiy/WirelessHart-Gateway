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
	file:		Socket.h
	author:		Claudiu Hobeanu, claudiu.hobeanu@nivis.com

	purpose:	 a wrapper class for bsd sockets
*********************************************************************/


#if !defined(AFX_SOCKET_H__6A87450F_D9E9_4D92_A92F_6B537E76B534__INCLUDED_)
#define AFX_SOCKET_H__6A87450F_D9E9_4D92_A92F_6B537E76B534__INCLUDED_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <stdlib.h>


/// @addtogroup libshared
/// @{

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1


class CNetAddressExt
{
public:
	static const int	m_nIPv6AddrLen = 16;

public:
	CNetAddressExt() {memset(this,0, sizeof(*this));}

	~CNetAddressExt() { free ( m_szIfName ) ; }
	
	void SetIP (unsigned int p_unIPv4) {m_nFamily = AF_INET; m_unIPv4 = p_unIPv4;}
	void SetIP (unsigned char p_pucPv6[]) {m_nFamily = AF_INET6; memcpy(m_pucIPv6,p_pucPv6,m_nIPv6AddrLen);}
	void SetPort (unsigned short p_usPort) { m_usPort = htons(p_usPort); }
	void SetIfName (const char* p_szIfName) 
	{ 
		if (m_szIfName == p_szIfName)
		{
			return;
		}

		if (m_szIfName) free (m_szIfName);
		m_szIfName = p_szIfName ? strdup (p_szIfName) : NULL;
	}

	CNetAddressExt (const CNetAddressExt &p_NetAddr):
	m_nFamily (p_NetAddr.m_nFamily),
		m_unIPv4 (p_NetAddr.m_unIPv4),
		m_usPort (p_NetAddr.m_usPort),
		m_unIfScopedId (p_NetAddr.m_unIfScopedId)
	{
		m_szIfName = p_NetAddr.m_szIfName ? strdup (p_NetAddr.m_szIfName) : NULL;
		memcpy (m_pucIPv6, p_NetAddr.m_pucIPv6, CNetAddressExt::m_nIPv6AddrLen);
	}

	CNetAddressExt& operator= (const CNetAddressExt &p_NetAddr)
	{
		if (this == &p_NetAddr)
		{
			return *this;
		}
		SetIfName(p_NetAddr.m_szIfName);
		memcpy (m_pucIPv6, p_NetAddr.m_pucIPv6, CNetAddressExt::m_nIPv6AddrLen);
		m_nFamily = p_NetAddr.m_nFamily;
		m_unIPv4 = p_NetAddr.m_unIPv4;
		m_usPort = p_NetAddr.m_usPort;
		m_unIfScopedId = p_NetAddr.m_unIfScopedId;
		return *this;
	}

	void SetIfScopedId( unsigned int p_unIfScopedId) {	m_unIfScopedId = p_unIfScopedId;}

	int						GetFamily()		const	{ return m_nFamily; }
	unsigned int			GetIPv4()		const	{ return m_unIPv4; }
	const unsigned char*	GetIPv6()		const	{ return m_pucIPv6; }
	unsigned short			GetPort()		const	{ return ntohs(m_usPort); }
	unsigned int			GetIfScopedId() const	{ return m_unIfScopedId; }	
	char*					GetIfName()		const	{ return m_szIfName; }
	
protected:
	int m_nFamily;		//AF_UNSPEC(0) -- not spec, AF_INET -- IPv4, AF_INET6 -- IPv6
	union
	{
		unsigned int	m_unIPv4;
		unsigned char	m_pucIPv6[m_nIPv6AddrLen];
	};
	unsigned short	m_usPort;
	char*			m_szIfName;
	unsigned int	m_unIfScopedId;	
};

class  CWrapSocket
{
public:
	CWrapSocket();

	virtual ~CWrapSocket();

public:

	virtual bool Create( int p_nType, int p_nFamily = AF_INET);
	bool Bind( unsigned short p_nPort, const char* p_szLocalAddress = NULL, const char* p_szIfName = NULL );
	bool Bind( const CNetAddressExt& p_rNetAddr);

	virtual void Close();

	//time is in usec
	virtual bool CheckRecv( unsigned int  p_nTimeout = 10000 ) ;

	/// Check if can send to socket. Time is in usec
	virtual bool CanSend( void ) ;

	virtual void Attach(int p_nSock) {	Close(); m_socket = p_nSock; }


	bool IsValid(){	return (m_socket != INVALID_SOCKET);}
	operator int() const { return m_socket; }
	void SetPort( unsigned short p_nPort ) { m_nPort = htons(p_nPort);}
	unsigned short GetPort( void ) { return  ntohs(m_nPort); };

	static unsigned long GetLocalIp();

protected:
	int			m_socket;
	unsigned short	m_nPort;	//net order
	int			m_nSockFamily;
	int			m_nSockType;
private:
	CWrapSocket( const CWrapSocket& p_rSock)
	{
		//just to remove compiling warning
		m_socket = p_rSock.m_socket ;
		m_nPort = p_rSock.m_nPort;
	}
	int	m_nOnline;

};

/// @}
#endif // !defined(AFX_SOCKET_H__6A87450F_D9E9_4D92_A92F_6B537E76B534__INCLUDED_)
