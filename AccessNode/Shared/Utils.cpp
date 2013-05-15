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
	created:	2004/10/04
	created:	4:10:2004   15:20
	filename: 	C:\home\AccessNode\Shared\Utils.cpp
	file path:	C:\home\AccessNode\Shared
	file base:	Utils
	file ext:	cpp
	author:		Claudiu Hobeanu

	purpose:
*********************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/vfs.h>

#ifndef CYG
#include <sys/sysinfo.h>
#endif

#include "Utils.h"
#include "Common.h"
#include "Socket.h"

// we don't need to access the pidfiles at every function call, only once every PIDFILES_TIMEOUT seconds
#define PIDFILES_TIMEOUT 60
// only delete the pidfiles once every PIDFILES_TIMEOUT * PIDFILES_FACTOR seconds.
#define PIDFILES_FACTOR 15


int GetBinData( const char* p_szInput, char* p_pData, int* p_pLen )
{
    switch ( p_szInput[0] )
    {
    case 's' :
    case 'S':
		*p_pLen = sprintf(p_pData, p_szInput+2) + 1;
        //strcpy( p_pData, p_szInput + 2 );
        //LOG("Sz data: %s", p_pData );
        //*p_pLen = strlen(p_pData)+1;
        break;
    case 'x':
    case 'X':
        {
            *p_pLen = Hex2Bin( p_szInput+2, p_pData );
            break;
        }
    }

    return 1;
}

// returns no of Kbytes of free memory of a type (types: "MemFree:", "Buffers:", "Cached:" )
// 0 if nothing found
int	GetFreeMemFor( const char* p_szMemInfo, const char* pNameType )
{
	const char *p = strstr(p_szMemInfo, pNameType) ;
	if( !p )
	{	return 0;
	}

	p += strlen(pNameType);

	return atoi(p);
}


int FileRead(const char* p_szFile, char* p_pBuff, int p_nBuffLen )
{
	int fd = open( p_szFile,  O_RDONLY);

	if (fd < 0)
	{	LOG_ERR( "FileRead: error at opening file %s",  p_szFile);
		return 0;
	}


	int nLen = read(fd, p_pBuff, p_nBuffLen);

	if (nLen<=0)
	{
		LOG_ERR( "FileRead: error at reading from file %s",  p_szFile);
	}
	close(fd);
	return nLen;
}

//return sys uptime in seconds
//	on error return <0
float SysGetUpTime()
{
	float dTime;

	char szData[128];

	int nLen = FileRead("/proc/uptime", szData, sizeof(szData) - 1);

	if (nLen<=0)
	{	return -1;
	}

	szData[nLen] = 0;

	if (sscanf(szData, "%f",&dTime) <= 0)
	{
		return -1;
	}

	return dTime;
}

// returns no of bytes of free memory  conform to sysinfo call
// return 0 if error occured
//we need /proc/meminfo for cached, which is not reported by sysinfo
int GetSysFreeMem( void )
{
#ifdef CYG
	return 0;
#else

	char szData[1024];

	int nLen = FileRead("/proc/meminfo", szData, sizeof(szData) - 1);

	if (nLen<=0)
	{	return -1;
	}
	szData[nLen] = 0;

	struct sysinfo inf;
	if ( sysinfo( &inf) )
	{
		LOG_ERR("ERROR GetSysFreeMem");
		inf.freeram=0;
	}

	struct statfs buf;
	long long nTmpUsed = 0;
	if( statfs( "/tmp", &buf)) {
		LOG_ERR( "GetSysFreeMem: statfs( \"tmp\") failed");
	}else{
		nTmpUsed= (buf.f_blocks-buf.f_bavail) * (long long) buf.f_bsize;
	}

	return inf.freeram * inf.mem_unit +
		( GetFreeMemFor( szData, "Buffers:") + GetFreeMemFor( szData, "Cached:")) * 1024 - nTmpUsed;

#endif
}

