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
 * MainApp.cpp
 *
 *  Created on: Nov 13, 2008
 *      Author: ovidiu.rauca
 */

#include <WHartGateway/MainApp.h>
#include <WHartGateway/Gateway.h>

#include <boost/bind.hpp>
#include <Shared/AnPaths.h>

#include <libgen.h>


namespace hart7 {
namespace gateway {

MainApp::MainApp()

{
}

MainApp::~MainApp()
{
}

void MainApp::Run(int argc, char* argv[])
{
	GatewayConfig config;
#ifdef ACCESS_NODE_LOG_ENABLED
	strcpy(config.m_szModuleName, argv[0]);
	sprintf(config.m_szAppPidFile, NIVIS_TMP"%s.pid", basename(config.m_szModuleName));

	char szLogName[256];

	sprintf(szLogName, NIVIS_TMP"%s.log", basename(config.m_szModuleName));

	LOG_INIT(szLogName);
#else
	LOG_INIT("etc/gw.log4cplus.conf");
#endif

	LOG_INFO_APP("Starting WHart_GW version "VERSION);


	if(config.Init() == true)
	{	LOG_INFO_APP("--------Starting --------");
		Gateway & gateway = Gateway::CreateInstance(config);

		gateway.Start();
	}

	else
	LOG_INFO_APP("-------------Stopped------------");
}

} // namespace host
} // namespace hart7
