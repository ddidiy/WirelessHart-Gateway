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

#include "../../../Shared/AnPaths.h"
#ifndef _DEFAULT_CONFIG
#define _DEFAULT_CONFIG NIVIS_PROFILE "config.ini"
#endif

#include "ConfigUnmarshaller.h"
#include "../../Strop.h"

CConfigUnmarshaller* CConfigUnmarshaller::m_pInstance =NULL;

////////////////////////////////////////////////////////////////////////////////
/// @author Marcel Ionescu
/// @brief Unescape a file name, then if the file name does not begin with . or /
///	@brief		add NIVIS_PROFILE in front of the string
/// @retval pointer to modified string, the same as the input string
/// @remarks Allow user to request config.ini without knowledge of NIVIS_PROFILE location
/// @note The pointer p_szFileName must be modifiable
////////////////////////////////////////////////////////////////////////////////
inline static char* addNivisProfilePrefix( char * p_szFileName, char * p_szFileNameWithPath )
{
	safeStr(p_szFileName);
	return ( '/' == *p_szFileName ) ? p_szFileName : p_szFileNameWithPath;
}


int CConfigUnmarshaller::getVariable( RPCCommand& cmd )
{
	char configFileWithPath[256] = NIVIS_PROFILE, group[256], varName[256] ;
	char * configFile = configFileWithPath + strlen (NIVIS_PROFILE);
	unsigned int ushVarPos;

	cmd.outObj = json_object_new_object();

	JSON_GET_DEFAULT_STRING( "configFile", configFile, sizeof(configFileWithPath)-strlen (NIVIS_PROFILE), _DEFAULT_CONFIG);
	JSON_GET_DEFAULT_STRING( "group", group, sizeof(group), "");
	JSON_GET_MANDATORY_STRING( "varName", varName, sizeof(varName));
	JSON_GET_DEFAULT_INT( "varPos", ushVarPos, 0);

	const char* return_value = m_oConfig.getVariable( addNivisProfilePrefix(configFile, configFileWithPath),
			safeStr(group), safeStr(varName), ushVarPos );

	JSON_RETURN_STRING( return_value, "getVariable");
}


int CConfigUnmarshaller::getGroups( RPCCommand& cmd )
{
	char configFileWithPath[256] = NIVIS_PROFILE;
	char * configFile = configFileWithPath + strlen (NIVIS_PROFILE);
	cmd.outObj = json_object_new_object();

	JSON_GET_DEFAULT_STRING( "configFile", configFile, sizeof(configFileWithPath)-strlen (NIVIS_PROFILE), _DEFAULT_CONFIG);

	std::vector<char*> *return_value = m_oConfig.getGroups( addNivisProfilePrefix( configFile, configFileWithPath ) );

	JSON_RETURN_ON_ERR( return_value, "getGroups" );

	struct json_object* rslt = json_object_new_array();
	for ( unsigned i=0; i< return_value->size(); ++i )
	{
		json_object_array_add( rslt, json_object_new_string( return_value->at(i) ) ) ;
		delete return_value->at(i) ;
	}
	json_object_object_add(cmd.outObj, "result", rslt);
	delete return_value ;
	return true ;
}

int CConfigUnmarshaller::getGroupVariables( RPCCommand& cmd )
{
	char configFileWithPath[256] = NIVIS_PROFILE, group[256];
	char * configFile = configFileWithPath + strlen (NIVIS_PROFILE);
	cmd.outObj = json_object_new_object();

	JSON_GET_DEFAULT_STRING( "configFile", configFile, sizeof(configFileWithPath)-strlen (NIVIS_PROFILE), _DEFAULT_CONFIG);
	JSON_GET_MANDATORY_STRING( "group", group, sizeof(group));

	std::vector<CConfigImpl::VarDetail*> *return_value = m_oConfig.getGroupVariables(
			addNivisProfilePrefix( configFile, configFileWithPath ), safeStr(group) );

	JSON_RETURN_ON_ERR( return_value, "getGroupVariables" );

	struct json_object* rslt = json_object_new_array();
	for ( unsigned i=0; i< return_value->size(); ++i )
	{
		struct json_object* tuple = json_object_new_object();
		json_object_object_add( tuple, return_value->at(i)->varName, json_object_new_string(return_value->at(i)->varValue)) ;
		json_object_array_add( rslt, tuple);

		delete return_value->at(i)->varName ;
		delete return_value->at(i)->varValue ;
		delete return_value->at(i);
	}
	json_object_object_add(cmd.outObj, "result", rslt);
	delete return_value ;
	return true ;
}

