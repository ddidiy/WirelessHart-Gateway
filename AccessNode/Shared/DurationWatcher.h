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

#ifndef DurationWatcher_h__
#define DurationWatcher_h__

#include "Common.h"

#include <sys/times.h> 
/// @addtogroup libshared
/// @{
class CProcessTimes
{
public:
	CProcessTimes()
	:m_n_SC_CLK_TCK(sysconf( _SC_CLK_TCK ))
	{
		ReadTime();
	}
	CProcessTimes( const CProcessTimes& p_rProcTime )
	:m_n_SC_CLK_TCK(sysconf( _SC_CLK_TCK ))
	{
		*this = p_rProcTime;
	}
	CProcessTimes&  operator = (const CProcessTimes& p_rProcTime)
	{
		m_nLastTimeReal = p_rProcTime.m_nLastTimeReal;
		m_nLastTimeUser = p_rProcTime.m_nLastTimeUser;
		m_nLastTimeSys	= p_rProcTime.m_nLastTimeSys;
		return *this;
	}
		
	void ReadTime()
	{
		struct tms tms_buf;	
		int nCrtTime = times( &tms_buf );

		m_nLastTimeReal = nCrtTime;
		m_nLastTimeUser = tms_buf.tms_utime;
		m_nLastTimeSys = tms_buf.tms_stime;
	}	
	
	int  ProcGetUserDiff (const CProcessTimes& p_rProcTime)
	{
		return (int)(m_nLastTimeUser - p_rProcTime.m_nLastTimeUser) * 1000 / m_n_SC_CLK_TCK;
	}

	int  ProcGetSysDiff (const CProcessTimes& p_rProcTime)
	{
		return (int)(m_nLastTimeSys - p_rProcTime.m_nLastTimeSys) * 1000 / m_n_SC_CLK_TCK;
	}

	int  ProcGetTotalDiff (const CProcessTimes& p_rProcTime)
	{
		return ProcGetUserDiff(p_rProcTime) + ProcGetSysDiff(p_rProcTime);
	}

	int  RealGetDiff (const CProcessTimes& p_rProcTime)
	{
		return (m_nLastTimeReal - p_rProcTime.m_nLastTimeReal) * 1000 / m_n_SC_CLK_TCK;
	}
		

private:
	clock_t m_nLastTimeUser;
	clock_t m_nLastTimeSys;
	clock_t	m_nLastTimeReal; 
	long m_n_SC_CLK_TCK;
};




/// @ingroup libshared
class CDurationWatcher
{
public:

	CDurationWatcher (const char *p_szFile = NULL, const char* p_szFunc = NULL, int p_nLine = 0, int p_nRealDuration = -1, int p_nProcessDuration = -1):
		m_szFile(p_szFile), m_szFunc(p_szFunc), m_nLine(-p_nLine), m_nRealDuration(p_nRealDuration), m_nProcessDuration(p_nRealDuration)
	{
		
	}

	~CDurationWatcher()
	{
		if (m_szFile && m_szFunc && m_nProcessDuration>=0 && m_nRealDuration >=0)
		{
			WatchDuration(m_szFile, m_szFunc, m_nLine, m_nRealDuration, m_nProcessDuration);
		}
	}

	// @  p_nDuration - msec 
	int WatchDuration (const char *p_szFile, const char* p_szFunc, int p_nLine, int p_nRealDuration, int p_nProcessDuration)
	{
		CProcessTimes oCrtTime;	
		
		int ret = (oCrtTime.RealGetDiff(m_oLastTime) > p_nRealDuration || oCrtTime.ProcGetTotalDiff(m_oLastTime) > p_nProcessDuration ) ;
		if (ret)
		{
			LOG("WatchDuration: %s; %s; line=%d real=%dms (l=%dms), proc=%dms (l=%dms) usr=%dms sys=%dms", p_szFile, p_szFunc, p_nLine, 
					oCrtTime.RealGetDiff(m_oLastTime), p_nRealDuration, oCrtTime.ProcGetTotalDiff(m_oLastTime),  p_nProcessDuration, 
					oCrtTime.ProcGetUserDiff(m_oLastTime), oCrtTime.ProcGetSysDiff(m_oLastTime)
				 );
		}

		m_oLastTime = oCrtTime;

		return ret;
	}

	int GetRealDuration()		{ return m_nRealDuration; } 
	int GetProcessDuration()	{ return m_nProcessDuration; } 

private:
	CProcessTimes	m_oLastTime;	
	const char*		m_szFile;
	const char*		m_szFunc;
	int				m_nLine;
	int				m_nRealDuration;
	int				m_nProcessDuration;
};

// duration in ms
#define DurationWatcherReal		(400)
#define DurationWatcherProc		(100)

#ifndef DISABLE_WATCH_DURATION

#define WATCH_DURATION_INIT_DEF(obj) CDurationWatcher obj(__FILE__,__FUNCTION__,__LINE__,DurationWatcherReal,DurationWatcherProc)  
#define WATCH_DURATION_INIT(obj,real_dur,proc_dur) CDurationWatcher obj(__FILE__,__FUNCTION__,__LINE__,real_dur, proc_dur) 

#define WATCH_DURATION_DEF(obj) ((obj).WatchDuration( __FILE__, __FUNCTION__, __LINE__,DurationWatcherReal,DurationWatcherProc)) 
#define WATCH_DURATION(obj,real_dur,proc_dur) ((obj).WatchDuration( __FILE__, __FUNCTION__, __LINE__,real_dur,proc_dur)) 
#define WATCH_DURATION_OBJ(obj) ((obj).WatchDuration( __FILE__, __FUNCTION__, __LINE__,(obj).GetRealDuration(),(obj).GetProcessDuration())) 

#else

#define WATCH_DURATION_INIT_DEF(obj) 
#define WATCH_DURATION_INIT(obj,real_dur,proc_dur) 

#define WATCH_DURATION_DEF(obj) 
#define WATCH_DURATION(obj,real_dur,proc_dur) 
#define WATCH_DURATION_OBJ(obj,real_dur,proc_dur) 

#endif

/// @}
#endif // DurationWatcher_h__
