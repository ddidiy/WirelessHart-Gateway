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
/// @file MultiServerUdp.cpp
/// @brief A class for multiple UDP server sockets - implementation
////////////////////////////////////////////////////////////////////////////////

#include "MultiServerUdp.h"
#include "Common.h"

////////////////////////////////////////////////////////////////////////////////
/// @class CMultiServerUdp
////////////////////////////////////////////////////////////////////////////////

CMultiServerUdp::CMultiServerUdp()
:m_nPortFreeSlot(0)
{

}

CMultiServerUdp::~CMultiServerUdp()
{
	for( int i=0; i< m_nPortFreeSlot; i++)
	{
		delete m_pUdpSockets[  i ];
	}
	m_nPortFreeSlot = 0;
}

////////////////////////////////////////////////////////////////////////////////
/// @author Marcel Ionescu
/// @brief Bind the pair IP:Port with the socket at the m_nPortFreeSlot index
/// @param p_szIP Number-and-dots represented IP
/// @param p_ushLocalPort the port number to listen on
/// @retval true - bind successfull
/// @retval false - cannot bind, some socket error had occured 
///     (example: somebody else is listening on the port)
////////////////////////////////////////////////////////////////////////////////
bool CMultiServerUdp::bind( const char * p_szIP, unsigned short p_ushLocalPort )
{
    sockaddr_in	sAddr;
    struct in_addr sIn;

    inet_aton( p_szIP, &sIn);

    sAddr.sin_addr      = sIn ;
    sAddr.sin_family    = AF_INET;
    sAddr.sin_port      = p_ushLocalPort;

    int nBindErr = ::bind( (int)*m_pUdpSockets[ m_nPortFreeSlot ], (sockaddr *)&sAddr, sizeof(sAddr));
    if(SOCKET_ERROR == nBindErr)
    {	
        LOG_ERR("CMultiServerUdp::bind(%s:%d)", p_szIP, p_ushLocalPort);
        return false;
    }

    m_pUdpSockets[ m_nPortFreeSlot ]->SetPort(p_ushLocalPort);
    return true;
}

////////////////////////////////////////////////////////////////////////////////
/// @author Marcel Ionescu
/// @brief Add a port for the server to listen on. Start listening on the port
/// @param p_szIP Number-and-dots represented IP
/// @param p_ushLocalPort the port number to listen on
/// @return the socket index, or -1 if cannot add port. 
///    The caller will use the index to identify the socket from now on
/// @retval -1 - the port cannot be added, because:
///    the 16 ports limit was exceeded, 
///    or some socket error had occured. Example: 
///         somebody else is listening on the port OR
///         ip:port pair is not availalbe
/// @remarks MAX number of sockets is 16 in this implementation
///    TODO: implement using std::vector to get rid of 16 ports limit
////////////////////////////////////////////////////////////////////////////////
int CMultiServerUdp::AddPort( const char * p_IP, unsigned short p_ushLocalPort )
{
    if( m_nPortFreeSlot > MAX_UDP_SLOTS )
	{
		LOG("ERROR CMultiUdpSocket::AddPort(%s:%d): max number of ports (16) exceeded.", p_ushLocalPort);
        return -1;
	}
	
	m_pUdpSockets[ m_nPortFreeSlot ] = new CUdpSocket();
    if( m_pUdpSockets[ m_nPortFreeSlot ]->Create() && bind( p_IP, p_ushLocalPort ) )
	{
        // Advance to next free slot after returning the current index
        return m_nPortFreeSlot++;
	}

	delete m_pUdpSockets[ m_nPortFreeSlot ] ;
    return -1;
}

