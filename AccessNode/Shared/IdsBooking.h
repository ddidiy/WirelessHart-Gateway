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

// IdsBooking.h: interface for the CIdsBooking class.
//
/// @addtogroup libshared
/// @{
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IDSBOOKING_H__249D10DC_4C89_4E56_8337_44932797EE73__INCLUDED_)
#define AFX_IDSBOOKING_H__249D10DC_4C89_4E56_8337_44932797EE73__INCLUDED_


#include "ObjList.h"

class CIdsBooking  
{
public:
	class TInterval
	{
	public:
		int m_nStart;
		int	m_nEnd;
	};

public:
	CIdsBooking(int p_nMin = 0, int p_nMax = 0xffff);
	virtual ~CIdsBooking();

public:
	int GetNewId2();
	void Log();
	void Remove(int p_nId);
	int GetNewId();
	int Add (int p_nId);

	int GetLastAdd() {return m_nLastAdd;}
	void SetLastAdd(int p_nLastAdd) { m_nLastAdd = p_nLastAdd;}

private:
	int m_nMin;
	int m_nMax;

	int m_nLastAdd;

	ObjList<TInterval>	m_oIdList;	
};

/// @}
#endif // !defined(AFX_IDSBOOKING_H__249D10DC_4C89_4E56_8337_44932797EE73__INCLUDED_)
