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

#include <WHartGateway/DrmCache.h>
#include <WHartGateway/GatewayTypes.h>
#include <WHartGateway/GwRequest.h>
#include <time.h>

namespace hart7 {
namespace gateway {

using namespace stack;

void CDrmCache::ResponseAdd(CDrmCacheCmdMap & p_pCmdMap, CHartCmdWrapper::Ptr p_pCmd, CHartCmdWrapper::Ptr p_pRsp)
{
	if ( p_pRsp->GetResponseCode() == RCS_E32_Busy
	            || p_pRsp->GetResponseCode() == RCS_E33_DelayedResponseInitiated
	            || p_pRsp->GetResponseCode() == RCS_E34_DelayedResponseRunning
	            || p_pRsp->GetResponseCode() == RCS_E36_DelayedResponseConflict )
	{
		return;
	}

	CDrmCacheEntryList & oEntryList = p_pCmdMap[p_pRsp->GetCmdId()];

	CDrmCacheEntryList::iterator itEntry = oEntryList.begin();

	for (;itEntry != oEntryList.end();++itEntry)
	{
		CDrmCacheEntry & oEntry = *itEntry;

		if (oEntry.m_pCmd->GetRawDataLen() != p_pCmd->GetRawDataLen())
		{
			continue;
		}

		if (memcmp(oEntry.m_pCmd->GetRawData(), p_pCmd->GetRawData(), p_pCmd->GetRawDataLen()) != 0)
		{
			continue;
		}

		oEntry.m_pCmd = p_pRsp;
		return;
	}

	CDrmCacheEntry oEntry;
	oEntry.m_nRevTime = ::time(NULL);
	oEntry.m_pCmd = p_pCmd;
	oEntry.m_pRsp = p_pRsp;
	oEntryList.push_back(oEntry);
}

void CDrmCache::ResponseAdd(const CGwRequest * p_pReq, const CHartCmdWrapperList & p_pRspList)
{
	CDrmCacheCmdMap & pCmdMap = m_oCacheMap[p_pReq->m_oDevUniqueID];

	CHartCmdWrapperList::const_iterator itRsp = p_pRspList.begin();

	LOG_INFO_APP("DrmCache add "<< p_pReq << " rsps=" << p_pRspList );

	CHartCmdWrapperList::const_iterator itCmd = p_pReq->m_oCmdList.begin();

	for (; itCmd != p_pReq->m_oCmdList.end() && itRsp != p_pRspList.end();  ++itCmd, ++itRsp)
	{
		ResponseAdd(pCmdMap, *itCmd, *itRsp);
	}
}

int CDrmCache::ResponsePop(CGwRequest * p_pReq)
{
	CDrmCacheUniqueIDMap::iterator itCmdMap = m_oCacheMap.find(p_pReq->m_oDevUniqueID);

	if (itCmdMap ==  m_oCacheMap.end())
	{
		LOG_INFO_APP("DrmCache pop "<< p_pReq << " cmds=" << CHartCmdWrapperListShortDetails(p_pReq->m_oCmdList) << " NotInDrmCache" );
		return m_sCmdNotInCache;
	}

	//On wired SHOULD come only one cmd

	int nListRet = m_sCmdNotInCache;
	for (; !p_pReq->m_oCmdList.empty() ;  )
	{
		CHartCmdWrapper::Ptr pCmd = p_pReq->m_oCmdList.front();
		CHartCmdWrapper::Ptr pRsp;

		int nRes = ResponsePop(itCmdMap->second, pCmd, pRsp);

		if (nRes == m_sCmdNotInCache || !pRsp)
		{
			LOG_INFO_APP("DrmCache pop "<< p_pReq << " cmds=" << CHartCmdWrapperListShortDetails(p_pReq->m_oCmdList) << " NotInDrmCache" );
			return m_sCmdNotInCache;
		}

		nListRet = nRes;

		p_pReq->m_oRspList.push_back(pRsp);
		p_pReq->m_oCmdExecList.push_back(pCmd);
		p_pReq->m_oCmdList.pop_front();
	}

	LOG_INFO_APP("DrmCache pop "<< p_pReq << " rsps=" << p_pReq->m_oRspList << "ret=" << nListRet );

	return nListRet;
}

int CDrmCache::ResponsePop(CDrmCacheCmdMap & p_pCmdMap, CHartCmdWrapper::Ptr & p_pCmd, CHartCmdWrapper::Ptr & p_prRsp)
{
	CDrmCacheCmdMap::iterator itCmdMap = p_pCmdMap.find(p_pCmd->GetCmdId());

	if (itCmdMap == p_pCmdMap.end())
	{
		return m_sCmdNotInCache;
	}

	CDrmCacheEntryList &oEntryList = itCmdMap->second;

	CDrmCacheEntryList::iterator itEntry = oEntryList.begin();

	int nRes = m_sCmdNotInCache;
	for (;itEntry != oEntryList.end();++itEntry)
	{
		nRes = RCS_E36_DelayedResponseConflict;
		CDrmCacheEntry& oEntry = *itEntry;

		if (oEntry.m_pCmd->GetRawDataLen() != p_pCmd->GetRawDataLen())
		{
			continue;
		}

		if (memcmp(oEntry.m_pCmd->GetRawData(), p_pCmd->GetRawData(), p_pCmd->GetRawDataLen()) != 0)
		{
			continue;
		}

		p_prRsp = oEntry.m_pRsp;

		oEntryList.erase(itEntry);

		return RCS_N00_Success;
	}

	return nRes;
}


void CDrmCache::Refresh()
{

	if (!m_oRefreshTimer.IsSignaling())
	{
		return;
	}

	m_oRefreshTimer.SetTimer(60 * 1000); //60s

	CDrmCacheUniqueIDMap::iterator itDevMap = m_oCacheMap.begin();

	int nCrtTime = ::time(NULL);
	/// TODO delete cmd maps entries with empty commands list and device map entries with empty cmd map
	for(;itDevMap != m_oCacheMap.end();++itDevMap)
	{
		CDrmCacheCmdMap& pCmdMap = itDevMap->second;

		CDrmCacheCmdMap::iterator itCmdMap = pCmdMap.begin();


		for(;itCmdMap != pCmdMap.end();++itCmdMap)
		{

			CDrmCacheEntryList::iterator itEntry = itCmdMap->second.begin();

			for (;itEntry != itCmdMap->second.end();)
			{
				CDrmCacheEntry& oEntry = *itEntry;
				CDrmCacheEntryList::iterator itDel = itEntry++;

				if ( nCrtTime - oEntry.m_nRevTime > m_nEntryLifetime )
				{
					itCmdMap->second.erase(itDel);
				}

			}
		}
	}
}

} // namespace gateway
} // namespace hart7
