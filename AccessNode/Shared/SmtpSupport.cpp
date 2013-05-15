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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>

#include <sys/time.h>
#include <time.h>
#include <limits.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

#include "SmtpSupport.h"
#include "Common.h"
#include "TcpSocket.h"
#include "udns.h"


void QueryFree(DNSQuery *p_pQuery) 
{
	free(p_pQuery->szDomainName);
	free(p_pQuery);
}


void
DnsError(DNSQuery *p_pQuery, int p_nErrNum )
{
	LOG ("Unable to lookup %s record for %s: %s", 
				dns_typename(p_pQuery->nQueryType), dns_dntosp(p_pQuery->szDomainName), dns_strerror(p_nErrNum));
				
 	if (p_nErrNum == DNS_E_NXDOMAIN || p_nErrNum == DNS_E_NODATA)
		++p_pQuery->nNotFound;
	else
		++p_pQuery->nErrors;
		
	//QueryFree(p_pQuery);
}



struct DNSQuery * QueryNew(const char *p_szName, const unsigned char *p_szDomain, enum dns_type nQueryType) 
{
	DNSQuery *pQuery = (DNSQuery *)malloc(sizeof(*pQuery));
	unsigned nLen = dns_dnlen(p_szDomain);
	unsigned char *pcCanonicalDName  =(unsigned char *) malloc(nLen);
	if (!pQuery || !pcCanonicalDName) 
	{
 		LOG("ERROR: malloc() cold not allocate memory");
 		return NULL;
	}
	memcpy(pcCanonicalDName, p_szDomain, nLen);
	pQuery->szName = p_szName;
	pQuery->szDomainName = pcCanonicalDName;
	pQuery->nQueryType = nQueryType;
	pQuery->nErrors=pQuery->nNotFound=0;
	pQuery->nQueryClass = DNS_C_IN;
	return pQuery;
}


void DNSCallBack(dns_ctx *p_Context, void *p_Result, void *p_pData)
{
	int nR = dns_status(p_Context);
	DNSQuery *pQuery = (DNSQuery  *)p_pData;
	dns_parse parse;
	// DNS Resource Record 
	dns_rr ResRec;
	
	unsigned nResRec;
	unsigned char dn[DNS_MAXDN]={0,};
	const unsigned char *pcPkt, *pcCur, *pcEnd;
	if (!p_Result) 
	{
		DnsError(pQuery, nR);
		return;
	}
	
	pcPkt =  (unsigned char *)p_Result; 
	pcEnd = pcPkt + nR; 
	pcCur = dns_payload(pcPkt);
	
	dns_getdn(pcPkt, &pcCur, pcEnd, dn, sizeof(dn));
	dns_initparse( &parse, NULL, pcPkt, pcCur, pcEnd);
	parse.dnsp_qcls = (dns_class)0;
	parse.dnsp_qtyp = (dns_type) 0;
	nResRec = 0;
	
	while((nR = dns_nextrr(&parse, &ResRec)) > 0)
	{
		if (!dns_dnequal(dn, ResRec.dnsrr_dn)) 
			continue;
			
		if ((pQuery->nQueryClass == DNS_C_ANY || pQuery->nQueryClass == ResRec.dnsrr_cls) &&	(pQuery->nQueryType == DNS_T_ANY || pQuery->nQueryType == ResRec.dnsrr_typ))
			++nResRec;
		else if (ResRec.dnsrr_typ == DNS_T_CNAME && !nResRec) 
		{
			if (dns_getdn(pcPkt, &ResRec.dnsrr_dptr, pcEnd, parse.dnsp_dnbuf, sizeof(parse.dnsp_dnbuf)) <= 0 || ResRec.dnsrr_dptr != ResRec.dnsrr_dend)
			{
				nR = DNS_E_PROTOCOL;
				break;
			}
			else 
			{
				dns_dntodn(parse.dnsp_dnbuf, dn, sizeof(dn));
			}
		}
	}
	
	if (!nR && !nResRec)
		nR = DNS_E_NODATA;
		
	if (nR < 0) 
	{
		DnsError( pQuery, nR);
		return;
	}
	
	// else it is already printed by dbgfn
	dns_rewind(&parse, NULL);
	parse.dnsp_qtyp = dns_type ( pQuery->nQueryType == (DNS_T_ANY ? 0 : pQuery->nQueryType));
	parse.dnsp_qcls = dns_class (pQuery->nQueryClass == (DNS_C_ANY ? 0 : pQuery->nQueryClass));
	
	int nSrvInd=0;
	int nMin=1000;
	while(dns_nextrr(&parse, &ResRec))
	{
		/*const unsigned char **/pcPkt = parse.dnsp_pkt;
		const unsigned char *end = parse.dnsp_end;
		const unsigned char *dptr = ResRec.dnsrr_dptr;
		const unsigned char *dend = ResRec.dnsrr_dend;
		unsigned char *pcDomainName = ResRec.dnsrr_dn;
		const unsigned char *pCh=dptr + 2;	
		//PrintResolverRec(&parse, &ResRec);
		
		if (ResRec.dnsrr_typ==DNS_T_MX)
		{
			pCh = dptr + 2;
			if ((dns_getdn(pcPkt, &pCh, end, pcDomainName, DNS_MAXDN) <= 0 || pCh != dend)) //goto xperr;
			{
				LOG("<parse error>");
				++pQuery->nErrors;	
				return ;
			}
///			LOG("%d %s.", dns_get16(dptr), dns_dntosp(pcDomainName));
			pQuery->serverList[nSrvInd].nPr=dns_get16(dptr);
			if (nMin>pQuery->serverList[nSrvInd].nPr) 
			{
//				LOG("serverList[nSrvInd].nPr=%d",pQuery->serverList[nSrvInd].nPr);
				nMin=pQuery->serverList[nSrvInd].nPr;
				pQuery->nMinInd=nSrvInd;
			}
			strcpy(pQuery->serverList[nSrvInd].szName, dns_dntosp(pcDomainName));
			(nSrvInd<MAXSRVNR)?nSrvInd++:nSrvInd;
		}
		else
		{ 
			LOG("ERROR :rr->dnsrr_typ!=DNS_T_MX\n");
			return ;
		}	
		
	}
	pQuery->nNrSrv=nSrvInd;
	free(p_Result);
}

