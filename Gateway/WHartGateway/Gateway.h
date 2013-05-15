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
 * Gateway.h
 *
 *  Created on: Dec 15, 2008
 *      Author: nicu.dascalu
 */

#ifndef WHART_GATEWAY_H_
#define WHART_GATEWAY_H_

#include <WHartGateway/GatewayConfig.h>
#include <WHartGateway/GatewayStack.h>
#include <WHartStack/util/WHartCmdWrapper.h>
#include <WHartGateway/CmdProcessorUniversal.h>
#include <WHartGateway/CmdProcessorWirelessHART.h>
#include <WHartGateway/CmdProcessorCommonPractice.h>

#include "LogicalDevice.h"
#include "LogicalDeviceTable.h"
#include "HostAppSession.h"
#include "GwRequest.h"
#include "DrmCache.h"

#include <nlib/log.h>
#include <nlib/socket/ServiceSocket.h>
#include <nlib/timer/Timer.h>
#include <WHartStack/CommInterfaceOverIP/MessageIP.h>

#include <WHartStack/CommInterfaceOverIP/SessionIP.h>
#include <WHartStack/CommInterfaceOverIP/UDPServerIP.h>
#include <Shared/app.h>

#include <loki/Function.h>

#define GW_REQUESTS_REFRESH_INTERVAL (50)


namespace hart7 {
namespace gateway {

	typedef std::map<int, CGwRequest::Ptr> CGwRspHandleMap;

/**
 * Gateway main class
 */
class Gateway : CApp
{
	LOG_DEF("hart7.gateway.Gateway");
private:
	typedef std::set<CHostAppSession::WeakPtr> CGwSessionSet;

    Gateway(GatewayConfig & config);

    static Gateway * instance;

public:
    static Gateway& CreateInstance(GatewayConfig & config);

	bool cold_start;
	void Start();
	void Stop();

private:
    /**
     * Send response for pending requests that match the given confirmed request.
     */
    void TransmitConfirm(const CGwRequest::Ptr & gwRequest);

	void TransmitConfirm(WHartHandle handle, WHartLocalStatus localStatus, DeviceIndicatedStatus status, CHartCmdWrapperList& responses);
	void TransmitIndicate(WHartHandle handle, const WHartAddress& src, WHartPriority priority, DeviceIndicatedStatus status,
	  WHartTransportType transportType, CHartCmdWrapperList& p_rRequests);

	static void GwHandlerFATAL(int p_signal);

	// host app session management
private:
    /**
     * Returns   true    - the command was processed, a response (even error response) is prepared and it will be sent
     * MUST return false only if the command should be fwd.
     */
 	bool ExecLocalCmd(CHartCmdWrapper::Ptr p_pCmd,
 	                  CHartCmdWrapper::Ptr & p_pRsp,
 	                  CHostAppSession::Ptr & p_pHostSession,
 	                  CmdProcessorUniversal & p_pCmdUniversal,
 	                  CmdProcessorWirelessHART & p_pCmdWirelessHART,
 	                  CmdProcessorCommonPractice & p_pCmdCommonPractice);

 	/**
 	 * Returns true    - the command was processed, a response (even error response) is prepared and it will be sent
     * MUST return false only if the command should be fwd.
 	 */
	bool ExecLocalCmds ( CGwRequest::Ptr & p_pReq,
						CHostAppSession::Ptr & p_pHostSession,
						CmdProcessorUniversal & p_pCmdUniversal,
						CmdProcessorWirelessHART & p_pCmdWirelessHART,
						CmdProcessorCommonPractice & p_pCmdCommonPractice);

	/**
	 * Return true if the command change the gateway device, otherwise return false.
	 */
	bool IsGwDeviceChanged(uint16_t cmdId);

	/**
	 * return   true  -- send message to field
     *          false -- response from cache or DRM response was sent, drop message
	 */
	bool DrmGetResponses(CGwRequest::Ptr & p_pQReq);

