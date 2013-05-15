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

#ifndef	GATEWAYIO_H_
#define GATEWAYIO_H_

#include <WHartStack/WHartStack.h>
#include <WHartStack/CommInterfaceOverIP/UDPClientIP.h>
#include <WHartStack/util/WHartCmdWrapper.h>

#include <WHartHost/applicationcmds/AbstractAppCommand.h>
#include <WHartHost/applicationcmds/AppNoBurstRspNotification.h>
#include <WHartHost/applicationcmds/AppTopologyNotification.h>
#include <WHartHost/applicationcmds/AppReportsNotification.h>
#include <WHartHost/applicationcmds/AppDevConfigNotification.h>
#include <WHartHost/applicationcmds/AppBurstNotification.h>

#include <string>
#include <deque>

#include <nlib/datetime.h>

#include <loki/Function.h>

#include <boost/cstdint.hpp> //used for inttypes

#include <WHartHost/gateway/GatewayExceptions.h>

/* WH_cmd */
#include <ApplicationLayer/Model/UniversalCommands.h>
#include <ApplicationLayer/Model/CommonPracticeCommands.h>
#include <ApplicationLayer/Model/WirelessNetworkManagerCommands.h>
#include <ApplicationLayer/Model/GatewayCommands.h>
#include <ApplicationLayer/Model/WirelessApplicationCommands.h>
#include <ApplicationLayer/Model/DataLinkLayerCommands.h>
#include <ApplicationLayer/Model/PhysicalLayerCommands.h>
#include <ApplicationLayer/Model/NetworkLayerCommands.h>
/* WH_cmd */




namespace hart7 {
namespace gateway {


class GatewayChannel;
class GatewayTrackingMng;



/*
 * Facade Pattern
 */
class GatewayIO
{
public:
	GatewayIO(const nlib::TimeSpan &requestTimedOut, bool allowSameCmdOnDev=false);
	~GatewayIO();


//functionality
public:
	void Start(const std::string &hostIP, int hostPort, int localPort);
	void Run();
	void RunOnce();
	void Stop();


//for observers
public:
	void AddForGWDisconnectNotify(Loki::Function<void(void)> funct);
	void AddForGWConnectNotify(Loki::Function<void(const std::string&, int)> funct);
	nlib::timer::DeadlineTimer::Ptr GetTimer(int periodTime/*ms*/);

//register for notifications
public:
	void RegisterForNoBurstRspNotif(const hostapp::AppNoBurstRspNotificationPtr noBurstRspNotifPtr);
	void UnRegisterForNoBurstRspNotif();
	void RegisterTopologyMsgFromDev(const hostapp::AppTopologyNotificationPtr topoNotificationPtr);
	void UnRegisterTopologyMsgFromDev();
	void RegisterBurstMsgFromDev(const hostapp::AppBurstNotificationPtr burstNotificationPtr, const WHartUniqueID &whAddr);
	void UnRegisterBurstMsgFromDev(const WHartUniqueID &whAddr);
	void RegisterReportsMsgFromDev(const hostapp::AppReportsNotificationPtr	reportsNotificationPtr);
	void UnRegisterReportsMsgFromDev();
	void RegisterDevConfigMsgFromDev(const hostapp::AppDevConfigNotificationPtr devConfigNotificationPtr);
	void UnRegisterDevConfigMsgFromDev();

//send data
public:
	struct AppData
	{
		enum CmdType
		{
			CmdType_Normal = 1,
			CmdType_General,
			CmdType_Multiple,
			CmdType_Meta_Notification,
		};
		hostapp::AbstractAppCommandPtr appCmd;
		int			innerDataHandle;
		CmdType		cmdType;

		unsigned short	whCmdID;
		WHartUniqueID	uniqueId;

		AppData():cmdType(CmdType_Normal) { memset(&uniqueId, 0, sizeof(WHartUniqueID)); whCmdID = 0xFFFF;}
	};

	void SendWHRequest(AppData &appData, const WHartUniqueID &whAddr, const stack::WHartCommand &whCmd, bool bypassIOCache = false, bool forceSameCmdOnDev = false);
	void SendWHRequest(AppData &appData, const WHartUniqueID &whAddr, int cmdNo, std::string& dataBytes, bool bypassIOCache = false);
	void SendWHRequest(AppData &p_rAppData, const WHartUniqueID &p_rWhAddr, stack::CHartCmdWrapperList& p_rRequestsList, bool p_bBypassIOCache = false);


//receive data
public:
	enum HostError
	{
		HostSuccess = 0,
		HostTimedOut = -1,
		HostDisconnected = -2,
		HostUnserializationFail = -3
	};
	struct GWResponse
	{
		AppData				appData;
		stack::WHartCommand	whCmd;
		stack::CHartCmdWrapperList* pCmdWrappersList;
		HostError			hostErr;	//0 -no error
										//  -1 host timedout
										//  -2 host disconnected
										//  -3 host unserialization
		GWResponse()
		{
			appData.cmdType = (AppData::CmdType)0;
			appData.innerDataHandle=-1;
			memset(&appData.uniqueId, 0, sizeof(WHartUniqueID));
			pCmdWrappersList = NULL;
			hostErr = (HostError)-1000;
		}
	};
	void RegisterForRecvMsg(Loki::Function<void(GWResponse&)> funct);

private:
	void ReceiveMessage(const stack::transport::MessageIP& msg, const AppData &appData);
	void ReceiveTimedOut(const AppData &appData);
	void ReceiveDisconnected(const AppData &appData);


//
private:
	GatewayChannel		*m_pChannel;
	GatewayTrackingMng	*m_pTrackingMng;
	bool				m_allowSameCmdOnDev;

private:
	Loki::Function<void(GWResponse&)> m_recvMsg;

private:
	//parser
	stack::CHartCmdWrapper m_parser;

private:
	nlib::TimeSpan m_requestTimedOut;
};


}
}

#endif
