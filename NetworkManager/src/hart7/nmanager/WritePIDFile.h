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
 * WritePIDFile.h
 *
 *  Created on: Feb 25, 2010
 *      Author: Andy
 */

#ifndef WRITEPIDFILE_H_
#define WRITEPIDFILE_H_

#include "IPeriodicTask.h"
#include <string>
#include <nlib/log.h>

namespace hart7 {
namespace nmanager {

/**
 * Writes PID file so that the watchdog knows the process is alive.
 * Implemented as a periodic task.
 */
class WritePIDFile : public IPeriodicTask
{
	LOG_DEF("PID");
public:
	WritePIDFile(std::string pidFileName);
	virtual ~WritePIDFile() {};

    virtual void PeriodicTask();

private:
	std::string pidFileName;
	int count;
};

}
}

#endif /* WRITEPIDFILE_H_ */
