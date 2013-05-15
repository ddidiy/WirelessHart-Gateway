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


#include <WHartGateway/Service.h>
#include <Shared/Common.h>


namespace hart7 {
namespace gateway {


CService::CService(uint8_t p_u8ServiceID, WHartShortAddress u16Nick)
{
	m_nServiceStatus = ServiceStatus_NotSet;
	m_nTimeLastUsed = GetClockTicks() - sysconf( _SC_CLK_TCK );

	m_bLastPenalty = false;
	m_nUsedBy = 0;
	m_nMsgSent = 0;
	m_stServiceEntry.m_unNicknameOfPeer = u16Nick;
	m_stServiceEntry.m_tPeriod.u32 =  16 * 1000 * 32 ; //  16s
	m_stServiceEntry.m_ucServiceId = p_u8ServiceID;
	m_stServiceEntry.m_ucServiceRequestFlags = (ServiceRequestFlagsMasks)(ServiceRequestFlagsMask_Source | ServiceRequestFlagsMask_Sink | ServiceRequestFlagsMask_Intermittent);
	m_nPeriod = WhTimeToTicks(m_stServiceEntry.m_tPeriod.u32);

	LOG_DEBUG("CService::CService - ID= " << (int)p_u8ServiceID << ", period=" << m_nPeriod);
}

CService::~CService()
{
}

bool CService::IsSendAllowed()
{
	if (m_nServiceStatus != ServiceStatus_Working)
	{
        LOG_DEBUG_APP("CService::IsSendAllowed=0 : m_nServiceStatus=" << m_nServiceStatus);
		return false;
	}

	clock_t nCrtTime = GetClockTicks();

	int nRealDelta = nCrtTime - m_nTimeLastUsed;

	if (nRealDelta < (int)m_nPeriod)
	{
        // LOG_DEBUG_APP("CService::IsSendAllowed=0 : ID=" << (int)m_stServiceEntry.m_ucServiceId << ", nRealDelta < m_nPeriod, nRealDelta=" << nRealDelta << ", period=" << m_nPeriod << ", nCrtTime=" << nCrtTime << ", TimeLastUsed=" << m_nTimeLastUsed);
		return false;
	}

	int nPenaltyDelta = m_nPeriod;

	if (m_nMsgSent > 0) // waiting for a confirm
	{
        // LOG_DEBUG_APP("CService::IsSendAllowed : ID=" << (int)m_stServiceEntry.m_ucServiceId << ", m_nMsgSent > 0, nCrtTime=" << nCrtTime << ", MsgSent=" << m_nMsgSent);
		nPenaltyDelta += (180 * sysconf( _SC_CLK_TCK )) ; // sysconf( _SC_CLK_TCK ) = clock ticks per second
	}

	bool bRet = false;
	if (nRealDelta >= nPenaltyDelta)
	{
		m_bLastPenalty = false;
		m_nTimeLastUsed = GetClockTicks();

        LOG_DEBUG_APP("CService::IsSendAllowed : ID=" << (int)m_stServiceEntry.m_ucServiceId << ", nRealDelta >= nPenaltyDelta - nRealDelta="
                      << nRealDelta << ", PenaltyDelta=" << nPenaltyDelta << ", TimeLastUsed=" << m_nTimeLastUsed << ", nCrtTime=" << nCrtTime);

		bRet = true;
	}

	if (!m_bLastPenalty)
	{
		LOG_DEBUG_APP("CService::IsSendAllowed: unbalanced req/conf=" << m_nMsgSent << " PenaltyDelta=" << nPenaltyDelta * 1000 / sysconf( _SC_CLK_TCK ) );
		m_bLastPenalty = true;
	}

//	{
//        LOG_DEBUG_APP("CService::IsSendAllowed : ID=" << (int)m_stServiceEntry.m_ucServiceId << ", ret=" << bRet << ", nRealDelta=" << nRealDelta << ", PenaltyDelta=" << nPenaltyDelta
//                      << ", period=" << m_nPeriod << ", LastUsed=" << m_nTimeLastUsed << ", CrtTime=" << nCrtTime << ", MsgSent=" << m_nMsgSent);
//	}

	return bRet;
}

void CService::NotifyRequestNew()
{
	m_nUsedBy++;
	m_nTimeLastUsed = GetClockTicks();
}

void CService::NotifyRequestCompleted()
{
	if (m_nUsedBy)
	{
		m_nUsedBy--;
	}
}

CHartCmdWrapper::Ptr CService::CheckNeedReqService()
{
	if ( m_nServiceStatus >= ServiceStatus_PendingResponse )
	{
		LOG_DEBUG_APP("CService::CheckNeedReqService: response pending -> just wait");
		return CHartCmdWrapper::Ptr();
	}

	if (m_stServiceEntry.m_unNicknameOfPeer == NetworkManager_Nickname() )
	{
		LOG_DEBUG_APP("CService::CheckNeedReqService: nick NM -> no need for service");
		return CHartCmdWrapper::Ptr();
	}
	if (m_stServiceEntry.m_unNicknameOfPeer == Gateway_Nickname() )
	{
		LOG_DEBUG_APP("CService::CheckNeedReqService: nick GW -> no need for service");
		return CHartCmdWrapper::Ptr();
	}

	CHartCmdWrapper::Ptr pCmd(new CHartCmdWrapper);
	pCmd->LoadParsed ( CMDID_C799_RequestService, sizeof(C799_RequestService_Req), &m_stServiceEntry) ;

	LOG_DEBUG_APP("CService::CheckNeedReqService: prepare service request for nick=" << std::hex << m_stServiceEntry.m_unNicknameOfPeer
			<< " serviceId=" << (int)m_stServiceEntry.m_ucServiceId
			<< " domain=" << (int)m_stServiceEntry.m_ucServiceApplicationDomain);
	return pCmd;
}

std::ostream & operator<<(std::ostream & stream, const CService & service)
{
    stream << "Status=" << service.m_nServiceStatus;
    stream << ", ServiceEntry={ ID=" << (int)service.m_stServiceEntry.m_ucServiceId
        << ", Nick=" << service.m_stServiceEntry.m_unNicknameOfPeer
        << ", Period=" << service.m_stServiceEntry.m_tPeriod.u32
        << ", RequestFlags=" << service.m_stServiceEntry.m_ucServiceRequestFlags
        << ", ApplicationDomain=" << service.m_stServiceEntry.m_ucServiceApplicationDomain
        << " }";
    stream << ", LastUsed=" << service.m_nTimeLastUsed;
    stream << ", Period=" << service.m_nPeriod;
    stream << ", RouteId=" << (int)service.m_nRouteId;
    stream << ", UsedBy=" << service.m_nUsedBy;
    stream << ", MsgSent=" << service.m_nMsgSent;
    stream << ", LastPenalty=" << service.m_bLastPenalty;

    return stream;
}


class PredicateGetFirstServiceForDomain
{
public:
	PredicateGetFirstServiceForDomain (ServiceApplicationDomain p_nDomain)
	{
		m_nDomain = p_nDomain;
	}

