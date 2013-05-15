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

#define UPGRADE_WEB		NIVIS_TMP"upgrade_web/"
#define VERSION_FILE	NIVIS_FIRMWARE"version"

#include "MiscImpl.h"

#include <stdio.h>
#include <stdlib.h>

#include <dirent.h>
#include <sys/types.h>

#include <sys/resource.h>
#include <signal.h>

#include "../../../Shared/Utils.h"
#include "../../../Shared/IniParser.h"
#include "../../../Shared/FileLock.h"
#include "../../../Shared/UdpSocket.h"
#include "../../../Shared/SimpleTimer.h"
#include <fcntl.h> // O_RDRW
#include <sys/file.h> // flock

#define RESOLV_CONF		"/etc/resolv.conf.eth"

#define EMAIL_TMP_FILE	NIVIS_TMP"email_tmp_file.txt"
#define UPGRADE_LOCK	NIVIS_TMP"fw_upgrade.lock"
#define DISABLE_DHCP_FLAG_FILE "/etc/config/no_dhcp"

enum {
	NENOFILE,
	NEVALIDATIONERROR
};

uint32_t CValidator::Ip(const char* p_szIp, uint32_t p_uiMask)
{
	uint32_t ip32 ;
	if ( strlen(p_szIp) > 15 ) { FLOG("Inalid IP length");return 0 ;}

	unsigned int a1,a2,a3,a4;
	a1=a2=a3=a4=0;
	sscanf( p_szIp, "%3u.%3u.%3u.%3u", &a1,&a2,&a3,&a4) ;
	if ( a1>255 || a2>255 || a3>255 || a4>255 ) { FLOG("Invalid ip group");return 0 ;}
	ip32 = a1<<24 | a2<<16 | a3<<8 | a4 ;
	if ( ip32==0xFFFFFFFF || !ip32 ) {FLOG("[%X](%s) is 00000000/FFFFFFFF",ip32,p_szIp);return  0;}
	if ( (ip32& ~p_uiMask)==0 ) { FLOG("[%X] doesn't match the NetMask[%X]");return 0 ;}
	return ip32 ;
}

uint32_t CValidator::Mask( const char *p_szMask )
{
	int r=0;
	uint32_t mask32 = Ip ( p_szMask,0x00000000U );

	if ( !mask32 || mask32&1 ||   0xFFFFFFFF == mask32 ) { FLOG("NetMask[%X] is 00000000 or FFFFFFFF or has 1 on right",mask32);return 0 ;}
	//http://www-graphics.stanford.edu/~seander/bithacks.html#ZerosOnRightMultLookup
	r = MultiplyDeBruijnBitPosition[((mask32 & -mask32) * 0x077CB531UL) >> 27];
	if ( 0xFFFFFFFF != (mask32 | (0xFFFFFFFF>>(32-r)) ) )
	{
		FLOG("Interspersed 0/1 found in NetMask[%X]",mask32);
		return 0 ;
	}
	return mask32 ;
}

uint32_t CValidator::Gateway( const char *p_szGw, uint32_t mask, uint32_t ip32 )
{
	uint32_t gw32 = Ip(p_szGw,mask);
	if (    !gw32
			||   (gw32 & mask) != (ip32 & mask)
			||   ~mask == (gw32 & ~mask) )
	{
		FLOG("Invalid Gateway[%X](%s):mask[%X]:ip[%X]",gw32,p_szGw, mask, ip32);
		return 0;
	}
	return gw32;
}

CMiscImpl::DiscoveryInfo::DiscoveryInfo( uint32_t p_u32_anID,
			uint32_t p_u32_staticIP,
			uint32_t p_u32_dynamicIP)
{
	sprintf(m_szAnID, "%08d", p_u32_anID);

	if (!inet_ntop(AF_INET, &p_u32_staticIP, m_szStaticIP, sizeof(m_szStaticIP))){
		PWARN("VRInfo, inet_ntop, StaticIP");
	}

	if (!inet_ntop(AF_INET, &p_u32_dynamicIP, m_szDynamicIP, sizeof(m_szDynamicIP))){
		PWARN("VRInfo, inet_ntop, DynamicIP");
	}
}

char *CMiscImpl::getVersion(void)
{
	char *line = new char[256 * sizeof (char)];
	FILE* f=fopen(VERSION_FILE, "r");
	if ( !f )
	{
		ERR("Unable to open "VERSION_FILE);
		delete [] line ;
		return NULL ;
	}
	if ( !fgets( line, 256, f) )
	{
		ERR("Unable to get version");
		delete [] line ;
		return NULL ;
	}
	return line;
}


