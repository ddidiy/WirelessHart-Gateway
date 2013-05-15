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

#ifndef WEBLOGGER_H
#define WEBLOGGER_H

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>


namespace SMState {

class WebLogger {

	private:

		const static int buffSize = 1440; 		// .. MTU is 1500, ussually MSS is 1460
		const static int mPortNumber = 4444;

		WebLogger()  							// ctor is hidden
		{
			buff = NULL;
			socket_fd = 0;
			noSocketWriteErrors = 0;
			stopped = false;
			serverIp = NE::Model::NetworkEngine::instance().getSettingsLogic().webLoggerIp;
			this->ConnectToServer();
		}

  		WebLogger(WebLogger const&);     		 // copy ctor is hidden
  		WebLogger& operator=(WebLogger const&);  // assign op is hidden
		~WebLogger(){}; 						 // dtor hidden

		char* buff;
		int socket_fd;
		int noSocketWriteErrors;
		bool stopped;
		int buffCursor;
		int lastSentTime;
		std::string serverIp;

	public:

		static WebLogger& Instance() 
		{
    		static WebLogger theLogger = WebLogger();
    		return theLogger;
		}

		void PublishOverTCPIP(char* varName, int value)
		{
			if (socket_fd == 0 && !stopped) ConnectToServer();

			if  (socket_fd > 0) 
			{ 
				int now = (int) time (NULL);
				char tmp[64];
				
				int need = strlen(varName);
				memset(tmp, 0, 64); 
				sprintf(tmp, "%d", value);
				need += strlen(tmp);
				need += 13;
				
				if (buffCursor + need  < buffSize - 2 &&  now - lastSentTime < 10) {
					
					sprintf (buff + buffCursor, "%s %d %d\n", varName, value, now);
					buffCursor += need;

				} else {

				    // should check here that all bytes were written ..
					int ret = write (socket_fd, buff, strlen(buff) + 1);

					if (ret == -1) {

					    // socket closed by the server
					    if (errno == EPIPE) Disconnect();

						++noSocketWriteErrors;
						if (noSocketWriteErrors >= 3)
							Disconnect();

					} else {
						noSocketWriteErrors = 0;
					}

					memset(buff, 0, buffSize);
					buffCursor = 0;
					lastSentTime = now;

					sprintf (buff + buffCursor, "%s %d %d\n", varName, value, now);
					buffCursor += need;
				}
		
			}
		}

		void PublishOverTCPIP(char* varName1, int value1, char* varName2, int value2)
		{
			if (socket_fd == 0 && !stopped) ConnectToServer();
			
			if  (socket_fd > 0) 
			{ 
				int now = (int) time (NULL);
				char tmp[64];
				
				int need = strlen(varName1);
				memset(tmp, 0, 64); 
				sprintf(tmp, "%d", value1);
				need += strlen(tmp);
				need += strlen(varName2);
				memset(tmp, 0, 64); 
				sprintf(tmp, "%d", value2);
				need += strlen(tmp);
				need += 15;
				
				if (buffCursor + need  < buffSize - 2 && now - lastSentTime < 10) {
					
					sprintf (buff + buffCursor, "%s %d %s %d %d\n", varName1, value1, varName2, value2, (int)time (NULL));
					buffCursor += need;

				} else {

				    // should check here that all bytes were written ..
					int ret = write (socket_fd, buff, strlen(buff) + 1);

					if (ret == -1) {

					    // socket closed by the server
					    if (errno == EPIPE) Disconnect();

						++noSocketWriteErrors;
						if (noSocketWriteErrors >= 3)
							Disconnect();

					} else {
						noSocketWriteErrors = 0;
					}

					memset(buff, 0, buffSize);
					buffCursor = 0;
					lastSentTime = now;

					sprintf (buff + buffCursor, "%s %d %s %d %d\n", varName1, value1, varName2, value2, (int)time (NULL));
					buffCursor += need;
				}
			}
		}