bool getDNSMXServer(char *p_szName,DNSServerList serverList[],int &nMinInd, int &nNrSrv)
{
	int nInd;
	int nFd;
	fd_set fds;
		
	timeval tv;
	time_t now;
	
	DNSQuery *pQuery;
	enum dns_type nQueryType = (dns_type) 0;
	dns_ctx *context = NULL;

	if (dns_init(NULL, 0) < 0 || !(context = dns_new(NULL)))
	{
		LOG("ERROR:Unable to initialize udns library");
		return false;
	}
	dns_free(context);
	
	nFd = dns_open(NULL);
	if (nFd < 0)
	{
		LOG("ERROR:Unable to initialize uDNS context");
		return false;
	}
	nQueryType = DNS_T_MX;
	
///	LOG("p_szName=%s", p_szName);
	
	unsigned char szDomainName[DNS_MAXDN];
	
	int nAbs;
	
	if (!dns_ptodn(p_szName, strlen(p_szName), szDomainName, sizeof(szDomainName), &nAbs))
	{
		LOG("ERROR:invalid name `%s'", p_szName);
		return false;
	}
		
	pQuery = QueryNew(p_szName, szDomainName, nQueryType);
	
	if (nAbs) 
		nAbs = DNS_NOSRCH;
		
	if (!dns_submit_dn(NULL, szDomainName, pQuery->nQueryClass, pQuery->nQueryType , nAbs, 0, DNSCallBack, pQuery))
	{
		DnsError(pQuery, dns_status(NULL));
	}
	
	FD_ZERO(&fds);
	now = 0;
	while((nInd = dns_timeouts(NULL, -1, now)) > 0) {
		FD_SET(nFd, &fds);
		tv.tv_sec = nInd;
		tv.tv_usec = 0;
		nInd = select(nFd+1, &fds, 0, 0, &tv);
		now = time(NULL);
		if (nInd > 0) dns_ioevent(NULL, now);
	}
///	LOG("nErrors=%d , nNotFound =%d", pQuery->nErrors, pQuery->nNotFound);
	memcpy(serverList,pQuery->serverList, sizeof(serverList[0])*MAXSRVNR);
	
	nMinInd=pQuery->nMinInd;
	nNrSrv=pQuery->nNrSrv;
	int nNotFound=pQuery->nNotFound;
	int nErrors=pQuery->nErrors;
	
/*	printf("\n------------------------------------------------------------------\n");
	printf("\n------------------------------------------------------------------\n");
	printf("nMinInd=pQuery->nMinInd=%d\n",nMinInd);
	printf("nNrSrv=pQuery->nNrSrv=%d", nNrSrv);
	printf("serverList[nMinInd].szName=%s",pQuery->serverList[nMinInd].szName);
	printf("\n------------------------------------------------------------------\n");
	printf("\n------------------------------------------------------------------\n");
*/	
	QueryFree(pQuery);
	return nErrors ? false : nNotFound ? false : true;
}



bool CSmtpSupport::IsErrorConnection(int nRetCode)
{
	switch (nRetCode)
	{
		// SMTP commands see in RFC 2821,  4.3.2 Command-Reply Sequences
		//CONNECTION ESTABLISHMENT
		case 220:
				return true;
				break; 
    	case 554:return false;
				break;
		default:
				return false;
	}
	
}

bool CSmtpSupport::IsErrorEhlo(int nRetCode)
{
	switch (nRetCode)
	{
		//EHLO or HELO
		case 250:return true;break;
		case 504:
		case 550:return false;
			break;
		default:return  false;
	}
}

bool CSmtpSupport::IsErrorMailFrom(int nRetCode)
{
	switch (nRetCode)
	{
		//MAIL
		case 250: return true;
		case 552:case 451:case 452:case 550:case 553:case 503: return false; break;
		default:return  false;
	}
}

bool CSmtpSupport::IsErrorRcptTo(int nRetCode)
{
	//LOG("IsErrorRcptTo %d", nRetCode);
	switch (nRetCode)
	{
		//RCPT
		case 250:case 251:return true; //(but see section 3.4 for discussion of 251 and 551)
		case 550:case 551:case 552:case 553:case 450:case 451:case 452:case 503: return false;break;
		default:return  false;
	}
}

bool CSmtpSupport::IsErrorData(int nRetCode)
{
	switch (nRetCode)
	{
		//DATA
		//I: 354 -> data 
		case 354:case 250: return true;
		case 552:case 554:case 451:case 452:return false;break;	
		case 503:return false;break;
		
		default:
			return  false;
	}
}

bool CSmtpSupport::IsErrorRset(int nRetCode)
{
		//RSET
	if (nRetCode==250)
	{
		return true;
	}
	else {
		return false;
	}
}

bool CSmtpSupport::IsErrorVrfy(int nRetCode)
{
	switch (nRetCode)
	{
	//VRFY
		case 250:case 251:case 252:return true;
		case 550:case 551:case 553:case 502:case 504:return false;
		default:return  false;		
	}
}

bool CSmtpSupport::IsErrorExpn(int nRetCode)
{
	switch (nRetCode)
	{
	//EXPN
		case 250:case 252: return true;
		case 550:case 500: case 502:case 504:return false;
		default:return  false;
	}
}

