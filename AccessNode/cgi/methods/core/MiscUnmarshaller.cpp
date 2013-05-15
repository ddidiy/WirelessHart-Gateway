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


#include "MiscUnmarshaller.h"
#include <algorithm>
#include <ctype.h>
#include "MiscImpl.h"

enum {
	NENOFILE,
	NEVALIDATIONERROR
};

CMiscUnmarshaller* CMiscUnmarshaller::m_pInstance =NULL;

int CMiscUnmarshaller::remoteCmd( RPCCommand& cmd )
{
	char* szCmd = NULL;
	char* szCmdRsp = NULL;

	cmd.outObj = json_object_new_object();

	JSON_GET_MANDATORY_STRING_PTR("cmd", szCmd );

	log2flash("CGI: %s remoteCmd [%s]", cmd.io.env.Get("REMOTE_ADDR"), szCmd ) ;
	m_oMisc.execCmd(szCmd, szCmdRsp);
	bool bRet = (bool) szCmdRsp;

	if ( bRet )
		json_object_object_add(cmd.outObj, "result", json_object_new_string( (szCmdRsp) ) );
	else
		json_object_object_add(cmd.outObj, "error", json_object_new_string( "remoteCmd failed"));

	delete szCmdRsp;	/// must deleet the response... cannot use one of the macros
	return bRet ;
}

int CMiscUnmarshaller::getVersion( RPCCommand& cmd )
{
	char *return_value  = m_oMisc.getVersion( );

	cmd.outObj = json_object_new_object();

	if ( !return_value )
	{
		json_object_object_add( cmd.outObj, "error",  json_object_new_string("unknown") );
		delete return_value ;
		return false ;
	}
	json_object_object_add(cmd.outObj, "result", json_object_new_string(return_value) );
	delete return_value ;
	return true ;
}


/**
 * file: optional if script is given
 * script: optional if file is given
 * scripparams: optional
 */
int CMiscUnmarshaller::fileUpload( RPCCommand& cmd )
{
	char szScript[512]={0}, szFile[512]={0} ;
	char *szScriptParams(NULL), *szShRsp(NULL);
	bool rv ;

	cmd.outObj = json_object_new_object();

	JSON_GET_DEFAULT_STRING( "file", szFile, sizeof(szFile), "" );
	JSON_GET_DEFAULT_STRING( "script", szScript, sizeof(szScript),"" );
	JSON_GET_DEFAULT_STRING_PTR( "scriptParams", szScriptParams, NULL );

	// case1: no file and no script
	if ( !*szFile && !*szScript )
	{
		json_object_object_add(cmd.outObj, "error",  json_object_new_string("No parameter given") );
		return false;
	}

	// case2: have file and no script
	if ( *szFile && !*szScript)
	{
		basename(szFile);
		sprintf( szScript, NIVIS_FIRMWARE "%s.sh", szFile );
	}
	// case3: have file and have script
	// case4: no file and have script

	log2flash("CGI: %s %s (file:%s, script:%s)", cmd.io.env.Get("REMOTE_ADDR"), __FUNCTION__, szFile, szScript );
	FLOG(" %s %s (file:%s, script:%s)", cmd.io.env.Get("REMOTE_ADDR"), __FUNCTION__, szFile, szScript );
	rv = m_oMisc.fileUpload(szScript, szScriptParams, szShRsp) ;
	const char* szResult =  szShRsp ? szShRsp : "";

	if ( rv )
	{
		FLOG("result=[%s]",szResult);
		json_object_object_add(cmd.outObj, "result",  json_object_new_string(szResult) );
	}
	else
	{
		FLOG("error=[%s]",szResult);
		json_object_object_add(cmd.outObj, "error",  json_object_new_string(szResult) );
	}

	delete [] szShRsp;
	return rv ;
}


/**
 *
 */

