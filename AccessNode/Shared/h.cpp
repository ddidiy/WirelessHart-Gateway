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

#include "h.h"
#include <arpa/inet.h>
//table used for CRC computing
const unsigned int crctable[256]={
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
	0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0};

#if 0
#ifdef DEBUG
#include <new>
void* operator new(size_t size, int line, const char* file, const char* function)
{
	void* ptr = ::new uint8_t [size];
	LOG("NEW %d %p %s %s %d", size, ptr, function, file, line);
	return ptr;
}
void* operator new[](size_t size, int line, const char* file, const char* function)
{
	void* ptr = ::new uint8_t [size];
	LOG("NEW %d %p %s %s %d", size, ptr, function, file, line);
	return ptr;
}

void operator delete( void* ptr, int line, const char* file, const char* function )
{
	LOG("DEL %p %s %s %d", ptr, function, file, line);
	::delete(ptr);
	ptr = 0 ;
}
void operator delete[](void* ptr, int line, const char* file, const char* function  )
{
	LOG("DEL %p %s %s %d", ptr, function, file, line);
	::delete[](ptr);
	ptr = 0 ;
}
#endif

#endif
int CNivisFrameParser_v3::ParseRxMessage ( uint8_t *p_buf, uint16_t &p_bufLen)
{
	if( !p_bufLen ) return 0;

	void *rv =NULL ;
	int i =0;
	if ( p_buf[0] != STX ) // STX not found
	{
		rv = memchr( p_buf, STX, p_bufLen );
		if ( !rv ) {
			WARN("No STX Found. Drop whole buffer.");
			p_bufLen = 0 ;
			return 0 ;
		}
		int diff = (uint8_t*)rv - p_buf;
		if ( diff )
		{
			LOG_HEX( "WARN: Drop the bytes found before STX:", p_buf, diff ) ;
			memmove( p_buf, p_buf+diff, p_bufLen - diff ) ;
			p_bufLen -= diff ;
		}
	}

	if (  p_buf[0] == STX  )
	{
		//LOG_HEX( "Scan ETX:", p_buf, p_bufLen );
		for ( i=1; i < p_bufLen; ++i )
		{
			if ( p_buf[i] == STX ) {
				LOG( "i=%d", i) ;
				LOG_HEX( "WARN: STX Again. Drop whole buffer", p_buf, p_bufLen ) ;
				p_bufLen = 0;
				return 0 ;
			}
			if ( p_buf[i] == ETX ) {
				//LOG_HEX( "MSG_WAITING: ", p_buf, i+1 ) ;
				return (i+1) ;
			}
		}
	}
	return 0 ;
}


///////////////////////////////////////////////////////////////////////////////
//! @return the length of the unEscaped message
//! @retval 0 Error
//! @retval !0 Success
///////////////////////////////////////////////////////////////////////////////
int CNivisFrameParser_v3::UnescapeMsg( uint8_t *msg, int msgSz)
{
	int o=0;
	if ( !msg || !msgSz ) return 0 ;
	if ( msg[0] != STX || msg[msgSz-1] != ETX )
	{
		LOG("Invalid message");
	}
	memmove( msg, msg+1 , msgSz -1 ) ;// don't copy the end ETX
	msgSz -= 2 ;
	for ( int i=0; i < msgSz ; ++o, ++i )
	{
		if ( msg[i] != CHX )
		{
			msg[o]= msg[i];
		}
		else
		{
			msg[o] = ~msg[++i] ;
		}
	}
	return o ;
}

int CNivisFrameParser_v2::ParseRxMessage ( uint8_t *p_buf, uint16_t &p_bufLen)
{
	if( !p_bufLen ) return 0;

	void *rv =NULL ;
	int i =0;
	if ( p_buf[0] != STX ) // STX not found
	{
		rv = memchr( p_buf, STX, p_bufLen );
		if ( !rv ) {
			WARN("No STX Found. Drop whole buffer.");
			p_bufLen = 0 ;
			return 0 ;
		}
		int diff = (uint8_t*)rv - p_buf;
		if ( diff )
		{
			LOG_HEX( "WARN: Drop the bytes found before STX:", p_buf, diff ) ;
			memmove( p_buf, p_buf+diff, p_bufLen - diff ) ;
			p_bufLen -= diff ;
		}
	}

	if (  p_buf[0] == STX  )
	{
		//LOG_HEX( "Scan ETX:", p_buf, p_bufLen );
		for ( i=1; i < p_bufLen; ++i )
		{
			if ( p_buf[i] == STX ) {
				LOG( "i=%d", i) ;
				LOG_HEX( "WARN: STX Again. Drop whole buffer", p_buf, p_bufLen ) ;
				p_bufLen = 0;
				return 0 ;
			}
			if ( p_buf[i] == ETX ) {
				//LOG_HEX( "MSG_WAITING: ", p_buf, i+1 ) ;
				return (i-1) ;
			}
		}
	}
	return 0 ;
}