bool CSmtpSupport::IsErrorHelp(int nRetCode)
{
	switch (nRetCode)
	{
	//HELP
		case 211:case 214:return true;
		case 502:case 504:return false;
		default:return false;
	}
}
bool CSmtpSupport::IsErrorNoop(int nRetCode)
{
	//NOOP
	if (nRetCode==250)
	{
		return true;
	}
	else 
	{
		return false;
	}
}

bool CSmtpSupport::IsErrorQuit(int )
{
	//QUIT
	/*
	if (nRetCode==221){
		return true;
	}
	else
	{
		return false;
	}
	*/
	//FORCE ALWAYS return TRUE
	return true;
}



bool CSmtpSupport::IsErrorAuthLogin(int nRetCode)
{
	//SMTP Authentication rfc 2554
	switch (nRetCode)
	{
		case 235:case 334:return true;
		case 432:case 534: case 538: 
		case 454: case 530: case  535:return false;
		default:return false;
	}
	return false;
}


bool CSmtpSupport::IsErrorSmtp(char *p_szReturnMessage,int nCommand)
{
	if (!p_szReturnMessage)
	{
		LOG("ERROR IsError(): p_szReturnMessage NULL");
		return false;
	}
 	char szReturnCode[4]={0};
	strncpy(szReturnCode, p_szReturnMessage, 3);
	//LOG("p_szReturnMessage=%s", p_szReturnMessage);
	
	int nReturnCode=atoi(szReturnCode);
	//LOG("szReturnCode=%d", nReturnCode);
	//Specific error handle of SMTP command
	bool bRetValue=false;
	switch (nCommand)
	{
		case CONNECTION_ESTABLISHMENT:
			bRetValue=IsErrorConnection(nReturnCode);
			break;
		case EHLO:
			bRetValue=IsErrorEhlo(nReturnCode);
			break;
		case MAIL_FROM:
			bRetValue=IsErrorMailFrom(nReturnCode);
			break;
		case RCPT:
			bRetValue=IsErrorRcptTo(nReturnCode);
			break;
		case DATA:
			bRetValue=IsErrorData(nReturnCode);
			break;
		case RSET:
			bRetValue=IsErrorRset(nReturnCode);
			break;
		case VRFY:bRetValue=IsErrorVrfy(nReturnCode);
			break;
		case EXPN:
			bRetValue=IsErrorExpn(nReturnCode);
			break;
		case HELP:
			bRetValue=IsErrorHelp(nReturnCode);
			break;
		case NOOP:
			bRetValue=IsErrorNoop(nReturnCode);
			break;
		case AUTH_LOGIN:
			bRetValue=IsErrorAuthLogin(nReturnCode);
			break;
		case QUIT:
			bRetValue=IsErrorQuit(nReturnCode);
			break;
		default:
		{
			LOG("ERROR IsError() Unkown SMTP command code");
			bRetValue=false;
		}	
	}
	
	if (bRetValue==false)
	{	
		LOG("ERROR: server error message=%.*s", 200, p_szReturnMessage);
	}
	return bRetValue;
}

bool CSmtpSupport::_read( void *buf, int count)
{
	//usleep(1000*500);
	
	if (!buf)
	{
		LOG("ERROR buf==NULL ");
		return false;
	}
	//LOG("buf=%s\n", (char *)buf);
	if (!oSocket.CheckRecv(10 * 1000000))
	{	
		LOG("_read(): 10s nothing received");
		return false;	
	}
	
	int nRead = count;
	if (!oSocket.Recv(buf, nRead, 0 ))
	{		
		return false;
	}
	
	int nPos = nRead;
	*((char*)buf + nPos) = 0;	
	
	clock_t nTime = GetClockTicks();
	
	int nRet = true;
	while (nTime + 10 * sysconf( _SC_CLK_TCK ) > GetClockTicks()  )
	{
		if (!oSocket.CheckRecv(2 * 1000000))
		{	
			//LOG("Wait another 2s");			
			break;
		}
		
		nRead = count - nPos;
		if (!oSocket.Recv((char*)buf + nPos, nRead, 0 ))
		{			
			nRet = false;
			break;
		}
		
		nPos += nRead;	
	}
	
	*((char*)buf + nPos) = 0;	
	//LOG("%.*", 120, buf );	
	DEBUG_SMTP_IO_READ;
	return nRet;
}

bool CSmtpSupport::_write( const void *buf, int count)
{
	DEBUG_SMTP_IO_WRITE;
	return oSocket.Send(buf,count);
}



bool CSmtpSupport::SmtpOpen( const char *sz_Srv, int p_nPort )
{
	if(!oSocket.Create())
	{	
		return false;
	}
	
	if (!oSocket.Connect (sz_Srv, p_nPort))
	{
		LOG("ERROR: The email server (%s:%d) does not allow connections.\n"
			"\tPosible reasons: \n"
			"\t\t 1. there is no email server at (%s:%d) \n"
			"\t\t 2. you are behind a firewall and port %d is blocked", 
			sz_Srv,p_nPort, sz_Srv,p_nPort,p_nPort);

		return false;
	}
	
	char sz_ReadBuffer[1024]={0};
	if (!_read( sz_ReadBuffer, sizeof(sz_ReadBuffer)-1 ))
	{
		LOG("ERROR: (%s:%d) is too busy or is NOT a email server",sz_Srv,p_nPort);
		return false;
	}
	return true;
	//return IsErrorSmtp(sz_ReadBuffer, p_nCommandCode);
}

