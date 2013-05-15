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

#include "TCPLogSync.h"
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>


CFLog g_stWdtLog;

bool CTCPSink::open(const char* host, unsigned int port)
{
	skt = socket(AF_INET,SOCK_STREAM,0);
	if ( skt <0 ) return false;
	struct sockaddr_in addr ;
	addr.sin_port=htons(port);
	addr.sin_addr.s_addr = inet_addr(host);
	addr.sin_family = AF_INET;
	int rv=connect(skt, (struct sockaddr*)&addr, sizeof(addr));
	if ( rv <0 )
	{
		skt=-1;
		return false ;
	}
	return true ;
}
bool CTCPSink::dissociate( )
{
	int rv = 1;
	if ( skt>0 )
	{
		rv = close(skt);
		skt=0;
	}
	return (rv >= 0 );
}
void CTCPSink::consume( const std::ostringstream& msg  )
{
	if ( skt <0 ) return ;
	int rv = write( skt, msg.str().c_str(), msg.str().length() );
	if ( -1 == rv )
	{
		dissociate() ;
	}
}
void CTCPSink::consume( const char* message, va_list ap  )
{
	if ( skt <0 ) return ;
	//vprintf( message, ap );
	//vfprintf( m_pFile, message, ap );
}
