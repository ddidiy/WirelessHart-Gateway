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
                          CLink.h  -  description
                             -------------------
    begin                : Thu June 4 2002
    email                : marcel.ionescu@nivis.com
 ***************************************************************************/

#ifndef LINK_H
#define LINK_H

#include <termios.h>
#include <sys/times.h>

#undef TEST_WITH_PIPE 
/// @addtogroup libshared
/// @{

//lint -library
class CLink 
{

private:
	bool    m_bRawLog;

protected:
    int     m_nFd;
	int     m_nOutFd;   //test only
    
public: 
	int IsLinkOpen();
	CLink( bool p_bRawLog = true );
	virtual ~CLink();

    virtual int OpenLink( const  char * p_sConn, unsigned int  p_nConnParam );  // default is a serial link
    virtual void CloseLink( void );
	void SetLogRaw( bool p_bRawLog = true ) { m_bRawLog = p_bRawLog; }

	int ReadAll(	unsigned char	*	p_pBlock,
							unsigned int		p_nMaxBlockLen,
							unsigned int		p_nSecTimeout = 0,
							unsigned int		p_nMicroSecTimeout = 100000);
	
	int openPipeLink( const char* p_szPipe );
    int openSerialLink( const char * p_sConn, unsigned int p_nConnParam );
    static unsigned short crc(unsigned short addedByte, unsigned short genpoli, unsigned short current);

    virtual int writeBlock( const unsigned char * p_pBlock, unsigned int p_nBlockLen );

// return number of read chars (-1 means error)
    virtual int readBlock(	unsigned char	*	p_pBlock,
							unsigned int		p_nMaxBlockLen,
							unsigned int		p_nSecTimeout = 0,
							unsigned int		p_nMicroSecTimeout = 100000 );

	int haveData ( unsigned int p_nSecTimeout = 0, unsigned int p_nMicroSecTimeout = 100000 );
	static int  HaveData ( int p_nFd, unsigned int p_nSecTimeout = 0, unsigned int p_nMicroSecTimeout = 100000 );
    
	int writeChar( unsigned char p_chChar ) { return writeBlock( &p_chChar, 1 ); } 

	int readChar (	unsigned char       * p_chChar,
					unsigned int          p_nSecTimeout = 0,
					unsigned int          p_nMicroSecTimeout = 100000 )
		{ return readBlock( p_chChar, 1, p_nSecTimeout, p_nMicroSecTimeout ); }


};

/// @}
#endif //LINK_H

