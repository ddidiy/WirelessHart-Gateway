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


#include "Shared/Common.h"
#include <string>
#include <sstream>
#include <ostream>

#ifdef ACCESS_NODE_LOG_ENABLED



void SetLogFile(std::string strFileName)
{
	g_stLog.Open(strFileName.c_str());
}

enum LogLevel
{
	LL_DEBUG = 4,
	LL_INFO = 3,
	LL_WARN = 2,
	LL_ERROR = 1
};


static enum LogLevel logStack = LL_DEBUG;
void EnableLog(int level)
{

	switch ((enum LogLevel)level)
	{
	case LL_DEBUG:
	case LL_INFO:
	case LL_WARN:
	case LL_ERROR:
		logStack = (enum LogLevel)level;
		break;
	}
}

bool IsLogEnabled(int level)
{
	//return level <= LL_DEBUG && level >= LL_ERROR ? true : false;
	return level <= logStack ? true : false;
}

const char *szLogLevel[]={ "NONE", "ERROR","WARN", "INFO","DEBUG"};
void mhlog(enum LogLevel level, const std::ostream& message)
{
	//if (!IsLogEnabled(level))
	//	return ;
	
	std::ostringstream ss;
	ss << message.rdbuf() ;
	LOG("%s - %s", szLogLevel[level], ss.str().c_str());
}
void mhlog(enum LogLevel level, const char* message)
{
	//if (!IsLogEnabled(level))
	//	return ;
	LOG("%s - %s", szLogLevel[level], message);
}


static enum LogLevel logAPP = LL_DEBUG;
void EnableLog_APP(int level)
{

	switch ((enum LogLevel)level)
	{
	case LL_DEBUG:
	case LL_INFO:
	case LL_WARN:
	case LL_ERROR:
		logAPP = (enum LogLevel)level;
		break;
	}
}

bool IsLogEnabled_APP(int level)
{
	//return level <= LL_DEBUG && level >= LL_ERROR ? true : false;
	return level <= logAPP ? true : false;
}

const char *szLogLevelApp[]={ "APP_NONE", "APP_ERROR","APP_WARN", "APP_INFO","APP_DEBUG"};
void mhlog_APP(enum LogLevel level, const std::ostream& message)
{
	//if (!IsLogEnabled_APP(level))
	//	return ;

	std::ostringstream ss;
	ss << message.rdbuf() ;
	LOG("%s - %s", szLogLevelApp[level], ss.str().c_str());
}
void mhlog_APP(enum LogLevel level, const char* message)
{
	//if (!IsLogEnabled_APP(level))
	//	return ;
	LOG("%s - %s", szLogLevelApp[level], message);
}

#endif
