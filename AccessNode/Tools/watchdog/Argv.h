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

#ifndef _ARGV_H_
#define _ARGV_H_

#include <getopt.h>
#include <stdlib.h>
#include "Shared/Common.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
class Argv {
public:
	Argv( int p_argc, char* p_argv[], const char* p_shortOpts, struct option* p_longOpts, int p_nbopts )
		: argc(p_argc)
		, argv(p_argv)
		, shortOpts(p_shortOpts)
		, longOpts(p_longOpts)
		, nbopts(p_nbopts)
	{
	}
	~Argv() {}
	int parse() {
		int opt ;
		do
		{
			struct option key,*rv ;
			opt = getopt_long(argc, argv, shortOpts, longOpts, NULL );
			switch ( opt ) {
			case -1 : // all done
			case '?':
				break;
			default:
				key.val = opt ;
				rv = (struct option*)bsearch( &key, longOpts, nbopts,sizeof(struct option), Argv::compmi);
				if ( rv )
				{
					if ( (rv->has_arg==required_argument)
					||   (optarg && rv->has_arg==optional_argument) )
					{
						rv->flag=(int*)optarg;
					}

					if ( (!optarg && rv->has_arg==optional_argument)
					||   (!rv->has_arg) )
						rv->flag=(int*)1;
				}
				break;
			}
		}while (opt != -1);
		return true ;
	}
	void* isSet(int opt)
	{
		struct option key,*rv ;
		key.val = opt ;
		rv = (struct option*)bsearch( &key, longOpts, nbopts,sizeof(struct option), Argv::compmi);
		if ( rv )
			return rv->flag;
		return 0 ;
	}
protected:
	static int compmi(const void *m1, const void *m2) ;
	int argc ;
	char **argv ;
	const char *shortOpts ;
	struct option *longOpts ;
	int nbopts ;
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
inline int Argv::compmi(const void *m1, const void *m2)
{
	struct option *mi1 = (struct option *) m1;
	struct option *mi2 = (struct option *) m2;
	return mi1->val - mi2->val;
}

#endif	//_ARGV_H_
