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

#include <sys/param.h>
#include <sys/file.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>
#include "Utils.h"

//STANDALONE is defined at Tools/Makefile level
#ifndef STANDALONE
	#include "Common.h"
#else
	#include "log2flash.h"
	#undef LOG_ERR
	#define LOG_ERR dummie
#endif

#define MAX_WRITE_CHAR  512                       // Maximum 100 characters can be written.
#define BLOCK_MAX       20                        // block at most 20 times (1 sec interval)

#ifdef LOG2FLASH_DEBUG
	#undef LOG_ERR
	#define LOG_ERR debug_print

	static void debug_print(const char *fmt, ...) 
	{
		va_list args;
		va_start (args, fmt);
		vprintf(fmt, args);
		va_end (args);
		printf("\n");
	}
#endif

static void dummie(const char */*fmt*/, ...) 
{
  return;
}

static bool loop_write(int fd, char *buf, int count) {
  int bytes_written=0 , bytes_to_write=count, write_ret=0; 
  int block_count=0;

  DPRINT("loop_write( %d, \"%s\", %d )\n", fd, buf, count);

  while ( bytes_to_write > 0) {
    write_ret=write(fd, buf+bytes_written , bytes_to_write );
    if (write_ret == -1 ) {
      // something wrong, it did not finish writing all bytes or fs error.
      block_count++;
      switch(errno) {
      case EINTR:      // interrupted. ignore and try again.
	break;        
      case EAGAIN:     // write not available. try again.
	sleep(1);
	break;
      default:
	LOG_ERR("log2flash ERROR: unexpected error occurred on write. errno=%d", errno);
	return false;
      }
      
      if ( block_count > BLOCK_MAX ) {
	LOG_ERR("log2flash ERROR: more than %d errors occurred.", BLOCK_MAX);
	return false;
      }
    } else {     // it wrote something.
      bytes_written += write_ret;
      bytes_to_write -= write_ret;
    }
    DPRINT( "written=%d, left=%d\n", bytes_written, bytes_to_write );
  }
 
  // write succeeded.
 
  return true;
}


static bool log2flash_fa(const char *fname, const char *fmt, va_list arglist)
{
  bool retcode=false;                // assume operation failed.
  int fd;
  time_t cur_time;
  time_t duration=0;
  struct tm *ptm;                    // structure to obtain date information
  char printbuf[MAX_WRITE_CHAR+2];   // size + '\n' + '\0'
  int bytes_to_write;
  int lock_to;                       // time out for flock()

  DPRINT( "log2flash_fa() called\n" );
  fd = open(fname, O_RDWR|O_CREAT|O_NONBLOCK, 00644);
  
  if (fd == -1) {
    LOG_ERR("log2flash ERROR: Unable to open file.");
    goto EXIT;
  }

  lock_to = FLOCK_TIMEOUT;
  while( flock(fd, LOCK_EX | LOCK_NB)  != 0 ) {
    sleep(1);
    lock_to--;
    if (lock_to < 0) {
      LOG_ERR("log2flash ERROR: lock timeout");
      goto CLOSE_EXIT;
    }
  }

  if ( lseek(fd, 0, SEEK_END) == -1 ) {
    LOG_ERR("log2flash ERROR: unable to seek to the end of file. errno=%d", errno);
    goto CLOSE_EXIT;
  }

  cur_time=time(NULL);
  ptm=gmtime(&cur_time);
  
  // tm_year from 1900, tm_mon [0-11]
  // it takes 20 characters spaces.
  sprintf(printbuf, "%d-%02d-%02d %02d:%02d:%02d ", 
	  ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday,
	  ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

  // generate the buffer string.
  vsnprintf(printbuf+20, MAX_WRITE_CHAR-20, fmt, arglist);

  bytes_to_write = strlen(printbuf);
  printbuf[bytes_to_write] = '\n';
  bytes_to_write ++;
  DPRINT( printbuf );

  // gather statistic

  struct stat f_stat;
  if ( fstat(fd, &f_stat) == -1 ) {
    LOG_ERR("log2flash ERROR: unable to gatter statistic for %s. errno=%d", fname, errno);
    goto CLOSE_EXIT;
  }

  // if write failed, goto CLOSE_EXIT
  if (loop_write(fd, printbuf, bytes_to_write) != true) goto CLOSE_EXIT;

  if ( f_stat.st_size + strlen(printbuf) > LOG_LIMIT) {    // starting to rotate
	DPRINT( "File Size exceeded LOG_LIMIT - Perform rotation\n" );
	char from[PATH_MAX];
	char to[PATH_MAX];
	for( int i=ROTATE_MAX; i>0; --i)
	{
		sprintf( from, "%s.%d", fname, i-1);
		sprintf( to, "%s.%d", fname, i);
		rename( from, to);
	}
	duration = time(NULL);
	Copy( fname, from, LOG_LIMIT );
	unlink( fname );
	duration -= time(NULL);

	if (FileIsExecutable(FTP_SCRIPT))
	{
		char sBaseFile[PATH_MAX];
		char sFileName[PATH_MAX];

		sprintf(sBaseFile,NIVIS_TMP"activity.log");

		AttachDatetime(sBaseFile,sFileName);

		Copy(from,sFileName);
		systemf_to(5, FTP_SCRIPT" %s &", sFileName);
	}


  }
  retcode=true;

 CLOSE_EXIT:
  flock(fd, LOCK_UN);
  close(fd);
  if (duration < -10) { //it really is negative :) we only convert it when we need to show it, below
	log2flash("Logfile rotation took quite long: %d seconds", -duration);
  }

 EXIT:
  return retcode;
}

bool log2flash(const char *fmt, ...)
{
  bool retcode;
  va_list arglist;
  va_start(arglist, fmt);

  DPRINT(( "log2flash() called\n" ));

  retcode=log2flash_fa(DEF_LOG_FILE, fmt, arglist);

  va_end(arglist);
  return retcode;
}

bool log2flash_f(const char *fname, const char *fmt, ...)
{
  bool retcode;
  va_list arglist;
  va_start(arglist, fmt);

  DPRINT(( "log2flash_f() called\n" ));
  retcode=log2flash_fa(fname, fmt, arglist);

  va_end(arglist);
  return retcode;
}
