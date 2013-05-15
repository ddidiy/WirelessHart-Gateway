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

/**
 * @file Flog.h
 * @date Mar 22, 2010
 * @author Marius Negreanu
 */

#ifndef _FLOG_H_
#define _FLOG_H_

#include <ostream>
#include <sstream>
#include <stdio.h>
#include <stdarg.h>

/**
 * @class CFLogSink
 * @brief Log Message destination. The received message might be written to
 * a file or sent on UDP or both. This decouples the Logging interface from
 * the logging actions.
 */
class CFLogSink {
public:
	CFLogSink(const char* path, const char*mode="w") { }
	virtual ~CFLogSink() {}
public:
	virtual bool dissociate( ) = 0 ;
	virtual void consume( const std::ostringstream& msg  )   =0 ;
	virtual void consume( const char* message, va_list ap  ) =0 ;
protected:
	CFLogSink() {}
	FILE* m_pFile;
};

/**
 * @class CFLog
 * @brief Logging Frontend. Takes string or stream messages and sends them to a
 * CFLogSink.
 */
class CFLog {
public:
	enum LogLevel
	{
		LL_ERROR = 1,
		LL_WARN,
		LL_INFO,
		LL_DEBUG,
		LL_FATAL,
		LL_MAX_LEVEL
	};
public:
	CFLog() ;
	virtual ~CFLog();
	void WriteMsg( const char* message, ... );
	void WriteMsg( const std::ostream& message );
	bool SetLogLevel(enum LogLevel logLevel)
	{
		if ( logLevel >= LL_MAX_LEVEL )
			return false ;
		m_eLogLevel = logLevel;
		return true;
	}
	template<typename T>
	std::stringstream& operator<<(T p)
	{
		m_oOutStr << p;
		return this->m_oOutStr ;
	}
	template <LogLevel level>
	bool IsLogEnabled() const
	{
		return level <= m_eLogLevel ? true : false;
	}
	void LogSink(CFLogSink* l) { m_pLogSink = l ; }
protected:
	std::stringstream  m_oOutStr ;
	std::ostringstream m_oStrStream ;
	enum LogLevel      m_eLogLevel ;
	CFLogSink*         m_pLogSink ;
};

extern CFLog g_stFlog ;



///////////////////////////////////////////////////////////////////////////////
#if __GNUC__ >= 3

#if !defined(N_UNLIKELY)
 #define N_UNLIKELY(expr) __builtin_expect(expr, 0)
#endif

#if !defined(N_LIKELY)
 #define N_LIKELY(expr) __builtin_expect(expr, 1)
#endif

#else
#define N_UNLIKELY(expr) expr
#define N_LIKELY(expr) expr

#endif


#define Paste(a,b) a ##_## b
#define PASTE(a,b) Paste(a,b)

// The PP_NARG macro returns 0 when it has zero __VA_ARGS__ parameters and X otherwise
#define PP_NARG(...) \
         PP_NARG_(__VA_ARGS__,PP_RSEQ_N())

#define PP_NARG_(...) \
         PP_ARG_N(__VA_ARGS__)

#define PP_ARG_N( \
          _1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,N,...) N

#define PP_RSEQ_N() \
           X, X, X, X, X, X, X, X, X, X, 0,0


// LOG_INFO calls either 1) or 2) depending on the number of parameters
// Ex:
// LOG_INFO( "cucu" << "rucu" )  calls 1)
// LOG_INFO( "cucu%s", rucu)     calls 2)
// 1) WriteMsg( ostream& message )
// 2) WriteMsg( const char*, ... )
#ifndef LOG_OBJECT
#define LOG_OBJECT g_stFlog
#endif

