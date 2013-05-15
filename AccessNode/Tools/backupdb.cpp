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

#include "sqlite3.h"
#include <cstdio>
#include <cstdlib>

/*
 ** Perform an online backup of database pDb to the database file named
 ** by zFilename. This function copies 5 database pages from pDb to
 ** zFilename, then unlocks pDb and sleeps for 250 ms, then repeats the
 ** process until the entire database is backed up.
 **
 ** The third argument passed to this function must be a pointer to a progress
 ** function. After each set of 5 pages is backed up, the progress function
 ** is invoked with two integer parameters: the number of pages left to
 ** copy, and the total number of pages in the source file. This information
 ** may be used, for example, to update a GUI progress bar.
 **
 ** While this function is running, another thread may use the database pDb, or
 ** another process may access the underlying database file via a separate
 ** connection.
 **
 ** If the backup process is successfully completed, SQLITE_OK is returned.
 ** Otherwise, if an error occurs, an SQLite error code is returned.
 */
int backupDb(
		sqlite3 *pDb,               /* Database to back up */
		const char *zFilename,      /* Name of file to back up to */
		void(*xProgress)(int, int)  /* Progress function to invoke */
	)
{
	int rc;                     /* Function return code */
	sqlite3 *pFile;             /* Database connection opened on zFilename */
	sqlite3_backup *pBackup;    /* Backup handle used to copy data */

	/* Open the database file identified by zFilename. */
	rc = sqlite3_open(zFilename, &pFile);
	if( rc==SQLITE_OK ){

		/* Open the sqlite3_backup object used to accomplish the transfer */
		pBackup = sqlite3_backup_init(pFile, "main", pDb, "main");
		if( pBackup ){

			/* Each iteration of this loop copies 5 database pages from database
			 ** pDb to the backup database. If the return value of backup_step()
			 ** indicates that there are still further pages to copy, sleep for
			 ** 250 ms before repeating. */
			do {
				rc = sqlite3_backup_step(pBackup, 5);
				xProgress(
						sqlite3_backup_remaining(pBackup),
						sqlite3_backup_pagecount(pBackup)
					 );
				if( rc==SQLITE_OK || rc==SQLITE_BUSY || rc==SQLITE_LOCKED ){
					sqlite3_sleep(250);
				}
			} while( rc==SQLITE_OK || rc==SQLITE_BUSY || rc==SQLITE_LOCKED );

			/* Release resources allocated by backup_init(). */
			(void)sqlite3_backup_finish(pBackup);
		}
		rc = sqlite3_errcode(pFile);
	}

	/* Close the database connection opened on database file zFilename
	 ** and return the result of this function. */
	(void)sqlite3_close(pFile);
	return rc;
}


int main()
{
	sqlite3 *pDb ;
	int rc ;
	rc = sqlite3_open("/tmp/Monitor_Host.db3", &pDb ) ;
	if( rc!=SQLITE_OK )
	{
		printf("Unable to open /tmp/Monitor_Host.db3");
		exit( EXIT_FAILURE );
	}
	backupDb(pDb, "/access_node/activity_files/Monitor_Host.db3", 0) ;
	sqlite3_close(pDb);
	return 0 ;
}