		void PublishOverTCPIP(char* varName1, int value1, char* varName2, int value2, char* varName3, int value3)
		{
			if (socket_fd == 0 && !stopped) ConnectToServer();
			
			if  (socket_fd > 0) 
			{ 
				int now = (int) time (NULL);
				char tmp[64];
				
				int need = strlen(varName1);
				memset(tmp, 0, 64); 
				sprintf(tmp, "%d", value1);
				need += strlen(tmp);
				need += strlen(varName2);
				memset(tmp, 0, 64); 
				sprintf(tmp, "%d", value2);
				need += strlen(tmp);
				need += strlen(varName3);
				memset(tmp, 0, 64); 
				sprintf(tmp, "%d", value3);
				need += strlen(tmp);
				need += 17;
				
				if (buffCursor + need  < buffSize - 2 && now - lastSentTime < 10) {
					
					sprintf (buff + buffCursor, "%s %d %s %d %s %d %d\n", varName1, value1, varName2, value2, varName3, value3, (int)time (NULL));
					buffCursor += need;

				} else {

				    // should check here that all bytes were written ..
					int ret = write (socket_fd, buff, strlen(buff) + 1);

					if (ret == -1) {

					    // socket closed by the server
					    if (errno == EPIPE) Disconnect();

						++noSocketWriteErrors;
						if (noSocketWriteErrors >= 3)
							Disconnect();

					} else {
						noSocketWriteErrors = 0;
					}

					memset(buff, 0, buffSize);
					buffCursor = 0;
					lastSentTime = now;

					sprintf (buff + buffCursor, "%s %d %s %d %s %d %d\n", varName1, value1, varName2, value2, varName3, value3, (int)time (NULL));
					buffCursor += need;
				}
			}
		}

		void PublishOverTCPIP(char* varName1, int value1, char* varName2, int value2, char* varName3, int value3, char* varName4, int value4)
		{
			if (socket_fd == 0 && !stopped) ConnectToServer();
			
			if  (socket_fd > 0) 
			{ 
				int now = (int) time (NULL);
				char tmp[64];
				
				int need = strlen(varName1);
				memset(tmp, 0, 64); 
				sprintf(tmp, "%d", value1);
				need += strlen(tmp);
				need += strlen(varName2);
				memset(tmp, 0, 64); 
				sprintf(tmp, "%d", value2);
				need += strlen(tmp);
				need += strlen(varName3);
				memset(tmp, 0, 64); 
				sprintf(tmp, "%d", value3);
				need += strlen(tmp);
				need += strlen(varName4);
				memset(tmp, 0, 64); 
				sprintf(tmp, "%d", value4);
				need += strlen(tmp);
				need += 19;
				
				if (buffCursor + need  < buffSize - 2 && now - lastSentTime < 10) {
					
					sprintf (buff + buffCursor, "%s %d %s %d %s %d %s %d %d\n", 
						varName1, value1, varName2, value2, varName3, value3, varName4, value4, (int)time (NULL));
					buffCursor += need;

				} else {

				    // should check here that all bytes were written ..
					int ret = write (socket_fd, buff, strlen(buff) + 1);

					if (ret == -1) {

					    // socket closed by the server
					    if (errno == EPIPE) Disconnect();

						++noSocketWriteErrors;
						if (noSocketWriteErrors >= 3)
							Disconnect();

					} else {
						noSocketWriteErrors = 0;
					}

					memset(buff, 0, buffSize);
					buffCursor = 0;
					lastSentTime = now;

					sprintf (buff + buffCursor, "%s %d %s %d %s %d %s %d %d\n", 
						varName1, value1, varName2, value2, varName3, value3, varName4, value4, (int)time (NULL));
					buffCursor += need;
				}
			}
		}

		void Start ()
		{
			char temp[1024];
			sprintf (temp, "starting weblogger ... \n");
			WriteToLog(temp);

			if (stopped || socket_fd == 0) ConnectToServer ();
			stopped = false;
		}

