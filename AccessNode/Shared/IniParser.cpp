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
* NAME:        IniParser.cpp
* AUTHOR:      Marius Chile
* DATE:        06.10.2003
* DESCRIPTION: Implementation of Ini Parser
****************************************************/

#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#else
#include <winsock.h>
#endif

#include <sys/file.h>
#include <stdio.h>

#include "IniParser.h"
#include "ctype.h"
#include "string.h"

#ifdef WIN32
#include "appmodule.h"
#endif

#ifndef WIN32
#define EOL "\n"
#define EOLSIZE 1
#define strnicmp strncasecmp
#else
#define EOL "\r\n"
#define EOLSIZE 2
#define snprintf _snprintf
#endif


using namespace std;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIniParser::CIniParser() : m_szLine( NULL ), m_szCrtGroup( NULL ),
                           m_szCrtSubgroup( NULL ), m_fpIniFile( NULL )
{
}

CIniParser::~CIniParser()
{
	Release();
}

///////////////////////////////////////////////////////////////////////////////////
// NAME: Load
// AUTHOR: Marius Chile
// DESCRIPTION: loads an ini file
// PARAMETERS:  p_szFileName - const char * [in] - the ini file to be loaded
//				p_szOpenMode - const char * [in] - file open mode (default: "rb")
//				p_bLock		 -
// RETURN:  0 - an error has occured
//			1 - file successfully loaded
///////////////////////////////////////////////////////////////////////////////////
int CIniParser::Load(const char * p_szFileName, const char* p_szOpenMode/* = "rb"*/, bool p_bLock /*= false*/)
{
	// close the loaded ini file first if necessary
	Release();


	// open file (binary for guaranteed success of fseek)
	m_fpIniFile = fopen(p_szFileName, p_szOpenMode);
	if (m_fpIniFile == NULL)
	{
		LOG_ERR("CIniParser::Load(%s)", p_szFileName);
		return 0;
	}

    m_szLine = new char[MAX_LINE_LEN+1];
    m_szCrtGroup = new char[MAX_LINE_LEN];
    m_szCrtSubgroup = new char[MAX_LINE_LEN];

	if(p_bLock && flock( fileno(m_fpIniFile), LOCK_EX ) )
	{
		LOG_ERR("CIniParser::Load: flock error");
	}
	return 1;
}

///////////////////////////////////////////////////////////////////////////////////
// NAME: Release
// AUTHOR: Marius Chile
// DESCRIPTION: releases an ini file
// PARAMETERS:
// RETURN: 1 - success
///////////////////////////////////////////////////////////////////////////////////
void CIniParser::Release()
{
	if ( m_fpIniFile )
	{	//if lock is set at close is unlock anyway
		fclose(m_fpIniFile);
        m_fpIniFile = NULL;
	}

    if( m_szLine )
    {
        delete [] m_szLine;
        m_szLine = NULL;
    }

    if( m_szCrtGroup )
    {
        delete [] m_szCrtGroup;
        m_szCrtGroup = NULL;
    }

    if( m_szCrtSubgroup )
    {
        delete [] m_szCrtSubgroup;
        m_szCrtSubgroup = NULL;
    }

    m_lValueFilePos = 0;
    m_lGroupStart = 0;
    m_lSubgroupStart = 0;
    m_lEnumVarStart = 0;
}


