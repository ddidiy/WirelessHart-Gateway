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

#ifndef _LOG2FLASH_H_
#define _LOG2FLASH_H_

#include "AnPaths.h"

/// @addtogroup libshared
/// @{

#define DEF_LOG_FILE             NIVIS_ACTIVITY_FILES"work_time.debug"
#define DEF_ERR_FILE             NIVIS_ACTIVITY_FILES"error.debug"
#define FLOCK_TIMEOUT            5                  // 5 seconds

//#define LOG2FLASH_DEBUG

#ifdef LOG2FLASH_DEBUG
	#define DPRINT          printf
	#define LOG_LIMIT       4*1024
	#define ROTATE_MAX	999
#else
	#define DPRINT          dummie
	#define LOG_LIMIT       200*1024                  // 200 kb
	#define ROTATE_MAX	3
#endif


bool log2flash(const char *fmt, ...);
bool log2flash_f(const char *fname, const char *fmt, ...);


/// @}
#endif
