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

/*
 * ScriptManager.cpp
 *
 *  Created on: Jul 22, 2010
 *      Author: andrei.petrut
 */
#include "ScriptManager.h"
#include "../DevicesManager.h"
#include <iostream>
#include <fstream>

ScriptManager::ScriptManager(const std::string& scriptFileName_, hart7::nmanager::DevicesManager& devicesManager_)
    : parser(devicesManager_), scriptFileName(scriptFileName_), devicesManager(devicesManager_)
{
}

void ScriptManager::ExecuteScripts()
{
    std::cout << "Executing scripts from file " << scriptFileName << std::endl;
	std::fstream fstr(scriptFileName.c_str(), std::ios_base::in);
	fstr.exceptions( std::ifstream::eofbit | std::ifstream::failbit | std::ifstream::badbit );

	std::string str;
	try
	{
		while (!fstr.eof())
		{
			std::getline(fstr, str);
			parser.ParseLine(str);
		}
	}
	catch (std::exception& ex)
	{
	}

	fstr.clear();
	fstr.seekg(1, std::ios::beg);
}
