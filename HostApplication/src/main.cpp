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

#include <WHartHost/MainApp.h>

#ifndef _WINDOWS

#include <stdio.h>
#include <signal.h>

#define BKTRACE(a) if(!__builtin_frame_address(a)) { return ; } else { printf("%p\n", __builtin_return_address(a) ); }
#define LOGBKTRACE(a, str) if(__builtin_frame_address(a)) { str << std::hex << __builtin_return_address(a) << " "; }

void bktrace()
{
    std::ostringstream str;
    for (int i = 0; i < 20; ++i)
    {
        switch (i)
        {
            case 0: LOGBKTRACE(0, str); break;
            case 1: LOGBKTRACE(1, str); break;
            case 2: LOGBKTRACE(2, str); break;
            case 3: LOGBKTRACE(3, str); break;
            case 4: LOGBKTRACE(4, str); break;
            case 5: LOGBKTRACE(5, str); break;
            case 6: LOGBKTRACE(6, str); break;
            case 7: LOGBKTRACE(7, str); break;
            case 8: LOGBKTRACE(8, str); break;
            case 9: LOGBKTRACE(9, str); break;
            case 10: LOGBKTRACE(10, str); break;
            case 11: LOGBKTRACE(11, str); break;
            case 12: LOGBKTRACE(12, str); break;
            case 13: LOGBKTRACE(13, str); break;
            case 14: LOGBKTRACE(14, str); break;
            case 15: LOGBKTRACE(15, str); break;
            case 16: LOGBKTRACE(16, str); break;
            case 17: LOGBKTRACE(17, str); break;
            case 18: LOGBKTRACE(18, str); break;
            case 19: LOGBKTRACE(19, str); break;
            case 20: LOGBKTRACE(20, str); break;
        }
    }
    std::cout << "Crash: " << str.str();
    LOG_ERROR("Crash: " << str.str());
}

void handle_sigsegv(int sig)
{
    printf("SIGNAL : %d\n", sig);
    bktrace();
    exit(EXIT_FAILURE);
}
#endif

int main(int argc, char* argv[])
{
#ifndef _WINDOWS
    ::signal(SIGSEGV, handle_sigsegv);
    ::signal(SIGABRT, handle_sigsegv);
    ::signal(SIGALRM, handle_sigsegv);
    ::signal(SIGKILL, handle_sigsegv);
    ::signal(SIGFPE, handle_sigsegv);
#endif
	hart7::MainApp app;
	return app.Run(argc, argv);
}
