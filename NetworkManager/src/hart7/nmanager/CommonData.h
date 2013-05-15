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
 * CommonData.h
 *
 *  Created on: May 25, 2009
 *      Author: Andy
 */

#include "../../Security/SecurityManager.h"
#include "../../NMSettingsLogic.h"
#include "../util/ManagerUtils.h"
#include "PeriodicTaskManager.h"
#include "ManagerStack.h"
#include "INodeVisibleVerifier.h"

#include "alarms/AlarmDispatcher.h"
#include "reports/ReportDispatcher.h"

#include <Model/NetworkEngine.h>
#include <Model/Operations/EngineOperations.h>
#include <Model/Services/Service.h>

#include <WHartStack/WHartLocalManagement.h>
#include <WHartStack/WHartStack.h>
#include <WHartStack/WHartTypes.h>

#ifndef COMMONDATA_H_
#define COMMONDATA_H_

using namespace hart7::stack;

namespace hart7 {
namespace nmanager {

/**
 * Common data for the entities in the Network Manager. Holds reference to common used components.
 */
struct CommonData
{
        struct Utils;

        CommonData(NE::Model::NetworkEngine& networkEngine_, hart7::security::SecurityManager& securityManager_,
                   hart7::util::NMSettingsLogic& settings_, PeriodicTaskManager& periodicTask_, ManagerStack& stack_,
                   hart7::nmanager::AlarmDispatcher& alarmDispatcher_,
                   hart7::nmanager::ReportDispatcher& reportDispatcher_,
                   hart7::nmanager::INodeVisibleVerifier& nodeVisibleVerifier_) :
            networkEngine(networkEngine_), securityManager(securityManager_), settings(settings_),
                        periodicTask(periodicTask_), stack(stack_), alarmDispatcher(alarmDispatcher_),
                        reportDispatcher(reportDispatcher_), nodeVisibleVerifier(nodeVisibleVerifier_), utils(*this)
        {
        }

        NE::Model::NetworkEngine& networkEngine;

        hart7::security::SecurityManager& securityManager;

        hart7::util::NMSettingsLogic& settings;

        PeriodicTaskManager& periodicTask;

        hart7::nmanager::ManagerStack& stack;

        hart7::nmanager::AlarmDispatcher& alarmDispatcher;

        hart7::nmanager::ReportDispatcher& reportDispatcher;

        hart7::nmanager::INodeVisibleVerifier& nodeVisibleVerifier;

        template<class T> bool serializeResponse(uint16_t cmdId, uint8_t* buffer, uint16_t buffSize, const T& t,
                                                 uint16_t& writtenBytes)
        {
            uint8_t responseCode = 0;
            WHartCommand commandsArray[] = { { cmdId, responseCode, (void *) (&t) } };
            WHartCommandList list = { 1, commandsArray };
            writtenBytes = 0;
            return stack.subapp.ComposePayload(buffer, buffSize, writtenBytes, list, true);
        }

        bool serializeResponse(uint16_t cmdId, uint8_t* buffer, uint16_t buffSize, void* t, uint16_t& writtenBytes)
        {
            uint8_t responseCode = 0;
            WHartCommand commandsArray[] = { { cmdId, responseCode, t } };
            WHartCommandList list = { 1, commandsArray };
            writtenBytes = 0;
            return stack.subapp.ComposePayload(buffer, buffSize, writtenBytes, list, true);
        }

        struct Utils
        {
            public:

                Utils(CommonData& commonData_) :
                    commonData(commonData_)
                {
                    nmService.serviceId = 0;
                    gwService.serviceId = 0x00;
                }

                NE::Model::Services::Service& GetServiceTo(const WHartAddress& address)
                {
                    Address32 destAddress;
					bool sendByProxy = false;

                    if (address == NetworkManager_Nickname() || address == NetworkManager_UniqueID())
                    {
                        return nmService;
                    }

                    if ((address == Gateway_Nickname() || address == Gateway_UniqueID()) || (address.type
                                == WHartAddress::whartaProxy && address.address.proxy.nickname == Gateway_Nickname())
                                || (address.type
                                        == WHartAddress::whartaProxyShort && address.address.proxyShort.nickname == Gateway_Nickname())
								)
                    {
                        return gwService;
                    }
                    else
                    {
                        if (address.type == WHartAddress::whartaProxy)
                        {
                            destAddress = address.address.proxy.nickname;
                            sendByProxy = true;
                        }
                        else if (address.type == WHartAddress::whartaProxyShort)
                        {
                            destAddress = address.address.proxyShort.nickname;
                            sendByProxy = true;
                        }
                        else
                        {
                            destAddress = address.address.nickname;
                        }
                    }

                    return commonData.networkEngine.findManagementService(destAddress, sendByProxy);
                }

                stack::WHartSessionKey::SessionKeyCode GetSessionType(const WHartAddress& address)
                {
                    Address32 destAddress;

                    if (address == NetworkManager_Nickname() || address == NetworkManager_UniqueID())
                    {
                        return stack::WHartSessionKey::sessionKeyed;
                    }

                    if ((address == Gateway_Nickname() || address == Gateway_UniqueID()))
                    {
                        destAddress = Gateway_Nickname();
                    }
                    else
                    {
                        if (address.type == WHartAddress::whartaProxy)
                        {
                            destAddress
                                        = commonData.networkEngine.createAddress32(
                                                                                   hart7::util::getAddress64FromUniqueId(
                                                                                                                         address.address.proxy.uniqueID));
                        }
                        else if (address.type == WHartAddress::whartaProxyShort)
                        {
                            destAddress = address.address.proxyShort.destNickname;
                        }
                        else
                        {
                            destAddress = address.address.nickname;
                        }
                    }
                    if (commonData.networkEngine.getDeviceSessionKey(destAddress))
                    {
                        return stack::WHartSessionKey::sessionKeyed;
                    }
                    return stack::WHartSessionKey::joinKeyed;
                }

                bool IsFailedResponse(const stack::WHartLocalStatus& localStatus, const stack::WHartCommandList& list)
                {
                    if (localStatus.status > stack::WHartLocalStatus::whartlsSuccess)
                    {
                        return true;
                    }

                    //CHECKME should I check in the list of commands too?
                    return false;
                }

            private:

                CommonData& commonData;

                NE::Model::Services::Service nmService;

                NE::Model::Services::Service gwService;
        };

        Utils utils;

};

}
}

#endif /* COMMONDATA_H_ */