int CMiscUnmarshaller::fileDownload( RPCCommand& cmd )
{
	char szScript[512]={0}, szFile[512]={0};
	char *szScriptParams(NULL), *szShRsp(NULL), *szFileBase(NULL);
	bool rv ;

	cmd.outObj = json_object_new_object();

	JSON_GET_DEFAULT_STRING( "file", szFile, sizeof(szFile), "" );
	JSON_GET_DEFAULT_STRING( "script", szScript, sizeof(szScript), "" );
	JSON_GET_DEFAULT_STRING_PTR( "scriptParams", szScriptParams, NULL );
	// Deal with file
	// case1: no file-> error
	if ( !*szFile )
	{
		json_object_object_add(cmd.outObj, "error",  json_object_new_string("No parameter given") );
		return false ;
	}

	// case2: file with no path -> take it from profile
	szFileBase=strdup(szFile) ;
	basename(szFileBase);

	if ( NULL== strrchr(szFile,'/') ) {
		sprintf( szFile, NIVIS_PROFILE "%s", szFileBase );
	}
	// case3: file with path -> don't touch it

	// Deal with script
	// case1: no script -> take script from firmware
	if ( !*szScript )
	{
		sprintf( szScript, NIVIS_FIRMWARE "%s_get.sh", szFileBase );
	}

	// case2: script with no path -> take it from firmware
	if ( NULL == strrchr(szScript, '/') )
	{
		sprintf( szScript, NIVIS_FIRMWARE "%s", szScript) ;
	}
	// case3: script with path, don't touch it

	log2flash("CGI: %s %s (file:%s, script:%s)", cmd.io.env.Get("REMOTE_ADDR"), __FUNCTION__, szFile, szScript );
	FLOG("%s %s (file:%s, script:%s)", cmd.io.env.Get("REMOTE_ADDR"), __FUNCTION__, szFile, szScript );
	rv = m_oMisc.fileDownload(szScript, szScriptParams, szShRsp);
	const char* szResult =  szShRsp ? szShRsp : "";
	free(szFileBase);

	// if the script exists and failed then report an error
	if ( FileExist(szScript) && !rv )
	{
		json_object_object_add(cmd.outObj, "error",  json_object_new_string(szResult) );
		return false ;
	}

	// read file and write to cmd.io.output
	int len;
	if ( -1 == (len=GetFileLen(szFile)) )
	{
		ERR("No such file[%s]", szFile );
		json_object_object_add(cmd.outObj, "error",  json_object_new_string("No such file.") );
		return false;
	}
	int fd = open(szFile, O_RDONLY );
	if ( fd < 0 )
	{
		json_object_object_add(cmd.outObj, "error",  json_object_new_string("Unable to open file.") );
		return false ;
	}

	json_object_put(cmd.outObj);
	cmd.outObj = NULL ;

	// serve basename of file, not whole path
	fprintf( cmd.io.output, _ATTACHEMENT("%s") CSV_HEADER(), basename(szFile) );
	while ( len > 0 )
	{
		char rbuf[8192];
		int rb = read( fd, rbuf, sizeof(rbuf) ) ;
		if ( rb < 0 )
		{
			ERR("read failed");
			close(fd);
			return false;
		}
		if ( rb == 0 )
			break ;
		len -= rb ;
		fwrite( rbuf, sizeof(char), rb, cmd.io.output );
	}
	close(fd);

	delete [] szShRsp;
	return true ;
}

int CMiscUnmarshaller::getGatewayNetworkInfo( RPCCommand& cmd )
{
	cmd.outObj = json_object_new_object();

	CMiscImpl::GatewayNetworkInfo net;
	bool bRet = m_oMisc.getGatewayNetworkInfo( net );

	JSON_RETURN_ON_ERR( bRet, "getGatewayNetworkInfo" );

	struct json_object* rsltObj = json_object_new_object() ;

	json_object_object_add( rsltObj,"ip", json_object_new_string(net.ip) );
	json_object_object_add( rsltObj,"mask", json_object_new_string(net.mask) );
	json_object_object_add( rsltObj,"defgw", json_object_new_string(net.defgw) );
	json_object_object_add( rsltObj,"mac0", json_object_new_string(net.mac0) );
	json_object_object_add( rsltObj,"mac1", json_object_new_string(net.mac1) );
	json_object_object_add( rsltObj,"dhcpEnabled", json_object_new_boolean(net.bDhcpEnabled) );

	struct json_object* namesrvs = json_object_new_array() ;
	for ( unsigned i=0; net.nameservers[i]!=NULL; ++i )
	{
		json_object_array_add( namesrvs, json_object_new_string( net.nameservers[i] ) ) ;
		free(net.nameservers[i]);
	}
	json_object_object_add( rsltObj,"nameservers", namesrvs);
	json_object_object_add( cmd.outObj, "result", rsltObj );
	return true ;
}

int CMiscUnmarshaller::setGatewayNetworkInfo( RPCCommand& cmd )
{
	CMiscImpl::GatewayNetworkInfo net;

	cmd.outObj = json_object_new_object();

	char szTmp[16] = {0};
	JSON_GET_MANDATORY_STRING( "ip",	net.ip, sizeof(net.ip ) );
	JSON_GET_MANDATORY_STRING( "mask",	net.mask, sizeof(net.mask) );
	JSON_GET_MANDATORY_STRING( "defgw",	net.defgw, sizeof(net.defgw) );
	JSON_GET_DEFAULT_STRING( "updateMAC",	szTmp, sizeof(szTmp), "false" );
	net.bUpdateMAC = ( !strcasecmp(szTmp, "true") || !strcasecmp(szTmp, "yes") ) ? true : false;
	JSON_GET_DEFAULT_STRING( "mac0",	net.mac0, sizeof(net.mac0), "" );
	JSON_GET_DEFAULT_STRING( "mac1",net.mac1, sizeof(net.mac1), "" );
	JSON_GET_DEFAULT_STRING( "dhcpEnabled", szTmp, sizeof(szTmp), "true");
	net.bDhcpEnabled= ( !strcasecmp(szTmp, "true") || !strcasecmp(szTmp, "yes") ) ? true : false;

	struct json_object*namesrvs = json_object_object_get( cmd.inObj,"nameservers");
	int srvcount(0),i ;
	if ( namesrvs )
		srvcount = json_object_array_length(namesrvs) ;
	for ( i=0; i< srvcount && i<(int)sizeof(net.nameservers); ++i)
	{
		struct json_object* a = json_object_array_get_idx(namesrvs,i);
		net.nameservers[i]=json_object_get_string(a) ;
	}
	net.nameservers[i]=NULL;

	int status ;
	bool bRet = m_oMisc.setGatewayNetworkInfo( net, &status );
	if ( NEVALIDATIONERROR == status )
	{
		json_object_object_add( cmd.outObj, "error",  json_object_new_string("ValidationError") );
		return false ;
	}

	JSON_RETURN_BOOL( bRet, "setGatewayNetworkInfo");
}

