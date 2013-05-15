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

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <limits.h>
#include <errno.h>
#include <ctype.h>
#include "../Shared/AnPaths.h"

#define __UPLOAD_FILE_LIST NIVIS_FIRMWARE"create_tgz.cfg"
#define str(x) # x
#define xstr(x) str(x)
#define MAX_LOG_FILES 256

int create_tgz(char const *p_rgkszFileName[], int p_iNbFiles )
{
	char files[4096]={0,};
	strcat(files,"tar cf - 2>/dev/null ");

	for (int i=0; i<p_iNbFiles; i++)
	{
		strcat( files, p_rgkszFileName[i]);
		strcat( files," ");
	}

	strcat (files, " | gzip -cf");
	fflush(stdout);
	FILE *f = fopen(NIVIS_TMP"create_tgz.log","w");
	if ( f )
	{
		fprintf( f, "system[%s]\n", files );
		fclose(f);
	}
	return system(files);
}

int main()
{
	char const *fileList[MAX_LOG_FILES];	// up to 256 files
	int fileIndex=0;

	FILE * in = fopen(__UPLOAD_FILE_LIST,"r");

	if ( !in )
	{
		printf("Content-type: text/html\r\n\r\nUnable to open config file");
		exit(EXIT_FAILURE);
	}

	char *line = (char*) malloc( PATH_MAX );

	while ( !feof(in) && fgets(line, PATH_MAX, in) && (fileIndex< MAX_LOG_FILES) )
	{
		int rv = strlen(line)-1 ;

		// eliminate the NL at end of line; eliminate empty lines
		while ( (rv >=0) && isspace(line[rv]) )
			line[rv--] = '\0';

		if (*line)
		{
			fileList[fileIndex++] = line ;
			line = (char*) malloc( PATH_MAX );
		}
	}
	if ( !fileIndex )
	{
		printf("Content-type: text/html\r\n\r\nUnable to get list files:[%s]", strerror(errno) );
		fclose(in);
		exit(EXIT_FAILURE);
	}
	fclose(in);

	printf(	"Content-disposition: attachment; filename=log_archive.tar.gz\r\n"
	        "Content-type: application/x-gzip\r\n\r\n");

	return create_tgz( fileList, fileIndex);
}