///////////////////////////////////////////////////////////////////////////////
//! @return the length of the unEscaped message
//! @retval 0 Error
//! @retval !0 Success
///////////////////////////////////////////////////////////////////////////////
int CNivisFrameParser_v2::UnescapeMsg( uint8_t *msg, int msgSz)
{
	int o=0;
	for ( int i=0; i < msgSz ; ++o, ++i ) {
		if ( msg[i] != CHX ) {
			msg[o]= msg[i];
		}else
			msg[o] = ~msg[++i] ;
	}
	return o ;
}


///////////////////////////////////////////////////////////////////////////////
// class NivisParse_v2 {
// }
///////////////////////////////////////////////////////////////////////////////
uint16_t CNivisFrameParser_v2::EscapeMsg( const uint8_t* msg, uint16_t msgSz, uint8_t* emsg )
{
	uint8_t * p = emsg;
	*(p++) = STX;
	for ( int i =0; i< msgSz; ++i ) {
		switch( msg[i] )
		{
		case STX: *(p++) = CHX; *(p++) = (uint8_t)~STX ; break;
		case ETX: *(p++) = CHX; *(p++) = (uint8_t)~ETX ; break;
		case CHX: *(p++) = CHX; *(p++) = (uint8_t)~CHX ; break;
		default:    *(p++) = msg[i];
		}
	}
	*(p++) = ETX;
	return (p-emsg) ;
}


uint16_t computeCRC( uint8_t *buff, uint16_t bufSz )
{
	uint16_t nAcumulator = 0xFFFF ;
	for ( int i=0; i < bufSz; ++i )
	{
		nAcumulator = (nAcumulator << 8) ^ crctable[(nAcumulator >> 8) ^ buff[i] ] ;
	}
	return htons(nAcumulator) ;
}
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
uint8_t CDiverFrameParser_v1::sOH='\x01' ;
uint8_t CDiverFrameParser_v1::sTX='\x02' ;
uint8_t CDiverFrameParser_v1::eTX='\x03' ;

int CDiverFrameParser_v1::ParseRxMessage( uint8_t *p_buf, uint16_t &p_bufLen)
{
	if( !p_bufLen )
	{
		p_bufLen = m_cCmd = m_nCargoLen = 0 ;
		return 0;
	}

	//LOG("p_bufLen :%d", p_bufLen );
	if ( p_buf[0] != CDiverFrameParser_v1::sOH ) // SOH not the first char
	{
		void* rv = memchr( p_buf, CDiverFrameParser_v1::sOH, p_bufLen );
		if ( !rv )
		{
			//WARN("No CDiverFrameParser_v1::sOH Found. Drop whole buffer.");
			p_bufLen = m_cCmd = m_nCargoLen = 0 ;
			return 0 ;
		}
		int diff = (uint8_t*)rv - p_buf;
		if ( diff && p_bufLen > diff )
		{
			//LOG( "WARN: Drop the bytes found before CDiverFrameParser_v1::sOH:", p_buf, p_bufLen -diff);
			memmove( p_buf, p_buf+diff, p_bufLen - diff ) ;
			p_bufLen -= diff ;
		}
	}

	if (  p_buf[0] == CDiverFrameParser_v1::sOH  )
	{
		int windowBytes =5;// SOH CMD P1 P2 STX , where P1 P2 are command length parameters
		for ( int i=1; i < windowBytes; ++i )
		{
			if ( p_buf[i] == CDiverFrameParser_v1::sOH )
			{
				//LOG_HEX( "WARN: CDiverFrameParser_v1::sOH Again. Drop bytes before SOH", p_buf, i ) ;
				memmove( p_buf, p_buf+i, p_bufLen - i ) ;
				p_bufLen -=i;
				m_cCmd = m_nCargoLen =0;
				return 0 ;
			}
			if ( p_buf[i] == CDiverFrameParser_v1::sTX )
			{
				//LOG_HEX( "sTX found:", p_buf, p_bufLen );
				// Extract command name and payload length.
				m_cCmd = p_buf[1] ;
				m_nCargoLen = 0 ;
				if ( i != 2 )
					sscanf((const char*)p_buf+2, "%2x", &m_nCargoLen) ;

				windowBytes = i+1 ;
				for ( unsigned i2=i+1; i2 < p_bufLen; ++i2 )//i2=i+1 , skip over sTX
				{
					if ( m_nCargoLen + windowBytes <= i2
					&&   i2+2 <= p_bufLen
					&&   p_buf[i2] == CDiverFrameParser_v1::eTX
					   )
					{
						//LOG_HEX( "MSG_WAITING: ", p_buf, i2+2 ) ;
						return i2+2 ;
					}
				}
			}
		}
	}
	return m_cCmd = m_nCargoLen =0 ;
}

