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

//////////////////////////////////////////////////////////////////////////////
/// @file	jsonrpc.cpp
/// @author	Marius Negreanu
/// @date	2008.10.17 08.49.13
/// @brief	JsonRPC Interface.
//////////////////////////////////////////////////////////////////////////////


#include "JsonRPC.h"
#include "methods/MethodsHash.h"
#include "../Shared/MicroSec.h"
#include "../Shared/h.h"


extern char* lastError ;
static int compuid(const void *p_pKey, const void *p_pArrEl ) ;


CJsonRPC::CJsonRPC( IO& p_io )
	: io(p_io)
{
}

struct errorCodes {
	int code ;
	const char* message ;
} ;

struct errorCodes ErrTbl[]={
	{-32700,  "-32700:parse error: not well formed"},
	{-32701,  "-32701:parse error: unsupported encoding"},
	{-32702,  "-32702:parse error: invalid character for encoding"},
	{-32600,  "-32600:server error: invalid json-rpc. not conforming to spec."},
	{-32601,  "-32601:server error: requested method not found"},
	{-32602,  "-32602:server error: invalid method parameters"},
	{-32603,  "-32603:server error: internal json-rpc error"},
	{-32500,  "-32500:application error"},
	{-32501,  "-32501:application error: login first"},
	{-32502,  "-32502:application error: permission denied"},
	{-32503,  "-32503:application error: method call failed"},
	{-32400,  "-32400:system error"},
	{-32300,  "-32300:transport error"},
	{0,NULL}
} ;

const char* getErrMessage( int errCode )
{
	int i=0;
	while( ErrTbl[i].code )
	{
		if ( errCode == ErrTbl[i].code )
			return ErrTbl[i].message ;
		++ i;
	}
	return "" ;
}

//////////////////////////////////////////////////////////////////////////////
/// @todo factorize(eat) this
/// @param p_szLine
/// @param p_bCallTop
/// @param sh
/// @return
//////////////////////////////////////////////////////////////////////////////
int CJsonRPC::HandleCall( char* p_szLine, bool p_bCallTop/*=false*/)
{
	CMicroSec uSec;	// start measuring time
	RPCCommand cmd(io) ;

	LOG("CALL [%s]", p_szLine);
	// Validate input
	struct json_object *req_obj = json_tokener_parse( p_szLine );

	if ( is_error(req_obj) )
	{
		JsonError(-32700) ;
		LOG("%s", p_szLine );
		return false ;
	}

	struct json_object *met_obj;
	if ( !  extractMethodAndParams(req_obj, met_obj, cmd.inObj, p_bCallTop) )
		return false ;

	char* met_str = json_object_to_json_string(met_obj) ;
	if ( !met_str )
	{
		json_object_put(req_obj);
		JsonError( -32600 , p_bCallTop);
		return false ;
	}
	//

	//
	const struct JsonRPCMethodOption *met=JsonRPCMethods::HasMethod(met_str, strlen(met_str)) ;
	if ( !met )
	{
		json_object_put(req_obj);
		JsonError( -32601, p_bCallTop);
		return false ;
	}

	if ( ! io.session.Start() )
	{
		json_object_put(req_obj);
		JsonError(-32603, p_bCallTop);
		return false ;
	}
	// If the method needs login, and there is no session,
	// then ask the user to login.
	int uid(0) ;
	if ( ! io.session.IsLoggedIn(uid) )
	{
		if ( met->id != USER_LOGIN &&   met->id != USER_LOGOUT && !allowAny(met,uid) )
		{
			json_object_put(req_obj);
			JsonError(-32501,p_bCallTop);
			return false ;
		}
	}
	else if ( !allowUser(met, uid) )
	{
		json_object_put(req_obj);
		JsonError( -32502,p_bCallTop);
		return false ;
	}

	logModifiers( met->id, p_szLine );

	// Call the RPC method.
	MethodUnmarshaller* obj = met->GetInstance() ;
	int rv = (obj->*(met->unmarshall))( cmd );

	// deal with the method response
	if ( !cmd.outObj )
	{	// the upper layer has stream-written the result already.
		if ( rv )
		{
			LOG("PERF[%s] %.3f sec", met_str, uSec.GetElapsedSec() );
			json_object_put(req_obj);
			io.session.Release() ;
			return true;
		}
		else if ( met->id == USER_LOGIN ) // if user.login failed, destroy the session
			io.session.Destroy() ;
		else
		{
			JsonError(-32503, /*p_bCallTop*/ true) ;
			LOG("PERF[%s] %.3f sec", met_str, uSec.GetElapsedSec() );
			json_object_put(req_obj);
			io.session.Release() ;
			return false ;
		}
#if 0
		if( !rv && ( met->id == USER_LOGIN ) )
		{
			io.session.Destroy() ;
		}
		else
		{
			if(!rv)
			{
				JsonError("Method call failed.", /*p_bCallTop*/ true) ;
			}
			LOG("PERF[%s] %.3f sec", met_str, uSec.GetElapsedSec() );
			json_object_put(req_obj);
			io.session.Release() ;
			return (rv) ;
		}
#endif
	}
	if ( !rv )
	{
		JsonError( cmd.outObj, p_bCallTop);
		LOG("PERF[%s] %.3f sec", met_str, uSec.GetElapsedSec() );
		json_object_put(req_obj);
		json_object_put(cmd.outObj);
		io.session.Release() ;
		return false ;
	}

	JsonOk( cmd.outObj, p_bCallTop) ;
	LOG("PERF[%s] %.3f sec", met_str, uSec.GetElapsedSec() );
	json_object_put(req_obj);
	json_object_put(cmd.outObj);
	io.session.Release() ;
	return true ;
}


