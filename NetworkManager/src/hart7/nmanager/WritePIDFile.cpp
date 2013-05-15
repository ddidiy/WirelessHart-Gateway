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
 * WritePIDFile.cpp
 *
 *  Created on: Feb 25, 2010
 *      Author: Andy
 */
#include "WritePIDFile.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>

namespace hart7 {
namespace nmanager {

WritePIDFile::WritePIDFile(std::string pidFileName)
{
	this->pidFileName = pidFileName;
	count = 0;
}

void WritePIDFile::PeriodicTask()
{
	if (++count >= 10)
	{
		FILE* pidFile = fopen(pidFileName.c_str(), "w+");
		if (pidFile == NULL) {
			LOG_FATAL("unable to write the pid file - exiting");
			std::cout << "unable to write the pid file - exiting" << std::endl;
			exit(3);
		}

		fprintf(pidFile, "%d", getpid());
		fclose(pidFile);
		std::cout << "write pid:" << getpid() << std::endl;

		count = 0;
	}
}


}
}
