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

#ifndef LogicalDevice_h__
#define LogicalDevice_h__

#include <WHartStack/WHartTypes.h>
#include <WHartStack/WHartStack.h>
#include <WHartStack/util/WHartCmdWrapper.h>
#include <WHartGateway/LogicalDeviceSubDevice.h>

#include "GatewayTypes.h"
#include "GwRequest.h"
#include "Service.h"

#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>

#include <list>
#include <set>
#include <map>

namespace hart7 {
namespace gateway {

using namespace stack;
/**
 * Cached response.
 */
class CCachedResponse
{
public:
	typedef boost::shared_ptr<CCachedResponse> Ptr;
public:
	int				m_nRecvCounter;
	time_t			m_nRecvTime;
	int				m_nTimeoutPeriod;
	uint16_t		m_usCmdId;
	uint8_t			m_u8Len;
	uint8_t			m_u8RspCode;

	bool			m_bIndicate;

	boost::shared_array<uint8_t> m_pResponse;
};

typedef std::list<CCachedResponse::Ptr> CCachedResponseList;
typedef boost::shared_ptr<CCachedResponseList> CCachedResponseListPtr;


typedef std::map<uint16_t,CCachedResponse::Ptr> CCachedResponseMap;

typedef std::map<uint16_t,CCachedResponseListPtr> CBurstResponseMap;


class CLogicalDeviceTable;

/**
 * Model of a device in the Gateway.
 */
class CLogicalDevice
{

public:
	typedef boost::shared_ptr<CLogicalDevice> Ptr;
	typedef boost::shared_ptr<C119_AcknowledgeEventNotification_Resp>	C119_AcknowledgeEventNotification_Resp_Ptr;
	typedef std::vector<C119_AcknowledgeEventNotification_Resp_Ptr>		C119_AcknowledgeEventNotification_Resp_Vector;
	typedef std::set<unsigned int> CmdsSet;

	enum {DeviceType_Unk = 0, DeviceType_WHart, DeviceType_Adapter, DeviceType_SubDevice, DeviceType_AccessPoint};


private:
	/*graphs*/
		typedef std::set<uint16_t> CNicknamesSet;
		typedef std::map<uint16_t, CNicknamesSet> CGraphNeighborsMap;
	/**/

	/*links*/
		struct LinksKey
		{	uint8_t bytes[5];
		};
		class CompareLinks {
		public:
			bool operator() (const LinksKey & p_oFirst, const LinksKey & p_oSecond) {
				return memcmp(p_oFirst.bytes, p_oSecond.bytes, sizeof(p_oSecond.bytes)) < 0;
			}
		};
		typedef std::map<LinksKey, C967_WriteLink_Resp, CompareLinks> CLinksMap;
	/**/

	/*superframes*/
		typedef uint8_t SuperframesKey;
		typedef std::map<SuperframesKey, C965_WriteSuperframe_Resp> CSuperframesMap;
	/**/

	/*routes*/
		typedef uint8_t RoutesKey;
		typedef std::map<RoutesKey, C974_WriteRoute_Resp> CRoutesMap;
	/**/

	/*source routes*/
		typedef uint8_t SourceRoutesKey;
		typedef std::map<SourceRoutesKey, C976_WriteSourceRoute_Resp> CSourceRoutesMap;
	/**/

	/*services*/
		typedef uint8_t ServicesKey;
		typedef std::map<ServicesKey, C973_WriteService_Resp> CServicesMap;
	/**/

private:

	LinksKey MakeLinksKey(uint8_t p_nSuperframeId, uint16_t p_nSlotNumber, uint16_t p_nNickname);
	CCachedResponseList::iterator findMatchingResponseInList(const CCachedResponseListPtr & p_oRespList, const CHartCmdWrapper::Ptr & p_pCmd);

	bool GetCachedBurstResponse(CHartCmdWrapper::Ptr & p_pReq, CHartCmdWrapper::Ptr & p_pRsp);
	bool GetCachedReadResponse(CHartCmdWrapper::Ptr & p_pReq, CHartCmdWrapper::Ptr & p_pRsp);

