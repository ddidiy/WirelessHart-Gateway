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

 /***************************************************************************
                          Common.cpp  -  description
                             -------------------
    begin                : Fri Apr 12 2002
    email                : marcel.ionescu@nivis.com
 ***************************************************************************/


#include <stdarg.h>
#include <stdio.h>

#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <sys/timeb.h>
#include <stdlib.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <fcntl.h>

#include <sys/time.h>
#include <sys/statfs.h>


#include "Common.h"
//#include "structures.h"

// CIniParser needed by ExportVariable functions
#include "IniParser.h"
//#include "ANEvent.h"



CLog			g_stLog;
NLOG_LVL		g_nNLogLevel = NLOG_LVL_INF;

/// @brief The callback linking library log with GW log
extern "C" void CallbackLog( const char* p_szMsg )
{
	LOG_STR( p_szMsg );
}

/// @brief The callback linking library log with GW log
/// @note just ignore first parameter LOGLVL p_eLogLvl
void CallbackLogWithLevel(  LOGLVL /*p_eLogLvl*/, const char* p_szMsg )
{
	LOG_STR( p_szMsg );
}



int TrackPointTimeout (int p_nDuration, const char *p_szFile, const char* p_szFunc, int p_nLine)
{
	static clock_t	s_nLastTimedLog = GetClockTicks();

	int ret = (int)(GetClockTicks() - s_nLastTimedLog) > p_nDuration;

	if (ret)
	{
		LOG("TRACK: %s; %s; line=%d timeout=%d, crt=%d, last=%d",
					p_szFile, p_szFunc, p_nLine,
					GetClockTicks() - s_nLastTimedLog, GetClockTicks(), s_nLastTimedLog );
	}

	s_nLastTimedLog = GetClockTicks();

	return ret;
}



NLOG_LVL NLogLevelGetType (const char*  p_szLogLevel)
{
	if (strcasecmp(p_szLogLevel,"ERR") == 0)
	{
		return NLOG_LVL_ERR;
	}
	else if (strcasecmp(p_szLogLevel,"ERROR") == 0)
	{
		return NLOG_LVL_ERR;
	}
	else if (strcasecmp(p_szLogLevel,"DEBUG") == 0)
	{
		return NLOG_LVL_DBG;
	}
	else if (strcasecmp(p_szLogLevel,"DBG") == 0)
	{
		return NLOG_LVL_DBG;
	}
	else if (strcasecmp(p_szLogLevel,"WARN") == 0)
	{
		return NLOG_LVL_WARN;
	}
	else if (strcasecmp(p_szLogLevel,"INFO") == 0)
	{
		return NLOG_LVL_INF;
	}
	else if (strcasecmp(p_szLogLevel,"INF") == 0)
	{
		return NLOG_LVL_INF;
	}


	return NLOG_LVL_UNK;
}

const char* NLogLevelGetName (NLOG_LVL p_nLogLevel)
{
	switch(p_nLogLevel)
	{
		case NLOG_LVL_NONE:	return "NLOG_LVL_NONE";
		case NLOG_LVL_ERR:	return "NLOG_LVL_ERR";
		case NLOG_LVL_WARN:	return "NLOG_LVL_WARN";
		case NLOG_LVL_INF:	return "NLOG_LVL_INF";
		case NLOG_LVL_DBG:	return "NLOG_LVL_DBG";
		case NLOG_LVL_UNK:	return "NLOG_LVL_UNK";
	}

	return "NLOG_LVL_UNK";
}

/** parse <string> searching for the occurence of first string different than blank
 *  Maybe a better name is TrimLeft
 */
const char* JumpOverBlank(const char *p_szString)
{
	while( *p_szString && isspace(*p_szString) )
		p_szString++;
	return p_szString;
}

