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

/***************************************************************************
log.cpp  -  description
-------------------
begin                : Sat Apr 13 2002
email                : marcel.ionescu@nivis.com
partly rewritten     : Mar 1 2005 - mihai.buha@nivis.com
$CVSHeader: AccessNode/Shared/log.cpp,v 2.77.2.2.2.2 2013/05/15 19:19:17 owlman Exp $
***************************************************************************/
#include <fcntl.h>
#include <sys/wait.h>
#include <dirent.h>

#include "log.h"
#include "Common.h"



CLog::CLog()
{
	m_pFile        = (FILE*) NULL;
	m_sFileName    = (char*) NULL;
	m_nCrtPosition = 0;
	m_nMaxSize     = 0;
	m_nMoveTimeout=0;
	m_bArchive	= false;
	m_bTailSafe	= false;
	m_pszStorage	= NULL;
	m_nFlashFreeLimit	= 0;
	m_bMaintenance	= false;
	m_bTruncThreadSafe = false;
}

CLog::~CLog()
{
	Close();
	free(m_pszStorage);
}

////////////////////////////////////////////////////////////////////////
//	Name:		Open
//	Description:	open the log file, initialise the max size
//	Parameters:	p_sFileName - input - log file name
//				p_sStartMessage - input - start message
//				p_nMaxSize - input - max log size. Truncate when this size is reached
//	Return value:	true if open ok
////////////////////////////////////////////////////////////////////////
bool CLog::Open( const char * p_sFileName,
				const char * p_sStartMessage /*= "Start session" */,
				int p_nMaxSize /*= 524288,*/ , bool p_bCreateBackup1 /*=false*/)
{
	Close(NULL);

	m_nMaxSize = p_nMaxSize;
	m_bCreateBackup1 = p_bCreateBackup1;

	m_pFile = fopen( p_sFileName, "a+" );
	if (!m_pFile)
	{	return false;
	}

	m_sFileName = (char *) strdup( p_sFileName );

	syncToEOF();

	if( p_sStartMessage )
	{
		m_nCrtPosition += fprintf( m_pFile, "%s process %d current pos %d\n",
			p_sStartMessage, getpid(), m_nCrtPosition );
	}

	return true;
}


////////////////////////////////////////////////////////////////////////
//	Function	:	Open
//	Description	: open the standard output
//	Parameters	:	none
//	Return value: true
////////////////////////////////////////////////////////////////////////
bool CLog::OpenStdout( void )
{
	m_pFile = stdout;
	m_nMaxSize = 0;

	return true;
}

////////////////////////////////////////////////////////////////////////
//	Function	:	Close
//	Description	: close the log
//	Parameters	:	p_sCloseMessage message to log before close
//	Return value:nonr
////////////////////////////////////////////////////////////////////////
void CLog::Close( const char * p_sCloseMessage /*= "End session" */ )
{
	if (m_pFile == stdout)
	{
		m_pFile = NULL;
		return;
	}

	if( m_pFile )
	{
		if( p_sCloseMessage )
		{
			WriteMsg( "%s process %d", p_sCloseMessage, getpid() );
		}

		fclose( m_pFile );
		m_pFile = NULL;

		free( m_sFileName ); //accept null value
		m_sFileName = NULL;
	}

}


//////////////////////////////////////////////////////////////////////////////////
// Description:	write a message in the log file
// Parameters:	int p_nErrorFlag      - input - 0 if is a regular message, 1 on a system error
//			const char *  p_sMsg  - input - the message or the message format
// Returns:		nothing
//////////////////////////////////////////////////////////////////////////////////
void CLog::writeMsg( int p_nErrorFlag, const char *  p_sMsg, va_list p_lstArg)
{
	if( ! m_pFile ) // log file is not open -> DO NOT use standart outputs.
	{	return;
	}
	// log file is open
	// write first the current time

	WriteTime();

	if( p_nErrorFlag )
	{
		m_nCrtPosition += fprintf( m_pFile, "%s : ", strerror(errno) );
	}

	m_nCrtPosition += vfprintf( m_pFile, p_sMsg, p_lstArg);

	m_nCrtPosition += fprintf( m_pFile, "\n" );
	fflush( m_pFile );

	logMaintenance();
}