int CConfigUnmarshaller::setGroupVariables( RPCCommand& cmd )
{
	char configFileWithPath[256] = NIVIS_PROFILE, group[256] ;
	char * configFile = configFileWithPath + strlen (NIVIS_PROFILE);
	unsigned short ushGroupPos, vecLen ;
	struct json_object* varVectorObj=NULL;

	cmd.outObj = json_object_new_object();

	JSON_GET_DEFAULT_STRING( "configFile", configFile, sizeof(configFileWithPath)-strlen (NIVIS_PROFILE), _DEFAULT_CONFIG);
	JSON_GET_MANDATORY_STRING( "group", group, sizeof(group));
	JSON_GET_DEFAULT_INT( "groupPos", ushGroupPos, 0 );
	varVectorObj =json_object_object_get( cmd.inObj, "varVector" );

	vecLen = json_object_array_length(varVectorObj);
	CConfigImpl::VarDetailVector varVector(vecLen) ;

	for ( unsigned i=0; i< vecLen; ++i )
	{
		struct json_object* a = json_object_array_get_idx(varVectorObj, i);
		char * varName = json_object_get_string( json_object_object_get( a, "varName" ) );
		char * varValue= json_object_get_string( json_object_object_get( a, "varValue" ) );
		int  varPos    = json_object_get_int( json_object_object_get( a, "varPos" ) );

		varVector.push_back( new CConfigImpl::VarDetail(strdup(varName),strdup(varValue),varPos) );
	}

	/// TODO: validate: may we send only variables which are changing?
	/// ASK:  If changing refers to comparing the current variables with the ones found in ConfigFile
	/// Then we might not get a boost out of this, since parsing is done by Config in both cases: Find/Set variable.
	/// The only improvement might be that we don't write so much on the disk.
	bool bRet = m_oConfig.setGroupVariables( addNivisProfilePrefix( configFile, configFileWithPath ),
				 safeStr(group), &varVector, ushGroupPos);

	JSON_RETURN_BOOL( bRet, "setGroupVariables");
}

int CConfigUnmarshaller::getConfig( RPCCommand& cmd )
{
	char configFileWithPath[256] = NIVIS_PROFILE;
	char * configFile = configFileWithPath + strlen (NIVIS_PROFILE);
	cmd.outObj = json_object_new_object();

	JSON_GET_DEFAULT_STRING( "configFile", configFile, sizeof(configFileWithPath)-strlen (NIVIS_PROFILE), _DEFAULT_CONFIG);

	std::vector<CConfigImpl::ConfigEntry*>* return_value = m_oConfig.getConfig( addNivisProfilePrefix( configFile, configFileWithPath ) );

	JSON_RETURN_ON_ERR( return_value, "getConfig" );

	struct json_object* rslt = json_object_new_array();
	for ( unsigned i=0; i<return_value->size();++i)
	{
		struct json_object* grp = json_object_new_object() ;
		json_object_object_add(grp,"group", json_object_new_string(return_value->at(i)->group) );

		struct json_object* arr = json_object_new_array();
		for ( unsigned k=0; k< return_value->at(i)->variables->size(); ++k )
		{
			struct json_object* tuple = json_object_new_object();

			json_object_object_add( tuple, return_value->at(i)->variables->at(k)->varName
				, json_object_new_string(( return_value->at(i)->variables->at(k)->varValue)));
			json_object_array_add( arr, tuple);
			delete [] return_value->at(i)->variables->at(k)->varName ;
			delete [] return_value->at(i)->variables->at(k)->varValue ;
			delete return_value->at(i)->variables->at(k);
		}
		json_object_object_add( grp, "variables", arr );
		json_object_array_add(rslt, grp );
		delete [] return_value->at(i)->group;
		delete return_value->at(i)->variables;
		delete return_value->at(i);
	}
	json_object_object_add(cmd.outObj, "result", rslt );
	delete return_value ;
	return true ;
}

