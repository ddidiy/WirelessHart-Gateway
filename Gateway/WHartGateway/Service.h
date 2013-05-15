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

#ifndef service_h__
#define service_h__

#include <boost/shared_ptr.hpp>

#include <WHartStack/WHartTypes.h>
#include <WHartStack/WHartStack.h>
#include <WHartStack/util/WHartCmdWrapper.h>

#include <ApplicationLayer/Model/CommonTables.h>
#include <WHartGateway/AllGatewayCommands.h>

#include "GatewayTypes.h"
#include <Shared/Common.h>



#include <map>

namespace hart7 {
namespace gateway {

using namespace stack;

/**
 * Network Management Specification - spec085r1.1-depr.pdf - pg 76/100 - Table 16. Service Table Entries
 * Represents a service of the Gateway.
 */
class CService : boost::noncopyable
{
public:
	typedef boost::shared_ptr<CService> Ptr;

	enum ServiceStatus { ServiceStatus_NotSet = 0, ServiceStatus_PendingRequest, ServiceStatus_PendingResponse, ServiceStatus_Delayed, ServiceStatus_Working, ServiceStatus_Deleted};
public:
	static const int s_nServiceIdMin = 1;
	static const int s_nServiceIdMax = 0x7f;

	static const int s_nServiceIdMinNM = 0x80;
	static const int s_nServiceIdMaxNM = 0xff;

public:
	CService(uint8_t p_u8ServiceID, WHartShortAddress u16Nick);
	~CService();

	/**
	 * True if can send on this service. False if the service period is larger than the time elapsed since the last send.
	 */
	bool IsSendAllowed();

	/**
	 * Add usage reference count.
	 */
	void NotifyRequestNew();
	/**
	 * Remove usage reference count.
	 */
	void NotifyRequestCompleted();

	/**
	 * Adds a pending request to the service.
	 */
	void NotifyTransmitRequest()	{	m_nMsgSent++; m_nTimeLastUsed = GetClockTicks(); }
	/**
	 * Clears a pending request of the service.
	 */
	void NotifyTransmitConfirm()	{	m_nMsgSent--; }

	CHartCmdWrapper::Ptr CheckNeedReqService();

public:
	ServiceStatus				m_nServiceStatus;

	C799_RequestService_Req		m_stServiceEntry;

	clock_t						m_nTimeLastUsed;
	clock_t						m_nPeriod;//ticks

	uint8_t						m_nRouteId;

	int							m_nUsedBy;
	int							m_nMsgSent;

	bool						m_bLastPenalty;
};

std::ostream & operator<<(std::ostream & stream, const CService & service);


typedef std::map<uint8_t, CService::Ptr> CServiceDeviceMap;

/**
 * Table of services with a devices.
 */
class CServiceDeviceTable : boost::noncopyable
{
public:
	typedef boost::shared_ptr<CServiceDeviceTable> Ptr;

public:
	CServiceDeviceTable(WHartShortAddress p_u16PeerNick) { m_u16PeerNick = p_u16PeerNick; }
	~CServiceDeviceTable() {}

	/**
	 * Gets the appropriate service to send the command with the requseted ID.
	 * Creates a new service if it does not exist.
	 */
	CService::Ptr GetService(int p_nCmdId);
	/**
	 * Gets the appropriate service to send the command with the requseted ID, if it exists.
	 * Does not create a new service if it does not exist.
	 */
	CService::Ptr GetServiceIfExist(int p_nCmdId);

	void ServiceReqResponse(const C799_RequestService_Req * p_pReq, const C799_RequestService_Resp * p_pResp, int p_nResponseCode);
	uint8_t ServiceWrite(const C973_WriteService_Req * p_pReq, const C973_WriteService_Resp * p_pResp, int p_nResponseCode);
	uint8_t	ServiceDelete(const C801_DeleteService_Req * p_pReq, const C801_DeleteService_Resp * p_pResp, int p_nResponseCode);

	/**
	 * Gets all services.
	 */
	const CServiceDeviceMap & GetServicesMap();

	/**
	 * Clears services.
	 */
	void Clear() { m_oServicesMap.clear(); }
private:
	WHartShortAddress m_u16PeerNick;
	CServiceDeviceMap m_oServicesMap;
};


} // namespace gateway
} // namespace hart7

#endif // service_h__

