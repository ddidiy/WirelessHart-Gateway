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

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../Shared/log2flash.h"
#include <time.h>

#define OPT_STRING               "f:h"
#define WHILE_GETOPT(var)        while( ((var)=getopt(argc, argv, OPT_STRING)) != -1 )

extern int optind;
extern char * optarg;


void print_help_and_exit(char *prog_name)
{ 
  printf("%s: log messages to log file with special format\n", prog_name);
  printf("Default LOG_LIMIT=%d bytes.\n", LOG_LIMIT);
  printf("USAGE: %s [-f log_file] [-h] messages ... \n", prog_name);
  printf("   [-f log_file]      log file. DEF:work_time.debug\n");
  printf("   [-h]               output this message.\n");
  exit(0);
}

int main(int argc, char ** argv)
{
  const char *fname=DEF_LOG_FILE;
  int optret;
  int bufsize=0;

  if (argc == 1) {
    printf("No message to log\n");
    exit(0);
  }

  WHILE_GETOPT(optret) {
    switch(optret) {
    case 'f':
      fname=optarg;
      break;
    case 'h':
      print_help_and_exit(argv[0]);
      break;
    default:
      ;
    }
  }

#ifdef LOG2FLASH_DEBUG
  printf("optind=%d, argc=%d\n", optind, argc);
#endif

  for(int i=optind ; i < argc; i++ ) {
    bufsize += strlen(argv[i]);
    bufsize += 1;
  }

  bufsize += 3;
  char * buf = (char *) malloc(bufsize);
  if ( buf == NULL ) {
    fprintf(stderr, "Memory error!\n");
    exit(1);
  }

  int pos=0;
  for (int i=optind; i < argc; i ++) {
    pos += sprintf(buf + pos, "%s ", argv[i]);
  }

  bool ret=log2flash_f(fname, buf);

#ifdef LOG2FLASH_DEBUG
  printf( "log2flash: %s\n", buf );
  
  if (ret)
    printf("log2flash succeeded.\n");
  else
    printf("log2flash failed.\n");
  
#endif

  free(buf);
	    
  return (ret?0:1);
}

