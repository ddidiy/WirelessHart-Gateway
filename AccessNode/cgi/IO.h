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

#ifndef _IO_H_
#define _IO_H_

#include <cstdio>


class IO ;
//class CSession ;
//class CEnvironment ;

#include "Session.h"
#include "Environment.hpp"

class IO {
public:
	FILE*		input ;
	FILE*		output ;
	CSession	session ;
	CEnvironment	env ;

	IO( FILE*in=stdin, FILE*out=stdout )
		: input(in)
		, output(out)
		, session(*this)
	{}
} ;

#endif	/* _IO_H_ */
