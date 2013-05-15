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

#ifndef GATEWAYTRACKINGMNG_H_
#define GATEWAYTRACKINGMNG_H_


#include <WHartStack/CommInterfaceOverIP/UDPClientIP.h>
#include <WHartStack/WHartStack.h>
#include <WHartStack/WHartTypes.h>

#include <WHartHost/applicationcmds/AbstractAppCommand.h>
#include <WHartHost/applicationcmds/AppTopologyNotification.h>
#include <WHartHost/applicationcmds/AppBurstNotification.h>

#include <map>

#include <nlib/datetime.h>

#include <loki/Function.h>



namespace hart7 {
namespace gateway {

class GatewayIO;

class GatewayTrackingMng
{

public:
	Loki::Function<boost::uint16_t(stack::transport::MessageIP&)>  m_sendMessage;
	Loki::Function<void(const stack::transport::MessageIP&, const GatewayIO::AppData&)> m_messageReceived;

/* 
 * Observer Pattern
 */
public:
	Loki::Function<void(const GatewayIO::AppData&)> m_timedOutReceived;
	Loki::Function<void(const GatewayIO::AppData&)> m_gwDisconnReceived;


//notifications
public:
	void RegisterForNoBurstRspNotif(const hostapp::AppNoBurstRspNotificationPtr noBurstRspNotifPtr);
	void UnRegisterForNoBurstRspNotif();
	void RegisterTopologyMsgFromDev(const hostapp::AppTopologyNotificationPtr topoNotificationPtr);
	void UnRegisterTopologyMsgFromDev();
	void RegisterBurstMsgFromDev(const hostapp::AppBurstNotificationPtr burstNotificationPtr, const WHartUniqueID &whAddr);
	void UnRegisterBurstMsgFromDev(const WHartUniqueID &whAddr);
	void UnRegisterAllBurstMsg();
	void RegisterReportsMsgFromDev(const hostapp::AppReportsNotificationPtr	reportsNotificationPtr);
	void UnRegisterReportsMsgFromDev();
	void RegisterDevConfigMsgFromDev(const hostapp::AppDevConfigNotificationPtr devConfigNotificationPtr);
	void UnRegisterDevConfigMsgFromDev();

//send msg
public:
	void SendMessage(const GatewayIO::AppData &appData, nlib::TimeSpan timeout, stack::transport::MessageIP& msg);

//receive msg
public:
	void MessageReceived(const stack::transport::MessageIP& msg);

//
public:
	bool IsWHCmdSent(unsigned short whCmdID, const WHartUniqueID &addr);

//gw disconnected
public:
	void GWDisconnReceived();

//timed-out
public:
	void CheckTimedOutMsgs();

//
private:
	struct MessageTracking
	{
		GatewayIO::AppData	appData;
		nlib::DateTime		timeoutDate;
		
		MessageTracking(const GatewayIO::AppData &appData_, const nlib::TimeSpan& timeout_) :
			appData(appData_)
		{
			timeoutDate = nlib::CurrentUniversalTime()+ timeout_;
		}
	};
	typedef std::map<boost::int32_t, MessageTracking> TrackingsMapT;
	TrackingsMapT m_pendingTrackings;


//notifications
private:
	hostapp::AppNoBurstRspNotificationPtr	m_noBurstRspNotifPtr;
	hostapp::AbstractAppCommandPtr	m_topoNotificationPtr;
	hostapp::AbstractAppCommandPtr	m_reportsNotificationPtr;
	hostapp::AbstractAppCommandPtr	m_devConfigNotificationPtr;

	struct BurstNotifyTracking
	{
		WHartUniqueID		whAddr;
		
		BurstNotifyTracking(const WHartUniqueID& whAddr_):whAddr(whAddr_)
		{
		}
		
		bool operator < (const BurstNotifyTracking &obj)const
		{
			return  stack::operator < (whAddr, obj.whAddr);
		}
	};
	typedef std::map<BurstNotifyTracking, hostapp::AppBurstNotificationPtr> BurstNotifysMapT;
	BurstNotifysMapT m_burstNotifyTrackings;
};

}
}


#endif
