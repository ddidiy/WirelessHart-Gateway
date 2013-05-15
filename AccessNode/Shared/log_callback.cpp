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




#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdlib.h>
#include <stdio.h> 
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include <unistd.h> 
#include "log_callback.h"
#include "AnPaths.h"

#define ISA100_LOG_START_MSG "STACK:"

int	fast_log_open_file (const char * p_szTrace);
void fast_log_entry ( LOGLVL p_eLogLvl, const char* p_szMsg);

static PfnCallbackLog	spIsaLogFunc 	= NULL;
static LOGLVL			sLogLevel 		= LOGLVL_DBG;

void LogCallback_SetFunc( PfnCallbackLog p_pCallbackLog )
{
	spIsaLogFunc = p_pCallbackLog;
}

void LogCallback_SetLevel( LOGLVL p_eLogLvl )
{
	c_log(LOGLVL_ERR, "Log level changed %d -> %d", sLogLevel, p_eLogLvl);
	sLogLevel = p_eLogLvl;
	
}

int LogCallback_IsLogLevelEnabled( LOGLVL p_eLogLvl )
{
	if( ( p_eLogLvl > sLogLevel )
		||	( !spIsaLogFunc) )	// No callback pfn, no need to parse
		return 0;

	return 1;
}

void LogCallback_UseFasLog(const char* p_szModule)
{
	char szFile[256];

	sprintf(szFile,NIVIS_TMP"stack_fast_%s.log",p_szModule);

	if ( fast_log_open_file(szFile) )
	{
		spIsaLogFunc = fast_log_entry;
	}	
}

////////////////////////////////////////////////////////////////////////////////
/// @brief Printf-like logging function. Send back to user a string with formatted parameters
/// @param fmt printf-like format specifier
/// @param ... log parameters
/// @return nothing
/// @remarks calls the callback spIsaLogFunc which is set by LogCallback_SetFunc
/// passing back the formatted string
/// It does not append timestamp/NL
////////////////////////////////////////////////////////////////////////////////
void c_log( LOGLVL p_eLogLvl, const char* fmt, ... )
{
	char szTmp[2048] = ISA100_LOG_START_MSG;
	int nPos = sizeof(ISA100_LOG_START_MSG) - 1;
	int ret;

	szTmp[sizeof(szTmp) - 1] = 0;

	va_list args;
	va_start(args, fmt);
	ret = vsnprintf( szTmp + nPos, sizeof(szTmp) - nPos - 1, fmt, args);
	va_end(args);

	if (ret >= 0 )
		spIsaLogFunc( p_eLogLvl, szTmp );
}

void c_log_lvlerr( const char* fmt, ... )
{
	if( !spIsaLogFunc)
		return;	// No callback pfn, no need to parse
	
	char szTmp[2048] = ISA100_LOG_START_MSG;
	int nPos = sizeof(ISA100_LOG_START_MSG) - 1;
	int ret;

	szTmp[sizeof(szTmp) - 1] = 0;

	va_list args;
	va_start(args, fmt);
	ret = vsnprintf( szTmp + nPos, sizeof(szTmp) - nPos - 1, fmt, args);
	va_end(args);

	if (ret >= 0 )
		spIsaLogFunc( LOGLVL_ERR, szTmp );
}


void c_log_errno_lvlerr( const char* fmt, ... )
{
	if( !spIsaLogFunc)
		return;	// No callback pfn, no need to parse
	
	char szTmp[2048] = ISA100_LOG_START_MSG;
	int nPos = sizeof(ISA100_LOG_START_MSG) - 1;
	int ret;

	szTmp[sizeof(szTmp) - 1] = 0;

	ret = snprintf( szTmp + nPos, sizeof(szTmp) - nPos - 1, "%s : ", strerror(errno) );

	if ( (ret < 0 ) || ( ret >= (int)(sizeof(szTmp) - 1)) )
		return;

	nPos += ret;

	va_list args;
	va_start(args, fmt);
	ret = vsnprintf( szTmp + nPos, sizeof(szTmp) - nPos - 1, fmt, args);
	va_end(args);

	szTmp[ sizeof( szTmp ) - 1 ] = 0;

	if( ret >= 0 )
		spIsaLogFunc( LOGLVL_ERR, szTmp );
}

////////////////////////////////////////////////////////////////////////////////
/// @brief Hex logger. Send back to user a string with hex represented buffer
/// @param p_sMsg Text to append
/// @param p_pBinBuffer - buffer to log
/// @param p_nBinBufferLen - number of bytesfrom buffer to log
/// @return nothing
/// @remarks calls the callback spIsaLogFunc which is set by LogCallback_SetFunc,
/// passing back the formatted string
/// It does not append timestamp/NL
////////////////////////////////////////////////////////////////////////////////
void c_loghex( LOGLVL p_eLogLvl, const char *  p_sMsg, const void * p_pBinBuffer, int p_nBinBufferLen )
{
	char szTmp[2048] = ISA100_LOG_START_MSG;
	int nPos = sizeof(ISA100_LOG_START_MSG) - 1;
	int		ret,i;
	

	szTmp[sizeof(szTmp) - 1] = 0;

	ret = snprintf(szTmp + nPos, sizeof(szTmp) - nPos - 1, p_sMsg );

	if (ret<0)
	{
		//error nothing to do
		return;
	}
	nPos += ret;

	for (i=0; (i<p_nBinBufferLen) && (nPos<(int)sizeof(szTmp)-4); ++i)
	{
		ret = snprintf(szTmp+nPos,sizeof(szTmp) - 1 - nPos, "%02X ", *((unsigned char*)p_pBinBuffer+i) );
		if (ret<=0)
		{
			break;
		}
		nPos += ret;
	}

	spIsaLogFunc( p_eLogLvl, szTmp );
}

/// Take binary buffer, convert it to hex and send formatted string back o user (no timestamp, no NL)
void c_loghex_lvlerr( const char* p_sMsg, const void * p_pBinBuffer, int p_nBinBufferLen )
{
	c_loghex( LOGLVL_ERR, p_sMsg, p_pBinBuffer, p_nBinBufferLen );
}

// Trace.cpp: implementation of the CTrace class.
//
//////////////////////////////////////////////////////////////////////

static int g_nFd = 1;

int	fast_log_open_file (const char * p_szTrace)
{
	int nFd = open(p_szTrace, O_RDWR | O_CREAT, 0666 );
	if (nFd<0)
	{	//TRACE(0, ("Failed to open file %s", p_szTrace));
		return 0;
	}
	lseek( nFd, 0, SEEK_END );

	g_nFd = nFd;
	return 1;
}

void fast_log_entry ( LOGLVL p_eLogLvl, const char* p_szMsg)
{
	char			szBuff[1024];
	char			szErr[128];

	time_t tTime    = time( NULL );
	struct tm * pTm = gmtime( &tTime );

	int nPrint = sprintf( szBuff, "%04d-%02d-%02d %02d:%02d:%02d %s",
		pTm->tm_year+1900,pTm->tm_mon+1,pTm->tm_mday, pTm->tm_hour,pTm->tm_min,pTm->tm_sec,
		szErr );

	write (g_nFd, szBuff, nPrint);
}