//#define _Max(a,b) ( ((a)>(b)) ? (a) : (b) )
////////////////////////////////////////////////////////////////////////////////
/// @author Marcel Ionescu
/// @brief Wait the specified timeout for a message to arrive on any of the sockets
/// @param p_nTimeoutUSec - the max timeout, specified in microseconds (sec*10^-6)
/// @param p_pnIndex - user-allocated array of indexes; MUST be at least MAX_UDP_SLOTS wide. 
///     The method will return indexes associated with the sockets with pending data
///     The client should parse the vector until reaching the value of -1
///     If NULL, the array is not populated
/// @retval true - at least one of the sockets has data pending 
/// @retval false - there is no data, the function is returning because:
///    timeout expired OR
///    signal received OR
///    socket error
/// @remarks
////////////////////////////////////////////////////////////////////////////////
bool CMultiServerUdp::CheckRecv( int p_nTimeoutUSec, int * p_pnIndex /*=NULL*/)
{ 
    fd_set		    readfds;
    struct timeval  tmval;
    int             nFs = 0;

    tmval.tv_usec = p_nTimeoutUSec%1000000;
    tmval.tv_sec = p_nTimeoutUSec/1000000;
    FD_ZERO(&readfds);

    for( int i = 0; i< m_nPortFreeSlot; i++ ) 
    {
        nFs = _Max( nFs, (int)m_pUdpSockets[ i ] );
        FD_SET( (int)*m_pUdpSockets[ i ], &readfds);
    }

    int nSelected = ::select( nFs + 1, &readfds, NULL, NULL, &tmval);

    switch( nSelected)
    {
        case SOCKET_ERROR:
            LOG_ERR("CMultiServerUdp::CheckRecv");
            if (errno == EINTR )
            {	
                return false;
            }

        CloseAll(); // close all sockets
        return false;

        case 0:  // Timeout expired
            return false;

        default:
            if(!p_pnIndex)
            {
                return true;    // Data waiting on some socket
            }

            p_pnIndex[ 0 ] = -1;
            for( int i = 0, j = 0; i<m_nPortFreeSlot; i++ ) 
            {
                if( FD_ISSET( (int)*m_pUdpSockets[ i ], &readfds ) )
                {
                    p_pnIndex[ j++ ] = i ; 
                    p_pnIndex[ j   ] = -1; //terminate the list
                }
            }
    }

    return true;    // Data waiting on some socket
}
	

////////////////////////////////////////////////////////////////////////////////
/// @author Marcel Ionescu
/// @brief Receive a message on socket associated with p_nLocalPort, returning source IP as uint
/// @param p_nIndex - the index associated with the socket to use
/// @param p_pMsg - data buffer, al least p_nLen bytes
/// @param p_nLen - [in]: the max data length. [out]: the actual data received
/// @param flags - socket flags, passed dirrectly to ::recvfrom()
/// @param  p_pnIpFrom - If not NULL, it will receive the source IP as u_int
/// @param p_pPortFrom - If not NULL, it will receive the source port
/// @retval true a message was received
/// @retval false no message was received (socket error or socket reset)
/// @remarks In all cases where false is returned, the socket is closed
///   TAKE CARE: the call is blocking until data is received. 
///   Make sure to call RecvFrom only after CheckRecv indicated that there is 
///   data on the socket associated with p_nLocalPort 
////////////////////////////////////////////////////////////////////////////////
bool CMultiServerUdp::RecvFrom( int p_nIndex, char * p_pMsg, size_t* p_nLen, int flags /*= 0*/, u_int *p_pnIpFrom /*= NULL*/, int* p_pPortFrom /*= NULL*/ )
{
    return m_pUdpSockets[ p_nIndex ]->RecvFrom( p_pMsg, p_nLen, flags, p_pnIpFrom, p_pPortFrom);
}

////////////////////////////////////////////////////////////////////////////////
/// @author Marcel Ionescu
/// @brief Receive a message on socket associated with p_nLocalPort, returning source IP as numbers-and-dots
/// @param p_nIndex - the index associated with the socket to use
/// @param p_pMsg - data buffer, al least p_nLen bytes
/// @param p_nLen - [in]: the max data length. [out]: the actual data received
/// @param flags - socket flags, passed dirrectly to ::recvfrom()
/// @param  p_pnIpFrom - If not NULL, it will receive the source IP as numbers-and-dots
/// @param p_pPortFrom - If not NULL, it will receive the source port
/// @retval true a message was received
/// @retval false no message was received (socket error or socket reset)
/// @remarks In all cases where false is returned, the socket is closed
///   TAKE CARE: the call is blocking until data is received. 
///   Make sure to call RecvFrom only after CheckRecv indicated that there is 
///   data on the socket associated with p_nLocalPort 
////////////////////////////////////////////////////////////////////////////////
bool CMultiServerUdp::RecvFrom( int p_nIndex, char * p_pMsg, size_t* p_nLen, int flags, char *p_szIpFrom, int* p_pPortFrom /*= NULL*/  )
{
    return m_pUdpSockets[ p_nIndex ]->RecvFrom( p_pMsg, p_nLen, flags, p_szIpFrom, p_pPortFrom);
}

////////////////////////////////////////////////////////////////////////////////
/// @author Marcel Ionescu
/// @brief Close all sockets
////////////////////////////////////////////////////////////////////////////////
void CMultiServerUdp::CloseAll( void )
{
    for( int i = 0; i < m_nPortFreeSlot; i++ )
    {
        m_pUdpSockets[ i ]->Close();
    }
}


