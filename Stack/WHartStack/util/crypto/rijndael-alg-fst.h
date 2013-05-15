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

/**
 * rijndael-alg-fst.h
 *
 * @version 3.0 (December 2000)
 *
 * Optimised ANSI C code for the Rijndael cipher (now AES)
 *
 * @author Vincent Rijmen <vincent.rijmen@esat.kuleuven.ac.be>
 * @author Antoon Bosselaers <antoon.bosselaers@esat.kuleuven.ac.be>
 * @author Paulo Barreto <paulo.barreto@terra.com.br>
 *
 * This code is hereby placed in the public domain.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ''AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __RIJNDAEL_ALG_FST_H
#define __RIJNDAEL_ALG_FST_H

//#ifdef __cplusplus
//  extern "C" {
//#endif

#include "../../WHartTypes.h"

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;


#define MAXKC	(256/32)
#define MAXKB	(256/8)
#define MAXNR	14

        int rijndaelKeySetupEnc(u32 rk[/*4*(Nr + 1)*/], const u8 cipherKey[],
                                int keyBits);
        int rijndaelKeySetupDec(u32 rk[/*4*(Nr + 1)*/], const u8 cipherKey[],
                                int keyBits);
        void rijndaelEncrypt(const u32 rk[/*4*(Nr + 1)*/], int Nr,
                             const u8 pt[16], u8 ct[16]);
        void rijndaelDecrypt(const u32 rk[/*4*(Nr + 1)*/], int Nr,
                             const u8 ct[16], u8 pt[16]);

#ifdef INTERMEDIATE_VALUE_KAT
    void rijndaelEncryptRound(const u32 rk[/*4*(Nr + 1)*/], int Nr, u8 block[16], int rounds);
    void rijndaelDecryptRound(const u32 rk[/*4*(Nr + 1)*/], int Nr, u8 block[16], int rounds);
#endif /* INTERMEDIATE_VALUE_KAT */

//#ifdef __cplusplus
// }
//#endif

#endif /* __RIJNDAEL_ALG_FST_H */
