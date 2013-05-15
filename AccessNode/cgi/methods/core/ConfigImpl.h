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

#ifndef _CONFIG_IMPL_H_
#define _CONFIG_IMPL_H_

#include <vector>
#include <cstddef>

/// @brief Extract primary key from variable value, config-file dependent
/// @retval true if key was extracted into p_pDst
/// @retval true false if cannot extract value (p_pDst is not modified in this case)
typedef bool (* PfnExtractKey) ( const char * p_pSrc, size_t p_dwSize, char * p_pDst);

//////////////////////////////////////////////////////////////////////
/// @class CConfigImpl
//////////////////////////////////////////////////////////////////////
class CConfigImpl
{
public:
	//////////////////////////////////////////////////////////////
	/// @struct VarDetail
	//////////////////////////////////////////////////////////////
	struct VarDetail
	{
		char* varName;
		char* varValue;
		int   varPos ;
		VarDetail(char*n,char*v,int p=0):varName(n),varValue(v),varPos(p) {}
	};

	typedef std::vector<VarDetail*> VarDetailVector;

	struct ConfigEntry
	{
		char* group ;
		VarDetailVector* variables ;
		ConfigEntry(char*g, VarDetailVector*v): group(g), variables(v) {}
	} ;
//////////////////////////////////////////////////////////////////////
	bool setVariable(const char *p_kszConfigName
	                 , const char *p_kszGroup, const char *p_kszVarName
	                 , const char *p_kszVarValue, unsigned short  p_usGroupPos = 0
	                                 , unsigned short  p_usVarPos = 0
	                ) ;

	bool  setGroupVariables( const char*  p_kszConfigName
	                         , const char * p_kszGroup
	                         , VarDetailVector * p_kszVarVector
	                         , unsigned short  p_usGroupPos = 0
	                       ) ;

	char *getVariable(const char *p_kszConfigName
	                  , const char *p_kszGroup, const char *p_kszVarName
	                  , unsigned short p_ushVarPos
	                 ) ;

	/// @note The caller MUST deallocate: each vector element (*std::vector<char*>)[i] and the vector itself *std::vector<char*>*
	std::vector<char*>* getGroups(const char *p_kszConfigName);

	/// @note The caller MUST deallocate:
	/// @note Each vector element details:
	///	@note 		(CConfigImpl::VarDetailVector::VarDetail[i].varName),
	/// @note 		(CConfigImpl::VarDetailVector::VarDetail[i].varValue)
	/// @note Each vector element:
	/// @note 	CConfigImpl::VarDetailVector::VarDetail[i]
	/// @note The vector itself:
	/// @note CConfigImpl::VarDetailVector*
	VarDetailVector*    getGroupVariables(const char *p_kszConfigName,
	                                      const char *p_kszGroup );

private:
    VarDetailVector*    getGroupVariablesPart(const char *p_kszConfigName,
                                              const char *p_kszGroup,
                                              int& p_nMaxVarsNo);
public:
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
	std::vector<ConfigEntry*>* getConfig(const char *p_kszConfigName);

	std::vector<ConfigEntry*>* getConfigPart(const char *p_kszConfigName, int& p_nMaxVarsNo);

	bool setConfigRow( const char * p_szConfigFile, const char * p_szSection
			, const char * p_szVarName, const char * p_szVarValue
			, PfnExtractKey p_pfnExtractKey, size_t p_dwKeySize );

	bool delConfigRow( const char * p_szConfigFile, const char * p_szSection
			, const char * p_szVarName, const char * p_szVarValue
			, PfnExtractKey p_pfnExtractKey, size_t p_dwKeySize );

	bool setVarInNthSection( const char * p_szConfigFile, const char * p_szSection
		, const char * p_szKeyVarName, const char * p_szKeyVarValue
		, const char * p_szVarName, const char * p_szVarValue
		, PfnExtractKey p_pfnExtractKey, size_t p_dwKeySize );

	bool delVarInNthSection( const char * p_szConfigFile, const char * p_szSection
		, const char * p_szKeyVarName, const char * p_szKeyVarValue
		, const char * p_szVarName, const char * p_szVarValue
		, PfnExtractKey p_pfnExtractKey, size_t p_dwKeySize );

};
#endif	/* _CONFIG_IMPL_H_ */
