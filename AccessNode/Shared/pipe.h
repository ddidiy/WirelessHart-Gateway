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
                          pipe.h  -  description
                           -------------------
    begin                : Fri Apr 12 2002
    email                : marcel.ionescu@nivis.com
 ***************************************************************************/

//lint -library

#ifndef _PIPE_H_
#define _PIPE_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/file.h>


#include "Common.h"

/// @addtogroup libshared
/// @{

// Structures and Classes defined in this file
//   THeaderToDNC
//   THeaderToCC
//   THeaderToRA
//   THeaderSim
//   TRFToCCMsgHeader
//   TCCToRFMsgHeader
//   THeaderHMCC
//   TTimeInterval
//   TCCToHMData
//   THMCCMsg
//   THeaderToCommMgr
//   CPipe
class CPipe
{
public:





public:
        CPipe();
        ~CPipe();

        int  Open( const char * p_sPipeName = "/tmp/pipe", int  /*p_nFlag*/ = O_TRUNC | O_RDWR,
                            bool p_bLock = true );
        void Close( );

        int  GetFd() const { return m_nFd; } ;
        int  HaveMsg( int p_nTime = 10 );

        int  WriteMsg( const void * p_pHeader, int m_nHeaderSize, const char *  p_pData , int p_nDataLen);
        int  ReadMsg( void * p_pHeader, int m_nHeaderSize, const char **  p_ppData , int * p_nDataLen);
		
		//release internal buffer, after use. This is needed because data size can be pretty large
		//don't close the pipe; all operation will continue normally after this call; just don't use data pointer, now invalid
		//TODO: check this can be safely called by HaveMsg
		void ReleaseBuffer( void );

private:
        int  writeBlock( const char *  p_pBlock, int p_nBlockLen );
        int  readBlock( char * p_pBlock, int p_nBlockLen );

        void lock() { if(m_bLock)   flock( m_nFd, LOCK_EX ); }
        void unlock() { if(m_bLock)   flock( m_nFd, LOCK_UN ); }

		int reset( void );
        int    m_nFd;
        char * m_pBuffer;
        bool   m_bLock;
		char	m_szPipeName[ 256 ];

			
protected:
	int readStartMarker();
};

/// @}
#endif //PIPE_H
