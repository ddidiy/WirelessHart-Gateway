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

// HashTable.h: interface for the CHashTable class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HASHTABLE_H__798DEDCE_6691_4549_AE03_7E5F604EB694__INCLUDED_)
#define AFX_HASHTABLE_H__798DEDCE_6691_4549_AE03_7E5F604EB694__INCLUDED_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


typedef void* HASH_POSITION;

/// @addtogroup libshared
/// @{
template<typename CKey, typename CValue>
class CHashTable  
{
protected:
	class THashElem
	{		
	public:
		CKey	m_oKey;
		CValue	m_oValue;
		THashElem* m_pNext;
	};

public:
	CHashTable(unsigned int p_nSize = 128)
	{	
		m_nSize = p_nSize;
		m_pTable = new THashElem*[m_nSize];

		memset(m_pTable, 0, m_nSize * sizeof(THashElem*) );
	}
	virtual ~CHashTable() { Reset(); delete []m_pTable; }

	void SetSize(unsigned int p_nSize )
	{	
		Reset();
		delete []m_pTable;
		m_nSize = p_nSize;

		m_pTable = new THashElem*[m_nSize];

		memset(m_pTable, 0, m_nSize * sizeof(THashElem*) );
	}

	void Reset()
	{
		for (u_int i = 0; i < m_nSize; i++ ) 
		{	
			for( THashElem* pIt = m_pTable[i]; pIt; )
			{	
				THashElem* pDel = pIt;
				pIt = pIt->m_pNext;
				
				delete pDel;
			}
		}
		memset(m_pTable, 0, m_nSize * sizeof(int) );
	}

	void Set( const CKey& p_rKey, const CValue& p_rValue )
	{	
		THashElem* pElem = findKey(p_rKey);

		if (pElem) 
		{	pElem->m_oValue = p_rValue;
			return;
		}

		THashElem* pTmp = new THashElem;
		pTmp->m_oKey = p_rKey;
		pTmp->m_oValue = p_rValue;

		int nLista = hash(p_rKey);
		pTmp->m_pNext = m_pTable[nLista]; 
		m_pTable[nLista] = pTmp;
	}
	
	bool Get( const CKey& p_rKey, CValue* p_pValue)
	{
		THashElem* pElem = findKey(p_rKey);

		if (p_pValue && pElem) 
		{	*p_pValue = pElem->m_oValue;
		}
		return pElem;
	}

	CValue* Get (const CKey& p_rKey)
	{
		THashElem* pElem = findKey(p_rKey);

		return pElem ? &pElem->m_oValue : NULL;
	}

	HASH_POSITION GetFirstPos()
	{
		unsigned int i;
		for ( i=0; i < m_nSize; i++ ) 
		{	
			if (!m_pTable[i]) 
			{	continue;
			}
			
			return (HASH_POSITION)m_pTable[i];			
		}
		return NULL;		
	}

	CValue& GetValueAt (HASH_POSITION p_oPos)
	{
		//expect p_rPos != NULL, if not better crash than have hard to detect bug
		return ((THashElem*)p_oPos)->m_oValue;
	}

	CKey& GetKeyAt (HASH_POSITION p_oPos)
	{
		//expect p_rPos != NULL, if not better crash than have hard to detect bug
		return ((THashElem*)p_oPos)->m_oKey;
	}


	void GetNext (HASH_POSITION& p_rPos)
	{
		//expect p_rPos != NULL, if not better crash than have hard to detect bug
		THashElem* pElem = (THashElem*)p_rPos;
		
		if (pElem->m_pNext)
		{	p_rPos = pElem->m_pNext;
			return;
		}

		unsigned int i = hash(pElem->m_oKey) + 1;
		
		for (; i < m_nSize; i++ ) 
		{	
			if (!m_pTable[i]) 
			{	continue;
			}
			
			p_rPos = m_pTable[i];
			return;			
		}
		p_rPos = NULL;		
	}


	void Remove(const CKey& p_rKey)
	{
		THashElem** ppIt = &m_pTable[hash(p_rKey)];

		for( ; *ppIt; ppIt = &(*ppIt)->m_pNext)
		{	
			if( (*ppIt)->m_oKey == p_rKey) 
			{
				THashElem* pDel = *ppIt;
				*ppIt = pDel->m_pNext;

				delete pDel;
				break;
			}			
		}
	}


#ifdef __DEBUG_ 
	void PrintDebug()
	{
		printf("\nHashTable:\n");
		for (u_int i = 0; i < m_nSize; i++ ) 
		{	
			if (!m_pTable[i]) 
			{	continue;
			}
			printf("%d: ",i);
			for( THashElem* pIt = m_pTable[i]; pIt; pIt = pIt->m_pNext)
			{	
				printf("(%d,%d) ", pIt->m_oKey, pIt->m_oValue);
			}
			printf("\n");
		}
		printf("------------------------\n");
	}
#endif

protected:
	THashElem* findKey(const CKey& p_rKey )
	{	
		THashElem* pElem = m_pTable[hash(p_rKey)];

		for (;pElem && pElem->m_oKey != p_rKey; pElem = pElem->m_pNext) 
			;

		return pElem;
	}

	unsigned int hash(const CKey& p_oKey) 
	{ 
		if (sizeof(CKey) <= sizeof(unsigned int))
		{	return (unsigned int)p_oKey % m_nSize;
		}

		return *((unsigned int*)&p_oKey) % m_nSize;		
	}

protected:
	unsigned int	m_nSize;
	THashElem**		m_pTable;

};

/// @}
#endif // !defined(AFX_HASHTABLE_H__798DEDCE_6691_4549_AE03_7E5F604EB694__INCLUDED_)
