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
 * Gateway.cpp
 *
 *  Created on: Dec 15, 2008
 *      Author: nicu.dascalu
 */

#include <map>

#include <WHartGateway/Gateway.h>

#include <WHartStack/util/WHartCmdWrapper.h>
#include <WHartStack/WHartSubApplicationData.h>

#include <WHartGateway/AllGatewayCommands.h>
#include <WHartGateway/GwUtil.h>

#include <boost/foreach.hpp>
#include <boost/bind.hpp> //for binding callbacks
#include <Shared/UtilsSolo.h>
#include "Shared/SignalsMgr.h"

using namespace boost;

namespace hart7 {
namespace gateway {

Gateway * Gateway::instance = NULL;

Gateway & Gateway::CreateInstance(GatewayConfig & config)
{
    if (!instance)
    {
        static Gateway staticInstance(config);
        instance = &staticInstance;
    }

    return *instance;
}

void Gateway::GwHandlerFATAL(int p_signal)
{
    HandlerFATAL(p_signal);

    Gateway::instance->PrepareShutdown();
    CSignalsMgr::Reset(p_signal);
}

Gateway::Gateway(GatewayConfig & config_) :
    CApp("WHART_GW.o"), cold_start(true), config(config_), service(nlib::socket::ServiceSocket::NewInstance()),
                timerFactory(new nlib::timer::TimerFactory(service)), stack(config_, service, timerFactory),
                m_oDevices(&m_oRequests, &config_), m_oCmdsProcessUniversal(*this), m_oCmdsProcessWirelessHART(*this),
                m_oCmdsProcessCommonPractice(*this)
{
    LOG_INFO_APP("-------------------------COLD_START SET TO TRUE ------------------------");

    CHartCmdWrapper::SetTable_ComposeReqParseResp(g_composeReqParseResp, g_nComposeReqParseRespSize);
    CHartCmdWrapper::SetTable_ParseReqComposeResp(g_parseReqComposeResp, g_nParseReqComposeRespSize);

    stack.TransmitConfirmCallBack = boost::bind(&Gateway::TransmitConfirm, this, _1, _2, _3, _4);
    stack.TransmitIndicateCallBack = boost::bind(&Gateway::TransmitIndicate, this, _1, _2, _3, _4, _5, _6);

    stack.NotifyWriteSessionCallback = boost::bind(&Gateway::Notify_WriteSession, this, _1, _2);
    stack.NotifyRejoinCallback = boost::bind(&Gateway::Notify_Rejoin, this);

    m_oOneSecondTimer = timerFactory->CreateDeadlineTimer(1000);
    m_oOneSecondTimer->Elapsed = boost::bind(&Gateway::OneSecondTasks, this);

    timer = timerFactory->CreateDeadlineTimer(GW_REQUESTS_REFRESH_INTERVAL);
    timer->Elapsed = boost::bind(&Gateway::RefreshGwRequests, this);

    if (config.LogInternalStatusPeriod() != 0)
    {
        m_oLogInternalStatusTimer = timerFactory->CreateDeadlineTimer(config.LogInternalStatusPeriod() * 1000);
        m_oLogInternalStatusTimer->Elapsed = boost::bind(&Gateway::logInternalStatus, this);
    }

    if (config.LogInternalRequestsStatisticsPeriod() != 0)
    {
        m_oLogInternalStatusTimer = timerFactory->CreateDeadlineTimer(config.LogInternalRequestsStatisticsPeriod() * 1000);
        m_oLogInternalStatusTimer->Elapsed = boost::bind(&Gateway::logInternalRequestsStatistics, this);
    }

    if (!config.m_nMockMode)
    {
        burstNotifyTimer = timerFactory->CreateDeadlineTimer(config.BurstNotificationRate());
        burstNotifyTimer->Elapsed = boost::bind(&Gateway::sendChangeNotification, this, NotificationMaskCodesMask_BurstMode);

        eventNotifyTimer = timerFactory->CreateDeadlineTimer(config.EventNotificationRate());
        eventNotifyTimer->Elapsed = boost::bind(&Gateway::sendChangeNotification, this, NotificationMaskCodesMask_EventNotification);

        deviceStatusNotifyTimer = timerFactory->CreateDeadlineTimer(config.DeviceStatusNotificationRate());
        deviceStatusNotifyTimer->Elapsed = boost::bind(&Gateway::sendChangeNotification, this, NotificationMaskCodesMask_DeviceStatus);

        deviceConfigurationNotifyTimer = timerFactory->CreateDeadlineTimer(config.DeviceConfigurationNotificationRate());
        deviceConfigurationNotifyTimer->Elapsed = boost::bind(&Gateway::sendChangeNotification, this, NotificationMaskCodesMask_DeviceConfiguration);

        m_oTopologyNotifyTimer = timerFactory->CreateDeadlineTimer(config.TopologyNotificationRate());
        m_oTopologyNotifyTimer->Elapsed = boost::bind(&Gateway::sendChangeNotification, this, NotificationMaskCodesMask_NetworkTopology);

        m_oScheduleNotifyTimer = timerFactory->CreateDeadlineTimer(config.ScheduleNotificationRate());
        m_oScheduleNotifyTimer->Elapsed = boost::bind(&Gateway::sendChangeNotification, this, NotificationMaskCodesMask_NetworkSchedule);
    }

    m_oDrmCache.SetEntryLifetime(config.m_nDrmEntryLifetime);

    m_oDevices.DeviceAdd(Gateway_UniqueID(), Gateway_Nickname());

    //wait until got a session with nm?
    CSignalsMgr::Install(SIGHUP);
    CSignalsMgr::Install(SIGUSR2);
    CSignalsMgr::Install(SIGUSR1);

    signal(SIGABRT, Gateway::GwHandlerFATAL);/// do NOT use delayed processing with HandlerFATAL. Stack trace must be dumped on event
    signal(SIGSEGV, Gateway::GwHandlerFATAL);/// do NOT use delayed processing with HandlerFATAL. Stack trace must be dumped on event
    signal(SIGFPE, Gateway::GwHandlerFATAL);/// do NOT use delayed processing with HandlerFATAL. Stack trace must be dumped on event

	// claudiu: SIGTERM and SIGINT are on main/normal flow and NOT on crash, there is no need for the stack to be dumped
	//			if the delayed processing is not used, depending where the signal catch the gw, it might not close properly
	CSignalsMgr::Install(SIGINT);
	CSignalsMgr::Install(SIGTERM);

    m_nDevicesRefreshCountDown = 0;

}


void Gateway::Start()
{
    if (!config.m_nMockMode)
    {
        HostApp_StartServer(config.HostApp_ListenPort(), config.HostApp_MinPort(), config.HostApp_MaxPort());
    }
    stack.Start();
}

void Gateway::Stop()
{
}

void Gateway::PrepareShutdown()
{
    LOG_WARN_APP("Gateway::PrepareShutdown");

    // stop GatewayStack
    stack.PrepareShutdown();

	// stop HostApp
    HostApp_StopServer();

    timer->Elapsed = NULL;
    burstNotifyTimer->Elapsed = NULL;
    eventNotifyTimer->Elapsed = NULL;
    deviceStatusNotifyTimer->Elapsed = NULL;
    deviceConfigurationNotifyTimer->Elapsed = NULL;
    m_oTopologyNotifyTimer->Elapsed = NULL;
    m_oScheduleNotifyTimer->Elapsed = NULL;

    service->Stop();
}

void Gateway::OneSecondTasks()
{
    //LOG_DEBUG("OneSecondTasks start");
    TouchPidFile(config.m_szAppPidFile);

    double dMin1, dMin5, dMin15, dTotal;
    if (::GetProcessLoad(dMin1, dMin5, dMin15, dTotal, GET_PROCESS_LOAD_AT_ONE_MIN))
    {
        char szTmp[2048];
        sprintf(szTmp, "ProcessLoad: 1/5/15 min: %.2f/%.2f/%.2f total: %.2f", dMin1, dMin5, dMin15, dTotal);
        LOG_INFO_APP("" << szTmp);
    }

    if (m_nDevicesRefreshCountDown-- <= 0)
    {
        m_nDevicesRefreshCountDown = config.m_nDevicesRefreshInterval;

        if (!config.m_nMockMode && m_oDevices.DeviceRefresh())
        {
            RefreshGwRequests();
        }
    }

    m_oDrmCache.Refresh();

    if (CSignalsMgr::IsRaised(SIGTERM))
    {
        LOG_WARN_APP("Signal to stop set (SIGTERM)");

        PrepareShutdown();
        CSignalsMgr::Reset(SIGTERM);
    }


    if (CSignalsMgr::IsRaised(SIGINT))
    {
        LOG_WARN_APP("Signal to stop set (SIGINT)");

        PrepareShutdown();
        CSignalsMgr::Reset(SIGINT);
    }


    if (CSignalsMgr::IsRaised(SIGHUP))
    {
        LOG_WARN_APP("Signal SIGHUP");

        config.RuntimeReload();
        m_oDrmCache.SetEntryLifetime(config.m_nDrmEntryLifetime);
        CSignalsMgr::Reset(SIGHUP);
    }

    if (CSignalsMgr::IsRaised(SIGUSR1))
    {
        logInternalRequestsStatistics();
        CSignalsMgr::Reset(SIGUSR1);
    }

    if (CSignalsMgr::IsRaised(SIGUSR2))
    {
        logDevicesStatistics();
        CSignalsMgr::Reset(SIGUSR2);
    }

    //LOG_DEBUG("OneSecondTasks end");
}

void Gateway::EraseGwRequest(CGwRequests::iterator & p_itDel)
{
    CGwRequest::Ptr pReq = *p_itDel;

    LOG_INFO_APP("EraseGwRequest: retries_left=" << pReq->m_nRetryNo << ", reqStatus=" << pReq->m_nRequestStatus);
    if (pReq->m_nSrc == CGwRequest::SourceHostApp)
    {
        CHostAppSession::Ptr pSession = pReq->m_pHostAppSession.lock();
        if (pSession)
        {
            pSession->RequestRemove(pReq);
        }
    }
    else if (pReq->m_nSrc == CGwRequest::SourceLocalGw)
    {
        CLogicalDevice::Ptr pDevice = m_oDevices.GetLogicalDevice(pReq->m_oDevUniqueID);
        if (pDevice)
        {
            pDevice->GwLocalRequestRemove(pReq);
        }
    }
    else
    {
        LOG_INFO_APP("EraseGwRequest: unknown src for req=" << pReq << ", src=" << pReq->m_nSrc);
    }

    m_oRequests.erase(p_itDel);

    CGwRspHandleMap::iterator it = m_oRspHandles.find(pReq->m_nHandle);
    if (it != m_oRspHandles.end())
    {
        LOG_INFO_APP("EraseGwRequest: remove handle=" << pReq->m_nHandle);
        // do not expect confirm anymore
        m_oRspHandles.erase(it);
    }

}

void Gateway::RefreshGwRequests()
{
    //LOG_DEBUG("Gateway::RefreshGwRequests");
    CGwRequests::iterator itReqs = m_oRequests.begin();

    for (; itReqs != m_oRequests.end();)
    {
        CGwRequest::Ptr pReq = *itReqs;

        CGwRequests::iterator itDel = itReqs;
        itReqs++;
        if (pReq->m_nRequestStatus == CGwRequest::RequestStatus_ResponseReceived)
        {
            EraseGwRequest(itDel);
            continue;
        }

        if (pReq->m_nNextTimeout > GetClockTicks())
        {
            continue;
        }

        if (pReq->m_nRequestStatus == CGwRequest::RequestStatus_Cancel)
        {
            EraseGwRequest(itDel);
            continue;
        }

        if (pReq->m_nRetryNo <= 0)
        {
            pReq->SendDR_DEAD(); //send NACK if host app
            EraseGwRequest(itDel);
            continue;
        }

        transmitGwRequest(pReq);
    }
}

/*
 * Send response for pending requests that match the given confirmed request.
 */
void Gateway::TransmitConfirm(const CGwRequest::Ptr & gwRequest)
{
    if (!gwRequest)
    {
        LOG_ERROR_APP("Gateway::TransmitConfirm - GwRequest is NULL");
        return;
    }

    for (CGwRequests::iterator itGwRequests = m_oRequests.begin(); itGwRequests != m_oRequests.end(); ++itGwRequests)
    {
        CGwRequest::Ptr & pReq = *itGwRequests;

        if (pReq->m_nRequestStatus == CGwRequest::RequestStatus_PendingSend && pReq->MatchConfirmedGwRequest(*gwRequest))
        {
            LOG_INFO_APP("Gateway::TransmitConfirm - Mark status as ResponseReceived for request that match the current confirmed request. Current confirmed Req="
                         << gwRequest << ", Changing Status for Req=" << std::dec << pReq);

            pReq->m_nRequestStatus = CGwRequest::RequestStatus_ResponseReceived;

            pReq->m_oRspList = gwRequest->m_oRspList;

            pReq->SendRspList();

            continue;
        }
    }
}

void Gateway::TransmitConfirm(WHartHandle handle, WHartLocalStatus localStatus, DeviceIndicatedStatus status, CHartCmdWrapperList & responses)
{
    CGwRspHandleMap::iterator it = m_oRspHandles.find(handle);

    if (it == m_oRspHandles.end())
    {
        //log unsolicited response and do nothing for now
        LOG_ERROR_APP("Gateway::TransmitConfirm: unsolicited response: handle=" << handle << ", cmds= " << responses);
        return;
    }
    CGwRequest::Ptr pReq = it->second; // don't get reference to smart pointer; erase; crash

    m_oRspHandles.erase(it);

    LOG_INFO_APP("Gateway::TransmitConfirm: " << pReq << ", cmds= " << CHartCmdWrapperListShortDetails(responses));
    LOG_DEBUG_APP("Gateway::TransmitConfirm: " << pReq << ", cmds= " << responses);

    CService::Ptr pService = pReq->m_pService.lock();

    if (pService)
    {
        pService->NotifyTransmitConfirm();
    }

    if (localStatus.status >= WHartLocalStatus::whartlsError_Start)
    {
        LOG_ERROR_APP("Gateway::TransmitConfirm: " << pReq << ", LocalStatus=" << (int)localStatus.status );
        pReq->m_nRequestStatus = CGwRequest::RequestStatus_PendingSend; // prepare to resend
        return;
    }

    if (responses.empty())
    {
        LOG_ERROR_APP("Gateway::TransmitConfirm: " << pReq << " response list empty");
        pReq->m_nRequestStatus = CGwRequest::RequestStatus_Cancel; // we change policy on response list empty error,
        pReq->SendDR_DEAD();
        return;
    }

    //add responses to cache


    CLogicalDevice::Ptr pDevice = m_oDevices.GetLogicalDevice(pReq->m_oDevUniqueID, false);

    if (!pDevice)
    {
        LOG_ERROR_APP("Gateway::TransmitConfirm: unknown device=" << pReq->m_oDevUniqueID);
        return;
    }
    pDevice->ResponseAdd(status.deviceStatus, status.deviceExtendedStatus, responses, pReq.get());

    if (config.m_nDrmType == GatewayConfig::DRM_TYPE_IO_SYTEM && pReq->IsWiredMessage() && pReq->m_nSrc == CGwRequest::SourceHostApp)
    {
        m_oDrmCache.ResponseAdd(pReq.get(), responses);
    }

    CHartCmdWrapperList::iterator itCmd = pReq->m_oCmdList.begin();

    pReq->m_nRequestStatus = CGwRequest::RequestStatus_ResponseReceived;

    for (; itCmd != pReq->m_oCmdList.end();)
    {
        CHartCmdWrapper::Ptr pCmd = *itCmd;

        if (responses.empty())
        {
            LOG_ERROR_APP("Gateway::TransmitConfirm: responses empty; startRetryTimeout=" << pReq->m_nStartRetryTimeout << ", configRetryTimeout=" << config.GwReqRetryTimeout()
                          << ", clockTicks=" << GetClockTicks() << ", req=" << pReq);

            //prevent drop of request. we are retrying because we have cmds left and not because of failure
            //they didn't all fit in the req package, so we send multiple requests
            pReq->m_nRetryNo++;

            pReq->m_nRequestStatus = CGwRequest::RequestStatus_PendingSend;
            break;
        }

        CHartCmdWrapper::Ptr pRsp = responses.front();
        responses.pop_front();

        if (pRsp->GetCmdId() != pCmd->GetCmdId())
        {
            LOG_ERROR_APP("Gateway::TransmitConfirm: pRsp->GetCmdId() != pCmd->GetCmdId() (" << pRsp->GetCmdId() << " != " << pCmd->GetCmdId() << "), " << pReq);
            pReq->m_nRequestStatus = CGwRequest::RequestStatus_PendingSend;
            // Beni - pReq->m_nNextTimeout = ???
            break;
        }

        //add special response processing here
        // caching responses is resolved above


        if (IsCmdNeedingRetry(pRsp->GetCmdId(), pRsp->GetResponseCode()) && pReq->m_nRetryNo > 0)
        {
            LOG_ERROR_APP("Gateway::TransmitConfirm: " << pReq << ", rspCodeError=" << pRsp->GetResponseCode() << ", retries left=" << pReq->m_nRetryNo);

            pReq->m_nRequestStatus = CGwRequest::RequestStatus_PendingSend;
            pReq->m_nNextTimeout = GetClockTicks() + config.GwDrmTimeout(); //for now only DRM can cause retry here

            itCmd++;
            continue; // TODO: Beni - or break ???
        }

        //log
        //LOG_INFO_APP("Gateway::TransmitConfirm: " << pReq);

        pReq->m_oCmdExecList.push_back(pCmd);
        pReq->m_oRspList.push_back(pRsp);
        itCmd = pReq->m_oCmdList.erase(itCmd);

        switch (pCmd->GetCmdId())
        {
            case CMDID_C799_RequestService:
            {
                if (pRsp->GetResponseCode() == RCS_N00_Success && pCmd->GetParsedData() && pRsp->GetParsedData()) // fix issue 962
                {
                    C799_RequestService_Resp * pStResp = (C799_RequestService_Resp*) pRsp->GetParsedData();
                    m_oDevices.ServiceReqResponse((C799_RequestService_Req*) pCmd->GetParsedData(), pStResp, pRsp->GetResponseCode());
                    if (pStResp)
                    {
                        C973_WriteService_Req stWriteService;

                        stWriteService.m_eApplicationDomain = pStResp->m_ucServiceApplicationDomain;
                        stWriteService.m_tPeriod = pStResp->m_tPeriod;
                        stWriteService.m_ucRequestFlags = pStResp->m_ucServiceRequestFlags;
                        stWriteService.m_ucRouteID = pStResp->m_ucRouteId;
                        stWriteService.m_unPeerNickname = pStResp->m_unNicknameOfPeer;
                        stWriteService.m_ucServiceID = pStResp->m_ucServiceId;

                        stack.GetLocalManagement()->WriteService(stWriteService);
                    }
                }
                else
                {
                    if (pRsp->GetResponseCode() != RCS_N00_Success)
                        LOG_WARN_APP("Gateway::TransmitConfirm - RC not success for rsp=" << pRsp);

                    if (!pCmd->GetParsedData())
                        LOG_WARN_APP("Gateway::TransmitConfirm - ParsedData is NULL for cmd=" << pCmd);

                    if (!pRsp->GetParsedData())
                        LOG_WARN_APP("Gateway::TransmitConfirm - ParsedData is NULL for rsp=" << pRsp);
                }
                break;
            }
            default:
            break;
        }
    }

    if (pReq->m_oCmdList.empty())
    {
        //LOG_INFO_APP("Gateway::TransmitConfirm: " << pReq );
        pReq->m_nRequestStatus = CGwRequest::RequestStatus_ResponseReceived; // just in case to protect against programmer mistakes

        /// Claudiu Hobeanu
        /// HartServer 3.2.1 seems to be bothered by the late (or optimized) responses
        /// when new and improved HartServers will be available test and remove the test
        /// when full DRM (I/o system DRM) will be tested and working change this test
        ///		if ( !pReq->IsWiredMessage() ||	config.m_nDrmType != GatewayConfig::DRM_TYPE_IO_SYTEM)
        if (!pReq->IsWiredMessage() || config.m_nDrmType == GatewayConfig::DRM_TYPE_NONE)
        {
            LOG_DEBUG_APP("Gateway::TransmitConfirm: not IsWiredMessage or DrmType is DRM_TYPE_NONE => SendRspList " << pReq << ", cmds=" << CHartCmdWrapperListShortDetails(pReq->m_oCmdList));
            pReq->SendRspList(); //

            TransmitConfirm(pReq);
        }

        return;
    }

    if (pReq->m_nRequestStatus == CGwRequest::RequestStatus_PendingSend)
    {
        clock_t clockTicks = GetClockTicks();

        LOG_INFO_APP("Gateway::TransmitConfirm: startRetryTimeout=" << pReq->m_nStartRetryTimeout << ", configRetryTimeout=" << config.GwReqRetryTimeout()
                      << ", clockTicks=" << clockTicks << ", req=" << pReq);

        if (pReq->m_nStartRetryTimeout + config.GwReqRetryTimeout() < clockTicks)
        {
            LOG_INFO_APP("Gateway::TransmitConfirm: reset RetryNo; " << pReq << ", cmds=" << CHartCmdWrapperListShortDetails(pReq->m_oCmdList));
            pReq->m_nRetryNo = 0;
        }
        else
        {
            LOG_INFO_APP("Gateway::TransmitConfirm: prepare resend " << pReq << ", cmds=" << CHartCmdWrapperListShortDetails(pReq->m_oCmdList) << ", retries left=" << pReq->m_nRetryNo);
        }
    }
}

void Gateway::TransmitIndicate(WHartHandle handle, const WHartAddress & src, WHartPriority priority, DeviceIndicatedStatus status,
                               WHartTransportType transportType, CHartCmdWrapperList & p_rRequests)
{
    LOG_DEBUG_APP("Gateway::TransmitIndicate: handle=" << handle << ", src=" << src << ", priority=" << priority << ", status=" << status
                  << ", transportType=" << transportType << ", requests=" << p_rRequests);

    bool bIsResponse = subapp::WHartSubApplicationData::IsResponseType(transportType);

    if (transportType == wharttRequestUnicast)
    {
        bool conversionOk = ConvertBurstsSentAsRequestToResponses(p_rRequests);
        if (conversionOk)
        {
            bIsResponse = true;
        }
    }

    //NotificationMaskCodesMask_BurstMode

    if (bIsResponse)
    {
        LOG_DEBUG_APP("Gateway::TransmitIndicate: isResponse = true");
        // add responses to cache

        if (!config.m_nMockMode)
        {
            m_oDevices.ResponseAdd(src, status.deviceStatus, status.deviceExtendedStatus, p_rRequests, NULL);
        }

        sendIndicateDirectly(src, status.deviceStatus, status.deviceExtendedStatus, p_rRequests);
        // cmd 119 must have a request as response with notify transport type

        return;
    }

    //local_management - commands from nm to setup stack levels
    CHartCmdWrapperList oListRsp;

    int nNMConfCmds = 0;
    int nOthersCmds = 0;

    CHartCmdWrapperList::iterator itCmds = p_rRequests.begin();
    for (itCmds = p_rRequests.begin(); itCmds != p_rRequests.end(); itCmds++)
    {
        CHartCmdWrapper::Ptr pCmd = *itCmds;

        if (!IsNetMgmtConfigCmd(pCmd->GetCmdId()))
        {
            LOG_ERROR_APP("Gateway::TransmitIndicate NOT a IsNetMgmtConfigCmd -> not implemented for now nCmdId=" << pCmd->GetCmdId());
            CHartCmdWrapper::Ptr pRsp(new CHartCmdWrapper);

            pRsp->LoadRaw(pCmd->GetCmdId(), 0, NULL, RCS_E64_CommandNotImplemented, config.DeviceStatus());
            oListRsp.push_back(pRsp);

            nOthersCmds++;
            continue;
        }

        nNMConfCmds++;

        CHartCmdWrapper::Ptr pRsp(new CHartCmdWrapper);

        if (pCmd->GetCmdId() == CMDID_C973_WriteService)
        {
            uint8_t res = 0;

            if (!config.m_nMockMode)
            {
                res = m_oDevices.ServiceWrite((C973_WriteService_Req*) pCmd->GetParsedData());
            }

            if (res)
            {
                pRsp->LoadRaw(CMDID_C973_WriteService, 0, 0, res, config.DeviceStatus());
                oListRsp.push_back(pRsp);
                continue;
            }
        }

        stack.TryExecCmd(src, pCmd.get(), pRsp.get());

        if (pRsp->GetCmdId() == CHartCmdWrapper::g_sCmdIdInvalid)
        {
            LOG_ERROR_APP("Gateway::TransmitIndicate: no response for cmdId=" << pCmd->GetCmdId());
            continue;
        }

        pRsp->SetDeviceStatus(config.DeviceStatus());
        oListRsp.push_back(pRsp);

        if (IsResponseCodeError(pRsp->GetResponseCode()) )
        {
            continue;
        }

        if (config.m_nMockMode)
        {
            continue;
        }
        // success or warning

        //update the notifications for gateway
        UpdateGwNotifications(pCmd->GetCmdId());

        switch (pCmd->GetCmdId())
        {
            case CMDID_C801_DeleteService:
                m_oDevices.ServiceDelete((C801_DeleteService_Req*) pCmd->GetParsedData(),
                                         (C801_DeleteService_Resp*) pRsp->GetParsedData(), pRsp->GetResponseCode());
                break;

            case CMDID_C973_WriteService:
                // handle before TryExec
                break;

            case CMDID_C963_WriteSession:
                Notify_WriteSession((const C963_WriteSession_Req*) pCmd->GetParsedData(), NULL);
                break;

            case CMDID_C964_DeleteSession:
                Notify_DeleteSession((const C964_DeleteSession_Req*) pCmd->GetParsedData(),
                                     (const C964_DeleteSession_Resp*) pRsp->GetParsedData());
                break;
        }
    }

    LOG_DEBUG_APP("Gateway::TransmitIndicate nOthersCmds=" << nOthersCmds << " nNMConfCmds" << nNMConfCmds);
    if (nOthersCmds && nNMConfCmds)
    {
        LOG_WARN_APP("Gateway::TransmitIndicate command mixt nOthersCmds=" << nOthersCmds << " nNMConfCmds" << nNMConfCmds);
    }

    if (!oListRsp.empty())
    {
        //send response to lower layer
        stack.TransmitResponse(handle, 0, oListRsp, stack::WHartSessionKey::sessionKeyed); //serviceID ?
        return;
    }
}

void Gateway::HostApp_StartServer(int listenPort, int minPort, int maxPort)
{
    LOG_INFO_APP("HostApp_StartServer: on port=" << listenPort);

    m_oHostAppServer.reset(new stack::transport::UDPServerIP(service, timerFactory, listenPort, minPort, maxPort));
    m_oHostAppServer->NewSession = boost::bind(&Gateway::HostApp_NewSession, this, _1, _2);
    m_oHostAppServer->SessionClosed = boost::bind(&Gateway::HostApp_SessionClosed, this, _1);
    m_oHostAppServer->Listen();
}

void Gateway::HostApp_StopServer()
{
    LOG_INFO_APP("HostApp_StopServer: on port=" << config.HostApp_ListenPort());

    m_oHostAppServer->NewSession = NULL;
    m_oHostAppServer->SessionClosed = NULL;

    m_oHostAppServer->Close();
    sleep(1);
}

void Gateway::HostApp_NewSession(const std::string & host, const stack::transport::SessionIP::Ptr session)
{
    LOG_INFO_APP("HostApp_NewSession: established from host=" << host);

    session->ReceiveMessage = boost::bind(&Gateway::HostApp_ReceiveMessage, this, session, _1, Loki::Function<void(const stack::transport::MessageIP&)>());

    m_oHostAppSessions.SessionAdd(host, session);
}

void Gateway::HostApp_SessionClosed(const stack::transport::SessionIP::Ptr session)
{
    std::ostringstream stream;
    stream << *session;
    LOG_WARN_APP("HostApp_SessionClosed: session=" << stream.str());

    //datalink.ResetNeighborSession(session);
    session->ReceiveMessage = NULL;
    m_oHostAppSessions.SessionRemove(session);
}

void Gateway::HostApp_ReceiveMessage(const stack::transport::SessionIP::Ptr session,
                                     const stack::transport::MessageIP & packet, Loki::Function<void(const stack::transport::MessageIP&)> callback)
{
    CHostAppSession::Ptr pHostSession = m_oHostAppSessions.GetSession(session);

    if (!pHostSession)
    {
        LOG_ERROR_APP("Invalid session! packet=" << packet << ", session=" << *session);
        return;
    }

    LOG_DEBUG_APP("HostApp_ReceiveMessage::session=" << *session << ", packet=" << packet);

    CGwRequest::Ptr pQReq = CGwRequest::ExtractRequest(packet, true);

    if (!pQReq)
    {
        LOG_ERROR_APP("Invalid packet received! packet=" << packet << " for session=" << *session);
        return;
    }

    if (pQReq->errorCodeResponse != transport::HARTWiredPDU_Response::rcSuccess)
    {
        LOG_ERROR_APP("Invalid packet received! errorCodeResponse=" << (int)pQReq->errorCodeResponse << " packet=" << packet << " for session=" << *session);
        pQReq->m_pHostAppSession = pHostSession;
        pQReq->SendDR(pQReq->errorCodeResponse);
        return;
    }

    pQReq->m_nRetryNo = config.GwReqMaxRetryNo();

    pQReq->m_pHostAppSession = pHostSession;

    if (pHostSession->m_nMessageID < 0)
    {
        pHostSession->m_nMessageID = pQReq->m_nMessageID;
    }

    LOG_INFO_APP("Gateway::HostApp - session=" << *session << ", recv: " << pQReq << " pQReq->m_oCmdList.size=" << pQReq->m_oCmdList.size() << ", cmds=" << CHartCmdWrapperListShortDetails(pQReq->m_oCmdList));
    LOG_DEBUG_APP("Gateway::HostApp - cmds=" << pQReq->m_oCmdList);

    CHartCmdWrapper::Ptr pCmdReq = *pQReq->m_oCmdList.begin();

    if (pCmdReq->MetaCmdInfoGet())
    {
        if ((pCmdReq->MetaCmdInfoGet()->m_u16MetaCmdId == CMDID_C64765_NivisMetaCommand))
        {
            pHostSession->m_nNivisSpecificCmdRecv = 1;
        }

        if (pCmdReq->MetaCmdInfoGet()->m_oMetaCmdUniqueId == Gateway_UniqueID())
        {
            CmdList_MetaCmdInfoDel(pQReq->m_oCmdList);
        }
    }

    if (pQReq->m_oDevUniqueID == Gateway_UniqueID()
                || (pQReq->m_oDevUniqueID == NetworkManager_UniqueID() && !pQReq->m_bByPassIOCache))
    {
        // if address is gw or NM command treat here
        if (ExecLocalCmds(pQReq, pHostSession, m_oCmdsProcessUniversal, m_oCmdsProcessWirelessHART, m_oCmdsProcessCommonPractice))
        {
            if (!pQReq->m_oRspList.empty())
            {
                pQReq->SendRspList();
            }

            return;
        }
    }

    if (!DrmGetResponses(pQReq))
    {
        return;
    }

    CLogicalDevice::Ptr pLogicalDevice = m_oDevices.GetLogicalDevice(pQReq->m_oDevUniqueID, false);
    if (!pLogicalDevice)
    {
        LOG_ERROR_APP("HostApp_ReceiveMessage::session=" << *session << ", Device " << pQReq->m_oDevUniqueID << " is NULL");
    }

    if (GetResponsesFromCache(pQReq, pLogicalDevice))
    {
        pQReq->SendRspList();
        return;
    }

    if (!CacheBasedDRMCheck(pQReq))
    {
        return;
    }

    CLogicalDevice * pLogicalDeviceToSendTo = GetDeviceToSendTo(pQReq, pLogicalDevice);

    if ((pLogicalDeviceToSendTo == 0) || !pLogicalDeviceToSendTo->IsActive()
                || ((pQReq->m_u16NickNameToSendTo = pLogicalDeviceToSendTo->GetNick()) == g_cAddrNicknameInvalid))
    {
        LOG_ERROR_APP( "device to send to is not valid -> send DR_DEAD back : "
                    << " deviceExists:" << (int)(pLogicalDeviceToSendTo != 0)
                    << " deviceIsActive:" << (int)(pLogicalDeviceToSendTo != 0 && pLogicalDeviceToSendTo->IsActive())
                    << " deviceHasValidNick:" << (int)(pLogicalDeviceToSendTo != 0 && pLogicalDeviceToSendTo->GetNick() != g_cAddrNicknameInvalid));

        pQReq->SendDR_DEAD();
        return;
    }

    CService::Ptr pService;

    if (config.m_nGwRequestService)
        pService = pLogicalDeviceToSendTo->ServiceGet(pCmdReq->GetCmdId());
    else
        pService = pLogicalDeviceToSendTo->ServiceGetTable()->GetServiceIfExist(pCmdReq->GetCmdId());

    if (!pService)
    {
        //send back response on session;
        LOG_ERROR_APP( "no service for " << pLogicalDeviceToSendTo->m_oDeviceUniqueID << " cmdId=" << std::dec << (int)pCmdReq->GetCmdId());
        pQReq->SendDR_DEAD();
        return;
    }

    if (m_oHostAppSessions.RequestAdd(session, pQReq))
    {
        pQReq->m_pService = pService;
        m_oRequests.push_back(pQReq);

        LOG_INFO_APP("Gateway::HostAppSessions - RequestAdd " << pQReq << " pQReq->m_oCmdList.size=" << pQReq->m_oCmdList.size() << ", cmds=" << CHartCmdWrapperListShortDetails(pQReq->m_oCmdList));

        // send req to stack
        transmitGwRequest(pQReq);
    }
}

//return	true  -- send message to field
//			false -- response from cache or DRM response was sent, drop message
bool Gateway::DrmGetResponses(CGwRequest::Ptr & p_pQReq)
{
    if (!p_pQReq->IsWiredMessage())
    {
        return true;
    }

    if (config.m_nDrmType != GatewayConfig::DRM_TYPE_IO_SYTEM)
    {
        return true;
    }

    int nRes = m_oDrmCache.ResponsePop(p_pQReq.get());

    if (nRes == CDrmCache::m_sCmdNotInCache)
    {
        return true;
    }

    if (nRes == RCS_E32_Busy || nRes == RCS_E33_DelayedResponseInitiated || nRes == RCS_E34_DelayedResponseRunning || nRes == RCS_E36_DelayedResponseConflict)
    {
        p_pQReq->SendDR(nRes);
        return false;
    }

    if (!p_pQReq->m_oRspList.empty())
    {
        p_pQReq->SendRspList();
        return false;
    }

    return true;
}

//return	true  -- send message to field
//			false -- DRM response sent, drop message
bool Gateway::CacheBasedDRMCheck(CGwRequest::Ptr p_pQReq)
{
    // LOG_DEBUG_APP("Gateway::CacheBasedDRMCheck pReq=" << p_pQReq);

    if (!p_pQReq->IsWiredMessage())
    {
        return true;
    }

    // expects only one command on wired message
    CHartCmdWrapper::Ptr pCmdReq = *p_pQReq->m_oCmdList.begin();

    CGwRequests::iterator itReqs = m_oRequests.begin();

    for (; itReqs != m_oRequests.end(); ++itReqs)
    {
        CGwRequest::Ptr pReq = *itReqs;

        uint8_t u8Res = pReq->MatchCacheBasedDRM(p_pQReq->m_oDevUniqueID, pCmdReq);
        if (u8Res == RCS_E33_DelayedResponseInitiated)
        {
            continue;
        }

        //DRM_TYPE_NONE

        //if (config.m_nDrmType != GatewayConfig::DRM_TYPE_CACHE_BASED)
        /// on wired and without DRM just drop message, the HartServer seems to expect this
        if (config.m_nDrmType != GatewayConfig::DRM_TYPE_NONE)
        {
            p_pQReq->SendDR(u8Res);
        }

        LOG_INFO_APP("Gateway::CacheBasedDRMCheck MatchCacheBasedDRM=" << (int)u8Res
                     << ", pCmdReq=" << pCmdReq << ", pReq=" << pReq);
        return false;
    }

    /// on wired and without DRM just drop message, the HartServer seems to expect this
    if (config.m_nDrmType != GatewayConfig::DRM_TYPE_NONE)
    {
        p_pQReq->SendDR(RCS_E33_DelayedResponseInitiated);
    }
    return true;
}

WHartHandle Gateway::transmitGwRequest(CGwRequest::Ptr & p_pQReq)
{
    if (stack.GetState() != GatewayStack::stOperational)
    {
        LOG_ERROR_APP("Gateway::transmitGwRequest: stack.State != GatewayStack::stOperational; stack.State=" << stack.GetState());
        return 0;
    }

    if (p_pQReq->m_nRequestStatus != CGwRequest::RequestStatus_PendingSend)
    {
        return 0;
    }

    CService::Ptr pService = p_pQReq->m_pService.lock();

    if (!pService || pService->m_nServiceStatus == CService::ServiceStatus_Deleted)
    {
        LOG_ERROR_APP("Gateway::transmitGwRequest: nick=" << std::hex << p_pQReq->m_u16NickNameToSendTo << " service was deleted");

        // if source host send NACK
        p_pQReq->SendDR_DEAD();
        p_pQReq->m_nRequestStatus = CGwRequest::RequestStatus_Cancel;
        return 0;
    }

    if (!pService->IsSendAllowed())
    {
        // LOG_DEBUG_APP("Gateway::transmitGwRequest: IsSendAllowed=0 for Req=" << p_pQReq << ", Service=" << *pService);
        return 0;
    }

    CLogicalDevice::Ptr pLogicalDevice = m_oDevices.GetLogicalDevice(p_pQReq->m_oDevUniqueID, false);
    if (!pLogicalDevice)
    {
        LOG_ERROR_APP("Try to send request for an invalid device(NULL); DeviceUniqueID=" << p_pQReq->m_oDevUniqueID << ", NickNameToSendTo=" << p_pQReq->m_u16NickNameToSendTo << ", req=" << p_pQReq);
        p_pQReq->SendDR_DEAD();
        p_pQReq->m_nRequestStatus = CGwRequest::RequestStatus_Cancel;
        return 0;
    }

    if ((pLogicalDevice->m_nJoinStatus & JoinProcessStatusMask_NormalOperationCommencing) != JoinProcessStatusMask_NormalOperationCommencing)
    {
        LOG_INFO_APP("Try to send request for a device=" << p_pQReq->m_oDevUniqueID << " with join status="
                     << pLogicalDevice->m_nJoinStatus << " (required join status=" << JoinProcessStatusMask_NormalOperationCommencing << "), req=" << p_pQReq);
        return 0;
    }

    if (!p_pQReq->m_bByPassIOCache)
    {
        if (!pLogicalDevice)
        {
            LOG_ERROR_APP("Gateway::transmitGwRequest: nick=" << p_pQReq->m_u16NickNameToSendTo << " logical device is NULL");

            p_pQReq->SendDR_DEAD();
            p_pQReq->m_nRequestStatus = CGwRequest::RequestStatus_Cancel;
            return 0;
        }
        if (GetResponsesFromCache(p_pQReq, pLogicalDevice))
        {
            p_pQReq->SendRspList();
            p_pQReq->m_nRequestStatus = CGwRequest::RequestStatus_ResponseReceived;
            return 0;
        }
    }

    CHartCmdWrapperList::iterator itCmds = p_pQReq->m_oCmdList.begin();
    int i = 0;
    CHartCmdWrapperList oToSendList;

    for (; itCmds != p_pQReq->m_oCmdList.end() && i < config.m_nMaxCmdsPerAPDU; ++itCmds, ++i)
    {
        oToSendList.push_back(*itCmds);
    }

    if (oToSendList.empty())
    {
        LOG_ERROR_APP("transmitGwRequest: empty cmd list for request: " << p_pQReq);
        p_pQReq->m_nRequestStatus = CGwRequest::RequestStatus_ResponseReceived; // just in case
        return 0;
    }

    pLogicalDevice->RegisterStxMessagesSentToDev((uint16_t) (oToSendList.size()));

    LOG_INFO_APP("TransmitRequest: " << p_pQReq << " cmds=" << CHartCmdWrapperListShortDetails(oToSendList) << " nick=" << std::hex << p_pQReq->m_u16NickNameToSendTo
                << " serviceId=" << std::dec << (int)pService->m_stServiceEntry.m_ucServiceId);
    //maybe the nick should be verified recalculate every send on sub devices commands
    WHartHandle nHandle = stack.TransmitRequest(p_pQReq->m_u16NickNameToSendTo, whartpCommand, wharttRequestUnicast,
                                                pService->m_stServiceEntry.m_ucServiceId, oToSendList,
                                                WHartSessionKey::sessionKeyed);

    p_pQReq->MarkTrySend(nHandle, config.GwReqTimeout());

    if (nHandle)
    {
        m_oRspHandles[p_pQReq->m_nHandle] = p_pQReq;

        pService->NotifyTransmitRequest();
    }
    else
    {
        // TODO: Beni - if the transmit for current request fail; the first retry transmit fail; ... => while
        LOG_WARN_APP("TransmitRequest fail for " << p_pQReq << " cmds=" << CHartCmdWrapperListShortDetails(oToSendList) << " nick=" << std::hex << p_pQReq->m_u16NickNameToSendTo
                    << " serviceId=" << std::dec << (int)pService->m_stServiceEntry.m_ucServiceId);
    }

    return nHandle;
}

void Gateway::Notify_WriteSession(const C963_WriteSession_Req * req, const C963_WriteSession_Resp * res)
{
    cold_start = false;
    WHartAddress oTmpAddr(req->m_aPeerUniqueID);
    LOG_INFO_APP("NOTIFY_WRITESESSION : cold_start set to false");
    LOG_INFO_APP("NOTIFY_WRITESESSION : Notify_WriteSession " << oTmpAddr);
    CLogicalDevice::Ptr pDevice = m_oDevices.DeviceAdd(oTmpAddr.address.uniqueID, req->m_unPeerNickname);

    if (!pDevice)
    {
        LOG_ERROR_APP("Gateway::Notify_WriteSession: peer=" << GetHex(req->m_aPeerUniqueID, sizeof(req->m_aPeerUniqueID)) << ", nick=" << std::hex << req->m_unPeerNickname);
        return;
    }

    if (req->m_aPeerUniqueID == NetworkManager_UniqueID())
    {
        m_oDevices.RaiseNotifListChanged();
    }

    if (config.m_nGwRequestService)
    {
        CGwRequest::Ptr pReq = pDevice->CheckNeedForIdInfo();

        if (pReq)
        {
            transmitGwRequest(pReq);
        }
    }
}

void Gateway::Notify_DeleteSession(const C964_DeleteSession_Req * req, const C964_DeleteSession_Resp * res)
{
    WHartAddress oTmpAddr(req->m_unPeerNickname);
    LOG_INFO_APP("Notify_DeleteSession " << oTmpAddr);
    CLogicalDevice::Ptr pDevice = m_oDevices.GetLogicalDevice(oTmpAddr);

    if (!pDevice)
    {
        //just in case
        LOG_ERROR_APP("Notify_DeleteSession device does not exist" << oTmpAddr);
        return;
    }

    pDevice->SetActive(false);
}

void Gateway::sendChangeNotification(NotificationMaskCodesMasks p_notificationType)
{
    // for each device
    BOOST_FOREACH(const CLogicalDeviceTable::CUniqueIDMap::value_type & rUniqueIdMapItem, m_oDevices.GetDeviceUniqueIDMap())
    {
        const CLogicalDevice::Ptr & rLogicalDevice = rUniqueIdMapItem.second;
        uint16_t deviceNotifMask = rLogicalDevice->m_u16NotificationMask;

        if ( (deviceNotifMask & p_notificationType) == 0)
        {
            continue;
        }

        LOG_INFO_APP ("sendChangeNotification device=" << WHartAddress(rLogicalDevice->m_oDeviceUniqueID) << " raised notif="
                       << std::hex << (int)deviceNotifMask << ", notificationType=" << p_notificationType);

        // for each notification session
        BOOST_FOREACH(const CHostAppSessionTransIdMap::value_type & itSession, m_oHostAppSessions.GetSessionsTable())
        {
            const CHostAppSession::Ptr & session = itSession.second;

            CHostAppDevNotifMap::iterator devicePos;
            uint8_t sessionNotifMask = 0;

            if ( p_notificationType == NotificationMaskCodesMask_NetworkSchedule || p_notificationType == NotificationMaskCodesMask_NetworkTopology )
            {
                // for network topology and schedule notifications and commands are distributed on target/source devices, but masks on sessions notification are only on NM
                devicePos = session->m_oDevNotifMaskMap.find(NetworkManager_UniqueID());
                if (devicePos != session->m_oDevNotifMaskMap.end())
                {
                    sessionNotifMask |= devicePos->second;
                }
                devicePos = session->m_oDevNotifMaskMap.find(Gateway_UniqueID());
                if (devicePos != session->m_oDevNotifMaskMap.end())
                {
                    sessionNotifMask |= devicePos->second;
                }
            }
            else
            {
                devicePos = session->m_oDevNotifMaskMap.find(rUniqueIdMapItem.first);
                if (devicePos != session->m_oDevNotifMaskMap.end())
                {
                    sessionNotifMask = devicePos->second;
                }
                LOG_DEBUG_APP("Device " << rUniqueIdMapItem.first << ", sessionNotifMask=" << (int)sessionNotifMask << ", Session=" << *session->m_oRawSession);

                /*TODO - temporary workaround -apply adapter burst mask on subdevs until mh will make notif subscription for subdevs*/
                if (sessionNotifMask == 0 && rLogicalDevice->m_pSubDeviceInfo != 0)
                {
                    CLogicalDevice * m_pAdapter = rLogicalDevice->m_pSubDeviceInfo->m_pAdapter;
                    if (m_pAdapter != 0)
                    {
                        CHostAppDevNotifMap::iterator adapterPos = session->m_oDevNotifMaskMap.find(m_pAdapter->m_oDeviceUniqueID);
                        if (adapterPos != session->m_oDevNotifMaskMap.end())
                        {
                            LOG_INFO_APP("Adapter " << m_pAdapter->m_oDeviceUniqueID << ", notif mask=" << (int)adapterPos->second << ", Session=" << *session->m_oRawSession);
                            sessionNotifMask |= (adapterPos->second & NotificationMaskCodesMask_BurstMode);
                        }
                        else
                        {
                        	// modbus session :: m_oDevNotifMaskMap might be empty
                            LOG_INFO_APP("Adapter " << m_pAdapter->m_oDeviceUniqueID << " not found in DevNotifyMaskMap. Session=" << *session->m_oRawSession);
                        }
                    }
                    else
                    {
                        LOG_WARN_APP("Adapter is NULL for " << rLogicalDevice->m_oDeviceUniqueID << ", Session=" << *session->m_oRawSession);
                    }
                }
                /**/
            }

            // the device has a notif. activated, and the host wants the notification
            if ( (sessionNotifMask & p_notificationType) != 0)
            {
                C839_ChangeNotification_Resp oResp;
                memcpy( oResp.DeviceAddress, rLogicalDevice->m_oDeviceUniqueID.bytes, sizeof(_device_address_t) );
                oResp.ChangeNotificationNo = 0;
                switch( p_notificationType )
                {
                    case NotificationMaskCodesMask_BurstMode:
                        BOOST_FOREACH(unsigned int cmdId, rLogicalDevice->m_oNotifBurstCmds)
                        {
                            if (oResp.ChangeNotificationNo < MaxChangeNotifications)
                                oResp.ChangeNotifications[oResp.ChangeNotificationNo++] = cmdId;
                            else
                                LOG_ERROR_APP("ChangeNotificationNo(" << (int)oResp.ChangeNotificationNo << ") >= MaxChangeNotifications(" << (int)MaxChangeNotifications << ")");
                        }
                        break;
                    case NotificationMaskCodesMask_EventNotification:
                        oResp.ChangeNotifications[oResp.ChangeNotificationNo++] = CMDID_C119_AcknowledgeEventNotification;
                        break;
                    case NotificationMaskCodesMask_DeviceStatus:
                        oResp.ChangeNotifications[oResp.ChangeNotificationNo++] = CMDID_C000_ReadUniqueIdentifier;
                        break;
                    case NotificationMaskCodesMask_DeviceConfiguration:
                        LOG_DEBUG_APP("Device " << rLogicalDevice->m_oDeviceUniqueID << ", ConfChangeCounter=" << config.m_u16ConfigChangeCounter << ", NotificationMaskCodesMask_DeviceConfiguration processing ...");
                        oResp.ChangeNotifications[oResp.ChangeNotificationNo++] = CMDID_C000_ReadUniqueIdentifier;
                        break;
                    case NotificationMaskCodesMask_NetworkTopology:
                        LOG_DEBUG_APP("Topology size=" <<(int) rLogicalDevice->m_oNotifNetworkTopology.size());
                        BOOST_FOREACH(unsigned int cmdId, rLogicalDevice->m_oNotifNetworkTopology)
                        {
                            if (oResp.ChangeNotificationNo < MaxChangeNotifications)
                                oResp.ChangeNotifications[oResp.ChangeNotificationNo++] = cmdId;
                            else
                                LOG_ERROR_APP("ChangeNotificationNo(" << (int)oResp.ChangeNotificationNo << ") >= MaxChangeNotifications(" << (int)MaxChangeNotifications << ")");
                        }
                        break;
                    case NotificationMaskCodesMask_NetworkSchedule:
                        LOG_DEBUG_APP("Schedule size=" <<(int) rLogicalDevice->m_oNotifNetworkSchedule.size());
                        BOOST_FOREACH(unsigned int cmdId, rLogicalDevice->m_oNotifNetworkSchedule)
                        {
                            if (oResp.ChangeNotificationNo < MaxChangeNotifications)
                                oResp.ChangeNotifications[oResp.ChangeNotificationNo++] = cmdId;
                            else
                                LOG_ERROR_APP("ChangeNotificationNo(" << (int)oResp.ChangeNotificationNo << ") >= MaxChangeNotifications(" << (int)MaxChangeNotifications << ")");
                        }
                        break;
                }

                // send Notification to host
                CHartCmdWrapper::Ptr pRsp(new CHartCmdWrapper);
                pRsp->LoadParsed(CMDID_C839_ChangeNotification, sizeof(C839_ChangeNotification_Resp), &oResp, RCS_N00_Success);

                pRsp->GetRawFromParsed();

                CGwRequest::SendNotification(session.get(), pRsp);
            }
        }

        // cleanup
        rLogicalDevice->m_u16NotificationMask &= ~p_notificationType;
        switch ( p_notificationType )
        {
            case NotificationMaskCodesMask_BurstMode:
                //burst notif. also notifies about DeviceConfiguration and DeviceStatus. so unset these notification flags
                rLogicalDevice->m_u16NotificationMask &= ~( NotificationMaskCodesMask_DeviceStatus | NotificationMaskCodesMask_DeviceConfiguration );
                rLogicalDevice->m_oNotifBurstCmds.clear();
                break;
            case NotificationMaskCodesMask_NetworkTopology :
                rLogicalDevice->m_oNotifNetworkTopology.clear();
                break;
            case NotificationMaskCodesMask_NetworkSchedule :
                rLogicalDevice->m_oNotifNetworkSchedule.clear();
                break;
            default:
                break;
        }
    }
}

void Gateway::sendIndicateDirectly(const WHartAddress & p_rSrcAddr, uint8_t p_u8DeviceStatus, uint8_t p_u8DeviceStatusExt, const CHartCmdWrapperList & p_oResponses)
{
    const WHartUniqueID * pSrcUniqueId = NULL;

    if (p_rSrcAddr.type == WHartAddress::whartaUniqueID)
    {
        pSrcUniqueId = & p_rSrcAddr.address.uniqueID;
    }
    else
    {
        CLogicalDevice::Ptr pDev = m_oDevices.GetLogicalDevice(p_rSrcAddr, false);
        if (pDev == 0)
        {
            LOG_ERROR_APP("sendIndicateDirectly: device=" << p_rSrcAddr << " does not exist" );
            return;
        }
        pSrcUniqueId = &pDev->m_oDeviceUniqueID;
    }

    BOOST_FOREACH(const CHostAppSessionTransIdMap::value_type & itSession, m_oHostAppSessions.GetSessionsTable())
    {
        const CHostAppSession::Ptr & session = itSession.second;

        CHartCmdWrapperList oIndList;

        WHartUniqueID oTmpUnique;

        CHartCmdWrapperList::const_iterator itRsp = p_oResponses.begin();
        for (;itRsp != p_oResponses.end(); itRsp++)
        {
            const CHartCmdWrapper::Ptr pRsp = *itRsp;
            LOG_DEBUG("Gateway::sendIndicateDirectly rsp=" << pRsp);

            if ((*itRsp)->MetaCmdInfoGet() != 0)
            {
                memcpy (oTmpUnique.bytes, (*itRsp)->MetaCmdInfoGet()->m_oInnerCmdUniqueId.bytes, sizeof(oTmpUnique.bytes));
                pSrcUniqueId = &oTmpUnique;
            }

            if (IsNmAlarmCmd(pRsp->GetCmdId()) || IsNmNotifAsBurstCmd(pRsp->GetCmdId()))
            {
                if (session->m_nNivisSpecificCmdRecv)
                {
                    oIndList.push_back(pRsp);
                }
            }
            else if (session->IsWiredFormat())
            {
                if (!IsNetworkManager(p_rSrcAddr) && config.m_bSendDirectWiredDeviceBurst)
                {
                    oIndList.push_back(pRsp);
                }
            }
        }
        if (!oIndList.empty() && pSrcUniqueId)
        {
            CGwRequest::SendHostIndicate(session.get(), *pSrcUniqueId, oIndList);
        }
        else
        {
        	LOG_DEBUG("Replace Host Forward with Host Notify");
        }
    }
}

// returns true if all commands were found in cache
CLogicalDevice* Gateway::GetDeviceToSendTo(CGwRequest::Ptr & p_pReq, CLogicalDevice::Ptr & p_pLogicalDevice)
{
    CHartCmdWrapper::Ptr pCmdReq = *p_pReq->m_oCmdList.begin(); // use only the first to identity

    CLogicalDevice* pLogicalDeviceToSendTo = NULL;

    if ( pCmdReq->MetaCmdInfoGet() )
    {
        //meta as C077 or Nivis Meta explicitly received from App Host
        pLogicalDeviceToSendTo = m_oDevices.GetLogicalDevice(pCmdReq->MetaCmdInfoGet()->m_oMetaCmdUniqueId).get();

        if (!pLogicalDeviceToSendTo)
        {
            LOG_ERROR_APP("NO logical device to send to meta cmd");
        }
        return pLogicalDeviceToSendTo;
    }

    if (!p_pLogicalDevice)
    {
        LOG_ERROR_APP("device not found -> send DR_DEAD back");
        return NULL;
    }

    if (p_pLogicalDevice->m_pSubDeviceInfo)
    {
        if (!p_pLogicalDevice->m_pSubDeviceInfo->m_pAdapter)
        {
            LOG_ERROR_APP( "subdevice exist" << p_pReq->m_oDevUniqueID << " but no adapter");
            return NULL;
        }

        pLogicalDeviceToSendTo = p_pLogicalDevice->m_pSubDeviceInfo->m_pAdapter;

        TMetaCmdsInfo oMetaInfo;

        oMetaInfo.m_u16MetaCmdId = CMDID_C077_SendCommandToSubDevice;
        oMetaInfo.m_oMetaCmdUniqueId = pLogicalDeviceToSendTo->m_oDeviceUniqueID;

        oMetaInfo.m_oCmdSubDevice.m_nCard = p_pLogicalDevice->m_pSubDeviceInfo->m_oSubDeviceKey.ioCard;
        oMetaInfo.m_oCmdSubDevice.m_nChannel = p_pLogicalDevice->m_pSubDeviceInfo->m_oSubDeviceKey.channel;
        oMetaInfo.m_oCmdSubDevice.m_nDevicePollingAddress = p_pLogicalDevice->m_pSubDeviceInfo->m_oSubDeviceKey.poolingAddress;
        oMetaInfo.m_oCmdSubDevice.m_nTransmitPreambleCount = ( p_pLogicalDevice->m_pC000_ReadUniqueIdentifier_Resp == 0 ) ? 5:(p_pLogicalDevice->m_pC000_ReadUniqueIdentifier_Resp->minReqPreamblesNo);
        oMetaInfo.m_oCmdSubDevice.m_nDelimiter = (HART_DELIM_FRAME_TYPE_M2D) | (config.UseSubdevPollingAddresses() ? HART_DELIM_ADDR_TYPE_SHORT : HART_DELIM_ADDR_TYPE_LONG);

        oMetaInfo.m_oInnerCmdUniqueId = p_pReq->m_oDevUniqueID;

        CmdList_MetaCmdInfoSet(p_pReq->m_oCmdList,&oMetaInfo);
        return pLogicalDeviceToSendTo;
    }

    if (p_pReq->m_bByPassIOCache || !IsCommadToRedirectToSM(p_pReq->m_oDevUniqueID, pCmdReq->GetCmdId()))
    {
        return p_pLogicalDevice.get();
    }

    LOG_INFO_APP("Redirect cmd " << (int)(pCmdReq->GetCmdId()) << " to NM");
    pLogicalDeviceToSendTo = m_oDevices.GetLogicalDeviceNM().get();

    if (!pLogicalDeviceToSendTo)
    {
        LOG_ERROR_APP("NO logical device NM ");

        return NULL;
    }

    CmdList_MetaCmdInfoSet_ToNM(p_pReq->m_oCmdList,p_pReq->m_oDevUniqueID);

    return pLogicalDeviceToSendTo;
}

// returns true if all commands were found in cache
bool Gateway::GetResponsesFromCache(CGwRequest::Ptr & p_pReq, CLogicalDevice::Ptr & p_pLogicalDevice)
{
    if (p_pReq->m_bByPassIOCache || !p_pLogicalDevice)
    {
        return false;
    }

    for (;! p_pReq->m_oCmdList.empty(); )
    {
        CHartCmdWrapper::Ptr pCmd = p_pReq->m_oCmdList.front();

        CHartCmdWrapper::Ptr pRsp;

        uint8_t u8RspCode = RCS_N00_Success;
        bool ret = p_pLogicalDevice->ResponseGet(pCmd, pRsp, u8RspCode);

        if (!ret)
        {
            LOG_INFO_APP("Gateway::GetResponseFromCache - did not find resp in cache for " << CHartCmdWrapperShortDetails(*pCmd));
            return false;
        }

        if (config.m_bSendInvalidRequestToDevice)
        {
            if (u8RspCode == RCS_E05_TooFewDataBytesReceived)
            {
                LOG_WARN_APP("Gateway::GetResponseFromCache - RCS_E05_TooFewDataBytesReceived - did not find resp in cache for " << *pCmd);
                pRsp.reset();
                return false;
            }
        }

        LOG_INFO_APP("Gateway::GetResponseFromCache - found resp in cache for " << CHartCmdWrapperShortDetails(*pCmd));

        pRsp->GetRawFromParsed();
        p_pReq->m_oRspList.push_back(pRsp);
        p_pReq->m_oCmdExecList.push_back(pCmd);
        p_pReq->m_oCmdList.pop_front();
    }

    return true;
}

// returns	true	- the command was processed, a response (even error response) is prepared and it will be sent
//		MUST return false only if the command should be fwd
bool Gateway::ExecLocalCmds( CGwRequest::Ptr & p_pReq,
                             CHostAppSession::Ptr & p_pHostSession,
                             CmdProcessorUniversal & p_pCmdUniversal ,
                             CmdProcessorWirelessHART & p_pCmdWirelessHART,
                             CmdProcessorCommonPractice & p_pCmdCommonPractice)
{
    CHartCmdWrapperList::iterator itCmd = p_pReq->m_oCmdList.begin();

    for (;itCmd != p_pReq->m_oCmdList.end();)
    {
        CHartCmdWrapper::Ptr pCmd = *itCmd;

        if (p_pReq->m_oDevUniqueID == Gateway_UniqueID() && (IsCommadToRedirectToSM(p_pReq->m_oDevUniqueID, pCmd->GetCmdId()) || IsCommadForNM(pCmd->GetCmdId())))
        {
            ++itCmd;
            continue;
        }

        if (p_pReq->m_oDevUniqueID == NetworkManager_UniqueID() && !IsCommadForGw(pCmd->GetCmdId()) && !IsCommadDualForGwOrNM(pCmd->GetCmdId()) )
        {
            ++itCmd;
            continue;
        }

        CHartCmdWrapper::Ptr pRsp;
        if (!ExecLocalCmd(pCmd, pRsp, p_pHostSession, m_oCmdsProcessUniversal, m_oCmdsProcessWirelessHART, m_oCmdsProcessCommonPractice))
        {
            LOG_DEBUG_APP("Gateway::ExecLocalCmds: FWD command to SM" << pCmd->GetCmdId());
            ++itCmd;
            continue;
        }

        if (pRsp)
        {
            p_pReq->m_oRspList.push_back(pRsp);
        }

        p_pReq->m_oCmdExecList.push_back(pCmd);
        itCmd = p_pReq->m_oCmdList.erase(itCmd);
    }

    if (!p_pReq->m_oCmdList.empty())
    {
        LOG_INFO_APP("Gateway::ExecLocalCmds: FWD commands to SM" << CHartCmdWrapperListShortDetails(p_pReq->m_oCmdList));
        LOG_DEBUG_APP("Gateway::ExecLocalCmds: FWD commands to SM" << p_pReq->m_oCmdList);

        if(p_pReq->m_oDevUniqueID == Gateway_UniqueID())
        {
            CmdList_MetaCmdInfoSet_ToNM(p_pReq->m_oCmdList, p_pReq->m_oDevUniqueID);
        }

        return false;
    }

    return true;
}

// returns	true	- the command was processed, a response (even error response) is prepared and it will be sent
//		MUST return false only if the command should be fwd
bool Gateway::ExecLocalCmd(CHartCmdWrapper::Ptr p_pCmd,
                           CHartCmdWrapper::Ptr & p_pRsp,
                           CHostAppSession::Ptr & p_pHostSession,
                           CmdProcessorUniversal & p_pCmdUniversal,
                           CmdProcessorWirelessHART & p_pCmdWirelessHART,
                           CmdProcessorCommonPractice & p_pCmdCommonPractice)
{
    bool bRet = false;
    //only one cmd

    uint8_t u8RspCode = p_pCmd->GetParsedFromRaw();
    if (u8RspCode != RCS_N00_Success)
    {
        //send error back
        LOG_DEBUG_APP("ExecLocalCmd: parsing error");
        p_pRsp.reset(new CHartCmdWrapper);

        p_pRsp->LoadParsed(p_pCmd->GetCmdId(), 0, NULL, u8RspCode);
        p_pRsp->SetDeviceStatus(config.DeviceStatus());

        return true;
    }

    if (p_pCmd->GetCmdId() == CMDID_C832_ReadNetworkDeviceIdentity || p_pCmd->GetCmdId() == CMDID_C841_ReadNetworkDeviceIdentityByNickname )
    {
        bRet = p_pCmdWirelessHART.ExecCmdNmOrGwDualCmds(p_pCmd, p_pRsp, p_pHostSession);
    }
    else if(((p_pCmd->GetCmdId() >= 0) && (p_pCmd->GetCmdId() <= 22)) || (p_pCmd->GetCmdId() == 38) || (p_pCmd->GetCmdId() == 48))
    {
        bRet = p_pCmdUniversal.ExecLocalCmdUniversal(p_pCmd, p_pRsp, p_pHostSession);
    }
    else if((p_pCmd->GetCmdId() >= 768) && (p_pCmd->GetCmdId() <= 1023))
    {
        bRet = p_pCmdWirelessHART.ExecLocalCmdWirelessHART(p_pCmd, p_pRsp, p_pHostSession);
    }
    else if ((p_pCmd->GetCmdId() >= 32 ) && (p_pCmd->GetCmdId() <= 121) /*&& (p_pCmd->GetCmdId != 38) && (p_pCmd->GetCmdId !=48)*/)
    {
        bRet = p_pCmdCommonPractice.ExecLocalCmdCommonPractice(p_pCmd, p_pRsp, p_pHostSession);
    }
    else
    {
        LOG_WARN_APP("ExecLocalCmd: cmdId=" << p_pCmd->GetCmdId() << " CommandNotImplemented ");
        p_pRsp.reset(new CHartCmdWrapper);
        p_pRsp->LoadParsed (p_pCmd->GetCmdId(), 0, 0, RCS_E64_CommandNotImplemented);
        bRet = true;
    }

    if (bRet && p_pRsp)
    {
        p_pRsp->GetRawFromParsed();
        p_pRsp->SetDeviceStatus(config.DeviceStatus());

        if (p_pCmd->GetCmdId() == CMDID_C048_ReadAdditionalDeviceStatus)
        {
            config.DeviceStatus(config.DeviceStatus() & (~FieldDeviceStatusesMask_MoreStatusAvailable));
            config.m_u8standardizedStatus2 &= (~StandardizedStatus2Mask_SubDeviceListChanged);
        }

        if (IsGwDeviceChanged(p_pCmd->GetCmdId()))
        {
            CLogicalDevice::Ptr pGw = m_oDevices.GetLogicalDeviceGW();
            if (!pGw)
            {
                LOG_ERROR_APP("Programmer ERROR -- no gw in device list");
            }
            else
            {
                config.m_u16ConfigChangeCounter++;
                m_oDevices.RaiseNotifConfigCounterChanged();
            }
        }
    }

    return bRet;
}

bool Gateway::IsGwDeviceChanged(uint16_t cmdId)
{
    if ( // Universal Command
        cmdId == 6 || cmdId == 17 || cmdId == 18 || cmdId == 19 || cmdId == 22

        // Common Practice Command pag. 12/172
        || (cmdId >= 34 && cmdId <= 37) || (cmdId >= 43 && cmdId <= 47) || cmdId == 49 || (cmdId >= 51 && cmdId <= 53)
            || (cmdId >= 55 && cmdId <= 56) || cmdId == 59 || cmdId == 79 || (cmdId >= 82 && cmdId <= 83) || (cmdId >= 87 && cmdId <= 88)
            || cmdId == 92 || cmdId == 97 || cmdId == 99 || (cmdId >= 102 && cmdId <= 104) || (cmdId >= 107 && cmdId <= 109)
            || cmdId == 113 || (cmdId >= 116 && cmdId <= 118) || cmdId == 513

        // WirelessHART Command pag. 29/140
        || cmdId == 771 || cmdId == 773 || cmdId == 775 || cmdId == 815 || cmdId == 816 || cmdId == 818 || cmdId == 821
            || cmdId == 837 || cmdId == 842 || cmdId == 845 )
    {
        return true;
    }
    return false;
}

// associated with cmd 42 flow
void Gateway::Reset_GW()
{
    LOG_INFO_APP("Gateway::Reset_GW() - reset Gateway");
    Notify_Rejoin();
}

void Gateway::Notify_Rejoin()
{
    LOG_INFO_APP("Gateway::Notify_Rejoin - reset all app level and clear requests");

    CGwRequests::iterator itReqs = m_oRequests.begin();

    for (;itReqs != m_oRequests.end();)
    {
        CGwRequests::iterator itDel = itReqs;
        itReqs++;
        EraseGwRequest(itDel);
    }

    m_oDevices.SetInactive();

    if (m_oHostAppServer) // close all MH sessions
    {
        ((transport::UDPServerIP*)m_oHostAppServer.get())->CloseAllSessions();
    }
}

void Gateway::UpdateGwNotifications(uint32_t p_nCmdId)
{
    uint32_t notifCmd;

    switch( p_nCmdId )
    {
        case CMDID_C965_WriteSuperframe:
        case CMDID_C966_DeleteSuperframe:
            notifCmd = CMDID_C783_ReadSuperframeList;
            break;
        case CMDID_C967_WriteLink:
        case CMDID_C968_DeleteLink:
            notifCmd = CMDID_C784_ReadLinkList;
            break;
        case CMDID_C973_WriteService:
        case CMDID_C801_DeleteService:
            notifCmd = CMDID_C800_ReadServiceList;
            break;
        case CMDID_C974_WriteRoute:
        case CMDID_C975_DeleteRoute:
        case CMDID_C976_WriteSourceRoute:
        case CMDID_C977_DeleteSourceRoute:
            notifCmd = CMDID_C802_ReadRouteList;
            break;
        default:
            return;
    }

    m_oDevices.GetLogicalDeviceGW()->m_oNotifNetworkSchedule.insert(notifCmd);
}

void Gateway::logInternalRequestsStatistics()
{
    std::map<uint16_t, int> requestsCounters;
    std::map<uint16_t, int> respondedSoFarCounters;

    for (CGwRequests::iterator reqIt = m_oRequests.begin(); reqIt != m_oRequests.end(); ++reqIt)
    {
        for (CHartCmdWrapperList::iterator cmdIt = (*reqIt)->m_oCmdList.begin(); cmdIt != (*reqIt)->m_oCmdList.end(); ++cmdIt)
        {
            uint16_t cmd = (*cmdIt)->GetCmdId();
            std::map<uint16_t, int>::iterator requestsCountersIt = requestsCounters.find(cmd);
            if (requestsCountersIt == requestsCounters.end())
            {
                requestsCounters[cmd] = 0;
            }
            else
            {
                requestsCountersIt->second++;
            }
        }

        for (CHartCmdWrapperList::iterator cmdIt = (*reqIt)->m_oRspList.begin(); cmdIt != (*reqIt)->m_oRspList.end(); ++cmdIt)
        {   uint16_t cmd = (*cmdIt)->GetCmdId();
            std::map<uint16_t, int>::iterator respondedSoFarCountersIt = respondedSoFarCounters.find(cmd);
            if (respondedSoFarCountersIt == respondedSoFarCounters.end())
            {
                respondedSoFarCounters[cmd] = 0;
            }
            else
            {
                respondedSoFarCountersIt->second++;
            }
        }
    }

    std::ostringstream str;
    str << std::endl << "********Gateway::RequestsListStatistics********";
    str << "\n\tRequests.size=" << m_oRequests.size();
    for (std::map<uint16_t, int>::iterator reqCountersIt = requestsCounters.begin(); reqCountersIt != requestsCounters.end(); ++reqCountersIt)
    {
        std::map<uint16_t, int>::iterator respondedSoFarCountersIt = respondedSoFarCounters.find(reqCountersIt->first);
        int respondedSoFar = (respondedSoFarCountersIt == respondedSoFarCounters.end()) ? 0 : respondedSoFarCountersIt->second;
        str << "\n\t\tCmd " << (int)(reqCountersIt->first) << " "
            << reqCountersIt->second << " requests"
            << " and " << respondedSoFar << " responded so far";
    }
    str << std::endl << "*************************************" << std::endl;
    LOG_INFO_APP(str.str());
}

void Gateway::logInternalStatus()
{
    if (!IsLogEnabled_APP(LL_INFO))
    {
        return;
    }

    std::ostringstream str;

    str << std::endl << "********Gateway::LogInternalStatus********" << std::endl;

    str << "[LogInternalStatus] Gw : "
        << "\n\tDevices.size = " << m_oDevices.GetDeviceUniqueIDMap().size()
        << "\n\tRequests.size = " << m_oRequests.size()
        << "\n\tHostAppSessions.size = " << m_oHostAppSessions.GetSessionsTable().size();

    BOOST_FOREACH(const CLogicalDeviceTable::CUniqueIDMap::value_type& rUniqueIdMapItem, m_oDevices.GetDeviceUniqueIDMap())
    {
        const CLogicalDevice::Ptr& rLogicalDevice = rUniqueIdMapItem.second;
        str << "\n[LogInternalStatus] device " << std::hex << (int)(rLogicalDevice->GetNick()) << " : " << std::dec
            << "\n\tLocalRequests.size = " << rLogicalDevice->m_oLocalRequests.size()
            << "\n\tCachedResponses.size = " << rLogicalDevice->m_oCachedResponses.size()
            << "\n\tBurstResponses.size = " << rLogicalDevice->m_oBurstResponses.size();

        CBurstResponseMap::iterator it = rLogicalDevice->m_oBurstResponses.begin();
        for (; it != rLogicalDevice->m_oBurstResponses.end(); ++it )
        {
            str << "\n\t\tBurstResponses[ " << (int)(it->first) << " ].size() = " << it->second->size();
        }

        str << "\n\tNotifBurstCmds.size = " << rLogicalDevice->m_oNotifBurstCmds.size()
            << "\n\tNotifNetworkTopology.size = " << rLogicalDevice->m_oNotifNetworkTopology.size()
            << "\n\tNotifNetworkSchedule.size = " << rLogicalDevice->m_oNotifNetworkSchedule.size();

        dumpServicesToStream(rLogicalDevice, str);
    }

    BOOST_FOREACH(const CHostAppSessionTransIdMap::value_type& itSession, m_oHostAppSessions.GetSessionsTable())
    {
        const CHostAppSession::Ptr& session = itSession.second;
        str << "\n[LogInternalStatus] session - Requests size = " << session->m_oRequests.size();
    }

    str << std::endl << "*************************************" << std::endl;

    LOG_INFO_APP(str.str());
}

void Gateway::logDevicesStatistics()
{
    //foreach device
    std::ostringstream deviceInfo;

    BOOST_FOREACH(const CLogicalDeviceTable::CUniqueIDMap::value_type& rUniqueIdMapItem, m_oDevices.GetDeviceUniqueIDMap())
    {
        const CLogicalDevice::Ptr& rLogicalDevice = rUniqueIdMapItem.second;

        deviceInfo << "\nDevice : " << std::hex
                    << rLogicalDevice->m_oDeviceUniqueID
                    << " [Nickname=" << rLogicalDevice->GetNick() << "] " << std::dec;

        deviceInfo << "\n\ttype=" << rLogicalDevice->m_nDeviceType;
        deviceInfo << ", has0=" << (rLogicalDevice->m_pC000_ReadUniqueIdentifier_Resp != 0);
        if (rLogicalDevice->m_pC000_ReadUniqueIdentifier_Resp != 0)
        {
            deviceInfo << ", hartVersion=" << (int)(rLogicalDevice->m_pC000_ReadUniqueIdentifier_Resp->protocolMajorRevNo);
        }

        deviceInfo << ", has20=" << (rLogicalDevice->m_pC020_ReadLongTag_Resp != 0);
        deviceInfo << ", has48=" << (rLogicalDevice->m_pC048_ReadAdditionalDeviceStatus_Resp != 0);
        deviceInfo << ", notifMask=" << std::hex << rLogicalDevice->m_u16NotificationMask << std::dec;
        deviceInfo << ", m_enumJoinStatus=" << rLogicalDevice->m_nJoinStatus;
        deviceInfo << ", isActive=" << rLogicalDevice->IsActive();

        deviceInfo << "\n\t\tCachedBurstResponses[";
        CBurstResponseMap::iterator itBurst = rLogicalDevice->m_oBurstResponses.begin();
        for (; itBurst != rLogicalDevice->m_oBurstResponses.end(); ++itBurst )
        {
            deviceInfo << (int)(itBurst->first) << " (" << itBurst->second->size() << ")";
        }
        deviceInfo << " ] ";

        deviceInfo << "\n\t\tCachedReadResponses[ ";
        CCachedResponseMap::iterator itRead = rLogicalDevice->m_oCachedResponses.begin();
        for (; itRead != rLogicalDevice->m_oCachedResponses.end(); ++itRead )
        {
            deviceInfo << (int)(itRead->first) << " ";
        }
        deviceInfo << " ] ";
        if (rLogicalDevice->m_pAdapterInfo != 0)
        {
            deviceInfo << "\n\tisAdapter:";

            if (rLogicalDevice->m_pAdapterInfo->m_pC074_ReadIOSystemCapabilities_Resp != 0)
            {
                deviceInfo << "\n\t\thas74 - noOfIOCards="
                            << (int)(rLogicalDevice->m_pAdapterInfo->m_pC074_ReadIOSystemCapabilities_Resp->noOfIOCards)
                            << " noOfChannelsPerIOCard="
                            << (int)(rLogicalDevice->m_pAdapterInfo->m_pC074_ReadIOSystemCapabilities_Resp->noOfChannelsPerIOCard)
                            << " noOfSubdevicesPerChannel="
                            << (int)(rLogicalDevice->m_pAdapterInfo->m_pC074_ReadIOSystemCapabilities_Resp->noOfSubdevicesPerChannel)
                            << " noOfSubDevicesDetected=" /*we don't count the adaptor*/
                            << (int)(rLogicalDevice->m_pAdapterInfo->m_pC074_ReadIOSystemCapabilities_Resp->noOfDevicesDetected - 1)
                            << " noOfDelayedResponsesSupported="
                            << (int)(rLogicalDevice->m_pAdapterInfo->m_pC074_ReadIOSystemCapabilities_Resp->noOfDelayedResponsesSupported)
                            << " masterMode="
                            << (int)(rLogicalDevice->m_pAdapterInfo->m_pC074_ReadIOSystemCapabilities_Resp->masterMode)
                            << " sendRetryCount="
                            << (int)(rLogicalDevice->m_pAdapterInfo->m_pC074_ReadIOSystemCapabilities_Resp->sendRetryCount);

                if (rLogicalDevice->m_pAdapterInfo->m_bDiscoveryInProgress)
                {
                    deviceInfo << "\n\t\tdiscoveryInProgress- ioCard="
                                << ((int)(rLogicalDevice->m_pAdapterInfo->m_oCurrentPollInfo.ioCard))
                                << " channel="
                                << ((int)(rLogicalDevice->m_pAdapterInfo->m_oCurrentPollInfo.channel))
                                << " poolingAddress="
                                << ((int)(rLogicalDevice->m_pAdapterInfo->m_oCurrentPollInfo.poolingAddress));
                }
                else
                {
                    deviceInfo << "\n\t\tdiscoveryInProgress=0";
                }
            }
            else
            {
                deviceInfo << " has74=0";
            }
        }
        else
        {
            deviceInfo << "\n\tis Not Adapter";
        }

        if (rLogicalDevice->m_pSubDeviceInfo != 0)
        {
            deviceInfo << "\n\tisSubdevice :";
            if (rLogicalDevice->m_pSubDeviceInfo->m_pAdapter != 0)
            {
                deviceInfo << "\n\t\tadapter:" << std::hex << rLogicalDevice->m_pSubDeviceInfo->m_pAdapter->m_oDeviceUniqueID
                            << " [Nickname=" << rLogicalDevice->m_pSubDeviceInfo->m_pAdapter->GetNick() << "] " << std::dec;
            }
            else
            {
                deviceInfo << "no Adapter";
            }

            deviceInfo << "\n\t\tsubDeviceInfo:"
                        << " ioCard=" << (int)(rLogicalDevice->m_pSubDeviceInfo->m_oSubDeviceKey.ioCard)
                        << " channel=" << (int)(rLogicalDevice->m_pSubDeviceInfo->m_oSubDeviceKey.channel)
                        << " poolingAddress=" << (int)(rLogicalDevice->m_pSubDeviceInfo->m_oSubDeviceKey.poolingAddress);
        }
        else
        {
            deviceInfo << "\n\tis Not Subdevice";
        }

        dumpServicesToStream(rLogicalDevice, deviceInfo);
    }

    LOG_INFO_APP( deviceInfo.str() );
}

void Gateway::dumpServicesToStream(const CLogicalDevice::Ptr& p_rLogicalDevice, std::ostream& p_oStr)
{
    p_oStr << "\n\tServices: " << p_rLogicalDevice->m_pServiceTable->GetServicesMap().size();

    BOOST_FOREACH(const CServiceDeviceMap::value_type& rServicedMapItem, p_rLogicalDevice->m_pServiceTable->GetServicesMap())
    {
        const CService::Ptr& rService = rServicedMapItem.second;
        p_oStr << "\n\t\tServiceId = " << (int)(rService->m_stServiceEntry.m_ucServiceId)
                << ", Nick = " << std::hex << rService->m_stServiceEntry.m_unNicknameOfPeer << std::dec
                << ", Period = " << (int)(rService->m_stServiceEntry.m_tPeriod.u32)
                << " | " << (int)(rService->m_stServiceEntry.m_tPeriod.u32>>5) <<" ms"
                << ", Domain = " << (int)(rService->m_stServiceEntry.m_ucServiceApplicationDomain)
                << ", Flags = " << std::hex << (int)(rService->m_stServiceEntry.m_ucServiceRequestFlags) << std::endl;
    }
}

} // namespace gateway
} // namespace hart7
