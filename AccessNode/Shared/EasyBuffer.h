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

#ifndef __EASY_BUFFER_H
#define __EASY_BUFFER_H

#include <string.h>

// type T must be type that require only bit copy operator
/// @addtogroup libshared
/// @{
template <typename T>
class CEasyBuffer
{
public:
	CEasyBuffer (int p_nSize = 32)
	{
		m_pBuff = new T[p_nSize];
		m_nMax = p_nSize;
		m_nUsed = 0;
	}

	CEasyBuffer (const CEasyBuffer& p_rEasyBuff)
	{
		*this = p_rEasyBuff;
	}

	virtual ~CEasyBuffer()
	{
		if (m_pBuff)
		{	delete []m_pBuff;
		}
	}

	int		GetSize() const { return m_nUsed; }

	void	Clear()
	{
		m_nUsed = 0;
		delete []m_pBuff;
		m_pBuff = NULL;
		m_nMax = 0;
	}

	T* Get() const	{	return m_pBuff;	}
	operator T*() const	{	return m_pBuff;	}

	T& operator [](int i) { Assure(i); return *(m_pBuff + i); }

	CEasyBuffer& operator = (const CEasyBuffer& p_rEasyBuff)
	{
		if (p_rEasyBuff->m_nMax > m_nMax)
		{
			delete m_pBuff;

			m_pBuff = new T[m_nMax];
			m_nMax = p_rEasyBuff->m_nMax;
		}

		m_nUsed = p_rEasyBuff->m_nUsed;
		memcpy(m_pBuff, p_rEasyBuff->m_pBuff, p_rEasyBuff->m_nMax);
		return *this;
	}

	void Assure(int nPos)
	{
		if (nPos < m_nUsed)
		{	return;
		}

		if (nPos < m_nMax)
		{	m_nUsed = nPos + 1;
			return;
		}

		T* pBuff = new T[nPos+1];

		memcpy(pBuff, m_pBuff, m_nUsed);
		m_nMax = nPos + 1;
		delete m_pBuff;

		m_pBuff = pBuff;
		m_nUsed = nPos + 1;
	}

	void Append( const T* p_pBuff, int p_nLen )
	{
		Set(GetSize(),p_pBuff,p_nLen);
	}

	void Set (int p_nPos, const T* p_pBuff, int p_nLen)
	{
		if (p_nLen <= 0)
		{	return;
		}
		Assure (p_nPos + p_nLen - 1);

		memcpy(m_pBuff + p_nPos, p_pBuff, p_nLen);
	}


private:
	T*		m_pBuff;
	int		m_nMax;
	int		m_nUsed;
};

/// @}
#endif