//////////////////////////////////////////////////////////////////////////////////
// Description:	write a message in the log file
// Parameters:	int p_nErrorFlag      - input - 0 if is a regular message, 1 on a system error
//			const char *  p_sMsg  - input - the message or the message format
// Returns:		nothing
//////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/// @author Marcel Ionescu
/// @brief Fast method - write a string to the log file (also write time before and \n after)
/// @param p_sMsg String to write
/// @return none
/// @remarks This is faster than writeMsg(0, p_sMsg) because it does not parse the input (format) string
/// @remarks It does not interpret the input string, so char % is allowed in input
/// @note TAKE CARE: it does NOT flush the stream, an exception from all the other methods in this class
/// @todo
/// @todo Must use a different method to identify the fault on segfaults (segfault handler)
/// @todo
////////////////////////////////////////////////////////////////////////////////
void CLog::WriteString( const char *  p_sMsg )
{
	if( ! m_pFile ) // log file is not open -> DO NOT use standart outputs.
	{	return;
	}
	WriteTime();
	/// no need to update m_nCrtPosition: logMaintenance does it anyway
	/// fputs + fputc('\n') is significantly faster (3x on 100 chars) than fprintf( "%s\n")
	/// fprintf("%s") has the same speed as fputs !! (but the disadvantage of interpreting the string)
	fputs( p_sMsg, m_pFile );
	fputc( '\n', m_pFile );
	///fflush( m_pFile );	/// 3x on 100 char speedup at the cost of loosing last log lines on segfaults

	logMaintenance();	/// will update m_nCrtPosition
}

//////////////////////////////////////////////////////////////////////////////////
// Description : write a messge in the log file
// Parameters  :
//		const char *  p_sMsg  - input - the message or the message format
// Return      : nothing
//////////////////////////////////////////////////////////////////////////////////
void	CLog::WriteMsg( const char *  p_sMsg, ...)
{
	if(( p_sMsg==( char*) 0)){
		WriteMsg("WARNING! You called the logging facility using the old style! Check log.h and then your code!");
		return;
	}
	va_list lstArg;
	va_start( lstArg, p_sMsg);
	writeMsg(false, p_sMsg, lstArg);
	va_end( lstArg );

	return;
}

//////////////////////////////////////////////////////////////////////////////////
// Description : write a messge in the log file, togheter with errno description
// Parameters  :
//		const char *  p_sMsg  - input - the message or the message format
// Return      : nothing
//////////////////////////////////////////////////////////////////////////////////
void	CLog::WriteErrMsg( const char *  p_sMsg, ...)
{
	if(( p_sMsg==( char*) 0)){
		WriteMsg("WARNING! You called the logging facility using the old style! Check log.h and then your code!");
		return;
	}
	va_list lstArg;
	va_start( lstArg, p_sMsg);
	writeMsg(true, p_sMsg, lstArg);
	va_end( lstArg );
}


//////////////////////////////////////////////////////////////////////////////////
// Description : truncate the log to minimum size.
// Parameters  : none
// Return      : true is truncated, false otherwise
//////////////////////////////////////////////////////////////////////////////////
void	CLog::logMaintenance()
{
	if (m_pFile == stdout)
	{	return;
	}

	if (m_bMaintenance)
	{	return;
	}
	m_bMaintenance = true;

	syncToEOF();

	if( m_nCrtPosition < m_nMaxSize )
	{	m_bMaintenance = false;
		return;
	}

	//help for tail
	m_nCrtPosition += fprintf( m_pFile, "---LOG File exceeded max size, truncated---\n" );
	fflush( m_pFile );


	char sFileName[ m_bCreateBackup1 ? Attach_1(m_sFileName) : AttachDatetime(m_sFileName)];
	if(m_bCreateBackup1) 
		Attach_1(m_sFileName,sFileName);
	else 
		AttachDatetime(m_sFileName,sFileName);

	/// NO ftp backup if we are using .1 naming scheme for backups
	bool bFtpReq = m_bCreateBackup1 ? false : FileIsExecutable(FTP_SCRIPT);

	if (m_bArchive || m_bCreateBackup1 || bFtpReq)
	{
		if (m_bTailSafe)
		{	cp (m_sFileName, sFileName);
		}
		else
		{	rename (m_sFileName, sFileName);
		}

		if (bFtpReq)
		{	systemf_to(5, FTP_SCRIPT" %s &", sFileName);
		}

		if (m_bArchive)
		{	moverProcess(sFileName);
		}
	}

	if (m_bTailSafe)
	{
		int nFd = fileno(m_pFile);
		if(ftruncate( nFd, 0)==0)
		{
			rewind( m_pFile);
			m_bMaintenance = false;
			return;
		}
	}

	strcpy( sFileName, m_sFileName);	//m_sFileName is deleted by Close()
	Close(NULL);
	remove(sFileName);
	m_nCrtPosition = 0;
	Open( sFileName, "", m_nMaxSize, m_bCreateBackup1 );
	m_bMaintenance = false;
}