bool getImageSize(const char *fn, int *x,int *y)
{
	FILE *f = fopen(fn, "rb");
	if (f == 0)
		return false;

	fseek(f, 0, SEEK_END);
	long len = ftell(f);
	fseek(f, 0, SEEK_SET);
	if (len < 24)
	{
		fclose(f);
		return false;
	}

	// Strategy:
	// reading GIF dimensions requires the first 10 bytes of the file
	// reading PNG dimensions requires the first 24 bytes of the file
	// reading JPEG dimensions requires scanning through jpeg chunks
	// In all formats, the file is at least 24 bytes big, so we'll read that always
	unsigned char buf[24];
	size_t br = fread(buf, 1, 24, f);
	if (br < 24)
	{
		if ( ferror(f) )
			FPERR("getImageSize: failed read(%s)", fn);
		else
			LOG_ERR("getImageSize: failed read(%s)", fn);
		fclose(f);
		return false;
	}

	// For JPEGs, we need to read the first 12 bytes of each chunk.
	// We'll read those 12 bytes at buf+2...buf+14, i.e. overwriting the existing buf.
	if (buf[0] == 0xFF && buf[1] == 0xD8 && buf[2] == 0xFF && buf[3] == 0xE0 &&
		buf[6] == 'J' && buf[7] == 'F' && buf[8] == 'I' && buf[9] == 'F')
	{
		long pos = 2;
		while (buf[2]==0xFF) {
			if (buf[3] == 0xC0 || buf[3] == 0xC1 || buf[3] == 0xC2 || buf[3] == 0xC3 ||
				buf[3] == 0xC9 || buf[3] == 0xCA || buf[3] == 0xCB)
				break;
			pos += 2 + (buf[4] << 8) + buf[5];
			if (pos + 12 > len)
				break;
			fseek(f, pos, SEEK_SET);
			br = fread(buf + 2, 1, 12, f);
			if (br < 12)
			{
				if ( ferror(f) )
					FPERR("getImageSize: failed read(%s)", fn);
				else
					LOG_ERR("getImageSize: failed read(%s)", fn);
				fclose(f);
				return false;
			}
		}
	}
	fclose(f);
	// JPEG: (first two bytes of buf are first two bytes of the jpeg file; rest of buf is the DCT frame
	if (buf[0] == 0xFF && buf[1] == 0xD8 && buf[2] == 0xFF)
	{
		*y = (buf[7] << 8) + buf[8];
		*x = (buf[9] << 8) + buf[10];
		return true;
	}
	// GIF: first three bytes say "GIF", next three give version number. Then dimensions
	if (buf[0] == 'G' && buf[1] == 'I' && buf[2] == 'F')
	{
		*x = buf[6] + (buf[7] << 8);
		*y = buf[8] + (buf[9] << 8);
		return true;
	}
	// PNG: the first frame is by definition an IHDR frame, which gives dimensions
	if (buf[0] == 0x89 && buf[1] == 'P' && buf[2] == 'N' && buf[3] == 'G' &&
		buf[4] == 0x0D && buf[5] == 0x0A && buf[6] == 0x1A && buf[7] == 0x0A &&
		buf[12] == 'I' && buf[13] == 'H' && buf[14] == 'D' && buf[15] == 'R')
	{
		*x = (buf[16] << 24) + (buf[17] << 16) + (buf[18] << 8) + (buf[19] << 0);
		*y = (buf[20] << 24) + (buf[21] << 16) + (buf[22] << 8) + (buf[23] << 0);
		return true;
	}
	return false;
}

bool CMiscImpl::fileUpload(const char *p_szScript, const char*p_kszScriptParam, char *& p_szScriptRsp) {
	size_t a= p_szScript?strlen(p_szScript):0 ;
	size_t b= p_kszScriptParam?strlen(p_kszScriptParam):0;

	if ( ! FileExist(p_szScript) )
	{
		WARN("No upload script:[%s]", p_szScript );
		return false ;
	}
	if(!lockUpgradeFile())
	{
		return false;
	}
	char * cmd = new char[ a+b+2 ] ;
	sprintf( cmd, "%s %s", p_szScript?p_szScript:"", p_kszScriptParam?p_kszScriptParam:"");
	LOG("fileUpload: sh=%s",cmd);

	bool bRet = execCmd(cmd, p_szScriptRsp);
	delete [] cmd ;
	LOG("fileUpload:DONE");
	unlockUpgradeFile();
	return bRet;
}


