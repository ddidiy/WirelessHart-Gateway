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

// IdsBooking.cpp: implementation of the CIdsBooking class.
//
//////////////////////////////////////////////////////////////////////

#include "IdsBooking.h"
#include "Common.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIdsBooking::CIdsBooking(int p_nMin, int p_nMax)
{
	m_nMin = p_nMin;
	m_nMax = p_nMax;

	if (m_nMin > m_nMax)
	{	m_nMax = m_nMin + 10000;
		LOG("CIdsBooking: m_nMin > m_nMax");
	}

	TInterval tMin = {m_nMin,m_nMin};
	m_oIdList.AddHead(tMin);

	TInterval tMax = {m_nMax,m_nMax};

	m_nLastAdd = m_nMax;
	m_oIdList.AddTail(tMax);
}

CIdsBooking::~CIdsBooking()
{
	
}

int CIdsBooking::Add(int p_nId)
{
	if (p_nId <= m_nMin || p_nId >= m_nMax)
	{
		LOG("CIdsBooking::Add: id %d out of range [%d,%d]", p_nId, m_nMin, m_nMax);
		return 0;
	}

	OBJ_POSITION pos1 = m_oIdList.GetHeadPosition();

	if (!pos1)
	{
		LOG("CIdsBooking::Add: SW error atleast one interval should exist [min,max]");
		return 0;
	}

	OBJ_POSITION pos2 = pos1;
	m_oIdList.GetNext(pos2);

	if (!pos2)
	{
		LOG("CIdsBooking::Add: WARN full");
		return 1;
	}

	for (;pos2;m_oIdList.GetNext(pos2))
	{
		TInterval& t2 = m_oIdList.GetAt(pos2);

		if (t2.m_nEnd >= p_nId)
		{	break;
		}

		pos1 = pos2;
	}
	
	if (!pos2)
	{	
		LOG("CIdsBooking::Add: SW error !!! ");
		return 0;
	}
	
	m_nLastAdd = p_nId;
	
	TInterval& t1 = m_oIdList.GetAt(pos1);
	TInterval& t2 = m_oIdList.GetAt(pos2);


	if (	(t1.m_nStart <= p_nId && p_nId <= t1.m_nEnd)
		||	(t2.m_nStart <= p_nId && p_nId <= t2.m_nEnd)
		)
	{	return 1;
	}

	if ((t1.m_nEnd + 1) == p_nId)
	{	
		t1.m_nEnd++;

		if (t1.m_nEnd + 1 == t2.m_nStart)
		{
			//join nodes
			t1.m_nEnd = t2.m_nEnd;
			m_oIdList.RemoveAt(pos2);		
		}
		return 1;
	}

	if ((t2.m_nStart - 1) == p_nId)
	{	
		t2.m_nStart--;

		if (t1.m_nEnd + 1 == t2.m_nStart)
		{
			//join nodes
			t1.m_nEnd = t2.m_nEnd;
			m_oIdList.RemoveAt(pos2);
		}
		return 1;
	}

	TInterval t3 = {p_nId,p_nId};

	m_oIdList.InsertAfter(pos1,t3);

	return 1;
}

int CIdsBooking::GetNewId()
{
	OBJ_POSITION pos1 = m_oIdList.GetHeadPosition();

	if (!pos1)
	{
		LOG("CIdsBooking::GetNewId: atleast one interval should exist [min.max]");
		return m_nMax; //invalid 
	}

	OBJ_POSITION pos2 = pos1;
	m_oIdList.GetNext(pos2);

	if (!pos2)
	{
		LOG("CIdsBooking::GetNewId: full ");
		return m_nMax; //invalid value
	}

	TInterval& t1 = m_oIdList.GetAt(pos1);
	TInterval& t2 = m_oIdList.GetAt(pos2);

	int nId = ++t1.m_nEnd;	//invalid value	
	if (t1.m_nEnd + 1 == t2.m_nStart)
	{
		//join nodes
		t1.m_nEnd = t2.m_nEnd;
		m_oIdList.RemoveAt(pos2);	
	}
	
	m_nLastAdd = nId;
	return nId;	
}

