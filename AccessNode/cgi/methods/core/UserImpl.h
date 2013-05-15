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

#ifndef _USER_MET_IMPL_H_
#define _USER_MET_IMPL_H_

#include "IO.h"

//////////////////////////////////////////////////////////////////////
/// @class CUserImpl
//////////////////////////////////////////////////////////////////////
class CUserImpl : public CIniParser
{
public:
	CUserImpl():m_DBLoaded(false) { }
public:
	bool login( IO& io, const char * p_kszUser, const char * p_kszPass) ;
	bool logout( IO& io ) ;
	bool update( IO& io, const char * p_kszPass) ;
	bool reset_password( IO& io, const char * p_kszUser, const char * p_kszSerialNo ) ;
protected:
	bool load(void) ;
	bool isValidSerial(const char *p_kszSerialNo ) ;
protected:
	bool m_DBLoaded ;
};
#endif	/* _USER_MET_IMPL_H_ */