int CConfigUnmarshaller::getConfigPart( RPCCommand& cmd )
{
    char configFileWithPath[256] = NIVIS_PROFILE;
    char * configFile = configFileWithPath + strlen (NIVIS_PROFILE);
    int maxVarsNo = 0;
    cmd.outObj = json_object_new_object();

    JSON_GET_DEFAULT_STRING( "configFile", configFile, sizeof(configFileWithPath)-strlen (NIVIS_PROFILE), _DEFAULT_CONFIG );
    JSON_GET_DEFAULT_INT( "maxVarsNo", maxVarsNo, 0 );

    std::vector<CConfigImpl::ConfigEntry*>* return_value = m_oConfig.getConfigPart( addNivisProfilePrefix( configFile, configFileWithPath ), maxVarsNo );

    JSON_RETURN_ON_ERR( return_value, "getConfig" );

    struct json_object* rslt = json_object_new_array();
    for ( unsigned i=0; i<return_value->size();++i)
    {
        struct json_object* grp = json_object_new_object() ;
        json_object_object_add(grp,"group", json_object_new_string(return_value->at(i)->group) );

        struct json_object* arr = json_object_new_array();
        for ( unsigned k=0; k< return_value->at(i)->variables->size(); ++k )
        {
            struct json_object* tuple = json_object_new_object();

            json_object_object_add( tuple, return_value->at(i)->variables->at(k)->varName
                , json_object_new_string(( return_value->at(i)->variables->at(k)->varValue)));
            json_object_array_add( arr, tuple);
            delete [] return_value->at(i)->variables->at(k)->varName ;
            delete [] return_value->at(i)->variables->at(k)->varValue ;
            delete return_value->at(i)->variables->at(k);
        }
        json_object_object_add( grp, "variables", arr );
        json_object_array_add(rslt, grp );
        delete [] return_value->at(i)->group;
        delete return_value->at(i)->variables;
        delete return_value->at(i);
    }
    json_object_object_add(cmd.outObj, "result", rslt );
    delete return_value ;
    return true ;
}

int CConfigUnmarshaller::setVariable( RPCCommand& cmd )
{
	char configFileWithPath[256] = NIVIS_PROFILE, group[256], varName[256], varValue[256] ;
	char * configFile = configFileWithPath + strlen (NIVIS_PROFILE);
	unsigned int ushVarPos, ushGroupPos;

	cmd.outObj = json_object_new_object();

	JSON_GET_DEFAULT_STRING( "configFile", configFile, sizeof(configFileWithPath)-strlen (NIVIS_PROFILE), _DEFAULT_CONFIG);
	JSON_GET_DEFAULT_STRING( "group", group, sizeof(group), "");
	JSON_GET_DEFAULT_INT( "groupPos", ushGroupPos, 0);
	JSON_GET_MANDATORY_STRING( "varName", varName, sizeof(varName));
	JSON_GET_DEFAULT_INT( "varPos", ushVarPos, 0);
	JSON_GET_MANDATORY_STRING( "varValue", varValue, sizeof(varValue));

	bool bRet = m_oConfig.setVariable( addNivisProfilePrefix( configFile, configFileWithPath ),
			safeStr(group), safeStr(varName), safeStr(varValue), ushGroupPos, ushVarPos );

	JSON_RETURN_BOOL( bRet, "setVariable");
}
