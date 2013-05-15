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
/// @file	jsonrpc.h
/// @author	Marius Negreanu
/// @date	2008.09.18 08.49.13
/// @brief	JsonRPC Interface.
/// $Id: JsonRPC.h,v 1.31.18.1 2013/05/15 19:19:17 owlman Exp $
//////////////////////////////////////////////////////////////////////////////

#ifndef _JSON_RPC_H_
#define _JSON_RPC_H_


#include <stdio.h>
#include "json/json.h"

#include "IO.h"

#include "Marshalling.h"

#define JSON_HEADER(body)  "Content-Type: application/json\r\n\r\n"body
#define CSV_HEADER(body)   "Content-Type: text/csv\r\n\r\n"body
#define _ATTACHEMENT(file) "Content-disposition: attachment; filename="file"\r\n"
#define CALL_TOP(message)  "Content-Type: text/html\r\n\r\n<script type=\"text/javascript\">window.top.operationDoneListener("message");</script>\r\n"


/// @todo See where this might be fitted
int	InvalidParameters( struct json_object*& outObj, const char* p_pParamName = 0 );


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
class RPCCommand {
public:
	struct json_object* inObj ;
	struct json_object* outObj ;
	IO& io ;
	RPCCommand(IO&p_io)
		: io(p_io)
	{}
} ;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
class CJsonRPC {
public:
	explicit CJsonRPC(IO&p_io) ;
	int	HandleCall( char* p_szLine, bool p_bCallTop=false) ;
	void	JsonOk( struct json_object *rsp_obj, bool p_bCallTop=false) ;
	void	JsonError( int code, bool p_bCallTop=false ) ;
	void	JsonError( const char* msg, bool p_bCallTop=false ) ;
	void	JsonError( struct json_object*&, bool p_bCallTop=false ) ;

	IO& io ;

protected:
	bool	allowUser( const struct JsonRPCMethodOption* p_met, int p_iUid ) ;
	bool	allowAny( const struct JsonRPCMethodOption* p_met, int p_iUid ) ;
	bool	extractMethodAndParams(struct json_object* req, struct json_object*&met, struct json_object*&prm, bool p_bCallTop) ;
	void	logModifiers( int met_id, char * str ) ;
} ;


#endif
