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
 * ScriptManager.h
 *
 *  Created on: Jul 22, 2010
 *      Author: andrei.petrut
 */

#ifndef SCRIPTMANAGER_H_
#define SCRIPTMANAGER_H_

#include "ScriptLineParser.h"

namespace hart7 {
namespace nmanager {

class DevicesManager;

/**
 * Allows scripts to be run on the Network Manager.
 */
class ScriptManager
{
public:
    /**
     * Constructor.
     */
	ScriptManager(const std::string& scriptFileName, hart7::nmanager::DevicesManager& devicesManager);

	/**
	 * Execute the scripts from the specified file.
	 */
	void ExecuteScripts();


private:
	ScriptLineParser parser;
	std::string scriptFileName;

	hart7::nmanager::DevicesManager& devicesManager;
};

}
}


#endif /* SCRIPTMANAGER_H_ */
