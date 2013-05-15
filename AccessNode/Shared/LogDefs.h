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

#ifndef LogDefs_h__
#define LogDefs_h__


#include "log.h"
#include "log2flash.h"
#include "log_callback.h" //for LOGLVL

/// @addtogroup libshared
/// @{
typedef enum {
	NLOG_LVL_NONE	= 0,	/// turn off all logs ONLY for test
	NLOG_LVL_ERR	= 10,	/// ERROR messages
	NLOG_LVL_WARN	= 20,	///
	NLOG_LVL_INF	= 30,	///
	NLOG_LVL_DBG	= 40,			/// User should NOT log this level, except for  debugging
	NLOG_LVL_UNK	= 255
} NLOG_LVL;

//log object
extern CLog			g_stLog;
extern NLOG_LVL		g_nNLogLevel;

extern "C" void CallbackLog( const char* p_szMsg );
extern "C" void CallbackLogWithLevel( LOGLVL p_eLogLvl, const char* p_szMsg );

#define LOG g_stLog.WriteMsg
#define LOG_HEX g_stLog.WriteHexMsg
#define LOG_ERR(format,...) LOG("OLD:"format":%s",##__VA_ARGS__,strerror(errno))
#define LOG_STR g_stLog.WriteString

#define LOG_IN		g_stLog.WriteInLine
#define LOG_DTIME	g_stLog.WriteTime
#define LOG_EOL		g_stLog.WriteEol

//used by LG
#define LOG_STATS(x...)       LOG("STATS::"x)
//#define LOG_STATS(x...) do {} while(0)

#define LOG_MESH g_stLog.WriteMsg

#define LOG_TESTING g_stLog.DoNothing
//#define LOG_TESTING LOG

#define LOG_N_HEX(pMsg, pBin, nLen, nPrint )					\
{																\
	int nPrintLen = _Min(nLen, nPrint);								\
	char afis[128];	sprintf( afis, "%s%s", pMsg, "%s %s");							\
	g_stLog.WriteMsg( afis, GetHex( pBin, nPrintLen, ' '), nLen>nPrintLen ? "...": "" );	\
}

#define LOG_UNUSED_COD LOG( "OLD CODE, UNUSED here; check code and remove this log; in file=%s, line=%d", __FILE__, __LINE__ );
#define LOG_ASSERT(msg) LOG( "ASSERT : %s in file=%s, line=%d", msg, __FILE__, __LINE__ );

#define LOG_TRACK_POINT_PF() LOG("TRACK: %s; %s; line=%d", __FILE__, __PRETTY_FUNCTION__, __LINE__);
#define LOG_TRACK_POINT() LOG("TRACK: %s; %s; line=%d", __FILE__, __FUNCTION__, __LINE__);

#define LOG_TRACK_POINT_TIMED(nDuration)	(TrackPointTimeout(nDuration, __FILE__, __FUNCTION__, __LINE__) )



#define NLOG_ERR(...)	{	if (g_nNLogLevel >= NLOG_LVL_ERR)	LOG("ERROR:"__VA_ARGS__);	}
#define NLOG_WARN(...)  {	if (g_nNLogLevel >= NLOG_LVL_WARN)	LOG("WARN:"__VA_ARGS__);		}
#define NLOG_INFO(...)  {	if (g_nNLogLevel >= NLOG_LVL_INF)	LOG("INFO:"__VA_ARGS__);		}
#define NLOG_DBG(...)	{	if (g_nNLogLevel >= NLOG_LVL_DBG)	LOG("DEBUG:"__VA_ARGS__);	}


#define LOG_S(msg)			{	CLog::StreamLog(std::stringstream().flush() <<msg);	}
#define NLOG_ERR_S(msg)		{	if (g_nNLogLevel >= NLOG_LVL_ERR)	CLog::StreamLog(std::stringstream().flush() <<msg);	}
#define NLOG_WARN_S(msg)	{	if (g_nNLogLevel >= NLOG_LVL_WARN)	CLog::StreamLog(std::stringstream().flush() <<msg);	}
#define NLOG_INFO_S(msg)  	{	if (g_nNLogLevel >= NLOG_LVL_INF)	CLog::StreamLog(std::stringstream().flush() <<msg);	}
#define NLOG_DBG_S(msg)		{	if (g_nNLogLevel >= NLOG_LVL_DBG)	CLog::StreamLog(std::stringstream().flush() <<msg);	}


#define NLOG_ERR_HEX(...)	{	if (g_nNLogLevel >= NLOG_LVL_ERR)	LOG_HEX("ERROR:"__VA_ARGS__);	}
#define NLOG_WARN_HEX(...)  {	if (g_nNLogLevel >= NLOG_LVL_WARN)	LOG_HEX("WARN:"__VA_ARGS__);		}
#define NLOG_INFO_HEX(...)  {	if (g_nNLogLevel >= NLOG_LVL_INF)	LOG_HEX("INFO:"__VA_ARGS__);		}
#define NLOG_DBG_HEX(...)	{	if (g_nNLogLevel >= NLOG_LVL_DBG)	LOG_HEX("DEBUG:"__VA_ARGS__);	}

// log message and error string of errno
#define NLOG_SYSC_ERR(format,...)	{	if (g_nNLogLevel >= NLOG_LVL_ERR)	LOG("ERROR:"format":%s",__VA_ARGS__,strerror(errno));	}
#define NLOG_SYSC_WARN(format,...)  {	if (g_nNLogLevel >= NLOG_LVL_WARN)	LOG("WARN:"format":%s",__VA_ARGS__,strerror(errno));	}


///////////////////////////////////////////////////////////////////////////////
//! Prefixed Log Macros.
///////////////////////////////////////////////////////////////////////////////
#define F1LOG(file, line, func, format,...) g_stLog.WriteMsg( "%u:%s@%s +%d> "format, getpid(), func, file, line, ##__VA_ARGS__)
#define FLOG(format,...) F1LOG(__FILE__, __LINE__, __func__, format, ##__VA_ARGS__)

#define ERR(...)  LOG("ERR:"__VA_ARGS__)
#define WARN(...) LOG("WARN:"__VA_ARGS__)

// log message and error string of errno
#define PERR(format,...)  ERR(format":%s",##__VA_ARGS__,strerror(errno))
#define PWARN(format,...) WARN(format":%s",##__VA_ARGS__,strerror(errno))

// log file location and log message
#define FERR(format,...)  FLOG("ERR:"format,##__VA_ARGS__)
#define FWARN(format,...) FLOG("WARN:"format, ##__VA_ARGS__)

// log file location, log message and error string of errno
#define FPERR(format,...) FERR(format,":%s",##__VA_ARGS__,strerror(errno))
#define FPWARN(format,...) FWARN(format,":%s",##__VA_ARGS__,strerror(errno))


/// @}
#endif // LogDefs_h__