int CIdsBooking::GetNewId2()
{
	if (m_nLastAdd <= m_nMin || m_nLastAdd >= m_nMax)
	{
		return GetNewId();
	}

	OBJ_POSITION pos1 = m_oIdList.GetHeadPosition();

	if (!pos1)
	{
		LOG("CIdsBooking::GetNewId2: SW error atleast one interval should exist [min,max]");
		return m_nMax;
	}

	OBJ_POSITION pos2 = pos1;
	m_oIdList.GetNext(pos2);

	if (!pos2)
	{
		LOG("CIdsBooking::GetNewId2: WARN full");
		return m_nMax;
	}

	m_nLastAdd++;

	for (;pos2;m_oIdList.GetNext(pos2))
	{
		TInterval& t2 = m_oIdList.GetAt(pos2);

		if (t2.m_nStart > m_nLastAdd)
		{	break;
		}

		if (t2.m_nEnd >= m_nLastAdd)
		{
			m_nLastAdd = t2.m_nEnd + 1;
		}

		pos1 = pos2;
	}
	
	if (!pos2)
	{	
		return GetNewId();
	}
		
	TInterval& t1 = m_oIdList.GetAt(pos1);
	TInterval& t2 = m_oIdList.GetAt(pos2);


	if (	(t1.m_nStart <= m_nLastAdd && m_nLastAdd <= t1.m_nEnd)
		||	(t2.m_nStart <= m_nLastAdd && m_nLastAdd <= t2.m_nEnd)
		)
	{	m_nLastAdd = t1.m_nEnd + 1;
	}

	if ((t1.m_nEnd + 1) == m_nLastAdd)
	{	
		t1.m_nEnd++;

		if (t1.m_nEnd + 1 == t2.m_nStart)
		{
			//join nodes
			t1.m_nEnd = t2.m_nEnd;
			m_oIdList.RemoveAt(pos2);		
		}
		return m_nLastAdd;
	}

	if ((t2.m_nStart - 1) == m_nLastAdd)
	{	
		t2.m_nStart--;

		if (t1.m_nEnd + 1 == t2.m_nStart)
		{
			//join nodes
			t1.m_nEnd = t2.m_nEnd;
			m_oIdList.RemoveAt(pos2);
		}
		return m_nLastAdd;
	}

	TInterval t3 = {m_nLastAdd,m_nLastAdd};

	m_oIdList.InsertAfter(pos1,t3);

	return m_nLastAdd;
}


void CIdsBooking::Remove(int p_nId)
{
	if (p_nId <= m_nMin || p_nId >= m_nMax)
	{
		LOG("CIdsBooking::Remove: id %d out of range (%d,%d)", p_nId, m_nMin, m_nMax);
		return;
	}

	OBJ_POSITION pos = m_oIdList.GetHeadPosition();

	if (!pos)
	{
		LOG("CIdsBooking::Remove: SW error atleast one interval should exist [min.max]");
		return;
	}

	for (;pos;m_oIdList.GetNext(pos))
	{
		TInterval& t = m_oIdList.GetAt(pos);

		if (p_nId < t.m_nStart || t.m_nEnd < p_nId )
		{	continue;
		}

		if (t.m_nStart == t.m_nEnd)
		{
			m_oIdList.RemoveAt(pos);
			return;
		}

		if (t.m_nStart == p_nId)
		{	t.m_nStart++;
			return;	
		}

		if (t.m_nEnd == p_nId)
		{	t.m_nEnd--;			
			return;
		}

		TInterval newT = {p_nId+1, t.m_nEnd};	

		t.m_nEnd = p_nId - 1;

		m_oIdList.InsertAfter(pos, newT);
		return;	
	}

	LOG("CIdsBooking::Remove: Id=%d not found", p_nId);
}

void CIdsBooking::Log()
{
	LOG("MeshIds:");
	OBJ_POSITION pos = m_oIdList.GetHeadPosition();
	for (;pos;m_oIdList.GetNext(pos))
	{	TInterval t = m_oIdList.GetAt(pos);
		LOG("\t[%d,%d]", t.m_nStart, t.m_nEnd);
	}
}

