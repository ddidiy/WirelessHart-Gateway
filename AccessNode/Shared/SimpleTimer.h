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

// SimpleTimer.h: interface for the CSimpleTimer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SIMPLETIMER_H__67C78686_4D6B_4902_A3A4_41BF243E0340__INCLUDED_)
#define AFX_SIMPLETIMER_H__67C78686_4D6B_4902_A3A4_41BF243E0340__INCLUDED_

/// @addtogroup libshared
/// @{

#include "../Shared/Common.h"
class CSimpleTimer
{
public:
	CSimpleTimer(u_int p_nMS = 0)
		:m_n_SC_CLK_TCK(sysconf( _SC_CLK_TCK )) { SetTimer(p_nMS); }
	virtual ~CSimpleTimer() {}

public:
	void SetTimer(u_int p_nMS)
	{
		m_nNextSignal = GetClockTicks() + p_nMS * m_n_SC_CLK_TCK / 1000;
	}

	void UpdateTimer(u_int p_nMS)
	{
		clock_t nNextSignal = GetClockTicks() + p_nMS * m_n_SC_CLK_TCK / 1000;

		if (nNextSignal > m_nNextSignal)
		{
			m_nNextSignal = nNextSignal;
		}
	}

	int IsSignaling()
	{
		clock_t nCrtTime = GetClockTicks();

		if ((m_nNextSignal > 1700 * 1000 * 1000) && (nCrtTime < 100 * 1000))
		{	m_nNextSignal = nCrtTime;
		}

		if (nCrtTime < m_nNextSignal)
		{	return 0;
		}
		return 1;
	}

	int	GetNextInterval() { return (m_nNextSignal-GetClockTicks())*1000/m_n_SC_CLK_TCK;}


private:
	clock_t m_nNextSignal;
	long m_n_SC_CLK_TCK;
};

/// @}
#endif // !defined(AFX_SIMPLETIMER_H__67C78686_4D6B_4902_A3A4_41BF243E0340__INCLUDED_)