bool CSmtpSupport::SmtpAuth(const char *p_szUser, const char * /*p_szPassword*/)
{
	if (!p_szUser)
	{
		return false;
	}
/*
        static int Encode( const unsigned char * p_pBuffer, int p_nBufferLen, unsigned char * p_pRsp, int * p_pRspLen );
        static int Decode( const unsigned char * p_pBuffer, int p_nBufferLen, unsigned char * p_pRsp, int * p_pRspLen );
*/	
	const char* pszMsg = "auth login\r\n";
	if (!_write(pszMsg, strlen(pszMsg)))
	{		
		return false;
	}
	
	char szBuf[512]={0};
	
	//LOG("read after _write(auth login)");
	_read(szBuf, sizeof(szBuf));
	if (!IsErrorSmtp(szBuf,AUTH_LOGIN))
	{
		LOG("Error SMTP command AUTH LOGIN fail");
		return false;
	}
	SMTP::CBase64 coBase64;
	
 	unsigned char pBuffer[1024]={0,};
	int nRespBufferLen=sizeof(pBuffer);
	if (!coBase64.Encode((unsigned char *)p_szUser, strlen(p_szUser), pBuffer, &nRespBufferLen))
	{
		LOG("ERROR: CBase64::Encode() for user return 0");
		return false;
	}
	strcat((char *)pBuffer, "\r\n");
	Print1310((char *)pBuffer);
	
	unsigned int nLen=strlen((char *)pBuffer);
	if (!_write((char *)pBuffer, nLen))
	{
		LOG("In SmtpAuth() _write return false");
 		return false;
	}
	//LOG("read after _write(user)");
	memset(szBuf, 0, sizeof(szBuf));
	_read(szBuf, sizeof(szBuf));
	
	if (!IsErrorSmtp(szBuf,AUTH_LOGIN))
	{
		LOG("Error SMTP command AUTH LOGIN user fail");
		return false;
	}
	
	
	memset(pBuffer, 0, sizeof(pBuffer));
	
	if (!coBase64.Encode((unsigned char *)p_szUser, strlen(p_szUser), pBuffer, &nRespBufferLen))
	{
		LOG("ERROR: CBase64::Encode() for password return 0");
		return false;
	}
	
	strcat((char *)pBuffer, "\r\n");
	//LOG("\n_write(password\n");
	
	nLen=strlen((char *)pBuffer);
 	if (!_write((char *)pBuffer, nLen))
	{
		LOG("In SmtpAuth() _write return false");
		return false;
	}
	
	memset(szBuf, 0, sizeof(szBuf));
	_read(szBuf, sizeof(szBuf));	
	if (!IsErrorSmtp(szBuf,AUTH_LOGIN))
	{
		LOG("Error SMTP command AUTH LOGIN password fail");
		return false;
	}

	return true;
}

bool CSmtpSupport::SmtpData( const char *p_szMsg )
{
	
	if (!_write(p_szMsg, strlen(p_szMsg)))
	{
		return false;
	}
	
	char sz_Buf[1024]={0,};
	//memset(sz_Buf, 0, sizeof(sz_Buf));
	
	if (!_read( sz_Buf, sizeof(sz_Buf)-1 ))
	{
		return false;
	}

	return IsErrorSmtp(sz_Buf, DATA);
}


void CSmtpSupport::Print1310(const char *p_szBody)
{
	int nBodyInd=0;	
	//LOG("Print1310()");

	while(p_szBody[nBodyInd])
	{	
		//LOG("c=%d nBodyInd=%d\n", p_szBody[nBodyInd], nBodyInd);

		if (p_szBody[nBodyInd]!=10 && p_szBody[nBodyInd]!=13)
		{
			//LOG("c=%d", p_szBody[nBodyInd]);
			g_stLog.WriteInLine("%c",p_szBody[nBodyInd++]);
			continue;
		}	
 		if (p_szBody[nBodyInd]==13)
		{
			if (p_szBody[++nBodyInd]==10)
			{
				LOG("1310");
				++nBodyInd;
				continue;
			}
			LOG("13");
			++nBodyInd;
			continue;
		}

		if (p_szBody[nBodyInd]==10)
		{
			LOG("10\n");
			nBodyInd++;
 			continue;
		}
		
	}
}


int CSmtpSupport::FormatBody(char *p_szBody, const char* p_szOrigBody, int p_nMaxBufferLen)
{
	if (p_nMaxBufferLen < 3)
	{
		LOG("FormatBody: no space in buffer in output buffer");
		p_szBody[0] = 0;
		return -1; // no space in buffer
	}

	int nPosOrig = 0;
	int nPosDest = 0;

	if (p_szOrigBody[0] == '.')
	{
		p_szBody[nPosDest++] = '.';
		p_szBody[nPosDest++] = '.';
		nPosOrig++;
	}

	p_nMaxBufferLen -= 5; // \n \r . . \0
	for (; p_szOrigBody[nPosOrig]; )
	{
		if (nPosDest > p_nMaxBufferLen)
		{
			LOG("FormatBody: no space in buffer in output buffer");
			p_szBody[nPosDest] = 0;
			return -1; // no space in buffer
		}

		if (	p_szOrigBody[nPosOrig] != '\n' && p_szOrigBody[nPosOrig] != '\r' )
		{
			p_szBody[nPosDest++] = p_szOrigBody[nPosOrig++];
			continue;
		}
		
		if ( p_szOrigBody[nPosOrig] == '\n')
		{
			p_szBody[nPosDest++] = '\r';
			p_szBody[nPosDest++] = '\n';
		}
		else if (p_szOrigBody[nPosOrig] == '\r' && p_szOrigBody[nPosOrig + 1] != '\n' ) 
		{
			p_szBody[nPosDest++] = '\r';
			p_szBody[nPosDest++] = '\n';			
		}
		else if (p_szOrigBody[nPosOrig] == '\r' && p_szOrigBody[nPosOrig + 1] == '\n' ) 
		{
			p_szBody[nPosDest++] = '\r';
			p_szBody[nPosDest++] = '\n';			
			nPosOrig++;
		}

		if (p_szOrigBody[nPosOrig+1] == '.')
		{
			p_szBody[nPosDest++] = '.';		
		}

		nPosOrig++;
	}

	p_szBody[nPosDest] = 0;

	return nPosDest;
}



