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

////////////////////////////////////////////////////////////////////////////////
/// @file MultiServerUdp.h
/// @brief A class for multiple UDP server sockets - interface
////////////////////////////////////////////////////////////////////////////////
#ifndef _MULTI_SERVER_UDP_H_
#define _MULTI_SERVER_UDP_H_

/// @addtogroup libshared
/// @{

#include "UdpSocket.h"

#define MAX_UDP_SLOTS 16
////////////////////////////////////////////////////////////////////////////////
/// @class CMultiServerUdp
/// @brief Container for multiple UDP server sockets. The server listen simultaneously on all ports
/// It has support for multiple IP's on the same machine.
/// Currently the class support up to 16 server sockets simultaneoulsy
////////////////////////////////////////////////////////////////////////////////
class CMultiServerUdp : public CUdpSocket  ///< Inheritance: just for SendTo
{
public:
	CMultiServerUdp();
	~CMultiServerUdp();

	/// Add a pair ip:port for the server to listen on. The IP is 4 bytes, network order
	/// TAKE CARE: max number of sockets is 16
    int AddPort( const char * p_IP, unsigned short p_ushLocalPort );
	
	/// Wait the specified timeout for a message to arrive on any of the sockets
	/// p_nTimeoutUSec - the max timeout, specified in microseconds
    /// p_pnIndex - user-allocated array of indexes. MUST be at least MAX_UDP_SLOTS wide
	/// The ports associated with the sockets are returned into parameter p_pnPorts
	/// (Parse the vector until reaching a -1)
    bool CheckRecv( int p_nTimeoutUSec, int * p_pnIndex = NULL );
	
	/// Receive a message on socket associated with p_nLocalPort, returning source IP as uint
    bool RecvFrom( int p_nIndex, char * p_pMsg, size_t* p_nLen, int flags = 0, u_int *p_pnIpFrom = NULL, int* p_pPortFrom = NULL );
	
	/// Receive a message on socket associated with p_nLocalPort, returning source IP as numbers-and-dots
    bool RecvFrom( int p_nIndex, char * p_pMsg, size_t* p_nLen, int flags, char *p_szIpFrom, int* p_pPortFrom = NULL  );

    /// Close all sockets
    void CloseAll( void );

private:

    /// Bind the pair IP:Port with the socket at the m_nPortFreeSlot index
    bool bind( const char * p_szIP, unsigned short p_ushLocalPort );

	int          m_nPortFreeSlot;
	CUdpSocket * m_pUdpSockets[ MAX_UDP_SLOTS ];
};

/// @}
#endif //_MULTI_SERVER_UDP_H_
