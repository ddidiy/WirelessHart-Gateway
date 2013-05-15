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


#include <string.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>


#include "Shared/Utils.h"
#include "methods/core/ConfigImpl.h"
#include "SmImpl.h"

#define CONF_SM_LOG			NIVIS_PROFILE"log4cpp.properties"

#if defined( RELEASE_ISA )
    #define CONF_SM_WHITELIST	NIVIS_PROFILE"system_manager.ini"
	#define EUI64_SEP	':'
	#define EUI64_INTERVAL_SEP	'-'
	#define SEPARATOR_STEP	5
#elif defined( RELEASE_WHART )
    #define CONF_SM_WHITELIST	NIVIS_PROFILE"whart_provisioning.ini"
	#define EUI64_SEP	'-'
	#define EUI64_INTERVAL_SEP	':'
	#define SEPARATOR_STEP	3
#endif

#define MCS_CONF_SM_WHITELIST_ISA		"/etc/opt/isa100/profile/"  "system_manager.ini"	
#define MCS_CONF_SM_WHITELIST_WHART		"/etc/opt/whart/profile/"  "whart_provisioning.ini"	

const char* GetSmWhiteListFile( int p_nNetworkType)
{
	switch(p_nNetworkType)
	{
	case 0:
		return CONF_SM_WHITELIST;
	case 1:
		return MCS_CONF_SM_WHITELIST_ISA;
	case 2:
		return MCS_CONF_SM_WHITELIST_WHART;
	}

	return  CONF_SM_WHITELIST;
}

#define EUI64_PAIR_SIZE (16*2+1+1)	// two EUI64, one dash, one null terminator
#define EUI64_SIZE (16+1) 			// one EUI64, one null terminator

static bool sExtractEUI64( const char * p_szSrc, size_t p_dwSize, char * p_pDst);
static bool sExtractEUI64Specifier( const char * p_szSrc, size_t p_dwSize, char * p_pDst);
static const char * sFormatEUI64( const char * p_pEUI64, char *p_pBuffer );
static const char * sFormatEUI64Specifier( const char * p_pEUI64Specifier, char *p_pBuffer );
static bool sFormatDevice( const char * p_szDevice, char *p_pBuffer, size_t p_dwBufferSize );

static long findLogLine(FILE* stream, char** lineptr, const char** rs )
{
	static char level[16];
	char file[64]={0} ;
	size_t n=0;
	long rb =0;
	*lineptr = NULL;

	while ( !feof(stream) )
	{
		ssize_t rv ;
		rv = getline( lineptr, &n, stream );
		if ( rv < 0 && !feof(stream) )
		{
			LOG("ERROR: Unable to read " CONF_SM_LOG ":%s", strerror(errno) );
			*rs = NULL ;
			return rb ;
		}
		rb += rv ;
		if( sscanf( *lineptr, "log4cplus.rootLogger = %15[a-zA-Z0-9], %63[a-zA-Z0-9]", level, file ) >0 )
		{
			*rs = level;
			break;
		}
		file[0] = 0;
	}
	return rb ;
}

////////////////////////////////////////////////////////////////////////////////
/// @author Marcel Ionescu
/// @brief Extract EUI64: 8 bytes, hex-represented: 16 chars) from a variety of input string formats
/// @brief stripping any whitespaces/separators except comma - which is a field separator
/// @param p_szSrc Formatted EUI64
/// @param p_szDst Filled with stripped EUI64, exactly 16 bytes without NULL terminator.
///			It is allocated by user and have at least 16 bytes reserved
/// @retval false if the string cannot be extracted - may also be used to validate the input
/// @note Does null-terminate the string.
////////////////////////////////////////////////////////////////////////////////
static bool sExtractEUI64( const char * p_szSrc, size_t p_dwSize, char * p_pDst)
{	size_t nChr = 0;
	if(p_dwSize < EUI64_SIZE)
	{
		LOG("PROGRAMMER ERROR sExtractEUI64: %u not enough storage for EUI64", p_dwSize);
		return false;
	}
	for( int i = 0; p_szSrc[i] && (p_szSrc[i] != ',') && (p_szSrc[i] != EUI64_INTERVAL_SEP) && (nChr < (p_dwSize-1)); ++i )	/// stop parsing at comma
	{	if( isxdigit( p_szSrc[i] ))
		{	if( p_pDst )
				p_pDst[ nChr ] = p_szSrc[ i ];
			++nChr;
		}
	}
	p_pDst[ nChr ] = 0;
	if(nChr != EUI64_SIZE-1)
		LOG("WARNING sExtractEUI64 [%s] %u %u => cannot extract", p_szSrc, nChr, p_dwSize);
	return nChr == EUI64_SIZE-1;
}