////////////////////////////////////////////////////////////////////////////////
// Name:        CheckAndFixWrapClockTicks
// Author:		Claudiu Hobeanu
// Description:	if the timeout is near upper edge and current time is near lower edge
//				then update timeout at current time
// Parameters:	p_nCrtTime				-- crt time get with GetClockTicks
//              clock_t *p_pnTimeout	-- a future timeout
// Return:      success/error
////////////////////////////////////////////////////////////////////////////////
int CheckAndFixWrapClockTicks( clock_t p_nCrtTime, clock_t *p_pnTimeout )
{
	if ((int)p_nCrtTime == -1)
	{	//error
		return 0;
	}

	if (*p_pnTimeout < 1000000000 )
	{	return 0;
	}

	if (p_nCrtTime > 10000 * 100)
	{	return 0;
	}


	*p_pnTimeout = p_nCrtTime;
	return 1;
}


TIPv6Address::TIPv6Address()
{
	memset(m_pu8RawAddress,0,sizeof(m_pu8RawAddress));
}

int TIPv6Address::FillIPv6 (char* p_szBuff, int p_nMaxSize) const
{
	if (p_nMaxSize < nIPv6PrintSize ) //
	{	return 0;
	}

	sprintf(p_szBuff,"%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X",
		m_pu8RawAddress[0], m_pu8RawAddress[1],m_pu8RawAddress[2],m_pu8RawAddress[3],
		m_pu8RawAddress[4], m_pu8RawAddress[5],m_pu8RawAddress[6],m_pu8RawAddress[7],
		m_pu8RawAddress[8],m_pu8RawAddress[9],m_pu8RawAddress[10],m_pu8RawAddress[11],
		m_pu8RawAddress[12],m_pu8RawAddress[13],m_pu8RawAddress[14],m_pu8RawAddress[15]
	);

	return nIPv6PrintSize;
}

const char* TIPv6Address::GetPrintIPv6()
{
	static char szBuff[nIPv6PrintSize];
	szBuff[0] = 0;
	FillIPv6(szBuff,nIPv6PrintSize);
	return szBuff;
}



int GetAbsDiffFromCrtTime(  unsigned char year,
                    unsigned char month,
                    unsigned char day,
                    unsigned char hour,
                    unsigned char minute,
                    unsigned char second
                   )
{
    struct tm timeUpdate;

    timeUpdate.tm_sec = second;
    timeUpdate.tm_min = minute;
    timeUpdate.tm_hour = hour;

    timeUpdate.tm_mday = day;
    timeUpdate.tm_mon = month - 1;
    timeUpdate.tm_year = year + 100;

    time_t tNewTime = ::mktime( &timeUpdate );
    if( ( time_t )-1  == tNewTime )
    {   LOG_ERR( "error mktime" );
        return -1;
    }

    time_t tCrtTime = time( (time_t*) NULL );
    if( -1 == tCrtTime )
    {
        LOG_ERR( "error time" );
        return -1;
    }

    return abs(tNewTime - tCrtTime);
}



#ifdef CYG
	int IGN_FCT1( ... )
	{	return 1;
	}
// 	const void * memmem( const void * p_pBuffer, unsigned int p_nBufferLen,
//                                 const void * p_pNeed,  unsigned int p_nNeedLen )
// 	{	return NULL;
// 	}
#endif

#if __GNUC__ < 3
const void * memmem( const void * p_pBuffer, unsigned int p_nBufferLen,
                                const void * p_pNeed,  unsigned int p_nNeedLen )
{
    if( p_nNeedLen )
    {
        while( p_nNeedLen <= p_nBufferLen )
        {
             if( *(char*)p_pBuffer == *(char*)p_pNeed )
             {
                 if( !memcmp( p_pBuffer, p_pNeed, p_nNeedLen ) )
                     return  p_pBuffer;
             }

             ((char*)p_pBuffer)++;
             p_nBufferLen --;
         }
     }

     return NULL;
}
#endif






