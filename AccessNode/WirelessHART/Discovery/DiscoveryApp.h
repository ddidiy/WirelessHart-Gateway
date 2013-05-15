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

#ifndef DISCOVERYAPP_H
#define DISCOVERYAPP_H

#include "../../Shared/app.h"
#include "../../Shared/Config.h"
#include <netinet/in.h>

#include "DiscoveryConfig.h"

extern volatile int m_nFd;

enum MessagTypes { DISCOVERY_QRY=1, DISCOVERY_REPLY, DISCOVERY_SET};

#define UCHAR unsigned char 
#define USHORT unsigned short
#define UINT unsigned int 

#define SERVPORT 41234


#define BUFLEN 512
#define NPACK 10
//#define PORT 10000

#ifdef M68K
/// TODO: quick-and-dirty fix for VR900
/// TODO: we should do setsockopt(..., SO_BINDTODEVICE, on both eth0 and eth1
	#define ETH0 "eth1"
	#define ETH0_0 "eth1:0"
	#define ETH0_1 "eth1:1"
#else
	#define ETH0 "eth0"
	#define ETH0_0 "eth0:0"
	#define ETH0_1 "eth0:1"
#endif

#define DISCOVERY_LOCK "/tmp/discovery.flock"
#define RC_NET_INFO "/etc/rc.d/rc.net.info"

#define DEFAULT_ETH0 1
#define DEFAULT_NO_ETH0 0
#define DEFAULT_NOT_SET -1

#define SOCK_SIZE sizeof (struct sockaddr_in)


class CDiscoveryApp : public CApp
{
public:

    char configFile[256];	
    CDiscoveryApp();
    ~CDiscoveryApp();
    CConfig m_stCfg;	
    CWHDiscoveryCfg m_WHDisc;
    /** mandatory initialisations  */

    	
    int Init( const char * p_szInterface );

    /** main loop. this should never end,  unless rebooted/killed, of course*/
    int Run( void );
    int udpServer( void );	

    	

public:
	void Close();


private:

	int sock_v;
	sockaddr_in si_me, si_other;
	
	//DiscoveryResponse_Msg  m_stDiscoveryResponse;
	unsigned char m_ucEth0MAC[6];
	
	
	int  num_interfaces;
	
private: // Private methods
	

	int processDataBIN(unsigned char *p_pBuffIn, int p_nLenIn,unsigned char *p_pBuffOutMax, int p_nLenOutMax, const struct sockaddr_in& p_stClientAddr);
	int processDataXML(unsigned char *p_pBuffIn, int p_nLenIn,unsigned char *p_pBuffOutMax, int p_nLenOutMax, const struct sockaddr_in& p_stClientAddr);

	unsigned int  getANId();	// Return ANID already converted to network order
	
	unsigned char*& incrBuffer(unsigned char *& p, unsigned offset);
	
	
	
	bool checkRecv(int p_nSocket, int p_nTimeout);

	
		
};

extern CDiscoveryApp g_stApp;

/* This structure gets passed by the SIOCADDRT and SIOCDELRT calls. */
struct rtentry
  {
    unsigned long int rt_pad1;
    struct sockaddr rt_dst;             /* Target address.  */
    struct sockaddr rt_gateway;         /* Gateway addr (RTF_GATEWAY).  */
    struct sockaddr rt_genmask;         /* Target network mask (IP).  */
    unsigned short int rt_flags;
    short int rt_pad2;
    unsigned long int rt_pad3;
    unsigned char rt_tos;
    unsigned char rt_class;
#if __WORDSIZE == 64
    short int rt_pad4[3];
#else
    short int rt_pad4;
#endif
    short int rt_metric;                /* +1 for binary compatibility!  */
    char *rt_dev;                       /* Forcing the device at add.  */
    unsigned long int rt_mtu;           /* Per route MTU/Window.  */
    unsigned long int rt_window;        /* Window clamping.  */
    unsigned short int rt_irtt;         /* Initial RTT.  */
  };
/* Compatibility hack.  */

#define RTF_UP          0x0001          /* Route usable.  */

#define RTF_GATEWAY     0x0002          /* Destination is a gateway.  */



#endif 
