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

#ifndef _STROP_H_

#define _STROP_H_

#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>



/* Removes all leading/ending space
 * Removes all leading/ending double quotes
 * Unescape the string */
inline char * safeStr( char* str)
{
	size_t origLen,len ;
	do {
		len = strlen( str ) ;
		origLen = len ;
		if (len < 2 )
			return str;

		char * begin=str;
		char * end = str+len-1/*ending zero*/ ;

		while ( begin && *begin=='"' ) begin++;
		while ( end && *end=='"' ) end--;
		len = end - begin+1 ;

		while ( begin && isspace(*begin) && begin!=end ) begin++ /*skip over leading spaces*/ ;
		while ( end && isspace(*end) && end != begin ) end-- /*skip over ending spaces*/ ;
		len = end - begin +1;

		if ( len )
		{
			memmove( str, begin, len );
			str[len] = '\0';
		}

		for (begin = end = str ; *begin!='\0';begin++, end++)
		{
			*begin = *end ;
			if ( *end == '\\' ) ++end;
		}
	} while ( origLen != len ) ;
	return str ;
}

#endif /* _STROP_H_ */

