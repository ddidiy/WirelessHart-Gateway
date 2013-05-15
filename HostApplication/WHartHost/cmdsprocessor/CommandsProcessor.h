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

#ifndef COMMANDPROCESSOR_H_
#define COMMANDPROCESSOR_H_


#include <nlib/datetime.h>

#include <WHartHost/model/DBCommand.h>
#include <WHartHost/database/DBCommandsManager.h>
#include <WHartHost/database/DevicesManager.h>
#include <WHartHost/ConfigApp.h>

#include <WHartHost/gateway/GatewayIO.h>

namespace hart7 {
namespace hostapp {


/*
 * Process commands: DBCommands, AppCommands and GWCommands
 */
class CommandsProcessor
{
public:
	LOG_DEF("hart7.hostapp.CommandsProcessor");

public:
	CommandsProcessor(gateway::GatewayIO &gateway_);

//process
public:
	void ProcessRequest(DBCommand& command, DBCommandsManager& commands, DevicesManager& devices);
	void ProcessResponse(gateway::GatewayIO::GWResponse &response);

public:
	gateway::GatewayIO &gateway;
};

} //namspace hostapp
} //namspace hart7

#endif 
