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

// Upload2Serial.cpp : Defines the entry point for the console application.
//
#include <cstdio>
#include <cstdlib>
#include <getopt.h>

#include "VersaNode.h"
#include "../Shared/Common.h"
#include "../Shared/Config.h"

void help_message(char *command)
{
	printf( "Gets packets from RS232 and prints into text files.\n"
			"Syntax:\n"
			"%s <input> [power] [ttyport] [baud] [Timeout] [>Log File]\n"
			"-i --input  The data file from where to load the packets.\n"
			"                   It is a mandatory parameter.\n"
			"-p --power   Set the VR900 transceiver number to power on\n"
			"                   Can be 0 for none, 1 or 2, default 0. This argument is optional\n"
			"-t --ttyport Serial tty port   The RS232 port to send the packets.\n"
			"                   Can be ttyS0,ttyS1,... or ttyUSB0, ttyUSB1, ... Default is ttyS1.\n"\
			"                   This parameter is optional.\n"
			"-b --baud    Set speed of serial port in baud. \n"
			"                   Default is B115200. This parameter optional\n"
			"-f --offset Offset time to power up the transciver. \n"
			"                   Can be 0, ...  300 , default is 10 seconds. This argument is optional\n"
			"-d --dest   The destination area for the packet to be written to.\n"
			"                   Can be 1, 2, ..., 5. Default is 2. The \n"
			"                   The most used destinattion area is 2. This parameter is optional.\n"
			"-o --timeout Time to expect responses in seconds. Default is 2.\n"
			"                   It is an optional positive parameter less than 61.\n"
			"-l --log     log in/out data in hex\n"
			"Example of usage that will upload data from file \"upload_file\"\n"
			"        to ttyS1 at destination 3 and with 2 seconds timeout:\n"
			"%s -i upload_file -p 1 -t /dev/ttyS1 -b B115200 -d 3\n"
			"%s -i upload_file -p 1\n",
			command, command, command );
}

using namespace std;

// The one and only application object
CVersaNode theApp;

int main(int argc, char *argv[], char* envp[])
{
	//struct 
	//{
	int nDst=2; 
	char *szInputFile=NULL;
	//} stDst[16] = {0};
	
	//char *szInputFile[16]=0;
	unsigned int unTimeOut = 2;
	// scan port
	const char *szSerialPort="/dev/ttyS1";
	
	int unBaudRate=B115200;
	unsigned int uTranciverNum = 0;
	unsigned int uTimeOff=10;
	bool bLogData=false;
	
	
	struct option long_options[] = 
	{
		{"ttyport" ,required_argument, 0, 't'},
		{"baud", required_argument, 0, 'b'},
		{"power", required_argument, 0, 'p'},
		{"timeoff", required_argument, 0, 'f'},
		{"dest", required_argument, 0, 'd'},
		{"input",required_argument, 0,'i'},
		{"timeout", required_argument, 0, 'o'},
		{"log", optional_argument, 0, 'l'},
		{"help", no_argument, 0, 'h'},
		{0,0,0,0}
	};

	if ( argc == 1 )
	{
		// Help message
		help_message(argv[0]);
		return EXIT_SUCCESS;
	}
	
	int nResp;
	int nOptIndex=0;
	while((nResp=getopt_long (argc, argv, "t:b:p:f:d:i:o:lh", long_options, &nOptIndex))!=EOF)
	{
		switch (nResp)
		{
			case 't':
				szSerialPort = optarg;	
				break;
				
			case 'b':
				if ((unBaudRate=CConfig::GetBaudRate(optarg))==-1)
				{
					printf("SpeedPort is invalid\n");
					return EXIT_FAILURE;
				}
				break;
			
			case 'd':
				// scan dest
				sscanf( optarg, "%u", &nDst );
				if ( !nDst || (nDst > 5) ) {
					printf( "ERROR: Invalid destination (format example: 1, 2, ..., 5)\n");
					return EXIT_FAILURE;
				}
				break;
				
			case 'i':
					szInputFile=optarg;
					//strncpy( szInputFile, optarg, sizeof(szInputFile));
				break;
			case 'o':
					// scan timeout
					if (sscanf(optarg, "%d" , &unTimeOut)!=1)
					{
						printf("ERROR: Invalid timeout\n");
						return EXIT_FAILURE;
					}
					if ((unTimeOut > 60)) 
					{
						printf( "ERROR: Invalid timeout (format example: 1, 2, ..., 60)\n");
						return EXIT_FAILURE;
					}
				break;
			case 'p': sscanf( optarg, "%u", &uTranciverNum );
				  if (uTranciverNum>2)
    				    {
					printf( "ERROR: Invalid transciver number 0, 1, 2\n");
					return EXIT_FAILURE;
				    }
				    
				    break;
			case 'f':sscanf(optarg, "%u", &uTimeOff);
				if (uTimeOff>300)
				{
					printf( "ERROR: Invalid timeoff value\n");
					return EXIT_FAILURE;
    				}
			
			case 'l':
					bLogData=true;
					break;
			case 'h':
					// Help message
					help_message(argv[0]);
					return EXIT_SUCCESS;

			case '?':
					//Invalid option or missing argument to option
					help_message(argv[0]);
					return EXIT_FAILURE;
			case EOF:
					continue;
			default:
					printf("Wrong parameters %d\n",nResp);
				return EXIT_FAILURE;
		}	
	}
	
	if (!szInputFile || !szInputFile[0])
	{
		printf("File to be loaded is missing\n");
		return EXIT_FAILURE;
	}


	
	g_stLog.OpenStdout();	
	bool bRetVal=theApp.MainTask((unsigned char *)szSerialPort, unBaudRate, nDst, szInputFile, unTimeOut*ONE_SEC_TIMEOUT, bLogData, uTranciverNum, uTimeOff);
	
	LOG(bRetVal?"File uploaded successfuly \n":"File upload failed\n");
	return  bRetVal? EXIT_SUCCESS : EXIT_FAILURE;

}