bool CSmtpSupport::CreateAngleBrackets(const char *p_szInString, char *p_szOutString)
{	
	if (p_szInString==NULL)
	{
		LOG("ERRRO:In  CreateAngleBrackets() p_szInString==NULL");
		return false;
	}
	
	if (p_szOutString==NULL)
	{
		LOG("ERRRO:In  CreateAngleBrackets() p_szInString==NULL");
		return false;
	}

	
	int nLength=strlen(p_szInString);

	*p_szOutString='<';
	
	memcpy((char *)(p_szOutString+1),p_szInString,nLength);
	
	*(p_szOutString+nLength+1)='>';
	*(p_szOutString+nLength+2)='\0';
	
	return true;
}

void CSmtpSupport::DeleteBlanks(char *p_szStr)
{
	int nRead = 0;
	int nWrite = 0;

	for (;p_szStr[nRead];nRead++)
	{
		if (!isblank(p_szStr[nRead]))
		{
			p_szStr[nWrite++] = p_szStr[nRead];
		}
	}
	p_szStr[nWrite] = 0;
}


void CSmtpSupport::SmtpLogCrlf( const char* p_szMsg )
{
	char szTmp[4*strlen(p_szMsg)+1];

	int i=0;
	int nPos=0;
	for (;p_szMsg[i];i++)
	{
		if (p_szMsg[i] == '\n')
		{
			nPos += sprintf(szTmp+nPos,"<CR>");
			continue;
		}
		if (p_szMsg[i] == '\r')
		{
			nPos += sprintf(szTmp+nPos,"<LF>\n");
			continue;
		}
		nPos += sprintf(szTmp+nPos,"%c",p_szMsg[i]);
	}
	LOG(szTmp);
}

struct ToAddr{
	char *szToAddresses;
	char *szToDomains;
};


static int 
CmpFunc(const void *p_Str1, const void *p_Str2)
{
//	LOG("%s %s \n", (const char *)(((ToAddr *)p_Str1)->szToDomains),  
//				  (const char *)((ToAddr *)p_Str2)->szToDomains);
	return strcmp((const char *)(((ToAddr *)p_Str1)->szToDomains),  
				  (const char *)((ToAddr *)p_Str2)->szToDomains);
}

