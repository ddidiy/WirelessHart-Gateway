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

#ifndef UtilsSolo_h__
#define UtilsSolo_h__

/// @addtogroup libshared
/// @{

template <class T>
inline const char * GetHexT( const T* p_Buff, char p_cByteDelim = -1 ) { return GetHex(p_Buff, sizeof(T), p_cByteDelim); }

const char * GetHex( const void* p_vBuff, int p_nLen, int p_cByteDelim = -1 );


const char * GetHexC( const void* p_vBuff, int p_nLen);


#define GET_PROCESS_LOAD_ALWAYS			0
#define GET_PROCESS_LOAD_AT_ONE_MIN		1

int GetProcessLoad (double& p_rdLoadMin1, double& p_rdLoadMin5, double& p_rdLoadMin15, double& p_rLoadTotal, int p_nWhen = GET_PROCESS_LOAD_ALWAYS );

int InterfaceIndex (const char *interface);
/// @}

#endif // UtilsC_h__
