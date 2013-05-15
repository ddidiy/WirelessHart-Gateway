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
                          link.cpp  -  description
                             -------------------
    begin                : Fri December 13 2002
    email                : claudiu.hobeanu@nivis.com
 ***************************************************************************/
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <termios.h>
#include <netinet/in.h>
#include <stdlib.h>

#include "Common.h"
#include "link.h"



CLink::CLink( bool p_bRawLog /*= true*/ )
{
	m_nFd = -1;
	m_bRawLog = p_bRawLog;
}

CLink::~CLink()
{
	CloseLink();
}

int CLink::OpenLink( const  char * p_sConn, unsigned int  p_nConnParam )  // implicit is a serial link
{
	if( ! openSerialLink( p_sConn, p_nConnParam ) )
	{
		//CloseLink();
		return 0;
	}
	return 1;
}


//////////////////////////////////////////////////////////////////////////////////
// Description : close the serial link
// Parameters  :
//              none
// Return      :
//              none
//////////////////////////////////////////////////////////////////////////////////
void CLink::CloseLink( void )
{
    if( m_nFd >= 0 )
    {
        close( m_nFd );
        m_nFd = -1;
    }
}


int CLink::IsLinkOpen()
{
	return m_nFd >= 0;
}

//////////////////////////////////////////////////////////////////////////////////
// Description : write all p_nDataLen chars in the rs232
// Parameters  :
//                const char * p_pBlock      - input - the block data
//                int          p_nBlockLen   - input - the block length
// Return      :
//             0 on error or 1 on success
//////////////////////////////////////////////////////////////////////////////////
int CLink::writeBlock(  const unsigned char * p_pBlock, unsigned int p_nBlockLen )
{
	if (m_nFd<0)
	{
		return 0; //invalid serial 
	}

    int nResult;
    unsigned int nCrtPos = 0;

    if( m_bRawLog )
        LOG_HEX( "writeBlock: ", p_pBlock, p_nBlockLen );

    do
    {


//to send one by one, put 1 below. To send the whole block, put 0 below (correct: sensitive to #define DEBUG!!)
//#if 0
//        nResult = write( m_nFd, p_pBlock+nCrtPos, 1);//write one by one - needed in debug mode
//#else
#ifdef TEST_WITH_PIPE
		nResult = write( m_nOutFd, p_pBlock+nCrtPos, p_nBlockLen-nCrtPos );   //write the whole block
#else
        nResult = write( m_nFd, p_pBlock+nCrtPos, p_nBlockLen-nCrtPos );   //write the whole block
#endif


        if( nResult <= 0 )
        {
            LOG_ERR( "writeBlock: Cannot write %d bytes", p_nBlockLen );
            return 0;
        }

        nCrtPos += nResult;
    }
    while( p_nBlockLen > nCrtPos );

    return 1;
}


#define SERIAL_CONS_ZERO 64
// return number of read chars (-1 means error)
//////////////////////////////////////////////////////////////////////////////////
// Description : listen serial line  for a specified interval and read if characters are received
// Parameters  :
//              [out] unsigned char       * p_pBlock,
//              [in]  unsigned int          p_nMaxBlockLen,
//              [in]  unsigned int          p_nSecTimeout = 0,
//              [in]  unsigned int          p_nMicroSecTimeout = 100 000 (0.1 sec)
// Return      :
//              -1 : error, else number of characters read (0 means no error, but nothing read!)
//////////////////////////////////////////////////////////////////////////////////
int CLink::readBlock(unsigned char * p_pBlock, unsigned int p_nMaxBlockLen, unsigned int p_nSecTimeout /*= 0*/
		     ,unsigned int p_nMicroSecTimeout /*= 100000*/ )
{
	if (m_nFd<0)
	{
		return -1; //invalid serial 
	}

	if( !haveData( p_nSecTimeout, p_nMicroSecTimeout ) )
	{
		return 0;//nothing to read, but NO error
	}

	int nReadLen = read( m_nFd, p_pBlock, p_nMaxBlockLen );

	if( nReadLen <= 0 )
	{
		LOG_ERR( "ERROR CLink::readBlock: cannot read from serial" );
		return -1;
	}

	if( m_bRawLog )
	{
		bool bLog = true;
		if ( (p_nSecTimeout>0 || p_nMicroSecTimeout > 0) && (nReadLen > SERIAL_CONS_ZERO) )
		{
			bLog = false;

			for(int i=0;i<nReadLen;i++)
			{
				if (p_pBlock[i])
				{	bLog = true;
					break;
				}
			}
		}

		if(bLog)
			LOG_HEX( "readBlock:  ", p_pBlock, nReadLen );
		else
			LOG("readBlock: 00[%d]",nReadLen);
	}

	return nReadLen;	
}


//////////////////////////////////////////////////////////////////////////////////
// Description : listen serial line  for a specified interval and read if characters are received
// Parameters  :
//              [out] unsigned char       * p_pBlock,
//              [in]  unsigned int          p_nMaxBlockLen,
//              [in]  unsigned int          p_nSecTimeout = 0,
//              [in]  unsigned int          p_nMicroSecTimeout = 100 000 (0.1 sec)
// Return      :
//             1/0 - success/error
//////////////////////////////////////////////////////////////////////////////////
int CLink::ReadAll(unsigned char * p_pBlock, unsigned int p_nMaxBlockLen, unsigned int p_nSecTimeout /*= 0*/
		   ,unsigned int p_nMicroSecTimeout /*= 100000*/)
{
	if (m_nFd<0)
	{
		return 0; //invalid serial 
	}

	u_int nPos;

	for( nPos = 0; nPos < p_nMaxBlockLen;)
	{
		int nCrt = readBlock( p_pBlock + nPos, p_nMaxBlockLen - nPos, p_nSecTimeout, p_nMicroSecTimeout );
		if ( nCrt <= 0 )
		{	return 0;
		}
		nPos += nCrt;
	}

	return 1;
}


