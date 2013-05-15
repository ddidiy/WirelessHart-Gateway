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
#include "../Shared/Utils.h"

#include "JsonRPC.h"
#include "Environment.hpp"


#define RPC_TIMEOUT 1200

void exportEnv(CEnvironment &env)
{
	if(char* tmp = getenv("CONTENT_TYPE")){
		env.Put("CONTENT_TYPE",tmp);
		LOG("CONTENT_TYPE:%s",tmp);
	}
	if(char* tmp = getenv("REQUEST_METHOD")){
		env.Put("REQUEST_METHOD",tmp);
		LOG("REQUEST_METHOD:%s",tmp);
	}
	if(char* tmp = getenv("CONTENT_LENGTH")){
		env.Put("CONTENT_LENGTH",tmp);
		LOG("CONTENT_LENGTH:%s",tmp);
	}
	if(char* tmp = getenv("HTTP_COOKIE")){
		env.Put("HTTP_COOKIE",tmp);
		LOG("HTTP_COOKIE:%s",tmp);
	}
}

int main(int argc, char*argv[], char*env[])
{
	g_stLog.Open(NIVIS_TMP"rpc.log");
	IO io ;

	CJsonRPC oJsonRPC(io) ;

	int nStartAlarmTimeout = alarm(RPC_TIMEOUT);

	LOG("Alarm timeout: old=%d new=%d", nStartAlarmTimeout, RPC_TIMEOUT);

	exportEnv(io.env);
	if ( io.env.RequestMethod() != REQ_MET_POST
	                ||  io.env.ContentLength() == 0 )
	{
		oJsonRPC.JsonError("Invalid Environment") ;
		exit(EXIT_FAILURE);
	}
	char*line = (char*)calloc( sizeof(char), io.env.ContentLength() +8 );
	if ( !line )
	{
		oJsonRPC.JsonError( -32700 );
		exit(EXIT_FAILURE);
	}

	for ( int br=0; !feof(stdin) && br< io.env.ContentLength(); )
	{
		br += fread(line+br, sizeof(char), io.env.ContentLength()-br,  stdin);
		if ( ferror(stdin) )
		{
			oJsonRPC.JsonError("STDIN error");
			exit(EXIT_FAILURE);
		}
	}

	oJsonRPC.HandleCall(line);
	free ( line );
	return EXIT_SUCCESS;
}
