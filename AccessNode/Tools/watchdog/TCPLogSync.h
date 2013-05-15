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

#define LOG_OBJECT g_stWdtLog
#include "WCI/scrsrv/Flog.h"

class CTCPSink : public CFLogSink {
public:
	CTCPSink()
	:skt(0)
	{
	}
	virtual ~CTCPSink() {}
public:
	bool open(const char* host, unsigned int port);
	bool dissociate( );
	void consume( const std::ostringstream& msg  );
	void consume( const char* message, va_list ap  );
protected:
	int skt ;
};

extern CFLog g_stWdtLog;
