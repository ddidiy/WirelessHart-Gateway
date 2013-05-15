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


#include "ConfigImpl.h"
#include "../../../Shared/h.h"

#include "../../../Shared/IniParser.h"
#include <sys/stat.h>
#include <fcntl.h>

bool CConfigImpl::setVariable(const char *p_kszConfigName
                              , const char *p_kszGroup, const char *p_kszVarName
                              , const char *p_kszVarValue, unsigned short  p_usGroupPos/*=0*/
                              , unsigned short  p_usVarPos/* = 0*/)
{
	CIniParser oIniParser;
	if ( !oIniParser.Load( p_kszConfigName, "r+" ))
	{
		ERR("Unable to open file[%s]", p_kszConfigName );
		return false ;
	}

	int group_index(0);
	const char* gf(NULL);
	do {
		gf = oIniParser.FindGroup(p_kszGroup, true) ;
		if ( NULL==gf )
		{
			LOG("Creating nonexisting group [%s]", p_kszGroup );
			break;
		}
		group_index++;
	}while(group_index<=p_usGroupPos);

	int retCode = oIniParser.SetVar( *p_kszGroup ? p_kszGroup : NULL, p_kszVarName, p_kszVarValue, p_usVarPos, true);
	if (retCode == 0)
		return false;

	return true;
}

bool  CConfigImpl::setGroupVariables( const char*  p_kszConfigName
                                      , const char * p_kszGroup
                                      , VarDetailVector * p_kszVarVector
                                      , unsigned short  p_usGroupPos/*=0*/
                                    )
{
	CIniParser oIniParser;
	if ( !oIniParser.Load( p_kszConfigName, "r+" ))
	{
		ERR("Unable to open file:[%s]", p_kszConfigName);
		return false ;
	}
	int group_index = 0;
	while ((oIniParser.FindGroup(p_kszGroup, true)!=NULL) )
	{
		if (group_index==p_usGroupPos)
			break;
		else if (group_index>p_usGroupPos)
			return false;
		group_index++;
	}
	CConfigImpl::VarDetailVector::iterator it = p_kszVarVector->begin() ;
	for ( ; it != p_kszVarVector->end(); ++it)
	{
		oIniParser.SetVar( p_kszGroup, (*it)->varName, (*it)->varValue, (*it)->varPos, true);
	}
	return true;
}

char *CConfigImpl::getVariable(const char *p_kszConfigName, const char *p_kszGroup, const char *p_kszVarName, unsigned short p_ushVarPos )
{
	CIniParser oIniParser;
	if ( !oIniParser.Load( p_kszConfigName, "r" ))
	{
		return NULL;
	}
	static char szVarValue[MAX_LINE_LEN];
	int retCode=oIniParser.GetVar(*p_kszGroup ? p_kszGroup : NULL, p_kszVarName, szVarValue, MAX_LINE_LEN, p_ushVarPos );
	if (retCode == 0)
	{
		return NULL;
	}
	return szVarValue;
}

/// @note
/// @note The caller MUST deallocate: each vector element (std::vector<char*>)[i] and the vector itself std::vector<char*>*
/// @note
std::vector<char*> *CConfigImpl::getGroups(const char *p_kszConfigName)
{
	CIniParser oIniParser;
	if ( !oIniParser.Load( p_kszConfigName, "r" ))
	{
		return NULL;
	}
	char *szGrpName= new char [MAX_LINE_LEN];
	std::vector<char*> *return_value = new std::vector<char*>();
	if ( !oIniParser.FindFirstGroup(szGrpName, MAX_LINE_LEN) )
	{
		// empty config-> return empty vector
		delete [] szGrpName;
		return return_value ;
	}

	return_value->push_back(szGrpName);

	szGrpName= new char [MAX_LINE_LEN];
	while ( oIniParser.FindNextGroup(szGrpName, MAX_LINE_LEN) )
	{
		return_value->push_back(szGrpName);
		szGrpName= new char [MAX_LINE_LEN];
	}
	delete [] szGrpName;
	return return_value;
}