/** write an emergency message to log.
does not do any new()/malloc(), so this can be used in case of memory allocation failure */
void CLog::EmergencyMsg(const char * p_pMsg){

	FILE * pFile = m_pFile;

	if( ! pFile ) // log file is not open -> use standard outpus
	{
		pFile = stdout;
	}
	m_nCrtPosition += fprintf( pFile, p_pMsg );
	m_nCrtPosition += fprintf( pFile, "\n" );
	fflush( pFile );

}

//////////////////////////////////////////////////////////////////////////////////
// Description : write a messge in the log file, then the hex representation of a binary buffer.
// Parameters  :
//		const char *  p_sMsg  - input - the message or the message format
// Return      : nothing
//////////////////////////////////////////////////////////////////////////////////
void     CLog::WriteHexMsg( const char *  p_sMsg, const unsigned char * p_pBinBuffer, int p_nBinBufferLen )
{
	//size: message to write + 3 char for every byte (2 for hex one for space) + null terminator
	int nIndex = 0, nMaxIndex = strlen( p_sMsg ) + p_nBinBufferLen*3 + 1 ;

	char * pHex = new char[nMaxIndex];

	if( pHex )
	{
		nIndex += snprintf( pHex + nIndex, nMaxIndex - nIndex, "%s", p_sMsg );
		for ( int i=0; i < p_nBinBufferLen; i++ )
		{
			if( nIndex > nMaxIndex)//PROGRAMMER ERROR
				EmergencyMsg("CLog::WriteHexMsg WARNING: buffer oevrflow. Ckeck the code.");
			nIndex += snprintf( pHex + nIndex, nMaxIndex - nIndex, " %02X", p_pBinBuffer[i] );
		}
		WriteMsg( "%s", pHex );

		delete[] pHex;
	}

	return;
}

//////////////////////////////////////////////////////////////////////////////////
// Name:	SetStorage
// Author:	Mihai Buha (mihai.buha@nivis.com)
// Description:	set the pathname where the log files will be archived
// Parameters:  const char p_szStorage[]  - input - the new pathname
// Returns:	nothing
//////////////////////////////////////////////////////////////////////////////////
void	CLog::SetStorage( const char p_szStorage[] )
{
	if( p_szStorage== NULL ){
		LOG("CLog::SetStorage: WARNING!! You called SetStorage() with a NULL pointer");
		return;
	}
	unsigned int nLen= strlen( p_szStorage)+ 1;  //size of the incoming buffer
	unsigned int nBufSize=0;    //won't archive anything, unless tests succeed
	nLen = nLen>=PATH_MAX ? PATH_MAX-1 : nLen;

	if( p_szStorage[0] ){  // if it's not the empty string ("")
		int err;
		if( !mkdir( p_szStorage, 0755)){
			err= EEXIST;
		}else{
			err= errno;
		}
		if( err== EEXIST){ // p_szStorage exists (not necessarily as a directory).  This includes the case where pathname is a symbolic link, dangling or not.
			DIR* pDir= opendir( p_szStorage);
			if( pDir){
				char szTmpFile[ PATH_MAX];
				int nFd;
				strncpy( szTmpFile, p_szStorage, PATH_MAX);
				strncat( szTmpFile, "/XXXXXX", PATH_MAX-nLen);
				szTmpFile[ PATH_MAX-1]= 0;
				if( (nFd= mkstemp( szTmpFile))!= -1){
					nBufSize= (p_szStorage[ nLen- 2]== '/')? nLen: nLen+1;  // we have a GOOD storage directory! :)
					close(nFd);
					remove( szTmpFile);
				}else{
					LOG("CLog::SetStorage: Archiving disabled: couldn't check %s is writable.", p_szStorage);
				}
				closedir( pDir);
			}else{
				LOG_ERR("CLog::SetStorage: Archiving disabled: couldn't open %s", p_szStorage);
			}
		}else{
			LOG_ERR("CLog::SetStorage: Archiving disabled: couldn't create %s", p_szStorage);
		}
	}

	void* pTemp= realloc( m_pszStorage, nBufSize);
	if (!pTemp && nBufSize){
		LOG_ERR("CLog::SetStorage: realloc failed: ");
		return;
	}
	m_pszStorage=(char*)pTemp;
	if(nBufSize){
		m_bArchive= true;
		strncpy(m_pszStorage, p_szStorage, nBufSize);
		m_pszStorage[ nLen-1 ]= 0;
		if( nLen!= nBufSize){
			strcat( m_pszStorage, "/");
		}
	}else{
		m_bArchive= false;
	}
}

