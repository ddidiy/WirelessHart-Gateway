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

#ifndef _SM_IMPL_H_
#define _SM_IMPL_H_

#include <vector>

//////////////////////////////////////////////////////////////////////
/// @class CIsa100Impl
//////////////////////////////////////////////////////////////////////

class SmImpl
{

public:
	bool setDevice( const char * p_szDevice, const char * p_szDeviceType, int p_nNetworkType );
	bool delDevice( const char * p_szDevice, const char * p_szDeviceType, int p_nNetworkType );
	const char *getLogLevel( void ) ;
	bool setLogLevel( const char * p_szLogLevel ) ;

private:
	bool isLocked( const char* p_szProcessName, unsigned p_unstaleLimit = 90 );
} ;
#endif	//_SM_IMPL_H_