////////////////////////////////////////////////////////////////////////////////
/// @author Marcel Ionescu
/// @brief Extract EUI64 specifier: either EUI64 pair (two EUI64 with a dash in between) or a single EUI64
/// @brief stripping any whitespaces/separators except comma - which is a field separator.
/// @brief Each EUI64 is 8 bytes, hex-represented: 16 chars.
/// @param p_szSrc EUI64 specifier: one EUI64 or EUI64 pair, in any format. Comma terminates the specifier
///			If present, dash specified a pair, otherwise there is a single value
/// @param p_szDst Filled with stripped EUI64, exactly 16 bytes without NULL terminator.
///			It is allocated by user and have at least 16 bytes reserved
/// @retval false if the string cannot be extracted - may also be used to validate the input
/// @note Does null-terminate the string.
////////////////////////////////////////////////////////////////////////////////
static bool sExtractEUI64Specifier( const char * p_szSrc, size_t p_dwSize, char * p_pDst)
{
	if(p_dwSize < EUI64_PAIR_SIZE)
	{
		LOG("PROGRAMMER ERROR sExtractEUI64Specifier: %u not enough storage for EUI64 pair", p_dwSize);
		return false;
	}
	const char * pDash  = strchr(p_szSrc, EUI64_INTERVAL_SEP);
	const char * pComma = strchr(p_szSrc, ',');
	if(pDash && pComma && (pDash < pComma))	// extract EUI64 pair
	{	char tmpEUI64[2][EUI64_SIZE];
		LOG("sExtractEUI64Specifier: extracting EUI64 pair from [%s]", p_szSrc);
		if( sExtractEUI64(p_szSrc, EUI64_SIZE, tmpEUI64[0]) && sExtractEUI64(pDash+1, EUI64_SIZE, tmpEUI64[1]))
		{	sprintf(p_pDst, "%s-%s", tmpEUI64[0], tmpEUI64[1]);
			return true;
		}
		return false;
	}
	//LOG("sExtractEUI64Specifier: extracting single EUI64 (%s)", (!pComma)?"NO comma":(!pDash || pDash > pComma)?"NO dash":"WHY?");
	return sExtractEUI64(p_szSrc, EUI64_SIZE, p_pDst); //there is no pair, extract a single EUI64
}

////////////////////////////////////////////////////////////////////////////////
/// @author Marcel Ionescu
/// @brief format EUI64 as XXXX:XXXX:XXXX:XXXX(isa)/XX-XX-XX-XX-XX-XX-XX-XX(whart)
/// @param p_pEUI64 input string, unformatted 16 bytes (hex-representation)
/// @param p_pBuffer buffer to receive formatted string.
/// @note TAKE CARE p_pBuffer must have allocated at least 20 bytes
////////////////////////////////////////////////////////////////////////////////
static const char * sFormatEUI64( const char * p_pEUI64, char *p_pBuffer )
{	int j = 0;
	for( int i = 0; i < 16; ++i, ++j )
	{
		if( !((j+1)%SEPARATOR_STEP) )
		{
			p_pBuffer[ j++ ] = EUI64_SEP;
		}
		p_pBuffer[ j ] = toupper(p_pEUI64[ i ]);
	}
	p_pBuffer[ j ] = 0;
	return p_pBuffer;
}

////////////////////////////////////////////////////////////////////////////////
/// @author Marcel Ionescu
/// @brief format EUI64Specifier as either XXXX:XXXX:XXXX:XXXX(isa)/XX-XX-XX-XX-XX-XX-XX-XX(whart) (if input is single EUI64) or
/// @brief XXXX:XXXX:XXXX:XXXX - XXXX:XXXX:XXXX:XXXX (if input is EUI64 pair dash-separated - only for isa)
/// @param p_pEUI64Specifier input string, unformatted 16 bytes (hex-representation) or pair of 16 bytes separated by dash
/// @param p_pBuffer buffer to receive formatted string.
/// @note TAKE CARE p_pBuffer must have allocated at least 19+19+3+1=42 (only for isa) bytes
////////////////////////////////////////////////////////////////////////////////
static const char * sFormatEUI64Specifier( const char * p_pEUI64Specifier, char *p_pBuffer )
{	const char * pDash  = strchr(p_pEUI64Specifier, EUI64_INTERVAL_SEP);
	sFormatEUI64( p_pEUI64Specifier, p_pBuffer);	// Format first EUI64
	/// pDash+1 points to second EUI64, skipping dash
	if( pDash && ((pDash + 1 + 16) <= (p_pEUI64Specifier + strlen(p_pEUI64Specifier))) )
	{	// Format second EUI64, if provided
		char sep[] = "   ";
		sep[1] = EUI64_INTERVAL_SEP;
		strcat(p_pBuffer, sep);
		sFormatEUI64( pDash+1, p_pBuffer+strlen(p_pBuffer) );
	}
	return p_pBuffer;
}

