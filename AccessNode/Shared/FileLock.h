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

// FileLock.h: interface for the CFileLock class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILELOCK_H__9F5B6474_2B7F_4C59_A64F_006D36D9D5E2__INCLUDED_)
#define AFX_FILELOCK_H__9F5B6474_2B7F_4C59_A64F_006D36D9D5E2__INCLUDED_


#include <stdlib.h>
/// @addtogroup libshared
/// @{
class CFileLock  
{
public:
	CFileLock(const char* p_szFile = NULL);
	virtual ~CFileLock();

	void SetFile(const char* p_szFile);

public:
	void Unlock();
	void Lock();
	
	bool TryLock();

	int ForceOldLock (int p_nAfterSec);

private:
	char*	m_szFile;
	int		m_nFd;	 
};

/// @}
#endif // !defined(AFX_FILELOCK_H__9F5B6474_2B7F_4C59_A64F_006D36D9D5E2__INCLUDED_)