		void Stop ()
		{
			char temp[1024];
			sprintf (temp, "stopping weblogger ... \n");
			WriteToLog(temp);

			Disconnect();
			stopped = true;
		}

		void Cycle ()
		{
			if (stopped) Start();
			else Stop();
		}

	private:

		/* sets socket to nonblocking mode */
		int SetSocketNonblocking(int fd)	
		{
    		int flags;

			#if defined(O_NONBLOCK)
    		if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
        		flags = 0;
    		return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
			#else
    		flags = 1;
    		return ioctl(fd, FIOBIO, &flags);
			#endif
		}

		bool ConnectToServer()
		{
			try {

				char temp[1024];
				sprintf (temp, "conecting to server ... \n");
				WriteToLog(temp);
				
				if (buff == 0) buff = new char [buffSize];

				memset(buff, 0, buffSize);
				lastSentTime = time(NULL);
				buffCursor = 0;				

				/* close previous socket */
				if (socket_fd > 0) 
				{
					close (socket_fd);
					socket_fd = 0;
				}
				
				/* create the socket */
				socket_fd = socket (PF_INET, SOCK_STREAM, 0);
				if (socket_fd == -1) 
				{				
					char temp[1024];
					sprintf (temp, "error while trying to initialize socket: %s \n", strerror(errno));
					WriteToLog(temp);
					throw 0;
				}
				
				/* set keep alive option: socket will be marked bad if the server shuts down */
   				int optval = 1;
   				socklen_t optlen = sizeof(optval);

   				if (setsockopt(socket_fd, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0) 
				{
					char temp[1024];
					sprintf (temp, "error while trying to setting keep alive socket option: %s \n", strerror(errno));
					WriteToLog(temp);
					throw 0;
   				}
				else 
				{
					/* set keep alive interval to 20 seconds */
					optval = 20;
   				
					if (setsockopt(socket_fd, SOL_TCP, TCP_KEEPINTVL, &optval, optlen) < 0) 
					{
						char temp[1024];
						sprintf (temp, "error while trying to setting keep alive interval socket option: %s \n", strerror(errno));
						WriteToLog(temp);
						throw 0;
					}
				}

				
				
				/* store the server name in the socket address */
				struct sockaddr_in name;
				name.sin_family = AF_INET;
				name.sin_port = htons(mPortNumber);
				inet_pton(AF_INET, serverIp.c_str (), &(name.sin_addr));
								
				/* connect the socket */
				int ret = connect (socket_fd, reinterpret_cast<sockaddr*>(&name), sizeof (struct sockaddr_in));
				if (ret == -1) 
				{
					char temp[1024];
					sprintf (temp, "could not establish connection to server: %s \n", strerror(errno));
					WriteToLog(temp);
					throw 0;
				}
				
				/* set socket to non blocking mode */
 				ret = SetSocketNonblocking(socket_fd);
				if (ret == -1) 
				{
					char temp[1024];
					sprintf (temp, "could not set socket to nonblocking mode: %s \n", strerror(errno));
					WriteToLog(temp);
					throw 0;
				}

				/* authenticate with the server */
				sprintf (buff, "my name is socket");
				write (socket_fd, buff, strlen(buff) + 1);
				memset(buff, 0, buffSize);
	
				return true;

			} catch (...) {
		
				Disconnect();
				stopped = true;
				return false;
			}	
		} 

		void Disconnect()
		{
			char temp[1024];
			sprintf (temp, "shutting down ... \n");
			WriteToLog(temp);
			
			if (socket_fd > 0) 
				close (socket_fd);

			delete buff;
			socket_fd = 0;
			buff = 0;
		}

		void WriteToLog(char* msg)
		{
			FILE* log =  fopen("webLogger.log", "a");

			if (log != NULL) {

				time_t rawtime;
  				struct tm * timeinfo;

  				time ( &rawtime );
  				timeinfo = localtime ( &rawtime );

				fprintf(log, "%s - %s\n",  asctime (timeinfo), msg);
				fclose(log);
			}
		}     
};

} // name space SMState

#endif 

