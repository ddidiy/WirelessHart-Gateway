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

#ifndef APPCOMMANDSFACTORY_H_
#define APPCOMMANDSFACTORY_H_

#include <nlib/log.h>
#include <nlib/exception.h>

#include <WHartHost/model/DBCommand.h>
#include <WHartHost/applicationcmds/AbstractAppCommand.h>
#include <WHartHost/database/DevicesManager.h>



namespace hart7 {
namespace hostapp {


/*
 * Factory pattern
 * Creates an application command
 */ 
class AppCommandsFactory
{
public:
	
	/**
	 * @throw InvalidCommandException
	 */ 
	AbstractAppCommandPtr Create(DBCommand& command, DevicesManager& devices);
};

}
}

#endif /*COMMANDSFACTORY_H_*/
