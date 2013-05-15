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

//only to be used for duration measure in tests

#ifndef DURATION_MEASURE_H_
#define DURATION_MEASURE_H_

#include <sys/times.h>
#include <stdio.h>


#ifndef LOG
#define LOG printf
#endif
//#define LOG_ERROR printf

/// @ingroup libshared
class CProcessTimesMeasure
{

public:
    const long int CLOCK_TCK;
	CProcessTimesMeasure(long int CLOCK_TCK_ =  sysconf( _SC_CLK_TCK )): CLOCK_TCK(CLOCK_TCK_)
	{
		ReadTime();
	}
	CProcessTimesMeasure( const CProcessTimesMeasure& p_rProcTime ):CLOCK_TCK(p_rProcTime.CLOCK_TCK)
	{
		*this = p_rProcTime;
	}
	CProcessTimesMeasure&  operator = (const CProcessTimesMeasure& p_rProcTime)
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

	int  ProcGetUserDiff (const CProcessTimesMeasure& p_rProcTime)
	{
		return (int)(m_nLastTimeUser - p_rProcTime.m_nLastTimeUser) * 1000 / CLOCK_TCK;
	}

	int  ProcGetSysDiff (const CProcessTimesMeasure& p_rProcTime)
	{
		return (int)(m_nLastTimeSys - p_rProcTime.m_nLastTimeSys) * 1000 / CLOCK_TCK;
	}

	int  ProcGetTotalDiff (const CProcessTimesMeasure& p_rProcTime)
	{
		return ProcGetUserDiff(p_rProcTime) + ProcGetSysDiff(p_rProcTime);
	}

	int  RealGetDiff (const CProcessTimesMeasure& p_rProcTime)
	{
		return (m_nLastTimeReal - p_rProcTime.m_nLastTimeReal) * 1000 / CLOCK_TCK;
	}


private:
	clock_t m_nLastTimeUser;
	clock_t m_nLastTimeSys;
	clock_t	m_nLastTimeReal;
};

/// @ingroup libshared
class CDurationMeasure
{
    const long int CLOCK_TCK;
public:


	CDurationMeasure():CLOCK_TCK( sysconf( _SC_CLK_TCK )), m_oLastTime(CLOCK_TCK){}

	// @  p_nDuration - msec
	int WatchDuration (const char *p_szFile, const char* p_szFunc, int p_nLine, int p_nRealDuration, int p_nProcessDuration)
	{
		CProcessTimesMeasure oCrtTime(CLOCK_TCK);

		int ret = (oCrtTime.RealGetDiff(m_oLastTime) > p_nRealDuration || oCrtTime.ProcGetTotalDiff(m_oLastTime) > p_nProcessDuration ) ;
		if (ret)
		{
			//char buffer[3000];

			printf( "WatchDuration: %s; %s; line=%d real=%dms (l=%dms), proc=%dms (l=%dms) usr=%dms sys=%dms", p_szFile, p_szFunc, p_nLine,
					oCrtTime.RealGetDiff(m_oLastTime), p_nRealDuration, oCrtTime.ProcGetTotalDiff(m_oLastTime),  p_nProcessDuration,
					oCrtTime.ProcGetUserDiff(m_oLastTime), oCrtTime.ProcGetSysDiff(m_oLastTime)
				 );

			//printf(buffer);
		}

		m_oLastTime = oCrtTime;

		return ret;
	}

	void Start()
	{
		m_oLastTime.ReadTime();
	}
	// @  p_nDuration - msec
	int LogDuration (const char * p_szlogMessage, int p_nIteration)
	{
		CProcessTimesMeasure oCrtTime(CLOCK_TCK);

		int userTotal = oCrtTime.ProcGetTotalDiff(m_oLastTime);
		double iter = userTotal/(double)p_nIteration;
		printf( "LogDuration: real=%4dms proc=%4dms usr=%4dms sys=%4dms it=%4.6f ops/sec=%9d <- %s\n",
					oCrtTime.RealGetDiff(m_oLastTime), userTotal, 
					oCrtTime.ProcGetUserDiff(m_oLastTime), oCrtTime.ProcGetSysDiff(m_oLastTime), 
					iter, iter ? (int)(1000/iter) : -1, p_szlogMessage
				 );
	
		m_oLastTime = oCrtTime;		
		return  userTotal;
	}

private:
	CProcessTimesMeasure m_oLastTime;
};




#endif /* DURATION_MEASURE_H_ */