bool CMiscImpl::fileDownload(const char *p_szScript, const char*p_kszScriptParam, char *& p_szScriptRsp)
{
	size_t a= p_szScript?strlen(p_szScript):0 ;
	size_t b= p_kszScriptParam?strlen(p_kszScriptParam):0;

	if ( !FileExist(p_szScript) )
	{
		return false ;
	}

	char * cmd = new char[ a+b+2 ] ;
	sprintf( cmd, "%s %s", p_szScript?p_szScript:"", p_kszScriptParam?p_kszScriptParam:"");
	LOG("fileDownload: sh=%s",cmd);
	bool bRet = execCmd(cmd, p_szScriptRsp);
	delete [] cmd ;
	return bRet;
}

/// TAKE CARE: p_szCmdRsp memory must be deleted by the caller
bool CMiscImpl::execCmd(const char *p_szCmd, char *& p_szCmdRsp)
{
	FLOG("cmd[%s]",p_szCmd);
	char szTmpFileSH[128];
	char szTmpFileOut[128];
	int nTime = time(NULL);
	sprintf(szTmpFileSH,NIVIS_TMP"web_ra_%d.sh", nTime);
	sprintf(szTmpFileOut,NIVIS_TMP"web_ra_%d.out", nTime);
	WriteToFile(szTmpFileSH, "#!/bin/sh\n.  /etc/profile\n set -x\n", true);
	WriteToFile(szTmpFileSH, p_szCmd);
	systemf_to(600, "chmod a+x %s >%s 2>&1", szTmpFileSH, szTmpFileOut);
	int nRet = systemf_to(600, "%s >%s 2>&1", szTmpFileSH, szTmpFileOut);
	int nRspLen = 0;
	GetFileData(szTmpFileOut, p_szCmdRsp, nRspLen);
	unlink(szTmpFileOut);
	unlink(szTmpFileSH);
	return !nRet;
}
bool CMiscImpl::lockUpgradeFile(void)
{
	m_nFd = open( UPGRADE_LOCK , O_RDWR | O_CREAT, 0666 );
	if (m_nFd < 0)
	{	LOG_ERR("lockUpgradeFile: failed open(%s)", UPGRADE_LOCK);
		return false;
	}
	if ( ::flock( m_nFd, LOCK_NB|LOCK_EX )!=0)
	{
		LOG_ERR("lockUpgradeFile: failed flock(%s)", UPGRADE_LOCK);
		struct stat buf;
		if( stat(UPGRADE_LOCK , &buf) == 0 )
		{
			int nCrtTime = time(NULL);
			if(  nCrtTime < buf.st_mtime || nCrtTime - buf.st_mtime >= 300 )
			{
				LOG("lockUpgradeFile: lock(%s) hold too long -> unlink", UPGRADE_LOCK);
				if (unlink(UPGRADE_LOCK ) ) LOG_ERR("lockUpgradeFile: failed unlink(%s)", UPGRADE_LOCK);
			}
		}
		return false;
	}
	ssize_t bw = write(m_nFd, "1", 1);
	if (bw < 1)
	{
		FPERR("lockUpgradeFile: failed write(%s)", UPGRADE_LOCK);
		unlockUpgradeFile();
		close(m_nFd);
		return false;
	}
	return true;
}

bool CMiscImpl::unlockUpgradeFile(void)
{
	if (flock( m_nFd, LOCK_UN)!=0)
	{
		LOG_ERR("unlockUpgradeFile: failed flock(%s)", UPGRADE_LOCK);
		return false;
	}
	return true;
}

#define RC_NET_INFO "/etc/rc.d/rc.net.info"

