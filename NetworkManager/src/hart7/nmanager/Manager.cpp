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
 * Manager.cpp
 *
 *  Created on: Dec 15, 2008
 *      Author: Radu Pop
 */

#include "Manager.h"
#include "../util/ManagerUtils.h"
#include <Model/NetworkEngine.h>
#include <ApplicationLayer/Model/WirelessNetworkManagerCommands.h>
#include <boost/bind.hpp>

namespace hart7 {

namespace nmanager {

using namespace hart7::util;
using namespace hart7::nmanager::operations;

Manager::Manager(NetworkEngine& _networkEngine, hart7::security::SecurityManager& securityManager_,
                 hart7::util::NMSettingsLogic& settings_) :
    networkEngine(_networkEngine), securityManager(securityManager_), settings(settings_), stack(settings_),
                commonData(networkEngine, securityManager, settings_, periodicTask, stack, alarmDispatcher,
                           reportDispatcher, nodeVisibleVerifier), operationQueue(commandsProcessor, commonData),
                commandsProcessor(commonData), devicesManager(commandsProcessor, commonData, operationQueue),
                writePidFile("/tmp/WHart_NM.o.pid")
{
    stack.TransmitIndicateCallBack = boost::bind(&CommandsProcessor::ProcessIndicate, &commandsProcessor, _1, _2, _3,
                                                 _4, _5);
    stack.TransmitConfirmCallBack = boost::bind(&CommandsProcessor::ProcessConfirm, &commandsProcessor, _1, _2, _3);

    commandsProcessor.TransmitRequestEvent
                = boost::bind(&ManagerStack::TransmitRequest, &stack, _1, _2, _3, _4, _5, _6);
    commandsProcessor.TransmitResponseEvent = boost::bind(&ManagerStack::TransmitResponse, &stack, _1, _2, _3, _4);

    commandsProcessor.CanSendToDestinationEvent = boost::bind(&ManagerStack::CanSendToDestination, &stack, _1);

    commandsProcessor.HandleJoinRequest = boost::bind(&DevicesManager::DeviceRegistrationRequest, &devicesManager, _1,
                                                      _2);

    devicesManager.managerReloadConfigFilesHandler = boost::bind(&Manager::ManagerReloadConfigFilesHandler, this);

    periodicTask.RegisterPeriodicTask(devicesManager);
    stack.SetKeepAliveHandler(boost::bind(&PeriodicTaskManager::PeriodicTask, &periodicTask, _1));

    periodicTask.RegisterPeriodicTask(writePidFile);
}

void Manager::Start()
{
    LOG_DEBUG("Start()");

    writePidFile.PeriodicTask(); // write pid file at start
    stack.Start();
}

void Manager::Stop()
{
    LOG_DEBUG("Stop()");
}

void Manager::ManagerMarkForReloadingConfigFilesHandler()
{
    // DO NOT LOG, CAN LEAD TO CRASH
    devicesManager.markForReloadingConfiguration();
}

void Manager::ManagerReloadConfigFilesHandler()
{
    if (mainAppReloadConfigFilesHandler)
    {
        mainAppReloadConfigFilesHandler();
        HandleHUP();
    }
}

void Manager::HandleHUP()
{
    stack.localManagement.RemoveJoinKeys();
    stack.setProvisioning();
    //TODO send message to NE to delete missing devices...
}

}
}
