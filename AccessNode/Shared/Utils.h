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

// Utils.h: interface for the Utils class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UTILS_H__280D7133_DD05_4A95_8511_8F7CADA6D428__INCLUDED_)
#define AFX_UTILS_H__280D7133_DD05_4A95_8511_8F7CADA6D428__INCLUDED_

#include <stdlib.h>
#include <sys/types.h>
#include "Common.h"

/// @addtogroup libshared
/// @{

#define STATUS_COMM_FILE "get_status.cfg"     

//expressed in kB
#define FREE_MEM_MIN_REQ	1024	

//polynom used to calculate crc
#define CRCCCITT 0x1021 

//crc code length
#define CRC_LENGTH 2

//serial line special characters
#define STX        0xF0 
#define ETX        0xF1
#define CHX        0xF2

#define MEMORY_SIZE 0x10000

int FileRead(const char* p_szFile, char* p_pBuff, int p_nBuffLen );

//return sys uptime in seconds
//	on error return <0
float SysGetUpTime();

//return free memory, expressed in bytes
int GetSysFreeMem( void );

//return free memory, expressed in kB
int GetSysFreeMemK( void ); 

//true if memory is below FREE_MEM_MIN_REQ kB
bool IsSysMemoryLow( int p_nKBRequested );



int GetBinData( const char* p_szInput, char* p_pData, int* p_pLen );



int Hex2Bin( const char* p_szInput, char* p_pOutput );
int Hex2Bin( const char* p_pInput, int n, char* p_pOutput );

const char* GetProxyString(net_address p_oProxy);

void ReplaceSpaces( char* p_pString, const char p_cChar = ' ' );
int NextToken( const char* p_pData, const int p_nLen, const char p_cDelim = ';' );
int NoOfDelim( const char* p_pData, const int p_nLen, const char p_cDelim = ';' );

int Copy( const char* p_szSrc, const char* p_szDest, int p_nStep = 4096 );

void EscapeChar(  const u_char * p_psPacket, u_int* p_pnIndexPacket,
                  u_char       * p_psBuffer, u_int* p_pnIndexBuffer );

int ComputeCRC(const unsigned char *packet, unsigned int packetLength, unsigned char crc[CRC_LENGTH], unsigned short p_usCrcStart = 0);

int UnescapePacket(u_char *p_pBuff, int *p_pLen);
int EscapePacket(u_char *p_pSrc, int p_nSrcLen, u_char* p_pDest, int* p_pDestLen );

int ExtractBoundedPacket(u_char* p_pRcvBuffer, int* p_pCrtPosition, u_char *p_pBuff, int *p_pLen);


int	WriteToFileAtOffset(  const char* p_szFile, int p_nOffset, char* p_pBuff, int p_nLen );
//returns integer part of the 1-minute load average
int GetLoadAvg( void );

//creates a file to prove the calling app lives
bool TouchPidFile( const char* p_szName );
//deletes all the files specified and reports if anyone was missing
bool CheckDelPidFiles( const char** p_pszNames, char* p_szMissNames = NULL );

int SetCloseOnExec( int fd );
time_t GetLastModificationOfFile( const char* p_szFile);


bool AdjustIPv6 (TIPv6Address* p_pIpv6, const char* p_szIpv4, int p_nPort);

#define IS_BIT_SET(_vector,_bit) (_vector&_bit) 


#define READ_DEF_VAR_STRING( _ini_, _gr_, _name_, _v_, _defVal_ )		\
	if( !_ini_.GetVar( _gr_, _name_, _v_, sizeof( _v_ ), 0, false ) )			\
	{	strncpy( _v_, _defVal_, sizeof( _v_ ));				\
		_v_ [ sizeof( _v_ )-1 ] = 0;					\
		LOG( "Assign default value for %s = %s", _name_, _v_  );	\
	}else{\
		LOG( "Assign configured value for %s = %s", _name_, _v_ );\
	}

#define READ_DEF_VAR_INT( _ini_, _gr_, _name_, _v_, _defVal_ )		\
	if( !_ini_.GetVar( _gr_, _name_, &_v_, 0, false) )							\
	{	_v_ = _defVal_;												\
		LOG(  "Assign default value for %s = %d", _name_, _v_ );	\
	}

#define READ_DEF_VAR_BOOL(_ini_, _gr_, _name_, _v_, _defVal_ )					\
	{	char _temp_[ 16 ];														\
		READ_DEF_VAR_STRING( _ini_, _gr_, _name_, _temp_, _defVal_  )				\
		_v_ = (		!strcasecmp( _temp_, "Y") || !strcasecmp( _temp_, "YES")	\
				||	!strcasecmp( _temp_, "1") || !strcasecmp( _temp_, "TRUE"));	\
	}

#define READ_DEF_VAR_SHORT(_ini_, _gr_, _name_, _v_, _defVal_ )					\
	{	int  _temp_;														\
		READ_DEF_VAR_INT( _ini_, _gr_, _name_, _temp_, _defVal_  )				\
		_v_ = _temp_;	 \
	}


/// @}
#endif // !defined(AFX_UTILS_H__280D7133_DD05_4A95_8511_8F7CADA6D428__INCLUDED_)
