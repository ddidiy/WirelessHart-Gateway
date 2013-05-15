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


#include <WHartHost/gateway/GatewayIO.h>
#include <WHartHost/gateway/GatewayExceptions.h>

#include "GatewayTrackingMng.h"
#include "GatewayProcessor.h"

#include <assert.h>

#include <nlib/log.h>
#include <boost/cstdint.hpp> //used for inttypes



namespace hart7 {
namespace gateway {


//notifications
void GatewayTrackingMng::RegisterForNoBurstRspNotif(const hostapp::AppNoBurstRspNotificationPtr noBurstRspNotifPtr)
{
	m_noBurstRspNotifPtr = noBurstRspNotifPtr;
}
void GatewayTrackingMng::UnRegisterForNoBurstRspNotif()
{
	m_noBurstRspNotifPtr.reset();
}
void GatewayTrackingMng::RegisterTopologyMsgFromDev(const hostapp::AppTopologyNotificationPtr topoNotificationPtr)
{
	m_topoNotificationPtr = topoNotificationPtr;
}
void GatewayTrackingMng::UnRegisterTopologyMsgFromDev()
{
	m_topoNotificationPtr.reset();
}
void GatewayTrackingMng::RegisterBurstMsgFromDev(const hostapp::AppBurstNotificationPtr burstNotificationPtr, const WHartUniqueID &whAddr)
{
	m_burstNotifyTrackings[BurstNotifyTracking(whAddr)] = burstNotificationPtr;
}
void GatewayTrackingMng::UnRegisterBurstMsgFromDev(const WHartUniqueID &whAddr)
{
	BurstNotifysMapT::iterator notifyTracking;
	if ((notifyTracking = m_burstNotifyTrackings.find(BurstNotifyTracking(whAddr))) != m_burstNotifyTrackings.end())
		m_burstNotifyTrackings.erase(notifyTracking);
}
void GatewayTrackingMng::UnRegisterAllBurstMsg()
{
	m_burstNotifyTrackings.clear();
}
void GatewayTrackingMng::RegisterReportsMsgFromDev(const hostapp::AppReportsNotificationPtr	reportsNotificationPtr)
{
	m_reportsNotificationPtr = reportsNotificationPtr;
}
void GatewayTrackingMng::UnRegisterReportsMsgFromDev()
{
	m_reportsNotificationPtr.reset();
}
void GatewayTrackingMng::RegisterDevConfigMsgFromDev(const hostapp::AppDevConfigNotificationPtr devConfigNotificationPtr)
{
	m_devConfigNotificationPtr = devConfigNotificationPtr;
}
void GatewayTrackingMng::UnRegisterDevConfigMsgFromDev()
{
	m_devConfigNotificationPtr.reset();
}


//send msg
void GatewayTrackingMng::SendMessage(const GatewayIO::AppData &appData, nlib::TimeSpan timeout,
				 stack::transport::MessageIP& msg)
{
	boost::uint16_t trackingID = m_sendMessage(msg);

	//check in pending trackings
	TrackingsMapT::const_iterator found = m_pendingTrackings.find(trackingID);
	if (found != m_pendingTrackings.end())
	{
		LOG_ERROR_APP("[TrackingManager]: SendMessage: failed! Already exists TrackingID=" << trackingID << " ExistingTrackedCommand="
		    << found->second.appData.appCmd << " NewMessage=" << appData.appCmd);
		THROW_EXCEPTION1(GWTrackingException, "Gateway Duplicated TrackingID!");
	}

	m_pendingTrackings.insert(TrackingsMapT::value_type(trackingID, MessageTracking(appData, timeout)));

	LOG_DEBUG_APP("[TrackingManager]: SendMessage: Timeout=" << nlib::ToString(timeout) << "TransactionID=" <<(int)(msg.transactionID) << ", Message=" << *appData.appCmd);
}

//receive msg
void GatewayTrackingMng::MessageReceived(const stack::transport::MessageIP& msg)
{
	GatewayIO::AppData appData;
	WHartUniqueID	addr;
	unsigned short  notificationMask;
	hostapp::AppNoBurstRspNotification::CMD_Type cmdType;
	bool isMetaPacket = false;


	if(MessageChecker().IsChangeNotificationMsg(msg, addr, notificationMask, cmdType, isMetaPacket))
	{
		appData.cmdType = isMetaPacket == true ? GatewayIO::AppData::CmdType_Meta_Notification :
					GatewayIO::AppData::CmdType_Normal;

		if (notificationMask & NotificationMaskCodesMask_BurstMode)
		{	BurstNotifysMapT::iterator notifyTracking;

			if ((notifyTracking = m_burstNotifyTrackings.find(BurstNotifyTracking(addr))) != m_burstNotifyTrackings.end())
			{
				appData.appCmd = notifyTracking->second;

				LOG_DEBUG_APP("[TrackingManager]: Burst NotificationReceived: Message=" << msg);
				m_messageReceived(msg, appData);
			}
			else
			{
				LOG_WARN_APP("[TrackingManager]: Burst NotificationReceived: Message=" << msg << " for unknown publisher " << addr);
			}
		}

		if (notificationMask & NotificationMaskCodesMask_NetworkTopology)
		{
			if (m_topoNotificationPtr)
			{
				appData.appCmd = m_topoNotificationPtr;
				appData.innerDataHandle = 0;

				m_messageReceived(msg, appData);
			}
			else
			{
				LOG_WARN_APP("[TrackingManager]: Topology NotificationReceived but we didn't expect it! Message=" << msg);
			}
		}

		if (notificationMask & NotificationMaskCodesMask_NetworkSchedule)
		{
			if (m_reportsNotificationPtr)
			{
				appData.appCmd = m_reportsNotificationPtr;
				appData.innerDataHandle = 0;

				m_messageReceived(msg, appData);
			}
			else
			{
				LOG_WARN_APP("[TrackingManager]: NetworkSchedule NotificationReceived but we didn't expect it! Message=" << msg);
			}
		}

		if (notificationMask & NotificationMaskCodesMask_DeviceConfiguration)
		{
			if (m_devConfigNotificationPtr)
			{
				appData.appCmd = m_devConfigNotificationPtr;
				appData.innerDataHandle = 0;

				m_messageReceived(msg, appData);
			}
			else
			{
				LOG_WARN_APP("[TrackingManager]: DeviceConfiguration NotificationReceived but we didn't expect it! Message=" << msg);
			}
		}

		if (notificationMask & NotificationMaskCodesMask_ResponseReceived)
		{
			if (m_noBurstRspNotifPtr)
			{
				m_noBurstRspNotifPtr->SetCmdType(cmdType);
				m_noBurstRspNotifPtr->SetAddr(addr);
				appData.appCmd = m_noBurstRspNotifPtr;
				appData.innerDataHandle = 0;

				m_messageReceived(msg, appData);
			}
			else
			{
				LOG_WARN_APP("[TrackingManager]:  Response Notification Received but we didn't expect it! Message=" << msg);
			}
		}

		if (notificationMask == 0)
			LOG_WARN_APP("[TrackingManager]: Unknown NotificationReceived: Message=" << msg);
		return;
	}

	TrackingsMapT::iterator foundTracking;
	if ((foundTracking = m_pendingTrackings.find(msg.transactionID)) != m_pendingTrackings.end())
	{
		appData = foundTracking->second.appData;
		m_pendingTrackings.erase(foundTracking);

		LOG_DEBUG_APP("[TrackingManager]: MessageReceived: Message=" << msg);
		m_messageReceived(msg, appData);
		return;
	}

	LOG_WARN_APP("[TrackingManager]: unknown transactionID=" << msg.transactionID << "MessageReceived: Message=" << msg);
}


//
bool GatewayTrackingMng::IsWHCmdSent(unsigned short whCmdID, const WHartUniqueID &addr)
{
	for (TrackingsMapT::iterator it = m_pendingTrackings.begin(); it != m_pendingTrackings.end(); ++it)
	{
		if (it->second.appData.whCmdID == whCmdID &&
			!memcmp(it->second.appData.uniqueId.bytes, addr.bytes, sizeof(addr.bytes)))
		{
			return true;
		}
	}

	return false;
}


//gw disconnected
void GatewayTrackingMng::GWDisconnReceived()
{
	LOG_WARN_APP("[TrackingManager]: Connection lost detected. Cancelling all pending messages...");

	std::list<GatewayIO::AppData> lost;

	for (TrackingsMapT::iterator it = m_pendingTrackings.begin(); it != m_pendingTrackings.end(); ++it)
		lost.push_back(it->second.appData);

	m_pendingTrackings.clear();

	if (lost.begin() != lost.end())
	{
		LOG_WARN_APP("[TrackingManager]: Canceling pending messages Count=" << lost.size());
		for (std::list<GatewayIO::AppData>::iterator it = lost.begin(); it != lost.end(); ++it)
			m_gwDisconnReceived(*it);
	}
	UnRegisterAllBurstMsg();
}

//timed-out
void GatewayTrackingMng::CheckTimedOutMsgs()
{

	//notify observers
	std::list<GatewayIO::AppData> timeouted;

	for (TrackingsMapT::iterator it = m_pendingTrackings.begin(); it != m_pendingTrackings.end();)
	{
		if (nlib::CurrentUniversalTime()> it->second.timeoutDate)
		{
			LOG_DEBUG_APP("[TrackingManager]: Command with TrackingID=" << it->first << " Timeouted.");
			timeouted.push_back(it->second.appData);

			TrackingsMapT::iterator oldIt = it;
			++it;
			m_pendingTrackings.erase(oldIt);
			continue;
		}
		++it;
	}

	if (timeouted.begin() != timeouted.end())
	{
		LOG_WARN_APP("[TrackingManager]: Timeout pending messages Count=" << timeouted.size());
		for (std::list<GatewayIO::AppData>::iterator it = timeouted.begin(); it != timeouted.end(); ++it)
			m_timedOutReceived(*it);
	}
}


}
}
