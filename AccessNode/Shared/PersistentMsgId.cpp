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

// PersistentMsgId.cpp: implementation of the CPersistentMsgId class.
//
//////////////////////////////////////////////////////////////////////

#include "PersistentMsgId.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPersistentMsgId::CPersistentMsgId()
{

}

CPersistentMsgId::~CPersistentMsgId()
{

}

int CPersistentMsgId::Init( const char *p_szStorage, u_int p_unMinMsgId, u_int p_unMaxMsgId, u_int p_unStep)
{
	m_unMaxMsgId	= p_unMaxMsgId;
	m_unMinMsgId	= p_unMinMsgId;
	m_unStep		= p_unStep;
	m_unSafeLimit	= p_unStep /2;
	if (m_unSafeLimit > 50) 
	{	m_unSafeLimit = 50;
	}

	m_oLastSavedMsgId.SetStorage(p_szStorage);
	m_oLastSavedMsgId.m_nValue = m_unMinMsgId;
	m_oLastSavedMsgId.Load();

	m_nMsgId = m_oLastSavedMsgId.m_nValue;

	m_oLastSavedMsgId.m_nValue += m_unStep;
	if ( m_oLastSavedMsgId.m_nValue >= m_unMaxMsgId ) 
	{	m_oLastSavedMsgId.m_nValue = m_unMinMsgId;
	}
	m_oLastSavedMsgId.Save();

	return 1;
}

u_int CPersistentMsgId::GetNextMsgId()
{
	if (++m_nMsgId >= m_unMaxMsgId) 
	{	m_nMsgId = m_unMinMsgId;
	}

	if (m_nMsgId >= m_oLastSavedMsgId.m_nValue - m_unSafeLimit) 
	{	
		m_oLastSavedMsgId.m_nValue += m_unStep;
		if (m_oLastSavedMsgId.m_nValue >= m_unMaxMsgId) 
		{	m_oLastSavedMsgId.m_nValue = m_unMinMsgId;
		}
		m_oLastSavedMsgId.Save();
	}
	
	return m_nMsgId;
}