bool CSmtpSupport::SMTP_SendMail (const char* p_szFrom, const char* p_szTo, const char* p_szSubject,
					const char* p_szBody,  const char* /*p_szUser*/ /* = "" */,
					const char* /*p_szPassword*/ /*= "" */, int /*p_nSMTPserverPort*/ /* = 25 */, const char* p_szSMTPserver /*="" */)
{
	if (p_szSMTPserver && p_szSMTPserver[0])
	{
		return SMTP_SendMailToServer (p_szFrom, p_szTo, p_szSubject, p_szBody,p_szSMTPserver);
	}

	bool retVal=true;
	
	if ( !p_szFrom || !p_szTo || !p_szSubject || !p_szBody )
	{
		LOG("SMTP_SendMail: some mandatory parameters are null");
		return false;
	}
	
	if (strchr(p_szTo,'@') == NULL)
	{
		LOG("SMTP_SendMail: invalid To field: %s", p_szTo);
		return false;
	}

	int nToLen = strlen(p_szTo)+1;	
	char szTmpTo[ArraySize(p_szTo)];
	szTmpTo[0]=0;
	ArrayCopy(szTmpTo, p_szTo);
	
	DeleteBlanks(szTmpTo);

	
//	LOG("p_szFrom=%s, p_szTO=%s, p_szSubject=%s,p_szBody=%s, serverList[nMinInd].szName%s", 
//		p_szFrom,  szTmpTo, p_szSubject, p_szBody, serverList[nMinInd].szName);

//	char szDomainPart[nToLen];
//	szDomainPart[nToLen-1]=0;
	
	char* szTmpTo4Strtok;
	char* szSavePoint;
	
	ToAddr ToList[1024];
	

	
	int nInd=0;
	for (szTmpTo4Strtok = szTmpTo;;szTmpTo4Strtok = NULL)
	{
		char* szToAddress = strtok_r(szTmpTo4Strtok, TO_DELIM, &szSavePoint);	
		if (szToAddress  == NULL)
		{
			break;
		}
		ToList[nInd++].szToAddresses=szToAddress ;
	}
	
///	LOG("nInd=%d\n", nInd);
	//ToList[nInd].szToAddresses[0]="";
	//ToList[nInd].szToDomains[0]="";
	for (int i=0;i<nInd;i++)
	{		
		ToList[i].szToDomains=strchr(ToList[i].szToAddresses,'@')+1;
///		LOG("szDomains[%d]%s", i, ToList[i].szToDomains);
	}
	
	qsort(&ToList[0], nInd,sizeof(ToList[1]),CmpFunc);
	//getDNSMXServer(szDomain);	
///	LOG("================================================\nSort():");
///	for (int i=0;i<nInd;i++)
///	{
		//szDomains[i]=strchr(szDomains[i],'@')+1;
		//char *szToAddresses;
		//char *szToDomains
///		LOG("szDomains[%d]=%s %s",i, ToList[i].szToDomains, ToList[i].szToAddresses);
///	}
	
///	LOG("================================================\nSame domains:():");
//	bool bEndTo=true;
	
	char szSameDomainAddr[nToLen];
	szSameDomainAddr[0]=0;
	strcat(szSameDomainAddr,ToList[0]. szToAddresses);
	//strcat(szSameDomainAddr,",");
	//char **szServerList=NULL;
	int i;
	bool nDNSMXres=true;
	for (i=0;i<nInd-1;i++)
	{	
///		char *szTmp=szSameDomainAddr;
			//szDomains[i]=strchr(szDomains[i],'@')+1;
///		LOG("szDomains[%d]=%s",i, ToList[i].szToDomains);
		if (/* !szToDomains[i+1] && */ strcmp(ToList[i].szToDomains,ToList[i+1].szToDomains)!=0)
		{
///			LOG("szSameDomainAddr=%s\n", szSameDomainAddr);
			
			DNSServerList serverList[MAXSRVNR];
			memset(serverList,0,sizeof(serverList));

			int nMinInd=0;
			int nNrSrv=0;
 			nDNSMXres=getDNSMXServer(ToList[i]. szToDomains,serverList,nMinInd,nNrSrv);
 			LOG("nDNDMXres=%s", nDNSMXres?"true":"false");
 			if (!nDNSMXres)
 			{
 				LOG("Cannot send to %s domain", ToList[i]. szToDomains);
 				szSameDomainAddr[0]=0;
				strcat(szSameDomainAddr,ToList[i+1].szToAddresses);
				retVal=false;
				continue;
 			}
			//serverList[nMinInd].nPr, serverList[nMinInd].szName

//			LOG("Send emails:\n");
			/*
			LOG("p_szFrom=%s, szSameDomainAddr=%s, p_szSubject=%s,p_szBody=%s, serverList[nMinInd].szName%s", 
				p_szFrom, szSameDomainAddr, p_szSubject,p_szBody, serverList[nMinInd].szName);
			*/
			
///			LOG("p_szFrom=%s, szSameDomainAddr=%s, p_szSubject=%s, serverList[nMinInd].szName%s", 
///				p_szFrom, szSameDomainAddr, p_szSubject,serverList[nMinInd].szName);

			SMTP_SendMailToServer(p_szFrom, szSameDomainAddr, p_szSubject,p_szBody, serverList[nMinInd].szName);
			
			szSameDomainAddr[0]=0;
			strcat(szSameDomainAddr,ToList[i+1].szToAddresses);
		}
		else
		{
			//strcpy(szTmp,szToAddresses[i]);
			//szTmp=szToAddresses[i]
			strcat(szSameDomainAddr,",");
			strcat(szSameDomainAddr, ToList[i+1].szToAddresses);
			
			
///			LOG("ToList:szSameDomainAddr=%s\n", szSameDomainAddr);
		}
		
	}
	int nMinInd=0;
	int nNrSrv=0;
	DNSServerList serverList[MAXSRVNR];
	memset(serverList,0,sizeof(serverList));

	nDNSMXres=getDNSMXServer(ToList[i].szToDomains, serverList,nMinInd,nNrSrv);
	LOG("nDNSMXres=%s", nDNSMXres?"true":"false");
	//serverList[nMinInd].nPr, serverList[nMinInd].szName
	/*
	LOG("p_szFrom=%s, p_szTO=%s, p_szSubject=%s,p_szBody=%s, serverList[nMinInd].szName=%s", 
		p_szFrom,  szSameDomainAddr, p_szSubject, p_szBody, serverList[nMinInd].szName);
	*/	
	if (nDNSMXres)
	{
		if (!SMTP_SendMailToServer(p_szFrom, szSameDomainAddr, p_szSubject,p_szBody, serverList[nMinInd].szName))
		{
			retVal=false;	
		}
//		LOG("Send emails\n");
//		LOG("p_szFrom=%s, szSameDomainAddr=%s, p_szSubject=%s, serverList[nMinInd].szName%s", 
//		p_szFrom, szSameDomainAddr, p_szSubject,serverList[nMinInd].szName);
		
	}
	else
	{
		retVal=false;
		LOG("Cannot send to %s domain", ToList[i].szToDomains);
	}
	return retVal;
}

bool CSmtpSupport::SmtpCommand(const char *p_szCommand, int p_nCommandCode)//, const char* szMsg)
{	

	if (!_write(p_szCommand, strlen(p_szCommand)))
	{
		return false;
	}
		
	//memset(sz_ReadBuffer, 0, sizeof(sz_ReadBuffer));
	
	char sz_ReadBuffer[16*1024]={0,};
	if (!_read( sz_ReadBuffer, sizeof(sz_ReadBuffer)-1 ))
	{
		return false;
	}
	
	return IsErrorSmtp(sz_ReadBuffer, p_nCommandCode);
}

bool CSmtpSupport::CreateSmtpCommand( const char *p_szCommand, const char *p_szMsg, char *p_szResultString, size_t p_nMaxSize)
{
	if (!p_szResultString)
	{
 		return false;
	}
	
//	memset(p_szResultString, 0, 1024);	
	
	size_t nMsgLength=strlen(p_szMsg);
	size_t nCommandLength=strlen(p_szCommand);
	
 	if (nCommandLength + 1 + nMsgLength + 2 > p_nMaxSize-1)
		return false;
	
	memcpy(p_szResultString, p_szCommand, nCommandLength);
	
//	LOG("p_szResultString=%s len=%u", p_szResultString, strlen(p_szResultString));
	*(p_szResultString+nCommandLength)=' ';
	
//	LOG("p_szResultString=%s len=%u", p_szResultString, strlen(p_szResultString));
	memcpy(p_szResultString+nCommandLength+1, p_szMsg, nMsgLength);
	
//	LOG("p_szResultString=%s len=%u", p_szResultString, strlen(p_szResultString));

	memcpy(p_szResultString+nCommandLength+1 + nMsgLength, "\r\n", 2);
//	LOG("p_szResultString=%s len=%u", p_szResultString, strlen(p_szResultString));
	
	*(p_szResultString+nCommandLength+1 + nMsgLength+2)=0;
//	LOG("p_szResultString=%s len=%u", p_szResultString, strlen(p_szResultString));
	
	return true;
}