/// @note
/// @note The caller MUST deallocate:
/// @note Each vector element details:
///	@note 		(CConfigImpl::VarDetailVector::VarDetail[i].varName),
/// @note 		(CConfigImpl::VarDetailVector::VarDetail[i].varValue)
/// @note Each vector element:
/// @note 	CConfigImpl::VarDetailVector::VarDetail[i]
/// @note The vector itself:
/// @note CConfigImpl::VarDetailVector*
/// @note
CConfigImpl::VarDetailVector* CConfigImpl::getGroupVariables(const char *p_kszConfigName, const char *p_kszGroup)
{
	CIniParser oIniParser;
	if ( !oIniParser.Load( p_kszConfigName, "r" ))
		return NULL;

	if ( !oIniParser.FindGroup(p_kszGroup) )
		return NULL ;

	VarDetailVector *return_value = new VarDetailVector();
	char *szVarName= new char [MAX_LINE_LEN];
	char *szVarValue= new char [MAX_LINE_LEN];
	if ( !oIniParser.FindFirstVar( szVarName, MAX_LINE_LEN, szVarValue, MAX_LINE_LEN) )
	{
		delete [] szVarName ;
		delete [] szVarValue ;
		return return_value ;	/// this is an EMPTY grpup, correct is to return a vector empty, not NULL
	}
	return_value->push_back(new VarDetail(szVarName, szVarValue));
	szVarName= new char [MAX_LINE_LEN];
	szVarValue= new char [MAX_LINE_LEN];
	while ( oIniParser.FindNextVar( szVarName, MAX_LINE_LEN, szVarValue, MAX_LINE_LEN) )
	{
		return_value->push_back( new VarDetail(szVarName, szVarValue) );
		szVarName= new char [MAX_LINE_LEN];
		szVarValue= new char [MAX_LINE_LEN];
	}
	delete [] szVarName;
	delete [] szVarValue;
	return return_value;
}

/// @note
/// @note The caller MUST deallocate:
/// @note Each vector element details:
/// @note       (CConfigImpl::VarDetailVector::VarDetail[i].varName),
/// @note       (CConfigImpl::VarDetailVector::VarDetail[i].varValue)
/// @note Each vector element:
/// @note   CConfigImpl::VarDetailVector::VarDetail[i]
/// @note The vector itself:
/// @note CConfigImpl::VarDetailVector*
/// @note
CConfigImpl::VarDetailVector* CConfigImpl::getGroupVariablesPart(const char *p_kszConfigName, const char *p_kszGroup, int& p_nMaxVarsNo)
{
    if (p_nMaxVarsNo <= 0)
        return NULL;

    CIniParser oIniParser;
    if ( !oIniParser.Load( p_kszConfigName, "r" ))
        return NULL;

    if ( !oIniParser.FindGroup(p_kszGroup) )
        return NULL ;

    VarDetailVector *return_value = new VarDetailVector();
    char *szVarName= new char [MAX_LINE_LEN];
    char *szVarValue= new char [MAX_LINE_LEN];
    if ( !oIniParser.FindFirstVar( szVarName, MAX_LINE_LEN, szVarValue, MAX_LINE_LEN) )
    {
        delete [] szVarName ;
        delete [] szVarValue ;
        return return_value ;   /// this is an EMPTY grpup, correct is to return a vector empty, not NULL
    }
    return_value->push_back(new VarDetail(szVarName, szVarValue));

    p_nMaxVarsNo--;
    if (p_nMaxVarsNo <= 0)
        return return_value;

    szVarName= new char [MAX_LINE_LEN];
    szVarValue= new char [MAX_LINE_LEN];
    while ( oIniParser.FindNextVar( szVarName, MAX_LINE_LEN, szVarValue, MAX_LINE_LEN) )
    {
        return_value->push_back( new VarDetail(szVarName, szVarValue) );

        p_nMaxVarsNo--;
        if (p_nMaxVarsNo <= 0)
            return return_value;

        szVarName= new char [MAX_LINE_LEN];
        szVarValue= new char [MAX_LINE_LEN];
    }
    delete [] szVarName;
    delete [] szVarValue;
    return return_value;
}

