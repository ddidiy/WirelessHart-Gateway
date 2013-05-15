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

#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>

#include "../Shared/Common.h"
#include "../Shared/Utils.h"
#include "../Shared/FileLock.h"

#include "Cgi.h"
#include "JsonRPC.h"

#define _UPLOAD_TMP NIVIS_TMP"upgrade_web/"
#define UPGRADE_LOCK	NIVIS_TMP"fw_upgrade.lock"

#include "Environment.hpp"


s_cgi *cgi;

bool uploadFile (CJsonRPC& oJsonRPC, char **env)
{
	char **vars, *tmp, *val;
	s_file *file;
	int i;
	//Remove _UPLOAD_TMP first
	if ( ! access(_UPLOAD_TMP, R_OK|W_OK) )
	{
		log2flash("CGI: dir "_UPLOAD_TMP" was not previously deleted");
		system("rm -rf "_UPLOAD_TMP"* &>/dev/null" );
	}

	////////////////////

	// Recreate _UPLOAD_TMP
	if ( access(_UPLOAD_TMP, R_OK|W_OK) && mkdir(_UPLOAD_TMP , 0777) )
	{
		FLOG("Unable to make dir");
		oJsonRPC.JsonError("Unable create temporary dir", true) ;
		exit(EXIT_FAILURE);
	}

	// get the file first then act uppon it
	vars = cgiGetFiles (cgi);
	if (vars)
	{
		for (i=0; vars[i] != NULL; i++)
		{
			file = cgiGetFile (cgi, vars[i]);
			if ( !file )
			{
				FLOG("Unable to get file");
				continue ;
			}

			tmp = EscapeSgml (file->filename);
			free (tmp);
			if (file->type)
			{
				tmp = EscapeSgml (file->type);
				free (tmp);
			}

			char dstfile[PATH_MAX]={0,};
			snprintf(dstfile, PATH_MAX, _UPLOAD_TMP"%s", file->filename );
			FLOG("FILE:%s", dstfile);

			if (rename(file->tmpfile, dstfile))
			{
				oJsonRPC.JsonError("Unable to rename file", true);
				exit(EXIT_FAILURE);
			}
			log2flash("CGI: %s uploaded [%s]", getenv("REMOTE_ADDR"), dstfile ) ;
		}
		cgiFreeList (vars);
	}

	// act uppon the above file
	vars = cgiGetVariables (cgi);
	if (vars)
	{
		for (i=0; vars[i] != NULL; i++)
		{
			val = cgiGetValue (cgi, vars[i]);
			if ( !val )
			{
				FLOG("Unable to get variable");
				continue ;
			}
			tmp = EscapeSgml (val);
			if ( !tmp ) continue ;
			if ( oJsonRPC.HandleCall(tmp, true))
			{
				FLOG("All done");
				return true;
			}
			free (tmp);
		}
		cgiFreeList (vars);
	}
	return true ;
}

void handle_sigpipe( int )
{
	LOG("SIGPIPE");
}

void handle_sigterm( int )
{
	LOG("SIGTERM");
	exit(1);
}

void handle_sigalarm( int )
{
	LOG("SIGALRM");
	log2flash("SIGALRM");
	exit(1);
}


#define UPLOAD_TIMEOUT 1200

int main (int argc, char **argv, char **env)
{
	signal(SIGPIPE, handle_sigpipe );
	signal(SIGTERM, handle_sigterm );
	signal(SIGALRM, handle_sigalarm );

	CJsonRPC oJsonRPC ;

	int nStartAlarmTimeout = alarm(UPLOAD_TIMEOUT);

	g_stLog.Open(NIVIS_TMP"upload.log");

	LOG("Alarm timeout: old=%d new=%d", nStartAlarmTimeout, UPLOAD_TIMEOUT);
	log2flash("CGI: Alarm timeout: old=%d new=%d", nStartAlarmTimeout, UPLOAD_TIMEOUT);

	log2flash( "CGI: Upload started" ) ;
	LOG( "pid=%d: Upload started ", getpid() ) ;

	// get the browser data first.
	cgi = cgiInit();
	if ( !cgi )
	{
		oJsonRPC.JsonError("Unable to init CGI", true) ;
		exit(EXIT_FAILURE);
	}

	CFileLock oLock(UPGRADE_LOCK);
	LOG("upload try to get lock");
	int nSecLeft = oLock.ForceOldLock(5*60); //use 5minutes without upload time and 15minutes with upload time

	// try to obtain the lock
	if (nSecLeft)
	{
		char szMsg[1024];
		sprintf(szMsg,"FW upgrade in progress please try again in %d minutes", nSecLeft/60 + 1);

		LOG(szMsg);

		oJsonRPC.JsonError(szMsg,true) ;
		exit(EXIT_FAILURE);
	}


	LOG("upload got lock");

	uploadFile (oJsonRPC, env);

	LOG("upload unlink and unlock");
	if (unlink(UPGRADE_LOCK)) LOG_ERR("upload unlink(%s)",UPGRADE_LOCK);
	oLock.Unlock();

	cgiFree (cgi);
	return 0;
}