//return free memory, expressed in bytes
int GetSysFreeMemK( void )
{
	return GetSysFreeMem()/1024;
}

//return true if a memory low condition is detected
//TAKE CARE: this detection is faulty. Use this only as a warning
//the requested memory is expressed in kB
bool IsSysMemoryLow( int p_nKBRequested )
{
	if( GetSysFreeMemK() < (p_nKBRequested + FREE_MEM_MIN_REQ))
	{
		LOG("ERROR MEMORY LOW, possible failure (%d < %d + %d kB)",
			GetSysFreeMemK(), p_nKBRequested, FREE_MEM_MIN_REQ );
		return true;
	}

	return false;
}

//convert hex string in binary data
//suppose that p_pOutput has allocated (strlen(p_szInput)+2)/2
//return the len of the bin buffer
int Hex2Bin( const char* p_szInput, char* p_pOutput )
{
	return Hex2Bin(p_szInput, strlen(p_szInput), p_pOutput );
}

int Hex2Bin( const char* p_pInput, int n, char* p_pOutput )
{
	int i,j =0;

	for( i = 0; i < n; j++ )
	{	
		while ( i < n && ( isspace(p_pInput[i]) || p_pInput[i] == ':' || p_pInput[i] == '-') )
		{
			i++;
		}

		if (i>=n)
		{
			break;
		}
		
		int nCrt = 0;
		int nValue;
		if (sscanf( p_pInput +i,"%2x%n", &nValue, &nCrt ) < 1 )
		{	break;
		}
		p_pOutput[j] = (char)nValue;
		i += nCrt;

	}
	return j;
}

//////////////////////////////////////////////////////////////////////////////////
// Description : compute no. of p_cDelim char in p_pData in first p_nLen bytes
//				or when it was encounter '\0'
// Parameters  :
//				const char *  p_pData      	-  	input   the data block
//				int			p_nLen			-	input
//				char		p_cDelim		- 	input	char to look for
// Return      :
//				return no. of chars p_cDelim
//////////////////////////////////////////////////////////////////////////////////
int NoOfDelim( const char* p_pData, const int p_nLen, const char p_cDelim )
{
	if( !p_pData )
		return -1;

	int nCount = 0;
	for( int i =0; i < p_nLen && p_pData[i]; i++ )
	{	if( p_pData[i] == p_cDelim )
			nCount++;
	}
	return nCount;
}


//////////////////////////////////////////////////////////////////////////////////
// Description : compute the position of first p_cDelim or of end of data in p_pData in first p_nLen bytes
//				or when it was encounter '\0'
// Parameters  :
//				const char * 	 p_pData      	-  	input   the data block
//				const int		p_nLen			-	input
//				const char		p_cDelim		- 	input	char to look for
// Return      :
//				return no. of chars p_cDelim
//////////////////////////////////////////////////////////////////////////////////
int NextToken( const char* p_pData, const int p_nLen, const char p_cDelim )
{
	if( !p_pData )
		return -1;

	int nPos = 0;

	for( int i = 0; i < p_nLen && p_pData[i]; i++ )
	{
		if( p_pData[i] == p_cDelim )
			return nPos;

		nPos++;
	}
	return nPos;
}


//////////////////////////////////////////////////////////////////////////////////
// Description : replace in string all space characters(' ','\n','\r','\f','\t','\v') with p_cChar character
// Parameters  :
//				char *  p_pString		      	-  input,output   string wich will be modified
//				const char p_cChar					-  char that will be used for replace
// Return      :
//				none
//////////////////////////////////////////////////////////////////////////////////
void ReplaceSpaces( char* p_pString, const char /*p_cChar*/ )
{
	while( *p_pString )
	{	if( isspace(*p_pString) )
			*p_pString = ' ';
		p_pString++;
	}
}






