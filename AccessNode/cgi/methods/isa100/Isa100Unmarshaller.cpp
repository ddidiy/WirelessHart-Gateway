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

#include "Isa100Unmarshaller.h"

CIsa100Unmarshaller* CIsa100Unmarshaller::m_pInstance =NULL;

int CIsa100Unmarshaller::getSystemStatus( RPCCommand& cmd )
{
	pthread_mutex_lock(&m_oMutex);	/// LOCK
	
	CIsa100Impl::SystemStatus *retVal = m_oIsa100.getSystemStatus() ;

	if ( !retVal )
	{
		pthread_mutex_unlock(&m_oMutex);	/// UNLOCK

		json_object_object_add(cmd.outObj, "error", json_object_new_string( "isa100.getSystemStatus" " failed"));
		return (int)(retVal) ;
	}

	JSON_RETURN_ON_ERR( retVal, "isa100.getSystemStatus");

	cmd.outObj = json_object_new_object();
	struct json_object* rsltObj= json_object_new_object();
	struct json_object* procArray = json_object_new_array() ;
	for ( unsigned i=0; i< retVal->processes.size(); ++i )
	{
		struct json_object* procObj = json_object_new_object();
		json_object_object_add( procObj,"name", 		json_object_new_string(retVal->processes[i].m_szDisplayName) );
		json_object_object_add( procObj,"status", 		json_object_new_string(retVal->processes[i].m_szStatus) );
		json_object_object_add( procObj,"memKb", 		json_object_new_int(   retVal->processes[i].m_ushMemKb) );
		json_object_object_add( procObj,"processorUsage", json_object_new_string( retVal->processes[i].m_szProcessorUsage ) );
		json_object_array_add( procArray, procObj);
	}
	json_object_object_add( rsltObj, "processes", procArray );

	json_object_object_add( rsltObj,"sysMemTotalKb", 	json_object_new_int(retVal->sysMemTotalKb) );
	json_object_object_add( rsltObj,"sysMemFreeKb", 	json_object_new_int(retVal->sysMemFreeKb) );
	json_object_object_add( rsltObj,"sysFlashTotalKb", 	json_object_new_int(retVal->sysFlashTotalKb) );
	json_object_object_add( rsltObj,"sysFlashFreeKb", 	json_object_new_int(retVal->sysFlashFreeKb) );
	json_object_object_add( rsltObj,"load", 			json_object_new_string(retVal->load) );
	json_object_object_add( cmd.outObj, "result", rsltObj );

	pthread_mutex_unlock(&m_oMutex);	/// UNLOCK

	return true;
}


