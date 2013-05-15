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
                          Common.h  -  description
                             -------------------
    begin                : Fri Apr 12 2002
    email                : marcel.ionescu@nivis.com
 ***************************************************************************/
/** @mainpage

    <table>
        <tr><th>Library     <td>libshared
        <tr><th>Author      <td>Nivis code monkeys
    </table>
    @section intro INTRODUCTION

@defgroup libshared Nivis Shared Library
@{ 
*/

//lint -library

#ifndef _COMMON_H_
#define _COMMON_H_

#include <time.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/param.h>
#include <stdint.h>
#include <cstdio>

#include "AnPaths.h"
#include "UtilsSolo.h"

#include "log_callback.h"

#ifdef CALLBACK_LOG	// support for callback logging


/// compatibility hack. c_log actually does not log perror
/// TODO: implement c_logerr
#define LOG_ERR c_log_errno_lvlerr
#define LOG c_log_lvlerr
#define LOG_HEX c_loghex_lvlerr

#else	// normal logging


//include log defines like LOG, LOG_HEX, NLOG_ERR, etc
#include "LogDefs.h"

const char* NLogLevelGetName (NLOG_LVL p_nLogLevel);
NLOG_LVL NLogLevelGetType (const char*  p_szLogLevel);

#endif //CALLBACK_LOG

// typedefs
struct rf_address{
    unsigned char id[4];
}__attribute__((packed));

struct net_address
{
    unsigned long m_nIP;
    unsigned short m_dwPortNo;
}__attribute__((packed));

#define IPv6_ADDR_LENGTH 16	///< The length of a IPv6 Address.
struct TIPv6Address
{
	const static int nIPv6PrintSize = 4 * 8 + 7 + 1; // 8 groups of 2 bytes and 7 ':'and '\0'
	uint8_t m_pu8RawAddress[IPv6_ADDR_LENGTH];

	TIPv6Address();

	int FillIPv6 (char* p_szBuff, int p_nMaxSize) const ;

	const char* GetPrintIPv6();
};




inline clock_t GetClockTicks( void ) {	struct tms tms_buf;	return times( &tms_buf );}

int CheckAndFixWrapClockTicks( clock_t p_nCrtTime, clock_t *p_pnTimeout );
int TrackPointTimeout (int p_nDuration, const char *p_szFile, const char* p_szFunc, int p_nLine);

#ifdef CYG
	int IGN_FCT1( ... );
	#define stime IGN_FCT1
	#define setpriority IGN_FCT1
	#define PRIO_PROCESS 0
	#define PIPE_BUF 4096
//	#define PATH_MAX 512
//#define INT_MAX 2000000000
	//const void * memmem( const void * p_pBuffer, unsigned int p_nBufferLen, const void * p_pNeed,  unsigned int p_nNeedLen );
#endif



#if __GNUC__ < 3 || CYG
//const void * memmem( const void * p_pBuffer, unsigned int p_nBufferLen, const void * p_pNeed,  unsigned int p_nNeedLen );
#endif

const char* JumpOverBlank(const char *p_szString);
//bool EscapeString(char *p_szString);

//char* RTrim( const char* p_szString );
//int ReadField( char* p_szString, char* p_pField, int p_nLen, int sens = 0 );

int GetAbsDiffFromCrtTime(  unsigned char year,
                    unsigned char month,
                    unsigned char day,
                    unsigned char hour,
                    unsigned char minute,
                    unsigned char second
                   );


int GetFileLen( int fd );
int GetFileLen( const char * p_szFile );

int WriteToFile( const char* p_szFile, const char* p_pBuff, bool p_nTrunc = false, int p_nPos = -1 );
int WriteToFile( const char* p_szFile, const char* p_pBuff, int p_nLen, bool p_nTrunc = false, int p_nPos = -1 );
int GetFileData( const char* p_pFileName, char *& p_pData, int& p_rLen );


/* offsetof macro */
#ifndef offsetof
#define offsetof(s,m)   (size_t)&(((s *)0)->m)
#endif


#if !defined(SAFE_DELETE)
#	define SAFE_DELETE(x)	if (x) {delete x; x = NULL;}
#endif



#define LOG_TIME( _szMsg_, _timeStart_ )                \
LOG( _szMsg_,                                                               \
2000+( int )_timeStart_[0], ( int )_timeStart_[1], ( int )_timeStart_[2],   \
     ( int )_timeStart_[3], ( int )_timeStart_[4], ( int )_timeStart_[5] );   \

#define LOG_TIME_INTERVAL( _szMsg_, _timeStart_, _timeEnd_ )                \
{                                                                           \
LOG( _szMsg_,                                                               \
2000+( int )_timeStart_[0], ( int )_timeStart_[1], ( int )_timeStart_[2],   \
     ( int )_timeStart_[3], ( int )_timeStart_[4], ( int )_timeStart_[5],   \
2000+( int )_timeEnd_[0],   ( int )_timeEnd_[1],   ( int )_timeEnd_[2],     \
     ( int )_timeEnd_[3],   ( int )_timeEnd_[4],   ( int )_timeEnd_[5] );   \
}


#ifndef _Max
#define _Max( x, y ) ( ( x ) > ( y ) ? x : y )
#endif

#ifndef _Min
#define _Min( x, y ) ( ( x ) < ( y ) ? x : y )
#endif






//execute shell command with timeout and variable number of parameters
int systemf_to(int timeout, const char *cmd,...);

//execute shell command with timeout
int system_to( int timeout, const char *cmd );

//execute connect with timeout
int connect_to(int sockfd, const struct sockaddr *serv_addr, socklen_t addrlen, int timeout);

//get free space (bytes) on the partition containing the file given as parameter
// valid only until free space is aroung 2 GB.
unsigned long GetFreeFlashSpace( const char * p_szFile );




//////////////////////////////////////////////////////////////////////////////
/// @brief       Copy file sections from roff to woff. Sections can overlap.
/// @author      Marius Negreanu
/// @param [in]  p_fhFile   File handle.
/// @param [in]  woff       Destination file offset.
/// @param [in]  roff       Source file offset.
/// @param [in]  n          Bytes to copy from roff to woff. If n=0 then will
///                         read until EOF.
/// @retval true            All n bytes were copied.
/// @retval false           Operation failed.
//////////////////////////////////////////////////////////////////////////////
bool fbmove( FILE* p_fhFile, off_t woff, off_t roff, size_t n ) ;



#define FLAG( i ) ( 1 << i )


#define RtlZeroMemory(Destination,Length) memset((Destination),0,(Length))
#define ZeroMemory RtlZeroMemory



void ExportVariable( const char* filename, const char* group, const char* varname, int value);
void ExportVariable( const char* filename, const char* group, const char* varname, const char* value);

void ImportVariable( const char* filename, const char* group, const char* varname, int* value);
void ImportVariable( const char* filename, const char* group, const char* varname, char** value);

bool Creat( const char * p_szFilename );
bool FileIsExecutable( const char * p_szFilename );
int		FileExist (const char * p_szFile);
int		AttachDatetime( const char* p_szName, char* p_szResult = NULL ) ;
int		Attach_1( const char* p_szName, char* p_szResult = NULL ) ;

void alrm_handler(int) ;

#ifdef CYG
inline int getline(char**, size_t*, FILE* ) { return 0; }
#endif
/// @}
#endif //COMMON_H