int GetFileLen( int fd )
{
	int nCrtPos = lseek( fd, 0, SEEK_CUR );

	if( nCrtPos < 0 )
	{	//GetFileLen is called by CLog::truncLog() - loggin is forbitted here
		return -1;
	}

	int	nLen = lseek( fd, 0, SEEK_END );

	if( nLen < 0 )
	{	//GetFileLen is called by CLog::truncLog() - loggin is forbitted here
		return -1;
	}
	lseek( fd, nCrtPos, SEEK_SET);

	return nLen;
}

int GetFileLen( const char * p_szFile )
{
	int fd = open( p_szFile,  O_RDONLY);

	if ( fd < 0 )
	{	//GetFileLen is called by CLog::truncLog() - loggin is forbitted here
		return -1;
	}

	int nLen = GetFileLen( fd );
	close(fd);

	return nLen;
}

int FileExist (const char * p_szFile)
{
	struct stat buffer;
	return stat( p_szFile, &buffer ) == 0;
}

int WriteToFile( const char* p_szFile, const char* p_pBuff, bool p_nTrunc, int p_nPos )
{
	return WriteToFile( p_szFile, p_pBuff, strlen(p_pBuff), p_nTrunc, p_nPos );
}


int WriteToFile( const char* p_szFile, const char* p_pBuff, int p_nLen, bool p_nTrunc, int p_nPos )
{
	int flags = O_RDWR | O_CREAT;
	if (p_nTrunc)
	{	flags |= O_TRUNC;
		p_nPos = -1;
	}
	int fd = open( p_szFile, flags, 00666 );
	if (fd<0)
	{	LOG_ERR("WriteToFile: opening file %s", p_szFile );
		return 0;
	}

	if( p_nPos < 0)
		lseek( fd, 0, SEEK_END );
	else
		lseek( fd, p_nPos, SEEK_SET );

	int ret =  write( fd, p_pBuff, p_nLen );

	if (ret <0)
	{	LOG_ERR("WriteToFile: writing in file %s", p_szFile );
	}
	close(fd);

	return ret > 0;
}

int GetFileData( const char* p_pFileName, char *& p_pData, int& p_rLen )
{
	p_pData = NULL;
	p_rLen = 0;

	int fd = open( p_pFileName,  O_RDONLY);

	if ( fd < 0 )
	{	LOG_ERR( "GetFileData() : error at opening file %s", p_pFileName);
		return 0;
	}

	p_rLen = GetFileLen( fd );
	if ( p_rLen < 0)
	{	close(fd);
		return 0;
	}


	p_pData = new char[p_rLen+1];

	p_pData[p_rLen] = 0;

	if (!(p_pData))
	{	LOG_ERR( "GetFileData() : can't allocate %d bytes ", p_rLen );
		close(fd);
		return 0;
	}

	p_rLen = read(fd, p_pData, p_rLen );
 	close(fd);

	if ( p_rLen < 0  )
	{	delete[] p_pData;
		p_pData = NULL;
		return 0;
	}
	return 1;
}





static pid_t alrm_pid = 0;
/* CHANGE: remove static and call this from CApp::Close */
void alrm_handler(int)
{
	if ( ! alrm_pid ) return  ;
	LOG("timed out waiting for %d, let's fry the punk...", alrm_pid);

	/* kill the whole group */
	kill(-alrm_pid, SIGHUP);
	kill(-alrm_pid, SIGTERM);
	kill(-alrm_pid, SIGKILL);
}

int systemf_to(int timeout, const char* cmd, ...)
{
	char pBuf[1024];
	va_list lstArg;

	va_start(lstArg, cmd);
	unsigned nRet = vsnprintf(pBuf, sizeof(pBuf), cmd, lstArg) ;
	if(  nRet >= sizeof(pBuf) )
	{
		LOG( "ERROR systemf_to: string size %d too big", nRet);
		errno = ENOMEM;
		return -1;
	}

	pBuf[ sizeof(pBuf) - 1 ] = 0;//safety
	va_end(lstArg);

	return system_to(timeout, pBuf );
}

