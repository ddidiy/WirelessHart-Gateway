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

#include "Csv.h"

struct CCsv::Impl {
	unsigned  m_nTmpSz;
	char* m_pTmp ;
	int  m_nLineSz;
	unsigned  m_nLineFree;
	char* m_pLine ;
	char* m_pIt;
	char m_cBegining ;
	char m_cSeparator ;
	char m_cEnd ;
	bool m_bEor;
	char m_cEndMark ;
	Impl()
	: m_nTmpSz(512)
	, m_pTmp(0)
	, m_nLineSz(512)
	, m_nLineFree(512)
	, m_pLine(0)
	, m_pIt(0)
	, m_cBegining('[')
	, m_cSeparator(',')
	, m_cEnd(']')
	, m_bEor(false)
	, m_cEndMark('\n')
	{
	}
} ;


CCsv::CCsv()
{
	m_pImpl = new Impl ;
	m_pImpl->m_pIt = m_pImpl->m_pLine = (char*)malloc(m_pImpl->m_nLineSz);
	m_pImpl->m_pTmp = (char*)malloc(m_pImpl->m_nTmpSz);
}


CCsv::~CCsv()
{
	free(m_pImpl->m_pLine);
	free(m_pImpl->m_pTmp);
	delete m_pImpl;
}


const bool CCsv::Eor() const
{
	return m_pImpl->m_bEor;
}

void CCsv::Eor(bool b)
{
	m_pImpl->m_bEor = b;
}

const char * CCsv::CurrentIt() const
{
	return m_pImpl->m_pIt ;
}