///////////////////////////////////////////////////////////////////////////////////
// NAME: moveToGroupEnd
// AUTHOR: Marius Chile
// DESCRIPTION: Moves the file pointer at the end of the (sub)group, i.e. after
//				the last non-empty line of the (sub)group and returns the new
//				file pointer position. It also checks if that particular line ends
//				with a newline (it can end with EOF).
// PARAMETERS: p_lFilePos		- long * [in/out] - the file pointer position
//			   p_bToNextEntity	- bool	 [in]	  - stop at the following entity (see note)
// RETURN:	true	- if the last non-empty line of the entity ends with a newline
//			false	- if the last non-empty line of the entity ends with EOF
// NOTE: When <p_bToNextEntity> = true, it stops at the first (sub)group encontered
//		 When <p_bToNextEntity> = false, it stops only at the first group encountered
///////////////////////////////////////////////////////////////////////////////////
bool CIniParser::moveToGroupEnd(long* p_lFilePos, bool p_bToNextEntity/* = false*/)
{
	long nCurrentLineStart = ftell(m_fpIniFile);
	long nPreviousLineStart = nCurrentLineStart;
	bool bJumpLine = false;

	// read line by line from file
	fseek(m_fpIniFile, 0, SEEK_CUR);
	while ( fgets(m_szLine, MAX_LINE_LEN, m_fpIniFile) )
	{
        char * szString = m_szLine;

		// escape spaces
		for ( ;isspace( *szString ); szString++ )
			;

        // is not a group definition
		if ( ! p_bToNextEntity )
		{
			if ( *szString != GROUP_NAME_BEGIN_CHAR )
			{
				if ( *szString )
				{
					nPreviousLineStart = nCurrentLineStart;
					bJumpLine = true;
				}
				nCurrentLineStart = ftell(m_fpIniFile);
				continue;
			}
		}
		else
		{
			if ( *szString != GROUP_NAME_BEGIN_CHAR && *szString != SUBGROUP_NAME_BEGIN_CHAR )
			{
				if ( *szString )
				{
					nPreviousLineStart = nCurrentLineStart;
					bJumpLine = true;
				}
				nCurrentLineStart = ftell(m_fpIniFile);
				continue;
			}
		}

		break;
	}

	if (bJumpLine)
	{
		fseek(m_fpIniFile, nPreviousLineStart, SEEK_SET);
		fgets(m_szLine, MAX_LINE_LEN, m_fpIniFile);
		nPreviousLineStart = ftell(m_fpIniFile);
	}

	*p_lFilePos = nPreviousLineStart;

	if ( feof(m_fpIniFile) )
	{
		fseek(m_fpIniFile, -1, SEEK_END);
		*m_szLine = 0;
		fgets(m_szLine, MAX_LINE_LEN, m_fpIniFile);
		return ( *m_szLine == '\n' ? true : false );
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////
// NAME: FindGroup
// AUTHOR: Marius Chile
// DESCRIPTION: locates a group in the file and positions the file pointer at the
//				beginning of the line following the group name
// PARAMETERS: p_szGroup      - const char * [in] - the group to be found
//			   p_bGetNext     - bool		 [in] - find next group
//			   p_bLogErrors   - bool		 [in] - write errors to log file
//			   p_bUnknownName - bool		 [in] - return the name of the found group
// RETURN: - the group name or the line containing the group name.
//		   - 1 when <p_szGroup> = NULL and <p_bUnknownName> = false
//		   - NULL when an error has occured or group not found
// NOTE: - Line format: [ group_name ]
//		 - When called with <p_szGroup> = NULL and <p_bUnknownName> = false, it positions the
//		   file pointer at the start of the variables of a group found with a successful
//		   previous call to FindGroup. The result is undefined if FindGroup is called for the
//		   first time with <p_szGroup> = NULL and <p_bUnknownName> = false.
//		 - p_bGetNext = FALSE - finds the first appearance of a group (from the start)
//						TRUE  - finds the next appearance of a group (from the position
//								of the last <p_szGroup> group found in the file)
//		 - p_bUnknownName = FALSE - finds the <p_szGroup> group
//							TRUE - returns the name of the found group (useful when
//								   enumerating the groups without knowing their name).
//								   In this case, <p_szGroup> is ignored.
///////////////////////////////////////////////////////////////////////////////////
const char * CIniParser::FindGroup(const char * p_szGroup, bool p_bGetNext/* = false*/,
				bool p_bLogErrors/* = true*/, bool p_bUnknownName/* = false*/)
{
    if ( ! p_szGroup && ! p_bUnknownName )
    {
		fseek(m_fpIniFile, m_lGroupStart, SEEK_SET);
		return (char*)1;
	}

	if ( ! p_bGetNext )
	{
		m_lGroupStart = 0;
		m_lSubgroupStart = 0;
	}

	fseek(m_fpIniFile, m_lGroupStart, SEEK_SET);

	// read line by line from file
	while ( fgets(m_szLine, MAX_LINE_LEN, m_fpIniFile) )
	{
        char * szString = m_szLine;

		// escape spaces
		for ( ;isspace( *szString ); szString++ )
			;

        // is not a group definition
		if ( *szString != GROUP_NAME_BEGIN_CHAR )
			continue;

		// escape spaces
		for ( szString++ ;isspace( *szString ); szString++ )
			;

		if ( ! p_bUnknownName )
		{
			if (strnicmp(p_szGroup, szString, strlen(p_szGroup)))
				continue;

			// escape spaces
			for( szString += strlen(p_szGroup); isspace( *szString ); szString++ )
				;

			// is not a group definition
			if ( *szString != GROUP_NAME_END_CHAR )
				continue;

			// remember found group name
			strncpy(m_szCrtGroup, p_szGroup, MAX_LINE_LEN);
		}
		else
		{
			char * pEndOfGroup = strchr(szString, GROUP_NAME_END_CHAR);
			if ( ! pEndOfGroup )
				continue;

			// escape spaces
			for( pEndOfGroup--; isspace( *pEndOfGroup ) && pEndOfGroup >= szString ; pEndOfGroup-- )
				;

			// is not a group definition
			if ( pEndOfGroup < szString )
				continue;

			*(++pEndOfGroup) = 0;
			if ( strchr(szString, '#') )
				continue;
			strncpy(m_szLine, szString, MAX_LINE_LEN);

			// remember found group name
			strncpy(m_szCrtGroup, m_szLine, MAX_LINE_LEN);
		}

		m_lGroupStart = ftell(m_fpIniFile);
		m_lSubgroupStart = m_lGroupStart;

        return m_szLine;
	}

	if ( ! p_bGetNext && p_bLogErrors )
	{
		if (p_szGroup)
		{
			LOG("CIniParser::FindGroup - the group %s was not found", p_szGroup);
			// remember group name
			strncpy(m_szCrtGroup, p_szGroup, MAX_LINE_LEN);
		}
		else
			LOG("CIniParser::FindGroup - No groups were found");
	}

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////
// NAME: FindFirstGroup
// AUTHOR: Marius Chile
// DESCRIPTION: locates the first group in the file, returns the group name and positions
//				the file pointer at the beginning of the line following the group name
// PARAMETERS: p_szGroup	   - char *	 [in, out]	- the buffer to hold the group
//			   p_nMaxGroupSize - int	 [in]		- the maximum buffer size
//			   p_bLogErrors    - bool	 [in]		- write errors to log file
// RETURN: true - group successfully returned
//		   false - an error has occured or group not found
///////////////////////////////////////////////////////////////////////////////////
bool CIniParser::FindFirstGroup(char * p_szGroup, int p_nMaxGroupSize, bool p_bLogErrors/* = true*/)
{
	const char * szGroup = FindGroup(NULL, false, p_bLogErrors, true);
	if ( szGroup && *szGroup )
	{
		strncpy(p_szGroup, szGroup, p_nMaxGroupSize);
		return true;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////
// NAME: FindNextGroup
// AUTHOR: Marius Chile
// DESCRIPTION: locates the next group in the file, returns the group name and positions
//				the file pointer at the beginning of the line following the group name
// PARAMETERS: p_szGroup	   - char *	[in, out]	- the buffer to hold the group
//			   p_nMaxGroupSize - int	[in]		- the maximum buffer size
//			   p_bLogErrors    - bool	[in]		- write errors to log file
// RETURN: true - group successfully returned
//		   false - an error has occured or group not found
// NOTE: The search begins at the position set by a previous successful call to
//		 FindFirstGroup or FindNextGroup
///////////////////////////////////////////////////////////////////////////////////
bool CIniParser::FindNextGroup(char * p_szGroup, int p_nMaxGroupSize, bool p_bLogErrors/* = true*/)
{
	const char * szGroup = FindGroup(NULL, true, p_bLogErrors, true);
	if ( szGroup && *szGroup )
	{
		strncpy(p_szGroup, szGroup, p_nMaxGroupSize);
		return true;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////
// NAME: CreateGroup
// AUTHOR: Marius Chile
// DESCRIPTION: creates a group in the file and positions the file pointer at the
//				beginning of the line following the group name
// PARAMETERS: p_szGroup    - const char * [in] - the group to be created
//			   p_bLogErrors - bool		   [in] - write errors to log file
// RETURN: 0 - an error has occured
//		   1 - group successfully created
///////////////////////////////////////////////////////////////////////////////////
#include <errno.h>
int CIniParser::CreateGroup(const char * p_szGroup, bool p_bLogErrors/* = true*/)
{
	int rv = fseek(m_fpIniFile, 0, SEEK_END);
	if ( rv == -1 ) {
		printf("ERROR in creategroup\n");
		return 0 ;
	}

	snprintf(m_szLine, MAX_LINE_LEN, "%s[%s]%s%s", EOL, p_szGroup, EOL, EOL);

	size_t nWritten = fwrite(m_szLine, strlen(m_szLine), 1, m_fpIniFile);
	if (!nWritten)
	{
		printf("ERR:%p %s\n", m_fpIniFile, strerror(errno) );
		if ( p_bLogErrors )
			LOG_ERR("CIniParser::CreateGroup - could write group %s", p_szGroup);
		return 0;
	}

	m_lGroupStart = ftell(m_fpIniFile) - EOLSIZE;
	m_lSubgroupStart = m_lGroupStart;

	return 1;
}



///////////////////////////////////////////////////////////////////////////////////
// NAME: FindSubgroup
// AUTHOR: Marius Chile
// DESCRIPTION: locates a subgroup in a group and positions the file pointer at the
//				beginning of the line following the subgroup name
// PARAMETERS: p_szSubgroup   - const char * [in] - the subgroup to be found
//			   p_bGetNext     - bool		 [in] - find next subgroup
//			   p_bLogErrors   - bool		 [in] - write errors to log file
//			   p_bUnknownName - bool		 [in] - return the name of the found subgroup
// RETURN: - the subgroup name or the line containing the subgroup name.
//		   - 1 when <p_szSubgroup> = NULL and <p_bUnknownName> = false
//		   - NULL when an error has occured or subgroup not found
// NOTE: - Line format: < subgroup_name >
//		 - FindSubgroup SHOULD BE CALLED AFTER A SUCCESSFUL PREVIOUS CALL TO FindGroup,
//		   FindFirstGroup or FindNextGroup
//		 - When called with <p_szSubgroup> = NULL and <p_bUnknownName> = false it positions
//		   the file pointer at the start of the variables of a subgroup found with a
//		   successful previous call to FindSubgroup. The result is undefined if FindSubgroup
//		   is called for the first time with <p_szSubgroup> = NULL and <p_bUnknownName> = false.
//		 - p_bGetNext = FALSE - finds the first appearance of the subgroup (from the group start)
//						TRUE  - finds the next appearance of the subgroup (from the position
//								of the last <p_szSubgroup> subgroup found in the searched group)
//		 - p_bUnknownName = FALSE - finds the <p_szSubgroup> subgroup
//							TRUE - returns the name of the found subgroup (useful when
//								   enumerating the subgroups without knowing their name).
//								   In this case, <p_szSubgroup> is ignored.
///////////////////////////////////////////////////////////////////////////////////
const char * CIniParser::FindSubgroup(const char * p_szSubgroup, bool p_bGetNext/* = false*/,
									  bool p_bLogErrors/* = true*/, bool p_bUnknownName/* = false*/)
{
    if ( ! p_szSubgroup && ! p_bUnknownName )
    {
		fseek(m_fpIniFile, m_lSubgroupStart, SEEK_SET);
		return (char*)1;
	}

	if ( ! p_bGetNext )
		m_lSubgroupStart = m_lGroupStart;

	fseek(m_fpIniFile, m_lSubgroupStart, SEEK_SET);

	// read line by line from file
	while ( fgets(m_szLine, MAX_LINE_LEN, m_fpIniFile) )
	{
        char * szString = m_szLine;

		// escape spaces
		for ( ;isspace( *szString ); szString++ )
			;

        // is not a subgroup definition
		if ( *szString != SUBGROUP_NAME_BEGIN_CHAR )
		{
			if ( *szString == GROUP_NAME_BEGIN_CHAR )
				break;
			continue;
		}

		// escape spaces
		for ( szString++ ;isspace( *szString ); szString++ )
			;

		if ( ! p_bUnknownName )
		{
			if (strnicmp(p_szSubgroup, szString, strlen(p_szSubgroup)))
				continue;

			// escape spaces
			for( szString += strlen(p_szSubgroup); isspace( *szString ); szString++ )
				;

			// is not a group definition
			if ( *szString != SUBGROUP_NAME_END_CHAR )
				continue;

			// remember found subgroup name
			strncpy(m_szCrtSubgroup, p_szSubgroup, MAX_LINE_LEN);
		}
		else
		{
			char * pEndOfSubgroup = strchr(szString, SUBGROUP_NAME_END_CHAR);
			if ( ! pEndOfSubgroup )
				continue;

			// escape spaces
			for( pEndOfSubgroup--; isspace( *pEndOfSubgroup ) && pEndOfSubgroup >= szString ; pEndOfSubgroup-- )
				;

			// is not a group definition
			if ( pEndOfSubgroup < szString )
				continue;

			*(++pEndOfSubgroup) = 0;
			if ( strchr(szString, '#') )
				continue;
			strncpy(m_szLine, szString, MAX_LINE_LEN);

			// remember found subgroup name
			strncpy(m_szCrtSubgroup, m_szLine, MAX_LINE_LEN);
		}

		m_lSubgroupStart = ftell(m_fpIniFile);

        return m_szLine;
	}

	if ( ! p_bGetNext && p_bLogErrors )
	{
		if (p_szSubgroup)
		{
			LOG("CIniParser::FindSubgroup - the subgroup %s was not found", p_szSubgroup);

			// remember subgroup name
			strncpy(m_szCrtSubgroup, p_szSubgroup, MAX_LINE_LEN);
		}
		else
			LOG("CIniParser::FindSubgroup - No subgroups were found");
	}

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////
// NAME: FindFirstSubgroup
// AUTHOR: Marius Chile
// DESCRIPTION: locates the first subgroup in a group, returns the subgroup name and positions
//				the file pointer at the	beginning of the line following the subgroup name
// PARAMETERS: p_szSubgroup	      - char *	[in, out] - the buffer to hold the subgroup
//			   p_nMaxSubgroupSize - int		[in]	  - the maximum buffer size
//			   p_bLogErrors       - bool	[in]	  - write errors to log file
// RETURN: true - subgroup successfully returned
//		   false - an error has occured or subgroup not found
// NOTE: The search begins at the position set by a previous successful call to
//		 FindFirstGroup or FindNextGroup.
///////////////////////////////////////////////////////////////////////////////////
bool CIniParser::FindFirstSubgroup(char * p_szSubgroup, int p_nMaxSubgroupSize,
								   bool p_bLogErrors/* = true*/)
{
	const char * szSubgroup = FindSubgroup(NULL, false, p_bLogErrors, true);
	if ( szSubgroup && *szSubgroup )
	{
		strncpy(p_szSubgroup, szSubgroup, p_nMaxSubgroupSize);
		return true;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////
// NAME: FindNextSubgroup
// AUTHOR: Marius Chile
// DESCRIPTION: locates the next subgroup in a group, returns the subgroup name and positions
//				the file pointer at the	beginning of the line following the subgroup name
// PARAMETERS: p_szSubgroup	      - char *	[in, out] - the buffer to hold the subgroup
//			   p_nMaxSubgroupSize - int		[in]	  - the maximum buffer size
//			   p_bLogErrors       - bool	[in]	  - write errors to log file
// RETURN: true - subgroup successfully returned
//		   false - an error has occured or subgroup not found
// NOTE: The search begins at the position set by a previous successful call to
//		 FindFirstSubgroup
///////////////////////////////////////////////////////////////////////////////////
bool CIniParser::FindNextSubgroup(char * p_szSubgroup, int p_nMaxSubgroupSize,
								  bool p_bLogErrors/* = true*/)
{
	const char * szSubgroup = FindSubgroup(NULL, true, p_bLogErrors, true);
	if ( szSubgroup && *szSubgroup )
	{
		strncpy(p_szSubgroup, szSubgroup, p_nMaxSubgroupSize);
		return true;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////
// NAME: CreateSubgroup
// AUTHOR: Marius Chile
// DESCRIPTION: creates a subgroup in a group and positions the file pointer at the
//				beginning of the line following the subgroup name
// PARAMETERS: p_szGroup     - const char * [in] - the group that should contain the new subgroup
//			   p_szSubgroup  - const char * [in] - the subgroup to be created
//			   p_bLogErrors  - bool			[in] - write errors to log file
// RETURN: 0 - an error has occured
//		   1 - subgroup successfully created
///////////////////////////////////////////////////////////////////////////////////
int CIniParser::CreateSubgroup(const char * p_szGroup, const char * p_szSubgroup,
							   bool p_bLogErrors/* = true*/)
{
	const char * szNewLine = "";

	// move to the targeted group first
	if ( ! FindGroup( p_szGroup ) )
	{
		// group not found, create it
		if ( ! CreateGroup( p_szGroup, p_bLogErrors ) )
			return 0;
	}

	fseek(m_fpIniFile, m_lGroupStart, SEEK_SET);
	if ( ! moveToGroupEnd(&m_lValueFilePos) )
		szNewLine = EOL;
	snprintf(m_szLine, MAX_LINE_LEN, "%s%s<%s>%s%s", szNewLine, EOL, p_szSubgroup, EOL,	EOL);
	if ( ! replaceValue("", m_szLine, strlen(m_szLine), p_bLogErrors) )
		return 0;

	m_lSubgroupStart = m_lValueFilePos + strlen(p_szSubgroup) + 2 * EOLSIZE + 2;

	return 1;
}






///////////////////////////////////////////////////////////////////////////////////
// NAME: FindFirstVar
// AUTHOR: Marius Chile
// DESCRIPTION: locates the first variable in a (sub)group and returns the
//				variable name and its value
// PARAMETERS: p_szVarName	      - char *	[in, out] - the buffer to hold the var. name
//			   p_nMaxVarNameSize  - int		[in]	  - the maximum var. name buffer size
//			   p_szVarValue	      - char *	[in, out] - the buffer to hold the var. value
//			   p_nMaxVarValueSize - int		[in]	  - the maximum var. value buffer size
//			   p_bFromSubgroup	  - bool	[in]	  - look for the variable in the subgroup
//												   returned from a Find op. for subgroups
// RETURN: true - var. name and value successfully returned
//		   false - an error has occured or variable not found
// NOTE: The search begins at the position set by a previous successful call to
//		 FindFirstGroup, FindNextGroup, FindFirstSubgroup or FindNextSubgroup
///////////////////////////////////////////////////////////////////////////////////
bool CIniParser::FindFirstVar(  char * p_szVarName, int p_nMaxVarNameSize,
				char * p_szVarValue, int p_nMaxVarValueSize,
				bool p_bFromSubgroup/* = false*/)
{
	if ( ! p_bFromSubgroup )
		m_lEnumVarStart = m_lGroupStart;
	else
		m_lEnumVarStart = m_lSubgroupStart;
	fseek(m_fpIniFile, m_lEnumVarStart, SEEK_SET);
	const char * szValue = findVariable(NULL, p_szVarName, p_nMaxVarNameSize);
	if (szValue)
	{
		strncpy(p_szVarValue, szValue, p_nMaxVarValueSize);
		return true;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////
// NAME: FindNextVar
// AUTHOR: Marius Chile
// DESCRIPTION: locates the next variable in a (sub)group and returns the
//				variable name and its value
// PARAMETERS: p_szVarName	      - char *	[in, out] - the buffer to hold the var. name
//			   p_nMaxVarNameSize  - int		[in]	  - the maximum var. name buffer size
//			   p_szVarValue	      - char *	[in, out] - the buffer to hold the var. value
//			   p_nMaxVarValueSize - int		[in]	  - the maximum var. value buffer size
// RETURN: true - variable name and value successfully returned
//		   false - an error has occured or variable not found
// NOTE: The search begins at the position set by a previous successful call to
//		 FindFirstVar
///////////////////////////////////////////////////////////////////////////////////
bool CIniParser::FindNextVar(char * p_szVarName, int p_nMaxVarNameSize,
			char * p_szVarValue, int p_nMaxVarValueSize)
{
	fseek(m_fpIniFile, m_lEnumVarStart, SEEK_SET);
	const char * szValue = findVariable(NULL, p_szVarName, p_nMaxVarNameSize);
	if (szValue)
	{
		strncpy(p_szVarValue, szValue, p_nMaxVarValueSize);
		return true;
	}

	return false;
}


///////////////////////////////////////////////////////////////////////////////////
// NAME: findVariable
// AUTHOR: Marius Chile
// DESCRIPTION: locates a variable from a (sub)group and positions the file
//				pointer at the beginning of the line following the variable
// PARAMETERS: 	p_szGroup    	- const char * [in]		 - the group where the variable resides
//		p_szVarName  	- const char * [in]		 - the variable name
//		p_nPos	    	- int &		   [in, out] 	- the number of the variable in a group of variables having the same name
//		p_bLogErrors 	- bool		   [in]		- write errors to log file
//		p_szSubgroup 	- const char * [in]		- look for variable in the <p_szSubgroup>  subgroup of the <p_szGroup> group

// RETURN: - a buffer containing the variable value
//		   - NULL if an error has occured or variable not found
// NOTE: - Line format: var_name = value
//		 - if the call is successful, <p_nPos> stores the difference between the original
//		   <p_nPos> and how many vars with the same <p_szVarName> exist in the (sub)group.
///////////////////////////////////////////////////////////////////////////////////

const char * CIniParser::findVariable(const char * p_szGroup, const char * p_szVarName,
				int & p_nPos, bool p_bLogErrors/* = true*/, const char * p_szSubgroup/* = ""*/)
{
	m_lValueFilePos = 0;

    if( FindGroup( p_szGroup, false, p_bLogErrors ) )
    {
		if ( ( ! p_szSubgroup || (p_szSubgroup && *p_szSubgroup)) &&
			! FindSubgroup( p_szSubgroup, false, p_bLogErrors ) )
		{
			return NULL;
		}

        for( ; p_nPos > 0 && findVariable( p_szVarName ); p_nPos-- )
            ;

        if( ! p_nPos )
            return findVariable( p_szVarName );
    }

	return NULL;
}
///////////////////////////////////////////////////////////////////////////////////
// NAME: findVariableRaw
// AUTHOR: Mihai Buha
// DESCRIPTION: locates a variable from a (sub)group and positions the file
//				pointer at the beginning of the line following the variable
// PARAMETERS: p_szGroup    - const char * [in]		 - the group where the variable resides
//			   p_szVarName  - const char * [in]		 - the variable name
//			   p_nPos	    - int &		   [in, out] - the number of the variable in a
//													group of variables having the same name
//			   p_bLogErrors - bool		   [in]		 - write errors to log file
//			   p_szSubgroup - const char * [in]		 - look for variable in the <p_szSubgroup>
//													   subgroup of the <p_szGroup> group
// RETURN: - a buffer containing the variable value
//		   - NULL if an error has occured or variable not found
// NOTE: - Line format: var_name = value
//		 - if the call is successful, <p_nPos> stores the difference between the original
//		   <p_nPos> and how many vars with the same <p_szVarName> exist in the (sub)group.
// This does exactly the same algorithm as the other similar findVariable, except it calls the findVariableRaw worker
///////////////////////////////////////////////////////////////////////////////////
const char * CIniParser::findVariableRaw(const char * p_szGroup, const char * p_szVarName,
				int & p_nPos, bool p_bLogErrors/* = true*/, const char * p_szSubgroup/* = ""*/)
{
	m_lValueFilePos = 0;

    if( FindGroup( p_szGroup, false, p_bLogErrors ) )
    {
		if ( ( ! p_szSubgroup || (p_szSubgroup && *p_szSubgroup)) &&
			! FindSubgroup( p_szSubgroup, false, p_bLogErrors ) )
		{
			return NULL;
		}

        for( ; p_nPos > 0 && findVariableRaw( p_szVarName ); p_nPos-- )
            ;

        if( ! p_nPos )
            return findVariableRaw( p_szVarName );
    }

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////
// NAME: findVariable
// AUTHOR: Marius Chile
// DESCRIPTION: locates a variable in the file and positions the file pointer at the
//				beginning of the line following the variable
// PARAMETERS: p_szVarName	  - const char * [in]	   - the variable name
//			   p_szName		  - char *		 [in, out] - buffer to hold the variable name
//			   p_nMaxNameSize - int			 [in]      - maximum buffer size for var. name
// RETURN: - a buffer containing the variable value
//		   - NULL if an error has occured or variable not found
// NOTE: Line format: var_name = value
//		 The last 2 arguments specify if the variable name should be returned. This is
//		 useful when enumerating the variables with their values, because then their names
//		 are not known. Leave them as default when looking for a variable with a known name.
///////////////////////////////////////////////////////////////////////////////////
const char * CIniParser::findVariable(const char * p_szVarName, char * p_szName/* = NULL*/,
									  int p_nMaxNameSize/* = 0*/)
{
	// read line by line from file
	m_lValueFilePos = ftell(m_fpIniFile);
	while ( fgets(m_szLine, MAX_LINE_LEN, m_fpIniFile) )
	{
		char * szString = m_szLine;

		// escape spaces
		for ( ;isspace( *szString ); szString++ )
			;

		if( *szString == GROUP_NAME_BEGIN_CHAR || *szString == SUBGROUP_NAME_BEGIN_CHAR )
			break;

		if ( p_szVarName )
		{
			if (strnicmp(p_szVarName, szString, strlen(p_szVarName)))
			{
				m_lValueFilePos = ftell(m_fpIniFile);
				continue;
			}

			// escape spaces
			for ( szString += strlen(p_szVarName);isspace( *szString ); szString++ )
				;

			// is not a valid definition
			if ( *szString != '=' )
			{
				m_lValueFilePos = ftell(m_fpIniFile);
				continue;
			}
		}
		else
		{
			char * pEndOfVar = strchr(szString, '=');
			if ( ! pEndOfVar )
				continue;

			// save the position of '='
			char * szValue = pEndOfVar;

			// escape spaces
			for( pEndOfVar--; isspace( *pEndOfVar ) && pEndOfVar >= szString ; pEndOfVar-- )
				;

			// is not a valid variable name
			if ( pEndOfVar < szString )
				continue;

			*(++pEndOfVar) = 0;
			if ( strchr(szString, '#') )
				continue;
			strncpy(p_szName, szString, p_nMaxNameSize);
			szString = szValue;
		}

		// escape spaces after "="
		for ( szString++; isspace( *szString ) && !strchr("\r\n", *szString); szString++ )
			;

		// seek to the beginning of the value (to know where to write when set is called)
		m_lValueFilePos += szString - m_szLine;

		char * pEndOfValue;
		if( *szString == '"' )
		{
			pEndOfValue = strchr( szString + 1, '"' );
			if (pEndOfValue)
				pEndOfValue++;
		}
		else
			if( *szString == '\'' )
			{
				pEndOfValue = strchr( szString + 1, '\'' );
				if (pEndOfValue)
					pEndOfValue++;
			}
			else
				pEndOfValue =  strpbrk( szString, "\n\r#" );

		if( pEndOfValue )
		{
			// trim trailing spaces
			for ( pEndOfValue--; pEndOfValue >= szString && isspace(*pEndOfValue); pEndOfValue--)
				;
			*(pEndOfValue + 1) = '\0';
		}

		m_lEnumVarStart = ftell(m_fpIniFile);
		return szString;
	}

	return NULL;
}
///////////////////////////////////////////////////////////////////////////////////
// NAME: findVariableRaw
// AUTHOR: Mihai Buha
// DESCRIPTION: locates a variable in the file and positions the file pointer at the
//				beginning of the line following the variable
// PARAMETERS: p_szVarName	  - const char * [in]	   - the variable name
//			   p_szName		  - char *		 [in, out] - buffer to hold the variable name
//			   p_nMaxNameSize - int			 [in]      - maximum buffer size for var. name
// RETURN: - a buffer containing the variable value
//		   - NULL if an error has occured or variable not found
// NOTE: Line format: var_name = value
//		 The last 2 arguments specify if the variable name should be returned. This is
//		 useful when enumerating the variables with their values, because then their names
//		 are not known. Leave them as default when looking for a variable with a known name.
///////////////////////////////////////////////////////////////////////////////////
const char * CIniParser::findVariableRaw(const char * p_szVarName, char * p_szName/* = NULL*/,
									  int p_nMaxNameSize/* = 0*/)
{
	// read line by line from file
	m_lValueFilePos = ftell(m_fpIniFile);
	while ( fgets(m_szLine, MAX_LINE_LEN, m_fpIniFile) )
	{
        char * szString = m_szLine;

		// escape spaces
		for ( ;isspace( *szString ); szString++ )
			;

        if( *szString == GROUP_NAME_BEGIN_CHAR || *szString == SUBGROUP_NAME_BEGIN_CHAR )
            break;

		if ( p_szVarName )
		{
			if (strnicmp(p_szVarName, szString, strlen(p_szVarName)))
			{
				m_lValueFilePos = ftell(m_fpIniFile);
				continue;
			}

			// escape spaces
			for ( szString += strlen(p_szVarName);isspace( *szString ); szString++ )
				;

			// is not a valid definition
			if ( *szString != '=' )
			{
				m_lValueFilePos = ftell(m_fpIniFile);
				continue;
			}
		}
		else
		{
			char * pEndOfVar = strchr(szString, '=');
			if ( ! pEndOfVar )
				continue;

			// save the position of '='
			char * szValue = pEndOfVar;

			// escape spaces
			for( pEndOfVar--; isspace( *pEndOfVar ) && pEndOfVar >= szString ; pEndOfVar-- )
				;

			// is not a valid variable name
			if ( pEndOfVar < szString )
				continue;

			*(++pEndOfVar) = 0;
			if ( strchr(szString, '#') )
				continue;
			strncpy(p_szName, szString, p_nMaxNameSize);
			szString = szValue;
		}

		// escape spaces after "="
		for ( szString++; isspace( *szString ) && !strchr("\r\n", *szString); szString++ )
			;

		// seek to the beginning of the value (to know where to write when set is called)
		m_lValueFilePos += szString - m_szLine;

        char * pEndOfValue =  strpbrk( szString, "\n\r" );

        if( pEndOfValue )
        {
			// trim trailing spaces
			for ( pEndOfValue--; pEndOfValue >= szString && isspace(*pEndOfValue); pEndOfValue--)
				;
            *(pEndOfValue + 1) = '\0';
        }

		m_lEnumVarStart = ftell(m_fpIniFile);
        return szString;
	}

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////
// NAME: GetVar
// AUTHOR: Marius Chile
// DESCRIPTION: gets a value of type octet-string of a variable from the ini file
// PARAMETERS: p_szGroup   - const char * [in] - the group where the the variable should be
//			   p_szVarName - const char * [in] - the variable name
//			   p_szValue   - char *	  [in/out] - the location where the variable value
//												 will be held
//			   p_nValueSize  - int		  [in] - the maximum length of the value
//			   p_nPos        - int		  [in] - the number of the variable in a group of
//												 variables having the same name
//			   p_bLogErrors - bool		   [in] - write errors to log file
//			   p_szSubgroup - const char * [in] - look for variable in the <p_szSubgroup>
//												  subgroup of the <p_szGroup> group
// RETURN:     0	 - an error has occured
//			   n > 0 - the real value size actually read.
// NOTE: If the value is enclosed in quotes they are removed.
///////////////////////////////////////////////////////////////////////////////////
int CIniParser::GetVar(  const char * p_szGroup, const char * p_szVarName, char * p_szValue
			, int p_nValueSize, int p_nPos/* = 0*/, bool p_bLogErrors/* = true*/
			, const char * p_szSubgroup/* = ""*/)
{
	int nDiff = p_nPos;
    const char * szValue = findVariable( p_szGroup, p_szVarName, nDiff, p_bLogErrors,
										 p_szSubgroup );
    if( szValue )
    {
		if ( *szValue == 0 )
		{
			// value missing
			*p_szValue = 0;
			if ( p_bLogErrors )
				LOG("[%s].%s is empty, set to \"\"", m_szCrtGroup, p_szVarName );
			return 1;
		}

		if ( *szValue == '"' || *szValue == '\'')
			szValue++;
        strncpy( p_szValue, szValue, p_nValueSize - 1 );

        p_szValue[ p_nValueSize - 1 ] = '\0';

		size_t nLen = strlen( p_szValue );
		if ( p_szValue[nLen - 1] == '"' || p_szValue[nLen - 1] == '\'' )
			p_szValue[nLen - 1] = '\0';

        return nLen;
    }

	if ( ! p_nPos && p_bLogErrors )
		LOG("Not found: [%s].%s", m_szCrtGroup, p_szVarName);

	return 0;
}
///////////////////////////////////////////////////////////////////////////////////
// NAME: GetVarRawString
// AUTHOR: Mihai Buha
// DESCRIPTION: gets a value of type octet-string of a variable from the ini file
// PARAMETERS: p_szGroup   - const char * [in] - the group where the the variable should be
//			   p_szVarName - const char * [in] - the variable name
//			   p_szValue   - char *	  [in/out] - the location where the variable value
//												 will be held
//			   p_nValueSize  - int		  [in] - the maximum length of the value
//			   p_nPos        - int		  [in] - the number of the variable in a group of
//												 variables having the same name
//			   p_bLogErrors - bool		   [in] - write errors to log file
//			   p_szSubgroup - const char * [in] - look for variable in the <p_szSubgroup>
//												  subgroup of the <p_szGroup> group
// RETURN:     0	 - an error has occured
//			   n > 0 - the real value size actually read.
// NOTE: No processing is applied to the string stored in the file
//  This function is derived from the regular GetVar
///////////////////////////////////////////////////////////////////////////////////
int CIniParser::GetVarRawString(const char * p_szGroup, const char * p_szVarName, char * p_szValue,
					   int p_nValueSize, int p_nPos/* = 0*/, bool p_bLogErrors/* = true*/,
					   const char * p_szSubgroup/* = ""*/)
{
	int nDiff = p_nPos;
	const char * szValue = findVariableRaw( p_szGroup, p_szVarName, nDiff, p_bLogErrors, p_szSubgroup );
	if( szValue ){
		if ( *szValue == 0 ){
			// value missing
			*p_szValue = 0;
			if ( p_bLogErrors )
				LOG("[%s].%s is empty, set to \"\"", m_szCrtGroup, p_szVarName );
			return 1;
		}
		strncpy( p_szValue, szValue, p_nValueSize - 1 );

		p_szValue[ p_nValueSize - 1 ] = '\0';

		size_t nLen = strlen( p_szValue );

	 	return nLen;
	}

	if ( ! p_nPos && p_bLogErrors )
		LOG("Not found: [%s].%s", m_szCrtGroup, p_szVarName);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////
// NAME: GetVar
// AUTHOR: Marius Chile
// DESCRIPTION: gets a value of type hex-octet-string of a variable from the ini file
// PARAMETERS: p_szGroup   - const char * [in] - the group where the variable should be
//			   p_szVarName - const char * [in] - the variable name
//			   p_pValue   - unsigned char * [in/out] - the location where the variable value
//												 will be held
//			   p_nValueSize  - int		  [in] - the length of the value
//			   p_nPos        - int		  [in] - the number of the variable in a
//											     group of variables having the same name
//			   p_bLogErrors - bool		  [in] - write errors to log file
//			   p_szSubgroup - const char * [in] - look for variable in the <p_szSubgroup>
//												  subgroup of the <p_szGroup> group
// RETURN:     -1	 - an error has occured
//			   n >= 0 - the real value size actually read
// NOTE:	   - If the value from the file contains invalid chars (non-hex), it is read
//				 into the buffer up to the first invalid char encontered, and the number
//				 of correct chars read is returned.
//			   - If the value is enclosed in quotes they are removed.
///////////////////////////////////////////////////////////////////////////////////
int CIniParser::GetVar(const char * p_szGroup, const char * p_szVarName,
					   unsigned char * p_pValue, int p_nValueSize, int p_nPos/* = 0*/,
					   bool p_bLogErrors/* = true*/, const char * p_szSubgroup/* = ""*/)
{
	int nDiff = p_nPos;
    const char * szValue = findVariable( p_szGroup, p_szVarName, nDiff, p_bLogErrors,
										 p_szSubgroup );
    if( szValue )
    {
		if ( *szValue == 0 )
		{
			// value missing
			//*p_pValue = 0;
			if ( p_bLogErrors )
				LOG("[%s].%s is empty", m_szCrtGroup, p_szVarName);
			return 0;
		}

		if ( *szValue == '"' || *szValue == '\'' )
			szValue++;

		unsigned char * pTmp = p_pValue;
		unsigned int nByte;
		while ( p_nValueSize-- && sscanf(szValue, "%2x", &nByte) == 1 )
		{
			*(pTmp++) = (unsigned char)nByte;

			for ( szValue += 2 ; isspace( *szValue ) || *szValue == ':'; szValue++ )
				;
		}

		return pTmp - p_pValue;
	}

	if ( ! p_nPos && p_bLogErrors )
		LOG("Not found: [%s].%s", m_szCrtGroup, p_szVarName);

	return -1;
}

///////////////////////////////////////////////////////////////////////////////////
// NAME: GetVar
// AUTHOR: Marius Chile
// DESCRIPTION: gets a value of type int of a variable from the ini file
// PARAMETERS: p_szGroup   - const char * [in] - the group where the variable should be
//			   p_szVarName - const char * [in] - the variable name
//			   p_nValue    - int *	  [in/out] - the location where the variable value
//												 will be held
//			   p_nPos        - int		  [in] - the number of the variable in a
//											     group of variables having the same name
//			   p_bLogErrors - bool		  [in] - write errors to log file
//			   p_szSubgroup - const char * [in] - look for variable in the <p_szSubgroup>
//												  subgroup of the <p_szGroup> group
// RETURN:     0 - an error has occured
//			   1 - value successfully returned
// NOTE: - Assignments of type <var> = '<char>' or <var> = 0xhhhhhhhh are also accepted.
//		 In the first case, the ascii code of <char> is returned. In the second case,
//		 h represents a hex-digit and the number of specified hex-digits must be between
//		 1 and 8.
//		 - If the value is enclosed in quotes they are removed.
///////////////////////////////////////////////////////////////////////////////////
int CIniParser::GetVar(const char * p_szGroup, const char * p_szVarName, int * p_pnValue,
					   int p_nPos/* = 0*/, bool p_bLogErrors/* = true*/,
					   const char * p_szSubgroup/* = ""*/)
{
	int nDiff = p_nPos;
    const char * szValue = findVariable( p_szGroup, p_szVarName, nDiff, p_bLogErrors,
										 p_szSubgroup );
    if( szValue )
    {
		if ( *szValue == 0 )
		{
			// value missing
			*p_pnValue = 0;
			if ( p_bLogErrors )
				LOG("[%s].%s is empty, set to 0", m_szCrtGroup, p_szVarName);
			return 1;
		}

		if ( szValue[0] == '\'' && strlen( szValue ) == 3 && szValue[2] == '\'' )
		{
			*p_pnValue = szValue[1];
			return 1;
		}

        return ( sscanf( szValue, "%i", p_pnValue ) == 1 );
	}

	if ( ! p_nPos && p_bLogErrors )
		LOG("Not found: [%s].%s", m_szCrtGroup, p_szVarName);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////
// NAME: GetVar
// AUTHOR: Marius Chile
// DESCRIPTION: gets a value of type float of a variable from the ini file
// PARAMETERS: p_szGroup   - const char * [in] - the group where the variable should be
//			   p_szVarName - const char * [in] - the variable name
//			   p_pfValue    - float *  [in/out] - the location where the variable value
//												 will be held
//			   p_nPos        - int		  [in] - the number of the variable in a
//											     group of variables having the same name
//			   p_bLogErrors - bool		 [in] - write errors to log file
//			   p_szSubgroup - const char * [in] - look for variable in the <p_szSubgroup>
//												  subgroup of the <p_szGroup> group
// RETURN:     0 - an error has occured
//			   1 - value successfully returned
// NOTE: If the value is enclosed in quotes they are removed
///////////////////////////////////////////////////////////////////////////////////
int CIniParser::GetVar(const char * p_szGroup, const char * p_szVarName, float * p_pfValue,
					   int p_nPos/* = 0*/, bool p_bLogErrors/* = true*/,
					   const char * p_szSubgroup/* = ""*/)
{
	int nDiff = p_nPos;
    const char * szValue = findVariable( p_szGroup, p_szVarName, nDiff, p_bLogErrors,
										 p_szSubgroup );
    if( szValue )
    {
		if ( *szValue == 0 )
		{
			// value missing
			*p_pfValue = 0;
			if (p_bLogErrors)
				LOG("[%s].%s is empty, set to 0", m_szCrtGroup, p_szVarName);
			return 1;
		}

        return ( sscanf( szValue, "%f", p_pfValue ) == 1 );
	}

	if ( ! p_nPos && p_bLogErrors )
		LOG("Not found: [%s].%s", m_szCrtGroup, p_szVarName);

	return 0;
}


///////////////////////////////////////////////////////////////////////////////////
// NAME: GetVar
// AUTHOR: Marius Chile
// DESCRIPTION: gets a value of type net_address of a variable from the ini file
// PARAMETERS: p_szGroup     - const char  * [in]     - the group where the variable should be
//			   p_szVarName   - const char  * [in]     - the variable name
//			   p_pNetAddress - net_address * [in/out] - the location where the variable value
//												        will be held
//			   p_nPos        - int		     [in]     - the number of the variable in a
//											            group of variables having the same name
//			   p_bLogErrors - bool		 [in] - write errors to log file
//			   p_szSubgroup - const char * [in] - look for variable in the <p_szSubgroup>
//												  subgroup of the <p_szGroup> group
// RETURN:     0 - an error has occured
//			   1 - value successfully returned
// NOTE: - Line format: var = IP:Port
//		 - Both IP & Port are returned in the struct in network byte order
//		 - If the value is enclosed in quotes they are removed
///////////////////////////////////////////////////////////////////////////////////
int CIniParser::GetVar(const char * p_szGroup, const char * p_szVarName,
					   net_address * p_pNetAddress, int p_nPos/* = 0*/, bool p_bLogErrors/* = true*/,
					   const char * p_szSubgroup/* = ""*/)
{
	int nDiff = p_nPos;
    const char * szValue = findVariable( p_szGroup, p_szVarName, nDiff, p_bLogErrors,
										 p_szSubgroup );
    if( szValue )
    {
		if ( *szValue == 0 )
		{
			// value missing
			p_pNetAddress->m_dwPortNo = 0;
			p_pNetAddress->m_nIP = 0;
			if ( p_bLogErrors )
				LOG("[%s].%s is empty, set to 0", m_szCrtGroup, p_szVarName);
			return 1;
		}

		// ignore quotes
		if ( *szValue == '"' || *szValue == '\'' )
			szValue++;

		char szIPBuf[32];

		if ( sscanf(szValue, "%[^:,] %*[:,] %hd", szIPBuf,
					&p_pNetAddress->m_dwPortNo) >= 2 )
		{
			p_pNetAddress->m_nIP = inet_addr(szIPBuf);
			p_pNetAddress->m_dwPortNo = htons(p_pNetAddress->m_dwPortNo);
			return 1;
		}
		else
		{
			LOG("CIniParser::GetVar: invalid net_address %s", szValue);
		}
	}

	if ( ! p_nPos && p_bLogErrors )
		LOG("Not found: [%s].%s", m_szCrtGroup, p_szVarName);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////
// NAME: GetVar
// AUTHOR: Catrina Mihailescu
// DESCRIPTION: gets an array of int
// PARAMETERS:	p_szGroup     - const char  * [in]	- the group where the variable should be
//		p_szVarName   - const char  * [in]	- the variable name
//		p_pValuesList - list * [in/out]		- the location where the variable value will be held
//	 	p_nPos        - int [in]	     	- the number of the variable in a group of variables having the same name
//		p_bLogErrors  - bool [in] 		- write errors to log file
//		p_szSubgroup  - const char * [in] 	- look for variable in the <p_szSubgroup> subgroup of the <p_szGroup> group
//
// RETURN:     	0 - an error has occured
//		1 - value successfully returned
// NOTE: - Line format: var = value1, value2, value3
///////////////////////////////////////////////////////////////////////////////////
int CIniParser::GetVar(const char * p_szGroup, const char * p_szVarName,
			valuesList * p_oValueList, int p_nPos/* = 0*/, bool p_bLogErrors/* = true*/,
			const char * p_szSubgroup/* = ""*/)
{

 int nDiff = p_nPos;
 const char * szValue = findVariable( p_szGroup, p_szVarName, nDiff, p_bLogErrors, p_szSubgroup );


 if( szValue )
    	{
		if ( *szValue == 0 )
		{
 			// value missing

 			LOG("[%s].%s is empty, no elements in list", m_szCrtGroup, p_szVarName);
 			return 1;
		}

 		// ignore quotes
 		if ( *szValue == '"' || *szValue == '\'' )
 			szValue++;


		int value = 0;

		// number of "," characters in input string
		const char *pch;
		int counter = 0;
		pch = strchr(szValue, ',');
		int iterator =0;
		while(pch!=NULL)
		{
			counter++;
			pch = strchr(pch+1, ',');
		}
		// counter - number of ',' characters in string => List with counter+1 elementes

		if ( sscanf(szValue, "%d %*[:,]", &value) >= 1 )
 		{
			p_oValueList->push_back(value);

 		}
 		else
 		{
 			LOG("CIniParser::GetVar: invalid input %s -- 1", szValue);
			return 0;
 		}

		pch = strchr(szValue, ',');
		for(iterator = 1; iterator < counter+1; iterator ++)
		{

			if ( sscanf(pch+1, "%d %*[:,]", &value) >= 1 )
 			{
				p_oValueList->push_back(value);

 			}
 			else
 			{
 				LOG("CIniParser::GetVar: invalid input %s --2", pch);
				return 0;
 			}

			pch = strchr(pch+1, ',');


		}
	return 1;


 	}
return 0;
}







///////////////////////////////////////////////////////////////////////////////////
// NAME: GetVar
// AUTHOR: Marius Chile
// DESCRIPTION: gets a value of type net_address of a variable from the ini file
// PARAMETERS: p_szGroup     - const char  * [in]     - the group where the variable should be
//			   p_szVarName   - const char  * [in]     - the variable name
//			   TIPv6Address - net_address * [in/out] - the location where the variable value
//												        will be held
//			   p_nPos        - int		     [in]     - the number of the variable in a
//											            group of variables having the same name
//			   p_bLogErrors - bool		 [in] - write errors to log file
//			   p_szSubgroup - const char * [in] - look for variable in the <p_szSubgroup>
//												  subgroup of the <p_szGroup> group
// RETURN:     0 - an error has occured
//			   1 - value successfully returned
// NOTE: - Line format: var = IP:Port
//		 - Both IP & Port are returned in the struct in network byte order
//		 - If the value is enclosed in quotes they are removed
///////////////////////////////////////////////////////////////////////////////////
int CIniParser::GetVar(const char * p_szGroup, const char * p_szVarName,
					   TIPv6Address * p_pNetAddress, int p_nPos/* = 0*/, bool p_bLogErrors/* = true*/,
					   const char * p_szSubgroup/* = ""*/)
{
	int nDiff = p_nPos;
	const char * szValue = findVariable( p_szGroup, p_szVarName, nDiff, p_bLogErrors, p_szSubgroup );
	if( szValue )
	{
		if ( *szValue == 0 )
		{
			// value missing
			memset(p_pNetAddress->m_pu8RawAddress,0,sizeof(p_pNetAddress->m_pu8RawAddress));
			if ( p_bLogErrors )
				LOG("[%s].%s is empty, set to 0", m_szCrtGroup, p_szVarName);
			return 1;
		}

		//use temporary buffer so the input buffer will not change on invalid IPv6 address
		unsigned short pusPieces[8];

		if( sscanf(szValue,"%04hx:%04hx:%04hx:%04hx:%04hx:%04hx:%04hx:%04hx",
							pusPieces + 0, pusPieces + 1,
							pusPieces + 2, pusPieces + 3,
							pusPieces + 4, pusPieces + 5,
							pusPieces + 6, pusPieces + 7
							) < 8 )
		{
			if ( p_bLogErrors )
				LOG("[%s].%s invalid IPv6 format", m_szCrtGroup, p_szVarName);
			return 0;
		}
		unsigned short* pusIPv6Address = (unsigned short*) p_pNetAddress->m_pu8RawAddress;
		int i;
		for ( i = 0; i < 8; i++ )
		{
			pusIPv6Address[i] = htons(pusPieces[i]);
		}
		return 1;
	}

	if ( ! p_nPos && p_bLogErrors )
		LOG("Not found: [%s].%s", m_szCrtGroup, p_szVarName);

	return 0;
}


///////////////////////////////////////////////////////////////////////////////////
// NAME: createVariable
// AUTHOR: Marius Chile
// DESCRIPTION: write a new variable in the ini file
// PARAMETERS: p_szGroup    - const char * [in] - the group where the variable will be
//			   p_szAttrib   - const char * [in] - the assignment string
//			   p_bLogErrors - bool		   [in] - write errors to log file
//			   p_szSubgroup - const char * [in] - the subgroup where the variable will be
// RETURN:     0 - an error has occured
//			   1 - variable successfully created
// NOTE: If the group is not found, it's created. Same for the subgroup.
///////////////////////////////////////////////////////////////////////////////////
int CIniParser::createVariable(const char * p_szGroup, const char * p_szAttrib,
							   bool p_bLogErrors/* = true*/, const char * p_szSubgroup/* = ""*/)
{
	const char* szNewLine = "";
	if ( ! m_lGroupStart )
	{
		// group does not exist
		if ( ! CreateGroup(p_szGroup, p_bLogErrors) )
			return 0;
		m_lValueFilePos = m_lGroupStart;
	}
	else
	{
		// if the group is on the last line of the file,
		// add a new line before the new variable
		fseek(m_fpIniFile, m_lGroupStart, SEEK_SET);
		if ( ! moveToGroupEnd(&m_lValueFilePos, true) )
			szNewLine = EOL;
	}

	if ( ! p_szSubgroup || *p_szSubgroup )
	{
		if ( m_lSubgroupStart == m_lGroupStart )
		{
			// subgroup does not exist
			if ( ! CreateSubgroup(p_szGroup, p_szSubgroup, p_bLogErrors) )
				return 0;
			szNewLine = "";
			m_lValueFilePos = m_lSubgroupStart;
		}
		else
		{
			// if the group is on the last line of the file,
			// add a new line before the new variable
			fseek(m_fpIniFile, m_lSubgroupStart, SEEK_SET);
			if ( ! moveToGroupEnd(&m_lValueFilePos, true) )
				szNewLine = EOL;
		}
	}

	snprintf(m_szLine, MAX_LINE_LEN, "%s%s%s", szNewLine, p_szAttrib, EOL);
	return replaceValue("", m_szLine, strlen(m_szLine), p_bLogErrors);
}

///////////////////////////////////////////////////////////////////////////////////
// NAME: replaceValue
// AUTHOR: Marius Chile
// DESCRIPTION: replaces a value with a new one in the ini file
// PARAMETERS: p_szOldValue - const char * [in] - the old variable value
//			   p_szNewVal   - const char * [in] - the new variable value
//			   p_nNewValLen - int		   [in] - the new variable value length
//			   p_bLogErrors - bool		   [in] - write errors to log file
// RETURN:  0 - an error occured
//			1 - value successfully replaced
///////////////////////////////////////////////////////////////////////////////////
int CIniParser::replaceValue(const char * p_szOldValue, const char * p_szNewVal,
							 unsigned int p_nNewValLen, bool p_bLogErrors/* = true*/)
{
	// optimize for equal length
	unsigned int lOldValLen = strlen(p_szOldValue);

	if (p_nNewValLen == lOldValLen)
	{
		// move back to the start of the found value
		fseek(m_fpIniFile, m_lValueFilePos, SEEK_SET);

		// just overwrite
		if (fwrite(p_szNewVal, 1, p_nNewValLen, m_fpIniFile) < p_nNewValLen)
		{
			if (p_bLogErrors)
				LOG_ERR("CIniParser::replaceValue - could not overwrite value %s",
						p_szOldValue);
			return 0;
		}
	}
	else
	if (p_nNewValLen < lOldValLen)
	{
		char szBuff[MAX_LINE_LEN];

		memset(szBuff,' ', sizeof(szBuff));
		szBuff[sizeof(szBuff)-1] = 0;

		memcpy(szBuff, p_szNewVal, strlen(p_szNewVal));


		// move back to the start of the found value
		fseek(m_fpIniFile, m_lValueFilePos, SEEK_SET);
		if (fwrite(szBuff, 1, lOldValLen, m_fpIniFile) < lOldValLen)
		{
			if (p_bLogErrors)
				LOG_ERR("CIniParser::replaceValue - could not overwrite value %s",
						p_szOldValue);
			return 0;
		}
	}
	else
	//if (p_nNewValLen > lOldValLen)
	{
		long lBackupSize;

		// backup the rest of the file forwards excluding the old value
		fseek(m_fpIniFile, 0, SEEK_END);
		lBackupSize = ftell(m_fpIniFile) - m_lValueFilePos - lOldValLen;

		if (lBackupSize <= 0)
		{
			// nothing after the old value, just write the new value
			fseek(m_fpIniFile, m_lValueFilePos, SEEK_SET);
			if (fwrite(p_szNewVal, 1, p_nNewValLen, m_fpIniFile) < p_nNewValLen)
			{
				if (p_bLogErrors)
					LOG_ERR("CIniParser::replaceValue - could not overwrite value %s",
							p_szOldValue);
				return 0;
			}
		}
		else
		{
			size_t nRead;
			char* szBackup = new char[lBackupSize];
			if (!szBackup)
			{
				if (p_bLogErrors)
					LOG("CIniParser::replaceValue - could not allocate %d for backup of the data after the old value %s",
						lBackupSize, p_szOldValue);
				return 0;
			}

			fseek(m_fpIniFile, m_lValueFilePos + lOldValLen, SEEK_SET);
			nRead = fread(szBackup, lBackupSize, 1, m_fpIniFile);

			// write the new value and then the backed up data
			fseek(m_fpIniFile, m_lValueFilePos, SEEK_SET);
			if (fwrite(p_szNewVal, 1, p_nNewValLen, m_fpIniFile) < p_nNewValLen)
			{
				if (p_bLogErrors)
					LOG_ERR("CIniParser::replaceValue - could not overwrite value %s",
							p_szOldValue);
				delete [] szBackup;
				return 0;
			}
			if (nRead == 0)
			{
				if (p_bLogErrors)
					LOG_ERR("CIniParser::replaceValue - could not read the data after the old value %s",
							p_szOldValue);
				delete [] szBackup;
				return 0;
			}
			if (fwrite(szBackup, lBackupSize, 1, m_fpIniFile) < 1)
			{
				if (p_bLogErrors)
					LOG_ERR("CIniParser::replaceValue - could not write the backup of the data after the old value %s",
							p_szOldValue);
				delete [] szBackup;
				return 0;
			}

			delete [] szBackup;
		}
	}

	return 1;
}

///////////////////////////////////////////////////////////////////////////////////
// NAME: SetVar
// AUTHOR: Marius Chile
// DESCRIPTION: sets a value of type octet-string of a variable from the ini file
// PARAMETERS: p_szGroup    - const char * [in] - the group where the variable should be
//			   p_szVarName  - const char * [in] - the variable name
//			   p_szValue    - const char * [in] - the variable value
//			   p_nPos       - int		   [in] - the number of the variable in a
//											      group of variables having the same name
//			   p_bCreateIfNotFound - bool  [in] - if the variable is missing, it is created
//			   p_bLogErrors - bool		   [in] - write errors to log file
//			   p_szSubgroup - const char * [in] - the subgroup where the variable should be
// RETURN:     0 - an error has occured
//			   1 - variable successfully set
// NOTE: If <p_bCreateIfNotFound> = true, then if the group is not found, it's created.
//		 Same for the subgroup.
///////////////////////////////////////////////////////////////////////////////////
int CIniParser::SetVar(const char * p_szGroup, const char * p_szVarName,
					   const char * p_szValue, int p_nPos/* = 0*/,
					   bool p_bCreateIfNotFound/* = false*/, bool p_bLogErrors/* = true*/,
					   const char * p_szSubgroup/* = ""*/)
{
    int nDiff = p_nPos;
    const char * szValue = findVariable( p_szGroup, p_szVarName, nDiff,
										 p_bLogErrors, p_szSubgroup );
    if( szValue )
    {
		return replaceValue(szValue, p_szValue, strlen(p_szValue), p_bLogErrors);
	}
	else
	{
		// value does not exist
		if ( ! p_bCreateIfNotFound )
			return 0;

		char * szAttrib = new char[MAX_LINE_LEN];
		snprintf(szAttrib, MAX_LINE_LEN, "%s = ", p_szVarName);

		int nRet = 1;
		for ( ; nDiff > 0 && nRet ; nDiff-- )
			nRet = createVariable(p_szGroup, szAttrib, p_bLogErrors, p_szSubgroup);

		if (nRet)
		{
			snprintf(szAttrib, MAX_LINE_LEN, "%s = %s", p_szVarName, p_szValue);
			nRet = createVariable(p_szGroup, szAttrib, p_bLogErrors, p_szSubgroup);
		}

		delete [] szAttrib;
		return nRet;
	}
}

///////////////////////////////////////////////////////////////////////////////////
// NAME: SetVarRawString
// AUTHOR: Mihai Buha
// DESCRIPTION: sets a value of type octet-string of a variable from the ini file, ignoring the # chars.
// PARAMETERS: p_szGroup    - const char * [in] - the group where the variable should be
//			   p_szVarName  - const char * [in] - the variable name
//			   p_szValue    - const char * [in] - the variable value
//			   p_nPos       - int		   [in] - the number of the variable in a
//											      group of variables having the same name
//			   p_bCreateIfNotFound - bool  [in] - if the variable is missing, it is created
//			   p_bLogErrors - bool		   [in] - write errors to log file
//			   p_szSubgroup - const char * [in] - the subgroup where the variable should be
// RETURN:     0 - an error has occured
//			   1 - variable successfully set
// NOTE: If <p_bCreateIfNotFound> = true, then if the group is not found, it's created.
//		 Same for the subgroup.
// This function is derived from the regular SetVar
///////////////////////////////////////////////////////////////////////////////////
int CIniParser::SetVarRawString(const char * p_szGroup, const char * p_szVarName,
					   const char * p_szValue, int p_nPos/* = 0*/,
					   bool p_bCreateIfNotFound/* = false*/, bool p_bLogErrors/* = true*/,
					   const char * p_szSubgroup/* = ""*/)
{
	int nDiff = p_nPos;
    const char * szValue = findVariableRaw( p_szGroup, p_szVarName, nDiff,
										 p_bLogErrors, p_szSubgroup );
    if( szValue )
    {
		return replaceValue(szValue, p_szValue, strlen(p_szValue), p_bLogErrors);
	}
	else
	{
		// value does not exist
		if ( ! p_bCreateIfNotFound )
			return 0;

		char * szAttrib = new char[MAX_LINE_LEN];
		snprintf(szAttrib, MAX_LINE_LEN, "%s = ", p_szVarName);

		int nRet = 1;
		for ( ; nDiff > 0 && nRet ; nDiff-- )
			nRet = createVariable(p_szGroup, szAttrib, p_bLogErrors, p_szSubgroup);

		if (nRet)
		{
			snprintf(szAttrib, MAX_LINE_LEN, "%s = %s", p_szVarName, p_szValue);
			nRet = createVariable(p_szGroup, szAttrib, p_bLogErrors, p_szSubgroup);
		}

		delete [] szAttrib;
		return nRet;
	}
}

///////////////////////////////////////////////////////////////////////////////////
// NAME: SetVarQuotes
// AUTHOR: Claudiu Hobeanu
// DESCRIPTION: sets a value of type octet-string of a variable from the ini file
// PARAMETERS: p_szGroup    - const char * [in] - the group where the variable should be
//			   p_szVarName  - const char * [in] - the variable name
//			   p_szValue    - const char * [in] - the variable value
//			   p_nPos       - int		   [in] - the number of the variable in a
//											      group of variables having the same name
//			   p_bCreateIfNotFound - bool  [in] - if the variable is missing, it is created
//			   p_bLogErrors - bool		   [in] - write errors to log file
//			   p_szSubgroup - const char * [in] - the subgroup where the variable should be
// RETURN:     0 - an error has occured
//			   1 - variable successfully set
// NOTE: If <p_bCreateIfNotFound> = true, then if the group is not found, it's created.
//		 Same for the subgroup.
///////////////////////////////////////////////////////////////////////////////////
int CIniParser::SetVarQuotes(const char * p_szGroup, const char * p_szVarName,
					   const char * p_szValue, int p_nPos/* = 0*/,
					   bool p_bCreateIfNotFound/* = false*/, bool p_bLogErrors/* = true*/,
					   const char * p_szSubgroup/* = ""*/)
{
	int nLen = strlen(p_szValue);

	char* szValueQuotes = new char[nLen+7]; //just in case
	if (!szValueQuotes)
	{	LOG_ERR("CIniParser::SetVarQuotes : can't allocate %d bytes", nLen + 3 );
		return 0;
	}

	sprintf(szValueQuotes, "\"%s\"", p_szValue );

	int ret = SetVarRawString( p_szGroup, p_szVarName, szValueQuotes, p_nPos, p_bCreateIfNotFound,
							p_bLogErrors, p_szSubgroup );
	delete[] szValueQuotes;
	return ret;
}


///////////////////////////////////////////////////////////////////////////////////
// NAME: SetVar
// AUTHOR: Marius Chile
// DESCRIPTION: sets a value of type hex-octet-string of a variable from the ini file
// PARAMETERS: p_szGroup   - const char * [in] - the group where the variable should be
//			   p_szVarName - const char * [in] - the variable name
//			   p_pValue    - const unsigned char * [in] - the variable value
//			   p_nValueSize - int		  [in] - the p_pValue size in bytes
//			   p_nPos      - int		  [in] - the number of the variable in a
//											     group of variables having the same name
//			   p_bCreateIfNotFound - bool [in] - if the variable is missing, it is created
//			   p_bLogErrors - bool		   [in] - write errors to log file
//			   p_szSubgroup - const char * [in] - the subgroup where the variable should be
// RETURN:     0 - an error has occured
//			   1 - variable successfully set
// NOTE: If <p_bCreateIfNotFound> = true, then if the group is not found, it's created.
//		 Same for the subgroup.
///////////////////////////////////////////////////////////////////////////////////
int CIniParser::SetVar(const char * p_szGroup, const char * p_szVarName,
					   const unsigned char * p_pValue, int p_nValueSize, int p_nPos/* = 0*/,
					   bool p_bCreateIfNotFound/* = false*/, bool p_bLogErrors/* = true*/,
					   const char * p_szSubgroup/* = ""*/)
{
	int nDiff = p_nPos;
    const char * szValue = findVariable( p_szGroup, p_szVarName, nDiff,
										 p_bLogErrors, p_szSubgroup );
	char * pNewValue = new char[MAX_LINE_LEN];
	int nRet = 1;
	int nPrinted = 0;

	if ( ! szValue )
	{
		// value does not exist
		if ( ! p_bCreateIfNotFound )
		{
			delete[] pNewValue;
			return 0;
		}

		nPrinted = snprintf(pNewValue, MAX_LINE_LEN, "%s = ", p_szVarName);
	}

	for ( ; nDiff > 0 && nRet ; nDiff-- )
		nRet = createVariable(p_szGroup, pNewValue, p_bLogErrors, p_szSubgroup);

	if ( ! nRet )
	{
		delete[] pNewValue;
		return 0;
	}

#ifndef TEST_FIPS
	// for each byte of the new value we will write 2 chars representing that byte
	// and a separating space, so check if the new string size is within limits
	if (nPrinted + 3 * p_nValueSize + 1 >= MAX_LINE_LEN)
#else
	if (nPrinted + 2 * p_nValueSize + 1 >= MAX_LINE_LEN)
#endif
	{
		if (p_bLogErrors)
			LOG("CIniParser::SetVar - the new value for variable %s exceeds maximum line length", p_szVarName);
		delete[] pNewValue;
		return 0;
	}

	char * pTmp = pNewValue + nPrinted;
	char szTmp[4];
	for ( int i = 0 ; i < p_nValueSize ; i++ )
	{
#ifndef TEST_FIPS
		snprintf(szTmp, 4, " %2.2x", p_pValue[i]);
		memcpy(pTmp, szTmp, 3);
		pTmp += 3;
#else
		snprintf(szTmp, 3, "%2.2x", p_pValue[i]);
		memcpy(pTmp, szTmp, 2);
		pTmp += 2;
#endif
	}

    if( szValue )
#ifndef TEST_FIPS
		nRet = replaceValue(szValue, pNewValue, 3 * p_nValueSize, p_bLogErrors);
#else
		nRet = replaceValue(szValue, pNewValue, 2 * p_nValueSize, p_bLogErrors);
#endif
	else
	{
		*pTmp = 0;
		nRet = createVariable(p_szGroup, pNewValue, p_bLogErrors, p_szSubgroup);
	}

	delete [] pNewValue;
	return nRet;
}

///////////////////////////////////////////////////////////////////////////////////
// NAME: SetVar
// AUTHOR: Marius Chile
// DESCRIPTION: sets a value of type int of a variable from the ini file
// PARAMETERS: p_szGroup   - const char * [in] - the group where the variable should be
//			   p_szVarName - const char * [in] - the variable name
//			   p_nValue    - int          [in] - the variable value
//			   p_nPos      - int		  [in] - the number of the variable in a
//											     group of variables having the same name
//			   p_bCreateIfNotFound - bool [in] - if the variable is missing, it is created
//			   p_bLogErrors - bool		   [in] - write errors to log file
//			   p_szSubgroup - const char * [in] - the subgroup where the variable should be
// RETURN:     0 - an error has occured
//			   1 - variable successfully set
// NOTE: If <p_bCreateIfNotFound> = true, then if the group is not found, it's created.
//		 Same for the subgroup.
///////////////////////////////////////////////////////////////////////////////////
int CIniParser::SetVar(const char * p_szGroup, const char * p_szVarName, int p_nValue,
					   int p_nPos/* = 0*/, bool p_bCreateIfNotFound/* = false*/,
					   bool p_bLogErrors/* = true*/, const char * p_szSubgroup/* = ""*/)
{
	int nDiff = p_nPos;
    const char * szValue = findVariable( p_szGroup, p_szVarName, nDiff,
										 p_bLogErrors, p_szSubgroup );
    if( szValue )
    {
		char szBuf[32];
		int nWritten = snprintf(szBuf, 32, "%d", p_nValue);
		return replaceValue(szValue, szBuf, nWritten, p_bLogErrors);
	}
	else
	{
		// value does not exist
		if ( ! p_bCreateIfNotFound )
			return 0;

		char * szAttrib = new char[MAX_LINE_LEN];
		snprintf(szAttrib, MAX_LINE_LEN, "%s = ", p_szVarName);

		int nRet = 1;
		for ( ; nDiff > 0 && nRet ; nDiff-- )
			nRet = createVariable(p_szGroup, szAttrib, p_bLogErrors, p_szSubgroup);

		if (nRet)
		{
			snprintf(szAttrib, MAX_LINE_LEN, "%s = %d", p_szVarName, p_nValue);
			nRet = createVariable(p_szGroup, szAttrib, p_bLogErrors, p_szSubgroup);
		}

		delete [] szAttrib;
		return nRet;
	}
}


///////////////////////////////////////////////////////////////////////////////////
// NAME: SetVar
// AUTHOR: Marius Chile
// DESCRIPTION: sets a value of type float of a variable from the ini file
// PARAMETERS: p_szGroup   - const char * [in] - the group where the variable should be
//			   p_szVarName - const char * [in] - the variable name
//			   p_fValue    - float        [in] - the variable value
//			   p_nPos      - int		  [in] - the number of the variable in a
//											     group of variables having the same name
//			   p_bCreateIfNotFound - bool [in] - if the variable is missing, it is created
//			   p_bLogErrors - bool		   [in] - write errors to log file
//			   p_szSubgroup - const char * [in] - the subgroup where the variable should be
// RETURN:     0 - an error has occured
//			   1 - variable successfully set
// NOTE: If <p_bCreateIfNotFound> = true, then if the group is not found, it's created.
//		 Same for the subgroup.
///////////////////////////////////////////////////////////////////////////////////
int CIniParser::SetVar(const char * p_szGroup, const char * p_szVarName, float p_fValue,
					   int p_nPos/* = 0*/, bool p_bCreateIfNotFound/* = false*/,
					   bool p_bLogErrors/* = true*/, const char * p_szSubgroup/* = ""*/)
{
	int nDiff = p_nPos;
    const char * szValue = findVariable( p_szGroup, p_szVarName, nDiff,
										 p_bLogErrors, p_szSubgroup );
    if( szValue )
    {
		char szBuf[32];
		int nWritten = snprintf(szBuf, 32, "%f", p_fValue);
		return replaceValue(szValue, szBuf, nWritten, p_bLogErrors);
	}
	else
	{
		// value does not exist
		if ( ! p_bCreateIfNotFound )
			return 0;

		char * szAttrib = new char[MAX_LINE_LEN];
		snprintf(szAttrib, MAX_LINE_LEN, "%s = ", p_szVarName);

		int nRet = 1;
		for ( ; nDiff > 0 && nRet ; nDiff-- )
			nRet = createVariable(p_szGroup, szAttrib, p_bLogErrors, p_szSubgroup);

		if (nRet)
		{
			snprintf(szAttrib, MAX_LINE_LEN, "%s = %f", p_szVarName, p_fValue);
			nRet = createVariable(p_szGroup, szAttrib, p_bLogErrors, p_szSubgroup);
		}

		delete [] szAttrib;
		return nRet;
	}
}

///////////////////////////////////////////////////////////////////////////////////
// NAME: SetVar
// AUTHOR: Marius Chile
// DESCRIPTION: sets a value of type net_address of a variable from the ini file
// PARAMETERS: p_szGroup     - const char  * [in]     - the group where the variable should be
//			   p_szVarName   - const char  * [in]     - the variable name
//			   p_rNetAddress - const net_address & [in] - the location where the variable value
//												          will be held
//			   nPos          - int		     [in]     - the number of the variable in a
//											            group of variables having the same name
//			   p_bCreateIfNotFound - bool [in] - if the variable is missing, it is created
//			   p_bLogErrors - bool		   [in] - write errors to log file
//			   p_szSubgroup - const char * [in] - the subgroup where the variable should be
// RETURN:     0 - an error has occured
//			   1 - value successfully returned
// NOTE: - Line format: var = IP:Port
//		 - Both IP & Port struct members are expected to be in network byte order
//		 - If <p_bCreateIfNotFound> = true, then if the group is not found, it's created.
//		   Same for the subgroup.
///////////////////////////////////////////////////////////////////////////////////
int CIniParser::SetVar(const char * p_szGroup, const char * p_szVarName,
					   const net_address & p_rNetAddress, int p_nPos/* = 0*/,
					   bool p_bCreateIfNotFound/* = false*/, bool p_bLogErrors/* = true*/,
					   const char * p_szSubgroup/* = ""*/)
{
	int nDiff = p_nPos;
    const char * szValue = findVariable( p_szGroup, p_szVarName, nDiff,
										 p_bLogErrors, p_szSubgroup );
    if( szValue )
    {
		char szBuf[32];
		int nWritten = snprintf(szBuf, 32, "%ld.%ld.%ld.%ld:%d",
							    p_rNetAddress.m_nIP & 0xff,
							    (p_rNetAddress.m_nIP & 0xff00) >> 8,
							    (p_rNetAddress.m_nIP & 0xff0000) >> 16,
							    (p_rNetAddress.m_nIP & 0xff000000) >> 24,
							    ntohs(p_rNetAddress.m_dwPortNo));
		return replaceValue(szValue, szBuf, nWritten, p_bLogErrors);
	}
	else
	{
		// value does not exist
		if ( ! p_bCreateIfNotFound )
			return 0;

		char * szAttrib = new char[MAX_LINE_LEN];
		snprintf(szAttrib, MAX_LINE_LEN, "%s = ", p_szVarName);

		int nRet = 1;
		for ( ; nDiff > 0 && nRet ; nDiff-- )
			nRet = createVariable(p_szGroup, szAttrib, p_bLogErrors, p_szSubgroup);

		if (nRet)
		{
			snprintf(szAttrib, MAX_LINE_LEN, "%s = %ld.%ld.%ld.%ld:%d", p_szVarName,
					 p_rNetAddress.m_nIP & 0xff,
					 (p_rNetAddress.m_nIP & 0xff00) >> 8,
					 (p_rNetAddress.m_nIP & 0xff0000) >> 16,
					 (p_rNetAddress.m_nIP & 0xff000000) >> 24,
					 ntohs(p_rNetAddress.m_dwPortNo));
			nRet = createVariable(p_szGroup, szAttrib, p_bLogErrors, p_szSubgroup);
		}

		delete [] szAttrib;
		return nRet;
	}
}

///////////////////////////////////////////////////////////////////////////////////
// NAME: SetVar
// AUTHOR: Claudiu Hobeanu
// DESCRIPTION: sets a value of type net_address of a variable from the ini file
// PARAMETERS: p_szGroup     - const char  * [in]     - the group where the variable should be
//			   p_szVarName   - const char  * [in]     - the variable name
//			   p_rNetAddress - const TIPv6Address & [in] - the location where the variable value
//												          will be held
//			   nPos          - int		     [in]     - the number of the variable in a
//											            group of variables having the same name
//			   p_bCreateIfNotFound - bool [in] - if the variable is missing, it is created
//			   p_bLogErrors - bool		   [in] - write errors to log file
//			   p_szSubgroup - const char * [in] - the subgroup where the variable should be
// RETURN:     0 - an error has occured
//			   1 - value successfully returned
// NOTE: - Line format: var = IP:Port
//		 - Both IP & Port struct members are expected to be in network byte order
//		 - If <p_bCreateIfNotFound> = true, then if the group is not found, it's created.
//		   Same for the subgroup.
///////////////////////////////////////////////////////////////////////////////////
int CIniParser::SetVar(const char * p_szGroup, const char * p_szVarName,
					   const TIPv6Address & p_rNetAddress, int p_nPos/* = 0*/,
					   bool p_bCreateIfNotFound/* = false*/, bool p_bLogErrors/* = true*/,
					   const char * p_szSubgroup/* = ""*/)
{
	int nDiff = p_nPos;
	const char * szValue = findVariable( p_szGroup, p_szVarName, nDiff,
		p_bLogErrors, p_szSubgroup );
	if( szValue )
	{
		char szBuf[MAX_LINE_LEN];
		int nWritten = p_rNetAddress.FillIPv6(szBuf, sizeof(szBuf));

		return replaceValue(szValue, szBuf, nWritten, p_bLogErrors);
	}
	else
	{
		// value does not exist
		if ( ! p_bCreateIfNotFound )
			return 0;

		char * szAttrib = new char[MAX_LINE_LEN];
		snprintf(szAttrib, MAX_LINE_LEN, "%s = ", p_szVarName);

		int nRet = 1;
		for ( ; nDiff > 0 && nRet ; nDiff-- )
			nRet = createVariable(p_szGroup, szAttrib, p_bLogErrors, p_szSubgroup);

		if (nRet)
		{
			int nWritten = snprintf(szAttrib, MAX_LINE_LEN, "%s = ", p_szVarName);
			p_rNetAddress.FillIPv6(szAttrib + nWritten,MAX_LINE_LEN-nWritten);

			nRet = createVariable(p_szGroup, szAttrib, p_bLogErrors, p_szSubgroup);
		}

		delete [] szAttrib;
		return nRet;
	}
}




////////////////////////////////////////////////////////////////////////////////
// NAME: DeleteGroup
// AUTHOR: Marius Negreanu
// DESCRIPTION: delete a group
// PARAMETERS: p_szGroup    - const char  * [in] - the group to be deleted
//             p_bLogErrors - bool          [in] - write errors to log file
// RETURN:     false - an error has occured
//             true - value successfully deleted
////////////////////////////////////////////////////////////////////////////////
bool CIniParser::DeleteGroup(const char * p_szGroup, bool p_bLogErrors /*= true*/)
{
	const char* rv;
	int ret;
	char *it, *end;
	long eraseStart, eraseEnd ;

	rv = FindGroup(p_szGroup, p_bLogErrors);
	if ( !rv || rv==(char*)1 )
	{
		LOG("Group [%s] not found", p_szGroup );
		return false;
	}
	//go back until we hit GROUP_NAME_BEGIN_CHAR
	ret = fseek( m_fpIniFile, -_Min(m_lGroupStart, MAX_LINE_LEN), SEEK_CUR);
	if ( -1 == ret ) { LOG("CIniParser:fseek backward failed") ; return 0 ; }

	ret = fread( m_szLine, sizeof(char), _Min(m_lGroupStart, MAX_LINE_LEN), m_fpIniFile) ;
	if ( -1 == ret ) { LOG("CIniParser:fread failed") ; return 0 ; }
	m_szLine[ret] = 0;

	it=end = m_szLine + strlen(m_szLine) ;
	while ( it && it != m_szLine && *it != GROUP_NAME_BEGIN_CHAR )  --it ;
	// Get the start offset
	if ( *it == GROUP_NAME_BEGIN_CHAR )
	{
		eraseStart = ftell( m_fpIniFile ) - (end-it) ;
	}
	else
	{
		FLOG("Unable to find group start");
		return false ;
	}
	// Get the end offset
	moveToGroupEnd( &eraseEnd ) ;

	fbmove( m_fpIniFile, eraseStart, eraseEnd, 0 );
	ftruncate( fileno(m_fpIniFile), ftell(m_fpIniFile) ) ;
	fflush(m_fpIniFile) ;
	return true ;
}

////////////////////////////////////////////////////////////////////////////////
// NAME: DeleteVar
// AUTHOR: Marius Negreanu
// DESCRIPTION: delete a variable
// PARAMETERS: p_szGroup    - const char* [in] - the group to be deleted
//             p_szVarName  - const char* [in] - variable's name
//             p_nPos       - int         [in] - the number of the variable in a
//             p_bLogErrors - bool        [in] - write errors to log file
//             p_szSubgroup - const char* [in] - the subgroup where the variable should be
// RETURN:     0 - ERROR has occured
//             1 - value successfully returned
////////////////////////////////////////////////////////////////////////////////
int CIniParser::DeleteVar(const char * p_szGroup, const char * p_szVarName
			, int p_nGroupPos/*=0*/, int p_nPos/* = 0*/
			, bool p_bLogErrors/*= true*/, const char * p_szSubgroup/*= ""*/)
{
	int nDiff = p_nPos, ret ;
	char *it, *end ;
	long eraseStart(0), eraseEnd ;

	if ( ! FindGroup(p_szGroup, false, p_bLogErrors) )
	{
		return 0 ;
	}
	for ( ; p_nGroupPos>0; --p_nGroupPos)
	{
		if ( ! FindGroup(p_szGroup, true, p_bLogErrors) )
		{
			return 0 ;
		}
	}

	const char * szValue = findVariable( NULL, p_szVarName, nDiff,
			p_bLogErrors, p_szSubgroup );
	if ( ! szValue )
		return 1 ;

	// m_lValueFilePos points right after the '=' sign
	// Read backwards to scan for \n or ]
	if ( m_lValueFilePos <= MAX_LINE_LEN )
		ret = fseek( m_fpIniFile, 0, SEEK_SET);
	else
		ret = fseek( m_fpIniFile, m_lValueFilePos-MAX_LINE_LEN, SEEK_SET);
	if ( -1 == ret ){FPERR() ;return 0 ;}

	ret = fread( m_szLine, sizeof(char), _Min(m_lValueFilePos, MAX_LINE_LEN) , m_fpIniFile) ;
	m_szLine[ret] = 0 ;

	it=end=m_szLine + strlen(m_szLine) ;

	while ( it && it!=m_szLine && *it!='\n' && *it!=']' ) --it ;
	if ( *it == '\n' || *it==']' )
	{
		// the only case(I can think of) when eraseStart is not obtained from ftell
		// is when we're trying to delete a variable on the first line.
		eraseStart = ftell( m_fpIniFile ) - (end-it) +1/*\n or ]*/;
	}

	// Read ahead MAX_LINE_LEN to scan for EOL (\n)
	ret = fread( m_szLine, sizeof(char), MAX_LINE_LEN, m_fpIniFile) ;
	m_szLine[ret]=0;
	for (  ret=0; m_szLine[ret]!='\n' && ret< MAX_LINE_LEN;++ret) ;
	eraseEnd = m_lValueFilePos+ret+1;

	// IF is EOF THEN adjust eraseEnd
	fseek(m_fpIniFile, 0, SEEK_END);
	int fileSize = ftell(m_fpIniFile);
	if (eraseEnd >= fileSize) 
		eraseEnd = fileSize - 1;

	fbmove( m_fpIniFile, eraseStart, eraseEnd, 0 );
	ftruncate( fileno(m_fpIniFile), ftell(m_fpIniFile) ) ;
	fflush(m_fpIniFile);
	return 1;
}

#ifndef WIN32
#include <unistd.h>

//
// Description: Altered functionality of CIniParser::ReplaceValue.
//   When new value is of size less than that of old value, do not
//   just write a # after the new value.
//
//
int CIniParserPlus::replaceValue(const char * p_szOldValue, const char * p_szNewVal,
							 unsigned int p_nNewValLen, bool p_bLogErrors/* = true*/)
{
	// optimize for equal length
	unsigned int lOldValLen = strlen(p_szOldValue);

	if (p_nNewValLen == lOldValLen)
	{
		// move back to the start of the found value
		fseek(m_fpIniFile, m_lValueFilePos, SEEK_SET);

		// just overwrite
		if (fwrite(p_szNewVal, 1, p_nNewValLen, m_fpIniFile) < p_nNewValLen)
		{
			if (p_bLogErrors)
				LOG_ERR("CIniParser::replaceValue - could not overwrite value %s",
						p_szOldValue);
			return 0;
		}
	}
	else
	//if (p_nNewValLen > lOldValLen)
	{
		long lBackupSize;

		// backup the rest of the file forwards excluding the old value
		fseek(m_fpIniFile, 0, SEEK_END);
		lBackupSize = ftell(m_fpIniFile) - m_lValueFilePos - lOldValLen;

		if (lBackupSize <= 0)
		{
			// nothing after the old value, just write the new value
			fseek(m_fpIniFile, m_lValueFilePos, SEEK_SET);
			if (fwrite(p_szNewVal, 1, p_nNewValLen, m_fpIniFile) < p_nNewValLen)
			{
				if (p_bLogErrors)
					LOG_ERR("CIniParser::replaceValue - could not overwrite value %s",
							p_szOldValue);
				return 0;
			}
		}
		else
		{
			size_t nRead;
			char* szBackup = new char[lBackupSize];
			if (!szBackup)
			{
				if (p_bLogErrors)
					LOG("CIniParser::replaceValue - could not allocate %d for backup of the data after the old value %s",
						lBackupSize, p_szOldValue);
				return 0;
			}

			fseek(m_fpIniFile, m_lValueFilePos + lOldValLen, SEEK_SET);
			nRead = fread(szBackup, lBackupSize, 1, m_fpIniFile);

			// write the new value and then the backed up data
			fseek(m_fpIniFile, m_lValueFilePos, SEEK_SET);
			if (fwrite(p_szNewVal, 1, p_nNewValLen, m_fpIniFile) < p_nNewValLen)
			{
				if (p_bLogErrors)
					LOG_ERR("CIniParser::replaceValue - could not overwrite value %s",
							p_szOldValue);
				delete [] szBackup;
				return 0;
			}
			if (nRead == 0)
			{
				if (p_bLogErrors)
					LOG_ERR("CIniParser::replaceValue - could not read the data after the old value %s",
							p_szOldValue);
				delete [] szBackup;
				return 0;
			}
			if (fwrite(szBackup, lBackupSize, 1, m_fpIniFile) < 1)
			{
				if (p_bLogErrors)
					LOG_ERR("CIniParser::replaceValue - could not write the backup of the data after the old value %s",
							p_szOldValue);
				delete [] szBackup;
				return 0;
			}

            // Truncate the rest of the file
            if (ftruncate( fileno(m_fpIniFile), ftell(m_fpIniFile) ))
            {
                if (p_bLogErrors)
                    LOG_ERR("CIniParserPlus::replaceValue - could not truncate file");
                delete[] szBackup;
                return 0;
            }

			delete [] szBackup;
		}
	}

	return 1;
}
#endif
