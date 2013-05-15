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
 * GatewayStack.h
 *
 *  Created on: Nov 28, 2008
 *      Author: nicu.dascalu
 */

#ifndef WHART_GATEWAYSTACK_H_
#define WHART_GATEWAYSTACK_H_

#include <WHartGateway/GatewayConfig.h>

#include <WHartStack/WHartStack.h>
#include <WHartStack/WHartSubApplicationData.h>
#include <WHartStack/WHartTransportData.h>
#include <WHartStack/WHartNetworkData.h>
#include <WHartStack/WHartDataLinkData.h>
#include <WHartStack/WHartLocalManagement.h>
#include <WHartStack/CommInterfaceOverIP/SessionIP.h>


#include <string>

#include <loki/Function.h>

#include <nlib/log.h>
#include <nlib/socket/ServiceSocket.h>
#include <nlib/timer/Timer.h>
#include <WHartStack/CommInterfaceOverIP/MessageIP.h>

#include <WHartStack/util/WHartCmdWrapper.h>

namespace hart7 {
namespace gateway {

using namespace stack;

/**
 * Application layer of the Gateway Stack.
 */
class GatewayStack: protected subapp::WHartSubApplicationData
{
	LOG_DEF("hart7.gateway.GatewayStack");

public:

	enum State	{ stIdle, stJoining, stOperational };
public:
	GatewayStack(GatewayConfig & config, nlib::socket::ServiceSocket::Ptr & p_service, nlib::timer::TimerFactory::Ptr & p_timerFactory);

	~GatewayStack();

	/**
	 * Requests a service to NM with specified device.
	 */
	stack::WHartHandle RequestService(const WHartAddress& with);


	/**
	 * Receive service response, for a previously request.
	 */
	//boost::function3<void, WHartHandle, WHartServiceID, WHartLocalStatus> ReceiveService;
	typedef Loki::Function<void(WHartHandle, WHartServiceID, WHartLocalStatus)> ReceiveServiceCallback;
	ReceiveServiceCallback ReceiveService;

	/**
	 * Transmit request.
	 * @to - destination
	 * @serviceID
	 * @req - command request
	 *
	 * @return the handled associated with request, an confirm will have same handle (for mapping response to request)
	 */
	WHartHandle TransmitRequest(const WHartAddress & to, WHartPriority priority, WHartTransportType transportType,
			WHartServiceID serviceID, const CHartCmdWrapperList& req, WHartSessionKey::SessionKeyCode sessionCode);


	/**
	* Transmit response.
	*/
	void TransmitResponse(WHartHandle handle, WHartServiceID serviceID, const CHartCmdWrapperList& res,
		WHartSessionKey::SessionKeyCode sessionCode);


	/**
	 * Call back confirm.
	 * @res is filled only for ack service
	 */
	//boost::function3<void, WHartHandle, WHartLocalStatus, const WHartCommandList&>TransmitConfirmCallBack;
	typedef Loki::Function<void( WHartHandle, WHartLocalStatus, DeviceIndicatedStatus, CHartCmdWrapperList & /*res*/)> TransmitConfirmCallBackType;
	TransmitConfirmCallBackType TransmitConfirmCallBack;

	/**
	 * Call back indicate.
	 * @res is filled only for ack service
	 */
	typedef Loki::Function<void(WHartHandle, const WHartAddress &, WHartPriority , DeviceIndicatedStatus, WHartTransportType, CHartCmdWrapperList&)>
			TransmitIndicateCallBackType;
	TransmitIndicateCallBackType TransmitIndicateCallBack;

	typedef Loki::Function<void( const C963_WriteSession_Req * , C963_WriteSession_Resp * )> NotifyWriteSessionCallbackType;
	NotifyWriteSessionCallbackType NotifyWriteSessionCallback;


	typedef Loki::Function<void( )> NotifyRejoinCallbackType;
	NotifyRejoinCallbackType NotifyRejoinCallback;

	void Start();
	void Stop();

	void PrepareShutdown();

	int GetState() { return state; }
	void NMSessionReset();
	void TryExecCmd ( const WHartAddress & src, CHartCmdWrapper * p_pCmd, CHartCmdWrapper * p_pRsp) ;

	LocalManagement* GetLocalManagement() { return &mng;}
private:

	void TransmitConfirm(WHartHandle requestHandle, const WHartLocalStatus & localStatus, DeviceIndicatedStatus status, const WHartPayload& apdu) ;

	// on destination, called by lower level
	void TransmitIndicate(WHartHandle handle, const WHartAddress& src, WHartPriority priority, DeviceIndicatedStatus status,
		WHartTransportType transportType, const WHartPayload& apdu, WHartSessionKey::SessionKeyCode sessionCode) ;

	int ExtractWHartCmds (bool p_bIsResponse, const WHartPayload& apdu, CHartCmdWrapperList& p_oConfList, uint8_t p_u8DeviceStatus) ;


private:
	void 	ResetStack();
	void	SendJoinRequest();
	bool	ProcessNMRegistrationReq(const WHartCommandList & listReq, WHartCommandList & listResp);

	void   ProcessJoinStatus();

	//nm session management
private:
	/*
	 * Register a client, and maintains UDP session with NM.
	 */
	void StartNMSession(const std::string & host, int port, int my_port);
	void NMSessionSucceeded();
	void NMSessionFailed(const std::string & reason);
	void NMSessionClosed();


	// ap session management
private:
	void StartAPsServer(int listenPort, int minPort, int maxPort);
	void NewAPSession(const std::string& /*host*/, const stack::transport::SessionIP::Ptr);
	void APSessionClosed(const stack::transport::SessionIP::Ptr);
	void GetAPSessionIdentity(const stack::transport::SessionIP::Ptr session, const stack::transport::MessageIP & packet,
			boost::function1<void, const stack::transport::MessageIP&> dllCallback);
//			Loki::Function<void(const transport::MessageIP&)> dllCallback);


private:
	GatewayConfig& config;

	nlib::socket::ServiceSocket::Ptr& service;
	nlib::timer::TimerFactory::Ptr& timerFactory;

	nlib::timer::DeadlineTimer::Ptr joinTimer;

	stack::CommonData commonData;
	stack::transport::WHartTransportData transport;
	stack::network::WHartNetworkData network;
	stack::datalink::WHartDataLinkData datalink;
	LocalManagement mng;



	transport::ServerIP::Ptr serverAPs; //listen for access points sessions

	transport::ClientIP::Ptr clientNM; // create NM session


	State state;

	WHartHandle m_nCurrentJoinReqHandle;
	WHartHandle m_nLastConfirmed;
};

} // namespace gateway
} // namespace hart7


#endif /* WHART_GATEWAYSTACK_H_ */