/// @note
/// @note The caller MUST deallocate:
/// @note Each vector element variables lists details:
/// @note 	std::vector<CConfigImpl::ConfigEntry*>::ConfigEntry[i]::VarDetailVector[j].varName
/// @note 	std::vector<CConfigImpl::ConfigEntry*>::ConfigEntry[i]::VarDetailVector[j].varValue
/// @note Each vector element variables lists:
/// @note 	std::vector<CConfigImpl::ConfigEntry*>::ConfigEntry[i]::VarDetailVector[j]
/// @note Each vector element variables:
/// @note 	std::vector<CConfigImpl::ConfigEntry*>::ConfigEntry[i].group
/// @note 	std::vector<CConfigImpl::ConfigEntry*>::ConfigEntry[i].variables
/// @note Each vector element:
/// @note 	std::vector<CConfigImpl::ConfigEntry*>[i]
/// @note The vector itself:
/// @note std::vector<CConfigImpl::ConfigEntry*> *
/// @note
std::vector<CConfigImpl::ConfigEntry*> *CConfigImpl::getConfig(const char *p_kszConfigName)
{
	std::vector<char*> *groups = getGroups(p_kszConfigName);
	if ( !groups )
		return NULL ;

	std::vector<ConfigEntry*> *retVal = new std::vector<ConfigEntry*>();
	for (unsigned g = 0;g < groups->size();++g)
	{
		CConfigImpl::VarDetailVector * groupVars = getGroupVariables(p_kszConfigName, groups->at(g));
		if (!groupVars)
			continue;

		ConfigEntry *ce = new ConfigEntry(groups->at(g), groupVars);
		retVal->push_back(ce);
	}
	delete groups ;
	return retVal;
}

/// @note
/// @note The caller MUST deallocate:
/// @note Each vector element variables lists details:
/// @note   std::vector<CConfigImpl::ConfigEntry*>::ConfigEntry[i]::VarDetailVector[j].varName
/// @note   std::vector<CConfigImpl::ConfigEntry*>::ConfigEntry[i]::VarDetailVector[j].varValue
/// @note Each vector element variables lists:
/// @note   std::vector<CConfigImpl::ConfigEntry*>::ConfigEntry[i]::VarDetailVector[j]
/// @note Each vector element variables:
/// @note   std::vector<CConfigImpl::ConfigEntry*>::ConfigEntry[i].group
/// @note   std::vector<CConfigImpl::ConfigEntry*>::ConfigEntry[i].variables
/// @note Each vector element:
/// @note   std::vector<CConfigImpl::ConfigEntry*>[i]
/// @note The vector itself:
/// @note std::vector<CConfigImpl::ConfigEntry*> *
/// @note
std::vector<CConfigImpl::ConfigEntry*> *CConfigImpl::getConfigPart(const char *p_kszConfigName, int& p_nMaxVarsNo)
{
    if (p_nMaxVarsNo <= 0)
        return NULL;

    std::vector<char*> *groups = getGroups(p_kszConfigName);
    if ( !groups )
        return NULL;

    std::vector<ConfigEntry*> *retVal = new std::vector<ConfigEntry*>();
    for (unsigned g = 0;g < groups->size();++g)
    {
        CConfigImpl::VarDetailVector * groupVars = getGroupVariablesPart(p_kszConfigName, groups->at(g), p_nMaxVarsNo);

        if (!groupVars)
            continue;

        ConfigEntry *ce = new ConfigEntry(groups->at(g), groupVars);
        retVal->push_back(ce);

        if (p_nMaxVarsNo <= 0)
            break;
    }

    delete groups ;
    return retVal;
}