/* emulates the "system" function but with a timeout */
/* warning: messes with SIGALRM so active alarms will be delayed! */
int system_to(int timeout, const char *cmd)
{
    const char *const argv[] = { "/bin/sh", "-c", (char*)cmd, NULL};
    void (*old_handler)(int);
    unsigned int old_alrm;
    int status, res;

	LOG("system_to(%d): |%s| begin", timeout, cmd);
    if((alrm_pid =
		#ifndef USE_VFORK
		fork()
		#else
		vfork()
		#endif
	) < 0)
	{
		LOG_ERR(
		#ifndef USE_VFORK
		"fork failed"
		#else
		"vfork failed"
		#endif
		);
		return -1;
    }

    if(!alrm_pid)
	{	//child process
		// create a new process group so we can kill all offsprings
		setpgrp();
		#ifndef USE_VFORK
		g_stLog.Close("child close log");
		#endif

		//close all open files
		for( int i=3; (close(i) == 0) || (i < 50); i++ )
			;
		/* execute the cmd using /bin/sh */
		execv("/bin/sh", (char* const*)argv);
		// execv failed, but we have already closed the log.
		exit(127);
    }
	else
	{	//parent process
		//save current alarm settings
		old_handler = signal(SIGALRM, alrm_handler);
		old_alrm = alarm(timeout);

		// keep waiting if interrupted
		do{
		res = waitpid(alrm_pid, &status, 0);
		}while((res < 0) && (errno == EINTR));

		// restore the alarm stuff
		signal(SIGALRM, old_handler);
		alarm(old_alrm);

		// retrieve the exit code (if any)
		if(res == -1)
		{
			LOG_ERR("waitpid failed: %s", strerror(errno));
		}
		else
		{
			if(WIFEXITED(status))
					res = WEXITSTATUS(status);
			else
					res = -1;
		}
    }

	LOG("system_to(%d): |%s| ret %d", timeout, cmd, res);

	alrm_pid = 0 ;

    return res;
}

int connect_to(int sockfd, const struct sockaddr *serv_addr, socklen_t addrlen, int timeout)
{
    int flags, res;
    socklen_t len;
    struct timeval tv = { timeout, 0};
    fd_set wr_set;

    /* set the socket in non-blocking mode */
    flags = fcntl(sockfd, F_GETFL, &flags);
    if(fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0){
	LOG("fcntl F_SETFL call failed: %s", strerror(errno));
	return -1;
    }

    res = connect(sockfd, serv_addr, addrlen);
    if((res < 0) && (errno != EINPROGRESS)){
	LOG("connect call failed: %s", strerror(errno));
	return -1;
    }

    /* wait for the socket to connect */
    FD_ZERO(&wr_set);
    FD_SET(sockfd, &wr_set);
    res = select(sockfd + 1, NULL, &wr_set, NULL, &tv);

    /* check for errors */
    if(res < 0){
	LOG("select call failed: %s", strerror(errno));
	return -1;
    }else
    if(res == 0){
	LOG("timed out (%ds)", timeout);
	errno = ETIMEDOUT;
	return -1;
    }

    /* verify the connection status */
    len = sizeof(res);
    if(getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &res, &len) < 0){
	LOG("getsockopt call failed: %s", strerror(errno));
	return -1;
    }
    if(res){
	LOG("connect failed: %s", strerror(res));
	return -1;
    }

    /* restore the socket flags */
    fcntl(sockfd, F_SETFL, flags);

    return 0;
}

#define MAX_VALUE_SIZE 256

void ExportVariable( const char* filename, const char* group, const char* varname, int value)
{
    char strValue[32];
    sprintf(strValue, "%d", value);
    ExportVariable(filename, group, varname, strValue);
}

void ExportVariable( const char* p_szFilename, const char* p_szGroup, const char* p_szVarname, const char* p_szValue)
{
	if( !Creat( p_szFilename ) )
		return;

    CIniParserPlus fileParser;
    if( !fileParser.Load(p_szFilename,"r+",true) )
		return;

	char szTmp[MAX_LINE_LEN];
	szTmp[0] = 0;
	fileParser.GetVarRawString( p_szGroup, p_szVarname, szTmp, sizeof(szTmp),0, false );

	if (strcmp(szTmp,p_szValue))
	{	fileParser.SetVarRawString(p_szGroup, p_szVarname, p_szValue,0, true, true);
	}

    fileParser.Release();
}


