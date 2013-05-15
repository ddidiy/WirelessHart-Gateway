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

//////////////////////////////////////////////////////////////////////////////
/// @file	Ccm.cpp
/// @author	Marius Negreanu
/// @date	Wed Nov 28 12:26:59 2007 UTC
/// @brief	Counter with CBC-MAC ( RFC 3610 - read before seeing the code).
//////////////////////////////////////////////////////////////////////////////


#include <WHartStack/util/crypto/Ccm.h>
#include <WHartStack/util/crypto/rijndael-alg-fst.h>

#include <string.h>
#include <cassert>

#define AAD_MSG_SZ 2
#define AES_ENCRYPT_BITS 128
#define ROUND_UP_TO_MUL_16(size) ((size)+(AES_BLOCK_SZ-(size)%AES_BLOCK_SZ)%AES_BLOCK_SZ)
#define blockPrint(...)
//static void blockPrint( const uint8_t *blk, int sz, const char* format, ...);


//   Name	Description				Size
//   ------------------------------------------------------------
//   M	`authentication field` size		3 bits
//   L	`length field` size			3 bits
//   K	Block cypher KEY			16 octets
//   ------------------------------------------------------------
//   N	Nonce					15-L octets
//   ------------------------------------------------------------
//   m	Message to be authenticate and encrypt	l(m) octets
//   a	Additional `authenticated data`		l(a) octets
//   ------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
stack_crypto::Ccm::Ccm(  )
	: m_iKeyNr(0)
	, m_puiKey(0)
	, m_puiNonce(0)
	, m_prguiWorkBuf(0)
	, m_uiWorkBufSz(0)
	, m_uiWorkBufIdx(0)
	, m_uiAuthFldSz(0)
	, m_uiLenFldSz(0)
	, m_uiNonceSz(0)
	, m_uiFlags(0)
	, m_uiMsgOffset(0)
{
}

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
stack_crypto::Ccm::~Ccm()
{
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Encrypt in CCM.
/// @retval Size of out buffer.
/// @param [in] key AES key. It's presumed to be 16bytes long.
/// @param [in] nonce Message Nonce. Its size is computed relative to m_uiLenFldSz
/// @param [in] aad Message to be authenticated.
/// @param [in] aadSz aad size.
/// @param [in] msg Message to be encrypted.
/// @param [in] msgSz msg size.
/// @param [out] out Authenticated and encrypted message.
///////////////////////////////////////////////////////////////////////////////
uint16_t stack_crypto::Ccm::AuthEncrypt( const uint8_t * p_key, const uint8_t * p_nonce
		    , const uint8_t * p_aad, uint16_t p_aadSz
		    , const uint8_t * p_msg, uint16_t p_msgSz
		    , uint8_t * p_out)
{
	assert( m_uiAuthFldSz );
	assert( m_uiLenFldSz );
	assert( (p_aad && p_aadSz) || !p_aadSz );
	assert( (p_msg && p_msgSz) || !p_msgSz );
	/// make sure that p_aadSz < 2^16 - 2^8, otherwise the p_aadSz encoding fails
	assert ( p_aadSz < 0xFF00 );

	m_uiNonceSz = 15 - m_uiLenFldSz ;
	/// @todo remove the two members ( p_key, p_nonce)
	m_puiKey = p_key ;
	m_puiNonce = p_nonce ;

	m_iKeyNr = rijndaelKeySetupEnc( m_rguiKeyRk, m_puiKey, AES_ENCRYPT_BITS ) ;

	m_uiMsgOffset = AES_BLOCK_SZ				///Block zero
		+ ROUND_UP_TO_MUL_16(p_aadSz?AAD_MSG_SZ+p_aadSz:0);	///aadSzFld+p_aadSz

	m_uiWorkBufSz = m_uiMsgOffset
		+ ROUND_UP_TO_MUL_16(p_msgSz)	;	/// Message


	m_prguiWorkBuf = new uint8_t[m_uiWorkBufSz];
	memset( m_prguiWorkBuf, 0, m_uiWorkBufSz );
	pushBlock0( p_aadSz, p_msgSz ) ;
	pushAad( p_aad, p_aadSz ) ;
	pushMsg( p_msg, p_msgSz ) ;
	cbcMac( );
	saltCtr( p_msg, p_msgSz);

	if ( p_out )
	{
		if ( p_aadSz )
			memcpy( p_out, p_aad, p_aadSz );
		if ( p_msgSz )
			memcpy( p_out+p_aadSz, m_prguiWorkBuf+m_uiMsgOffset, p_msgSz );
		memcpy( p_out+p_aadSz+p_msgSz, m_rguiU, m_uiAuthFldSz );
	}
	memset(m_prguiWorkBuf,0,m_uiWorkBufSz);
	delete [] m_prguiWorkBuf ;
	return p_aadSz+p_msgSz+m_uiAuthFldSz ;
}


///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
uint16_t stack_crypto::Ccm::CheckAuthDecrypt( const uint8_t* p_key, const uint8_t* p_nonce,
				const uint8_t* p_aad, uint16_t p_aadSz,
				const uint8_t* p_encMsg, uint16_t p_encMsgSz,
				uint8_t* p_out)
{
	assert( m_uiAuthFldSz );
	assert( m_uiLenFldSz );
	assert( (p_aad && p_aadSz) || !p_aadSz );
	assert( p_encMsg && p_encMsgSz );///this contains the encrypted msg or the MIC or both; msg can't be null or zero length
	/// make sure that p_aadSz < 2^16 - 2^8, otherwise the p_aadSz encoding fails
	assert ( p_aadSz < 0xFF00 );
	assert( p_out && p_key && p_nonce );
	assert( p_encMsgSz >= m_uiAuthFldSz );///the encrypted buffer also contains the U at the end

	m_uiNonceSz = 15 - m_uiLenFldSz ;
	m_puiKey = p_key ;
	m_puiNonce = p_nonce ;
	p_encMsgSz -= m_uiAuthFldSz;

	/// Setup keys
	m_iKeyNr = rijndaelKeySetupEnc( m_rguiKeyRk, m_puiKey, AES_ENCRYPT_BITS ) ;

	/// prepare the work buffer:
	m_uiWorkBufIdx = 0 ;
	m_uiMsgOffset = AES_BLOCK_SZ				/// Block zero
		+ ROUND_UP_TO_MUL_16(p_aadSz?AAD_MSG_SZ+p_aadSz:0);/// aadSzFld +p_aadSz

	m_uiWorkBufSz = m_uiMsgOffset
		+ ROUND_UP_TO_MUL_16(p_encMsgSz);		/// Message

	m_prguiWorkBuf = new uint8_t[m_uiWorkBufSz];
	memset( m_prguiWorkBuf, 0, m_uiWorkBufSz );
	pushBlock0( p_aadSz, p_encMsgSz ) ;
	pushAad( p_aad, p_aadSz ) ;
	pushMsg( p_encMsg, p_encMsgSz ) ;

	/// decrypt the T and the message from p_encMsg
	memcpy( m_rguiT, p_encMsg+p_encMsgSz, m_uiAuthFldSz);
	saltCtr( p_encMsg, p_encMsgSz );
	cbcMac();/// decrypted T is now in m_rguiU
	blockPrint( m_rguiT, AES_BLOCK_SZ, "T:", m_uiAuthFldSz );
	blockPrint( m_rguiU, AES_BLOCK_SZ, "U:", m_uiAuthFldSz );

	/// recomputed T now in T, let's compare
	if ( memcmp( m_rguiT,  m_rguiU, m_uiAuthFldSz) ){
		///authentication error
		memset(m_prguiWorkBuf,0,m_uiWorkBufSz);
		delete [] m_prguiWorkBuf ;
		memset(m_rguiT, 0, sizeof(m_rguiT));
		memset(m_rguiU, 0, sizeof(m_rguiU));
		return 0;
	}
	/// authentication successful
	if ( p_encMsgSz )
		memcpy( p_out, m_prguiWorkBuf+m_uiMsgOffset, p_encMsgSz );
	memset(m_prguiWorkBuf,0,m_uiWorkBufSz);
	delete [] m_prguiWorkBuf ;
	return p_encMsgSz ;
}



///////////////////////////////////////////////////////////////////////////////
/// Flags|NonceN|length(message)
/// Flags = 64*Adata + 8*M' + L'
/// @param [in] msgSz Size of the Message to be encrypted.
/// @param [in] nonce The nonce.
///////////////////////////////////////////////////////////////////////////////
void stack_crypto::Ccm::pushBlock0( uint16_t p_aadSz, uint16_t p_msgSz )
{
	m_uiWorkBufIdx = 0 ;
	/// Push Flags
	m_prguiWorkBuf[m_uiWorkBufIdx++] = (p_aadSz?64:0) + 4*(m_uiAuthFldSz-2) + (m_uiLenFldSz-1);

	/// Push Nonce
	memcpy( m_prguiWorkBuf+m_uiWorkBufIdx, m_puiNonce, m_uiNonceSz ) ;
	m_uiWorkBufIdx +=m_uiNonceSz ;

	/// Push Message Size
	m_prguiWorkBuf[m_uiWorkBufIdx++] = (uint8_t) (p_msgSz >> 8 );
	m_prguiWorkBuf[m_uiWorkBufIdx++] = (uint8_t) (p_msgSz & 0xFF);
}


///////////////////////////////////////////////////////////////////////////////
/// @param [in] aad Additional Authenticated Data.
/// @param [in] aadSz The size of aad.
///////////////////////////////////////////////////////////////////////////////
void stack_crypto::Ccm::pushAad( const uint8_t* p_aad, uint16_t p_aadSz )
{
	if ( !p_aadSz ) return ;

	/// Encode the length
	m_prguiWorkBuf[m_uiWorkBufIdx++] = p_aadSz >> 8 ;
	m_prguiWorkBuf[m_uiWorkBufIdx++] = p_aadSz & 0x00FF;
	memcpy( m_prguiWorkBuf+m_uiWorkBufIdx, p_aad, p_aadSz) ;
	m_uiWorkBufIdx += p_aadSz ;

	/// Zero padd to a AES_BLOCK_SZ length
	m_uiWorkBufIdx = ROUND_UP_TO_MUL_16(m_uiWorkBufIdx) ;
}


///////////////////////////////////////////////////////////////////////////////
/// @brief Add 16byte chunks from the msg
/// @param [in] msg Message to be encrypted.
/// @param [in] msgSz The size of msg.
///////////////////////////////////////////////////////////////////////////////
void stack_crypto::Ccm::pushMsg( const uint8_t* p_msg, uint16_t p_msgSz )
{
	if ( !p_msgSz ) return ;

	memcpy( m_prguiWorkBuf+m_uiWorkBufIdx, p_msg, p_msgSz ) ;
	m_uiWorkBufIdx += p_msgSz ;

	/// Zero padd to a AES_BLOCK_SZ length
	m_uiWorkBufIdx = ROUND_UP_TO_MUL_16(m_uiWorkBufIdx) ;
}


///////////////////////////////////////////////////////////////////////////////
/// @brief Compute the authentication field (T)
///////////////////////////////////////////////////////////////////////////////
void stack_crypto::Ccm::cbcMac(void)
{
	uint8_t X_1[AES_BLOCK_SZ], X_n[AES_BLOCK_SZ];
	uint16_t blk_it=AES_BLOCK_SZ;

	/// X_1 := E( K, B_0 )
	blockPrint( m_prguiWorkBuf, AES_BLOCK_SZ, "\nCBC IV in: " );
	rijndaelEncrypt( m_rguiKeyRk, m_iKeyNr, m_prguiWorkBuf, X_1 ) ;
	blockPrint( X_1, AES_BLOCK_SZ, "CBC IV out:");

	/// X_i+1 := E( K, X_i XOR B_i )  for i=1, ..., n
	for ( ; blk_it < m_uiWorkBufSz ; blk_it+=AES_BLOCK_SZ )
	{
		blockPrint( m_prguiWorkBuf+blk_it, AES_BLOCK_SZ, "XOR      : " ) ;

		for ( uint8_t i=0; i < AES_BLOCK_SZ; ++i)
			X_1[i] ^= m_prguiWorkBuf[blk_it+i] ;

		blockPrint( X_1, AES_BLOCK_SZ, "After xor: " );
		rijndaelEncrypt( m_rguiKeyRk, m_iKeyNr, X_1, X_n ) ;
		memcpy(X_1, X_n, AES_BLOCK_SZ);
		blockPrint( X_1, AES_BLOCK_SZ, "After AES: " );
	}
	memcpy( m_rguiT, X_1, AES_BLOCK_SZ);
	blockPrint( m_rguiT, m_uiAuthFldSz, "CBC-MAC  : " );
}


///////////////////////////////////////////////////////////////////////////////
/// @brief Encrypt the authentication field and the message
/// @param [in] msg Message to be encrypted.
/// @param [in] msgSz The size of msg.
/// @todo we don't use msg here, remove it but take care of the decrypt!
///////////////////////////////////////////////////////////////////////////////
void stack_crypto::Ccm::saltCtr( const uint8_t* p_msg, uint16_t p_msgSz )
{
	uint8_t A[AES_BLOCK_SZ];
	uint8_t S[AES_BLOCK_SZ];
	uint16_t s_blk = 0; /// uint16 is ok, because we won't have 4096 blocks

	/// Prepare A_0
	/// octet 0	 : Flags
	A[0] = m_uiLenFldSz-1 ;
	/// Octet 1..15-L : Push Nonce
	memcpy(A+1, m_puiNonce, m_uiNonceSz ) ;
	A[15] = A[14] = (uint8_t) 0;

	/// Compute the authentication value U
	rijndaelEncrypt( m_rguiKeyRk, m_iKeyNr, A, S ) ;
	blockPrint( S, m_uiAuthFldSz, "CTR[MAC ]: "  );
	for ( int i=0; i< AES_BLOCK_SZ ; ++i)
		m_rguiU[i] = m_rguiT[i] ^ S[i];

	blockPrint( m_rguiU, m_uiAuthFldSz, "U        : "  );

	if ( !p_msgSz) return ;

	for ( uint16_t msgIdx=m_uiMsgOffset; msgIdx < m_uiMsgOffset+p_msgSz ; )
	{
		/// Prepare A_i
		++s_blk;
		/// octets 16-L..15: Counter i
		A[14] = (uint8_t) (s_blk >> 8 );
		A[15] = (uint8_t) (s_blk & 0xFF);

		/// S_i := E( K, A_i )
		rijndaelEncrypt( m_rguiKeyRk, m_iKeyNr, A, S ) ;
		blockPrint( S, AES_BLOCK_SZ, "CTR[%.4d]: ", s_blk );
		/// Loop through blocks bytes
		for ( int i=0; i<AES_BLOCK_SZ && msgIdx<m_uiMsgOffset+p_msgSz; ) {
			m_prguiWorkBuf[msgIdx++] ^= S[i++];
		}
	}
}



///////////////////////////////////////////////////////////////////////////////
/// @brief Prints a HEX dump of nbytes block.
/// @param [in] p Prefix to be printed before the HEX dump line.
/// @param [in] blk The block that is HEX printed.
/// @param [in] sz The size of block. Default= AES_BLOCK_SZ
///////////////////////////////////////////////////////////////////////////////
/*static void blockPrint( const uint8_t *blk, int sz, const char* format, ...)
{
	va_list args ;
	va_start(args, format);
	vprintf(format, args) ;
	va_end(args);
	for ( int i =0; i< sz ; ++i )
		printf( "%.2X%s", blk[i], (!((i+1)%4)?"  ":" ") ) ;
	printf("\n");
}
*/

