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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <getopt.h>

#define DEBUG_SMTP_IO
#include "../Shared/SmtpSupport.h"
#include "../Shared/Common.h"


/*
If user is not provided no AUTH is tried.
smtp_send OPTIONS

-r, --server          - SMTP server address IP or DNS mandatory
-t, --to                   - destination email addresses comma or semicolon separated mandatory
-f, --from             - source email address default joe.doe@nivis.com
-s, --subject        - mail subject default
-b, --body            - mail body default
-u, --user             - user default
-a,--pass              - password default
-p, --port             - SMTP server port default 25

 */


extern char *optarg;
extern int optind, opterr, optopt;

void print_help(void)
{
	fprintf( stderr, "\nsmtp_send OPTIONS\n"\
		"-r, --server	- SMTP server address IP or DNS mandatory \n"\
		"-t, --to	- destination email addresses comma or semicolon separated mandatory\n"\
		"-f, --from	- source email address default nobody@nivis.com\n"\
		"-s, --subject	- mail subject default \"\"\n"\
		"-o, --file_body	- mail body read it from a file default \"\"\n"\
		"-b, --body	- mail body default \"\"\n"\
		"-u, --user	- user default \"\"\n"\
		"-a, --pass	- password default \"\"\n"\
		"-p, --port	- SMTP server port default 25");
}



int main(int argc, char **argv)
{
//	int nOpt;
	char* szServerAddress = NULL;
	char* szDestEmail = NULL;
	char* szFromEmail = (char *)"joe.doe@nivis.com";

	char cSubject[SUBJECTSIZE]={0,};
	char cUser[USERSIZE]={0,};
	char cPassword[PASSWORDSIZE]={0,};

	char *szBody=NULL;
	int nBodyLen = 0;
	unsigned short nPort=25;

	g_stLog.Open("/tmp/smtp_send.log");
	//getDNSMXServer("gmail.com");

	if (argc<2)
	{
		print_help();
		return EXIT_SUCCESS;
	}


	struct option long_options[] =
	{
		{"server", required_argument, 0, 'r'},
		{"to", required_argument, 0, 't'},
		{"from", required_argument, 0, 'f'},
		{"subject", required_argument, 0, 's' },
		{"file_body", required_argument, 0, 'o'},
		{"body", required_argument, 0, 'b'},
		{"user", required_argument, 0, 'u'},
		{"pass",required_argument, 0, 'a'},
		{"port",required_argument, 0, 'p'},
		{"help", no_argument, 0, 'h'},
		{0,0,0,0}

	};


	//while((nOpt=getopt(argc, argv, "r:t:f:s:b:o:u:a:p:hd"))!=-1)
	signed char cResp;

	int nOptIndex=0;
	if (argc<2)
	{
		LOG("Missing parameteres\n -h for help\n");
		return EXIT_FAILURE;
	}
	const char *szLongOpts="r:t:f:s:b:o:u:a:p:hd";

	cResp=getopt_long (argc, argv, szLongOpts,
						long_options, &nOptIndex);
	while(cResp!=-1)
	{

		//LOG("opt=%c", cResp);
		switch(cResp)
		{
			case 'r':
				//LOG("r: optarg=%s", optarg);
				szServerAddress = optarg;
				break;

			case 't':
				//LOG("t: optarg=%s", optarg);
				szDestEmail = optarg;

				break;

			case 'f':
				//LOG("f:optarg=%s", optarg);
				szFromEmail = optarg;
				break;

			case 's':
				//LOG("s:optarg=%s", optarg);
				strcpy( cSubject, optarg);
				break;

			case 'b':
				//LOG("b:optarg=%s", optarg);

				//szBody = optarg;
				nBodyLen = strlen(optarg);
				szBody=new char[nBodyLen];
				strcpy(szBody, optarg);
				break;

			case 'o':

				if (!(optarg && optarg[0]))
				{
					LOG(" Missing parameter for body file");
					return 0;
				}

				//LOG("b:optarg=%s", optarg);

				if (!GetFileData(optarg,szBody,nBodyLen))
				{
					LOG("File %s can not be read it", optarg);
					return EXIT_FAILURE;
				}
				break;

			case 'u':
				//LOG("User:optarg=%s", optarg);
				strcpy( cUser, optarg);
				break;

			case 'a':
				//LOG("Password:optarg=%s", optarg);
				strcpy( cPassword, optarg);
				break;

			case 'p':
				//LOG("Port:optarg=%s", optarg);
				nPort=atoi(optarg);
				break;

			case 'd':

				break;

			case 'h':
				//LOG("h:optarg=%s\n", optarg);
				print_help();
				return EXIT_SUCCESS;

			case '?':
				LOG("parameter not needed: optarg=%s", optarg);
				break;
			default:
				LOG("parameter not needed: optarg=%s", optarg);
				break;
		}

		cResp=getopt_long (argc, argv, szLongOpts,	long_options, &nOptIndex);
	}


	CSmtpSupport oSmtpSupport;
	bool bRet;
	// The same 'if' is done inside bool CSmtpSupport::SMTP_SendMail.
	// Should remove one of them (redundancy)
	if (szServerAddress && szServerAddress[0])
	{
		bRet = oSmtpSupport.SMTP_SendMailToServer( szFromEmail, szDestEmail, cSubject,
						szBody, szServerAddress, cUser , cPassword , nPort );
	}
	else
	{
		bRet = oSmtpSupport.SMTP_SendMail( szFromEmail, szDestEmail, cSubject,
 					szBody,  cUser , cPassword , nPort );
	}



	LOG("SMTP send email %s", bRet ? "SUCCESS" : "FAILED" );

	delete szBody;

	return !bRet;
}

