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

#include "sqlitexx_log.h"

#include "../Common.h"
#include <string>


void sqlitexx_SetLogFile(std::string strFileName)
{
	g_stLog.Open(strFileName.c_str(), "Start sqlitexx solo logging", 5*1024*1024);
}


static enum LogLevel logAPP = LL_DEBUG;
void sqlitexx_EnableLog(int level)
{

	switch ((enum LogLevel)level)
	{
	case LL_TRACE:
	case LL_DEBUG:
	case LL_INFO:
	case LL_WARN:
	case LL_ERROR:
	case LL_FATAL:
		logAPP = (enum LogLevel)level;
		break;
	}
}

bool sqlitexx_IsLogEnabled(int level)
{
	return level <= logAPP ? true : false;
}

void sqlitexx_WriteLog(const char *pszLevel, const char *pszContent)
{
	LOG("%s - %s", pszLevel, pszContent);
}