bool CMiscImpl::getGatewayNetworkInfo( GatewayNetworkInfo& p_rGatewayNetworkInfo )
{
	CIniParser oP;

	if ( ! oP.Load(RC_NET_INFO) )
	{
		ERR("getGateway: unable to load "RC_NET_INFO);
		return false;
	}
	oP.GetVar( 0, "ETH0_IP",	p_rGatewayNetworkInfo.ip,	sizeof(p_rGatewayNetworkInfo.ip)    );
	oP.GetVar( 0, "ETH0_MASK",	p_rGatewayNetworkInfo.mask,	sizeof(p_rGatewayNetworkInfo.mask)  );
	oP.GetVar( 0, "ETH0_GW",	p_rGatewayNetworkInfo.defgw,sizeof(p_rGatewayNetworkInfo.defgw) );
	p_rGatewayNetworkInfo.mac0[0] = p_rGatewayNetworkInfo.mac1[0]=0;	/// compatibility with systems without MAC
	oP.GetVar( 0, "ETH0_MAC",	p_rGatewayNetworkInfo.mac0,	sizeof(p_rGatewayNetworkInfo.mac0)  );
	oP.GetVar( 0, "ETH1_MAC",	p_rGatewayNetworkInfo.mac1,	sizeof(p_rGatewayNetworkInfo.mac1)  );
	p_rGatewayNetworkInfo.bDhcpEnabled = access( DISABLE_DHCP_FLAG_FILE, R_OK );

	FILE * f;
	if( (f = fopen( RESOLV_CONF, "r")) )
	{
		flock( fileno(f), LOCK_EX );
		int i;
		for( i=0; (!feof( f )) && i<(int)sizeof(p_rGatewayNetworkInfo.nameservers) ; )
		{	// Match 15 in the format with DNS_SERVER_SIZE-1
			char tmp[512];
			if(fscanf( f, " nameserver %15[0-9.] ", tmp) == 1 )
			{
				p_rGatewayNetworkInfo.nameservers[i] = strdup(tmp);
				LOG("getGateway DNS[%d]=[%s]", i, p_rGatewayNetworkInfo.nameservers[i] );
				i++;
			}
			else
			{
				char* l(NULL);
				size_t s(0);
				ssize_t cr = getline(&l, &s, f) ; // ignore malformed lines
				if (l) free(l);
				if (cr == -1)
				{
					FPERR("getGateway: failed getline (%s)", RESOLV_CONF);
					break;
				}
			}
		}
		p_rGatewayNetworkInfo.nameservers[i]=NULL;
		flock( fileno(f), LOCK_UN );
		fclose(f);
	}	// open file errors are ok: report no DNS servers

	oP.Release() ;

	return true;
}

