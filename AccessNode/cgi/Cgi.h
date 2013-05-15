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

#ifndef _CGI_H_
#define _CGI_H_


typedef struct var_s
{
	char	*name,
	*value;
} s_var;

typedef struct file_s
{
	char	*name,
	*type,
	*filename,
	*tmpfile;
} s_file;


#include "IO.h"

class CGI {
public:
	CGI(IO&p_io)
		:io(p_io)
	{}
	bool Init( ) ;
	char* GetValue ( const char* );
	char** GetVariables() ;
	char** GetFiles();
	s_file* GetFile( const char *name) ;
	void FreeList( char** ) ;
	void Free() ;
	IO&	io ;

protected:
	bool ReadVariables( ) ;
	bool ReadMultipart ( char *boundary, int contentLength) ;
	char *ReadFile ( char *boundary, int& contentLength ) ;

protected:
	s_var **vars;
	s_file **files;
} ;

/* EscapeSgml
 *
 * Escapes <&> in a string
 */
char *EscapeSgml (char *string);
char *UnescapeSgml (char *text) ;

#endif /* _CGI_H_ */
