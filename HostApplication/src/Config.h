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


#include <Shared/Config.h>

class  CConfigExt : public CIniParser {
public:
	CConfigExt() {}
	~CConfigExt() {}
	bool GetVar( const char* varName,std::string& value, const char* defValue)
	{
		char tmp[256];
		if ( ! CIniParser::GetVar( 0, varName, tmp, sizeof(tmp), 0, false ) )
			value = defValue;
		else
			value = tmp ;
		return true ;
	}
	bool GetVar( const char* varName,int& value, int defValue)
	{
		int tmp;
		if ( ! CIniParser::GetVar( 0, varName, &tmp, 0, false ) )
			value = defValue;
		else
			value = tmp ;
		return true ;
	}
	bool GetVar( const char* varName,bool& value, bool defValue)
	{
		char tmp[256];
		if ( ! CIniParser::GetVar( 0, varName, tmp, sizeof(tmp), 0, false ) )
			value = defValue;
		else
		{
			value=false;
			if ( !strcmp(tmp,"true") || !strcmp(tmp,"TRUE") || !strcmp(tmp,"1") ) value = true ;
		}
		return true ;
	}
	bool GetVar( const char* varName, float& value, float defValue)
	{
		float tmp;
		if ( ! CIniParser::GetVar( 0, varName, &tmp, 0, false ) )
			value = defValue;
		else
			value = tmp ;
		return true ;
	}
} ;