int CSmtpSupport::ArraySize(const char *p_szArray)
{
	int nSize;
	if (p_szArray)
	{
		nSize=strlen(p_szArray)+1;
	}
	else 
		nSize=1;
	return nSize;
}

void CSmtpSupport::ArrayCopy(char p_szDestionation[], const char* p_szSource)
{
	if (p_szSource)
		strcpy(p_szDestionation, p_szSource);
	else
		strcpy(p_szDestionation, "");
}

bool CSmtpSupport::SMTP_SendMailToServer (const char* p_szFrom, const char* p_szTo, const char* p_szSubject, 
					const char* p_szBody, const char* p_szSMTPserver, const char* /*p_szUser*/ /* = "" */, 
					const char* /*p_szPassword*/ /* = ""*/, int p_nSMTPserverPort /* = 25 */)
{	
	if ( !p_szFrom || !p_szTo || !p_szSubject || !p_szBody || !p_szSMTPserver )
	{
		LOG("SMTP_SendMailToServer: some mandatory parameters are null");
		return false;
	}

	if (strchr(p_szTo,'@') == NULL)
	{
		LOG("SMTP_SendMailToServer: invalid To field: %s", p_szTo);
		return false;
	}

	char szTmpServer[ArraySize(p_szSMTPserver)];
	ArrayCopy(szTmpServer, p_szSMTPserver);

	char szTmpTo[ArraySize(p_szTo)];
	ArrayCopy(szTmpTo, p_szTo);
	
	char szTmpFrom[ArraySize(p_szFrom)];
	ArrayCopy(szTmpFrom,p_szFrom);
	
	const char *szTmpSubject="";
	
	if (p_szSubject)
	{	
		szTmpSubject=p_szSubject;
	}

///	char szTmpUser[strlen(p_szUser)+1];
///	char szTmpPassword[strlen(p_szPassword)+1];

	const char *szTmpBody="";
	
	if (p_szBody)
	{	
		szTmpBody=p_szBody;
	}

		bool bIsHtml=false;

	if (strstr(szTmpBody, "<html>"))
		bIsHtml=true;
	
	size_t nBodySize=strlen(szTmpBody);
	
	//LOG("szTmpBody=%s nBodySize=%d\n", szTmpBody, nBodySize);
	char *szPtrFrom=szTmpFrom;
	DeleteBlanks(szPtrFrom);
	char *szPtrTo=szTmpTo;
	DeleteBlanks(szPtrTo);
	
	if (!SmtpOpen(szTmpServer, p_nSMTPserverPort))
	{	
		oSocket.Close();
		return false;
	}	
	
	char szCommand[16*1024];
	szCommand[0] = 0;

	CreateSmtpCommand(STR_EHLO, szTmpServer, szCommand, sizeof(szCommand));
	if (!(SmtpCommand(szCommand, EHLO)))
	{
		oSocket.Close();
		return false;
	}
/*	
	if (p_szUser[0])
	{
		if (!SmtpAuth(szTmpUser, szTmpPassword))
		{
			LOG("SmtpAuth return false\n");
			return false;
		}
	}
*/	
	
	char szAngularString[24*1024]={0,};
	if (strchr(szTmpFrom,'@'))
	{		
		CreateAngleBrackets(szTmpFrom, szAngularString);
		
		szCommand[0]=0;
		CreateSmtpCommand(STR_MAIL_FROM, szAngularString, szCommand, sizeof(szCommand));
		
		if (!SmtpCommand(szCommand, MAIL_FROM))
		{
			oSocket.Close();
			return false;
		}
	}
	char* szTmpTo4Strtok;
	char* szSavePoint;

	for (szTmpTo4Strtok = szTmpTo;;szTmpTo4Strtok = NULL)
	{
		char* szMailAddress = strtok_r(szTmpTo4Strtok, TO_DELIM, &szSavePoint);

		if (szMailAddress == NULL)
		{
			break;
		}		
		
		//LOG("szMailAddress=%s", szMailAddress);
			
		szAngularString[0]=0;
		CreateAngleBrackets(szMailAddress, szAngularString);
		
		//LOG("To: szAngularString=%s", szAngularString);
		
		szCommand[0]=0;
		if (!CreateSmtpCommand(STR_RCPT, szAngularString, szCommand, sizeof(szCommand)))
		{
			continue;
		}
		
		if (!SmtpCommand(szCommand,RCPT))
		{
			oSocket.Close();
			return false;
		}		
	}
	
	
	szCommand[0]=0;
	CreateSmtpCommand(STR_DATA, "", szCommand, sizeof(szCommand));
	if (!SmtpCommand(szCommand,DATA))
	{
		oSocket.Close();
		return false;
	}
	
	int nFullBodyLen = 3 * nBodySize + strlen(szTmpSubject) + strlen(szTmpFrom) + strlen(p_szTo) + 1 + 1024;

	char *szBody = new char [nFullBodyLen];

	szBody[0] = 0;
	
	//sprintf(szBody, "From: %s\r\nTo: %s\r\nSubject: %s\r\n\r\n%s\r\n.\r\n", szTmpFrom, szTmpTo, szTmpSubject, szTmpBody);
	int nPos = sprintf(szBody, "From: %s\r\nTo: %s\r\nSubject: %s\r\n%s\r\n", szTmpFrom, p_szTo, szTmpSubject, 
								bIsHtml ? "Content-Type: text/html;\r\n" : "" );

	
	int nLen = FormatBody(szBody + nPos, szTmpBody, nFullBodyLen - nPos);

	bool bRet = false;
	if (nLen >= 0)
	{
		strcpy(szBody+nPos+nLen, "\r\n.\r\n" );	

		//Print1310(szBody);
		bRet = SmtpData(szBody);
		//LOG("bRet=%d", bRet);
	}

	delete szBody;
	
	szCommand[0]=0;

	CreateSmtpCommand(STR_QUIT, "", szCommand, sizeof(szCommand));
	SmtpCommand(szCommand, QUIT);
	
	oSocket.Close();
	return bRet;	
}