// copy src file to dest file in p_nStep bytes chunks
// always creats dest file ( needed in saving mesh net)
int Copy( const char* p_szSrc, const char* p_szDest, int p_nStep /*= 4096*/ )
{
	int nFdSrc, nFdDest;

	nFdDest = open( p_szDest, O_CREAT | O_WRONLY | O_TRUNC,  0666 );
	if( nFdDest < 0 )
	{	LOG_ERR( "Copy: can't creat file %s", p_szDest );
		return 0;
	}

	nFdSrc = open( p_szSrc,  O_RDONLY);
	if( nFdSrc < 0 )
	{	LOG_ERR( "Copy: can't open file %s", p_szSrc );
		close(nFdDest);
		return 0;
	}

	char* pBuff = new char[p_nStep]; //throws exceptions if error
	int nCount = 0;

	for(;;)
	{	nCount = read( nFdSrc, pBuff, p_nStep );

		if ( nCount < 0 )
		{	LOG_ERR( "Copy: can't read from file %s", p_szSrc );
			break;
		}

		if (nCount == 0)
		{	break;
		}

		nCount = write( nFdDest, pBuff, nCount );
		if ( nCount < 0 )
		{	LOG_ERR( "Copy: can't write to file %s", p_szDest );
			break;
		}
	}

	delete []pBuff;
	close(nFdDest);
	close(nFdSrc);
	return nCount >= 0;
}


//computes CRC for only one byte, based on the previous one.
unsigned short crc( unsigned short addedByte,
                                  unsigned short genpoli,
                                  unsigned short current )
{
    register int i;

    addedByte <<= 8;
    for (i=8; i>0; i--){
        if ((addedByte^current) & 0x8000)
            current = (current << 1) ^ genpoli;
        else
            current <<= 1;
        addedByte <<= 1;
    }
    return current;
}

//'escapes' special chars: STX, ETX, CHX which might appear inside data
//ie, send a CHX and then 0xFF - char instead of char
//in case of normal char, send it unchanged
void EscapeChar(  const u_char * p_psPacket, u_int* p_pnIndexPacket,
                  u_char       * p_psBuffer, u_int* p_pnIndexBuffer )
{
    switch( p_psPacket[*p_pnIndexPacket] ) {
        case STX:
        case ETX:
        case CHX:
            p_psBuffer[ (*p_pnIndexBuffer)++ ] = CHX;
            p_psBuffer[ (*p_pnIndexBuffer)++ ] = 0xFF - p_psPacket[ (*p_pnIndexPacket)++ ];
            break;
        default :
            p_psBuffer[ (*p_pnIndexBuffer)++ ] = p_psPacket[ (*p_pnIndexPacket)++ ];
    }
}



//computes CRC for packet and return the result in crc field
int ComputeCRC( const u_char * p_pPacket, u_int p_nPacketLength, u_char p_pCrc[CRC_LENGTH], unsigned short p_usCrcStart )
{
    unsigned int i;
    unsigned short current = p_usCrcStart;
    unsigned short *pCrc = (unsigned short*)p_pCrc;

    for( i = 0; i < p_nPacketLength; i++ ){
        current = crc( (unsigned short)p_pPacket[i], CRCCCITT, current );
    }
    *pCrc = htons(current);
    for( i=0; i<2; i++ )
        switch( p_pCrc[ i ] ) {
            case STX:
            case ETX:
            case CHX:
                p_pCrc[ i ] = 0xFF - p_pCrc[ i ];
             //   LOG( "computeCRC: special char %x Substracted from 0xFF", p_pCrc[ i ]);
        }
    return 1;
}


int EscapePacket(u_char *p_pSrc, int p_nSrcLen, u_char* p_pDest, int* p_pDestLen )
{
	if (p_nSrcLen > *p_pDestLen - 2 )
	{	LOG( "CNivisPacket::EscapePacket: dest buffer too small" );
		return 0;
	}
	int i = 0, j = 1;

    while( i < p_nSrcLen )
    {   EscapeChar(p_pSrc, (unsigned int*)&i, p_pDest, (unsigned int*)&j );
		if (j >= *p_pDestLen)
		{	return 0;
		}
    }

	if ( *p_pDestLen <= j )
	{	LOG( "CNivisPacket::EscapePacket: dest buffer too small" );
		return 0;
	}

	p_pDest[0] = STX;
	p_pDest[j++] = ETX;
	*p_pDestLen = j;

	return 1;
}

