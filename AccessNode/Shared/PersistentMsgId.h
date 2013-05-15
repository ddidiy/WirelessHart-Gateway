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

// PersistentMsgId.h: interface for the CPersistentMsgId class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PERSISTENTMSGID_H__73BD4F17_F23D_4C9A_9079_50048DB2FBA8__INCLUDED_)
#define AFX_PERSISTENTMSGID_H__73BD4F17_F23D_4C9A_9079_50048DB2FBA8__INCLUDED_

/// @addtogroup libshared
/// @{

#include "Common.h"
#include "PersistentInteger.h"

class CPersistentMsgId  
{
public:
	CPersistentMsgId();
	virtual ~CPersistentMsgId();

public:
	u_int GetNextMsgId();
	int Init(	const char* p_szStorage, u_int p_unMinMsgId, 
				u_int p_unMaxMsgId, u_int p_unStep = 5000  );

private:
	CPersistentInteger m_oLastSavedMsgId;
	u_int	m_nMsgId;

	u_int	m_unMaxMsgId;
	u_int	m_unMinMsgId;
	u_int	m_unStep;
	u_int	m_unSafeLimit;
	
};

/// @}
#endif // !defined(AFX_PERSISTENTMSGID_H__73BD4F17_F23D_4C9A_9079_50048DB2FBA8__INCLUDED_)