#define LOG_DEBUG(message,...) \
	LOG_DEBUG_TO(LOG_OBJECT,message,##__VA_ARGS__)
#define LOG_DEBUG_TO(logger, message,...) \
	PASTE(LOG_DEBUG_TO,PP_NARG(1,##__VA_ARGS__))(logger,message,##__VA_ARGS__)
#define LOG_DEBUG_TO_0(logger,message,...)\
	do { if ( N_UNLIKELY( logger.IsLogEnabled<CFLog::LL_DEBUG>() ) )  logger.WriteMsg( logger << message );}while(0);
#define LOG_DEBUG_TO_X(logger,message,...)\
	do { if ( N_UNLIKELY( logger.IsLogEnabled<CFLog::LL_DEBUG>() ) )  logger.WriteMsg( message,##__VA_ARGS__ );}while(0);


#define LOG_INFO(message,...) \
	LOG_INFO_TO(LOG_OBJECT,message,##__VA_ARGS__)
#define LOG_INFO_TO(logger, message,...) \
	PASTE(LOG_INFO_TO,PP_NARG(1,##__VA_ARGS__))(logger,message,##__VA_ARGS__)
#define LOG_INFO_TO_0(logger,message,...)\
	do { if ( N_LIKELY( logger.IsLogEnabled<CFLog::LL_INFO>() ) )  logger.WriteMsg( logger << message );}while(0);
#define LOG_INFO_TO_X(logger,message,...)\
	do { if ( N_LIKELY( logger.IsLogEnabled<CFLog::LL_INFO>() ) )  logger.WriteMsg( message,##__VA_ARGS__ );}while(0);


#define LOG_WARN(message,...) \
	LOG_WARN_TO(LOG_OBJECT,message,##__VA_ARGS__)
#define LOG_WARN_TO(logger, message,...) \
	PASTE(LOG_WARN_TO,PP_NARG(1,##__VA_ARGS__))(logger,message,##__VA_ARGS__)
#define LOG_WARN_TO_0(logger,message,...)\
	do { if ( N_LIKELY( logger.IsLogEnabled<CFLog::LL_WARN>() ) )  logger.WriteMsg( logger << message );}while(0);
#define LOG_WARN_TO_X(logger,message,...)\
	do { if ( N_LIKELY( logger.IsLogEnabled<CFLog::LL_WARN>() ) )  logger.WriteMsg( message,##__VA_ARGS__ );}while(0);


#define LOG_ERROR(message,...) \
	LOG_ERROR_TO(LOG_OBJECT,message,##__VA_ARGS__)
#define LOG_ERROR_TO(logger, message,...) \
	PASTE(LOG_ERROR_TO,PP_NARG(1,##__VA_ARGS__))(logger,message,##__VA_ARGS__)
#define LOG_ERROR_TO_0(logger,message,...)\
	do { if ( N_LIKELY( logger.IsLogEnabled<CFLog::LL_ERROR>() ) )  logger.WriteMsg( logger << message );}while(0);
#define LOG_ERROR_TO_X(logger,message,...)\
	do { if ( N_LIKELY( logger.IsLogEnabled<CFLog::LL_ERROR>() ) )  logger.WriteMsg( message,##__VA_ARGS__ );}while(0);


#define LOG_FATAL(message,...) \
	LOG_FATAL_TO(LOG_OBJECT,message,##__VA_ARGS__)
#define LOG_FATAL_TO(logger, message,...) \
	PASTE(LOG_FATAL_TO,PP_NARG(1,##__VA_ARGS__))(logger,message,##__VA_ARGS__)
#define LOG_FATAL_TO_0(logger,message,...)\
	do { if ( N_UNLIKELY( logger.IsLogEnabled<CFLog::LL_FATAL>() ) )  logger.WriteMsg( logger << message );}while(0);
#define LOG_FATAL_TO_X(logger,message,...)\
	do { if ( N_UNLIKELY( logger.IsLogEnabled<CFLog::LL_FATAL>() ) )  logger.WriteMsg( message,##__VA_ARGS__ );}while(0);


#define LOG_DEF(name) inline void __NOOP(){}

#define LOG_DEBUG_ENABLED() LOG_DEBUG_ENABLED_TO(LOG_OBJECT)
#define LOG_DEBUG_ENABLED_TO(logger) logger.IsLogEnabled<CFLog::LL_DEBUG>()

#define LOG_INFO_ENABLED()  LOG_INFO_ENABLED_TO(LOG_OBJECT)
#define LOG_INFO_ENABLED_TO(logger) logger.IsLogEnabled<CFLog::LL_INFO>()

#define LOG_WARN_ENABLED()  LOG_WARN_ENABLED_TO(LOG_OBJECT)
#define LOG_WARN_ENABLED_TO(logger) logger.IsLogEnabled<CFLog::LL_WARN>()

#define LOG_ERROR_ENABLED() LOG_ERROR_ENABLED_TO(LOG_OBJECT)
#define LOG_ERROR_ENABLED_TO(logger) logger.IsLogEnabled<CFLog::LL_ERROR>()

#define LOG_FATAL_ENABLED() LOG_FATAL_ENABLED_TO(LOG_OBJECT)
#define LOG_FATAL_ENABLED_TO(logger) logger.IsLogEnabled<CFLog::LL_FATAL>()


#endif /* CFLOG_H_ */
