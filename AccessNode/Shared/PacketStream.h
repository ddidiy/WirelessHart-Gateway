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

#ifndef _PACKET_STREAM_H_
#define _PACKET_STREAM_H_
/// @addtogroup libshared
/// @{

#include "StreamLink.h"
#include "ProtocolPacket.h"

#define READ_TIMEOUT 256
#define READ_OK      257

///////////////////////////////////////////////////////////////////////////////
/// @class CPacketStream
/// @brief Exposes a single interface to TtyLink, FileLink and UdpLink.
///////////////////////////////////////////////////////////////////////////////
class CPacketStream
{
public:
	CPacketStream( bool rawLog=false, bool p_bUseEscape=true ) ;
	virtual ~CPacketStream()
	{
		delete m_poBaseLink ;
		delete m_poParse ;
	}
public:
	template<class LinkClass, class ParseClass >
	void		Create( )
	{
		Create<LinkClass>() ;
		m_poParse= new ParseClass();
	}
	template<class LinkClass >
	void		Create( )
	{
		m_poBaseLink = new LinkClass(m_bRawLog) ;
		m_poBaseLink->SetRawLog(m_bRawLog);
	}
	// The link is used/created elsewhere
	// TODO: if m_poBaseLink is modified using this method
	// then don't delete it in destructor. FTM used for
	// MOCKUP testing only
	void		Create(CStreamLink* p_pLink )
	{
		m_poBaseLink = p_pLink ;
		m_poBaseLink->SetRawLog(m_bRawLog);
	}
	void SetRawLog(bool pRawLog) { m_bRawLog = pRawLog; }
	operator CUdpLink*() { return (CUdpLink*)m_poBaseLink ; }

	int	OpenLink( char const* p_sConn, unsigned int  p_nConnParam, unsigned int p_nConnParam2 = 0  ) ;
	virtual void	SetTimeout(int tout) { m_nTimeout = tout ;  }
	void	CloseLink( ) ;
	int		GetMsgLen( int tout=100000 ) ;
	void	SetOffsets( uint16_t p_uiHeadSpare , uint16_t p_uiTailSpare) ;
	virtual ProtocolPacket* Read( int* status=NULL ) ;
	virtual ProtocolPacket* Write( const uint8_t* p_prguiMsg, uint16_t p_uiMsgSz ) ;
	virtual int	Write( ProtocolPacket* pkt ) ;
	int reopenLink() ;
protected:
	bool		m_bRawLog ;	///< Do Raw Logging.

private:
	time_t		m_nTimeout ;
	uint16_t	m_uiHeadSpare ;	///< A ProtocolPacket head spare.
	uint16_t	m_uiTailSpare;	///< A ProtocolPacket tail spare.
	bool		m_bEscMsg ;	///< Do character escaping.
	CStreamLink*	m_poBaseLink ;
	uint16_t	m_bufLen ;
	unsigned char	m_buf[512];
	char const*	m_sConn ;
	int	m_nConnParam ;
	int	m_nConnParam2 ;
protected:
	CFrameParser*	m_poParse ;
};

/// @}
#endif	//_PACKET_STREAM_H_