int UnescapePacket(u_char *p_pBuff, int *p_pLen)
{
	int i = 1, j = 0;

	for(; i < *p_pLen; i++ )
	{
		if (p_pBuff[i] == ETX)
		{	break;
		}
		switch(p_pBuff[i])
		{
		case STX:
			return 0;
		case CHX:
			p_pBuff[j++] = 0xff - p_pBuff[++i];
			break;
		default:
			p_pBuff[j++] = p_pBuff[i];
		}
	}
	*p_pLen = j;
	return 1;
}



int ExtractBoundedPacket(u_char* p_pRcvBuffer, int* p_pCrtPosition, u_char *p_pBuff, int *p_pLen)
{
	if (!*p_pCrtPosition)
	{	return 0;
	}

    u_char *pStx, *pEtx;

    if( ( pStx = (unsigned char *) memchr( p_pRcvBuffer, STX, *p_pCrtPosition ) ) == NULL )
    {   // STX not found  -> discard all chars
		// log only if something else that 0 is recv
// 		int i;
// 		for( i = 0; i < *p_pCrtPosition && p_pRcvBuffer[i] == 0 ; i++)
// 			;
// 		if ( i < *p_pCrtPosition )
// 		{	//LOG( "ExtractBoundedPacket: STX not found -> discard all chars");
// 		}

        *p_pCrtPosition = 0;
        return 0;
    }

	pEtx = (u_char*)memchr( pStx, ETX, *p_pCrtPosition - (pStx - p_pRcvBuffer));

//	if ( !pEtx && *p_pCrtPosition < MAX_RS232_SIZE *3/4 )  //no etx and plenty of space in read buff, do nothing
//	{	return 0;
//	}

	//keep only the last packet
	pStx = pEtx ? pEtx : p_pRcvBuffer + *p_pCrtPosition -1;
	for( ; *pStx != STX; pStx-- ) ; //exists at least one STX

	int nResult = 0;

	if (pEtx)
	{	if (pEtx-pStx+1 <= *p_pLen)
		{	*p_pLen = pEtx-pStx+1;
			memcpy( p_pBuff, pStx, *p_pLen );
			nResult = 1;
		}
		pStx = pEtx + 1; //on this position is expected a STX
	}

	*p_pCrtPosition -= (pStx - p_pRcvBuffer);
	memmove( p_pRcvBuffer, pStx, *p_pCrtPosition );
	return nResult;
}

///////////////////////////////////////////////////////////////////////////////
// Name:	GetLoadAvg
// Author:	Mihai Buha (mihai.buha@nivis.com)
// Description:	uses sysinfo() to calculate the integer part of the 1-minute
//		load average
// Parameters:	none
// Returns:	the integer part of the first number in `cat /proc/loadavg`
///////////////////////////////////////////////////////////////////////////////
#define FSHIFT          16              /* nr of bits of precision */
#define FIXED_1         (1<<FSHIFT)     /* 1.0 as fixed-point */
#define LOAD_INT(x) ((x) >> FSHIFT)
#define LOAD_FRAC(x) LOAD_INT(((x) & (FIXED_1-1)) * 100)
int GetLoadAvg( void )
{
#ifndef CYG
	struct sysinfo info;
	info.loads[0] = 0;
	sysinfo(&info);
	return LOAD_INT( info.loads[0] );
#else
	return 0;
#endif
}

