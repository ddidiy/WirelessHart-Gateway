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

#include <new>
#include "DiscoveryApp.h"
#include "../../Shared/Common.h"
#include "../../Shared/IniParser.h"

extern CDiscoveryApp g_stApp;

CIniParser iniParser;

volatile int m_nFd = -1;

int
main(int argc, char *argv[])
{
		
    g_stLog.OpenStdout();
    char szExeName[30]; 
    strcpy(szExeName,basename(argv[0]) );

	if( g_stApp.Init((const char *) szExeName))
	{
		try
		{
			if (argc > 2)
			{	
				//TODO :
				LOG("Help");
			}
			else
			{	g_stApp.Run(); //entering normal operation loop
				g_stApp.Close();
			}
			
		}
		catch( std::bad_alloc &ba )
		{
			g_stLog.EmergencyMsg("FATAL ERROR Discovery: catch std::bad_alloc. Probably OUT OF MEMORY");
		}
		catch(...)
		{
			g_stLog.EmergencyMsg("FATAL ERROR Discovery: catch(...)");
		}
	}
	
	return 1;
	
}