////////////////////////////////////////////////////////////////////////////////
/// @author Marcel Ionescu
/// @brief Update a row into a config file, searching by it's unique key - part of the value extracted by PfnExtractKey.
/// @brief Search row trough multiple variable with the same name
/// @param p_szConfigFile - config file name
/// @param p_szSection - section name
/// @param p_szVarName - variable name
/// @param p_szVarValue - new variable value. Must be formatted as requested by destination config file
/// @param p_pfnExtractKey - key extractor, (from p_szVarValue and from existing
///		config variables values). It is necessary to accomodate small formatting
///		differences between input and file (the amount of whitespaces, etc)
/// @param p_dwKeySize key max possible size, INCLUDING null-terminator
/// @retval false cannot set row
/// @retval true row set ok (updated or inserted)
/// @remarks If row primary key was already in the config file, update row info, otherwise add the row
/// @remarks Advantage of this method over setVariable: in multiple variable environment
/// @remarks 	the caller is unaware of variable position. The variable is found using it's primary key
/// @note
/// @note a much efficient approach is to have setConfigRow in IniParser -
/// @note parse the config file only once as opposed to twice in the current implementation
/// @note the parsing is pretty fast, let's keep it as is for the time being
/// @note
////////////////////////////////////////////////////////////////////////////////
bool CConfigImpl::setConfigRow( const char * p_szConfigFile, const char * p_szSection
		, const char * p_szVarName, const char * p_szVarValue
		, PfnExtractKey p_pfnExtractKey, size_t p_dwKeySize )
{
	char inputKey[ p_dwKeySize ], tmpKey[ p_dwKeySize ];
	int idx = 0;

	if( !(*p_pfnExtractKey)( p_szVarValue, p_dwKeySize, inputKey ) )
		return false;	/// Invalid call

	VarDetailVector * pVariables = getGroupVariables( p_szConfigFile, p_szSection );
	if( !pVariables )
		return setVariable( p_szConfigFile, p_szSection, p_szVarName, p_szVarValue);

	for( VarDetailVector::iterator it = pVariables->begin(); it != pVariables->end(); ++it )
	{
		if( !(*p_pfnExtractKey)( (*it)->varValue, p_dwKeySize, tmpKey ) )/// Search input key in variables space
			continue;	/// skip invalid entries

		if(	( 	!strcasecmp( inputKey, tmpKey ) &&	/// Search primary key
				!strcasecmp( p_szVarName, (*it)->varName ) ) )	/// Only in variables with specified name
		{	LOG("setConfigRow: Found key [%s] at Idx %d ",inputKey, idx );
			break;	/// Found it
		}
		/// Compute the index only for p_szVarName, skip other variable in the same section
		if( !strcasecmp(p_szVarName, (*it)->varName) )
			++idx;
	}
	/// cleanup
	for( VarDetailVector::iterator it = pVariables->begin(); it != pVariables->end(); ++it )
	{
		delete (*it)->varName ;
		delete (*it)->varValue ;
		delete (*it);
	}
	delete pVariables;

	/// idx has now a good value: either is positioned to a device to update, or it points to next available position to add a device
	return setVariable( p_szConfigFile, p_szSection, p_szVarName, p_szVarValue, 0, idx);
}

