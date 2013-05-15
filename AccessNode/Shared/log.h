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
                          log.h  -  description
                           -------------------
    begin                : Sat Apr 13 2002
    email                : marcel.ionescu@nivis.com
    partly rewritten     : Mar 1 2005 - mihai.buha@nivis.com
 ***************************************************************************/
//lint -library

#ifndef _LOG_H_
#define _LOG_H_

#include <stdio.h>
#include <stdarg.h>

/// @addtogroup libshared
/// @{

#define FTP_SCRIPT "/access_node/ftp_log_upload.sh" 
# ifdef __cplusplus

#include <sys/stat.h>

class CLog
{
public:
	CLog();
	~CLog();

	bool	Open(	const char * p_sFileName,
			    const char * p_sStartMessage = "Start session",
			    int p_nMaxSize = 524288, bool p_bCreateBackup1=false);

	bool	OpenStdout( void );

	void	Close( const char * p_sCloseMessage = "End session" );

	const char* GetLogFileName() { return m_sFileName ? m_sFileName : ""; }

	void	WriteMsg( const char *  p_sMsg, ...)__attribute__((weak))  ;

	void	WriteErrMsg( const char *  p_sMsg, ...);
	void 	DoNothing( const char *, ...) const { return ;}

	/// @brief Fast method - write a string to the log file (also write time before and \n after)
	/// @note TAKE CARE: it does NOT flush the stream, an exception from all the other mrthods in this class
	void    WriteString( const char *  p_sMsg );

	void	WriteTime();
	void	WriteInLine(const char *  p_sMsg, ...);
	void	WriteWithEol(const char *  p_sMsg, ...);
	void	WriteEol();

	void	WriteHexMsg( const char *  p_sMsg, const unsigned char * p_pBinBuffer, int p_nBinBufferLen );

	void    SetMaxSize( int p_nMaxSize ) { m_nMaxSize = p_nMaxSize; };
	void	SetTail( bool p_bTail ) { m_bTailSafe = p_bTail; };
	void	SetStorage( const char p_szStorage[] );
	void	SetMoveTimeout( int p_nMoveTimeout) { m_nMoveTimeout= p_nMoveTimeout; };
	void	SetFlashFreeLimit( int p_nFFree) { m_nFlashFreeLimit = p_nFFree; };
	void	SetTruncThreadSafe( bool p_bTruncThreadSafe ) { m_bTruncThreadSafe = p_bTruncThreadSafe; }
    void	SetBackup1(bool p_bCreateBackup1 ) { m_bCreateBackup1 = p_bCreateBackup1; };

	/** write an emergency message to log.
	* does not do any new()/malloc(), so this can be used in case of
	* memory allocation failure */
	void	EmergencyMsg(const char * p_pMsg);
	void	WriteVarMsg( const char *  p_sMsg, va_list p_lstArg){writeMsg(0 ,p_sMsg,p_lstArg);};

private:
	void	logMaintenance();
	void	writeMsg( int p_nErrorFlag, const char *  p_sMsg, va_list p_lstArg);
	int		syncToEOF();
	int		cp(const char *oldpath, const char *newpath);
	void	moverProcess( const char* sFileName);

	FILE * m_pFile;

	char * m_sFileName;

	int    m_nCrtPosition;
	int    m_nMaxSize;
	bool	m_bArchive;
	bool	m_bTailSafe;
	char*	m_pszStorage;
	int		m_nMoveTimeout;
	int		m_nFlashFreeLimit;
	bool	m_bMaintenance;
	bool	m_bTruncThreadSafe;
	bool	m_bCreateBackup1;/// create .1 backup files, DO NOT call ftp upload script
	struct stat m_stat ;
};
# else
   typedef struct CLog CLog ;
# endif	// __cplusplus

/// @}
#endif
