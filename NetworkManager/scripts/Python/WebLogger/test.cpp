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

#include <iostream>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "WebLogger.h"

int main ()
{	
	srand ( time(NULL) );

	while (true) 
	{
		WebLogger::Instance().PublishOverTCPIP("test1", rand() % 1000, "test2", rand() % 1000);
		WebLogger::Instance().PublishOverTCPIP("test3", rand() % 1000, "test4", rand() % 1000);
		WebLogger::Instance().PublishOverTCPIP("test5", rand() % 1000);
		WebLogger::Instance().PublishOverTCPIP("test1", rand() % 1000, "test2", rand() % 1000);
		WebLogger::Instance().PublishOverTCPIP("test3", rand() % 1000, "test4", rand() % 1000);
		WebLogger::Instance().PublishOverTCPIP("test5", rand() % 1000);
		WebLogger::Instance().PublishOverTCPIP("testa", rand() % 1000, "testb", rand() % 1000, "testc", rand() % 1000);
		WebLogger::Instance().PublishOverTCPIP("test9", rand() % 1000, "test8", rand() % 1000, "test7", rand() % 1000, "test6", rand() % 1000);
		sleep(1);
		WebLogger::Instance().PublishOverTCPIP("test1", rand() % 1000, "test2", rand() % 1000);
		WebLogger::Instance().PublishOverTCPIP("test3", rand() % 1000, "test4", rand() % 1000);
		WebLogger::Instance().PublishOverTCPIP("test5", rand() % 1000);
		WebLogger::Instance().PublishOverTCPIP("test1", rand() % 1000, "test2", rand() % 1000);
		WebLogger::Instance().PublishOverTCPIP("test3", rand() % 1000, "test4", rand() % 1000);
		WebLogger::Instance().PublishOverTCPIP("test5", rand() % 1000);
	}

	return 0;	
}