//////////////////////////////////////////////////////////////////////////////
/// @brief Print out the response json object.
//////////////////////////////////////////////////////////////////////////////
void CJsonRPC::JsonOk( struct json_object *rsp_obj, bool p_bCallTop/*=false*/)
{
	json_object_object_add( rsp_obj, "id",  NULL);
	json_object_object_add( rsp_obj, "error",  NULL );

	if ( p_bCallTop )
		fprintf( io.output, CALL_TOP("%s"), json_object_to_json_string(rsp_obj));
	else
		fprintf( io.output, JSON_HEADER("%s"), json_object_to_json_string(rsp_obj));

	if( ferror(io.output) || feof(io.output) )
	{
		ERR("OutputStream:%s", strerror(errno) );
	}
}


//////////////////////////////////////////////////////////////////////////////
/// @brief
//////////////////////////////////////////////////////////////////////////////
void CJsonRPC::JsonError( struct json_object*& rsp_obj, bool p_bCallTop/*=false*/ )
{
	json_object_object_add( rsp_obj, "id",  NULL);
	json_object_object_add( rsp_obj, "result", NULL );

	if ( p_bCallTop )
	{
		fprintf( io.output, CALL_TOP("%s"), json_object_to_json_string(rsp_obj));
		LOG(CALL_TOP("%s"), json_object_to_json_string(rsp_obj) );
	}
	else
	{
		fprintf( io.output, JSON_HEADER("%s"), json_object_to_json_string(rsp_obj));
		LOG(JSON_HEADER("%s"), json_object_to_json_string(rsp_obj));
	}
}


//////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////
void CJsonRPC::JsonError( int errCode, bool p_bCallTop/*=false*/ )
{
	struct json_object *rsp_obj = json_object_new_object () ;
	const char *err = getErrMessage(errCode) ;
	json_object_object_add(rsp_obj, "error", json_object_new_string(err) ) ;

	JsonError(rsp_obj,p_bCallTop);
	json_object_put(rsp_obj);
}
void CJsonRPC::JsonError( const char* err, bool p_bCallTop/*=false*/ )
{
	struct json_object *rsp_obj = json_object_new_object () ;
	json_object_object_add(rsp_obj, "error", json_object_new_string(err) ) ;

	JsonError(rsp_obj,p_bCallTop);
	json_object_put(rsp_obj);
	log2flash("JsonError: %s", err);
}


//////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////
int InvalidParameters(struct json_object*& outObj, const char* p_pParamName)
{
	if ( !outObj ) outObj = json_object_new_object();
	char buf[128];

	sprintf(buf, "Invalid method parameters : %s ", p_pParamName ? p_pParamName : "");

	LOG(buf);

	json_object_object_add(outObj, "result", NULL );
	json_object_object_add(outObj, "error",  json_object_new_string(buf) );
	json_object_object_add(outObj, "id",  NULL );
	return false;
}


//////////////////////////////////////////////////////////////////////////////
/// @todo this might be moved in an ACL specific secion
//////////////////////////////////////////////////////////////////////////////
static int compuid(const void *p_pKey, const void *p_pArrEl )
{
	int uid1= *(int*)p_pKey;
	int uid2= *(int*)p_pArrEl;
	return (uid1 - uid2) ;
}

//////////////////////////////////////////////////////////////////////////////
/// @param p_met
/// @param p_iUid
/// @return
/// @todo this might be moved in an ACL specific secion
//////////////////////////////////////////////////////////////////////////////
bool CJsonRPC::allowAny( const struct JsonRPCMethodOption* p_met, int p_iUid )
{
	int max_user_id = MAX_USER_ID ;
	void *res = bsearch(&max_user_id, p_met->ace, p_met->nbAce, sizeof(int), compuid);
	if ( res ) return true ;
	return false ;
}


//////////////////////////////////////////////////////////////////////////////
/// @param p_met
/// @param p_iUid
/// @return
/// @todo this might be moved in an ACL specific secion
//////////////////////////////////////////////////////////////////////////////
bool CJsonRPC::allowUser( const struct JsonRPCMethodOption* p_met, int p_iUid )
{
	void *res = bsearch(&p_iUid, p_met->ace, p_met->nbAce, sizeof(int), compuid);
	if ( res ) return true ;

	ERR("UID:%d not found in %s:ACE", p_iUid, p_met->MethodName );
	return false ;
}

void CJsonRPC::logModifiers( int met_id, char * str )
{
	switch ( met_id )
	{
#if defined(RELEASE_ISA)
	case ISA100_SM_SETDEVICE:
#endif
	case MISC_HARDRESET:
	case MISC_SOFTWARERESET:
	case MISC_RESTARTAPP:
	case CONFIG_SETVARIABLE:
		FLOG("CGI: %s", str);
		log2flash( "CGI: %s", str);
		break ;
	default:
		break;
	}
}


//////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////
bool CJsonRPC::extractMethodAndParams(struct json_object* req, struct json_object*&met, struct json_object*&prm, bool p_bCallTop)
{
	// Search the method.
	met = json_object_object_get(req,"method");
	prm = json_object_object_get(req,"params");
	if ( !met || !prm )
	{
		JsonError("JsonRPC Error: Invalid parameters", p_bCallTop) ;
		return false ;
	}
	return true ;
}
