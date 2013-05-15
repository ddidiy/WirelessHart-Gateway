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

#include "Argv.h"
#include "App.h"

//////////////////////////////////////////////////////////////////////////////
// TODO ADD TO CONFIG.INI from template
//////////////////////////////////////////////////////////////////////////////
int main( int argc, char* argv[])
{
	const char short_opts[] = "d:hk:t:";
	struct option long_opts[] =
	{
		{"dev",		1, NULL, 'd'},
		{"help",	0, NULL, 'h'},
		{"keep-alive",	1, NULL, 'k'},
		{"pidlist",	1, NULL, 'p'},
		{"timeout",	1, NULL, 't'},
		{NULL,		0, NULL, 0}
	};
	int nbopts=(sizeof(long_opts)/sizeof(struct option))-1;

	Argv arg(argc,argv,short_opts,long_opts, nbopts);

	CWtdgApp app(arg) ;

	if (arg.isSet('h')) return CWtdgApp::Usage();

	if ( ! app.Init() )
		return EXIT_FAILURE ;

	bool stopwd = app.Run();
	app.Close(stopwd) ;
	return EXIT_SUCCESS ;
}
