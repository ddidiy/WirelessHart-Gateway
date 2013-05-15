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

//g++ ../../Shared/IniParser.o ../../Shared/Utils.o ../../Shared/log.o ../../Shared/Common.o  -Wall update_pass.cpp  -lcrypt  -o update_pass

#include "../Shared/IniParser.h"
#include "../Shared/Common.h"

#include <unistd.h>

CIniParser iniParser;

#define USERDB_FILE NIVIS_ACTIVITY_FILES"userdb.ini"


bool set_password(char *file_name, char *user,const char *password)
{

	if (!file_name)
	{
		LOG("NULL file_name");
		return false;
	}

	if (!iniParser.Load(file_name,"r+"))
	{
		LOG("Load() fail");
		return false;
	}


	LOG("user=%s, password=%s\n",user, password);

	if (!user || !password)
	{
		LOG("User or passowrd empty");
		return false;
	}

	const char *user1=iniParser.FindGroup( user, false);
	LOG("user1=%s\n", user1);

	if (!user1)
	{
		LOG("FindGroup() User %s not found\n", user);
		return false;
	}


	char *encrypted_password=NULL;
	char *salt="$1$";

	if ((encrypted_password=crypt(password, salt) )==NULL)
	{
		LOG("crypt() == NULL\n");
		return false;
	}
	LOG("encrypted_password = %s\n",encrypted_password);

	int ret_code=0;
	if (!(ret_code=iniParser.SetVar( user,"pass",encrypted_password, 0, true)))
	{

		LOG("SetVar() error ret_code = %d\n", ret_code);
		return false;
	}

	/*    char value[100]={0};
	    LOG("getVar==%d\n",iniParser.GetVar(user, "password", value, 100, 0));
	    LOG("Value=%s\n", value);
	*/
	return true;
}



int
main(int argc, char **argv)
{
	g_stLog.OpenStdout();

	LOG("Output to Stdout is Working");


	if (argc<3)
	{
		LOG("Incorrect number of parameters");
		return EXIT_FAILURE;
	}


	if (!set_password(USERDB_FILE, argv[1],argv[2]))
	{
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
