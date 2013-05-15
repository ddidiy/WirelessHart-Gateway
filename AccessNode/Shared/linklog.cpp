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

/***************************************************************************
                          linklog.cpp  -  description
                             -------------------
    begin                : Mon Sep 13 2004
    email                : viorel.turtoi@nivis.com
 ***************************************************************************/

#include "linklog.h"
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>

CLinkLog::CLinkLog(){
}
CLinkLog::~CLinkLog(){
}

int CLinkLog::WriteMsg(const char* p_sMsg,...)
{
    if(m_nFd < 0)
    	return 0;
	
	va_list lstArg;
	char pBuf[512];

	va_start(lstArg, p_sMsg);
	vsprintf(pBuf, p_sMsg, lstArg);
	va_end(lstArg);

	strcat(pBuf, "\n");
	return writeBlock((const unsigned char*)pBuf, strlen(pBuf));
	
}

int CLinkLog::WriteErrMsg( int p_nErrNo, const char* p_sMsg,...)
{
    if(m_nFd < 0)
    	return 0;

	va_list lstArg;
	char pErrBuf[512];

	int nCount = sprintf(pErrBuf, "%s", strerror(p_nErrNo));

	va_start(lstArg, p_sMsg);
	vsprintf(pErrBuf+nCount, p_sMsg, lstArg);
	va_end(lstArg);

	strcat(pErrBuf, "\n");
	return writeBlock((const unsigned char*)pErrBuf, strlen(pErrBuf));
}
