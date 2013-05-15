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

#include "../Shared/Common.h"
#include "../Shared/FileLock.h"



int main (int argc, char** argv)
{
	if (argc<3)
	{	
		printf("Usage: %s timeout cmd_line", argv[0] );
		return 0;
	}

	g_stLog.OpenStdout();

	int nTimeout = 600;
	if (argc < 3)
	{	
		return 0;
	}
	
	nTimeout = atoi(argv[1]);
	
	char pCommand[4096];

	pCommand[0] = 0;
	int nPos = 0;

	int i;
	for (i = 2; i < argc; i++)
	{
		nPos += sprintf(pCommand + nPos, "%s ",argv[i]);
	}
	return system_to (nTimeout, pCommand);	
}
