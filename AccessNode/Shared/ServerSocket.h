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
	file:		ServerSocket.h
	author:		Claudiu Hobeanu, claudiu.hobeanu@nivis.com
	
	purpose:	 handles tcp server operations over a socket
*********************************************************************/




#if !defined(AFX_SERVERSOCKET_H__EF007CC4_DE83_4F8D_A807_2655F3982C49__INCLUDED_)
#define AFX_SERVERSOCKET_H__EF007CC4_DE83_4F8D_A807_2655F3982C49__INCLUDED_

/// @addtogroup libshared
/// @{

#include "Socket.h"
#include "TcpSocket.h"

class CServerSocket : public CWrapSocket  
{
public:
	CServerSocket();
	virtual ~CServerSocket();

	bool StartServer( unsigned short p_nPort, int nListenParam = 10 );
	bool Listen( int p_nNoConn );
	bool Accept(CTcpSocket* p_sock );
};

/// @}
#endif // !defined(AFX_SERVERSOCKET_H__EF007CC4_DE83_4F8D_A807_2655F3982C49__INCLUDED_)
