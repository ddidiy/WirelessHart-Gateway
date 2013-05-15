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

#include <WHartHost/model/MMappedFile.h>
#include <stddef.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <nlib/log.h>

namespace hart7 {
namespace hostapp {

MMappedFile::MMappedFile() :
	fileRegion(NULL), fileDescriptor(-1), mapSize(0)
{
}

MMappedFile::~MMappedFile()
{
	LOG_INFO_APP("bulk -> MMappedFile::~MMappedFile() " << std::hex << std::uppercase << (int)(this) );
	if (fileRegion != NULL && mapSize > 0)
	{
		munmap(fileRegion, mapSize);
		fileRegion = NULL;
		mapSize = 0;
	}
	if (fileDescriptor >= 0)
	{
		while (close( fileDescriptor))
			if (errno != EINTR)
			{
				LOG_ERROR_APP("UDO::File descriptor << " << fileDescriptor << ". Cannot close file errno = " << errno << " !");
				break;
			}
		fileDescriptor = -1;
	}
}

bool MMappedFile::init(char const *fileName, unsigned int &fileSize)
{
	fileDescriptor = open(fileName, O_RDONLY, 0);
	if (fileDescriptor < 0)
	{
		LOG_ERROR_APP("MMappedFile::init << " << fileName << " does not exist!, errno = " << errno);
		return false;
	}
	if (0 > (mapSize = fileSize = lseek(fileDescriptor, 0, SEEK_END)))
	{
		LOG_ERROR_APP("MMappedFile::init << " << fileName << " cannot reveal the size of it!, errno = " << errno);
		return false;
	}
	fileRegion = (char *) mmap(NULL, fileSize, PROT_READ, MAP_SHARED, fileDescriptor, 0);
	if (fileRegion == MAP_FAILED)
	{
		LOG_ERROR_APP("MMappedFile::init<< " << fileName << " cannot be mmap, errno = " << errno << " !");
		fileRegion = NULL;
		return false;
	}
	lseek(fileDescriptor, 0, SEEK_SET);
	return true;
}

char* MMappedFile::getRegion() const
{
	return fileRegion;
}
unsigned int MMappedFile::getMapSize() const
{
	return mapSize;
}
} //hostapp
} //hart7
