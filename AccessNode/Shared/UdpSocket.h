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

// UdpSocket.h: interface for the CUdpSocket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UDPSOCKET_H__0D6A4165_1FEA_48F2_8803_9D7BB1040433__INCLUDED_)
#define AFX_UDPSOCKET_H__0D6A4165_1FEA_48F2_8803_9D7BB1040433__INCLUDED_



#include "Socket.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/// @addtogroup libshared
/// @{

class CUdpSocket : public CWrapSocket  
{
public:
	CUdpSocket();
	virtual ~CUdpSocket();

	bool Create(int p_nType = SOCK_DGRAM, int p_nFamily = AF_INET);
	bool SendTo (unsigned int p_unTo, int p_nPort, const void *msg, size_t len, int flags = 0);
	bool SendTo (const char* p_szTo, int p_nPort, const void *msg, size_t len, int flags = 0);
	bool RecvFrom( void *msg, size_t* p_nLen, int flags, char *p_szIpFrom, int* p_pPortFrom = NULL  );
	bool RecvFrom( void *msg, size_t* p_nLen, int flags = 0, u_int *p_pnIpFrom = NULL, int* p_pPortFrom = NULL );

	bool RecvFrom	(CNetAddressExt* p_pNetAddrFrom, void *msg, size_t* p_nLen, int flags = 0);
	bool SendTo		(const CNetAddressExt* p_pNetAddrTo, const void *msg, size_t len, int flags = 0);

	bool RecvFrom6(  void *msg, size_t* p_nLen, int flags = 0, char* p_szIPv6 = NULL, int* p_pPortFrom = NULL );
	bool RecvFrom6(  void *msg, size_t* p_nLen, int flags,  uint8_t* p_pIPv6 = NULL, int* p_pPortFrom  = NULL );

	bool SendTo6 (const char* p_szTo, int p_nPort, const void *msg, size_t len, int flags = 0, unsigned int p_unIfScopedId = 0);
	bool SendTo6(const uint8_t * p_pTo, int p_nPort, const void *msg, size_t len, int flags, unsigned int p_unIfScopedId = 0 );

	int  GetMsgLen( void ) ;
	bool AllowBroadcast(void);
};

/// @}
#endif // !defined(AFX_UDPSOCKET_H__0D6A4165_1FEA_48F2_8803_9D7BB1040433__INCLUDED_)
