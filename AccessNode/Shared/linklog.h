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
                          linklog.h  -  description
                             -------------------
    begin                : Mon Sep 13 2004
    email                : viorel.turtoi@nivis.com
 ***************************************************************************/


#ifndef LINKLOG_H
#define LINKLOG_H

#include "link.h"

/**
  *@author Viorel Turtoi
  */
/// @ingroup libshared
class CLinkLog : public CLink  {
public: 
	CLinkLog();
	~CLinkLog();

	int WriteMsg(const char* p_sMsg,...);
	int WriteErrMsg(int p_nErrNo, const char* p_sMsg,...);
};

#endif