	bool operator()(const std::pair <uint8_t, CService::Ptr> & p_pServicePair)
	{
		// for now only one service per domain with fix band
		return true;
		//return p_pServicePair.second->m_stServiceEntry.m_ucServiceApplicationDomain == m_nDomain;
	}

private:
	ServiceApplicationDomain m_nDomain;
};

CService::Ptr CServiceDeviceTable::GetService(int p_nCmdId)
{
	ServiceApplicationDomain nDomain = (p_nCmdId == CMDID_C112_TransferService) ?
								ServiceApplicationDomain_BlockTransfer : ServiceApplicationDomain_Maintenance;

	CServiceDeviceMap::iterator it = std::find_if(m_oServicesMap.begin(), m_oServicesMap.end(), PredicateGetFirstServiceForDomain(nDomain) );

	if (it != m_oServicesMap.end())
	{
		return it->second;
	}

	int nFirstEmptyServiceId = CService::s_nServiceIdMin - 1;
	for (it = m_oServicesMap.begin(); it != m_oServicesMap.end() ;++it)
	{
		CService::Ptr & pCrtService = it->second;

		if ( (int)pCrtService->m_stServiceEntry.m_ucServiceId > nFirstEmptyServiceId + 1)
		{
			break;
		}
		nFirstEmptyServiceId = pCrtService->m_stServiceEntry.m_ucServiceId ;
	}

	++nFirstEmptyServiceId;

	if (nFirstEmptyServiceId > CService::s_nServiceIdMax)
	{
		LOG_ERROR_APP("CServiceDeviceTable::GetService: service table full for nick=" << m_u16PeerNick);
		return CService::Ptr ();
	}

	uint8_t u8ServiceID = (uint8_t) nFirstEmptyServiceId;

	CService::Ptr pService(new CService(u8ServiceID, m_u16PeerNick));

	pService->m_stServiceEntry.m_ucServiceApplicationDomain = nDomain;


	if (m_u16PeerNick == NetworkManager_Nickname() || m_u16PeerNick == Gateway_Nickname())
	{
		pService->m_stServiceEntry.m_tPeriod.u32 = 1; //every 1/32 from ms
		pService->m_nServiceStatus = CService::ServiceStatus_Working;
	}
	else
	{
		pService->m_stServiceEntry.m_tPeriod.u32 = 16 * 1000 * 32; // aprox 16s
		pService->m_nServiceStatus = CService::ServiceStatus_NotSet;
	}
	m_oServicesMap.insert(std::make_pair(u8ServiceID,pService));


	return pService;
}

CService::Ptr CServiceDeviceTable::GetServiceIfExist(int p_nCmdId)
{
	ServiceApplicationDomain nDomain = (p_nCmdId == CMDID_C112_TransferService) ?
			ServiceApplicationDomain_BlockTransfer : ServiceApplicationDomain_Maintenance;

	CServiceDeviceMap::iterator it = std::find_if (m_oServicesMap.begin(), m_oServicesMap.end(), PredicateGetFirstServiceForDomain(nDomain) );

	if (it != m_oServicesMap.end())
	{
		return it->second;
	}
	return CService::Ptr();
}

void CServiceDeviceTable::ServiceReqResponse(const C799_RequestService_Req * p_pReq, const C799_RequestService_Resp * p_pResp, int p_nResponseCode)
{
	CServiceDeviceMap::iterator itService = m_oServicesMap.find(p_pReq->m_ucServiceId);

	if (itService == m_oServicesMap.end())
	{
		LOG_ERROR_APP("CServiceDeviceTable::ServiceReqResponse no serviceId=" << (int)p_pReq->m_ucServiceId << " for nick=" << std::hex << p_pReq->m_unNicknameOfPeer);
		return;
	}

	CService::Ptr pService = itService->second;

	if (p_nResponseCode == RCS_E33_DelayedResponseInitiated || p_nResponseCode == RCS_E34_DelayedResponseRunning)
	{
		pService->m_nServiceStatus = CService::ServiceStatus_PendingResponse;
		return;
	}

	if (!p_pResp || IsResponseCodeError(p_nResponseCode))
	{
		LOG_ERROR_APP("CServiceDeviceTable::ServiceReqResponse: rsp_code="<< p_nResponseCode << " for nick=" << std::hex << p_pReq->m_unNicknameOfPeer << " serviceId=" << p_pReq->m_ucServiceId << " -> delete");

		m_oServicesMap.erase(itService);
		return;
	}

	LOG_INFO_APP("ServiceReqResponse: nick=" << std::hex << p_pResp->m_unNicknameOfPeer << " serviceId=" << std::dec << (int)p_pReq->m_ucServiceId
		<< " period=" << std::dec << (int)p_pResp->m_tPeriod.u32 << "|" << std::hex << (int)(p_pResp->m_tPeriod.u32 >> 5) << "ms"
		<< " domain=" <<  std::dec << (int)p_pResp->m_ucServiceApplicationDomain << " flags=" << std::hex << (int)p_pResp->m_ucServiceRequestFlags);


	pService->m_nServiceStatus = CService::ServiceStatus_Working;


	if ( pService->m_stServiceEntry.m_tPeriod.u32 > p_pResp->m_tPeriod.u32)
	{
		LOG_INFO_APP("Capacity denied " );
		/// TODO: set status to "Capacity denied" status and "More status avail"

	}
	pService->m_stServiceEntry.m_tPeriod.u32 = p_pResp->m_tPeriod.u32;

	pService->m_nPeriod = WhTimeToTicks(pService->m_stServiceEntry.m_tPeriod.u32);
	if (pService->m_nPeriod <= 0)
	{
		pService->m_nPeriod = (1 + sysconf( _SC_CLK_TCK )/10);
	}

	//just until service management 1tick - NM/ 1s others
	//pService->m_nPeriod = (p_pResp->m_unNicknameOfPeer == NetworkManager_Nickname()) ? (1 + sysconf( _SC_CLK_TCK )/10) : 8 * sysconf( _SC_CLK_TCK );
	LOG_DEBUG_APP("CServiceDeviceTable::ServiceWrite: ID=" << pService->m_stServiceEntry.m_ucServiceId << ", period=" << pService->m_nPeriod * 1000 / sysconf( _SC_CLK_TCK ) << "ms");

	pService->m_nRouteId = p_pResp->m_ucRouteId;
}

uint8_t CServiceDeviceTable::ServiceWrite(const C973_WriteService_Req * p_pReq, const C973_WriteService_Resp * p_pResp, int p_nResponseCode)
{
	CServiceDeviceMap::iterator itService = m_oServicesMap.find(p_pReq->m_ucServiceID);

	LOG_INFO_APP("ServiceWrite: nick=" << std::hex << p_pReq->m_unPeerNickname << " serviceId=" << std::dec << (int)p_pReq->m_ucServiceID
					<< " new=" << std::dec << (int)(itService == m_oServicesMap.end()) << " period=" << std::dec
					<< (int)p_pReq->m_tPeriod.u32 << "|" <<  std::dec << (int)(p_pReq->m_tPeriod.u32 >> 5) << "ms"
					<< " domain=" <<  std::dec << (int)p_pReq->m_eApplicationDomain << " flags=" << std::hex << (int)p_pReq->m_ucRequestFlags);

	CService::Ptr pService;

	if (itService != m_oServicesMap.end())
	{
		pService = itService->second;

		if (pService->m_nServiceStatus == CService::ServiceStatus_PendingResponse || pService->m_nServiceStatus == CService::ServiceStatus_PendingRequest)
		{
			LOG_ERROR_APP("CServiceDeviceTable::ServiceWrite: service pending response" );
			return RCM_E67_OpenTransactionPending;
		}
		if (pService->m_stServiceEntry.m_ucServiceId < CService::s_nServiceIdMinNM && pService->m_stServiceEntry.m_tPeriod.u32 > p_pReq->m_tPeriod.u32)
		{
			LOG_INFO_APP("Capacity denied " );
			/// TODO: set status to "Capacity denied" status and "More status avail"
		}
	}
	else
	{
		pService.reset(new CService(p_pReq->m_ucServiceID, m_u16PeerNick));

		m_oServicesMap.insert(std::make_pair(p_pReq->m_ucServiceID, pService));
		//LOG_ERROR_APP("CServiceDeviceTable::ServiceWrite no serviceId="<<(int)p_pResp->m_ucServiceId << " for nick="<<p_pResp->m_unNicknameOfPeer);
	}

	pService->m_nServiceStatus = CService::ServiceStatus_Working;
	pService->m_nTimeLastUsed = GetClockTicks() - sysconf( _SC_CLK_TCK );
	pService->m_nUsedBy = 0;

	pService->m_stServiceEntry.m_tPeriod						= p_pReq->m_tPeriod;
	pService->m_stServiceEntry.m_ucServiceApplicationDomain		= (ServiceApplicationDomain)p_pReq->m_eApplicationDomain;
	pService->m_stServiceEntry.m_ucServiceId					= p_pReq->m_ucServiceID;
	pService->m_stServiceEntry.m_unNicknameOfPeer				= p_pReq->m_unPeerNickname;
	pService->m_stServiceEntry.m_ucServiceRequestFlags			= (ServiceRequestFlagsMasks)p_pReq->m_ucRequestFlags ;

	pService->m_nPeriod = WhTimeToTicks(pService->m_stServiceEntry.m_tPeriod.u32);
	if (pService->m_nPeriod <= 0)
	{
		pService->m_nPeriod = (1 + sysconf( _SC_CLK_TCK )/10);
	}
	//just until service management 10ticks - NM/ 8s others
	//pService->m_nPeriod = (p_pReq->m_unPeerNickname == NetworkManager_Nickname()) ? (1 + sysconf( _SC_CLK_TCK )/10) : 8 * sysconf( _SC_CLK_TCK );

	LOG_DEBUG_APP("CServiceDeviceTable::ServiceWrite: ID=" << (int)p_pReq->m_ucServiceID << ", period=" << pService->m_nPeriod * 1000 / sysconf( _SC_CLK_TCK ) << "ms");

	return RCS_N00_Success;
}

uint8_t CServiceDeviceTable::ServiceDelete(const C801_DeleteService_Req * p_pReq, const C801_DeleteService_Resp* p_pResp, int p_nResponseCode)
{
	CServiceDeviceMap::iterator itService = m_oServicesMap.find(p_pReq->m_ucServiceId);

	LOG_INFO_APP("CServiceDeviceTable::ServiceDelete: nick=" << std::hex << p_pReq->m_peerNickname << " serviceId=" << std::dec <<(int)p_pReq->m_ucServiceId
		<< " exists=" << (int)(itService != m_oServicesMap.end()) << " reason=" << std::dec << (int)p_pReq->m_ucReason);


	if (itService != m_oServicesMap.end())
	{
		m_oServicesMap.erase(itService);
	}

	return RCS_N00_Success;
}

const CServiceDeviceMap& CServiceDeviceTable::GetServicesMap()
{
	return m_oServicesMap;
}

} // namespace gateway
} // namespace hart7
