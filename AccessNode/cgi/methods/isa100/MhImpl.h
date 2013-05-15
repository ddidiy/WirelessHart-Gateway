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

#ifndef _MH_IMPL_H_
#define _MH_IMPL_H_

#include <vector>

typedef std::vector<const char*> ChannelsVector;

//////////////////////////////////////////////////////////////////////
/// @class CIsa100Impl
//////////////////////////////////////////////////////////////////////

class MhImpl
{

public:
	MhImpl() {}
	~MhImpl() {}

#if defined( RELEASE_ISA )
	bool setPublisher(	const char * p_szEUI64, const char * p_szConcentrator,
						const ChannelsVector * p_pVectorChannels);
#elif defined( RELEASE_WHART )
	bool setPublisher(  const char * p_szEUI64, const char * p_szBurstMessage,
						const ChannelsVector * p_pVectorVariables,
						const char * p_szTrigger);
#endif

#if defined( RELEASE_ISA )
	bool delPublisher( const char * p_szEUI64 );
#elif defined( RELEASE_WHART )
	bool delPublisher( const char * p_szEUI64, const char * p_szBurst );
#endif
} ;
#endif	//_MH_IMPL_H_