// you will see the same code in ElmShared.
// [Marcel] no longer true
bool CMiscImpl::setGatewayNetworkInfo( GatewayNetworkInfo& p_rGatewayNetworkInfo, int*status)
{
	CValidator isValid ;
	uint32_t mask32 = isValid.Mask( p_rGatewayNetworkInfo.mask );
	if ( !mask32 )
	{
		ERR("setGatewayNetworkInfo: Invalid mask [%s]", p_rGatewayNetworkInfo.mask);
		if (status) *status = NEVALIDATIONERROR ;
		return false ;
	}

	uint32_t ip32 = isValid.Ip( p_rGatewayNetworkInfo.ip, mask32 );
	if ( !ip32 )
	{
		ERR("setGatewayNetworkInfo: Invalid ip [%s]", p_rGatewayNetworkInfo.ip);
		if (status) *status = NEVALIDATIONERROR ;
		return false ;
	}

	if ( ! isValid.Gateway( p_rGatewayNetworkInfo.defgw, mask32, ip32))
	{
		ERR("setGatewayNetworkInfo: Invalid gateway [%s]", p_rGatewayNetworkInfo.defgw);
		if (status) *status = NEVALIDATIONERROR ;
		return false ;
	}

	mask32 = isValid.Mask( "255.255.255.0" );

	// try to use the ETH_NAME that already exists.
	CIniParser oP;
	char ethName[MAX_LINE_LEN]={0};
	char eth0MAC[MAX_LINE_LEN]={0};
	char eth1MAC[MAX_LINE_LEN]={0};
	if( p_rGatewayNetworkInfo.bUpdateMAC )
	{	strncpy( eth0MAC, p_rGatewayNetworkInfo.mac0, sizeof(eth0MAC) );/// user input takes precedence. it will override whatever was previously there
		strncpy( eth1MAC, p_rGatewayNetworkInfo.mac1, sizeof(eth1MAC) );/// user input takes precedence. it will override whatever was previously there
		eth0MAC[ sizeof(eth0MAC)-1 ] = eth1MAC[ sizeof(eth1MAC)-1 ] = 0;
	}

	if ( ! oP.Load(RC_NET_INFO) )
	{
		ERR("getGateway: unable to load "RC_NET_INFO" will use ETH_NAME=eth0");
	}
	else
	{
		oP.GetVar( 0, "ETH_NAME",	ethName,	sizeof(ethName) );
		if( !p_rGatewayNetworkInfo.bUpdateMAC )/// update not requested by user, read & use current value
		{	oP.GetVar( 0, "ETH0_MAC",	eth0MAC,	sizeof(eth0MAC) );
			oP.GetVar( 0, "ETH1_MAC",	eth1MAC,	sizeof(eth1MAC) );
		}
		oP.Release();
	}

	FILE*f=fopen( RC_NET_INFO, "w");
	if ( !f )
	{
		LOG_ERR("ERROR setGatewayNetworkInfo: fopen(" RC_NET_INFO ",w) failed");
		return false ;
	}
	flock( fileno(f), LOCK_EX );
	fputs("#AUTOMATICALLY GENERATED.\n", f );
	fprintf( f, "ETH0_IP=%s\n",   p_rGatewayNetworkInfo.ip ) ;
	fprintf( f, "ETH0_MASK=%s\n", p_rGatewayNetworkInfo.mask ) ;
	fprintf( f, "ETH0_GW=%s\n",   p_rGatewayNetworkInfo.defgw ) ;
	if ( 0==ethName[0] ) fprintf( f, "ETH_NAME=eth0\n") ;
	else fprintf( f, "ETH_NAME=%s\n", ethName ) ;

	/// user request update OR value previously set
	if( p_rGatewayNetworkInfo.bUpdateMAC || eth0MAC[0] )
	{
		fprintf( f, "ETH0_MAC=%s\n", eth0MAC ) ;
	}
	/// else (update not requested AND previous value inexistent): do not set at all, fallback to IP-based MAC

	if( p_rGatewayNetworkInfo.bUpdateMAC || eth1MAC[0] )
	{
		fprintf( f, "ETH1_MAC=%s\n", eth1MAC ) ;
	}
	/// else (update not requested AND previous value inexistent): do not set at all, fallback to IP-based MAC

	flock( fileno(f), LOCK_UN );
	fclose(f);

	bool bPreviouslyDisabled = !access( DISABLE_DHCP_FLAG_FILE, R_OK );

	if(p_rGatewayNetworkInfo.bDhcpEnabled && bPreviouslyDisabled )
	{	// delete disable flag file
		unlink( DISABLE_DHCP_FLAG_FILE );
	}
	if( !p_rGatewayNetworkInfo.bDhcpEnabled && !bPreviouslyDisabled )
	{	// create disable flag file
		int rv ;
		rv = open( DISABLE_DHCP_FLAG_FILE , O_CREAT, 0644 );
		if ( -1 != rv )
		{
			close( rv );
		}
	}

	if ( NULL == p_rGatewayNetworkInfo.nameservers[0] ) return true ;

	f=fopen( RESOLV_CONF, "w");
	if ( !f )
	{
		LOG_ERR("ERROR: setGatewayNetworkInfo: fopen(" RESOLV_CONF ") failed");
		return false ;
	}
	flock( fileno(f), LOCK_EX );
	for ( int i=0; p_rGatewayNetworkInfo.nameservers[i] != NULL;++i )
	{
		fprintf( f, "nameserver %s\n", p_rGatewayNetworkInfo.nameservers[i] );
	}
	flock( fileno(f), LOCK_UN );
	fclose(f);

	return true;
}

#define NTP_CONF "/etc/ntp.conf"
bool CMiscImpl::getNtpServers( std::vector<char*>& servers )
{
	char *dline=NULL, srv[512];
	size_t n;
	ssize_t rv ;
	FILE* df;
	df=fopen(NTP_CONF,"r");
	if ( !df ) return false ;
	while ( -1 != (rv=getline(&dline, &n, df)) )
	{
		rv=sscanf( dline, "server %511s", srv );
		if ( EOF==rv || rv < 1 ) continue ;
		LOG("ntp-server:[%s]", srv);
		servers.push_back(strdup(srv));
	}
	if (dline) free(dline);
	fclose(df);
	return true ;
}


/// Set the server lines to those inside servers vector.
/// leave the unknown lines intact
bool CMiscImpl::setNtpServers( const std::vector<const char*>& servers )
{
	char *dline=NULL, srv[512];
	FILE *df,*out;
	ssize_t rv;
	size_t n;
	df=fopen(NTP_CONF,"r");
	if ( !df ) return false ;

	out=fopen(NTP_CONF".halfway","w");
	if ( !out ) return false ;
	while ( -1 != (rv=getline(&dline, &n, df)) )
	{
		srv[0] = 0;
		rv=sscanf( dline, "server %511s", srv);
		if ( rv == EOF || rv <0 ) break ;
		if ( (rv == 0) || !strncmp(srv,"127.127.1.0", 11) )/// unknown lines OR server 127.127.1.0
		{
			fprintf( out, "%s",dline);
			continue ;
		}
	}
	for ( unsigned i=0; i<servers.size(); ++i)
	{
		if ( strncmp(servers[i],"127.127.1.0", 11) )
		{
			LOG("set:%s", servers[i]);
			fprintf( out, "server %s iburst\n",servers[i]);
		}
	}

	fclose(df);
	fclose(out);
	rename(NTP_CONF".halfway",NTP_CONF);
	return true ;
}