////////////////////////////////////////////////////////////////////////////////
/// @author Marcel Ionescu
/// @brief Delete a row into a config file, searching by it's unique key - part of the value extracted by PfnExtractKey.
/// @brief Search row trough multiple variable with the same name
/// @param p_szConfigFile - config file name
/// @param p_szSection - section name
/// @param p_szVarName - variable name
/// @param p_szVarValue - variable value to delete. Must contain at least the key
/// @param p_pfnExtractKey - key extractor, (from p_szVarValue and from existing config variables values).
/// @param p_dwKeySize key max possible size, INCLUDING null-terminator
/// @retval false cannot erase row
/// @retval true row erased or non existent
/// @remarks Advantage of this method over delVariable: in multiple variable environment
/// @remarks 	the caller is unaware of variable position. The variable is found using it's primary key
/// @note
/// @note a much efficient approach is to have delConfigRow in IniParser -
/// @note parse the config file only once as opposed to twice in the current implementation
/// @note the parsing is pretty fast, let's keep it as is for the time being
/// @note
////////////////////////////////////////////////////////////////////////////////
bool CConfigImpl::delConfigRow( const char * p_szConfigFile, const char * p_szSection
		, const char * p_szVarName, const char * p_szVarValue
		, PfnExtractKey p_pfnExtractKey, size_t p_dwKeySize )
{
	char inputKey[ p_dwKeySize ], tmpKey[ p_dwKeySize ];
	int idx = 0;
	bool bFound = false;

	if( !(*p_pfnExtractKey)( p_szVarValue, p_dwKeySize, inputKey ) )
		return false;	/// Invalid call

	VarDetailVector * pVariables = getGroupVariables( p_szConfigFile, p_szSection );
	if( !pVariables )
		return false;

	for( VarDetailVector::iterator it = pVariables->begin(); it != pVariables->end(); ++it )
	{
		if( !(*p_pfnExtractKey)( (*it)->varValue, p_dwKeySize, tmpKey ) )/// Search input key in variables space
			continue;	/// skip invalid devices

		if(	( 	!strcasecmp( inputKey, tmpKey ) &&	/// Search primary key
				!strcasecmp( p_szVarName, (*it)->varName ) ) )	/// Only in variables with specified name
		{	LOG("delConfigRow: Found key [%s] at Idx %d ", inputKey, idx );
			bFound = true;
			break;	/// Found it
		}
		/// Compute the index only for p_szVarName, skip other variable in the same section
		if( !strcasecmp(p_szVarName, (*it)->varName) )
			++idx;
	}
	/// cleanup
	for( VarDetailVector::iterator it = pVariables->begin(); it != pVariables->end(); ++it )
	{
		delete (*it)->varName;
		delete (*it)->varValue;
		delete (*it);
	}
	delete pVariables;

	if( !bFound )
	{
		LOG("delConfigRow: Key [%s] not found", inputKey);
		return true;	/// key does not exist after the call, so it's success
	}

	CIniParser oIniParser;
	if ( !oIniParser.Load( p_szConfigFile, "r+" ))
	{
		LOG("delConfigRow: Unable to open file [%s]", p_szConfigFile);
		return false ;
	}
	return oIniParser.DeleteVar( p_szSection, p_szVarName, 0, idx );
}

