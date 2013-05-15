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

////////////////////////////////////////////////////////////////////////////////
/// @file log_callback.h
/// @brief C-linkage interface for a callback logging system
/// @remarks
/// @remarks Usage:
/// @remarks 1: Call LogCallback_SetFunc to register logger callback
/// @remarks 2: [Optional] Use LogCallback_SetLevel to set the logging level (dafault to LOGLVL_DBG: log ALL messages)
/// @remarks 3: Use c_log/c_loghex (ACTUALLY use LOG / LOG_HEX which expand to c_log / c_loghex) for logging
/// @remarks 4: The p_pCallbackLog logs a string to user logging system
/// @remarks (user implementation should prefix with timestamp/postfix with NL)
/// @remarks - Use LogCallback_SetLevel to change the logging level (dafault to LOGLVL_DBG: log ALL messages)
/// @remarks - Use LogCallback_SetFunc whenever stack logging needs to cease/change to other method
/// @remarks
/// @remarks
/// @remarks Code-level integration for external code using files from Shared/
/// @remarks (which use LOG/LOG_HEX macros):
/// @remarks Define CALLBACK_LOG (use -DCALLBACK_LOG) in compile flags for any
/// @remarks of the Shared objects used which have to use callback logging
/// @remarks LOG/LOG_HEX will be defined to point to c_log/c_loghex instead of CLog methods
/// @remarks 
/// @remarks TAKE CARE: Make sure the Shared/ objects use only LOG/LOG_HEX,
/// @remarks no other LOG* macro and neither CLog methods dirrectly
/// @remarks 
/// @remarks TAKE CARE: Duplicated .h file in folders Shared/ and IsaGw/ISA100/
/// @remarks		any change should be done in Shared folder and the file copied to ISA100
/// @remarks		any module, except SM, MUST include the Shared/ file; the SM may include IsaGw/ISA100/ file
///
/// @see AccessNode/Shared/log_callback.h
/// @see AccessNode/IsaGw/ISA100/log_callback.h
///
////////////////////////////////////////////////////////////////////////////////
#ifndef _LOG_CALLBACK_H_
#define _LOG_CALLBACK_H_

/// @addtogroup libshared
/// @{

typedef enum {
	LOGLVL_NONE = 0,	/// ONLY for test
	LOGLVL_ERR = 1,	/// User should ALWAYS log this log level.
	LOGLVL_INF = 2,	/// User may choose to ignore this loglevel, to reduce the log noise. It is recommended to log it, however.
	LOGLVL_DBG = 3	/// User should NOT log this level, except for stack debugging
} LOGLVL;

//#define c_log( p_eLogLvl, fmt, ...)
#define LOG_ISA100_ENABLED(lvl) LogCallback_IsLogLevelEnabled(lvl)
#define LOG_ISA100(lvl, fmt, ...) \
	do	\
	{	\
		if(LOG_ISA100_ENABLED(lvl))		\
		{	c_log(lvl, fmt, ##__VA_ARGS__);	\
		}	\
	}while(0)


#define LOG_ISA100_ENABLED(lvl) LogCallback_IsLogLevelEnabled(lvl)
#define LOG_HEX_ISA100(lvl, fmt, ...) \
	do	\
	{	\
		if(LOG_ISA100_ENABLED(lvl))		\
		{	c_loghex(lvl, fmt, ##__VA_ARGS__);	\
		}	\
	}while(0)

//#define c_loghex( p_eLogLvl, p_sMsg, p_pBinBuffer, p_nBinBufferLen );

////////////////////////////////////////////////////////////////////////////////
/// @brief User-provided callback which logs to user logging system a string
/// @param p_ucLogLevel - the loglevel, @see LOGLVL_*
/// @param p_szMsg - Null terminated string, max 2048 bytes, with the result of parsing done in c_log/c_loghex
/// It does not prefix the string with timestamp / postfix with new line.
/// The user is responssible to prefix/postfix the string
/// @remarks The user links the callback to it's logging system
/// The callback is called by c_log/c_loghex after formating the string
////////////////////////////////////////////////////////////////////////////////
typedef void (* PfnCallbackLog )( LOGLVL p_eLogLvl, const char* p_szMsg );

/// Register the callback function
void LogCallback_SetFunc( PfnCallbackLog p_pCallbackLog );

/// Set logging level
/// Any log level smaller or equal with p_eLogLvl will be logged
void LogCallback_SetLevel( LOGLVL p_eLogLvl );

//return 1/0 - log enabled/disabled
int LogCallback_IsLogLevelEnabled( LOGLVL p_eLogLvl );

/// test only 
void LogCallback_UseFasLog(const char* p_szModule);

/// Take printf-like parameters and send formatted string back o user (no timestamp, no NL)
void c_log( LOGLVL p_eLogLvl, const char* fmt, ... );

/// Take binary buffer, convert it to hex and send formatted string back o user (no timestamp, no NL)
void c_loghex( LOGLVL p_eLogLvl, const char* p_sMsg, const void * p_pBinBuffer, int p_nBinBufferLen );

/// Take printf-like parameters and send formatted string back o user (no timestamp, no NL)
void c_log_lvlerr( const char* fmt, ... );

/// Take binary buffer, convert it to hex and send formatted string back o user (no timestamp, no NL)
void c_loghex_lvlerr( const char* p_sMsg, const void * p_pBinBuffer, int p_nBinBufferLen );

/// Take printf-like parameters and send formatted string back o user (no timestamp, no NL)
/// Also log strerror(errno)
void c_log_errno_lvlerr( const char* fmt, ... );



/// @}
#endif // _LOG_CALLBACK_H_