void ImportVariable( const char* p_szFilename, const char* p_szGroup, const char* p_szVarname, char** p_pszValue )
{
	if( !Creat( p_szFilename ) )
		return;

    *p_pszValue = new char[MAX_VALUE_SIZE];

    CIniParser fileParser;
    if( !fileParser.Load(p_szFilename,"r",true) )
		return;

    if ( !fileParser.GetVarRawString( p_szGroup, p_szVarname, *p_pszValue, MAX_VALUE_SIZE ))
    {
        delete *p_pszValue;
        *p_pszValue = NULL;
    }

    fileParser.Release();

}

void ImportVariable( const char* p_szFilename, const char* p_szGroup, const char* p_szVarname, int* p_nValue )
{
	if( !Creat( p_szFilename ) )
		return;

    CIniParser fileParser;
    if( !fileParser.Load(p_szFilename,"r") )
		return;

    fileParser.GetVar( p_szGroup, p_szVarname, p_nValue );

    fileParser.Release();
}


// This function replaces the system( "touch <filename>" ) mechanism
// that was used by ImportVariable, ExportVariable.
//
// The purpose of this call is to create a file if it does not already
// exist.
//
bool Creat( const char * p_szFilename )
{
	int fd = open( p_szFilename, O_CREAT, 0666 );

	if( -1 == fd )
	{	//this is not error. the file is already there
		//LOG_ERR( "ERROR Creat: call to open() failed" );
		return false;
	}

	if( -1 == close( fd ) )
	{	//this is an error
		LOG_ERR( "ERROR Creat: call to close() failed" );
		return false;
	}

	return true;
}

bool FileIsExecutable( const char * p_szFilename )
{
	struct stat buffer;
	if (stat( p_szFilename, &buffer ))
	{
		return false;
	}
	if ( buffer.st_mode | S_IXUSR | S_IXGRP | S_IXOTH )
	{
		return true;
	}
	return false;
}


///////////////////////////////////////////////////////////////////////////////////
// Name:        AttachDatetime
// Author:		Claudiu Hobeanu
// Description: append datetime to name: name_yyyy_MM_dd_hh_mm_ss ,
//				if dest buffer is NULL the needed space is computed
// Parameters:
//
// Return:      space needed or used
//////////////////////////////////////////////////////////////////////////////////
int	AttachDatetime( const char* p_szName, char* p_szResult )
{
	if (!p_szResult)
	{	return  6 + 4 + 2 + 2 + 2 + 2 + 2 + strlen(p_szName) + 1; //space needed
	}

	time_t t= time(NULL);
	struct tm* ts=gmtime( &t);

	int nLen = strlen (p_szName);

	memcpy( p_szResult, p_szName, nLen);

	return nLen + sprintf( p_szResult + nLen, "_%04d_%02d_%02d_%02d_%02d_%02d",
							ts->tm_year+1900, ts->tm_mon+1, ts->tm_mday, ts->tm_hour, ts->tm_min, ts->tm_sec);
						// be careful here: the name is used to find the age of the file in agesort()
}
///////////////////////////////////////////////////////////////////////////////////
// Name:        Attach_1
// Author:		Marcel Ionescu
// Description: append .1 to name: name.1 ,
//				if dest buffer is NULL the needed space is computed
// Parameters:
//
// Return:      space needed or used
//////////////////////////////////////////////////////////////////////////////////
int	Attach_1( const char* p_szName, char* p_szResult )
{
	if (!p_szResult)
	{	return  2 + strlen(p_szName) + 1; //space needed
	}

	int nLen = strlen (p_szName);

	memcpy( p_szResult, p_szName, nLen);

	return nLen + sprintf( p_szResult + nLen, ".1");
}
///////////////////////////////////////////////////////////////////////////////////
// Name:        GetFreeFlashSpace
// Author:		Marcel Ionescu
// Description: get the available space on the flash
// Parameters:
//
// Return:      space available in bytes
//////////////////////////////////////////////////////////////////////////////////
unsigned long GetFreeFlashSpace( const char * p_szFile )
{
	struct statfs buf;

	if( !statfs( p_szFile, &buf))
	{
		return buf.f_bavail * buf.f_bsize;
	}

	LOG_ERR("ERROR GetFreeFlashSpace:statfs");
	return 0;
}