////////////////////////////////////////////////////////////////////////////////
/// @author Marcel Ionescu
/// @brief Update a section into a config file, searching by it's unique key -
///	@brief a variable within the section (?extracted by PfnExtractKey)
/// @brief Search section trough multiple sections with the same name
/// @param p_szConfigFile - config file name
/// @param p_szSection - section name
/// @param p_szKeyVarName - primary key variable name
/// @param p_szKeyVarValue - primary key variable name value.
/// @param p_szVarName - variable name to set (multiple such variables may appear in the section)
/// @param p_szVarName - variable value to set
/// @param p_pfnExtractKey - key extractor, (from p_szVarValue and from existing
///		config variables values). It is necessary to accomodate small formatting
///		differences between input and file (the amount of whitespaces, etc)
/// @retval false cannot set row/section
/// @retval true section/row set ok (updated or inserted)
/// @remarks If row primary key was already in the config file, update row info, otherwise add the row
/// @remarks Advantage of this method over setVariable: in multiple variable environment
/// @remarks 	the caller is unaware of section&variable position.
///	@remarks 	The section is found using p_szKeyVarName/p_szKeyVarValuep_szConfigFile
///	@remarks 	The variable is found using it's primary key
/// @note
/// @note a much efficient approach is to have setConfigRow in IniParser -
/// @note parse the config file only once as opposed to 3 times in the current implementation
/// @note the parsing is pretty fast, let's keep it as is for the time being
/// @note
////////////////////////////////////////////////////////////////////////////////
bool CConfigImpl::setVarInNthSection( const char * p_szConfigFile, const char * p_szSection
		, const char * p_szKeyVarName, const char * p_szKeyVarValue
		, const char * p_szVarName, const char * p_szVarValue
		, PfnExtractKey p_pfnExtractKey, size_t p_dwKeySize )
{
	char inputKey[ p_dwKeySize + 1 ], tmpKey[ p_dwKeySize + 1 ];
	unsigned nSectionIdx = 0, nVariableIdx = 0;
	bool bSectionFound(false), bVarFound(false);

	if( !(*p_pfnExtractKey)( p_szVarValue, p_dwKeySize, inputKey ) )
		return false;	/// Invalid call

	std::vector<CConfigImpl::ConfigEntry*> * pConfig = getConfig( p_szConfigFile );

	for( std::vector<CConfigImpl::ConfigEntry*>::iterator itC = pConfig->begin(); itC != pConfig->end(); ++itC )
	{	if( !strcasecmp( p_szSection, (*itC)->group ) )	/// Search only sections with the right name
		{	for( VarDetailVector::iterator it = (*itC)->variables->begin(); it != (*itC)->variables->end(); ++it)
			{	if( !strcasecmp(p_szKeyVarName, (*it)->varName)		/// good variable type
				&& !strcasecmp( p_szKeyVarValue, (*it)->varValue) )	/// value match
				{	/// section found
					bSectionFound = true;
					break;
				}
			}
			if( bSectionFound )
			{	for( VarDetailVector::iterator it = (*itC)->variables->begin(); it != (*itC)->variables->end(); ++it)
				{	if( !strcasecmp(p_szVarName, (*it)->varName) )	/// good variable type
					{	if( !(*p_pfnExtractKey)( (*it)->varValue, p_dwKeySize, tmpKey ) )/// Search input key in variables space
							continue;	/// skip invalid entries

						if(!strcasecmp( inputKey, tmpKey) )		/// value match
						{	bVarFound = true;
							break;	/// variable & section found
						}
						++nVariableIdx;
					}
				}
				break;	/// section found, variable not found, but nVariableIdx has good value for insert
			}
			++nSectionIdx;
		}
	}
	for ( unsigned i=0; i<pConfig->size();++i)
	{	for ( unsigned k=0; k< pConfig->at(i)->variables->size(); ++k )
		{	delete pConfig->at(i)->variables->at(k);
		}
		delete pConfig->at(i)->group;
		delete pConfig->at(i)->variables;
		delete pConfig->at(i);
	}
	delete pConfig ;

	LOG("setVarInNthSection: Sect/Var found %d/%d section key [%s=%s] var Key [%s] idx Sect/Var %u/%u"
	   , bSectionFound, bVarFound, p_szKeyVarName, p_szKeyVarValue, inputKey, nSectionIdx, nVariableIdx);

	if( !bSectionFound )
	{	/// Create the section, adding section primary key variable
		setVariable( p_szConfigFile, p_szSection, p_szKeyVarName, p_szKeyVarValue, nSectionIdx );
	}

	/// idx has now a good value: either is positioned to a device to update, or it points to next available position to add a device
	return setVariable( p_szConfigFile, p_szSection, p_szVarName, p_szVarValue, nSectionIdx, nVariableIdx);
}