///////////////////////////////////////////////////////////////////////////////
// Name:	TouchPidFile
// Author:	Mihai Buha (mihai.buha@nivis.com)
// Description:	creates a file containing the PID
// Parameters:	p_szName - input - name of the file (ex: modulename.pid)
// Returns:	true if successful, false if failed
///////////////////////////////////////////////////////////////////////////////
bool TouchPidFile( const char* p_szName )
{
	static clock_t last_checked;
	static long n_SC_CLK_TCK;
	clock_t timestamp;
	int nFd;
	static char pid[6];
	bool status = true;
	if(!n_SC_CLK_TCK) n_SC_CLK_TCK = sysconf( _SC_CLK_TCK);

	timestamp = GetClockTicks();
	if( timestamp < last_checked){ // large uptime overflowed the clock_t
		last_checked = timestamp;
	}
	if( timestamp - last_checked < PIDFILES_TIMEOUT * n_SC_CLK_TCK)
	{	return status;
	}
	last_checked = timestamp;

	nFd = open( p_szName, O_CREAT | O_RDWR,  0666 );
	if( nFd < 0 )
	{	LOG_ERR( "TouchPidFile: can't create pidfile %s", p_szName );
		return false;
	}

	if(!pid[0])
	{	snprintf( pid, 5, "%d", getpid() );
	}

// 	if( lseek( nFd, 0, SEEK_SET ) < 0)
// 	{	LOG_ERR( "TouchPidFile: can't write to pidfile %s", p_szName );
// 		status = false;
// 	}

	if( write( nFd, pid, strlen( pid) ) < 0)
	{	LOG_ERR( "TouchPidFile: can't write to pidfile %s", p_szName );
		status = false;
	}

	if( close( nFd))
	{	LOG_ERR( "TouchPidFile: can't close pidfile %s", p_szName );
		status = false;
	}
	return status;
}

///////////////////////////////////////////////////////////////////////////////
// Name:	CheckDelPidFiles
// Author:	Mihai Buha (mihai.buha@nivis.com)
// Description:	removes all the pidfiles specified, but not too frequently
// Parameters:	p_pszNames - input - null-terminated array of filenames
//		p_szMissNames - output - if not null, fill with names of
//			missing files. Take care to allocate enough memory for
//			"Watchdog: <all filenames in p_pszNames> " or else bad
//			things will happen.
// Returns:	true if all pidfiles existed or timeout not expired
//		false if some pidfile was missing (controlling app was dead -
//			did not create a file during latest 2 intervals)
///////////////////////////////////////////////////////////////////////////////
bool CheckDelPidFiles (const char** p_pszNames, char* p_szMissNames)
{
	static clock_t last_checked;
	static long n_SC_CLK_TCK;
	static bool last_existed = true;
	clock_t timestamp;
	bool status = true;
	bool exists = true;
	if(!n_SC_CLK_TCK) n_SC_CLK_TCK = sysconf( _SC_CLK_TCK);

	timestamp = GetClockTicks();
	if( timestamp < last_checked){ // large uptime overflowed the clock_t
		last_checked = timestamp;
	}
	if( timestamp - last_checked < PIDFILES_FACTOR * PIDFILES_TIMEOUT * n_SC_CLK_TCK){
		return status;
	}
	last_checked = timestamp;
	int i;
	for( i=0; p_pszNames[i]; ++i)
	{
		int nFileLen = GetFileLen(p_pszNames[i]);
		if ( nFileLen > 0)
		{	unlink( p_pszNames[i]);
			continue;
		}
		if (nFileLen == 0)
		{	LOG("CheckDelPidFiles: file %s len==0",  p_pszNames[i]);
			unlink( p_pszNames[i]);
		}

		LOG_ERR( "CheckDelPidFiles: pidfile %s missing!", p_pszNames[i]);
		if( exists && p_szMissNames){
			sprintf( p_szMissNames, "Watchdog: ");
		}
		exists = false;
		if( p_szMissNames){
			strcat( p_szMissNames, p_pszNames[i]);
			strcat( p_szMissNames, " ");
		}
		system_to( 60, NIVIS_TMP"take_system_snapshot.sh "ACTIVITY_DATA_PATH"snapshot_warning.txt &");

	}
	status = exists || last_existed;
	last_existed = exists;
	return status;
}

