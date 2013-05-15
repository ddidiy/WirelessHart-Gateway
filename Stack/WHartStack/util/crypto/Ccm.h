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

#ifndef _CCM_H_
#define _CCM_H_

//////////////////////////////////////////////////////////////////////////////
/// @file	Ccm.h
/// @author	Marius Negreanu
/// @date	Wed Nov 28 12:26:59 2007 UTC
/// @brief	Counter with CBC-MAC ( RFC 3610 - read before seeing the code) (Ccm).
//////////////////////////////////////////////////////////////////////////////

#include "../../WHartTypes.h"

#define AES_BLOCK_SZ 16

namespace stack_crypto {

	typedef std::basic_string<uint8_t> Bytes;
	//typedef boost::shared_ptr<Bytes> BytesPointer;

	//////////////////////////////////////////////////////////////////////////////
	/// @class Ccm
	/// @brief Counter with CBC-MAC ( RFC 3610 - read before seeing the code).
	//////////////////////////////////////////////////////////////////////////////
	class Ccm {
		friend class TCcm ;///< The UnitTesting class.
	public:
		Ccm() ;
		~Ccm();

	public:
		uint16_t AuthEncrypt( const uint8_t *p_key, const uint8_t *p_nonce
					, const uint8_t *p_aad, uint16_t p_aadSz
					, const uint8_t *p_msg, uint16_t p_msgSz
					, uint8_t *p_out) ;
		uint16_t CheckAuthDecrypt( const uint8_t* p_key, const uint8_t* p_nonce,
									const uint8_t* p_aad, uint16_t p_aadSz,
									const uint8_t* p_msg, uint16_t p_msgSz,
									uint8_t* p_out) ;
		void SetAuthenFldSz( uint16_t p_authFldSz ) { m_uiAuthFldSz = p_authFldSz ; }
		void SetLenFldSz( uint16_t p_lenFldSz ) { m_uiLenFldSz = p_lenFldSz ; }
		const uint8_t* GetAuthTag(void) const	{ return m_rguiT; }
		const uint8_t* GetEncryptMic(void) const{ return m_rguiU; }

	private:
		void	pushBlock0( uint16_t p_aadSz, uint16_t p_msgSz ) ;
		void	pushAad( const uint8_t* p_aad, uint16_t p_aadSz ) ;
		void	pushMsg( const uint8_t* p_msg, uint16_t p_msgSz ) ;
		void	cbcMac( void );
		void	saltCtr( const uint8_t* p_msg, uint16_t p_msgSz) ;

	private:
		uint8_t		m_rguiT[AES_BLOCK_SZ] ;
		uint8_t		m_rguiU[AES_BLOCK_SZ] ;
		uint32_t	m_rguiKeyRk[60] ;
		int		m_iKeyNr ;
		const uint8_t*	m_puiKey ;
		const uint8_t*	m_puiNonce ;
		uint8_t*	m_prguiWorkBuf ;
		uint16_t	m_uiWorkBufSz ;
		uint16_t	m_uiWorkBufIdx ;
		uint16_t	m_uiAuthFldSz ;
		uint16_t	m_uiLenFldSz ;
		uint16_t	m_uiNonceSz ;
		uint8_t		m_uiFlags ;
		uint16_t	m_uiMsgOffset ;
	};

} // namespace stack_crypto

#endif	//_CCM_H_
