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
		printf("Usage: %s lock_file cmd_line [timeout]", argv[0] );
		return 0;
	}

	g_stLog.OpenStdout();

	int nTimeout = 600;
	if (argc>=4)
	{	
		nTimeout = atoi(argv[3]);
	}
	CFileLock oLock;

	oLock.SetFile(argv[1]);

	oLock.Lock();
	int ret = system_to(nTimeout, argv[2]);	
	oLock.Unlock();

	return ret;
}