//////////////////////////////////////////////////////////////////////////////////
// Name:	getFileLen
// Author:	Mihai Buha (mihai.buha@nivis.com)
// Description:	Measures file size. It borrows code from the two GetFileLen()'s in Common.cpp
// Returns:	0 on error, 1 on success
//////////////////////////////////////////////////////////////////////////////////
int CLog::syncToEOF()
{
	if (!m_pFile)
	{	return 0;
	}
	// the old version. still here for back-reference
	// fseek( m_pFile, 0, SEEK_END );
	// int nLen = ftell( m_pFile );
	int nLen = fstat( fileno(m_pFile), &m_stat );
	if( nLen < 0 )
	{
		if (m_sFileName)
		{	WriteErrMsg( "Cannot get the length of file %s", m_sFileName );
		}

		return 0;
	}
	m_nCrtPosition = m_stat.st_size;
	return 1;
}

//////////////////////////////////////////////////////////////////////////////////
// Name:	cp
// Author:	Mihai Buha (mihai.buha@nivis.com)
// Description:	copies a file. Inspired from stdio's rename()
// Parameters:	const char oldpath[]  - input - the name of the file to copy (not used, because we already have
//									the open stream, but it's easier to read the calling code)
//			const char newpath[]  - input - the name of the copy
// Returns:	0 if successful, or -1 if an error occurred
//////////////////////////////////////////////////////////////////////////////////
int CLog::cp(const char *p_szOldpath, const char *p_szNewpath)
{
	bool err = false;
	if ( !m_pFile){
		LOG( "CLog::cp: log file not open!");
		return -1;
	}
	char* pBuffer = new char[ 4096]; //a copy buffer
	if ( !pBuffer ){	//very low space available
		LOG_ERR( "CLog::cp: out of memory");
		return -1;
	}
	FILE* pDest;
	if( ( pDest = fopen( p_szNewpath, "w"))== NULL ){
		delete[] pBuffer;
		LOG_ERR( "CLog::cp: fopen(%s) failed", p_szNewpath);
		return -1;	//can't open backup log file, for some reason
	}
	rewind( m_pFile);
	size_t  nIOSizeR, nIOSizeW;
	int i=0;
	do{
		nIOSizeR= fread( pBuffer, 1, 4096, m_pFile);
		if( ferror( m_pFile)){
			fprintf( stderr, "CLog::cp: fread error! i=%d\n", i);
			err = true;
			break;
		}
		nIOSizeW= fwrite(pBuffer, 1, nIOSizeR, pDest);
		if( ferror( pDest)){
			fprintf( stderr, "CLog::cp: fwrite error! i=%d\n", i);
			err = true;
			break;
		}
		if( nIOSizeR != nIOSizeW){
			fprintf( stderr, "CLog::cp: Didn't write as much as requested, but no error!\n"); // this can't happen, can it?
		}
		i++;
	}while( !feof( m_pFile));
	if( fseek( m_pFile, 0, SEEK_END)){
		fprintf( stderr, "CLog::cp: fseek error!\n");
		err = true;
	}
	fclose( pDest);
	delete[] pBuffer;
	if( err ){
		LOG_ERR( "CLog::cp: copy error from %s to %s", p_szOldpath, p_szNewpath);
		return -1;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////
// Name:	moverProcess
// Author:	Mihai Buha (mihai.buha@nivis.com)
// Description:	launches a mv command and watches over it
// Parameters:	const char sFileName[]  - input - the name of the file to move
// Returns:	nothing
//////////////////////////////////////////////////////////////////////////////////
void CLog::moverProcess( const char* szFileName)
{
#ifndef ARM
	if (m_pszStorage)
	{
		char	szCmdLine[PATH_MAX*2 +6];
		snprintf( szCmdLine, PATH_MAX*2 +6, "mv -f %s %s", szFileName, m_pszStorage);
		system_to( 15, szCmdLine);
	}
	return;
#endif
	pid_t pid=
#ifndef USE_VFORK
	fork();
#else
	vfork();
#endif
	if(pid) {
		if( pid== -1){
			remove( szFileName); // intermediate fork() failed. OutOfMem? deleting file!
		}else{
			wait(NULL); // I am the parent, waiting for the intermediate
		}
		return;
	}
	pid=
#ifndef USE_VFORK
	fork();
#else
	vfork();
#endif
	if (pid){   // I am the intermediate process, and my child will be waited for by init!
		if( pid== -1){
			remove( szFileName); // child fork() failed. OutOfMem? deleting file!
		}
		exit(0);
	}

	// I am the child!
	LOG("CLog::moverProcess: execlp()ing log_maint -c -m %s", szFileName);
	execlp( "log_maint", "log_maint", "-c", "-m", szFileName, NULL);
	LOG_ERR("CLog::moverProcess: execlp() failed, deleting file!");
	remove( szFileName);
	exit( 0);
}


void CLog::WriteTime()
{	static time_t tOldtime = 0;	/// initial old is always invalid
	static char sBuf[ 32 ] = {0,}; // 21 should be enough

	if (!m_pFile)
	{	return;
	}

	time_t tTime = time( NULL );
	if( tTime != tOldtime )
	{	struct tm * pTm = gmtime( &tTime );
		/// strftime is faster than sprintf on
		///		coldfire MCF548x (17 000 :  9 000 op/sec)
		///		Nivis ARM9       (19 500 : 10 000 op/sec)
		/// strftime is slower than sprintf on x86 (1.05 M:1.15 M op/sec)
		///sprintf( sBuf, "%04d-%02d-%02d %02d:%02d:%02d ",pTm->tm_year+1900,pTm->tm_mon+1,pTm->tm_mday,pTm->tm_hour,pTm->tm_min,pTm->tm_sec);
		strftime( sBuf, sizeof(sBuf), "%F %T ", pTm);
		tOldtime = tTime;
	}

	/// do not replace fprintf with fputs: need to maintain m_nCrtPosition
	m_nCrtPosition += fprintf( m_pFile, sBuf);
}

void CLog::WriteInLine(const char *  p_sMsg, ...)
{
	if (!m_pFile || !p_sMsg)
	{	return;
	}
	va_list lstArg;
	va_start( lstArg, p_sMsg);
	m_nCrtPosition += vfprintf( m_pFile, p_sMsg, lstArg);
	va_end( lstArg );
}

void CLog::WriteEol()
{
	if (!m_pFile)
	{	return;
	}

	m_nCrtPosition += fprintf( m_pFile, "\n" );
	fflush( m_pFile );

	logMaintenance();
}

void CLog::WriteWithEol(const char *  p_sMsg, ...)
{
	if (!m_pFile || !p_sMsg)
	{	return;
	}
	va_list lstArg;
	va_start( lstArg, p_sMsg);
	m_nCrtPosition += vfprintf( m_pFile, p_sMsg, lstArg);
	va_end( lstArg );
	WriteEol();
}