int CDiverFrameParser_v1::UnescapeMsg( uint8_t */*msg*/, int msgSz)
{
	return msgSz;
}
uint16_t CDiverFrameParser_v1::EscapeMsg( const uint8_t* msg, uint16_t msgSz, uint8_t* emsg )
{
	memcpy(emsg, msg,msgSz);
	return msgSz;
}

uint8_t CAmberFrameParser_v1::sOH='\x02' ;
uint8_t CAmberFrameParser_v1::sTX='\x02' ;
uint8_t CAmberFrameParser_v1::eTX='\x03' ;
int CAmberFrameParser_v1::ParseRxMessage( uint8_t *p_buf, uint16_t &p_bufLen)
{
	if( !p_bufLen )
	{
		p_bufLen = m_cCmd = m_nCargoLen = 0 ;
		return 0;
	}

	//LOG("p_bufLen :%d", p_bufLen );
	if ( p_buf[0] != CAmberFrameParser_v1::sOH ) // SOH not the first char
	{
		void* rv = memchr( p_buf, CAmberFrameParser_v1::sOH, p_bufLen );
		if ( !rv )
		{
			WARN("CAmberFrameParser_v1::sOH NOT found. Drop whole buffer.");
			return p_bufLen = m_cCmd = m_nCargoLen =0 ;
		}
		int diff = (uint8_t*)rv - p_buf;
		if ( diff && p_bufLen > diff )
		{
			WARN( "Drop the bytes found before CAmberFrameParser_v1::sOH:", p_buf, p_bufLen -diff);
			memmove( p_buf, p_buf+diff, p_bufLen - diff ) ;
			p_bufLen -= diff ;
		}
	}

	if (  p_buf[0] == CAmberFrameParser_v1::sOH  )
	{
		unsigned i ;
		// Extract command name and payload length.
		m_nCargoLen = p_buf[1] ;
		//LOG("CargoLen: %d", m_nCargoLen );
		if (!m_nCargoLen )
			return p_bufLen = m_cCmd = m_nCargoLen =0 ;

		//i2=i+1 , skip over sTX
		for ( i=2; i < p_bufLen && i<m_nCargoLen+3; ++i )
		{
		}
		//LOG("i:%d", i);
		if ( i == m_nCargoLen+3 )
		{
			/*
			LOG("TotalLen: %d, cargoLen: %d p_bufLen: %d i2:%d"
					,m_nCargoLen+3, m_nCargoLen, p_bufLen, i);
			LOG_HEX( "MSG_WAITING: ", p_buf, i ) ;
			*/
			return i ;
		}
	}
	return m_cCmd = m_nCargoLen =0 ;
}
/*int CDummyFrameParser::ParseRxMessage( uint8_t *msg, uint16_t & msgSz)
{
	return msgSz;
}
int CDummyFrameParser::UnescapeMsg( uint8_t *msg, int msgSz)
{
	return msgSz;
}
uint16_t CDummyFrameParser::EscapeMsg( const uint8_t* msg, uint16_t msgSz, uint8_t* emsg )
{
	memcpy(emsg, msg,msgSz);
	return msgSz;
}*/
