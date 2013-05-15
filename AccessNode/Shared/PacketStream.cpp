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

#include "PacketStream.h"
CPacketStream::CPacketStream( bool rawLog/*=false*/, bool p_bUseEscape/*=true*/ )
	: m_bRawLog(rawLog)
	, m_nTimeout(0)
	, m_uiHeadSpare(0)
	, m_uiTailSpare(0)
	, m_bEscMsg(p_bUseEscape)
	, m_poBaseLink(NULL)
//	, m_bufLen(512)
	, m_bufLen(0)
	, m_poParse(NULL)
{
	memset(m_buf,0,m_bufLen);
}
int	CPacketStream::OpenLink( char const* p_sConn, unsigned int  p_nConnParam, unsigned int p_nConnParam2/*=0*/  )
{
	if ( m_bRawLog )
		LOG("Conn:%s ConnParam:%d ConnParam2:%d", p_sConn, p_nConnParam, p_nConnParam2 );
	m_sConn = p_sConn ;
	m_nConnParam = p_nConnParam ;
	m_nConnParam2 = p_nConnParam2 ;
	return m_poBaseLink->OpenLink( m_sConn, m_nConnParam, m_nConnParam2);
}
int CPacketStream::reopenLink()
{
	while ( true ) {
		m_poBaseLink->CloseLink() ;
		if ( m_poBaseLink->OpenLink( m_sConn, m_nConnParam, m_nConnParam2) )
			return true ;
		sleep( 5 ) ;
	}
	//return true ; Never executed
}
void	CPacketStream::CloseLink( )
{
	if (m_poBaseLink && m_poBaseLink->IsLinkOpen() )
		m_poBaseLink->CloseLink();
}

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
void CPacketStream::SetOffsets( uint16_t p_uiHeadSpare , uint16_t p_uiTailSpare)
{
	m_uiHeadSpare = p_uiHeadSpare ;
	m_uiTailSpare = p_uiTailSpare ;
}


///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
int CPacketStream::GetMsgLen( int /*tout=1000*/ )
{
	if ( m_bEscMsg )
	{
		assert(m_poParse!=NULL) ;
		int length=m_poParse->ParseRxMessage( m_buf, m_bufLen ) ;
		if ( length > 0 ) return length ;
	}

	if ( m_bufLen >= sizeof(m_buf) )
	{
		//The buffer is full, and no ETX was received.
		//Drop the whole buffer and go on reading.
		LOG("Buffer Full:m_bufLen[%u]>=sizeof(m_buf)[%u]", m_bufLen, sizeof(m_buf) );
		m_bufLen = 0 ;
	}
	assert( m_poBaseLink );
	if ( ! m_poBaseLink->IsLinkOpen() )
	{
		ERR("Attempt to read from un-oppened link");
		return 0 ;
	}
	int rv = m_poBaseLink->Read( m_buf + m_bufLen, sizeof(m_buf) - m_bufLen ) ;
	if ( ! rv && errno==EINTR ) { errno=0 ; return -1; }
	if ( ! rv ) return 0;// nothing read
	//if ( ! rv ) {LOG("nothing read");return 0;}// nothing read
	if ( rv < 0 )
	{
		ERR("CTtyLink.GetMsgLen: readBlock() error. ReOpening Link.");
		/// Usb disconnected. Try to reopen link.
		reopenLink() ;
		return 0;
	}
	m_bufLen += rv ;
	//LOG("bufLen=%d", m_bufLen );
	int retval =0;
	if ( m_bEscMsg )
	{
		assert(m_poParse!=NULL);
		retval = m_poParse->ParseRxMessage( m_buf, m_bufLen ) ;
	}
	else
		retval = m_bufLen ;
	return retval ;
}


///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
ProtocolPacket* CPacketStream::Read( int *status )
{
	int rb =0;
	if ( m_nTimeout ) {
		time_t exp=time(NULL) + m_nTimeout;
		for(;;) {
			rb = GetMsgLen() ;
			//LOG("rb:%d %s", rb, strerror(errno) );
			if ( rb > 0 ) { *status = !READ_TIMEOUT ; break ; }
			if ( rb < 0 ) { LOG("Serial Link ERROR"); return NULL ; }
			//if ( rb==0 && errno==EINTR) { LOG("EINTR"); break; }
			time_t now = time(NULL);
			if ( now >= exp) {
				LOG("CPacketStream.Read: EXPIRED: now(%d) >= exp(%d) timeout(%d)",now, exp, m_nTimeout ) ;
				if ( status ) *status = READ_TIMEOUT ;
				return NULL ;
			}
		}
	}else if( 0 >= (rb=GetMsgLen()) ) {
		return NULL ;
	}

	uint8_t*buf = new uint8_t[m_uiHeadSpare+rb +m_uiTailSpare ] ;
	memcpy( buf+m_uiHeadSpare, m_buf, rb ) ;
	memmove( m_buf, m_buf+rb, m_bufLen -rb ) ;
	//LOG("m_bufLen:%d rb:%d", m_bufLen, rb);
	m_bufLen -= rb ;
	uint16_t nPkLen = rb ;

	if ( m_bEscMsg ) {
		nPkLen = m_poParse->UnescapeMsg(buf+m_uiHeadSpare, rb ) ;
	}
	ProtocolPacket* pkt = new ProtocolPacket( ) ;
	if( ! pkt->SetIn( buf, nPkLen, m_uiHeadSpare, m_uiTailSpare) ) {
		delete pkt ;
		delete[] buf ;
		return NULL ;
	}

	if ( status ) *status = READ_OK ;
	return pkt;
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
int CPacketStream::Write( ProtocolPacket* pkt )
{
	//LOG("CPacketStream::Write_1");
	if ( ! m_bEscMsg )
		return m_poBaseLink->Write( pkt->Data(), pkt->DataLen() );

	uint8_t* emsg = new uint8_t [2*pkt->DataLen() + 2] ;	// allow for space for STX & ETX in worst case scenario
	uint16_t emsgSz = m_poParse->EscapeMsg( pkt->Data(), pkt->DataLen(), emsg);
	int rv = m_poBaseLink->Write( emsg, emsgSz );
	delete[] emsg ;
	return rv ;
}


///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
ProtocolPacket* CPacketStream::Write( const uint8_t* p_rguiMsg, uint16_t p_uiMsgSz )
{
	ProtocolPacket* pkt = new ProtocolPacket( ) ;
	pkt->AllocateCopy( p_rguiMsg, p_uiMsgSz);
	Write(pkt);
	return pkt ;
}

