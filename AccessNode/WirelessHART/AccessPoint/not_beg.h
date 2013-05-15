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


#define notation_constchar(a,...)    m_kc##a
#define notation_char(a,...)         m_c##a
#define notation_unsignedchar(a,...) m_uc##a
#define notation_uint8_t(a,...)      m_ui8##a
#define notation_uint16_t(a,...)     m_ui16##a

#define unsigned unsigned,
#define const const,
#define char char,
#define uint8_t uint8_t,
#define uint16_t uint16_t,

#define P(a,b,...) a b
#define M(a,b,...) notation_##a##b

#define DECLARE(a,...) P(__VA_ARGS__)  M(__VA_ARGS__)(a)

#define GETSET(var,type) \
   P(type) Get##var() { return M(type)(var); } ; \
   P(type) Get##var##NtoH() { return ntohs( M(type)(var) ) ; } ; \
   void Set##var( P(type) _p ) { M(type)(var) = _p ; } ; \
   void Set##var##HtoN( P(type) _p ) { M(type)(var) = htons( _p  ) ; }
