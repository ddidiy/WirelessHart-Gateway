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

/****************************************************
* Name:        IniParser.h
* Author:      Marius Chile
* Date:        06.10.2003
* Description: Definition of ini parser
* Changes:
* Revisions:
* @addtogroup libshared
* @{
****************************************************/

//lint -library

#if !defined(_INIPARSER_H_)
#define _INIPARSER_H_
#include <iostream>
#include <list>
#include <stdio.h>
#ifdef WIN32
#include "structures.h"
#else
#include "Common.h"
#endif

// the maximum line length
#define MAX_LINE_LEN 256
#ifdef TEST_FIPS
    #undef MAX_LINE_LEN
    #define MAX_LINE_LEN 2048
#endif

// group name delimiters
#define GROUP_NAME_BEGIN_CHAR '['
#define GROUP_NAME_END_CHAR ']'

// subgroup name delimiters
#define SUBGROUP_NAME_BEGIN_CHAR '<'
#define SUBGROUP_NAME_END_CHAR '>'

///////////////////////////////////////////////////////////////////////////////////
// Name:        CIniParser
// Author:      Marius Chile
// Description: Reads and sets groups, subgroups and variables from an ini file
///////////////////////////////////////////////////////////////////////////////////
//using namespace std;
class CIniParser
{
protected:
    char*   m_szLine;					// the current line read by the parser
    char*   m_szCrtGroup;				// the current group read by the parser
	char*   m_szCrtSubgroup;			// the current subgroup read by the parser
	long	m_lValueFilePos;			// the file position of the last found var. value
	long	m_lGroupStart;				// the file position of the group variables start
	long	m_lSubgroupStart;			// the file position of the subgroup variables start

	// variable used to mark the last variable found
	// when enumerating them using FindFirst and FindNext
	long	m_lEnumVarStart;

	FILE*	m_fpIniFile;

private:
	const char * findVariable(const char * p_szGroup, const char * p_szVarName, int & p_nPos,
							  bool p_bLogErrors = true, const char * p_szSubgroup = "");
	const char * findVariableRaw(const char * p_szGroup, const char * p_szVarName, int & p_nPos,
							  bool p_bLogErrors = true, const char * p_szSubgroup = "");
	int createVariable(const char * p_szGroup, const char * p_szAttrib, bool p_bLogErrors = true,
					   const char * p_szSubgroup = "");

	bool moveToGroupEnd(long* p_lFilePos, bool p_bToNextEntity = false);
protected:
	const char * findVariable(const char * p_szVarName, char * p_szName = (char*)NULL,
							  int p_nMaxNameSize = 0);
	const char * findVariableRaw(const char * p_szVarName, char * p_szName = (char*)NULL,
							  int p_nMaxNameSize = 0);

	virtual int replaceValue(const char * p_szOldValue, const char * p_pvNewVal,
					 unsigned int p_nNewValLen, bool p_bLogErrors = true);

public:
	CIniParser();
	virtual ~CIniParser();

	typedef std::list<int> valuesList;

	bool DeleteGroup(const char * p_szGroup, bool p_bLogErrors = true);
	int DeleteVar(const char * p_szGroup, const char * p_szVarName
			, int p_nGroupPos=0, int p_nPos = 0, bool p_bLogErrors = true, const char * p_szSubgroup = "");

	int Load(const char * p_szFileName, const char* p_szOpenMode = "rb", bool p_bLock = false);
	void Release();

	const char * FindGroup(const char * p_szGroup, bool p_bGetNext = false,
						   bool p_bLogErrors = true, bool p_bUnknownName = false);
	bool FindFirstGroup(char * p_szGroup, int p_nMaxGroupSize, bool p_bLogErrors = true);
	bool FindNextGroup(char * p_szGroup, int p_nMaxGroupSize, bool p_bLogErrors = true);
	int CreateGroup(const char * p_szGroup, bool p_bLogErrors = true);



	virtual const char * FindSubgroup(const char * p_szSubgroup, bool p_bGetNext = false,
							  bool p_bLogErrors = true, bool p_bUnknownName = false);
	bool FindFirstSubgroup(char * p_szSubgroup, int p_nMaxSubgroupSize, bool p_bLogErrors = true);
	bool FindNextSubgroup(char * p_szSubgroup, int p_nMaxSubgroupSize, bool p_bLogErrors = true);
	int CreateSubgroup(const char * p_szGroup, const char * p_szSubgroup,
					   bool p_bLogErrors = true);



	bool FindFirstVar(char * p_szVarName, int p_nMaxVarNameSize,
					  char * p_szVarValue, int p_nMaxVarValueSize,
					  bool p_bFromSubgroup = false);
	bool FindNextVar(char * p_szVarName, int p_nMaxVarNameSize,
					  char * p_szVarValue, int p_nMaxVarValueSize);