bool CMiscImpl::restartApp( const char * p_szAppName, const char * p_szAppParams )
{
	log2flash("CGI: restartApp[%s]", p_szAppName);

	/// TODO: ADD SOME MINIMAL PROTECTION: illegal chars: & ; and space
	/// This should not be implemented as restartApp, but rather as a wrapper
	/// for start.sh ApplicationName. This way we are not aware of the release
	/// specific application list nor it's permissions.
	//#warning MAKE/CONVINCE restartApp to use start.sh
	// systemf_to( 600, "exec 0>&- ; exec 1>&- ; exec 2>&- ; . /etc/profile; killall %s; sleep 1; killall -9 %s; usleep 500000; %s %s &", p_szAppName, p_szAppName, p_szAppName, p_szAppParams?p_szAppParams:"");

	// Splits restart in stop and start. This is necessary for starting the process if it is already stopped.
	try
	{
		systemf_to( 600, "exec 0>&- ; exec 1>&- ; exec 2>&- ; . /etc/profile; killall %s; sleep 1; killall -9 %s; usleep 500000", p_szAppName, p_szAppName);
	}
	catch (...)	{ }
	systemf_to( 600, "%s %s &", p_szAppName, (p_szAppParams ? p_szAppParams : ""));
	return true;
}

void detach()
{
	// HTTP server won't exit until we close all stds
	// so close them as fast as possible
	close(0) ;
	close(1) ;
	close(2) ;

	// I haven't found out why ( renice -n 0 $$ ; ... ) was not enough to
	// bring the processes to normal priority, so I'm using setpriority
	setpriority(PRIO_PGRP,0,0);
	setsid() ;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief Restart all applications using start.sh
/// @retval false fork failed.
/// @retval true  only the parent returns true; the child stays alive to run start.sh
////////////////////////////////////////////////////////////////////////////////
bool CMiscImpl::softwareReset( )
{
	log2flash("CGI: %s", __FUNCTION__ );

	pid_t pid = vfork() ;
	if ( pid < 0 ) return false ;

	if ( pid > 0 ) return true ;

	if ( pid == 0 )
	{
		detach() ;
		// wait for the parent to return the response
		// then it's safe to kill the http server(from start.sh)
		sleep(2) ;

//#warning ALWAYS SOURCE /etc/profile BEFORE RUNNING FIRMWARE SCRIPTS
		return execlp("/bin/sh", "sh", "-c", ". /etc/profile && "NIVIS_FIRMWARE"start.sh >/dev/null 2>&1 && "NIVIS_FIRMWARE"log2flash 'CGI: softwareReset done'", NULL );

		//return execlp("/bin/sh", "sh", "-c", ". /etc/profile && "NIVIS_FIRMWARE"web_start.sh 5 && "NIVIS_FIRMWARE"log2flash 'CGI: softwareReset done'", NULL );
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief Reboot the device
/// @retval false fork failed.
/// @retval true  only the parent returns true; the child stays alive to run stop.sh
////////////////////////////////////////////////////////////////////////////////
bool CMiscImpl::hardReset( )
{
	log2flash("CGI: %s", __FUNCTION__ );

	pid_t pid = vfork() ;
	if ( pid < 0 ) return false ;

	if ( pid > 0 ) return true ;

	if ( pid == 0 )
	{
		detach() ;
		// wait for the parent to return the response
		// then it's safe to kill the http server(from start.sh)
		sleep(2) ;

//		#warning ALWAYS SOURCE /etc/profile BEFORE RUNNING FIRMWARE SCRIPTS
		execlp("/bin/sh", "sh", "-c", ". /etc/profile && "NIVIS_FIRMWARE"stop.sh ; reboot", NULL );
	}

	return true;
}


////////////////////////////////////////////////////////////////////////////////
/// @brief   Apply configuration file changes (signal the module to reload configurations)
/// @param   p_szModule Module to signal
/// @param   p_szSignal Siganl to send. Allow only HUP, USR1, USR2 (default HUP)
/// @retval  false cannot signal the module
/// @retval  true  module signalled ok
/// @note    This opens yet another security hole, since the user can kill any
///          process he wishes.
/// @todo	Check the module names & send signal only to legal ones to close the security hole
////////////////////////////////////////////////////////////////////////////////
bool CMiscImpl::applyConfigChanges( const char * p_szModule, const char * p_szSignal )
{	/// send p_szSignal/HUP to specified module
	#define READ_BUF_SIZE 50

	DIR *dir;
	struct dirent *next;

	int nSignal = SIGHUP;
	if( !p_szSignal || !*p_szSignal || !strcasecmp(p_szSignal, "HUP") )
		nSignal = SIGHUP;
	else if( !strcasecmp(p_szSignal, "USR1") )
		nSignal = SIGUSR1;
	else if( !strcasecmp(p_szSignal, "USR2") )
		nSignal = SIGUSR2;
	else{
		LOG("WARNING applyConfigChanges: invalid signal") ;
		return false ;
	}
	if( p_szModule && strcmp(p_szModule, "MonitorHost") && (nSignal == SIGUSR1) )// send USR1 ONLY to MH for the time being
	{
		LOG("WARNING applyConfigChanges: only MH can take USR1") ;
		return false ;
	}

	if ( strcmp(p_szModule, "init")==0){
		LOG("Can't kill init") ;
		return false ;
	}

	dir = opendir("/proc");
	if (!dir){
		LOG("Cannot open /proc");
		return false ;
	}

	while ((next = readdir(dir)) != NULL)
	{
		FILE *status;
		char filename[READ_BUF_SIZE];
		char buffer[READ_BUF_SIZE];
		char name[READ_BUF_SIZE];

		/* Must skip ".." since that is outside /proc */
		if (strcmp(next->d_name, "..") == 0)
			continue;

		/* If it isn't a number, we don't want it */
		if (!isdigit(*next->d_name))
			continue;

		sprintf(filename, "/proc/%s/status", next->d_name);
		if (! (status = fopen(filename, "r")) ) {
			continue;
		}
		if (fgets(buffer, READ_BUF_SIZE-1, status) == NULL) {
			fclose(status);
			continue;
		}
		fclose(status);

		/* Buffer should contain a string like "Name:   binary_name" */
		sscanf(buffer, "%*s %s", name);
		if (strcmp(name, p_szModule) == 0) {
			kill(strtol(next->d_name, NULL,0), nSignal );
			return true ;
		}
	}
	WARN("Process %s not found", p_szModule );
	return false ;
}

// ER Protocol (Discovery) {

const unsigned short g_au16_ERP_CRCTable[ERPROTO_CRC_TABLE_LEN]={
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
	0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
	0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
	0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
	0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
	0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
	0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
	0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
	0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
	0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
	0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
	0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
	0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
	0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
	0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
	0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
	0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
	0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
	0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
	0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
	0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
	0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
	0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
	0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
	0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
	0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
	0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
	0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
	0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
	0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
	0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
	0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};

CMiscImpl::ERProtoPkHeader::ERProtoPkHeader() : m_u8_Version(ERPROTO_MSG_VER), m_u8_Type(ERPROTO_MSG_REQ) {}


CMiscImpl::ERProtoReqPk::ERProtoReqPk() : m_oHdr() {
	m_u16_CRC = ERProtoCRC((unsigned char*)this, sizeof (ERProtoReqPk) - ERPROTO_CRC_LEN);
}

void CMiscImpl::ERProtoRspPk::NTOH(){
	m_u32_ANID = ntohl(m_u32_ANID);
	// other members are not used for now
}

unsigned short CMiscImpl::ERProtoCRC(unsigned char * p_puchBuff, unsigned int p_nLen) {
	unsigned short nAcumulator = 0xFFFF;
	for (unsigned int i = 0; i < p_nLen; ++i) {
		nAcumulator = (nAcumulator << 8) ^ g_au16_ERP_CRCTable[(nAcumulator >> 8) ^ p_puchBuff[i]];
	}
	return htons(nAcumulator);
}

// ER Protocol (Discovery) }

bool CMiscImpl::discoverRouters( std::vector<DiscoveryInfo>& routerList )
{
	CUdpSocket oSocket;
	// search timer
	CSimpleTimer oSearchTimer;
	static ERProtoReqPk stERProtoReqPk;
	// buffer used on socket
	static unsigned char st_auchBuff[ERPROTO_BUFF_LEN];
	size_t nBuffLen;
	// dest ip (int&presentation form), port
	static char szSrcIp[INET_ADDRSTRLEN];
	unsigned int unSrcIp;
	int nSrcPort;

	// init socket
	if (!oSocket.Create()){
		NLOG_ERR("[discoverRouters] Socket.Create");
		return false;
	}
	if (!oSocket.Bind(ERPROTO_PORT)){
		NLOG_ERR("[discoverRouters] Cannot Bind on port %d. Is Discovery running on local machine?", ERPROTO_PORT);
		return false;
	}
	if (!oSocket.AllowBroadcast()){
		NLOG_ERR("[discoverRouters] Socket.AllowBroadcast");
		return false;
	}

	// send broadcast request
	oSocket.SendTo(ERPROTO_BROADCAST_ADDR, ERPROTO_PORT, (unsigned char*) &stERProtoReqPk, sizeof stERProtoReqPk);
	NLOG_INFO("[discoverRouters] Broadcast msg dump[%s]", GetHex((unsigned char*) &stERProtoReqPk, sizeof stERProtoReqPk));

	// arm seach timer
	oSearchTimer.SetTimer(ERPROTO_SEARCH_TIMEOUT);

	while (!oSearchTimer.IsSignaling()) {
		if (oSocket.CheckRecv()) {

			oSearchTimer.SetTimer(ERPROTO_SEARCH_TIMEOUT);
			// init bufferlen
			nBuffLen = ERPROTO_BUFF_LEN;
			if (!oSocket.RecvFrom(st_auchBuff, &nBuffLen, 0, &unSrcIp, &nSrcPort)) {
				continue;
			}
			// get the presentation form of src ip
			strcpy(szSrcIp, inet_ntoa(*(struct in_addr*)(&unSrcIp)));
			// verify header length
			if (nBuffLen < sizeof (ERProtoPkHeader)) {
				NLOG_ERR("[discoverRouters] Recv msg from[%s:%hd], hdr size recv %d < exp: %d ", szSrcIp, nSrcPort, nBuffLen, sizeof (ERProtoPkHeader));
				NLOG_ERR("[discoverRouters] Recv msg dump[%s]", GetHex(st_auchBuff, nBuffLen));
				continue;
			}
			// verify version and message type from header
			ERProtoRspPk* oERProtoRspPk = (ERProtoRspPk*)st_auchBuff;
			if (oERProtoRspPk->m_oHdr.m_u8_Version != ERPROTO_MSG_VER || oERProtoRspPk->m_oHdr.m_u8_Type != ERPROTO_MSG_RSP) {
				NLOG_WARN("[discoverRouters] Recv msg from[%s:%hd], Version/Type exp: %d/%d != recv %d/%d, skipping (may be the echo message)",
						  szSrcIp, nSrcPort, ERPROTO_MSG_VER, ERPROTO_MSG_RSP, oERProtoRspPk->m_oHdr.m_u8_Version, oERProtoRspPk->m_oHdr.m_u8_Type);
				NLOG_WARN("[discoverRouters] Recv msg dump[%s]", GetHex(st_auchBuff, nBuffLen));
				continue;
			}
			// verify size of packet (type RSP)
			if (nBuffLen != sizeof (ERProtoRspPk)) {
				NLOG_ERR("[discoverRouters] Recv msg from[%s:%hd], size recv %d != exp: %d ", szSrcIp, nSrcPort, nBuffLen, sizeof (ERProtoRspPk));
				NLOG_ERR("[discoverRouters] Recv msg dump[%s]", GetHex(st_auchBuff, nBuffLen));
				continue;
			}
			// verify crc
			unsigned short expCrc = *(unsigned short *) (st_auchBuff + nBuffLen - ERPROTO_CRC_LEN	);
			unsigned short rcvCrc = ERProtoCRC(st_auchBuff, nBuffLen - ERPROTO_CRC_LEN	);
			if (rcvCrc != expCrc) {
				NLOG_ERR("[discoverRouters] Recv msg from[%s:%hd], CRC exp: %d != recv %d", szSrcIp, nSrcPort, expCrc, rcvCrc);
				NLOG_ERR("[discoverRouters] Recv msg dump[%s]", GetHex(st_auchBuff, nBuffLen));
				continue;
			}

			oERProtoRspPk->NTOH();

			DiscoveryInfo oDiscoveryInfo(
				oERProtoRspPk->m_u32_ANID,
				oERProtoRspPk->m_u32_IP,
				oERProtoRspPk->m_u32_DHCPIP);

			NLOG_INFO("[discoverRouters] Found vr[an: %s, staticIP: %s, dynamicIP: %s]",
				oDiscoveryInfo.m_szAnID,
				oDiscoveryInfo.m_szStaticIP,
				oDiscoveryInfo.m_szDynamicIP);

			routerList.push_back(oDiscoveryInfo);
		}
	}
	return true;
}