////////////////////////////////////////////////////////////////////////////////
/// @author Marcel Ionescu
/// @brief format a device as needed in system_manager.ini
/// @param p_szDevice the input string format
/// @remarks the output format is <EUI64Specifier>, <JoinKey>, <SubnetId>
///	@remarks <EUI64Specifier> is <EUI64> or <EUI64> - <EUI64>
///	@remarks <EUI64> formatted XXXX:XXXX:XXXX:XXXX(isa)/XX-XX-XX-XX-XX-XX-XX-XX(whart)
///	@remarks <JoinKey> formatted XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX
///	@remarks <SubnetId> decimal
/// @note helper for CIsa100Impl::setDevice
////////////////////////////////////////////////////////////////////////////////
static bool sFormatDevice( const char * p_szDevice, char *p_pBuffer, size_t p_dwBufferSize )
{	char inputDevice[ EUI64_PAIR_SIZE ], formattedInputDevice[ 64 ];
	if( !sExtractEUI64Specifier( p_szDevice, EUI64_PAIR_SIZE, inputDevice ) )
	{
		LOG("ERROR sFormatDevice: cannot extract EUI64 from [%s]", p_szDevice);
		return false;
	}

	const char *p = strchr( p_szDevice, ',' );
	if( !p )
	{
		LOG("ERROR sFormatDevice: no comma found in [%s]", p_szDevice);
		return false;
	}

	strncpy( p_pBuffer, sFormatEUI64Specifier( inputDevice, formattedInputDevice ), p_dwBufferSize );
	p_pBuffer[ p_dwBufferSize - 1 ] = 0;

	/// TODO: format <JoinKey>
	strncat( p_pBuffer, p, p_dwBufferSize - strlen(p_pBuffer) );
	p_pBuffer[ p_dwBufferSize -1 ] = 0;

	return true;
}

////////////////////////////////////////////////////////////////////////////////
/// @author Marcel Ionescu
/// @brief Update a device info in SM provisioning file
/// @param p_szDevice The device value with format <EUI64Specifier>, <JoinKey>, <SubnetId>
/// @param p_szDeviceType - DEVICE / BACKBONE / GATEWAY
/// @retval false cannot set the device
/// @retval true device updated ok
///	@remarks <EUI64Specifier> is <EUI64> or <EUI64> - <EUI64>
/// @remarks primary key is <EUI64Specifier>
/// @remarks If device was already there, updates it's info, otherwise add the device
////////////////////////////////////////////////////////////////////////////////
bool SmImpl::setDevice( const char * p_szDevice, const char * p_szDeviceType, int p_nNetworkType )
{	CConfigImpl oConfig;
	char szFormattedRow[ 1024 ];
	if( !sFormatDevice(p_szDevice, szFormattedRow, sizeof(szFormattedRow) ))
		return false;
	return oConfig.setConfigRow( GetSmWhiteListFile(p_nNetworkType), "SECURITY_MANAGER", p_szDeviceType, szFormattedRow, &sExtractEUI64Specifier, EUI64_PAIR_SIZE );
}

////////////////////////////////////////////////////////////////////////////////
/// @brief   Delete a device line from the SM provisioning file.
/// @param   p_szDevice Device string="<EUI64Specifier>, <JoinKey>, <SubnetId>"
/// @param   p_szDeviceType - DEVICE / BACKBONE / GATEWAY
/// @retval  false cannot erase the device
/// @retval  true  device erased ok
///	@remarks <EUI64Specifier> is <EUI64> or <EUI64> - <EUI64>
/// @remarks primary key is <EUI64Specifier>
////////////////////////////////////////////////////////////////////////////////
bool SmImpl::delDevice( const char * p_szDevice, const char * p_szDeviceType, int p_nNetworkType )
{
	CConfigImpl oConfig;


	return oConfig.delConfigRow( GetSmWhiteListFile(p_nNetworkType), "SECURITY_MANAGER", p_szDeviceType, p_szDevice, &sExtractEUI64Specifier, EUI64_PAIR_SIZE );
}


//log4cplus.rootLogger=INFO, Isa100LogFile
const char * SmImpl::getLogLevel( void )
{
	char *lineptr = NULL;
	const char *rs = NULL;
	FILE * stream ;

	stream = fopen( CONF_SM_LOG ,"r") ;
	if ( ! stream )
	{
		LOG("Unable to open " CONF_SM_LOG );
		return NULL ;
	}
	findLogLine(stream, &lineptr, &rs) ;
	free(lineptr);
	fclose(stream);
	return rs ;
}

bool SmImpl::setLogLevel( const char * p_szLogLevel )
{
	FILE * stream ;
	char * lineptr = NULL;
	const char *rs = NULL;

	stream = fopen( CONF_SM_LOG,"r+") ;
	if ( ! stream )
	{
		LOG("Unable to open " CONF_SM_LOG );
		return false ;
	}
	long rb = findLogLine(stream, &lineptr, &rs );

	if ( rs ) /// LEVEL line found
	{
		char buf[128] ;
		long before, after = rb, fsize ;

		fseek(stream,0, SEEK_END) ;
		fsize = ftell(stream) ;

		before = after - strlen(lineptr) ;

		sprintf( buf, "log4cplus.rootLogger=%s, Isa100LogFile\n", p_szLogLevel ) ;
		fbmove(stream, before+strlen(buf), after, fsize - after );
		fseek( stream, before, SEEK_SET ) ;
		fprintf( stream, "%s", buf );
	}
	free(lineptr);
	fclose(stream);
	return true ;
}

