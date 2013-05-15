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

// CfgBuffVar.h: interface for the CCfgBuffVar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CFGBUFFVAR_H__8B3A5055_D00B_4B49_89F5_05CEC043C7F8__INCLUDED_)
#define AFX_CFGBUFFVAR_H__8B3A5055_D00B_4B49_89F5_05CEC043C7F8__INCLUDED_


#define ER_REPORT_FILE NIVIS_TMP"er_report.txt" 

/// @ingroup libshared
class CCfgBuffVar  
{
public:
	CCfgBuffVar();
	virtual ~CCfgBuffVar();

	void Close();
	void Init (const char* p_szFile, const char* p_szGroup, const char* p_szVar);

public:
	void SetVar (const char* p_szValue);

private:
	char* m_szFile;
	char* m_szGroup;
	char* m_szVar;

	char m_szValue[1024];
};

#endif // !defined(AFX_CFGBUFFVAR_H__8B3A5055_D00B_4B49_89F5_05CEC043C7F8__INCLUDED_)
