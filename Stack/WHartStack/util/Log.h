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
 * Log.h - include <nlib/log.h> for c++, and disabled for c
 *
 *  Created on: Nov 21, 2008
 *      Author: nicu.dascalu
 */

#ifndef WHART_STACK_LOG_H_
#define WHART_STACK_LOG_H_

#ifdef __cplusplus
#	include <nlib/log.h>
#else

#if !defined(_MSC_VER)
#define __noop ((void)0)
#endif

#define LOG_INIT(filePath) __noop
#define LOG_REINIT(filePath) __noop

#define LOG_DEF(name)
#define LOG_DEF_BY_CLASS(class_) LOG_DEF(class_)

#define LOG_TRACE_ENABLED() false
#define LOG_TRACE(message) __noop

#define LOG_DEBUG_ENABLED() false
#define LOG_DEBUG(message) __noop

#define LOG_INFO_ENABLED() false
#define LOG_INFO(message) __noop

#define LOG_WARN_ENABLED() false
#define LOG_WARN(message) __noop

#define LOG_ERROR_ENABLED() false
#define LOG_ERROR(message) __noop

#define LOG_FATAL_ENABLED() false
#define LOG_FATAL(message) __noop

#define LOG_CALL(methodName) __noop

#endif

#endif /* WHART_STACK_LOG_H_ */