const char* CCsv::GetLine()
{
	if ( *(m_pImpl->m_pIt-1) == m_pImpl->m_cSeparator )
		*(m_pImpl->m_pIt-1) = '\0' ;
	return m_pImpl->m_pLine ;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief Set the CSV line to read from
////////////////////////////////////////////////////////////////////////////////
CCsv& CCsv::SetLine(const char* line)
{
	free(m_pImpl->m_pLine);
	m_pImpl->m_bEor  = false;
	m_pImpl->m_pLine = strdup(line) ;
	m_pImpl->m_pIt   = m_pImpl->m_pLine ;
	m_pImpl->m_nLineSz = strlen(line);
	return *this ;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief Set the separators used when reading from the CSV line
////////////////////////////////////////////////////////////////////////////////
void CCsv::SetSeparator(char sep, char eor/*='\n'*/)
{
	m_pImpl->m_cSeparator = sep ;
	m_pImpl->m_cEndMark=eor;
}


void CCsv::Reset()
{
	m_pImpl->m_pIt = m_pImpl->m_pLine ;
	m_pImpl->m_nLineFree=m_pImpl->m_nLineSz;
}


void CCsv::Put(int& in)
{
	int rv = snprintf( m_pImpl->m_pTmp, m_pImpl->m_nTmpSz, "%i%c", in, m_pImpl->m_cSeparator );
	if ( rv < 0 )
	{
		return ;
	}
	if ( m_pImpl->m_nLineFree < (unsigned)rv )
	{
		addFreeSpace( rv*2 );
	}
	updateIt( rv ) ;
}


void CCsv::Put(float& in)
{
	int rv = snprintf( m_pImpl->m_pTmp, m_pImpl->m_nTmpSz, "%f%c", in, m_pImpl->m_cSeparator );
	updateIt(rv);
}


void CCsv::Put(double& in)
{
	int rv = snprintf( m_pImpl->m_pTmp, m_pImpl->m_nTmpSz, "%f%c", in, m_pImpl->m_cSeparator );
	updateIt(rv);
}


void CCsv::Put(uint64_t in)
{
	int rv = snprintf( m_pImpl->m_pTmp, m_pImpl->m_nTmpSz, "%llu%c", in, m_pImpl->m_cSeparator );
	updateIt(rv);
}


void CCsv::Put(const char* in)
{
	Put(in,strlen(in));
}


void CCsv::Put(const char* in, size_t inLen)
{
	int copyOffset = 1;
	unsigned i,o ;

	if ( inLen > m_pImpl->m_nTmpSz )
	{
		m_pImpl->m_pTmp = (char*)realloc(m_pImpl->m_pTmp, inLen*2+2+1);
		m_pImpl->m_nTmpSz = inLen*2 +2 +1; // this covers the case when in consists only of `"'
	}
	for ( i=0,o=copyOffset; i<inLen; ++i,++o )
	{
		if ( o > m_pImpl->m_nTmpSz )
		{
			m_pImpl->m_nTmpSz *= 2;
			m_pImpl->m_pTmp = (char*)realloc(m_pImpl->m_pTmp, m_pImpl->m_nTmpSz );
		}

		m_pImpl->m_pTmp[o] = in[i] ;

		if ( in[i] == '"' )
		{
			m_pImpl->m_pTmp[++o] = in[i] ;
			copyOffset = 0 ;
			continue ;
		}
		if ( in[i]=='\x0A'
		  || (in[i]=='\x0D' && in[i+1]=='\x0A')
		  || (in[i]== m_pImpl->m_cSeparator)
		   )
		{
			copyOffset = 0 ;
		}
	}
	if ( copyOffset==0 )
	{
		m_pImpl->m_pTmp[0] = m_pImpl->m_pTmp[o++] = '"' ;
	}
	m_pImpl->m_pTmp[o++] = m_pImpl->m_cSeparator ;
	o -= copyOffset ;
	if ( m_pImpl->m_nLineFree < o )
	{
		addFreeSpace( o-m_pImpl->m_nLineFree ) ;
	}
	memcpy( m_pImpl->m_pIt, m_pImpl->m_pTmp+copyOffset, o ) ;
	m_pImpl->m_pIt+=o ;
	m_pImpl->m_nLineFree -=o ;
}


CCsv& CCsv::Get(int& out)
{
	int rb, rf ;

	readValue() ;
	rf = sscanf( m_pImpl->m_pTmp, "%i%n", &out,&rb );
	if ( rf != 1 )
		out = 0;
	return *this ;
}

CCsv& CCsv::Get(char*&out)
{
	readValue() ;
	out = strdup(m_pImpl->m_pTmp);
	return *this ;
}
CCsv& CCsv::Get(std::string&out)
{
	readValue();
	out.assign(m_pImpl->m_pTmp);
	return *this ;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief Read a string from the CSV line
/// @retval false when end of line was reached
/// @remarks Read from CSV line until a separator or the end of line is reached.
////////////////////////////////////////////////////////////////////////////////
bool CCsv::readValue()
{
	bool waitEnd = false ;
	m_pImpl->m_bEor = false ;
	if ( *m_pImpl->m_pIt == m_pImpl->m_cBegining )
	{
		++m_pImpl->m_pIt ;
		waitEnd = true ;
	}

	int i ;
	for ( i=0
		; *m_pImpl->m_pIt != m_pImpl->m_cSeparator
			&& m_pImpl->m_pIt != m_pImpl->m_pLine+m_pImpl->m_nLineSz
			&& *m_pImpl->m_pIt != m_pImpl->m_cEndMark
		; ++m_pImpl->m_pIt,++i )
	{
		m_pImpl->m_pTmp[i] = *m_pImpl->m_pIt ;
		if ( *m_pImpl->m_pIt == m_pImpl->m_cEnd && waitEnd )
		{
			++m_pImpl->m_pIt ;
			if ( *m_pImpl->m_pIt == m_pImpl->m_cSeparator
			||   *m_pImpl->m_pIt == m_pImpl->m_cEndMark )
			{
				m_pImpl->m_bEor = true ;
			}
			break ;
		}
	}

	if ( *m_pImpl->m_pIt == m_pImpl->m_cEndMark )
	{
		m_pImpl->m_bEor = true;
	}

	m_pImpl->m_pTmp[i]= 0 ;

	if ( *m_pImpl->m_pIt == m_pImpl->m_cSeparator )
	{
		++m_pImpl->m_pIt ;
	}
	if ( m_pImpl->m_pIt==m_pImpl->m_pLine+m_pImpl->m_nLineSz )
	{
		return false ;
	}
	return true ;
}


void CCsv::addFreeSpace( int extra )
{
	int sz = m_pImpl->m_pIt - m_pImpl->m_pLine ;
	m_pImpl->m_pLine = (char*)realloc(m_pImpl->m_pLine, m_pImpl->m_nLineSz + extra +1/*string ending zero*/ ) ;
	m_pImpl->m_pIt = m_pImpl->m_pLine + sz;
	m_pImpl->m_nLineSz += extra ;
	m_pImpl->m_nLineFree += extra ;
}


void CCsv::updateIt( int rv )
{
	memcpy( m_pImpl->m_pIt, m_pImpl->m_pTmp, rv ) ;
	m_pImpl->m_pIt+=rv ;
	*m_pImpl->m_pIt = '\0' ;
	m_pImpl->m_nLineSz+=rv;
	m_pImpl->m_nLineFree-=rv ;
}
