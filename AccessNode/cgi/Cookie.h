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

/////////////////////////////////////////////////////////////////////////////
/// @file       Cookie.hpp
/// @author     Marius Negreanu
/// @date       Tue Sep  2 16:45:54 EEST 2008
/// @brief      Cookie parser/getter.
/// $Id: Cookie.h,v 1.9.18.1 2013/05/15 19:19:17 owlman Exp $
/////////////////////////////////////////////////////////////////////////////


#ifndef _COOKIE_H_
#define _COOKIE_H_

/////////////////////////////////////////////////////////////////////////////
/// @class CCookie Parses a string containing a cookie header line.
/////////////////////////////////////////////////////////////////////////////
class CCookie
{
public:
	CCookie()
			: m_szCookies(0)
	{}
	~CCookie() ;
public:
	/// @struct CCookie::Value Structure describing a Cookie.
	struct Value
	{
		char	*version,
		*name,
		*value,
		*path,
		*domain;
	} ;
	/// @brief Parse cookie_string and import the key=value.
	/// @param [in] cookie_string The string containing cookie header.
	/// @retval true Cookie header parsed and imported ok.
	/// @retval false Unable to parse cookie header.
	bool ReadCookies( const char *cookie_string) ;

	/// @brief Get the value of a cookie.
	/// @param [in] name Cookie key.
	/// @retval NULL Cookie key not found.
	/// @retval Value Value coresponding to Cookie key.
	Value *GetCookie (const char *name) ;

	/// @brief Get the value of a cookie.
	/// @retval NULL No cookies were imported.
	/// @return An array of cookies.
	char **GetCookies() ;
private:
	CCookie::Value **m_szCookies ;
};


#endif	/* _COOKIE_H_ */
