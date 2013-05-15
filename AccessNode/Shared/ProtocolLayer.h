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

#ifndef _PROTOCOL_LAYER_H_
#define _PROTOCOL_LAYER_H_

/// @addtogroup libshared
/// @{

#include "Shared/ProtocolPacket.h"

struct ControlBuffer ;

enum LAYER_TYPE
{
	ENCASE_LAYER,
	DATA_LAYER,
	NETWORK_LAYER,
	TRANSPORT_LAYER,
	APPLICATION_LAYER
} ;



/**
 * @brief Base class for Protocol Layers.
 */
class ProtocolLayer
{
public:
	explicit ProtocolLayer(LAYER_TYPE lt)
		: m_above(0)
		, m_below(0)
		, m_layerID(lt)	{ }
	virtual ~ProtocolLayer()  { }

public:
	virtual ProtocolPacket* Read( void )  =0;			//! Application Layer Read.
	//virtual ProtocolPacket* Write( const uint8_t* msg, uint16_t msgSz ) =0;	//! Application Layer Write.

	inline ProtocolLayer* SelectLayer( LAYER_TYPE lt, ProtocolPacket* pkt ) ;
	inline  void SetAbove(ProtocolLayer* l) { m_above=l; }
	inline  void SetBelow(ProtocolLayer* l) { m_below=l; }

	inline  ProtocolLayer* GetAbove() const { return m_above ; }
	inline  ProtocolLayer* GetBelow() const { return m_below ; }
	//virtual int Write( ProtocolPacket* pkt )  =0;
	virtual inline ProtocolPacket* Read( ProtocolPacket* pkt )   ;	// Read and strip Layer infos.
	virtual int SendTo( const char *p_szIP, int p_iRemotePort, const uint8_t *p_rguiMsg, uint16_t p_uiMsgSz)
		{
			LOG("Base Method. SHOULD NOT BE CALLED");
			return false;
		};

protected:
	ProtocolLayer* m_above ;	// The Layer above this one
	ProtocolLayer* m_below ;	// The Layer below this one
protected:
	LAYER_TYPE m_layerID ;

protected:
	//! The Layers that pass ProtocolPackets between them are of the
	//! same type, so there is no need to make these methods public.
};


/* INLINE METHODS */
ProtocolLayer* ProtocolLayer::SelectLayer( LAYER_TYPE lt, ProtocolPacket* pkt )
{
	//! this is the layer to select. No changes are made on the Packet.
	if( m_layerID == lt )
		return this ;

//	if( ! m_above )
//		return this ;

	//! If above is not set, then don't change the Packet.
	//! This is the case when the last layer is not the application,
	//! And further operations will be done with the data of this
	//! layer.
	if( m_above && m_above->m_above==0 )
		return m_above ;

	//! This is the case of the upper most layer. It will strip the infos
	//! from the packet, and return;
	if( m_above == this )
	{
		if ( ! Read( pkt ) ) {
			ERR("ProtocolLayer::SelectLayer");
			return 0 ;
		}
		return this ;
	}
	if ( ! Read( pkt ) ) {
		ERR("ProtocolLayer::SelectLayer final.");
		return 0 ;
	}
	return m_above->SelectLayer( lt, pkt ) ;
}

ProtocolPacket* ProtocolLayer::Read( ProtocolPacket* pkt )
{
	if( ! m_above )
	{
		INFO("Last layer.");
		return pkt ;
	}
	assert( m_above ) ;
	return m_above->Read(pkt);
}
/*
ProtocolPacket*
ProtocolLayer::Write( const uint8_t* msg, uint16_t msgSz )
{

	ProtocolPacket* pkt = new ProtocolPacket( ) ;
	if ( !pkt ) {
		ERR("Unable to allocate space");
		return 0 ;
	}
	if ( ! pkt->AllocateCopy( msg, msgSz)  )
	{
		ERR("NOT GOOD");
		delete pkt ;
		return 0 ;
	}
	assert( m_below ) ;
	m_below->Write(pkt);
	return pkt ;
}
*/
/// @}
#endif	// _PROTOCOL_LAYER_H_