	bool GetCachedCustomResponse(CHartCmdWrapper::Ptr & p_pReq, CHartCmdWrapper::Ptr & p_pRsp);
	bool GetCachedCustomResponse_769(CHartCmdWrapper::Ptr & p_pReq, CHartCmdWrapper::Ptr & p_pRsp);
	bool GetCachedCustomResponse_785(CHartCmdWrapper::Ptr & p_pReq, CHartCmdWrapper::Ptr & p_pRsp);
	bool GetCachedCustomResponse_784(CHartCmdWrapper::Ptr & p_pReq, CHartCmdWrapper::Ptr & p_pRsp);
	bool GetCachedCustomResponse_783(CHartCmdWrapper::Ptr & p_pReq, CHartCmdWrapper::Ptr & p_pRsp);
	bool GetCachedCustomResponse_802(CHartCmdWrapper::Ptr & p_pReq, CHartCmdWrapper::Ptr & p_pRsp);
	bool GetCachedCustomResponse_803(CHartCmdWrapper::Ptr & p_pReq, CHartCmdWrapper::Ptr & p_pRsp);
	bool GetCachedCustomResponse_800(CHartCmdWrapper::Ptr & p_pReq, CHartCmdWrapper::Ptr & p_pRsp);

	void RemoveCMDID_C000_ReadUniqueIdentifierFromCache();

public:
	CLogicalDevice (CLogicalDeviceTable * p_pDevicesTable, int p_nDeviceType, const WHartUniqueID & p_oUniqueID, WHartShortAddress p_u16Nick):
	  m_pDevicesTable(p_pDevicesTable), m_nDeviceType(p_nDeviceType), m_oDeviceUniqueID(p_oUniqueID), m_u16DeviceNickName(p_u16Nick),
	  m_oC119_AcknowledgeEventNotification_Resp_Vector(8), m_u16NotificationMask(0), m_pServiceTable(new CServiceDeviceTable(p_u16Nick)),
	  m_nJoinStatus(0), m_u16StxMessagesSentTo(0), m_u16AckMessagesReceivedFrom(0), m_u16BackMessagesReceivedFrom(0)
	{
		m_nTransactionID = 1;
		m_bInitCmdsSent = false;
		m_bDeviceActive = false;
	}

	int	GetNextTransactionID()
	{
		int nTransId = m_nTransactionID++;
		if (m_nTransactionID>=0xffff)
		{
			m_nTransactionID = 1;
		}
		return nTransId;
	}

	/**
	 * Handles requests that are not reads.
	 */
	bool ResponseAddSpecial(uint8_t p_u8DeviceStatus, uint8_t p_u8DeviceStatusExt, CHartCmdWrapper::Ptr & p_pRsp, CGwRequest * p_pGwReq );

	/**
	 * Adds responses for this device for the specified requests. Used for caching.
	 */
	void ResponseAdd(uint8_t p_u8DeviceStatus, uint8_t p_u8DeviceStatusExt, const CHartCmdWrapperList & p_oResponses, CGwRequest * p_pGwReq );
	/**
	 * Gets responses from cache.
	 */
	bool ResponseGet(CHartCmdWrapper::Ptr & p_pReq, CHartCmdWrapper::Ptr & p_pRsp, uint8_t & u8RspCode);

	/**
	 * Request a CMD 48 if cached response not available.
	 */
	void LocalGwReq_Add_C048();
	/**
	 * Create a request for the command. Check for available service.
	 */
	CGwRequest::Ptr LocalGwReq_Prepare(CHartCmdWrapper::Ptr & p_pCmd);
	/**
	 * Create a request for the command id. Check for available service.
	 */
	CGwRequest::Ptr LocalGwReq_Prepare(int p_nCmdId);

	/**
	 * Check if command with requested ID is pending.
	 */
	CGwRequest::Ptr LocalGwReq_IsPending(int p_nCmdId);
	/**
	 * Clears pending commands with the specified command IDs.
	 */
	CGwRequest::Ptr LocalGwReq_ClearPending(std::set<int> & p_oCmdsSet);

	/**
	 * Check if cmd 0 and cmd 20 responses are needed for this device. If so, request them.
	 */
	CGwRequest::Ptr CheckNeedForIdInfo();

	/**
	 * Check if universal commands (7, 8, 12, 13, 14, 15, 16, 48) are needed for this device. If so, request them.
	 */
	CGwRequest::Ptr CheckNeedUnivCmds();

	/**
	 * Used by CheckNeedForIdInfo to request 0 and 20.
	 */
	CGwRequest::Ptr AddIdInfoRequest(bool p_bNeed0 = true, bool p_bNeed20 = true);

	/**
	 * Check if a service is needed and not available to communicate with this device. If so, request it.
	 */
	CGwRequest::Ptr CheckNeedRequestService(CService::Ptr & p_pService) ;


	bool Refresh() ;