namespace SMTP
{
unsigned char CBase64::m_aTable[64] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 
	'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 
	'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/' };


unsigned char CBase64::m_aRevTable[256] = {
        0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
        0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
        0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x3E,0x80,0x80,0x80,0x3F,
        0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x80,0x80,0x80,0x80,0x80,0x80,
        0x80,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,
        0x0F,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x80,0x80,0x80,0x80,0x80,
        0x80,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,
        0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,0x30,0x31,0x32,0x33,0x80,0x80,0x80,0x80,0x80,
        0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
        0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
        0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
        0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
        0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
        0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
        0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
        0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Class constructor
///////////////////////////////////////////////////////////////////////////////////////////////////

CBase64::CBase64()
{
//	int i;

//	for( i=0; i<64; i++ )
//	{
//		m_aRevTable[ m_aTable[i] ] = i;
//	}
}

//---------------------------------- Public methods ------------------------------------------//

///////////////////////////////////////////////////////////////////////////////////////////////////
// Description: Encode from binary to ascii
// Parameters:
//                const unsigned char * p_pBuffer     - input  - the binary buffer
//                int                   p_pBufferLen  - input  - the binary buffer len
//                unsigned char *       p_pRsp        - output - the response buffer
//                int *                 p_pRspLen     - output - the response buffer len
// Return value: 1 on success, 0 on fail
///////////////////////////////////////////////////////////////////////////////////////////////////
int CBase64::Encode( const unsigned char * p_pBuffer, int p_nBufferLen, unsigned char * p_pRsp, int * p_pRspLen )
{
	unsigned char * pRsp = p_pRsp;

	for( int i=0; i<p_nBufferLen; i += 3 )
	{
		*( pRsp ++ ) =  m_aTable[ p_pBuffer[i] >> 2 ];

		switch( p_nBufferLen-i )
		{
		case 1:
			*( pRsp ++ ) = m_aTable[ (p_pBuffer[i] & 0x03) << 4  ];
			*( pRsp ++ ) = '=';
			*( pRsp ++ ) = '=';
			break;

		case 2:
			*( pRsp ++ ) = m_aTable[ ( (p_pBuffer[i] & 0x03) << 4 )   | (p_pBuffer[i+1] >> 4 ) ];
			*( pRsp ++ ) = m_aTable[ ( (p_pBuffer[i+1] & 0x0F) << 2 ) ];
			*( pRsp ++ ) = '=';
			break;
	
		default:
			*( pRsp ++ ) = m_aTable[ ( (p_pBuffer[i] & 0x03) << 4 )   | (p_pBuffer[i+1] >> 4 ) ];
			*( pRsp ++ ) = m_aTable[ ( (p_pBuffer[i+1] & 0x0F) << 2 ) | (p_pBuffer[i+2] >> 6 ) ];
			*( pRsp ++ ) = m_aTable[ p_pBuffer[i+2] & 0x3F ];
		}
	}

	*p_pRspLen = (pRsp-p_pRsp);
	return 1;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Description: Decode from ascii to bin
// Parameters:
//                const unsigned char * p_pBuffer     - input  - the ascii buffer
//                int                   p_pBufferLen  - input  - the ascii buffer len
//                unsigned char *       p_pRsp        - output - the response buffer
//                int *                 p_pRspLen     - output - the response buffer len
// Return value: 1 on success, 0 on fail
///////////////////////////////////////////////////////////////////////////////////////////////////
int CBase64::Decode( const unsigned char * p_pBuffer, int p_nBufferLen, unsigned char * p_pRsp, int * p_pRspLen )
{
	unsigned char * pRsp = p_pRsp;

	for( int i=0; i<p_nBufferLen; i += 4 )
	{
		for( int j=0; j<4; j++ )
		{
			if( p_pBuffer[i+j] == '=' || m_aRevTable[ p_pBuffer[i+j] ] == 0x80  )
			{
				p_nBufferLen = i+j;
				break;
			}
		}

		switch( p_nBufferLen-i )
		{
		case  1 :
			*( pRsp ++ ) = ( m_aRevTable[ p_pBuffer[i] ] << 2 );
			break;
		case  2 :
			*( pRsp ++ ) = ( m_aRevTable[ p_pBuffer[i] ] << 2 )   | ( m_aRevTable[ p_pBuffer[i+1] ] >> 4 );
			*( pRsp ++ ) = ( m_aRevTable[ p_pBuffer[i+1] ] << 4 );
			break;
		case  3 :
			*( pRsp ++ ) = ( m_aRevTable[ p_pBuffer[i] ] << 2 )   | ( m_aRevTable[ p_pBuffer[i+1] ] >> 4 );
			*( pRsp ++ ) = ( m_aRevTable[ p_pBuffer[i+1] ] << 4 ) | ( m_aRevTable[ p_pBuffer[i+2] ] >> 2 );
			*( pRsp ++ ) = ( m_aRevTable[ p_pBuffer[i+2] ] << 6 );
			break;
		default :
			*( pRsp ++ ) = ( m_aRevTable[ p_pBuffer[i] ] << 2 )   | ( m_aRevTable[ p_pBuffer[i+1] ] >> 4 );
			*( pRsp ++ ) = ( m_aRevTable[ p_pBuffer[i+1] ] << 4 ) | ( m_aRevTable[ p_pBuffer[i+2] ] >> 2 );
			*( pRsp ++ ) = ( m_aRevTable[ p_pBuffer[i+2] ] << 6 ) | ( m_aRevTable[ p_pBuffer[i+3] ] );
		}
	}

	*p_pRspLen = p_nBufferLen*3/4;

	return 1;
}
}
