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
                         pipe.cpp  -  description    
                              -------------------
    begin                : Fri Apr 12 2002
    email                : marcel.ionescu@nivis.com
 ***************************************************************************/
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <stdarg.h>
#include <stdlib.h>
//#include "CString.h"

#include "Common.h"
#include "Utils.h"
#include "pipe.h"


#define PIPE_START_MARKER 0xcbcbcbcb

CPipe::CPipe()
{
	m_nFd       = -1;
	m_pBuffer   = NULL;
	m_bLock     = false;
	m_szPipeName[ 0 ] = 0;
}

CPipe::~CPipe()
{
	LOG( "Pipe Destructor called()" );
	Close();
	if(m_pBuffer)
		free( m_pBuffer );
}

//////////////////////////////////////////////////////////////////////////////////
// Description : open the pipe
// Parameters  :
//              const char * p_sPipeName  - input - the pipe name
//                int          p_nFlag      - input - open type flag
// Return      :
//             0 on error or 1 on success  
//////////////////////////////////////////////////////////////////////////////////
int CPipe::Open(   const char * p_sPipeName /*= "/tmp/pipe"*/, int  /*p_nFlag = O_TRUNC | O_RDWR */, bool p_bLock/*= false*/)
{
    m_bLock = p_bLock;
    
	LOG( "opening pipe \"%s\"", p_sPipeName );
	strncpy( m_szPipeName,  p_sPipeName, sizeof( m_szPipeName ) );
	
	if( mkfifo( p_sPipeName, 0666 ) ) // == -1
    {
		;	//do nothing, this is OK to fail
        //LOG_ERR( "(EEXIST) is normal behaviour");
    }

    //ignore p_nFlag: as result, modules are allowed to start independently, and not wait one after another
	if( ( m_nFd = open( p_sPipeName, O_TRUNC | O_RDWR /*p_nFlag*/  ) ) < 0 )
	{
		LOG_ERR( "open pipe : cannot open pipe %s", p_sPipeName );
		return 0;
	}

	LOG( "pipe %s open, fd %d", p_sPipeName, m_nFd );

	return 1;
}

//////////////////////////////////////////////////////////////////////////////////
// Description : close the pipe
// Parameters  :
//              none
// Return      :
//              none
//////////////////////////////////////////////////////////////////////////////////
void CPipe::Close( )
{

	if( m_nFd >= 0 )
	{
		LOG("Closing the pipe %d", m_nFd);
		close( m_nFd );
		m_nFd = -1;
        m_bLock = false;
	}
}


//////////////////////////////////////////////////////////////////////////////////
// Description : write a message in the pipe
// Parameters  :
//              const char* p_pHeader     - input - the header
//              int         m_nHeaderSize - input - the header size
//              const char* p_pData       - input - the data
//              int         p_nDataLen    - input - the data length
// Return      :
//             0 on error or 1 on successfull
//////////////////////////////////////////////////////////////////////////////////
int CPipe::WriteMsg( const void * p_pHeader, int m_nHeaderSize, const char *  p_pData, int p_nDataLen )
{
	int    nResult;
	int    nBufferSize = m_nHeaderSize + p_nDataLen;

    lock();
	u_int nMarker = PIPE_START_MARKER;

	int nDataLen = nBufferSize-4;	//TODO: question: what if we write DataLen in first 4 header bytes?
	nResult =		writeBlock( (char*) &nMarker, sizeof(nMarker) )	
				&&	writeBlock( (char*) &nDataLen, sizeof(nDataLen) )  
				&&	writeBlock( (char*) p_pHeader+4, m_nHeaderSize-4 ) 
				&&	writeBlock( p_pData, p_nDataLen );
    unlock();

	return nResult;
}

