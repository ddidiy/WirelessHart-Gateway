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



#include <WHartHost/MyLog.h>

#include <Shared/Common.h>
#include <Shared/Config.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <string>
#include <sstream>
#include <ostream>



//############### -our logger

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



//############### -our logger


struct LogDetails
{
	std::string		strLogFile;
	int 			FileSize;
	int				FileArchiveEnable;
	enum LogLevel 	logLevel;
	enum LogLevel 	logLevelAPP;
};
static bool ParseIniFile(const char * fileName, const char *pszSection, LogDetails & logDetails)
{
	CIniParser oIniParser;
	if (!oIniParser.Load(fileName))
		return false;

	char szLogVarString[256] = "";
	if (!oIniParser.GetVarRawString(pszSection, "FileName", szLogVarString, sizeof(szLogVarString)))
		return false;

#ifdef HW_VR900
	logDetails.strLogFile = NIVIS_TMP;
#else
	logDetails.strLogFile = "logs/";
#endif

	logDetails.strLogFile += szLogVarString;
	logDetails.strLogFile += ".log";

	if (!oIniParser.GetVarRawString("MH_LOG", "FileMaxSize", szLogVarString, sizeof(szLogVarString)))
		return false;
	logDetails.FileSize = atoi(szLogVarString);

	//printf("log size = %d", atoi(szLogVarString));
	//fflush(stdout);

	if (!oIniParser.GetVarRawString("MH_LOG", "EnableLogArchive", szLogVarString, sizeof(szLogVarString)))
		return false;
	logDetails.FileArchiveEnable = atoi(szLogVarString);

	if (!oIniParser.GetVarRawString("MH_LOG", "LogLevel_STACK", szLogVarString, sizeof(szLogVarString)))
		return false;
	logDetails.logLevel = (enum LogLevel)atoi(szLogVarString);

	if (!oIniParser.GetVarRawString("MH_LOG", "LogLevel_APP", szLogVarString, sizeof(szLogVarString)))
		return false;
	logDetails.logLevelAPP = (enum LogLevel)atoi(szLogVarString);

	return true;
}

static bool InitLogFile(LogDetails & logDetails)
{
	if (!g_stLog.Open(logDetails.strLogFile.c_str()))
	{
		if (mkdir("logs", 0777) == -1)
			return false;
		if (!g_stLog.Open(logDetails.strLogFile.c_str()))
			return false;
	}

	g_stLog.SetMaxSize(logDetails.FileSize);

#ifdef HW_VR900
	std::string logFolder = NIVIS_TMP;
#else
	std::string logFolder = "logs/";
#endif

	if (logDetails.FileArchiveEnable == 1)
		g_stLog.SetStorage(logFolder.c_str());

	return true;
}


bool InitLogEnv(const char *pszIniFile)
{
	LogDetails logDetails;
	if (!ParseIniFile(pszIniFile, "MH_LOG" /*section*/, logDetails))
		return false;


	if (!InitLogFile(logDetails))
		return false;

	EnableLog(logDetails.logLevel);
	EnableLog_APP(logDetails.logLevelAPP);

	return true;
}