/// @author:    Marius Negreanu (sue me)
/// @brief Move bytes inside a file.
/// @note It will not resize the file for you.
/*@ requires p_fhFile == NULL || \valid(p_fhFile) ;
  @ requires woff>=0;
  @ requires roff>=0;
  @ behavior zero:
  @    requires p_fhFile == NULL || roff<0 || woff<0;
  @    assigns \nothing;
  @    ensures \result == 0;
  @ behavior one:
  @    requires woff == roff;
  @    assigns \nothing;
  @    ensures \result == 1;
  @ behavior n_less_than_rlen:
  @    requires n < sizeof(rlen);
  @*/
bool fbmove( FILE* p_fhFile, off_t woff, off_t roff, size_t n )
{
	if ( !p_fhFile || roff<0 || woff<0 )  return 0 ;
	if ( woff==roff ) return 1 ;

	char rbuf[8*1024];
	short wdir=1; /* writing direction */
	LOG("moving %i bytes from roff:%u to woff:%u", n, roff, woff);

	// read until EOF
	if ( n==0  )
	{
		if ( -1 == fseek(p_fhFile, 0, SEEK_END) )
		{
			FPERR("fseek for file size failed");
			return 0 ;
		}
		long cur=ftell(p_fhFile)-roff;
		if ( cur <= 0 )
		{
			FPERR("empty file");
			return 0; // error or empty file
		}
		n=(size_t)cur;
		//@ assert n != 0;
	}
	LOG("moving %i bytes",n);
	// overlaping areas
	if ( (roff+n) >= woff && roff<woff)
	{
		LOG("overlaping areas");
		wdir =-1;
		roff += n - _Min(sizeof(rbuf), n) ;
		/*@ for n_less_than_rlen : assert roff == \at(roff,Here) ;*/
		woff += n - _Min(sizeof(rbuf), n) ;
	}
	/*@ decreases n for R;
	  @ loop invariant (rlen>0) ;
	  @ loop variant (n>0) ;*/
	while ( n>0 )
	{
		off_t rlen=_Min(sizeof(rbuf),n);

		if ( -1 == fseek(p_fhFile, roff, SEEK_SET) )
		{
			FPERR("fseek for roff failed") ;
			return 0 ;
		}
		size_t rb = fread( rbuf, sizeof(char), rlen, p_fhFile ) ;
		//@ asume 0<=rb<=rlen;
		if ( !rb && feof(p_fhFile) ) return 1;
		if( ferror(p_fhFile) )
		{
			FPERR("read failed rb %d roff[%d] rlen[%d] n[%d]", rb, roff, rlen, n);
			return 0;
		}

		if ( -1 == fseek(p_fhFile, woff, SEEK_SET) )
		{
			FPERR("fseek for woff failed");
			return 0 ;
		}
		size_t wb = fwrite( rbuf, sizeof(char), rb, p_fhFile );
		if ( wb != rb )
		{
			FPERR("write failed. wb[%d] rb[%d] n[%i] woff[%u] roff[%u]", wb, rb, n, woff, roff );
			return 0 ;
		}
		n-=rb ;
		FLOG("rlen:%i n:%i woff:%i roff:%i br:%i wb:%i", rlen,n,woff,roff, rb, wb);
		//@ assert rlen > 0;
		roff += wdir*rb ;
		woff += wdir*rb;
	}
	fflush(p_fhFile);
	return 1 ;
}


