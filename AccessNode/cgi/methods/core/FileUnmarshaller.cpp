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


#include "FileUnmarshaller.h"
#include "../../Strop.h"

CFileUnmarshaller* CFileUnmarshaller::m_pInstance =NULL;

inline static char* addNivisProfilePrefix( char * p_szFileName, char * p_szFileNameWithPath )
{
	safeStr(p_szFileName);
	return ( '/' == *p_szFileName ) ? p_szFileName : p_szFileNameWithPath;
}
int CFileUnmarshaller::create( RPCCommand& cmd )
{
	char absPath[256] = NIVIS_PROFILE ;
	char * file = absPath + strlen (NIVIS_PROFILE);

	cmd.outObj = json_object_new_object();

	JSON_GET_MANDATORY_STRING( "file", file,sizeof(absPath)-sizeof(NIVIS_PROFILE)  );

	bool return_value = m_oFile.bind( addNivisProfilePrefix(file, absPath)).create();

	JSON_RETURN_BOOL( return_value, "create");
}

int CFileUnmarshaller::exists( RPCCommand& cmd )
{
	char absPath[256] = NIVIS_PROFILE ;
	char * file = absPath + strlen (NIVIS_PROFILE);

	cmd.outObj = json_object_new_object();

	char * tmp = json_object_get_string( json_object_object_get(cmd.inObj,"file") );
	if ( !tmp ) return InvalidParameters( cmd.outObj, "file" );
	memmove(file, tmp, strlen(tmp) );

	glob_t globbuf ;
	m_oFile.bind( addNivisProfilePrefix(file,absPath)).exists( &globbuf );

	// marshall
	struct json_object* rslt = json_object_new_array();
	for ( size_t i=0; i< globbuf.gl_pathc; ++i )
	{
		json_object_array_add( rslt, json_object_new_string( globbuf.gl_pathv[i] ) ) ;
	}
	globfree(&globbuf);
	json_object_object_add(cmd.outObj, "result", rslt);
	return true ;
}

int CFileUnmarshaller::remove( RPCCommand& cmd )
{
	char absPath[256] = NIVIS_PROFILE ;
	char * file = absPath + strlen (NIVIS_PROFILE);

	cmd.outObj = json_object_new_object();

	JSON_GET_MANDATORY_STRING( "file", file,sizeof(absPath)-sizeof(NIVIS_PROFILE)  );

	bool return_value = m_oFile.bind( addNivisProfilePrefix(file,absPath) ).remove();

	JSON_RETURN_BOOL( return_value, "remove");
}