//////////////////////////////////////////////////////////////////////////////////
// Description : listen serial line  for a specified interval and tell if there is anything to read
// Parameters  : [in] p_nSecTimeout      numebr of seconds to wait
//				 [in] p_nMicroSecTimeout number of microseconds (10 pow -6) to wait
// Return      :
//              true if data available, otherwise false
//////////////////////////////////////////////////////////////////////////////////
int CLink::haveData(unsigned int p_nSecTimeout /*= 0*/,
					unsigned int p_nMicroSecTimeout /*= 100000*/ )
{
	if (m_nFd<0)
	{
		return 0; //	FD_SET( m_nFd, &setFd ); will crash if m_nFd < 0
	}

	fd_set          setFd;
	struct timeval  tv;
	int				nRet;

	FD_ZERO( &setFd );
	FD_SET( m_nFd, &setFd );

	tv.tv_sec = p_nSecTimeout;
	tv.tv_usec= p_nMicroSecTimeout;

	nRet = select ( m_nFd + 1, &setFd, NULL, NULL, &tv );
	if( nRet < 0 )
	{
		LOG_ERR( "CLink::HaveData: error on select " );
		return 0;
	}

	return nRet;
}

int CLink::openSerialLink( const char * p_sConn, unsigned int p_nConnParam )
{
	struct termios trm;
	struct stat st;
	int i;

	signal(SIGTTOU, SIG_IGN);   //this is probably alrady done...
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);

	LOG( "OpenSerialLink( %s, %d ) begin", p_sConn, p_nConnParam );

	m_nFd = open( p_sConn, O_RDWR | O_NOCTTY | O_NDELAY );

	if( m_nFd==-1 )
	{
		LOG_ERR( "OpenSerialLink: cannot open serial link %s", p_sConn );
		return 0;
	}
	if(  -1 == (i = fcntl(m_nFd, F_GETFL)))
		LOG_ERR( "OpenSerialLink: fcntl get returned -1 ");

	if( -1 == fcntl(m_nFd, F_SETFL, ( i & ~O_NDELAY ) | O_RDWR ) )
		LOG_ERR( "OpenSerialLink: fcntl set returned -1 ");

	if( fstat(m_nFd, &st) == -1 )
	{
		LOG_ERR( "OpenSerialLink: error in fstat" );
		return 0;
	}

	if( ! S_ISCHR(st.st_mode) )
	{
		LOG_ERR( "OpenSerialLink: probably not character device :)" );
		return 0;
	}

	for( i=0 ; i<6 && tcgetattr( m_nFd, &trm )==-1 ; i++ )
		sleep(1);

	if( i >= 6 )
	{
		LOG_ERR( "OpenSerialLink: cannot get the serial line attributes" );
		return 0;
	}
	//INPUT flags: DO NOT translate CR/NL: INLCR | ICRNL on output
	trm.c_iflag &= ~( ISTRIP | INLCR | ICRNL | IXOFF | IXON );
	trm.c_iflag |=   IGNBRK;
	//OUTPUT  flags: DO NOT translate CR/NL: ONLCR | OCRNL on input
	trm.c_oflag &= ~( OPOST | ONLCR | OCRNL );
	//CONTROL flags: 8N1
	trm.c_cflag &= ~( CSIZE | PARENB | CSTOPB | CRTSCTS );
	trm.c_cflag |= CS8;
	//LOCAL flags
	trm.c_lflag &= ~( ECHO | ECHONL | ICANON | ISIG | IEXTEN | ECHOE | ECHOK | ECHOCTL | ECHOKE);

	cfsetospeed(&trm, p_nConnParam);
	cfsetispeed(&trm, p_nConnParam);

	if( tcsetattr(m_nFd, TCSANOW, &trm) == -1 )
	{
		LOG_ERR( "OpenSerialLink: cannot set the serial line attributes" );
		return 0;
	}

	if( setreuid( 0, 0 )<0 )
		LOG_ERR( "OpenSerialLink: cannot set the real/effective user id. Not a problem :) " );

	return 1;
}

int CLink::openPipeLink(const char *p_szPipe)
{
	LOG( "opening pipe \"%s\"", p_szPipe );

	if( mkfifo( p_szPipe, 0666 ) ) // == -1
    {
		;	//do nothing, this is OK to fail
        //LOG_ERR( "(EEXIST) is normal behaviour");
    }

	int nFd;
    //ignore p_nFlag: as result, modules are allowed to start independently, and not wait one after another
	if( ( nFd = open( p_szPipe, O_TRUNC | O_RDWR /*p_nFlag*/  ) ) < 0 )
	{
		LOG_ERR( "open pipe : cannot open pipe %s", p_szPipe );
		return -1;
	}

	//this is not necessary to log...
	//LOG( "pipe \"%s\" opened OK", p_sPipeName );

	return nFd;
}



