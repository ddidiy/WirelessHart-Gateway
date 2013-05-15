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

#ifndef _PROTOCOL_PAKET_H_
#define _PROTOCOL_PAKET_H_

#include "h.h"

/**
 * @brief Protocol Packet Buffer.
 * Implement a sk_buf/pbuf concept.
 * See: Network Buffers and Memory Management. [http://www.linuxjournal.com/article/1312].
 * @ingroup libshared
 **/
class ProtocolPacket
{
public:
	ProtocolPacket() ;

	~ProtocolPacket() ;

public:
	template<typename T1>T1	Checksum() ;

	template<typename T1> bool PushFront( T1 in) ;
	bool	PushFront( const uint8_t* in, uint16_t size) ;

	template<typename T1> bool PushBack( T1 in) ;
	bool	PushBack( const void* in, uint16_t size) ;
	ProtocolPacket &operator<<(uint8_t in)
	{
		this->PushBack(in);
		return *this;
	}
	uint8_t &operator[](int idx)
	{
		return (this->m_pHead[idx]);
	}

	bool	PopFront( void* out, uint16_t size) ;
	bool	PopBack( void* out, uint16_t size) ;

	//! Sets the initial payload of the packet, from the application layer
	bool	AllocateCopy( const uint8_t* msg=0, uint16_t size=0, uint16_t headSpare=0, uint16_t tailSpare=0 ) ;

	//! Sets the initial buffer, containing the whole packet.
	bool	SetIn( uint8_t* msg, uint16_t cargoSize, uint16_t headSpare=0, uint16_t tailSpare=0) ;

	uint8_t* Data(unsigned int p_nOffset=0) const ;
	uint32_t DataLen() const ;

	unsigned char Type() const { return m_eType ; }
	void Type(char type) { m_eType = type; }
	void Status( char status ) { m_status = status ; }
	int Status( ) const { return m_status ; }

private:
	uint8_t* m_pBuf;	//! Buffer where the packet will be built.
	uint8_t* m_pHead ;	//! Top Pointer
	uint8_t* m_pTail ;	//! Bottom Pointer
	uint32_t m_cargoOff ;	//! Cargo(application layer) offset.
	uint16_t m_bufSz;	//! The size of the packet buffer.
	uint16_t m_tailSpare ;
	unsigned int m_eType ;
	unsigned int m_status ;
} ;


// Used for implicit types.
template<typename T1>
bool ProtocolPacket::PushFront( T1 in)
{
	return PushFront( (uint8_t*)&in, sizeof(T1) );
}

// Used for implicit types.
template<typename T1>
bool ProtocolPacket::PushBack( T1 in)
{
	return PushBack( (uint8_t*)&in, sizeof(T1) ) ;
}

template<typename T1>
T1 ProtocolPacket::Checksum(void)
{
	T1 check=0;
	uint8_t *it;
	for ( it=m_pHead; it < m_pTail; ++it )
	{
		check += *it ;
	}
	return check;
}

#endif	//_PROTOCOL_PAKET_H_