//////////////////////////////////////////////////////////////////////////////////
// Description : read a message from the pipe
// Parameters  :
//      char      * p_pHeader     - output - the header
//      int         m_nHeaderSize - input - the expected header size
//      char     ** p_ppData      - output - the data - it not must be deallocated outside
//      int       * p_pDataLen    - output - the data length
// Return      :
//             0 on error or 1 on success
//////////////////////////////////////////////////////////////////////////////////
int CPipe::ReadMsg( void * p_pHeader, int m_nHeaderSize, const char **  p_ppData, int * p_nDataLen )
{
//    LOG( "CPipe::readMsg reading header %d", m_nHeaderSize);

//    lock();
	if (!readStartMarker()) 
	{	//unlock();
        return 0;
	}

	if( ! readBlock( (char*)p_pHeader, m_nHeaderSize ) )
	{
		LOG_ERR( "CPipe::ReadMsg: could not read header from pipe");
		//unlock();
        return 0;
    }

	char * p;
	int    nDataLen = 0;
	memcpy( &nDataLen, p_pHeader, sizeof(nDataLen) ); 

	nDataLen -= (m_nHeaderSize-4);

	//LOG( "CPipe::readMsg header ok. datalen %d", nDataLen);
	if(IsSysMemoryLow(nDataLen/1024) )
	{	log2flash("ERR (CPipe::ReadMsg): MEMORY LOW %dkB (req %d kB)", GetSysFreeMemK(), nDataLen/1024);
		return 0;
	}

	if( ( p = (char *) realloc( m_pBuffer, nDataLen ) ) == NULL && nDataLen )
	{
		LOG_ERR( "CPipe::ReadMsg: cannot realloc %d bytes, pipe corrupted", nDataLen );
        //unlock();
		return 0;
	}
	m_pBuffer = p;


	if( nDataLen && !readBlock( m_pBuffer, nDataLen ) )
	{   
		LOG_ERR( "CPipe::ReadMsg: could not read data section, pipe corrupted");
		//unlock();
        return 0;
    }
 //   unlock();

	*p_ppData   = m_pBuffer;
	*p_nDataLen = nDataLen;

	return 1;
}

//repease internal buffer, after use. This is needed because data size can be pretty large
//don't close the pipe; all operation will continue normally after this call; just don't use data pointer, now invalid
void CPipe::ReleaseBuffer( void )
{
	if(m_pBuffer)
	{	
		//LOG("CPipe::ReleaseBuffer (%s)", m_szPipeName );
		free(m_pBuffer);
		m_pBuffer = NULL;	//prepare for another realloc call
	}
}

#define MAX_PIPE_ERR_COUNT 50000
//////////////////////////////////////////////////////////////////////////////////
// Description : write all p_nBlockLen chars in the pipe
// Parameters  :
//                const char* p_pBlock      - input - the block data
//                int         p_nBlockLen   - input - the block length
// Return      :
//             0 on error or 1 on success
//////////////////////////////////////////////////////////////////////////////////
int CPipe::writeBlock( const char *  p_pBlock, int p_nBlockLen )
{
	int nCrtPos = 0;
	int nErrCount = 0;

	if( p_nBlockLen < 0 )
	{
		LOG_ERR( "ERROR CPipe::writeBlock: invalid request to write %d bytes", p_nBlockLen ); 
		return 0;
	}

/*
	THIS WARNING IS INCORRECT. WE LOCK THE PIPE BEFORE WRITING
	if( p_nBlockLen > PIPE_BUF )
	{
		LOG("WARNING CPipe::writeBlock: received %d bytes > %d. Data might be interleaved", p_nBlockLen, PIPE_BUF);
	}
*/
	do
	{
		int nResult;

		nResult = write( m_nFd, p_pBlock+nCrtPos, p_nBlockLen-nCrtPos );
		if( nResult > 0 )
		{
			nCrtPos += nResult;
			nErrCount = 0;
		}
		else if( nResult == 0 )	
		{
			if(++nErrCount > MAX_PIPE_ERR_COUNT)
			{
				LOG_ERR( "ERROR CPipe::writeBlock writing on fd %d returned zero too many times (%d)", m_nFd, nErrCount ); 
				return reset();		//return 0
			}
			sleep(0);	//noting written to pipe. try again, but relinquish control first
		}
		else //nResult == -1 or negative...
		{
			if( nResult != -1)		//undoccumented retcode. log this and go on
				LOG_ERR("ERROR CPipe::writeBlock: write() on fd %d returned undocumented %d", m_nFd, nResult );
			
			if( errno == EINTR ) // If write() returns -1 because a signal interrupted it, then we want to continue writing to the pipe 
				LOG("SIGNAL INTERRUPTED write(). Continue."); 
			else 
			{ 
				LOG_ERR( "ERROR CPipe::writeBlock writing on fd %d", m_nFd ); 
				return reset(); 	//return 0
			} 
		}
	}
	while( p_nBlockLen>nCrtPos );

	return 1;
}
#undef MAX_PIPE_ERR_COUNT

