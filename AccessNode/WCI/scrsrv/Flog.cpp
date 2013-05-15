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

/*
 * CFLog.cpp
 *
 *  Created on: Mar 22, 2010
 *      Author: mariusn
 */

#include <sys/types.h>
#include <sys/stat.h>
#include "Flog.h"

#include <stdio.h>
#include <stdarg.h>

CFLog::CFLog()
	: m_eLogLevel(LL_INFO)
	, m_pLogSink(NULL)
{
	// TODO Auto-generated constructor stub
}

CFLog::~CFLog()
{
	// TODO Auto-generated destructor stub
}

void CFLog::WriteMsg(const std::ostream& message)
{
	m_oStrStream << message.rdbuf() ;
	m_pLogSink->consume(m_oStrStream);
	m_oStrStream.str("");
}
void CFLog::WriteMsg(const char* message, ... )
{
	va_list ap;
	va_start(ap, message);
	m_pLogSink->consume( message, ap  );
	va_end(ap);
}
CFLog g_stFlog;
