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

#ifndef __SMTP_SUPPORT__
#define __SMTP_SUPPORT__

/// @addtogroup libshared
/// @{

#include "TcpSocket.h"
#include "udns.h"

#define 	MAXSRVNR 10
struct DNSServerList
{
	char szName[1024];	
	int nPr;
};

struct DNSQuery
{
	const char *szName;		/* original query string */
	unsigned char *szDomainName;		/* the DN being looked up */
	enum dns_type nQueryType;		/* type of the query */
	int nNotFound;
	int nErrors;
	int nMinInd;
	int nNrSrv;
	enum dns_class nQueryClass;
	DNSServerList serverList[MAXSRVNR];
};

bool getDNSMXServer(char *p_szName,DNSServerList serverList[],int &nMinInd, int &nNrSrv);

class CSmtpSupport
{
public:
	void Print1310(const char *p_szBody);
	
	bool SmtpAuth(const char *p_szUser, const char * p_szPassword);

	bool SmtpOpen(const char* srv, int nPort);

	bool SmtpData(const char* msg);

	bool CreateSmtpCommand( const char *p_szCommand, const char *p_szMsg, char *p_szResultString, size_t p_nMaxSize);

	bool SmtpCommand(const char *p_szCommand, int p_nCommandCode);
	
	
//	bool SMTP_SendMail ( const char* p_szFrom, const char* p_szTo, const char* p_szSubject, 
//							const char* p_szBody, int p_nSMTPserverPort = 25);
	bool SMTP_SendMail (const char* p_szFrom, const char* p_szTo, const char* p_szSubject,
						const char* p_szBody,  const char* p_szUser = "",
						const char* p_szPassword = "", int p_nSMTPserverPort = 25, const char* p_szSMTPserver="");


	bool SMTP_SendMailToServer (const char* p_szFrom, const char* p_szTo, const char* p_szSubject, 
						const char* p_szBody, const char* p_szSMTPserver, const char* p_szUser = "", 
						const char* p_szPassword = "", int p_nSMTPserverPort = 25);	
	

private:
	
	//
	/*static  void QueryFree(DNSQuery *p_pQuery);*/
	/*static void
	DnsError(DNSQuery *p_pQuery, int p_nErrNum);*/
	/*static struct DNSQuery *
	QueryNew(const char *p_szName, const unsigned char *p_szDomain, enum dns_type nQueryType);*/
	/*static  void DNSCallBack(dns_ctx *p_Context, void *p_Result, void *p_pData);*/
	;	
	

private:
	CTcpSocket oSocket;
	
	bool IsErrorConnection(int nRetCode);

	bool IsErrorEhlo(int nRetCode);

	bool IsErrorMailFrom(int nRetCode);

	bool IsErrorRcptTo(int nRetCode);

	bool IsErrorData(int nRetCode);

	bool IsErrorRset(int nRetCode);
	bool IsErrorVrfy(int nRetCode);

	bool IsErrorExpn(int nRetCode);

	bool IsErrorHelp(int nRetCode);

	bool IsErrorNoop(int nRetCode);

	bool IsErrorQuit(int nRetCode);

	bool IsErrorAuthLogin(int nRetCode);

	bool IsErrorSmtp(char *p_szReturnMessage,int nCommand);

	bool _read( void *buf, int count);

	bool _write( const void *buf, int count);

	int FormatBody(char *p_szBody, const char* p_szOrigBody, int p_nMaxBufferLen);

	bool CreateAngleBrackets(const char *p_szInString, char *p_szOutString);

	void DeleteBlanks(char *p_szStr);
	
	void SmtpLogCrlf( const char* p_szMsg );
	
	void ArrayCopy(char p_szDestionation[], const char* p_szSource);
	int ArraySize(const char *p_szArray);


	#define DEBUG_SMTP_IO_READ\
			LOG("SMTP_RECV: %.*s", 150, (char *)buf)
	
	#define DEBUG_SMTP_IO_WRITE\
			LOG("SMTP_SEND: %.*s", 150, (char *)buf)

					
	#define SERVERSIZE 256
	#define DESTSIZE 256

	#define FROMSIZE 256
	#define SUBJECTSIZE 1024
	#define USERSIZE 1024
	#define PASSWORDSIZE 256
	
	
	#define CONNECTION_ESTABLISHMENT 0

	#define  HELO	1
	#define STR_HELO "HELO"
	
	#define EHLO	1
	#define STR_EHLO "EHLO"
	
	//MAIL FROM:
	#define MAIL_FROM	2
	#define STR_MAIL_FROM "MAIL FROM:"
	
	//RCPT TO:
	#define RCPT	3
	#define STR_RCPT "RCPT TO:"
	
	#define DATA	4
	#define STR_DATA "DATA"
	
	#define RSET	5
	#define STR_RSET "RSET"
	
	#define VRFY	6
	#define STR_VRFY "VRFY"
	
	#define EXPN	7
	#define STR_EXPN "EXPN"
	
	#define HELP	8
	#define STR_HELP "HELP"
	
	#define NOOP	9
	#define STR_NOOP "NOOP"
	
	#define QUIT	10
	#define STR_QUIT "QUIT"
	
	#define AUTH_LOGIN 11
	//#define 
	
	/*
	struct SMTPCommands
	{
		int nCommand;
		char szCommand[];
	}={{
	*/
	
	#define TO_DELIM ";,"
	//#define BODYSIZE 4096
	

};

namespace SMTP
{
	class CBase64 
	{
	public:
		CBase64();
		~CBase64() {} ;

		static int Encode( const unsigned char * p_pBuffer, int p_nBufferLen, unsigned char * p_pRsp, int * p_pRspLen );
		static int Decode( const unsigned char * p_pBuffer, int p_nBufferLen, unsigned char * p_pRsp, int * p_pRspLen );
 
	private:
		static unsigned char m_aTable[ 64 ];
		static unsigned char m_aRevTable[ 256 ];
};					

}

/// @}
#endif