//////////////////////////////////////////////////////////////////////////////////
// Description : read all p_nBlockLen chars from the pipe
// Parameters  :
//                char      * p_pBlock      - output - the block data
//                int         p_nBlockLen   - input  - the block length
// Return      :
//             0 on error or 1 on success
//////////////////////////////////////////////////////////////////////////////////
int CPipe::readBlock( char *  p_pBlock, int p_nBlockLen )
{
	int nCrtPos = 0;

	if( p_nBlockLen <= 0 )
	{
		LOG_ERR( "ERROR CPipe::readBlock: invalid request to read %d bytes", p_nBlockLen ); 
		return 0;
	}

	do
	{
		int nResult;

		nResult = read( m_nFd, p_pBlock+nCrtPos, p_nBlockLen-nCrtPos );
		if( nResult > 0 )
		{
			nCrtPos += nResult;
		}
		else if( nResult == 0 )	//EOF. No more data available, return error. We know for sure p_nBlockLen > nCrtPos
		{
			LOG_ERR( "ERROR CPipe::readBlock EOF reading from fd %d", m_nFd ); 
				return reset(); 	//return 0
		}
		else //nResult == -1 or negative...
		{
			if( nResult != -1)	//undoccumented retcode. log this and go on
				LOG_ERR("ERROR CPipe::readBlock: read() from fd %d returned undocumented %d", m_nFd, nResult );
			
			if( errno == EINTR ) // If read() returns -1 because a signal interrupted it, then we want to continue reading from the pipe 
				LOG("SIGNAL INTERRUPTED read(). Continue."); 
			else 
			{ 
				LOG_ERR( "ERROR CPipe::readBlock readinf from fd %d", m_nFd ); 
				return reset(); 	//return 0
			} 
		}
	}
	while( p_nBlockLen>nCrtPos );

	return 1;
}


int  CPipe::HaveMsg( int p_nTime /*=10*/ )
{
    struct timeval tv;
    tv.tv_sec = p_nTime / 1000000;
    tv.tv_usec = p_nTime % 1000000; 
        
    fd_set stFd;
    FD_ZERO( &stFd );
    FD_SET( m_nFd, &stFd );
	
    if( select( m_nFd+1, &stFd, NULL, NULL, &tv ) <= 0 )
	{	return 0;
	}
	
    return FD_ISSET( m_nFd, &stFd );
}

//////////////////////////////////////////////////////////////////////////////////
// Description : reset the pipe (close/open) as a last attempt to recover
// Parameters  : none
// Return      : always zero (use it to return failure)
//TAKE CARE: do not call this from Open or Close!
//////////////////////////////////////////////////////////////////////////////////
int CPipe::reset( void )
{
	LOG("WARNING: Pipe %d MALFUNCTION, try to reset", m_nFd );
	Close();
	Open( m_szPipeName, O_TRUNC | O_RDWR, m_bLock );
	return 0;	//always. will be used to return
}



int CPipe::readStartMarker()
{
	u_int nMarker = 0;
	do 
	{	u_char ucTmp;	

		read(m_nFd,&ucTmp,1);
		nMarker = nMarker<<8 | ucTmp;

		if (nMarker == PIPE_START_MARKER) 
		{	return 1;
		}
	} while(HaveMsg(100000));
	
	LOG("CPipe::readStartMarker -- no pipe start marker");
	return 0;
}
