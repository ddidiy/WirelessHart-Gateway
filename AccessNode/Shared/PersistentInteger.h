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

// PersistentInteger.h: interface for the CPersistentInteger class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PERSISTENTINTEGER_H__00EB1405_7861_4530_8113_E1B36BF501E2__INCLUDED_)
#define AFX_PERSISTENTINTEGER_H__00EB1405_7861_4530_8113_E1B36BF501E2__INCLUDED_
/// @addtogroup libshared
/// @{
class CPersistentInteger  
{
private:
	char	m_szFile[128];

public:
	u_int		m_nValue;

public:
	CPersistentInteger(const char* p_szFile = NULL );
	virtual ~CPersistentInteger();

public:
	int Load();
	int Save();
	void SetStorage( const char* p_szFile  );
};

/// @}
#endif // !defined(AFX_PERSISTENTINTEGER_H__00EB1405_7861_4530_8113_E1B36BF501E2__INCLUDED_)
