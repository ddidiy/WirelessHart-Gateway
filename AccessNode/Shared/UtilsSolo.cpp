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


#include "UtilsSolo.h"
#include "DurationWatcher.h"

#include <stdio.h> 
#include <deque>

#include <arpa/inet.h>

const char * GetHexC( const void* p_vBuff, int p_nLen) 
{ 
	return GetHex(p_vBuff,p_nLen,-1); 
}

#define HEX_BUFF_DIM 2048
const char* GetHex( const void* p_vBuff, int p_nLen, int p_cByteDelim /*=-1*/)
{
	static char str[HEX_BUFF_DIM];
	static int pos = 0;


	int nByteSpace = p_cByteDelim == -1 ? 2 :3;

	if ( (p_nLen * nByteSpace + 1) >= HEX_BUFF_DIM)
	{	return "buffer too small";
	}

	if ( pos + p_nLen * nByteSpace + 1 >= HEX_BUFF_DIM )
	{	//overwrite the first part of the buffer
		pos = 0;
	}

	str[pos] = 0;
	const unsigned char*	pBuff = (const unsigned char*)p_vBuff;

	int i=0;
	for(; i < p_nLen; i++ )
	{	
		if (p_cByteDelim == -1)
		sprintf( str + pos + i*2, "%02X", pBuff[i] );
		else
			sprintf( str + pos + i*3, "%02X%c", pBuff[i], p_cByteDelim );
	}

	const char*		szHex = str + pos;
	pos += p_nLen *nByteSpace + 1;

	return szHex;
}


static	CProcessTimes	s_oProcessStart;
static	std::deque<CProcessTimes>	s_oLastMin15Vector;

int GetProcessLoad (double& p_rdLoadMin1, double& p_rdLoadMin5, double& p_rdLoadMin15, double& p_rLoadTotal, int p_nWhen )
{
	if (s_oLastMin15Vector.size() < 15)
	{		
		s_oLastMin15Vector.assign(15, CProcessTimes());
	}

	CProcessTimes	oCrtTime;
	int nRet = 0;

	int nRealDiff = oCrtTime.RealGetDiff(s_oLastMin15Vector[0]);

	p_rdLoadMin1 = (nRealDiff > 0) ? (oCrtTime.ProcGetTotalDiff(s_oLastMin15Vector[0]) / (double)nRealDiff) : 0;

	if ( nRealDiff >= 60 * 1000 || nRealDiff < 0)
	{
		s_oLastMin15Vector.push_front( oCrtTime);
		s_oLastMin15Vector.pop_back();
		nRet = 1;
	}
	else if (p_nWhen != GET_PROCESS_LOAD_ALWAYS)
	{
		return 0;
	}
	
	nRealDiff = oCrtTime.RealGetDiff(s_oLastMin15Vector[4]);
	p_rdLoadMin5 = 	(nRealDiff > 0) ? (oCrtTime.ProcGetTotalDiff(s_oLastMin15Vector[4]) / (double)nRealDiff) : 0;
	
	nRealDiff = oCrtTime.RealGetDiff(s_oLastMin15Vector[14]);
	p_rdLoadMin15 = (nRealDiff > 0) ? (oCrtTime.ProcGetTotalDiff(s_oLastMin15Vector[14]) / (double)nRealDiff) : 0;
	

	nRealDiff = oCrtTime.RealGetDiff(s_oProcessStart);

	if (nRealDiff < 0)
	{
		s_oProcessStart = oCrtTime;
	}
	
	p_rLoadTotal = 	 (nRealDiff > 0) ? (oCrtTime.ProcGetTotalDiff(s_oProcessStart) / (double)nRealDiff) : 0;

	return nRet;	
}

#ifdef HW_I386

#include <ifaddrs.h>

int
InterfaceIndex (const char *interface)
{
	int index=-1;
	struct ifaddrs *ifaddr, *ifa;
	if (getifaddrs(&ifaddr) == -1)
	  {
	    LOG ("getifaddr error: %s", strerror (errno));
	  }
	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
	{
		if (ifa->ifa_addr == NULL)
			continue;
		if ( (AF_INET6 == ifa->ifa_addr->sa_family) && 
			 (!strcmp (interface, ifa->ifa_name) ) )
		{
			index = ((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_scope_id;
			break;
		}
	}
	freeifaddrs(ifaddr);
	return index;
}
#else  // HW_I386

#warning "TODO:  NOT USING getifaddrs .      :IOCTL:"
int
InterfaceIndex (const char *interface)
{
  return -1;
}
#endif	// HW_I386
