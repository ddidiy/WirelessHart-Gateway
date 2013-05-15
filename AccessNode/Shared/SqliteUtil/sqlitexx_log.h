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

#ifndef sqlitexx_log_h__
#define sqlitexx_log_h__

#include <string>
#include <sstream> 

/// @addtogroup libshared
/// @{

enum LogLevel
{
	LL_TRACE = 5,
	LL_DEBUG = 4,
	LL_INFO = 3,
	LL_WARN = 2,
	LL_ERROR = 1,
	LL_FATAL = 0
};
void sqlitexx_EnableLog(int);
bool sqlitexx_IsLogEnabled(int);
void sqlitexx_WriteLog(const char *, const char*);
void sqlitexx_SetLogFile(std::string);


#define LOG_INIT(filePath) sqlitexx_SetLogFile(filePath)
#define LOG_REINIT(filePath) sqlitexx_SetLogFile(filePath)
#define LOG_SHUTDOWN() __noop

#define LOG_DEF(name) inline void __NOOP(){}
#define LOG_DEF_BY_CLASS(class_) LOG_DEF(class_)

#define LOG_TRACE_ENABLED() sqlitexx_IsLogEnabled(LL_TRACE)
#define LOG_TRACE(message) \
	do { \
	if(sqlitexx_IsLogEnabled(LL_TRACE)) { \
	std::ostringstream str; \
	str << message; \
	sqlitexx_WriteLog("TRACE", str.str().c_str()); \
	} \
	} while(0);

#define LOG_DEBUG_ENABLED() sqlitexx_IsLogEnabled(LL_DEBUG)
#define LOG_DEBUG(message) \
	do { \
	if(sqlitexx_IsLogEnabled(LL_DEBUG)) { \
	std::ostringstream str; \
	str << message; \
	sqlitexx_WriteLog("DEBUG", str.str().c_str()); \
	} \
	} while(0);

#define LOG_INFO_ENABLED() sqlitexx_IsLogEnabled(LL_INFO)
#define LOG_INFO(message) \
	do { \
	if(sqlitexx_IsLogEnabled(LL_INFO)) { \
	std::ostringstream str; \
	str << message; \
	sqlitexx_WriteLog("INFO", str.str().c_str()); \
	} \
	} while(0);

#define LOG_WARN_ENABLED() sqlitexx_IsLogEnabled(LL_WARN)
#define LOG_WARN(message) \
	do { \
	if(sqlitexx_IsLogEnabled(LL_WARN)) { \
	std::ostringstream str; \
	str << message; \
	sqlitexx_WriteLog("WARN", str.str().c_str()); \
	} \
	} while(0);

#define LOG_ERROR_ENABLED() sqlitexx_IsLogEnabled(LL_ERROR)
#define LOG_ERROR(message) \
	do { \
	if(sqlitexx_IsLogEnabled(LL_ERROR)) { \
	std::ostringstream str; \
	str << message; \
	sqlitexx_WriteLog("ERROR", str.str().c_str()); \
	} \
	} while(0);

#define LOG_FATAL_ENABLED() sqlitexx_IsLogEnabled(LL_FATAL)
#define LOG_FATAL(message) \
	do { \
	if(sqlitexx_IsLogEnabled(LL_FATAL)) { \
	std::ostringstream str; \
	str << message; \
	sqlitexx_WriteLog("FATAL", str.str().c_str()); \
	} \
	} while(0);

#define LOG_CALL(methodName) __noop

/// @}
#endif // sqlitexx_log_h__