///////////////////////////////////////////////////////////////////////////////
// Name:	GetProxyString
// Description:	creates a string containig ProxyIP ProxyPort
// Parameters:	p_oProxy - input - proxy which will be converted to string
// Returns:	-string containing the proxy
///////////////////////////////////////////////////////////////////////////////
const char* GetProxyString(net_address p_oProxy)
{
	static char szProxyString[50];
	in_addr s;

	s.s_addr = p_oProxy.m_nIP;
	sprintf(szProxyString, "%s:%d",inet_ntoa(s), ntohs(p_oProxy.m_dwPortNo) );

	return szProxyString;

}


////////////////////////////////////////////////////////////////////////////////
// Name:        SetCloseOnExec
// Author:		Claudiu Hobeanu
// Description: wrapper for setting the FD_CLOEXEC flag on a file descriptor
// Parameters:  int fd  -- file descriptor
//
// Return:      success/error
////////////////////////////////////////////////////////////////////////////////
int SetCloseOnExec( int fd )
{
	int flags;

	flags = fcntl(fd, F_GETFD);
	if (flags == -1)
	{
		LOG_ERR("SetCloseOnExec: fcntl(F_GETFD)");
		return 0;
	}

	flags |= FD_CLOEXEC;
	if (fcntl(fd, F_SETFD, flags) == -1)
	{
		LOG_ERR("SetCloseOnExec: fcntl(F_SETFD)");
		return 0;
	}

	return 1;
}


////////////////////////////////////////////////////////////////////////////////
// Name:        WriteToFile
// Author:		Claudiu Hobeanu
// Description:
// Parameters:
//
// Return:      success/error
////////////////////////////////////////////////////////////////////////////////
int WriteToFileAtOffset( const char* p_szFile, int p_nOffset, char* p_pBuff, int p_nLen )
{
	int flags = O_RDWR;

	//if writing at start or end creat file not exist
	if (p_nOffset <= 0)
	{	flags |= O_CREAT;
	}

	int fd = open( p_szFile, flags );
	if (fd<0)
	{	LOG_ERR("WriteToFile: opening file %s", p_szFile );
		return 0;
	}

	if (p_nOffset<0)
	{	lseek( fd, 0, SEEK_END );
	}
	else
	{	lseek(fd,p_nOffset,SEEK_SET);
	}

	int ret =  write( fd, p_pBuff, p_nLen );

	if (ret <0)
	{	LOG_ERR("WriteToFile: writing in file %s", p_szFile );
	}
	close(fd);

	return ret >= 0;
}


time_t GetLastModificationOfFile(const char* p_szFile)
{
	struct stat buf;

	if (stat(p_szFile,&buf))
	{
		//LOG_ERR("GetLastModificationOfFile");
		return -1;
	}

	return buf.st_mtime;
}










//assume that p_szIpv4 has at least 13 bytes reserved
bool AdjustIPv6 (TIPv6Address* p_pIpv6, const char* p_szIpv4, int p_nPort)
{
	unsigned int nIPv4 = INADDR_NONE;

	if ( strcmp(p_szIpv4,"0.0.0.0") == 0)
	{
		nIPv4 = CWrapSocket::GetLocalIp();				
	}
	else
	{
		nIPv4 = inet_addr(p_szIpv4);
	}

	if (nIPv4 == INADDR_NONE)
	{
		LOG("AdjustIPv6: ERROR IPv4 <%s> %d invalid", p_szIpv4, nIPv4  );
		return false;
	}

	memcpy( p_pIpv6->m_pu8RawAddress + 12, &nIPv4, sizeof(nIPv4));

	unsigned short usPort = htons((unsigned short)p_nPort);
	memcpy(p_pIpv6->m_pu8RawAddress + 10, &usPort, sizeof(usPort));
	return true;
}
