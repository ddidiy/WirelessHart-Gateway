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
 *  Created on: Dec 05, 2008
 *      Author: Radu Pop
 */

#include "MainApp.h"
#include "Manager.h"
#include <Model/NetworkEngine.h>

#include "../../Security/MockKeyGenerator.h"
#include "../../Security/KeyGenerator.h"
#include "../../Security/SecurityManager.h"

#include "script/ScriptManager.h"

#include <boost/bind.hpp>

#include <iostream>

namespace hart7 {

namespace nmanager {

MainApp::MainApp(const std::string& pathToLog_, const std::string& pathToScript_) :
    pathToLog(pathToLog_), pathToScript(pathToScript_)
{
}

MainApp::~MainApp()
{
}

void MainApp::Run(int argc, char* argv[])
{
    LOG_INFO("Starting network manager...");

    try
    {
        settingsLogic.LoadProvisioning();
        settingsLogic.LoadConfig();
        settingsLogic.scriptFileName = pathToScript;
        NE::Model::NetworkEngine::instance().setSettingsLogic(settingsLogic);

        hart7::security::KeyGenerator::Ptr
                    keyGenerator(settingsLogic.activateMockKeyGenerator ? new hart7::security::MockKeyGenerator()
                                : new hart7::security::KeyGenerator());

        hart7::security::SecurityManager securityManager(keyGenerator);

        Manager networkManager(NetworkEngine::instance(), securityManager, settingsLogic);
        managerMarkForReloadingConfigFilesHandler = boost::bind(&Manager::ManagerMarkForReloadingConfigFilesHandler,
                                                                &networkManager);
        networkManager.mainAppReloadConfigFilesHandler = boost::bind(&MainApp::ReloadConfigFiles, this);

        managerHupHandler = boost::bind(&Manager::HandleHUP, &networkManager);
        networkManager.Start();
    }
    catch (std::exception& ex)
    {
        LOG_ERROR("Error" << ex.what());
        std::cout << "Error: " << ex.what() << std::endl;
    }
    catch (...)
    {
        LOG_ERROR("Unknown exception!");
        std::cout << "Unknown exception!" << std::endl;
    }

    LOG_INFO("Stopped.");
}

void MainApp::Reload()
{
    //DO NOT LOG, CAN LEAD TO CRASH
    if (managerMarkForReloadingConfigFilesHandler)
    {
        managerMarkForReloadingConfigFilesHandler();
    }
}

void MainApp::ReloadConfigFiles()
{

    LOG_INFO("Reload(): BEGIN reloading configuration...");

    LOG_INFO("Reload(): reload logging configuration");
    LOG_REINIT(pathToLog.c_str());

    LOG_DEBUG("Reload(): reload provisioning");
    settingsLogic.LoadProvisioning();

    LOG_DEBUG("ExecuteScripts()");
    LOG_DEBUG("ExecuteScripts END");

    LOG_INFO("Reload(): finished loading configuration");

    if (managerHupHandler)
    {
        managerHupHandler();
    }

    LOG_INFO("Reload(): END");
}

} // namespace gateway
} // namespace hart7
