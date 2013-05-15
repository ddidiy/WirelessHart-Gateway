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

#include "ProtocolPacket.h"
#include <cstring>
#include <cstdio>

ProtocolPacket::ProtocolPacket(  )
	: m_pBuf(0)
	, m_pHead(0)
	, m_pTail(0)
	, m_cargoOff(0)
	, m_bufSz(0)
	, m_tailSpare(0)
	, m_eType(0)
	, m_status(0)
{
}




ProtocolPacket::~ProtocolPacket()
{
	delete[] m_pBuf ;
}



bool ProtocolPacket::PushFront( const uint8_t* in, uint16_t size)
{
	uint8_t * pNewHead = m_pHead - size;

	// There is no space to go up.
	if ( pNewHead < m_pBuf )
	{
		ERR("ProtocolPacket.PushFront:pushSize[%u] > sizeAvailable[%d], (m_bufSz[%d])",
					  size ,  m_pHead-m_pBuf, m_bufSz ) ;
		return false ;
	}
	m_pHead = pNewHead ;
	memcpy( pNewHead, in, size);

	//LOG_HEX( "ProtocolPacket.PushFront:", in, size);
	return true ;
}




bool ProtocolPacket::PopFront( void* out, uint16_t size)
{
	uint8_t * pNewHead = m_pHead + size;
	if ( pNewHead > m_pTail ) {
		ERR("ProtocolPacket.PopFront: size_popped [%d] > size_available [%d] (m_bufSz[%d])",
				size , m_pBuf+m_bufSz-m_pTail,  m_bufSz ) ;
		return false ;
	}

	char fmt[128];
	snprintf(fmt, 128, "ProtocolPacket.PopFront size=(%d) RemainingCargo=", size );

	if ( !out )
	{
		m_pHead = pNewHead;
		//LOG_HEX( fmt, m_pHead, m_pTail-m_pHead ) ;
		return true ;
	}
	memcpy(out, m_pHead, size) ;
	m_pHead = pNewHead ;

	//LOG_HEX( fmt, (unsigned char*)out, size);
	return true ;
}



bool ProtocolPacket::PushBack( const void* in, uint16_t size)
{
	uint8_t * pNewTail = m_pTail + size;
	// There is no space to go down.
	if ( pNewTail > m_pBuf+m_bufSz )
	{
		ERR("ProtocolPacket.PushBack: size_pushed [%d] > size_available [%d] (m_bufSz[%d])",
				size , m_pBuf+m_bufSz-m_pTail,  m_bufSz ) ;
		return false ;
	}

	memcpy(m_pTail, in, size);
	m_pTail = pNewTail ;

	//LOG_HEX( "ProtocolPacket.PushBack:", (unsigned char*)in, size);
	return true ;
}



bool ProtocolPacket::PopBack( void* out, uint16_t size)
{
	if ( !size ) return true ;
	uint8_t * pNewTail = m_pTail - size;
	if ( m_pHead > pNewTail ) {
		ERR("ProtocolPacket.PopBack: size_popped [%d] > size_available [%d] (m_bufSz[%d])",
				size , m_pBuf+m_bufSz-m_pTail,  m_bufSz ) ;
		return false ;
	}

	m_pTail = pNewTail ;
	 //modify in conjunction with size
	if ( ! out ) return true ;

	memcpy(out, m_pTail, size) ;

	//LOG_HEX( "ProtocolPacket.PopBack :", (unsigned char*)out, size);
	return 0;
}




//! Sets the initial payload of the packet, from the application layer
//! Each layer will add specific infos.
bool ProtocolPacket::AllocateCopy( const uint8_t* msg, uint16_t size, uint16_t headSpare, uint16_t tailSpare )
{
	if( m_cargoOff > size )
	{
		ERR( "ProtocolPacket.AllocateCopy:m_cargoOff(%d) > size (%d)", m_cargoOff, size );
		return false;
	}

	m_bufSz = headSpare + size + tailSpare ;

	m_pBuf = new uint8_t[ m_bufSz ] ;

	m_pTail = m_pBuf + size + headSpare;
	if ( ! msg )
	{
		m_pHead = m_pTail ;
		return true ;
	}
	m_pHead = m_pBuf + headSpare + m_cargoOff ;
	memcpy( m_pHead, msg, size ) ;
	return true ;
}




//! Sets the initial buffer, containing the whole packet.
bool ProtocolPacket::SetIn( uint8_t* buf, uint16_t cargoSize, uint16_t headSpare, uint16_t tailSpare )
{
	assert( buf );

	m_pBuf      = buf ;
	m_bufSz     = headSpare + cargoSize + tailSpare ;
	m_pHead     = m_pBuf + headSpare ;
	m_pTail     = m_pHead + cargoSize ;
	return true ;
}




uint8_t* ProtocolPacket::Data(unsigned int p_nOffset/*=0*/) const
{
	return m_pHead + p_nOffset ;
}




uint32_t ProtocolPacket::DataLen() const
{
	return m_pTail-m_pHead ;
}

