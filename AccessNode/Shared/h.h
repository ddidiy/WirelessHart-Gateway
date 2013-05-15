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

#ifndef _H_H_
#define _H_H_

/*****************************************************************************
 * Name:        h.h
 * Author:      Marius Negreanu
 * Date:        Tue Nov 20 09:15:37 2007 UTC
 * Description: Common Use datatypes/structures/macros.
 ****************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <assert.h>
#include <arpa/inet.h> // hton ntoh

/// @addtogroup libshared
/// @{

#define GCC_PACKED __attribute__((__packed__))

//// safe string macros
//#define str(x) # x
//#define xstr(x) str(x)

#if defined(__STDC__) && !defined( __cplusplus )
#    include "c_log.h"
#    define LOG(...) c_log( &g_stLog, __VA_ARGS__ )
#else
#    include "Common.h"
#endif // defined(__STDC__) && !defined( __cplusplus )




// All macro's evaluate to compile-time constants
#define HEX__(n) 0x##n##LU

// 8-bit conversion function
#define B8__(x) ((x&0x0000000FLU)?1:0) \
+((x&0x000000F0LU)?2:0) \
+((x&0x00000F00LU)?4:0) \
+((x&0x0000F000LU)?8:0) \
+((x&0x000F0000LU)?16:0) \
+((x&0x00F00000LU)?32:0) \
+((x&0x0F000000LU)?64:0) \
+((x&0xF0000000LU)?128:0)

/* for upto 8-bit binary constants */
#define B8(d) ((unsigned char)B8__(HEX__(d)))

/* for upto 16-bit binary constants, MSB first */
#define B16(dmsb,dlsb) (((unsigned short)B8(dmsb)<<8) \
+ B8(dlsb))

/* for upto 32-bit binary constants, MSB first */
#define B32(dmsb,db2,db3,dlsb) (((unsigned long)B8(dmsb)<<24) \
+ ((unsigned long)B8(db2)<<16) \
+ ((unsigned long)B8(db3)<<8) \
+ B8(dlsb))



#ifdef __cplusplus


uint16_t computeCRC( uint8_t *buff, uint16_t bufSz ) ;

///////////////////////////////////////////////////////////////////////////////
//! @brief Binary Search function.
//! @param [in] p_iHayStackSz The size of the Hay stack.
//! @param [in] p_tNeedle Key to be searched.
//! @param [in] p_tHayStack The colection to be searched.
//! @param [in] compare Callback function used to compare two elements.
//! @param [out] p_riPos Position where the element is/can_be found/inserted.
//! @retval true Element found.
//! @retval false Element not found.
///////////////////////////////////////////////////////////////////////////////
template<typename T1, typename T2, typename C>
bool
binSearch( int p_iHayStackSz, T1 p_tNeedle, T2 p_tHayStack, C compare, int& p_riPos )
{
	int low=0, mid=0;

	assert(p_tHayStack!=NULL) ;

	int high = p_iHayStackSz - 1;	//zero based array

	while( low <= high )
	{
		mid = ( low  + high ) / 2;
		int rv = compare(p_tNeedle, p_tHayStack[mid]) ;
		if ( !rv ) {
			p_riPos = mid ;
			return true ;
		}
		if( rv < 0 )
			high = mid-1 ;
		else
			low = mid+1;
	}
	p_riPos = low ;
	return false ;
}


///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
template< class T>
void printfBin ( T num)
{
    for( int i=sizeof(T)*8-1; i>=0; --i)
    {
	printf ("%d", (num >> i) & 1);
    }
    printf("\n") ;
}

template < typename T >
void printfHexArray( T array, int start, int end )
{
	printf("len =%d hex=", end-start ) ;
	for ( ; start < end ; ++start )
		printf( "%.2x ", array[start] );
}

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
template< class T>
void LOG_BIN ( T num, const char* prefix=0)
{
    unsigned int bits = sizeof(T)*8 ;
    char* buf = (char*)malloc(bits+1) ;

    for( int i=bits-1; i>=0; --i)
    {
	buf[bits-1-i] = ((num>>i) & 1) + '0';
    }
    buf[bits] = 0;
    if( prefix==0 )
	prefix=" ";
    LOG("BIN:%s %s", prefix, buf);
    free(buf);
}


///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
class CFrameParser {
public:
	CFrameParser() {} ;
	virtual ~CFrameParser() {} ;
	virtual int ParseRxMessage( uint8_t *p_buf, uint16_t &p_bufLen) =0;
	virtual int UnescapeMsg( uint8_t *msg, int msgSz) =0;
	virtual uint16_t EscapeMsg( const uint8_t* msg, uint16_t msgSz, uint8_t* emsg ) =0;
};


class CNivisFrameParser_v2 : public CFrameParser {
public:
	CNivisFrameParser_v2() {} ;
	~CNivisFrameParser_v2() {} ;
	int ParseRxMessage( uint8_t *p_buf, uint16_t &p_bufLen) ;
	int UnescapeMsg( uint8_t *msg, int msgSz) ;
	uint16_t EscapeMsg( const uint8_t* msg, uint16_t msgSz, uint8_t* emsg ) ;
};

class CNivisFrameParser_v3 : public CNivisFrameParser_v2 {
public:
	CNivisFrameParser_v3() {} ;
	~CNivisFrameParser_v3() {} ;
	int ParseRxMessage( uint8_t *p_buf, uint16_t &p_bufLen) ;
	int UnescapeMsg( uint8_t *msg, int msgSz) ;
};

class CDiverFrameParser_v1 : public CFrameParser {
public:
	CDiverFrameParser_v1()
		: m_nCargoLen(0)
		, m_cCmd(0)
		{}
	~CDiverFrameParser_v1() {} ;
	virtual int ParseRxMessage( uint8_t *p_buf, uint16_t &p_bufLen) ;
	int UnescapeMsg( uint8_t *msg, int msgSz) ;
	uint16_t EscapeMsg( const uint8_t* msg, uint16_t msgSz, uint8_t* emsg ) ;
public:
	static uint8_t	sOH ;
	static uint8_t	sTX ;
	static uint8_t	eTX ;
	unsigned int	m_nCargoLen;
	unsigned char	m_cCmd ;

};

class CAmberFrameParser_v1 : public CDiverFrameParser_v1 {
public:
	CAmberFrameParser_v1()
		: m_nCargoLen(0)
		, m_cCmd(0)
		{}
	~CAmberFrameParser_v1() {} ;
	int ParseRxMessage( uint8_t *p_buf, uint16_t &p_bufLen) ;
public:
	static uint8_t	sOH ;
	static uint8_t	sTX ;
	static uint8_t	eTX ;
	unsigned int	m_nCargoLen;
	unsigned char	m_cCmd ;

};


struct delete_object
{
	template <typename T>
	void operator()(T *ptr){ delete ptr; ptr=NULL; }
};

inline uint16_t networkorder16( uint16_t w)
{
	return htons(w) ;
}
inline uint32_t networkorder32( uint32_t w)
{
	return htonl(w) ;
}

inline uint16_t hostorder16( uint16_t w)
{
	return ntohs(w) ;
}
inline uint32_t hostorder32( uint32_t w)
{
	return ntohl(w) ;
}


#endif	// __cplusplus



#define STX 0xF0
#define ETX 0xF1
#define CHX 0xF2
/// @}
#endif	//_H_H_