	void GwLocalRequestTransmitConfirm(CGwRequest::Ptr p_pReq, CHartCmdWrapperList & responses) ;
	void GwLocalRequestRemove(CGwRequest::Ptr p_pReq);

	WHartShortAddress GetNick() { return m_u16DeviceNickName; }

	WHartShortAddress GetNickToSendTo();

	CServiceDeviceTable::Ptr	ServiceGetTable() { return m_pServiceTable; }
	CService::Ptr				ServiceGet(int p_nCmdId) ;

	/**
	 * Handles service requests with the device.
	 */
	void			ServiceReqResponse(const C799_RequestService_Req * p_pReq, const C799_RequestService_Resp * p_pResp, int p_nResponseCode);
	/**
	 * Handles service writes with the device.
	 */
	uint8_t			ServiceWrite(const C973_WriteService_Req * p_pReq, const C973_WriteService_Resp * p_pResp, int p_nResponseCode);
	/**
	 * Handles delete service with the device.
	 */
	uint8_t			ServiceDelete(const C801_DeleteService_Req * p_pReq, const C801_DeleteService_Resp * p_pResp, int p_nResponseCode);
	//void			ServiceDelete(const C801_DeleteService_Req* p_pReq);


	uint8_t*		GetLongTag();

	void			SetNickname(WHartShortAddress p_u16Nick) { m_u16DeviceNickName = p_u16Nick;}

	bool			IsActive() ;
	void			SetActive(bool p_bActive);

	void 			ResetStatistics();

	uint16_t		GetStxMessagesSentToDev();
	uint16_t 		GetAckMessagesReceivedFromDev();
	uint16_t 		GetBackMessagesReceivedFromDev();

	void			RegisterStxMessagesSentToDev(uint16_t m_u16Nb = 1);
	void 			RegisterAckMessagesReceivedFromDev(uint16_t m_u16Nb = 1);
	void 			RegisterBackMessagesReceivedFromDev(uint16_t m_u16Nb = 1);

public:
	CLogicalDeviceTable *	m_pDevicesTable;

	int					m_nDeviceType;
	bool				m_bDeviceActive;

	WHartUniqueID		m_oDeviceUniqueID;
	WHartShortAddress	m_u16DeviceNickName;	//0000 for subdevices

	int					m_nTransactionID;

	uint8_t				m_u8DeviceStatus;
	uint8_t				m_u8DeviceStatusExt;

	bool				m_bInitCmdsSent;


	boost::shared_ptr<C000_ReadUniqueIdentifier_Resp>		m_pC000_ReadUniqueIdentifier_Resp;
	boost::shared_ptr<C020_ReadLongTag_Resp>			m_pC020_ReadLongTag_Resp;
	boost::shared_ptr<C048_ReadAdditionalDeviceStatus_Resp>		m_pC048_ReadAdditionalDeviceStatus_Resp;
	C119_AcknowledgeEventNotification_Resp_Vector			m_oC119_AcknowledgeEventNotification_Resp_Vector;

	CCachedResponseMap	m_oCachedResponses;
	CBurstResponseMap	m_oBurstResponses;

	CGraphNeighborsMap	m_oGraphNeighbors;
	CLinksMap			m_oLinksMap;
	CSuperframesMap		m_oSuperframesMap;
	CRoutesMap			m_oRoutesMap;
	CSourceRoutesMap	m_oSourceRoutesMap;
	CServicesMap		m_oServicesMap;

	CHostAppReqTransMap m_oLocalRequests;

	uint16_t			m_u16NotificationMask;


	//all sub devices share the adapter services table
	CServiceDeviceTable::Ptr	m_pServiceTable;

	//burst commands that have arrived since the last sent burst notification
	CmdsSet m_oNotifBurstCmds;

	//for network topology and schedule notifications and commands are distributed on target/source devices, but masks on sessions notification are only on NM
	CmdsSet m_oNotifNetworkTopology;
	CmdsSet m_oNotifNetworkSchedule;

	// only for WH Adapters
	boost::shared_ptr<CLogicalDeviceInfoAdapter>	m_pAdapterInfo;

	//only for subDevices
	boost::shared_ptr<CLogicalDeviceInfoSubDevice>	m_pSubDeviceInfo;

	int m_nJoinStatus;

private:
	//statistics
	uint16_t m_u16StxMessagesSentTo;
	uint16_t m_u16AckMessagesReceivedFrom;
	uint16_t m_u16BackMessagesReceivedFrom;
};



} // namespace gateway
} // namespace hart7

#endif // LogicalDevice_h__