int CMiscUnmarshaller::getNtpServers( RPCCommand& cmd )
{
	cmd.outObj = json_object_new_object() ;
	std::vector<char*> servers ;
	bool bRet = m_oMisc.getNtpServers(servers);
	if ( !bRet )
	{
		return false ;
		json_object_object_add(cmd.outObj, "error", json_object_new_string("Unable to getNtpServers") );
	}
	json_object* srvObj = json_object_new_array();
	for ( unsigned i=0; i<servers.size();++i)
	{
		json_object_array_add( srvObj, json_object_new_string(servers[i]) );
		free(servers[i]);
	}
	struct json_object* rsltObj = json_object_new_object() ;
	json_object_object_add( rsltObj, "servers", srvObj );
	json_object_object_add( cmd.outObj, "result", rsltObj );
	return true ;
}

int CMiscUnmarshaller::setNtpServers( RPCCommand& cmd )
{
	cmd.outObj = json_object_new_object();
	std::vector<const char*> servers ;
	json_object* varVectorObj =json_object_object_get( cmd.inObj, "servers" );
	if ( ! varVectorObj )
	{
		json_object_object_add( cmd.outObj, "error", json_object_new_string("servers parameter is required") );
		return false ;
	}

	unsigned short vecLen = json_object_array_length(varVectorObj);

	for ( unsigned i=0; i< vecLen; ++i )
	{
		json_object* a = json_object_array_get_idx(varVectorObj, i);
		servers.push_back( json_object_get_string(a) );
	}
	if ( ! m_oMisc.setNtpServers(servers) )
	{
		json_object_object_add( cmd.outObj, "error", json_object_new_string("setNtpServers failed") );
		return false ;
	}
	json_object_object_add( cmd.outObj, "result", json_object_new_boolean(true) );
	return true ;
}

int CMiscUnmarshaller::restartApp( RPCCommand& cmd )
{
	char szAppName[ 128 ], szAppParams[ 256 ];
	cmd.outObj = json_object_new_object();
	JSON_GET_MANDATORY_STRING( "appName", szAppName, sizeof( szAppName ) );
    JSON_GET_DEFAULT_STRING( "appParams", szAppParams, sizeof( szAppParams ), "" );
	bool bRet = m_oMisc.restartApp( szAppName, szAppParams );
	JSON_RETURN_BOOL( bRet, "restartApp");
}

int CMiscUnmarshaller::softwareReset( RPCCommand& cmd )
{
	cmd.outObj = json_object_new_object();
	bool bRet = m_oMisc.softwareReset( );
	JSON_RETURN_BOOL( bRet, "softwareReset");
}

int CMiscUnmarshaller::hardReset( RPCCommand& cmd )
{
	cmd.outObj = json_object_new_object();
	bool bRet = m_oMisc.hardReset( );
	JSON_RETURN_BOOL( bRet, "hardReset");
}

int CMiscUnmarshaller::applyConfigChanges( RPCCommand& cmd )
{
	const char * pModule;
	const char * pSigName;
	cmd.outObj = json_object_new_object();
	JSON_GET_MANDATORY_STRING_PTR( "module", pModule );
	JSON_GET_DEFAULT_STRING_PTR( "signal", pSigName, "HUP" );	//UNDOCUMENTED feature

	int nRet = m_oMisc.applyConfigChanges( pModule, pSigName );

	JSON_RETURN_BOOL( nRet, "misc.applyConfigChanges");
}

int CMiscUnmarshaller::discoverRouters( RPCCommand& cmd )
{
	cmd.outObj = json_object_new_object() ;
	std::vector<CMiscImpl::DiscoveryInfo> routerList ;
	bool bRet = m_oMisc.discoverRouters(routerList);

	JSON_RETURN_ON_ERR(bRet, "discoverRouters");

	json_object* routerArray = json_object_new_array();

	for ( unsigned i=0; i<routerList.size();++i){

		json_object* routerInfoTuple = json_object_new_object();

		json_object_object_add( routerInfoTuple, "anID", json_object_new_string(routerList[i].m_szAnID));
		json_object_object_add( routerInfoTuple, "staticIP", json_object_new_string(routerList[i].m_szStaticIP));
		json_object_object_add( routerInfoTuple, "dynamicIP", json_object_new_string(routerList[i].m_szDynamicIP));

		json_object_array_add(routerArray, routerInfoTuple);
	}

	json_object_object_add( cmd.outObj, "result", routerArray );

	return true ;
}
