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
 * ManagerStack.cpp
 *
 *  Created on: Dec 05, 2008
 *      Author: Radu Pop
 */
#include "ManagerStack.h"
#include "AllNetworkManagerCommands.h"
#include "AllNMParsersComposers.h"
#include "../util/ManagerUtils.h"

#include <WHartStack/CommInterfaceOverIP/UDPServerIP.h>
#include <WHartStack/WHartStack.h>
#include <boost/bind.hpp>

#include "Shared/UtilsSolo.h"

#include "Provisioning/UniqueKeyProvisioning.h"
#include "Provisioning/DefaultProvisioning.h"


namespace hart7 {

namespace nmanager {

const int KEEP_ALIVE_TIMER_MULTIPLIER = 1;
const int KEEP_ALIVE_TIMER_INTERVAL = 250 * 12; //milliseconds


ManagerStack::ManagerStack(hart7::util::NMSettingsLogic& _settingsLogic) :
            service(nlib::socket::ServiceSocket::NewInstance()), settingsLogic(_settingsLogic), transport(commonData),
                datalink(commonData), timerFactory(new nlib::timer::TimerFactory(service, KEEP_ALIVE_TIMER_INTERVAL)),
                network(commonData), subapp(commonData), localManagement(commonData, subapp, transport, network, datalink, service,
                                                                         timerFactory)
{
    subapp.upper = this;
    subapp.lower = &transport;

    {
        subapp.composeReqParseResp = g_composeReqParseResp;
        subapp.composeReqParseRespSize = sizeof(g_composeReqParseResp) / sizeof(g_composeReqParseResp[0]);
        subapp.parseReqComposeResp = g_parseReqComposeResp;
        subapp.parseReqComposeRespSize = sizeof(g_parseReqComposeResp) / sizeof(g_parseReqComposeResp[0]);
    }

    transport.upper = &subapp;
    transport.lower = &network;

    network.upper = &transport;
    network.lower = &datalink;

    datalink.upper = &network;

    commonData.myUniqueID = NetworkManager_UniqueID();
    commonData.myNickname = NetworkManager_Nickname();

    //add by default the GW neighbor session
    datalink.CreateNeighborSession(Gateway_UniqueID(), Gateway_Nickname());

    //add TL entry
    transport.AddTableEntry(Gateway_Nickname(), true);

    timer = service->CreateDeadLineTimer();
    timer->Elapsed = boost::bind(&ManagerStack::PeriodicTask, this);
    timer->Start(KEEP_ALIVE_TIMER_INTERVAL);

    commonData.isJoined = true;

    localManagement.SetOnJoinCallback(boost::bind(&ManagerStack::OnJoin, this, _1));

    setProvisioning();
    initGWService();
}

void ManagerStack::setProvisioning()
{
    if (settingsLogic.uniqueKey)
    {
        LOG_INFO("Unique key provisioning set: " << settingsLogic.NetworkUniqueKey.toString());
        provisioning = UniqueKeyProvisioning::Ptr(new UniqueKeyProvisioning(settingsLogic.NetworkUniqueKey));
    }
    else
    {
        LOG_INFO("Default provisioning set");
        provisioning = DefaultProvisioning::Ptr(new DefaultProvisioning(settingsLogic));
    }

    initAPsJoinKey();
    initGWJoinKey();
}

void ManagerStack::PeriodicTask()
{
    static int count = 0;
    count++;
    if (count == KEEP_ALIVE_TIMER_MULTIPLIER)
    {
        count = 0;
        if (keepAliveHandler)
        {
            keepAliveHandler(KEEP_ALIVE_TIMER_MULTIPLIER * KEEP_ALIVE_TIMER_INTERVAL);
        }
    }

    static int loadLogCount = 0;
    loadLogCount++;
    if (loadLogCount == 60) // 60 seconds
    {
        loadLogCount = 0;
        double dMin1, dMin5, dMin15, dTotal;
        if (::GetProcessLoad(dMin1, dMin5, dMin15, dTotal, 1))
        {
            char szTmp[2048];
            sprintf(szTmp, "ProcessLoad: 1/5/15 min: %.2f/%.2f/%.2f total: %.2f", dMin1, dMin5, dMin15, dTotal);
            LOG_WARN(szTmp);
        }

    }
}

void ManagerStack::initGWService()
{
    C974_WriteRoute_Req route;
    C973_WriteService_Req service;
    C969_WriteGraphNeighbourPair_Req graph;

    graph.m_unGraphID = 0x0103;
    graph.m_unNeighborNickname = Gateway_Nickname();

    route.m_unPeerNickname = Gateway_Nickname();
    route.m_unGraphID = 0x0103;
    route.m_ucRouteID = 0;

    service.m_unPeerNickname = Gateway_Nickname();
    service.m_ucServiceID = 0x00;
    service.m_ucRouteID = 0;
    service.m_tPeriod.u32 = 32;

    AddGraph(graph);
    WriteRoute(route);
    WriteService(service);
}

/**
 * Creates sessions with APs (independent on how is configured provisioning (NETWORK=true/false UniqueKey/Default)
 */
void ManagerStack::initAPsJoinKey()
{
    std::vector<Address64>::iterator it;
    hart7::util::NMSettingsLogic::SecurityProvisioning::iterator itAddress;

    for (it = settingsLogic.backbones.begin(); it != settingsLogic.backbones.end(); it++)
    {

        itAddress = settingsLogic.provisioningKeys.find(*it);

        if (itAddress == settingsLogic.provisioningKeys.end())
        {
            LOG_INFO("AP " << it->toString() << " not found in provisioning.");
            continue;
        }

        C963_WriteSession_Req apJoinKey;

        memcpy(apJoinKey.m_aKeyValue, (*itAddress).second.value, sizeof((*itAddress).second.value));

        apJoinKey.m_ulPeerNonceCounterValue = 0; //TODO-generate nonce
        apJoinKey.m_unPeerNickname = No_Nickname();
        apJoinKey.m_eSessionType = WHartSessionKey::joinKeyed;
        apJoinKey.m_ucReserved = 0;
        memcpy(apJoinKey.m_aPeerUniqueID, hart7::util::getUniqueIdFromAddress64((*itAddress).first).bytes, 5);

        WHartTime40 time; //TODO-generate time
        time.hi = 0;
        time.u32 = 0;
        memcpy(apJoinKey.m_tExecutionTime, &time, 5);

        LOG_INFO("AP join key: " << (*itAddress).second.toString());
        WriteKey(apJoinKey);
    }
}

/**
 * Creates session with gateway (independent on how is configured provisioning (NETWORK=true/false UniqueKey/Default)
 */
void ManagerStack::initGWJoinKey()
{
    //set sessions for GW
    hart7::util::NMSettingsLogic::SecurityProvisioning::iterator itAddress = settingsLogic.provisioningKeys.find(settingsLogic.gatewayAddress64);
    if (itAddress == settingsLogic.provisioningKeys.end())
    {
        LOG_INFO("Address " << settingsLogic.gatewayAddress64.toString() <<" not found in provisioning file.");
        return;
    }

    C963_WriteSession_Req gwJoinKey;

    memcpy(gwJoinKey.m_aKeyValue, (*itAddress).second.value, sizeof((*itAddress).second.value));

    gwJoinKey.m_ulPeerNonceCounterValue = 0; //TODO-generate nonce
    gwJoinKey.m_unPeerNickname = Gateway_Nickname();
    gwJoinKey.m_eSessionType = WHartSessionKey::joinKeyed;
    gwJoinKey.m_ucReserved = 0;
    memcpy(gwJoinKey.m_aPeerUniqueID, hart7::util::getUniqueIdFromAddress64((*itAddress).first).bytes, 5);

    WHartTime40 time; //TODO-generate time
    time.hi = 0;
    time.u32 = 0;
    memcpy(gwJoinKey.m_tExecutionTime, &time, 5);

    LOG_INFO("GW join key: " << (*itAddress).second.toString());
    WriteKey(gwJoinKey);

}

//Disabled - New provisioning mechanism employed (Mihai)
void ManagerStack::initJoinKeys()
{
    //DEPRECATED, will be removed
}

ManagerStack::~ManagerStack()
{
}

void ManagerStack::Start()
{
    LOG_INFO("Start listen for gw on port=" << settingsLogic.listenPort);
    StartGWSession(settingsLogic.listenPort, settingsLogic.listenNetworkManagerMinRangePort,
                   settingsLogic.listenNetworkManagerMaxRangePort);
    service->Run();
}

void ManagerStack::Stop()
{

}

WHartHandle ManagerStack::TransmitRequest(const WHartAddress& to, WHartPriority priority,
                                          WHartTransportType transportType, WHartServiceID serviceID,
                                          const WHartCommandList& req, WHartSessionKey::SessionKeyCode sessionCode)
{
    if (to == NetworkManager_Nickname() || to == NetworkManager_UniqueID())
    {
        ProcessCommands(req);
        return LOCALMANAGEMENT_HANDLE;
    }

    return subapp.TransmitRequest(to, priority, serviceID, transportType, req, sessionCode);
}

void ManagerStack::TransmitResponse(WHartHandle handle, WHartServiceID serviceID, const WHartCommandList& res,
                                    WHartSessionKey::SessionKeyCode sessionCode)
{
    subapp.TransmitResponse(handle, serviceID, res, sessionCode);
}

void ManagerStack::TransmitConfirm(WHartHandle requestHandle, const WHartLocalStatus& localStatus,
                                   DeviceIndicatedStatus status, const WHartCommandList& list)
{
    if (TransmitConfirmCallBack)
    {
        TransmitConfirmCallBack(requestHandle, localStatus, list);
    }
}

void ManagerStack::TransmitIndicate(WHartHandle handle, const WHartAddress& src, WHartPriority priority,
                                    DeviceIndicatedStatus status, WHartTransportType transportType,
                                    const WHartCommandList& list)
{
    if (TransmitIndicateCallBack)
    {
        TransmitIndicateCallBack(handle, src, priority, transportType, list);
    }
}

bool ManagerStack::CanSendToDestination(const WHartAddress& dest)
{
	return transport.CanSendToDestination(dest);
}

void ManagerStack::SetKeepAliveHandler(const boost::function1<void, uint32_t>& keepAliveHandler)
{
    this->keepAliveHandler = keepAliveHandler;
}

void ManagerStack::StartGWSession(int networkManagerListenPort, int networkManagerMinRangePort,
                                  int networkManagerMaxRangePort)
{
    server.reset(new stack::transport::UDPServerIP(service, timerFactory, networkManagerListenPort,
                                                   networkManagerMinRangePort, networkManagerMaxRangePort));

    server->NewSession = boost::bind(&ManagerStack::NewGWSession, this, _1, _2);
    server->SessionClosed = boost::bind(&ManagerStack::GWSessionClosed, this, _1);
    server->Listen();
}

void ManagerStack::NewGWSession(const std::string& host, const stack::transport::SessionIP::Ptr session)
{
    LOG_INFO("GWSession is established! From host=" << host);
    datalink.SetNeighborSession(Gateway_UniqueID(), session);
}

void ManagerStack::GWSessionClosed(const stack::transport::SessionIP::Ptr session)
{
    LOG_INFO("GWSession was closed! session=" << *session);
    datalink.ResetNeighborSession(session);
}

void ManagerStack::WriteNickname(C2009_NM_WriteUniqueIdNickname_Req& req)
{
    localManagement.WriteNickname(req.uniqueId, req.nickname, req.isTR);
}

void ManagerStack::WriteKey(C963_WriteSession_Req& req)
{
    localManagement.WriteKey(req);
}

void ManagerStack::WriteRoute(C974_WriteRoute_Req& req)
{
    localManagement.WriteRoute(req);
}

void ManagerStack::WriteService(C973_WriteService_Req& req)
{
    localManagement.WriteService(req);
}

void ManagerStack::DeleteService(C801_DeleteService_Req& req)
{
    localManagement.DeleteService(req);
}

void ManagerStack::AddGraph(C969_WriteGraphNeighbourPair_Req& req)
{
    localManagement.AddGraph(req);
}

void ManagerStack::DeleteGraph(C970_DeleteGraphConnection_Req& req)
{
    localManagement.DeleteGraph(req);
}

/**
 * Called from network layer when join flow is detected
 */
void ManagerStack::OnJoin(const WHartAddress& address)
{
    LOG_DEBUG("OnJoin: " << address);

    if(address.address.uniqueID == Gateway_UniqueID())
    {
        LOG_DEBUG("OnJoin: GATEWAY_ADDRESS");
        return;
    }

    std::vector<Address64>::iterator itAddress;
    itAddress = settingsLogic.backbones.begin();
    for (; itAddress != settingsLogic.backbones.end(); itAddress++)
    {
        if (*itAddress == hart7::util::getAddress64FromUniqueId(address.address.uniqueID))
        {
            LOG_DEBUG("OnJoin: BACKBONE_ADDRESS");
            return;
        }
    }

    if (!provisioning)
    {
        return;
    }

    try
    {
        NE::Model::SecurityKey key;
        provisioning->getProvisioning(address.address.uniqueID, key);

        C963_WriteSession_Req req;
        {
            memcpy(req.m_aKeyValue, key.value, key.LENGTH);
            req.m_ulPeerNonceCounterValue = 0; //TODO-generate nonce
            req.m_unPeerNickname = No_Nickname();
            req.m_eSessionType = WHartSessionKey::joinKeyed;
            req.m_ucReserved = 0;
            memcpy(req.m_aPeerUniqueID, address.address.uniqueID.bytes, 5);
            WHartTime40 time; //TODO-generate time
            time.hi = 0;
            time.u32 = 0;
            memcpy(req.m_tExecutionTime, &time, 5);
        }

        localManagement.SetSession(req);
    }
    catch (ProvisioningException& )
    {
        LOG_INFO("No provisioning information found for " << address);
        localManagement.DeleteSession(address, WHartSessionKey::joinKeyed);
    }
}

void ManagerStack::ProcessCommands(const WHartCommandList& req)
{
    for (int i = 0; i < req.count; i++)
    {
        WHartCommand* currentCommand = req.list + i;

        switch (currentCommand->commandID)
        {
            case CMDID_C963_WriteSession:
            {
                localManagement.WriteKey(*((C963_WriteSession_Req*) currentCommand->command));
                break;
            }
            case CMDID_C974_WriteRoute:
            {
                localManagement.WriteRoute(*((C974_WriteRoute_Req*) currentCommand->command));
                break;
            }
            case CMDID_C976_WriteSourceRoute:
            {
                localManagement.WriteSourceRoute(*((C976_WriteSourceRoute_Req*) currentCommand->command));
                break;
            }
            case CMDID_C975_DeleteRoute:
            {
                localManagement.DeleteRoute(*((C975_DeleteRoute_Req*)currentCommand->command));
                break;
            }
            case CMDID_C977_DeleteSourceRoute:
            {
				localManagement.DeleteSourceRoute(*((C977_DeleteSourceRoute_Req*)currentCommand->command));
                break;
            }
            case CMDID_C973_WriteService:
            {
                localManagement.WriteService(*((C973_WriteService_Req*) currentCommand->command));
                break;
            }
            case CMDID_C801_DeleteService:
            {
                localManagement.DeleteService(*((C801_DeleteService_Req*) currentCommand->command));
                break;
            }
            case CMDID_C969_WriteGraphNeighbourPair:
            {
                localManagement.AddGraph(*((C969_WriteGraphNeighbourPair_Req*) currentCommand->command));
                break;
            }
            case CMDID_C970_DeleteGraphConnection:
            {
                localManagement.DeleteGraph(*((C970_DeleteGraphConnection_Req*) currentCommand->command));
                break;
            }
            case CMDID_C2009_NM_WriteUniqueIdNickname:
            {
                WriteNickname(*((C2009_NM_WriteUniqueIdNickname_Req*) currentCommand->command));
                break;
            }
            default:
            {
                LOG_WARN("Untreated local management command ID=" << (int) currentCommand->commandID);
                break;
            }
        }
    }
}

} // namespace nmanager
} // namespace hart7
