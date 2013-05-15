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

// SimpleStream.h: interface for the CSimpleStream class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SIMPLESTREAM_H__F18A9AD0_3184_43B0_AEF5_FAE755299DCB__INCLUDED_)
#define AFX_SIMPLESTREAM_H__F18A9AD0_3184_43B0_AEF5_FAE755299DCB__INCLUDED_

/// @addtogroup libshared
/// @{

class CSimpleStream  
{
public:
	void Load (const char* p_szBuff);
	void SkipDelim();
	CSimpleStream( char p_cDelim = ',',  char p_cComm = '#', unsigned int  p_nSize = 1024 );
	virtual ~CSimpleStream();
	void Release();

	bool IsEmpty();
	bool GetInt (int* p_pInt);
	bool GetHexInt (int* p_pInt);

private:
	char*	m_szStream;
	int		m_nSize;
	int		m_nCrtPos;
	char	m_cDelim;
	char	m_cComm;
};

/// @}
#endif // !defined(AFX_SIMPLESTREAM_H__F18A9AD0_3184_43B0_AEF5_FAE755299DCB__INCLUDED_)
