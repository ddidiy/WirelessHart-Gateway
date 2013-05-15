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

#include "hart7/util/NMLog.h"
#include "hart7/nmanager/MainApp.h"
#include <string>
#include <stdio.h>
#include <sstream>
#include <signal.h>
#include "hart7/nmanager/operations/WHOperationQueue.h"
#include "hart7/nmanager/DevicesManager.h"
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <stdlib.h>
#include <time.h>
#include "SMState/SMStateLog.h"
#include "SMState/WebLogger.h"

LOG_DEF("h7.n.main")
;

#define BKTRACE(a) if(__builtin_frame_address(a)) { \
     n += sprintf((str+n), "%p ", __builtin_return_address(a)); \
     std::cout << "BKTRACE<" << (a) << "> " << __builtin_return_address(a) << std:: endl; \
     LOG_ERROR("BKTRACE<" << (a) << "> " << __builtin_return_address(a)); \
    } else { bContinue = false;}

void HandlerFATAL(int p_signal)
{

     std::cout << "HandlerFATAL [SystemManager] " << ( (p_signal == SIGSEGV) ? "SIGSEGV" : (p_signal == SIGABRT) ?
                     "SIGABRT" : (p_signal == SIGFPE)  ? "SIGFPE" : "UNK" ) << " " << p_signal << " ."<< std::endl;
     LOG_ERROR("HandlerFATAL [NetworkManager] " << ( (p_signal == SIGSEGV) ? "SIGSEGV" : (p_signal == SIGABRT) ?
                    "SIGABRT" : (p_signal == SIGFPE)  ? "SIGFPE" : "UNK" ) << " " << p_signal << " .");

     static char str[6000];   /// 11 bytes per address. Watch for overflow
     int n=0;
     bool bContinue=true;
     *str=0;/// reset every time
     /// Dump the stack.
     n+=snprintf( str+n, sizeof(str)-n-1, "STACK DUMP:\n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             "        0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n"
             );
     LOG_ERROR(str);
     str[0] = 0;
     n = 0;
     for ( int i=0; bContinue && (i<16) ; ++i)
     {   switch (i)  /// parameter for __builtin_frame_address must be a constant integer
         {   case 0: BKTRACE(0); break ;
             case 1: BKTRACE(1); break ;
             case 2: BKTRACE(2); break ;
             case 3: BKTRACE(3); break ;
             case 4: BKTRACE(4); break ;
             case 5: BKTRACE(5); break ;
             case 6: BKTRACE(6); break ;
             case 7: BKTRACE(7); break ;
             case 8: BKTRACE(8); break ;
             case 9: BKTRACE(9); break ;
             case 10:BKTRACE(10);break ;
             case 11:BKTRACE(11);break ;
             case 12:BKTRACE(12);break ;
             case 13:BKTRACE(13);break ;
             case 14:BKTRACE(14);break ;
             case 15:BKTRACE(15);break ;
         }
     }

#warning should integrated with log2flash
     std::cout << "PANIC [NetworkManager] "
<< ( (p_signal == SIGSEGV) ? "SIGSEGV" : (p_signal == SIGABRT) ?
"SIGABRT" : (p_signal == SIGFPE)  ? "SIGFPE" : "UNK" ) << " " << p_signal << " . Backtrace " << str << std::endl;
     LOG_FATAL("PANIC [NetworkManager] "
<< ( (p_signal == SIGSEGV) ? "SIGSEGV" : (p_signal == SIGABRT) ?
"SIGABRT" : (p_signal == SIGFPE)  ? "SIGFPE" : "UNK" ) << " " << p_signal << " . Backtrace " << str );

     exit(EXIT_FAILURE);
}


void handle_sigsegv(int sig)
{
    printf("SIGNAL : %d\n", sig);
    //bktrace();
    HandlerFATAL(sig);
    exit(EXIT_FAILURE);
}

bool hart7::nmanager::DevicesManager::LogAllInfo = false;

void handle_sigusr(int sig)
{
    if (sig == SIGUSR2)
    {
        hart7::nmanager::DevicesManager::LogAllInfo = true;
        printf("SIGNAL : %d handled\n", sig);
    }
    else if (sig == SIGUSR1)
    {
        SMState::WebLogger::Instance().Cycle();
        printf("SIGNAL : %d handled\n", sig);
    }
}

void handle_sigpipe(int sig)
{
    // do nothing
    printf("SIGNAL : %d handled\n", sig);
}

boost::function1<void, std::string> hupCallback;
std::string pathToLog;

void hupHandler(int sig)
{
    if (hupCallback)
    {
        hupCallback(pathToLog);
    }

    ::signal(SIGHUP, hupHandler);
}

int main(int argc, char* argv[])
{
    ::signal(SIGSEGV, handle_sigsegv);
    ::signal(SIGABRT, handle_sigsegv);
    ::signal(SIGALRM, handle_sigsegv);
    ::signal(SIGKILL, handle_sigsegv);
    ::signal(SIGFPE, handle_sigsegv);
    ::signal(SIGHUP, hupHandler);
    ::signal(SIGUSR1, handle_sigusr);
    ::signal(SIGUSR2, handle_sigusr);
    ::signal(SIGPIPE, handle_sigpipe);

    ::signal(SIGILL, handle_sigsegv);
    ::signal(SIGBUS, handle_sigsegv);


    srand(time(NULL));
    pathToLog = "nm.log4cplus.conf";
    std::string pathToScript = "/access_node/profile/script.scr";
    if (argc > 1)
    {
        std::ostringstream stream;
        stream << argv[1] << "nm.log4cplus.conf";
        pathToLog = stream.str();
    }
    else
    {
        std::ostringstream stream;
        stream << NIVIS_PROFILE << "nm.log4cplus.conf";
        pathToLog = stream.str();
    }

    LOG_INIT(pathToLog.c_str());
    LOG_INFO("Start NetworkManager");

    hart7::nmanager::MainApp app(pathToLog, pathToScript);
    hupCallback = boost::bind(&hart7::nmanager::MainApp::Reload, &app);

    app.Run(argc, argv);

    return 0;
}