	virtual int GetVar(const char * p_szGroup, const char * p_szVarName, char * p_szValue,
			   int p_nValueSize, int p_nPos = 0, bool p_bLogErrors = true,
			   const char * p_szSubgroup = "");
	virtual int GetVarRawString(const char * p_szGroup, const char * p_szVarName, char * p_szValue,
			   int p_nValueSize, int p_nPos = 0, bool p_bLogErrors = true,
			   const char * p_szSubgroup = "");
	virtual int GetVar(const char * p_szGroup, const char * p_szVarName, unsigned char * p_pValue,
			   int p_nValueSize, int p_nPos = 0, bool p_bLogErrors = true,
			   const char * p_szSubgroup = "");
	virtual int GetVar(const char * p_szGroup, const char * p_szVarName, int * p_pnValue,
			   int p_nPos = 0, bool p_bLogErrors = true,
			   const char * p_szSubgroup = "");
	virtual int GetVar(const char * p_szGroup, const char * p_szVarName, float * p_pfValue,
			   int p_nPos = 0, bool p_bLogErrors = true,
			   const char * p_szSubgroup = "");
	virtual int GetVar(const char * p_szGroup, const char * p_szVarName,
			   net_address * p_pNetAddress, int p_nPos = 0, bool p_bLogErrors = true,
			   const char * p_szSubgroup = "");
	virtual int GetVar(const char * p_szGroup, const char * p_szVarName,
				TIPv6Address * p_pIPv6Address, int p_nPos = 0, bool p_bLogErrors = true,
				const char * p_szSubgroup = "");
	virtual int GetVar(const char * p_szGroup, const char * p_szVarName,
			   valuesList * p_oValueList, int p_nPos = 0, bool p_bLogErrors = true, 
			   const char * p_szSubgroup = "");

	int SetVar(const char * p_szGroup, const char * p_szVarName,
			   const char * p_szValue, int p_nPos = 0, bool p_bCreateIfNotFound = false,
			   bool p_bLogErrors = true, const char * p_szSubgroup = "");
	int SetVarRawString(const char * p_szGroup, const char * p_szVarName,
			   const char * p_szValue, int p_nPos = 0, bool p_bCreateIfNotFound = false,
			   bool p_bLogErrors = true, const char * p_szSubgroup = "");

	int SetVarQuotes(const char * p_szGroup, const char * p_szVarName,
			   const char * p_szValue, int p_nPos = 0, bool p_bCreateIfNotFound = false,
			   bool p_bLogErrors = true, const char * p_szSubgroup = "");

	int SetVar(const char * p_szGroup, const char * p_szVarName, const unsigned char * p_pValue,
			   int p_nValueSize, int p_nPos = 0, bool p_bCreateIfNotFound = false,
			   bool p_bLogErrors = true, const char * p_szSubgroup = "");
	int SetVar(const char * p_szGroup, const char * p_szVarName, int p_nValue,
			   int p_nPos = 0, bool p_bCreateIfNotFound = false,
			   bool p_bLogErrors = true, const char * p_szSubgroup = "");
	int SetVar(const char * p_szGroup, const char * p_szVarName, float p_fValue,
			   int p_nPos = 0, bool p_bCreateIfNotFound = false,
			   bool p_bLogErrors = true, const char * p_szSubgroup = "");
	int SetVar(const char * p_szGroup, const char * p_szVarName,
			   const net_address & p_rNetAddress, int p_nPos = 0, bool p_bCreateIfNotFound = false,
			   bool p_bLogErrors = true, const char * p_szSubgroup = "");

	int SetVar(const char * p_szGroup, const char * p_szVarName,
			const TIPv6Address & p_rNetAddress, int p_nPos = 0, bool p_bCreateIfNotFound = false,
			bool p_bLogErrors = true, const char * p_szSubgroup = "");
};


#ifndef WIN32
//
// This class is created for the /tmp/RMP_Shared file, which is accessed
// by the user_interface. The replaceValue function of CIniParser tries
// to minimize the number of bytes written to disk, which ends up having
// a bad side effect on files that are written to frequently
//
// A value varies in length and is written many times will end up with a
// long comment trailing after it. This class overrides the replaceValue
// function to prevent this problem from occuring
//
class CIniParserPlus : public CIniParser
{
protected:
	virtual int replaceValue(const char * p_szOldValue, const char * p_pvNewVal,
					 unsigned int p_nNewValLen, bool p_bLogErrors = true);

};
#endif


/// @}
#endif // !defined(_INIPARSER_H_)