	/**
	 * Return true if all commands were found in cache.
	 */
	bool GetResponsesFromCache (CGwRequest::Ptr& p_pReq, CLogicalDevice::Ptr& p_pLogicalDevice);
	CLogicalDevice* GetDeviceToSendTo (CGwRequest::Ptr& p_pReq, CLogicalDevice::Ptr& p_pLogicalDevice);


	void HostApp_StartServer(int listenPort, int minPort, int maxPort);
	void HostApp_StopServer();
	void HostApp_NewSession(const std::string& /*host*/, const stack::transport::SessionIP::Ptr);
	void HostApp_SessionClosed(const stack::transport::SessionIP::Ptr);
	void HostApp_ReceiveMessage(const stack::transport::SessionIP::Ptr session, const stack::transport::MessageIP& packet,
		//			boost::function1<void, const MessageIP&> dllCallback);
		Loki::Function<void(const transport::MessageIP&)> dllCallback);
	void RefreshGwRequests();

	void OneSecondTasks();
	void EraseGwRequest( CGwRequests::iterator& p_itDel);
	WHartHandle transmitGwRequest(CGwRequest::Ptr & p_pQReq);

	void Notify_WriteSession (const C963_WriteSession_Req* req, const C963_WriteSession_Resp* res);
	void Notify_DeleteSession (const C964_DeleteSession_Req* req, const C964_DeleteSession_Resp* res);

	void Notify_Rejoin();

	void sendChangeNotification(NotificationMaskCodesMasks notificationType);
	void sendIndicateDirectly(const WHartAddress& p_rSrcAddr, uint8_t p_u8DeviceStatus, uint8_t p_u8DeviceStatusExt,
				const CHartCmdWrapperList& p_oResponses);

	void PrepareShutdown();

	/**
	 * return   true  -- send message to field
     *          false -- DRM response sent, drop message
	 */
	bool CacheBasedDRMCheck(CGwRequest::Ptr p_pQReq);

	void logInternalStatus();
	void logInternalRequestsStatistics();
	void logDevicesStatistics();
	void dumpServicesToStream(const CLogicalDevice::Ptr& p_rLogicalDevice, std::ostream& p_oStr);


private:
	void UpdateGwNotifications(uint32_t p_nCmdId);

public :
	GatewayConfig& 			config;
	void Reset_GW();
	friend class CmdProcessorCommonPractice;
	friend class CmdProcessorUniversal;
	friend class CmdProcessorWirelessHART;

private:

	nlib::socket::ServiceSocket::Ptr service;
	nlib::timer::TimerFactory::Ptr timerFactory;
	GatewayStack stack;
	transport::ServerIP::Ptr 	m_oHostAppServer; //listen for access points sessions
	CHostAppSessions		m_oHostAppSessions;
	CGwRequests			m_oRequests;
	CLogicalDeviceTable 		m_oDevices;
	CGwRspHandleMap			m_oRspHandles;
	CmdProcessorUniversal		m_oCmdsProcessUniversal;
	CmdProcessorWirelessHART	m_oCmdsProcessWirelessHART;
	CmdProcessorCommonPractice	m_oCmdsProcessCommonPractice;

	CDrmCache					m_oDrmCache;

	int m_nDevicesRefreshCountDown;

	nlib::timer::DeadlineTimer::Ptr timer;

	nlib::timer::DeadlineTimer::Ptr burstNotifyTimer;
	nlib::timer::DeadlineTimer::Ptr eventNotifyTimer;
	nlib::timer::DeadlineTimer::Ptr deviceStatusNotifyTimer;
	nlib::timer::DeadlineTimer::Ptr deviceConfigurationNotifyTimer;
	nlib::timer::DeadlineTimer::Ptr m_oTopologyNotifyTimer;
	nlib::timer::DeadlineTimer::Ptr m_oScheduleNotifyTimer;

	nlib::timer::DeadlineTimer::Ptr m_oOneSecondTimer;

	nlib::timer::DeadlineTimer::Ptr m_oLogInternalStatusTimer;

public:
	GatewayStack&  GetStack() {return stack;}

};

} // namespace gateway
} // namespace hart7


#endif /* WHART_GATEWAY_H_ */