////////////////////////////////////////////////////////////////////////////////
/// @author Marcel Ionescu
/// @brief Update a section into a config file, searching by it's unique key -
///	@brief a variable within the section (?extracted by PfnExtractKey)
/// @brief Search section trough multiple sections with the same name
/// @param p_szConfigFile - config file name
/// @param p_szSection - section name
/// @param p_szKeyVarName - primary key variable name
/// @param p_szKeyVarValue - primary key variable name value.
/// @param p_szVarName - variable name to set (multiple such variables may appear in the section)
/// @param p_szVarName - variable value to set
/// @param p_pfnExtractKey - key extractor, (from p_szVarValue and from existing
///		config variables values). It is necessary to accomodate small formatting
///		differences between input and file (the amount of whitespaces, etc)
/// @retval false cannot set row/section
/// @retval true section/row set ok (updated or inserted)
/// @remarks If row primary key was already in the config file, update row info, otherwise add the row
/// @remarks Advantage of this method over delVariable: in multiple variable environment
/// @remarks 	the caller is unaware of section&variable position.
///	@remarks 	The section is found using p_szKeyVarName/p_szKeyVarValuep_szConfigFile
///	@remarks 	The variable is found using it's primary key
/// @note
/// @note a much efficient approach is to have setConfigRow in IniParser -
/// @note parse the config file only once as opposed to 3 times in the current implementation
/// @note the parsing is pretty fast, let's keep it as is for the time being
/// @note
////////////////////////////////////////////////////////////////////////////////
bool CConfigImpl::delVarInNthSection( const char * p_szConfigFile, const char * p_szSection
		, const char * p_szKeyVarName, const char * p_szKeyVarValue
		, const char * p_szVarName, const char * p_szVarValue
		, PfnExtractKey p_pfnExtractKey, size_t p_dwKeySize )
{
	char inputKey[ p_dwKeySize + 1 ], tmpKey[ p_dwKeySize + 1 ];
	unsigned nSectionIdx = 0, nVariableIdx = 0;
	bool bSectionFound(false), bVarFound(false);

	if( !(*p_pfnExtractKey)( p_szVarValue, p_dwKeySize, inputKey ) )
		return false;	/// Invalid call

	std::vector<CConfigImpl::ConfigEntry*> * pConfig = getConfig( p_szConfigFile );

	for( std::vector<CConfigImpl::ConfigEntry*>::iterator itC = pConfig->begin(); itC != pConfig->end(); ++itC )
	{	if( !strcasecmp( p_szSection, (*itC)->group ) )	/// Search only sections with the right name
		{	for( VarDetailVector::iterator it = (*itC)->variables->begin(); it != (*itC)->variables->end(); ++it)
			{	if( !strcasecmp(p_szKeyVarName, (*it)->varName)		/// good variable type
				&& !strcasecmp( p_szKeyVarValue, (*it)->varValue) )	/// value match
				{	/// section found
					bSectionFound = true;
					break;
				}
			}
			if( bSectionFound )
			{	for( VarDetailVector::iterator it = (*itC)->variables->begin(); it != (*itC)->variables->end(); ++it)
				{	if( !strcasecmp(p_szVarName, (*it)->varName) )	/// good variable type
					{	if( !(*p_pfnExtractKey)( (*it)->varValue, p_dwKeySize, tmpKey ) )/// Search input key in variables space
							continue;	/// skip invalid entries

						if(!strcasecmp( inputKey, tmpKey) )		/// value match
						{	bVarFound = true;
							break;	/// variable & section found
						}
						++nVariableIdx;
					}
				}
				break;	/// section found and nVariableIdx has good value for insert or update
			}
			++nSectionIdx;
		}
	}
	for ( unsigned i=0; i<pConfig->size();++i)
	{	for ( unsigned k=0; k< pConfig->at(i)->variables->size(); ++k )
		{	delete pConfig->at(i)->variables->at(k);
		}
		delete pConfig->at(i)->group;
		delete pConfig->at(i)->variables;
		delete pConfig->at(i);
	}
	delete pConfig ;

	LOG("delVarInNthSection: Sect/Var found %d/%d sect key [%s=%s] var Key [%s] idx Sect/Var %u/%u"
	   , bSectionFound, bVarFound, p_szKeyVarName, p_szKeyVarValue, inputKey, nSectionIdx, nVariableIdx);

	if( !(bSectionFound && bVarFound) )
	{
		return true;	/// key does not exist after the call, so it's success
	}

	CIniParser oIniParser;
	if ( !oIniParser.Load( p_szConfigFile, "r+" ))
	{
		LOG("delVarInNthSection: Unable to open file [%s]", p_szConfigFile);
		return false ;
	}
	return oIniParser.DeleteVar( p_szSection, p_szVarName, nSectionIdx, nVariableIdx );

}
