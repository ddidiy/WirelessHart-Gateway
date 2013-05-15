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

#include <stdio.h>
#include <time.h>
#include <iterator>

#include <boost/foreach.hpp>
#include <WHartGateway/GwCmdsDescriptor.h>
#include <WHartGateway/LogicalDevice.h>
#include <WHartGateway/LogicalDeviceTable.h>
#include <ApplicationLayer/Model/UniversalCommands.h>
#include <WHartGateway/GatewayConfig.h>


using namespace boost;


namespace hart7 {
namespace gateway {

CCachedResponse::Ptr CreateChachedResponse( uint16_t p_u16CmdId, uint8_t p_u8RspCode, int p_nDataLen, void * p_pData )
{
	CCachedResponse::Ptr pChachedRsp(new CCachedResponse);

	pChachedRsp->m_nRecvCounter = 1;
	pChachedRsp->m_nRecvTime = time(NULL);
	pChachedRsp->m_nTimeoutPeriod = 0; //no timeout
	pChachedRsp->m_usCmdId = p_u16CmdId;
	pChachedRsp->m_u8RspCode = p_u8RspCode;

	pChachedRsp->m_u8Len = p_nDataLen; //p_oResponses.list[i].count;
	pChachedRsp->m_pResponse.reset(new uint8_t[pChachedRsp->m_u8Len]);
	memcpy (pChachedRsp->m_pResponse.get(), p_pData, p_nDataLen );
	return pChachedRsp;
}

CCachedResponse::Ptr CreateChachedResponse(CHartCmdWrapper * p_pRsp)
{
	return CreateChachedResponse(p_pRsp->GetCmdId(), p_pRsp->GetResponseCode(), p_pRsp->GetRawDataLen(), p_pRsp->GetRawData());
}

CLogicalDevice::LinksKey CLogicalDevice::MakeLinksKey(uint8_t p_nSuperframeId, uint16_t p_nSlotNumber, uint16_t p_nNickname)
{
	LinksKey key;
	key.bytes[0] = p_nSuperframeId;
	memcpy(key.bytes + sizeof(p_nSuperframeId), &p_nSlotNumber, sizeof(p_nSlotNumber));
	memcpy(key.bytes +sizeof(p_nSuperframeId) + sizeof(p_nSlotNumber), &p_nSlotNumber, sizeof(p_nSlotNumber));
	return key;
}

CCachedResponseList::iterator CLogicalDevice::findMatchingResponseInList(const CCachedResponseListPtr & p_oRespList, const CHartCmdWrapper::Ptr & p_pCmd)
{
	uint8_t variablesSize = 0;
	uint8_t variables[8];

	uint8_t start = 0;
	uint8_t step = 0;

	if( p_oRespList->empty() )
	{
		return p_oRespList->end();
	}

	switch (p_pCmd->GetCmdId())
	{
	case CMDID_C033_ReadDeviceVariables:
		//In Cmd33 response, the device variable code is located in bytes 0, 6, 12, 18
		start = 0;
		step = 6;
		if (p_pCmd->IsResponse())
		{
			C033_ReadDeviceVariables_Resp* respBytes = static_cast<C033_ReadDeviceVariables_Resp*> (p_pCmd->GetParsedData());
			variablesSize = respBytes->variablesSize;
			for( int i = 0 ; i < variablesSize ; i++)
			{
				variables[i] = respBytes->variables[i].code;
			}
		}
		else //isRequest
		{
			C033_ReadDeviceVariables_Req* reqBytes = static_cast<C033_ReadDeviceVariables_Req*> (p_pCmd->GetParsedData());
			variablesSize = reqBytes->variablesSize;
			memcpy(variables, reqBytes->variables, variablesSize /* *sizeof(uint8_t() */ );
		}
		break;
	case CMDID_C009_ReadDeviceVariablesWithStatus:

		//In Cmd9 response, the device variable code is located in bytes 1, 9, 17, 25, 33, 41, 49, 57
		start = 1;
		step = 8;
		if (p_pCmd->IsResponse())
		{
			C009_ReadDeviceVariablesWithStatus_Resp* respBytes = static_cast<C009_ReadDeviceVariablesWithStatus_Resp*> (p_pCmd->GetParsedData());
			variablesSize =  respBytes->variablesSize;
			for( int i = 0 ; i < variablesSize ; i++)
			{
				variables[i] = respBytes->slots[i].deviceVariableCode;
			}
		}
		else //isRequest
		{
			C009_ReadDeviceVariablesWithStatus_Req* reqBytes = static_cast<C009_ReadDeviceVariablesWithStatus_Req*> (p_pCmd->GetParsedData());
			variablesSize =  reqBytes->variablesSize;
			for( int i = 0 ; i < variablesSize ; i++)
			{
				variables[i] = reqBytes->slotDVC[i];
			}
		}
		break;
	default:
		return p_oRespList->begin() ;
	}

	if (variablesSize <= 0)
	{

		return p_oRespList->end();
	}

	for (CCachedResponseList::iterator response = p_oRespList->begin() ; response !=  p_oRespList->end() ; response++ )
	{
		int responseVariablesSize = ((*response)->m_usCmdId == CMDID_C009_ReadDeviceVariablesWithStatus) ? (( (*response)->m_u8Len - 1/*device status*/ - 4/*sl.0timestamp*/ )/8/*slot size*/) :
			((*response)->m_u8Len / 6);
		if (responseVariablesSize != variablesSize)
		{
			continue;
		}
		bool responseOk = true;

		for (int i = 0 ; i < variablesSize ; i++ )
		{
			if ( variables[i] != (*response)->m_pResponse[start + i*step] )
			{
				responseOk = false;
				break;
			}
		}
		if (responseOk)
		{
			return response;
		}
	}

	return p_oRespList->end();
}

bool CLogicalDevice::GetCachedBurstResponse(CHartCmdWrapper::Ptr & p_pReq, CHartCmdWrapper::Ptr & p_pRsp)
{
	switch(p_pReq->GetCmdId())
	{
        case CMDID_C119_AcknowledgeEventNotification:
        {
            C119_AcknowledgeEventNotification_Req* pEventReq = (C119_AcknowledgeEventNotification_Req*)p_pReq->GetParsedData();
            if (!m_oC119_AcknowledgeEventNotification_Resp_Vector[pEventReq->eventNo])
            {
                return false;
            }
            p_pRsp.reset(new CHartCmdWrapper);
            p_pRsp->LoadParsed(p_pReq->GetCmdId(), sizeof(C119_AcknowledgeEventNotification_Req), // Beni - sizeof(C119_AcknowledgeEventNotification_Resp)
                        m_oC119_AcknowledgeEventNotification_Resp_Vector[pEventReq->eventNo].get(), RCS_N00_Success, m_u8DeviceStatus);
            return true;
        }
	}

	CBurstResponseMap::iterator it = m_oBurstResponses.find(p_pReq->GetCmdId());

	if (it == m_oBurstResponses.end())
	{
	    return false;
	}

	CCachedResponseListPtr pBurstList = it->second;
	if (pBurstList->empty())
	{
	    return false;
	}

	CCachedResponseList::iterator pChachedRsp;

	bool isSpecialReq = true;

	/*check for special request from hostapp (cmd 9 or 33 with all slots as FA) => respond with newest response*/
	if ( p_pReq->GetCmdId() == CMDID_C009_ReadDeviceVariablesWithStatus )
	{
		C009_ReadDeviceVariablesWithStatus_Req* reqBytes = static_cast<C009_ReadDeviceVariablesWithStatus_Req*> (p_pReq->GetParsedData());
        if (!reqBytes)
        {
            LOG_ERROR_APP("CLogicalDevice::GetCachedBurstResponse - ParsedData is not CMDID_C009_ReadDeviceVariablesWithStatus; req=" << p_pReq);
            return false;
        }
		for ( int i = 0 ; i < reqBytes->variablesSize ; i++)
		{
		    if (reqBytes->slotDVC[i] != DeviceVariableCodes_None)
			{
		        isSpecialReq = false;
				break;
			}
		}
	}
	else if (p_pReq->GetCmdId() == CMDID_C033_ReadDeviceVariables)
	{
		C033_ReadDeviceVariables_Req* reqBytes = static_cast<C033_ReadDeviceVariables_Req*> (p_pReq->GetParsedData());
		if (!reqBytes)
		{
		    LOG_ERROR_APP("CLogicalDevice::GetCachedBurstResponse - ParsedData is not C033_ReadDeviceVariables_Req; req=" << p_pReq);
		    return false;
		}
		for ( int i = 0 ; i < reqBytes->variablesSize ; i++)
		{
		    if (reqBytes->variables[i] != DeviceVariableCodes_None)
			{
		        isSpecialReq = false;
				break;
			}
		}
	}
	else
	{
	    isSpecialReq = false;
	}

	if (isSpecialReq)
	{
		//find newest burst
		for ( pChachedRsp = pBurstList->begin() ;
			  pChachedRsp != pBurstList->end() && !((*pChachedRsp)->m_bIndicate);
			  ++pChachedRsp);
	}
	else
	{
	    pChachedRsp = findMatchingResponseInList(pBurstList, p_pReq);
	}

	if( pChachedRsp == pBurstList->end() )
	{
	    return false;
	}

	LOG_DEBUG_APP("ResponseGet - Got response from cache for burst cmd " << (int)((*pChachedRsp)->m_usCmdId) << " request");

	//see if the cached response has expired
	time_t now = time(NULL);
	if ( difftime(now, ((*pChachedRsp)->m_nRecvTime)) >= ((*pChachedRsp)->m_nTimeoutPeriod) )
	{
        LOG_DEBUG("Device " << m_oDeviceUniqueID << " remove from cache, cmdId=" << std::dec << (*pChachedRsp)->m_usCmdId << ", RecvTime=" << (*pChachedRsp)->m_nRecvTime << ", now=" << now);

	    pBurstList->erase(pChachedRsp);
		return false;
	}


	p_pRsp.reset(new CHartCmdWrapper);
	p_pRsp->LoadRaw((*pChachedRsp)->m_usCmdId, (*pChachedRsp)->m_u8Len, (*pChachedRsp)->m_pResponse.get(), (*pChachedRsp)->m_u8RspCode, m_u8DeviceStatus);

	return true;
}

bool CLogicalDevice::GetCachedReadResponse(CHartCmdWrapper::Ptr & p_pReq, CHartCmdWrapper::Ptr & p_pRsp)
{
	switch(p_pReq->GetCmdId())
	{
	case CMDID_C000_ReadUniqueIdentifier:
	{
		if (!m_pC000_ReadUniqueIdentifier_Resp)
		{
			return false;
		}

		p_pRsp.reset(new CHartCmdWrapper);
		p_pRsp->LoadParsed(p_pReq->GetCmdId(), sizeof(C000_ReadUniqueIdentifier_Resp), m_pC000_ReadUniqueIdentifier_Resp.get(), RCS_N00_Success, m_u8DeviceStatus);

		return true;
	}
	case CMDID_C011_ReadUniqueIdentifierAssociatedWithTag:
	{
		if (!m_pC000_ReadUniqueIdentifier_Resp)
		{
			return false;
		}

		CCachedResponseMap::iterator itCachedRsp = m_oCachedResponses.find(CMDID_C013_ReadTagDescriptorDate);

		if (itCachedRsp == m_oCachedResponses.end())
		{
			return false;
		}

		CCachedResponse::Ptr pChachedRsp = itCachedRsp->second;
		if ( pChachedRsp->m_u8Len < p_pReq->GetRawDataLen() ||
			 memcmp(pChachedRsp->m_pResponse.get(), p_pReq->GetRawData(), p_pReq->GetRawDataLen()) != 0 )
		{
			return false;
		}

		LOG_INFO_APP("ResponseGet -- Get data from cache : " << CMDID_C011_ReadUniqueIdentifierAssociatedWithTag);
		p_pRsp.reset(new CHartCmdWrapper);
		p_pRsp->LoadParsed(p_pReq->GetCmdId(), sizeof(C000_ReadUniqueIdentifier_Resp), m_pC000_ReadUniqueIdentifier_Resp.get(), RCS_N00_Success, m_u8DeviceStatus);

		return true;
	}
	case CMDID_C020_ReadLongTag:

		if (!m_pC020_ReadLongTag_Resp)
		{
			return false;
		}

		p_pRsp.reset(new CHartCmdWrapper);
		p_pRsp->LoadParsed(p_pReq->GetCmdId(), sizeof(C020_ReadLongTag_Resp), m_pC020_ReadLongTag_Resp.get(), RCS_N00_Success, m_u8DeviceStatus);

		return true;

	case CMDID_C021_ReadUniqueIdentifierAssociatedWithLongTag:

		if (!m_pC000_ReadUniqueIdentifier_Resp)
		{
			return false;
		}

		if (!m_pC020_ReadLongTag_Resp)
		{
			return false;
		}

		if ( sizeof(m_pC020_ReadLongTag_Resp->longTag) < p_pReq->GetRawDataLen() ||
			 memcmp(m_pC020_ReadLongTag_Resp->longTag, p_pReq->GetRawData(), p_pReq->GetRawDataLen()) != 0 )
		{	return false;
		}

		p_pRsp.reset(new CHartCmdWrapper);
		p_pRsp->LoadParsed(p_pReq->GetCmdId(), sizeof(C000_ReadUniqueIdentifier_Resp), m_pC000_ReadUniqueIdentifier_Resp.get(), RCS_N00_Success, m_u8DeviceStatus);

		return true;

	case CMDID_C074_ReadIOSystemCapabilities:
		if (!m_pAdapterInfo || !m_pAdapterInfo->m_pC074_ReadIOSystemCapabilities_Resp)
		{
			return false;
		}
		p_pRsp.reset(new CHartCmdWrapper);
		p_pRsp->LoadParsed(p_pReq->GetCmdId(), sizeof(C074_ReadIOSystemCapabilities_Resp),
			m_pAdapterInfo->m_pC074_ReadIOSystemCapabilities_Resp.get(), RCS_N00_Success, m_u8DeviceStatus);
		return true;
	}

	CCachedResponseMap::iterator itCachedRsp = m_oCachedResponses.find(p_pReq->GetCmdId());

	if (itCachedRsp == m_oCachedResponses.end())
	{
		return false;
	}
	CCachedResponse::Ptr pChachedRsp = itCachedRsp->second;

	LOG_INFO_APP("ResponseGet -- Get data from cache : " << pChachedRsp->m_usCmdId);
	p_pRsp.reset(new CHartCmdWrapper);
	p_pRsp->LoadRaw(pChachedRsp->m_usCmdId, pChachedRsp->m_u8Len, pChachedRsp->m_pResponse.get(), pChachedRsp->m_u8RspCode, m_u8DeviceStatus); //status for host app wired request

	return true;
}

bool CLogicalDevice::GetCachedCustomResponse(CHartCmdWrapper::Ptr & p_pReq, CHartCmdWrapper::Ptr & p_pRsp)
{
	switch(p_pReq->GetCmdId())
	{
        case CMDID_C769_ReadJoinStatus:
        {
            return (m_pDevicesTable->GetGatewayConfig()->NmBurstsCachingEnabled()) ? GetCachedCustomResponse_769(p_pReq, p_pRsp) : false;
        }

        case CMDID_C785_ReadGraphList:
        {
            return (m_pDevicesTable->GetGatewayConfig()->NmBurstsCachingEnabled()) ? GetCachedCustomResponse_785(p_pReq, p_pRsp) : false;
        }

        case CMDID_C784_ReadLinkList:
        {
            return (m_pDevicesTable->GetGatewayConfig()->NmBurstsCachingEnabled()) ? GetCachedCustomResponse_784(p_pReq, p_pRsp) : false;
        }

        case CMDID_C783_ReadSuperframeList:
        {
            return (m_pDevicesTable->GetGatewayConfig()->NmBurstsCachingEnabled()) ? GetCachedCustomResponse_783(p_pReq, p_pRsp) : false;
        }

        case CMDID_C802_ReadRouteList:
        {
            return (m_pDevicesTable->GetGatewayConfig()->NmBurstsCachingEnabled()) ? GetCachedCustomResponse_802(p_pReq, p_pRsp) : false;
        }

        case CMDID_C803_ReadSourceRoute:
        {
            return (m_pDevicesTable->GetGatewayConfig()->NmBurstsCachingEnabled()) ? GetCachedCustomResponse_803(p_pReq, p_pRsp) : false;
        }

        case CMDID_C800_ReadServiceList:
        {
            return (m_pDevicesTable->GetGatewayConfig()->NmBurstsCachingEnabled()) ? GetCachedCustomResponse_800(p_pReq, p_pRsp) : false;
        }

        case CMDID_C084_ReadSubDeviceIdentitySummary:
        case CMDID_C074_ReadIOSystemCapabilities:
        case CMDID_C075_PollSubDevice:
        {
            return (m_pAdapterInfo == 0) ? false : m_pAdapterInfo->ResponseGet(p_pReq, p_pRsp);
        }
	}

	return false;
}

bool CLogicalDevice::GetCachedCustomResponse_769(CHartCmdWrapper::Ptr & p_pReq, CHartCmdWrapper::Ptr & p_pRsp)
{
	if (m_nJoinStatus == 0)
	{
	    return false; // no info yet
	}

	C769_ReadJoinStatus_Resp resp;
	memset(&resp, 0, sizeof(C769_ReadJoinStatus_Resp));
	p_pRsp.reset(new CHartCmdWrapper);
	p_pRsp->LoadParsed(p_pReq->GetCmdId(), sizeof(C769_ReadJoinStatus_Resp), &resp, RCS_N00_Success, m_u8DeviceStatus);

	return true;
}

bool CLogicalDevice::GetCachedCustomResponse_785(CHartCmdWrapper::Ptr & p_pReq, CHartCmdWrapper::Ptr & p_pRsp)
{
	int responseCode = RCS_N00_Success;

	if (m_oGraphNeighbors.empty())
	{
	    return false;
	}

	p_pRsp.reset(new CHartCmdWrapper);

	C785_ReadGraphList_Req* pReq = (C785_ReadGraphList_Req*)p_pReq->GetParsedData();
	if (pReq == 0)
	{
	    p_pRsp->LoadParsed(p_pReq->GetCmdId(), 0, 0, C785_E05);
		LOG_WARN_APP("ResponseGet - 785 - parse error");
		return true;
	}

	C785_ReadGraphList_Resp resp;

	memset(&resp, 0, sizeof(C785_ReadGraphList_Resp));

	resp.m_ucTotalNoOfGraphs = m_oGraphNeighbors.size();

	CGraphNeighborsMap::iterator it = m_oGraphNeighbors.begin();

	if (pReq->m_ucGraphListIndex >= resp.m_ucTotalNoOfGraphs)
	{
	    responseCode = C784_W08;
		advance(it, resp.m_ucTotalNoOfGraphs - 1);
		resp.m_ucGraphListIndex = resp.m_ucTotalNoOfGraphs - 1;
		LOG_WARN_APP("ResponseGet - 785 - no such graph index " << (int)(pReq->m_ucGraphListIndex) <<" in cahe; returning last graph in cache");
	}
	else
	{
	    advance(it, pReq->m_ucGraphListIndex);
		resp.m_ucGraphListIndex = pReq->m_ucGraphListIndex;
	}

	resp.m_unGraphId = it->first;

	for (CNicknamesSet::iterator nicknameIt = it->second.begin() ; nicknameIt != it->second.end() ; ++nicknameIt)
	{
	    resp.m_aNicknameOfNeighbor[resp.m_ucNoOfNeighbors++] = *nicknameIt;
		if (resp.m_ucNoOfNeighbors >= C785_MAX_NEIGHBORS_LIST)
		{
		    LOG_WARN_APP("ResponseGet - 785 - neighbors for graphId=" << (int)(resp.m_unGraphId)
				<< " ,graph index=" << (int)(resp.m_ucGraphListIndex)
				<< " doesn't fit in package. We have " << (int)(it->second.size()) << " and max is " <<  C785_MAX_NEIGHBORS_LIST
				<< " ; Extra neighbors are discarded");
			break;
		}
	}

	p_pRsp->LoadParsed(p_pReq->GetCmdId(), sizeof(C785_ReadGraphList_Resp), &resp, responseCode, m_u8DeviceStatus);

	return true;
}

bool CLogicalDevice::GetCachedCustomResponse_784(CHartCmdWrapper::Ptr & p_pReq, CHartCmdWrapper::Ptr & p_pRsp)
{
	int responseCode = RCS_N00_Success;

	if (m_oLinksMap.empty())
	{
	    return false;
	}

	p_pRsp.reset(new CHartCmdWrapper);

	C784_ReadLinkList_Req* pReq = (C784_ReadLinkList_Req*)p_pReq->GetParsedData();
	if (pReq == 0)
	{	p_pRsp->LoadParsed (p_pReq->GetCmdId(), 0, 0, C784_E05);
		LOG_WARN_APP("ResponseGet - 784 - parse error");
		return true;
	}

	C784_ReadLinkList_Resp resp;
	memset(&resp, 0 , sizeof(C784_ReadLinkList_Resp));
	resp.m_unNoOfActiveLinks = m_oLinksMap.size();

	CLinksMap::iterator it = m_oLinksMap.begin();
	if (pReq->m_unLinkIndex >= resp.m_unNoOfActiveLinks)
	{	responseCode = C784_W08;
		advance(it, resp.m_unNoOfActiveLinks - 1);
		resp.m_unLinkIndex = resp.m_unNoOfActiveLinks - 1;
		LOG_WARN_APP("ResponseGet - 784 - no such link index " << (int)(pReq->m_unLinkIndex) <<" in cache; corrected to last index in cache");
	}
	else
	{	advance(it, pReq->m_unLinkIndex);
		resp.m_unLinkIndex = pReq->m_unLinkIndex;
	}

	for (int i = 0 ; i < pReq->m_ucNoOfLinksToRead ; ++i)
	{
		resp.m_aLinks[resp.m_ucNoOfLinksRead].superframeId = it->second.m_ucSuperframeID;
		resp.m_aLinks[resp.m_ucNoOfLinksRead].channelOffsetForThisLink = it->second.m_ucChannelOffset;
		resp.m_aLinks[resp.m_ucNoOfLinksRead].slotNoForThisLink = it->second.m_unSlotNumber;
		resp.m_aLinks[resp.m_ucNoOfLinksRead].nicknameOfNeighborForThisLink = it->second.m_unNeighborNickname;
		resp.m_aLinks[resp.m_ucNoOfLinksRead].linkOptions = it->second.m_ucLinkOptions;
		resp.m_aLinks[resp.m_ucNoOfLinksRead].linkType = (LinkType)(it->second.m_eLinkType);

		resp.m_ucNoOfLinksRead++;
		it++;

		if ( (resp.m_unLinkIndex + resp.m_ucNoOfLinksRead >= resp.m_unNoOfActiveLinks) ||
			 (resp.m_ucNoOfLinksRead >= C784_MAX_LINKS_LIST) )
		{
			responseCode = C784_W08;
			LOG_INFO_APP("ResponseGet - 784 - requested to read too many links; changed to maximum possible number = " << (int)(resp.m_ucNoOfLinksRead));
			break;
		}
	}

	p_pRsp->LoadParsed(p_pReq->GetCmdId(), sizeof(C784_ReadLinkList_Resp), &resp, responseCode, m_u8DeviceStatus);

	return true;
}

bool CLogicalDevice::GetCachedCustomResponse_783(CHartCmdWrapper::Ptr & p_pReq, CHartCmdWrapper::Ptr & p_pRsp)
{
	int responseCode = RCS_N00_Success;

	if (m_oSuperframesMap.empty())
	{	return false;
	}

	p_pRsp.reset(new CHartCmdWrapper);

	C783_ReadSuperframeList_Req* pReq = (C783_ReadSuperframeList_Req*)p_pReq->GetParsedData();
	if (pReq == 0)
	{	p_pRsp->LoadParsed (p_pReq->GetCmdId(), 0, 0, C783_E05);
		LOG_WARN_APP("ResponseGet - 783 - parse error");
		return true;
	}

	C783_ReadSuperframeList_Resp resp;
	memset(&resp, 0 , sizeof(C783_ReadSuperframeList_Resp));
	resp.m_ucNoOfActiveSuperframes = m_oSuperframesMap.size();

	CSuperframesMap::iterator it = m_oSuperframesMap.begin();
	if (pReq->m_ucSuperframeIndex >= resp.m_ucNoOfActiveSuperframes)
	{	responseCode = C783_W08;
		advance(it, resp.m_ucNoOfActiveSuperframes - 1);
		resp.m_ucSuperframeIndex = resp.m_ucNoOfActiveSuperframes - 1;
		LOG_WARN_APP("ResponseGet - 783 - no such Superframe index " << (int)(pReq->m_ucSuperframeIndex) <<" in cache; corrected to last index in cache");
	}
	else
	{	advance(it, pReq->m_ucSuperframeIndex);
		resp.m_ucSuperframeIndex = pReq->m_ucSuperframeIndex;
	}

	for (int i = 0 ; i < pReq->m_ucNoOfEntriesToRead ; ++i)
	{
		resp.m_aSuperframes[resp.m_ucNoOfEntriesRead].superframeId = it->second.m_ucSuperframeID;
		resp.m_aSuperframes[resp.m_ucNoOfEntriesRead].superframeModeFlags = (SuperframeModeFlagsMasks)(it->second.m_ucSuperframeMode);
		resp.m_aSuperframes[resp.m_ucNoOfEntriesRead].noOfSlotsInSuperframe = it->second.m_unSuperframeSlotsNo;

		resp.m_ucNoOfEntriesRead++;
		it++;

		if ( (resp.m_ucSuperframeIndex + resp.m_ucNoOfEntriesRead >= resp.m_ucNoOfActiveSuperframes) ||
			 (resp.m_ucNoOfEntriesRead >= C783_MAX_SUPERFRAMES_LIST) )
		{
			responseCode = C783_W08;
			LOG_INFO_APP("ResponseGet - 783 - requested to read too many Superframes; changed to maximum possible number = " << (int)(resp.m_ucNoOfEntriesRead));
			break;
		}
	}

	p_pRsp->LoadParsed(p_pReq->GetCmdId(), sizeof(C783_ReadSuperframeList_Resp), &resp, responseCode, m_u8DeviceStatus);

	return true;
}

bool CLogicalDevice::GetCachedCustomResponse_802(CHartCmdWrapper::Ptr & p_pReq, CHartCmdWrapper::Ptr & p_pRsp)
{
	int responseCode = RCS_N00_Success;

	if (m_oRoutesMap.empty())
	{
	    return false;
	}

	p_pRsp.reset(new CHartCmdWrapper);

	C802_ReadRouteList_Req* pReq = (C802_ReadRouteList_Req*)p_pReq->GetParsedData();
	if (pReq == 0)
	{
	    p_pRsp->LoadParsed (p_pReq->GetCmdId(), 0, 0, C802_E05);
		LOG_WARN_APP("ResponseGet - 802 - parse error");
		return true;
	}

	C802_ReadRouteList_Resp resp;
	memset(&resp, 0 , sizeof(C802_ReadRouteList_Resp));
	resp.m_ucNoOfActiveRoutes = m_oRoutesMap.size();

	CRoutesMap::iterator it = m_oRoutesMap.begin();
	if (pReq->m_ucRouteIndex >= resp.m_ucNoOfActiveRoutes)
	{
	    responseCode = C802_W08;
		advance(it, resp.m_ucNoOfActiveRoutes - 1);
		resp.m_ucRouteIndex = resp.m_ucNoOfActiveRoutes - 1;
		LOG_WARN_APP("ResponseGet - 802 - no such Route index " << (int)(pReq->m_ucRouteIndex) <<" in cache; corrected to last index in cache");
	}
	else
	{
	    advance(it, pReq->m_ucRouteIndex);
		resp.m_ucRouteIndex = pReq->m_ucRouteIndex;
	}

	for (int i = 0 ; i < pReq->m_ucNoOfEntriesToRead ; ++i)
	{
		resp.m_aRoutes[resp.m_ucNoOfEntriesRead].routeId = it->second.m_ucRouteID;

		CSourceRoutesMap::iterator sourceRoutesIt = m_oSourceRoutesMap.find(it->second.m_ucRouteID);

		resp.m_aRoutes[resp.m_ucNoOfEntriesRead].sourceRouteAttached = (sourceRoutesIt != m_oSourceRoutesMap.end());
		resp.m_aRoutes[resp.m_ucNoOfEntriesRead].destinationNickname = it->second.m_unPeerNickname;
		resp.m_aRoutes[resp.m_ucNoOfEntriesRead].graphId = it->second.m_unGraphID;

		resp.m_ucNoOfEntriesRead++;
		it++;

		if ( (resp.m_ucRouteIndex + resp.m_ucNoOfEntriesRead >= resp.m_ucNoOfActiveRoutes) || (resp.m_ucNoOfEntriesRead >= C802_MAX_ROUTES_LIST) )
		{
			responseCode = C802_W08;
			LOG_INFO_APP("ResponseGet - 802 - requested to read too many Routes; changed to maximum possible number = " << (int)(resp.m_ucNoOfEntriesRead));
			break;
		}
	}

	resp.m_ucNoOfRoutesRemaining = resp.m_ucNoOfActiveRoutes - (resp.m_ucRouteIndex + resp.m_ucNoOfEntriesRead);

	p_pRsp->LoadParsed(p_pReq->GetCmdId(), sizeof(C802_ReadRouteList_Resp), &resp, responseCode, m_u8DeviceStatus);

	return true;
}

bool CLogicalDevice::GetCachedCustomResponse_803(CHartCmdWrapper::Ptr & p_pReq, CHartCmdWrapper::Ptr & p_pRsp)
{
	int responseCode = RCS_N00_Success;

	p_pRsp.reset(new CHartCmdWrapper);

	C803_ReadSourceRoute_Req* pReq = (C803_ReadSourceRoute_Req*)p_pReq->GetParsedData();
	if (pReq == 0)
	{
	    p_pRsp->LoadParsed (p_pReq->GetCmdId(), 0, 0, C803_E05);
		LOG_WARN_APP("ResponseGet - 803 - parse error");
		return true;
	}

	CSourceRoutesMap::iterator it = m_oSourceRoutesMap.find(pReq->m_ucRouteId);
	if (it == m_oSourceRoutesMap.end())
	{
	    return false;
	}

	C803_ReadSourceRoute_Resp resp;
	resp.m_ucRouteId = pReq->m_ucRouteId;
	resp.m_ucNoOfHops = it->second.m_ucHopsNo;
	memcpy(resp.m_aHopNicknames, it->second.m_aNicknameHopEntries, sizeof(uint16_t)*resp.m_ucNoOfHops);

	p_pRsp->LoadParsed(p_pReq->GetCmdId(), sizeof(C803_ReadSourceRoute_Resp), &resp, responseCode, m_u8DeviceStatus);

	return true;
}

bool CLogicalDevice::GetCachedCustomResponse_800(CHartCmdWrapper::Ptr & p_pReq, CHartCmdWrapper::Ptr & p_pRsp)
{
	int responseCode = RCS_N00_Success;

	if (m_oServicesMap.empty())
	{
	    return false;
	}

	p_pRsp.reset(new CHartCmdWrapper);

	C800_ReadServiceList_Req* pReq = (C800_ReadServiceList_Req*)p_pReq->GetParsedData();
	if (pReq == 0)
	{
	    p_pRsp->LoadParsed (p_pReq->GetCmdId(), 0, 0, C800_E05);
		LOG_WARN_APP("ResponseGet - 800 - parse error");
		return true;
	}

	C800_ReadServiceList_Resp resp;
	memset(&resp, 0 , sizeof(C800_ReadServiceList_Resp));
	resp.m_ucNoOfActiveServices = m_oServicesMap.size();

	CServicesMap::iterator it = m_oServicesMap.begin();
	if (pReq->m_ucServiceIndex >= resp.m_ucNoOfActiveServices)
	{
	    responseCode = C800_W08;
		advance(it, resp.m_ucNoOfActiveServices - 1);
		resp.m_ucServiceIndex = resp.m_ucNoOfActiveServices - 1;
		LOG_WARN_APP("ResponseGet - 800 - no such Service index " << (int)(pReq->m_ucServiceIndex) <<" in cache; corrected to last index in cache");
	}
	else
	{
	    advance(it, pReq->m_ucServiceIndex);
		resp.m_ucServiceIndex = pReq->m_ucServiceIndex;
	}

	for (int i = 0 ; i < pReq->m_ucNoOfEntriesToRead ; ++i)
	{
		resp.m_aServices[resp.m_ucNoOfEntriesRead].serviceRequestFlags = it->second.m_ucRequestFlags;
		resp.m_aServices[resp.m_ucNoOfEntriesRead].serviceApplicationDomain = (ServiceApplicationDomain)it->second.m_eApplicationDomain;
		resp.m_aServices[resp.m_ucNoOfEntriesRead].nicknameOfPeer = it->second.m_unPeerNickname;
		resp.m_aServices[resp.m_ucNoOfEntriesRead].period = it->second.m_tPeriod;
		resp.m_aServices[resp.m_ucNoOfEntriesRead].serviceId = it->second.m_ucServiceID;
		resp.m_aServices[resp.m_ucNoOfEntriesRead].routeId = it->second.m_ucRouteID;

		resp.m_ucNoOfEntriesRead++;
		it++;

		if ( (resp.m_ucServiceIndex + resp.m_ucNoOfEntriesRead >= resp.m_ucNoOfActiveServices) ||
			 (resp.m_ucNoOfEntriesRead >= C800_MAX_SERVICES_LIST) )
		{
			responseCode = C800_W08;
			LOG_INFO_APP("ResponseGet - 800 - requested to read too many Services; changed to maximum possible number = " << (int)(resp.m_ucNoOfEntriesRead));
			break;
		}
	}

	p_pRsp->LoadParsed(p_pReq->GetCmdId(), sizeof(C800_ReadServiceList_Resp), &resp, responseCode, m_u8DeviceStatus);

	return true;
}

bool CLogicalDevice::ResponseAddSpecial(uint8_t p_u8DeviceStatus, uint8_t p_u8DeviceStatusExt, CHartCmdWrapper::Ptr & p_pRsp, CGwRequest * p_pGwReq )
{
	switch(p_pRsp->GetCmdId())
	{
        case CMDID_C000_ReadUniqueIdentifier:
        case CMDID_C011_ReadUniqueIdentifierAssociatedWithTag:
        case CMDID_C021_ReadUniqueIdentifierAssociatedWithLongTag:
        {
            if (!p_pRsp->GetParsedData())
            {
                return true;
            }
            int nCurentConfChangedNo = -1;
            if (!m_pC000_ReadUniqueIdentifier_Resp)
            {
                LOG_DEBUG_APP("CLogicalDevice::ResponseAddSpecial - Dev: UniqueID=" << m_oDeviceUniqueID << ", Nick=" << std::hex << (int)m_u16DeviceNickName << ", Create m_pC000_ReadUniqueIdentifier_Resp");
                m_pC000_ReadUniqueIdentifier_Resp.reset(new C000_ReadUniqueIdentifier_Resp);
                nCurentConfChangedNo = -1;
            }
            else
            {
                TMetaCmdsInfo* pMetaCmdInfo = p_pRsp->MetaCmdInfoGet();
                if ( pMetaCmdInfo && pMetaCmdInfo->m_u16MetaCmdId == CMDID_C64765_NivisMetaCommand )
                {	break; //packed resp to cmd 0 from nm cannot overwrite already existing resp to 0 cmd
                }

                nCurentConfChangedNo = m_pC000_ReadUniqueIdentifier_Resp->configChangeCounter;
            }

            LOG_DEBUG_APP("CLogicalDevice::ResponseAddSpecial - Dev: UniqueID=" << m_oDeviceUniqueID << ", Nick=" << std::hex << (int)m_u16DeviceNickName << ", Fill m_pC000_ReadUniqueIdentifier_Resp");
            memcpy(m_pC000_ReadUniqueIdentifier_Resp.get(), p_pRsp->GetParsedData(), sizeof(C000_ReadUniqueIdentifier_Resp) );

            if (p_u8DeviceStatus & FieldDeviceStatusesMask_ConfigurationChanged)
            {
                LOG_INFO("Receive response for cmdID=" << (int)p_pRsp->GetCmdId() << ", deviceStatus=" << (int)p_u8DeviceStatus);

                /// TODO: add local gw req for c038
                C038_ResetConfigurationChangedFlag_Resp stResp;

                stResp.configurationChangeCounter = m_pC000_ReadUniqueIdentifier_Resp->configChangeCounter;

                CHartCmdWrapper::Ptr pCmd (new CHartCmdWrapper);

                pCmd->LoadParsed(CMDID_C038_ResetConfigurationChangedFlag, sizeof(stResp), &stResp);

                LocalGwReq_Prepare(pCmd);
            }

            if (m_pC000_ReadUniqueIdentifier_Resp->flags & FlagAssignmentsMask_ProtocolBridge)
            {
                //mark as subdevice
                //send C074 and loop C075

                m_nDeviceType = DeviceType_Adapter;
                if (!m_pAdapterInfo)
                {
                    m_nDeviceType = CLogicalDevice::DeviceType_Adapter;
                    m_pAdapterInfo.reset(new CLogicalDeviceInfoAdapter(this));

                    CHartCmdWrapper::Ptr pC074(new CHartCmdWrapper);

                    pC074->LoadParsed(CMDID_C074_ReadIOSystemCapabilities, 0, 0);

                    LocalGwReq_Prepare(pC074);
                }
            }

            if (m_pC000_ReadUniqueIdentifier_Resp->deviceProfile == DeviceProfileCodes_WIRELESSHART_GATEWAY)
            {
                m_nDeviceType = CLogicalDevice::DeviceType_AccessPoint;
            }
            if (!m_bInitCmdsSent && (m_nDeviceType != CLogicalDevice::DeviceType_AccessPoint) && (m_u16DeviceNickName != NetworkManager_Nickname()) )
            {
                CHartCmdWrapperList & rInitCmdsList = m_pDevicesTable->GetGatewayConfig()->GetDeviceInitCmds(m_oDeviceUniqueID);

                //packing idea
                CGwRequest::Ptr pCrtReq;
                int nCrtLen = 1000;
                CHartCmdWrapperList::iterator itInitCmds = rInitCmdsList.begin();
                for (;itInitCmds != rInitCmdsList.end(); itInitCmds++)
                {
                    CHartCmdWrapper::Ptr pCmd = *itInitCmds;

                    if (nCrtLen > 50)
                    {
                        pCrtReq = LocalGwReq_Prepare(pCmd->GetCmdId());
                        if (!pCrtReq)
                        {
                            break;
                        }
                        nCrtLen = 0;
                    }

                    pCrtReq->m_oCmdList.push_back(pCmd);
                    nCrtLen += pCmd->GetRawDataLen() + 1; //len + rsp code
                }

                m_bInitCmdsSent = true;
                LOG_INFO_APP("LocalGw init cmds prepare for sending");
            }
            return true;
        }

        case CMDID_C006_WritePollingAddress:
        {
            if (!p_pRsp->GetParsedData())
            {
                return true;
            }
            m_oCachedResponses[CMDID_C007_ReadLoopConfiguration]
                        = CreateChachedResponse(CMDID_C007_ReadLoopConfiguration, p_pRsp->GetResponseCode(), p_pRsp->GetRawDataLen(), p_pRsp->GetRawData());
            break;
        }

        case CMDID_C017_WriteMessage:
        {
            if (!p_pRsp->GetParsedData())
            {
                return true;
            }

            m_oCachedResponses[CMDID_C012_ReadMessage]
                               = CreateChachedResponse(CMDID_C012_ReadMessage, p_pRsp->GetResponseCode(), p_pRsp->GetRawDataLen(), p_pRsp->GetRawData());
            break;
        }

        case CMDID_C018_WriteTagDescriptorDate:
        {
            if (!p_pRsp->GetParsedData())
            {
                return true;
            }
            m_oCachedResponses[CMDID_C013_ReadTagDescriptorDate]
                                    = CreateChachedResponse(CMDID_C013_ReadTagDescriptorDate, p_pRsp->GetResponseCode(), p_pRsp->GetRawDataLen(), p_pRsp->GetRawData());
            break;
        }

        case CMDID_C019_WriteFinalAssemblyNumber:
        {
            if (!p_pRsp->GetParsedData())
            {
                return true;
            }
            m_oCachedResponses[CMDID_C016_ReadFinalAssemblyNumber]
                                = CreateChachedResponse(CMDID_C016_ReadFinalAssemblyNumber, p_pRsp->GetResponseCode(), p_pRsp->GetRawDataLen(), p_pRsp->GetRawData());
            break;
        }

        case CMDID_C020_ReadLongTag:
        {
            if (!p_pRsp->GetParsedData())
            {
                return true;
            }
            if (!m_pC020_ReadLongTag_Resp)
            {
                LOG_DEBUG_APP("CLogicalDevice::ResponseAddSpecial - Dev: UniqueID=" << m_oDeviceUniqueID << ", Nick=" << std::hex << (int)m_u16DeviceNickName << ", Create m_pC020_ReadLongTag_Resp");
                m_pC020_ReadLongTag_Resp.reset(new C020_ReadLongTag_Resp);
            }

            LOG_DEBUG_APP("CLogicalDevice::ResponseAddSpecial - Dev: UniqueID=" << m_oDeviceUniqueID << ", Nick=" << std::hex << (int)m_u16DeviceNickName << ", Fill m_pC020_ReadLongTag_Resp");
            memcpy(m_pC020_ReadLongTag_Resp.get(), p_pRsp->GetParsedData(), sizeof(C020_ReadLongTag_Resp) );

            return true;
        }

        case CMDID_C022_WriteLongTag:
        {
            if (!p_pRsp->GetParsedData())
            {
                return true;
            }
            // translate pRsp for C022 in pRsp for C020
            CHartCmdWrapper::Ptr cmd20Rsp(new CHartCmdWrapper);
            cmd20Rsp->LoadRaw(CMDID_C020_ReadLongTag, p_pRsp->GetRawDataLen(), p_pRsp->GetRawData(), p_pRsp->GetResponseCode());
            ResponseAddSpecial(p_u8DeviceStatus, p_u8DeviceStatusExt, cmd20Rsp, p_pGwReq);

            break;
        }

        case CMDID_C048_ReadAdditionalDeviceStatus:
        {
            if (!p_pRsp->GetParsedData())
            {
                return true;
            }
            m_oCachedResponses[CMDID_C048_ReadAdditionalDeviceStatus] =
                    CreateChachedResponse(CMDID_C048_ReadAdditionalDeviceStatus, p_pRsp->GetResponseCode(), p_pRsp->GetRawDataLen(), p_pRsp->GetRawData());

            if (!m_pC048_ReadAdditionalDeviceStatus_Resp)
            {
                m_pC048_ReadAdditionalDeviceStatus_Resp.reset(new C048_ReadAdditionalDeviceStatus_Resp);
            }

            memcpy(m_pC048_ReadAdditionalDeviceStatus_Resp.get(), p_pRsp->GetParsedData(), sizeof(C048_ReadAdditionalDeviceStatus_Resp) );

            if (m_pAdapterInfo != 0)
            {
                m_pAdapterInfo->ResponseAdd(p_pRsp, p_pGwReq);
            }
            return true;
        }

        case CMDID_C074_ReadIOSystemCapabilities:
        {
            if (!p_pRsp->GetParsedData())
            {
                return true;
            }
            if (!m_pAdapterInfo)
            {
                if (m_pC000_ReadUniqueIdentifier_Resp && (m_pC000_ReadUniqueIdentifier_Resp->flags & FlagAssignmentsMask_ProtocolBridge))
                {
                    m_pAdapterInfo.reset(new CLogicalDeviceInfoAdapter(this));
                }
                else
                {
                    LOG_WARN_APP("CMDID_C074_ReadIOSystemCapabilities resp for " << m_oDeviceUniqueID << " that is not WHart adapter");
                    return true;
                }
            }

            m_pAdapterInfo->ResponseAdd(p_pRsp, p_pGwReq);
            return true;
        }

        case CMDID_C075_PollSubDevice:
        {
            if (!p_pRsp->GetParsedData())
            {
                return true;
            }
            if (!m_pAdapterInfo)
            {
                LOG_WARN_APP("CMDID_C075_PollSubDevice resp for " << m_oDeviceUniqueID <<" that has no adapter info attached");
                return true;
            }

            m_pAdapterInfo->ResponseAdd(p_pRsp,p_pGwReq);
            return true;
        }
        case CMDID_C084_ReadSubDeviceIdentitySummary:
        {
            if (!p_pRsp->GetParsedData())
            {
                return true;
            }

            if (!m_pAdapterInfo)
            {
                LOG_WARN_APP("CMDID_C084_ReadSubDeviceIdentitySummary resp for " << m_oDeviceUniqueID <<" that has no adapter info attached");
                return true;
            }

            m_pAdapterInfo->ResponseAdd(p_pRsp, p_pGwReq);
            return true;
        }
        case CMDID_C119_AcknowledgeEventNotification:
        {
            LOG_INFO_APP("CLogicalDevice::ResponseAddSpecial - C119");
            if (!p_pRsp->GetParsedData())
            {
                return true;
            }

            C119_AcknowledgeEventNotification_Resp* pEvent = (C119_AcknowledgeEventNotification_Resp* ) p_pRsp->GetParsedData();

            if (pEvent->eventNo > m_oC119_AcknowledgeEventNotification_Resp_Vector.size())
            {
                m_oC119_AcknowledgeEventNotification_Resp_Vector.resize(pEvent->eventNo+3); //add 2-3 extra events slots

            }
            if (!m_oC119_AcknowledgeEventNotification_Resp_Vector[pEvent->eventNo])
            {
                m_oC119_AcknowledgeEventNotification_Resp_Vector[pEvent->eventNo].reset(new C119_AcknowledgeEventNotification_Resp);
            }

            memcpy(m_oC119_AcknowledgeEventNotification_Resp_Vector[pEvent->eventNo].get(), pEvent, sizeof(C119_AcknowledgeEventNotification_Resp) );

            // ? add cmd 48 data also to cmd 48?
            if (!p_pGwReq && !(m_pDevicesTable->m_pConfig->m_bDontAckC119WhenTime_minus1 && pEvent->firstUnacknowledged.u32 == 0xffFFffFF) )
            {
                m_u16NotificationMask |= NotificationMaskCodesMask_EventNotification;

                CHartCmdWrapper::Ptr pCmd (new CHartCmdWrapper);
                pCmd->LoadRaw(p_pRsp->GetCmdId(), p_pRsp->GetRawDataLen(), p_pRsp->GetRawData() );
                LocalGwReq_Prepare(pCmd);
            }
            return true;
        }

        case CMDID_C769_ReadJoinStatus:
        {
            if (!p_pRsp->GetParsedData())
            {
                return true;
            }
            C769_ReadJoinStatus_Resp * pResp = (C769_ReadJoinStatus_Resp*) p_pRsp->GetParsedData();
            if ( pResp == 0)
            {
                return true;
            }
            LOG_DEBUG("Device " << m_oDeviceUniqueID << ", C769_ReadJoinStatus - status=" << pResp->joinStatus);

            m_nJoinStatus = pResp->joinStatus;

            if (/*((pResp->joinStatus & JoinProcessStatusMask_JoinRequested) == JoinProcessStatusMask_JoinRequested) // JoinProcessStatusMask_JoinRequested is a cumulative bit
                ||*/ ((pResp->joinStatus & JoinProcessStatusMask_JoinFailed) == JoinProcessStatusMask_JoinFailed) )
            {
                SetActive(false);
            } else  if ((pResp->joinStatus & JoinProcessStatusMask_NormalOperationCommencing) == JoinProcessStatusMask_NormalOperationCommencing)
                    {
                        SetActive(true);
                    }

            return true;
        }
        case CMDID_C832_ReadNetworkDeviceIdentity:
        {
            if (!p_pRsp->GetParsedData())
            {
                return true;
            }
            C832_ReadNetworkDeviceIdentity_Resp* pResp = (C832_ReadNetworkDeviceIdentity_Resp*) p_pRsp->GetParsedData();
            if ( pResp == 0)
            {
                return true;
            }

            //init 20 response based on 832 response
            if (m_pC020_ReadLongTag_Resp == 0)
            {
                LOG_DEBUG_APP("CLogicalDevice::ResponseAddSpecial - Dev: UniqueID=" << m_oDeviceUniqueID << ", Nick=" << std::hex << (int)m_u16DeviceNickName << ", Create/Fill m_pC020_ReadLongTag_Resp");
                m_pC020_ReadLongTag_Resp.reset(new C020_ReadLongTag_Resp);
                memcpy(m_pC020_ReadLongTag_Resp->longTag, pResp->LongTag, sizeof(m_pC020_ReadLongTag_Resp->longTag));
            }

            WHartUniqueID uid;
            memcpy(uid.bytes, pResp->DeviceUniqueID , sizeof(uid.bytes));

            m_pDevicesTable->SetDeviceNickname(*this, pResp->Nickname, uid);
            return true;
        }

        case CMDID_C839_ChangeNotification:
        {
            if (!p_pRsp->GetParsedData())
            {
                return true;
            }

            C839_ChangeNotification_Resp* pResp = (C839_ChangeNotification_Resp*)p_pRsp->GetParsedData();

            CLogicalDevice::Ptr pDevice = m_pDevicesTable->GetLogicalDevice(pResp->DeviceAddress, false);
            if (!pDevice)
            {
                WHartUniqueID uid;
                memcpy(uid.bytes, pResp->DeviceAddress , sizeof(uid.bytes));
                pDevice = m_pDevicesTable->DeviceAdd(uid, g_cAddrNicknameInvalid, false);
            }

            LOG_DEBUG_APP("CMDID_C839_ChangeNotification: device=" << GetHex(pResp->DeviceAddress, sizeof(pResp->DeviceAddress)) << " no_notifications=" << (int)pResp->ChangeNotificationNo);

            for (int i=0; i < pResp->ChangeNotificationNo; i++)
            {
                if (IsNetworkScheduleCmd(pResp->ChangeNotifications[i]))
                {
                    pDevice->m_oNotifNetworkSchedule.insert(pResp->ChangeNotifications[i]);
                    pDevice->m_u16NotificationMask |= NotificationMaskCodesMask_NetworkSchedule;
                    LOG_DEBUG_APP("SM CMDID_C839_ChangeNotification: device=" << GetHex(pResp->DeviceAddress, sizeof(pResp->DeviceAddress))<< " SCHEDULE=" << GetCmdDescriptor(pResp->ChangeNotifications[i]).m_pszCmdName);
                }
                else //default is network topology
                {
                    pDevice->m_oNotifNetworkTopology.insert(pResp->ChangeNotifications[i]);
                    pDevice->m_u16NotificationMask |= NotificationMaskCodesMask_NetworkTopology;
                    LOG_DEBUG_APP("SM CMDID_C839_ChangeNotification: device=" << GetHex(pResp->DeviceAddress, sizeof(pResp->DeviceAddress)) << " TOPOLOGY=" << GetCmdDescriptor(pResp->ChangeNotifications[i]).m_pszCmdName);
                }
            }
            return true;
        }

        case CMDID_C969_WriteGraphNeighbourPair:
        {
            if (!p_pRsp->GetParsedData())
            {
                return true;
            }
            if (!m_pDevicesTable->GetGatewayConfig()->NmBurstsCachingEnabled())
            {
                return false;
            }
            C969_WriteGraphNeighbourPair_Resp* pResp = (C969_WriteGraphNeighbourPair_Resp*)p_pRsp->GetParsedData();
            m_oGraphNeighbors[pResp->m_unGraphID].insert(pResp->m_unNeighborNickname);

            return true;
        }

        case CMDID_C970_DeleteGraphConnection:
        {
            if (!p_pRsp->GetParsedData())
            {
                return true;
            }

            if (!m_pDevicesTable->GetGatewayConfig()->NmBurstsCachingEnabled())
            {
                return false;
            }

            C970_DeleteGraphConnection_Resp* pResp = (C970_DeleteGraphConnection_Resp*)p_pRsp->GetParsedData();
            CGraphNeighborsMap::iterator it = m_oGraphNeighbors.find(pResp->m_unGraphID);
            if (it != m_oGraphNeighbors.end())
            {
                it->second.erase(pResp->m_unNeighborNickname);
            }
            return true;
        }

        case CMDID_C967_WriteLink:
        {
            if (!p_pRsp->GetParsedData())
            {
                return true;
            }
            if (!m_pDevicesTable->GetGatewayConfig()->NmBurstsCachingEnabled())
            {
                return false;
            }
            C967_WriteLink_Resp* pResp = (C967_WriteLink_Resp*)p_pRsp->GetParsedData();
            LinksKey key = MakeLinksKey(pResp->m_ucSuperframeID, pResp->m_unSlotNumber, pResp->m_unNeighborNickname);
            m_oLinksMap[key] = *pResp;
            return true;
        }

        case CMDID_C968_DeleteLink:
        {
            if (!p_pRsp->GetParsedData())
            {
                return true;
            }
            if (!m_pDevicesTable->GetGatewayConfig()->NmBurstsCachingEnabled())
            {
                return false;
            }
            C968_DeleteLink_Resp* pResp = (C968_DeleteLink_Resp*)p_pRsp->GetParsedData();
            LinksKey key = MakeLinksKey(pResp->m_ucSuperframeID, pResp->m_unSlotNumber, pResp->m_unNeighborNickname);
            CLinksMap::iterator it = m_oLinksMap.find(key);
            if (it != m_oLinksMap.end())
            {
                m_oLinksMap.erase(it);
            }
            return true;
        }

        case CMDID_C965_WriteSuperframe:
        {
            if (!p_pRsp->GetParsedData())
            {
                return true;
            }
            if (!m_pDevicesTable->GetGatewayConfig()->NmBurstsCachingEnabled())
            {	return false;
            }
            C965_WriteSuperframe_Resp* pResp = (C965_WriteSuperframe_Resp*)p_pRsp->GetParsedData();
            m_oSuperframesMap[pResp->m_ucSuperframeID] = *pResp;
            return true;
        }

        case CMDID_C966_DeleteSuperframe:
        {
            if (!p_pRsp->GetParsedData())
            {
                return true;
            }
            if (!m_pDevicesTable->GetGatewayConfig()->NmBurstsCachingEnabled())
            {	return false;
            }
            C966_DeleteSuperframe_Resp* pResp = (C966_DeleteSuperframe_Resp*)p_pRsp->GetParsedData();
            CSuperframesMap::iterator it = m_oSuperframesMap.find(pResp->m_ucSuperframeID);
            if (it != m_oSuperframesMap.end())
            {	m_oSuperframesMap.erase(it);
            }
            return true;
        }

        case CMDID_C974_WriteRoute:
        {
            if (!p_pRsp->GetParsedData())
            {
                return true;
            }
            if (!m_pDevicesTable->GetGatewayConfig()->NmBurstsCachingEnabled())
            {	return false;
            }
            C974_WriteRoute_Resp* pResp = (C974_WriteRoute_Resp*)p_pRsp->GetParsedData();
            m_oRoutesMap[pResp->m_ucRouteID] = *pResp;
            return true;
        }

        case CMDID_C975_DeleteRoute:
        {
            if (!p_pRsp->GetParsedData())
            {
                return true;
            }
            if (!m_pDevicesTable->GetGatewayConfig()->NmBurstsCachingEnabled())
            {	return false;
            }
            C975_DeleteRoute_Resp* pResp = (C975_DeleteRoute_Resp*)p_pRsp->GetParsedData();
            CRoutesMap::iterator it = m_oRoutesMap.find(pResp->m_ucRouteID);
            if (it != m_oRoutesMap.end())
            {	m_oRoutesMap.erase(it);
            }
            return true;
        }

        case CMDID_C976_WriteSourceRoute:
        {
            if (!p_pRsp->GetParsedData())
            {
                return true;
            }
            if (!m_pDevicesTable->GetGatewayConfig()->NmBurstsCachingEnabled())
            {	return false;
            }
            C976_WriteSourceRoute_Resp* pResp = (C976_WriteSourceRoute_Resp*)p_pRsp->GetParsedData();
            m_oSourceRoutesMap[pResp->m_ucRouteID] = *pResp;
            return true;
        }

        case CMDID_C977_DeleteSourceRoute:
        {
            if (!p_pRsp->GetParsedData())
            {
                return true;
            }
            if (!m_pDevicesTable->GetGatewayConfig()->NmBurstsCachingEnabled())
            {
                return false;
            }
            C977_DeleteSourceRoute_Resp* pResp = (C977_DeleteSourceRoute_Resp*)p_pRsp->GetParsedData();
            CSourceRoutesMap::iterator it = m_oSourceRoutesMap.find(pResp->m_ucRouteID);
            if (it != m_oSourceRoutesMap.end())
            {
                m_oSourceRoutesMap.erase(it);
            }
            return true;
        }

        case CMDID_C973_WriteService:
        {
            if (!p_pRsp->GetParsedData())
            {
                return true;
            }
            if (!m_pDevicesTable->GetGatewayConfig()->NmBurstsCachingEnabled())
            {
                return false;
            }
            C973_WriteService_Resp* pResp = (C973_WriteService_Resp*)p_pRsp->GetParsedData();
            m_oServicesMap[pResp->m_ucServiceID] = *pResp;
            return true;
        }

        case CMDID_C801_DeleteService:
        {
            if (!p_pRsp->GetParsedData())
            {
                return true;
            }
            if (!m_pDevicesTable->GetGatewayConfig()->NmBurstsCachingEnabled())
            {	return false;
            }
            C801_DeleteService_Resp* pResp = (C801_DeleteService_Resp*)p_pRsp->GetParsedData();
            CServicesMap::iterator it = m_oServicesMap.find(pResp->m_ucServiceId);
            if (it != m_oServicesMap.end())
            {
                m_oServicesMap.erase(it);
            }
            return true;
        }
	}
	//LOG_INFO("CLogicalDevice::ResponseAddSpecial - added nothing");
	return false;
}

void CLogicalDevice::RemoveCMDID_C000_ReadUniqueIdentifierFromCache()
{
	// TODO: Beni - Check
    LOG_INFO_APP("CLogicalDevice::RemoveCMDID_C000_ReadUniqueIdentifierFromCache - Device UniqueID=" << m_oDeviceUniqueID << ", NickName=" << std::hex << (int)m_u16DeviceNickName << ". Reset m_pC000_ReadUniqueIdentifier_Resp");
	m_pC000_ReadUniqueIdentifier_Resp.reset();
	m_oCachedResponses.erase(CMDID_C000_ReadUniqueIdentifier);
}


void CLogicalDevice::ResponseAdd(uint8_t p_u8DeviceStatus, uint8_t p_u8DeviceStatusExt, const CHartCmdWrapperList & p_oResponses, CGwRequest * p_pGwReq )
{
	if (p_u8DeviceStatus || p_u8DeviceStatusExt)
	{
		LOG_INFO_APP(" ResponseAdd: device= " << m_oDeviceUniqueID << " deviceStatus = " << std::hex <<(int)p_u8DeviceStatus << " deviceExtendedStatus = " << std::hex << (int)p_u8DeviceStatusExt << " indicate = " << (p_pGwReq == NULL) );
	}
	else
	{
		LOG_DEBUG_APP(" ResponseAdd: device= " << m_oDeviceUniqueID << " deviceStatus = " << std::hex <<(int)p_u8DeviceStatus << " deviceExtendedStatus = " << std::hex << (int)p_u8DeviceStatusExt << " indicate = " << (p_pGwReq == NULL) );
	}

	if ( (m_u16DeviceNickName == NetworkManager_Nickname() || m_u16DeviceNickName == Gateway_Nickname()) && (p_u8DeviceStatus || p_u8DeviceStatusExt))
	{
		p_u8DeviceStatus = 0;
		p_u8DeviceStatusExt = 0;
		LOG_INFO_APP("ResponseAdd: NM or GW and statuses not 0 -> force to 0");
	}

	CHartCmdWrapperList::const_iterator itRsp = p_oResponses.begin();

	for ( ; itRsp != p_oResponses.end(); itRsp++ )
	{
		CHartCmdWrapper::Ptr pRsp = *itRsp;

		LOG_DEBUG_APP(" ResponseAdd: device= " << m_oDeviceUniqueID << CHartCmdWrapperShortDetails(*pRsp) );

		TMetaCmdsInfo * metaCmdsInfo = pRsp->MetaCmdInfoGet();
		if (metaCmdsInfo != 0)
		{
			if (!(m_oDeviceUniqueID == metaCmdsInfo->m_oInnerCmdUniqueId))
			{
				//fwd command to appropriate device
				CLogicalDevice::Ptr pInnerDev = m_pDevicesTable->GetLogicalDevice(pRsp->MetaCmdInfoGet()->m_oInnerCmdUniqueId);

				if (pInnerDev == 0 && m_u16DeviceNickName == NetworkManager_Nickname())
				{
					pInnerDev = m_pDevicesTable->DeviceAdd(metaCmdsInfo->m_oInnerCmdUniqueId, g_cAddrNicknameInvalid, false);
				}

				if (pInnerDev != 0)
				{
					CHartCmdWrapperList oList;
					oList.push_back(pRsp);
					pInnerDev->ResponseAdd(pRsp->GetDeviceStatus(), 0, oList, p_pGwReq);
				}
				continue;
			}
		}

		//forward 832 resp that came to nm to the appropriate logical device
		if (m_u16DeviceNickName == NetworkManager_Nickname() && pRsp->GetCmdId() == CMDID_C832_ReadNetworkDeviceIdentity)
		{
			C832_ReadNetworkDeviceIdentity_Resp* p832Rsp = (C832_ReadNetworkDeviceIdentity_Resp*) pRsp->GetParsedData();
			if (p832Rsp != 0 && p832Rsp->Nickname != NetworkManager_Nickname())
			{
				CLogicalDevice::Ptr pTargetDevice = m_pDevicesTable->GetLogicalDevice(p832Rsp->DeviceUniqueID, false);
				if( pTargetDevice != 0)
				{
				    pTargetDevice->ResponseAddSpecial(p_u8DeviceStatus, p_u8DeviceStatusExt, pRsp, p_pGwReq);
				}
				continue;
			}
		}

		if (p_pGwReq == 0) //indicate
		{
		    RegisterBackMessagesReceivedFromDev();
		}
		else
		{
		    RegisterAckMessagesReceivedFromDev();
		}

		if (ResponseAddSpecial(p_u8DeviceStatus, p_u8DeviceStatusExt, pRsp, p_pGwReq))
		{
			continue;
		}

		//if error code, continue for now, maybe in future on some cache cmds remove cache
		if (IsResponseCodeError(pRsp->GetResponseCode()))
		{
			LOG_DEBUG_APP("rsp_code error  " << pRsp );
			continue;
		}


 		if (!pRsp->GetParsedData())
 		{
 			LOG_ERROR_APP("error on parsing  " << pRsp );
 			continue;
 		}

		if (IsCachedResponseRead(pRsp->GetCmdId()) || m_pDevicesTable->m_pConfig->IsSpecificReadCmd(pRsp->GetCmdId()))
		{
			LOG_DEBUG_APP("ResponseAdd -- CachedResponseRead for cmd : " << pRsp->GetCmdId());
			m_oCachedResponses[pRsp->GetCmdId()] = CreateChachedResponse(pRsp.get());
			continue;
		}

		if (!IsBurstResponse(pRsp->GetCmdId()) && !m_pDevicesTable->m_pConfig->IsSpecificBurstCmd(pRsp->GetCmdId())) //&& !m_bSpecificBurstCmd
		{
			continue;
		}

		//is burst message or response  || is specific burst message
		//TODO: if send burst directly to HostApp => comment the code bellow ...
		if (p_pGwReq == 0)
		{
			// is burst response
			m_oNotifBurstCmds.insert(pRsp->GetCmdId());
			m_u16NotificationMask |= NotificationMaskCodesMask_BurstMode;
		}

		CCachedResponseListPtr pDevList;

		CBurstResponseMap::iterator it = m_oBurstResponses.find(pRsp->GetCmdId());

		if (it == m_oBurstResponses.end() )
		{
			pDevList.reset(new CCachedResponseList);
			m_oBurstResponses.insert(std::make_pair(pRsp->GetCmdId(), pDevList));
		}
		else
		{
			pDevList = it->second;
		}

		CCachedResponseList::iterator pExistingRsp = findMatchingResponseInList(pDevList, pRsp);

		CCachedResponse::Ptr pNewResponse = CreateChachedResponse(pRsp.get());

		pNewResponse->m_nTimeoutPeriod = (!p_pGwReq) ?
			(m_pDevicesTable->GetGatewayConfig()->GwBurstRespTimeout()) :
			(m_pDevicesTable->GetGatewayConfig()->GwRespTimeout());

		pNewResponse->m_bIndicate = (p_pGwReq == NULL);

		if ( pExistingRsp != pDevList->end())
		{
			//keep track of the total nb. of received messages of this type
			pNewResponse->m_nRecvCounter += (*pExistingRsp)->m_nRecvCounter;

            LOG_DEBUG("Device " << m_oDeviceUniqueID << " remove from cache, cmdId=" << std::dec << (*pExistingRsp)->m_usCmdId << ", RecvTime=" << (*pExistingRsp)->m_nRecvTime);

			//remove older exisiting response since it will be replaced;
			pDevList->erase(pExistingRsp);
		}


		//make room if cache is full
		int nMaxCmdNo = GetMaxNoForCacheCommand(pRsp->GetCmdId());
		if ((int)pDevList->size() >= nMaxCmdNo )
		{
			//remove the oldest response from list
			pDevList->pop_back();
		}

		pDevList->push_front(pNewResponse);
	}

	if ( (m_u8DeviceStatus & (~FieldDeviceStatusesMask_ConfigurationChanged)) != (p_u8DeviceStatus & (~FieldDeviceStatusesMask_ConfigurationChanged)))
	{
		m_u16NotificationMask |= NotificationMaskCodesMask_DeviceStatus;
	}


    if (m_u8DeviceStatus != p_u8DeviceStatus || m_u8DeviceStatusExt != p_u8DeviceStatusExt)
    {
        LOG_INFO_APP("CLogicalDevice::ResponseAdd: Device " << m_oDeviceUniqueID << ", update Device Status: oldStatus=" << (int)m_u8DeviceStatus
                       << ", newStatus=" << (int)p_u8DeviceStatus << ", oldExStatus=" << (int)m_u8DeviceStatusExt << ", newExStatus=" << (int)p_u8DeviceStatusExt);
    }
    else
    {
        LOG_DEBUG_APP("CLogicalDevice::ResponseAdd: Device " << m_oDeviceUniqueID << ", update Device Status: oldStatus=" << (int)m_u8DeviceStatus
                       << ", newStatus=" << (int)p_u8DeviceStatus << ", oldExStatus=" << (int)m_u8DeviceStatusExt << ", newExStatus=" << (int)p_u8DeviceStatusExt);
    }

	m_u8DeviceStatus = p_u8DeviceStatus;
	m_u8DeviceStatusExt = p_u8DeviceStatusExt;

	if (m_u8DeviceStatus & FieldDeviceStatusesMask_ConfigurationChanged)
	{
		LOG_INFO_APP ("CLogicalDevice::ResponseAdd: Configuration Changed Status from=" << m_oDeviceUniqueID );

		RemoveCMDID_C000_ReadUniqueIdentifierFromCache();

		m_u16NotificationMask |= NotificationMaskCodesMask_DeviceConfiguration;

		if (!LocalGwReq_IsPending(CMDID_C000_ReadUniqueIdentifier) && !LocalGwReq_IsPending(CMDID_C038_ResetConfigurationChangedFlag))
		{
			AddIdInfoRequest();
		}
	}
	// else // Beni - check the flow
	if (m_u8DeviceStatus & FieldDeviceStatusesMask_MoreStatusAvailable)
	{
		LOG_INFO_APP ("CLogicalDevice::ResponseAdd: MoreStatusAvailable from=" << m_oDeviceUniqueID );

		LocalGwReq_Add_C048();
	}
}

bool CLogicalDevice::ResponseGet(CHartCmdWrapper::Ptr & p_pReq, CHartCmdWrapper::Ptr & p_pRsp, uint8_t & u8RspCode)
{
	int nCmdId = p_pReq->GetCmdId();

	if ( !IsCachedResponseRead(nCmdId) &&
		 !IsBurstResponse(nCmdId) &&
		 !IsCachedResponseCustom(nCmdId) &&
		 !m_pDevicesTable->m_pConfig->IsSpecificReadCmd(nCmdId) &&
		 !m_pDevicesTable->m_pConfig->IsSpecificBurstCmd(nCmdId) )
	{
		return false;
	}


	u8RspCode = p_pReq->GetParsedFromRaw();
	if (u8RspCode != RCS_N00_Success)
	{
		//send error back
		LOG_DEBUG_APP("ResponseGet: parsing error");
		p_pRsp.reset(new CHartCmdWrapper);
		p_pRsp->LoadParsed(p_pReq->GetCmdId(), 0, NULL, u8RspCode, m_u8DeviceStatus);
		return true;
	}

	if (IsCachedResponseRead(nCmdId) || m_pDevicesTable->m_pConfig->IsSpecificReadCmd(nCmdId))
	{
	    return GetCachedReadResponse(p_pReq, p_pRsp);
	}

	if (IsCachedResponseCustom(nCmdId))
	{
	    return GetCachedCustomResponse(p_pReq, p_pRsp);
	}

	if (IsBurstResponse(nCmdId) || m_pDevicesTable->m_pConfig->IsSpecificBurstCmd(nCmdId))
	{
	    return GetCachedBurstResponse(p_pReq, p_pRsp);
	}

	return false;
}


void  CLogicalDevice::GwLocalRequestRemove(CGwRequest::Ptr p_pReq)
{
	CHostAppReqTransMap::iterator itReq = m_oLocalRequests.find(p_pReq->m_u16TransactionID);

	if (itReq != m_oLocalRequests.end())
	{
	    m_oLocalRequests.erase(itReq);
	}
}

WHartShortAddress  CLogicalDevice::GetNickToSendTo()
{
	if (!IsActive())
	{
		return g_cAddrNicknameInvalid;
	}
	if (m_u16DeviceNickName != g_cAddrNicknameInvalid)
	{
		return m_u16DeviceNickName;
	}
	//sub device
	if (!m_pSubDeviceInfo)
	{
		return g_cAddrNicknameInvalid;
	}

	if (!m_pSubDeviceInfo->m_pAdapter)
	{
		return g_cAddrNicknameInvalid;
	}

	return m_pSubDeviceInfo->m_pAdapter->GetNickToSendTo();
}

bool CLogicalDevice::IsActive()
{
	if (!m_pSubDeviceInfo)
	{
		return m_bDeviceActive;
	}
	//sub device return sub device active && apdater active

	if (!m_pSubDeviceInfo->m_pAdapter)
	{
		return m_bDeviceActive;
	}

	return m_bDeviceActive && m_pSubDeviceInfo->m_pAdapter->IsActive();
}

void CLogicalDevice::SetActive(bool p_bActive)
{
    LOG_INFO_APP("CLogicalDevice::SetActive - Dev: UniqueID=" << m_oDeviceUniqueID << ", CURRENT=" << m_bDeviceActive << ", NEW=" << p_bActive);

	m_bDeviceActive = p_bActive;

    m_pDevicesTable->GetGatewayConfig()->DeviceStatus(m_pDevicesTable->GetGatewayConfig()->DeviceStatus() | FieldDeviceStatusesMask_MoreStatusAvailable);

    m_pDevicesTable->GetGatewayConfig()->DeviceStatus(m_pDevicesTable->GetGatewayConfig()->DeviceStatus() | FieldDeviceStatusesMask_ConfigurationChanged);

	m_pDevicesTable->GetGatewayConfig()->m_u8standardizedStatus2 |= StandardizedStatus2Mask_SubDeviceListChanged;

	if (!m_bDeviceActive)
	{
	    LOG_DEBUG_APP("CLogicalDevice::SetActive - Dev: UniqueID=" << m_oDeviceUniqueID << ", Nick=" << std::hex << (int)m_u16DeviceNickName << ", Reset m_pC000_ReadUniqueIdentifier_Resp");
		m_pC000_ReadUniqueIdentifier_Resp.reset();

        LOG_DEBUG_APP("CLogicalDevice::SetActive - Dev: UniqueID=" << m_oDeviceUniqueID << ", Nick=" << std::hex << (int)m_u16DeviceNickName << ", Reset m_pC020_ReadLongTag_Resp");
		m_pC020_ReadLongTag_Resp.reset();

		m_pC048_ReadAdditionalDeviceStatus_Resp.reset( );
		m_bInitCmdsSent = false;
		m_pServiceTable->Clear();
		m_oLocalRequests.clear();

		//clear statistics
		ResetStatistics();

		/*clear cache for inactive device*/
		m_oGraphNeighbors.clear();
		m_oLinksMap.clear();
		m_oSuperframesMap.clear();
		m_oRoutesMap.clear();
		m_oSourceRoutesMap.clear();
		m_oServicesMap.clear();

		if (m_pAdapterInfo != 0)
		{
			CSubDeviceKeyMap::iterator it = m_pAdapterInfo->m_oSubDeviceKeyMap.begin();
			for ( ; it !=  m_pAdapterInfo->m_oSubDeviceKeyMap.end() ; ++it)
			{
 				CLogicalDevice::Ptr pSubDev = m_pDevicesTable->GetLogicalDevice(it->second);
				if (pSubDev != 0)
				{
				    pSubDev->SetActive(false);
				}
			}
			m_pAdapterInfo->RestartDiscoveryFlow();
		}
	}
}

void CLogicalDevice::ResetStatistics()
{
	m_u16StxMessagesSentTo = 0;
	m_u16AckMessagesReceivedFrom = 0;
	m_u16BackMessagesReceivedFrom = 0;
}

uint16_t CLogicalDevice::GetStxMessagesSentToDev()
{
	return m_u16StxMessagesSentTo;
}

uint16_t CLogicalDevice::GetAckMessagesReceivedFromDev()
{
	return m_u16AckMessagesReceivedFrom;
}

uint16_t CLogicalDevice::GetBackMessagesReceivedFromDev()
{
	return m_u16BackMessagesReceivedFrom;
}

void CLogicalDevice::RegisterStxMessagesSentToDev(uint16_t m_u16Nb/* = 1*/)
{
	m_u16StxMessagesSentTo += m_u16Nb;

	if (m_u16StxMessagesSentTo < m_u16Nb) //overflow
	{
	    m_u16AckMessagesReceivedFrom = 0;
		m_u16BackMessagesReceivedFrom = 0;
	}
}

void CLogicalDevice::RegisterAckMessagesReceivedFromDev(uint16_t m_u16Nb /*= 1*/)
{
	m_u16AckMessagesReceivedFrom += m_u16Nb;

	if (m_u16AckMessagesReceivedFrom < m_u16Nb) //overflow
	{
	    m_u16StxMessagesSentTo = 0;
		m_u16BackMessagesReceivedFrom = 0;
	}
}

void CLogicalDevice::RegisterBackMessagesReceivedFromDev(uint16_t m_u16Nb/* = 1*/)
{
	m_u16BackMessagesReceivedFrom += m_u16Nb;

	if (m_u16BackMessagesReceivedFrom < m_u16Nb) //overflow
	{
	    m_u16StxMessagesSentTo = 0;
		m_u16AckMessagesReceivedFrom = 0;
	}
}

bool CLogicalDevice::Refresh()
{
	if (!IsActive())
	{
		/// TODO: after an inactivity TIMEOUT (?1h or ?1day) delete cache keep only stats
		return false;
	}
	bool bNewLocalGwReq = false;

	if (CheckNeedForIdInfo())
	{
		bNewLocalGwReq = true;
	}

	if(CheckNeedUnivCmds())
	{
		bNewLocalGwReq = true;
	}

	if (m_pAdapterInfo)
	{
		bNewLocalGwReq |= m_pAdapterInfo->Refresh();
	}

	return bNewLocalGwReq;
}

void  CLogicalDevice::GwLocalRequestTransmitConfirm(CGwRequest::Ptr p_pReq, CHartCmdWrapperList & responses)
{
}

CGwRequest::Ptr CLogicalDevice::LocalGwReq_IsPending(int p_nCmdId)
{
	CHostAppReqTransMap::iterator itReq = m_oLocalRequests.begin();

	for (;itReq != m_oLocalRequests.end(); itReq++)
	{
		CHartCmdWrapperList::iterator itCmd = itReq->second->m_oCmdList.begin();

		for (;itCmd != itReq->second->m_oCmdList.end(); itCmd++)
		{
			if ((*itCmd)->GetCmdId() == p_nCmdId)
			{
				return itReq->second;
			}
		}
	}

	return CGwRequest::Ptr();
}

CGwRequest::Ptr CLogicalDevice::LocalGwReq_ClearPending(std::set<int>& p_oCmdsSet)
{
	CHostAppReqTransMap::iterator itReq = m_oLocalRequests.begin();

	for (;itReq != m_oLocalRequests.end(); itReq++)
	{
		CHartCmdWrapperList::iterator itCmd = itReq->second->m_oCmdList.begin();

		for (;itCmd != itReq->second->m_oCmdList.end();)
		{
			if (p_oCmdsSet.count((*itCmd)->GetCmdId()) !=0 )
 			{
			    itCmd = itReq->second->m_oCmdList.erase(itCmd);
			}
			else
			{
			    ++itCmd;
			}
		}
	}

	return CGwRequest::Ptr();
}

//if need cmd #0 response and to req
//if need cmd #20 response and to req
CGwRequest::Ptr CLogicalDevice::CheckNeedForIdInfo()
{
	if (!m_pDevicesTable->m_pConfig->m_nSendIdInfoCmds)
	{
		return CGwRequest::Ptr();
	}

	if (m_u16DeviceNickName == Gateway_Nickname() || m_u16DeviceNickName == NetworkManager_Nickname())
	{
		//add fake responses?
		return CGwRequest::Ptr();
	}

	if (m_pC000_ReadUniqueIdentifier_Resp && m_pC020_ReadLongTag_Resp)
	{
		return CGwRequest::Ptr();
	}

	//LOG_DEBUG_APP("CLogicalDevice::CheckNeedForIdInfo");

	bool bNeed0 = !m_pC000_ReadUniqueIdentifier_Resp && !LocalGwReq_IsPending(0);
	bool bNeed20 = !m_pC020_ReadLongTag_Resp && !LocalGwReq_IsPending(20);

	return AddIdInfoRequest(bNeed0, bNeed20);
}


//if need cmd #0 response and to req
//if need cmd #20 response and to req
CGwRequest::Ptr CLogicalDevice::CheckNeedUnivCmds()
{
	if (!m_pDevicesTable->m_pConfig->m_nSendIdInfoCmds)
	{
		return CGwRequest::Ptr();
	}

	if (!m_pDevicesTable->m_pConfig->m_bBuildUnivCommandsCache)
	{
		return CGwRequest::Ptr();
	}

	if (m_u16DeviceNickName == Gateway_Nickname() || m_u16DeviceNickName == NetworkManager_Nickname() || m_nDeviceType == CLogicalDevice::DeviceType_AccessPoint)
	{
		//add fake responses?
		return CGwRequest::Ptr();
	}

	if (!m_pC000_ReadUniqueIdentifier_Resp || !m_pC020_ReadLongTag_Resp)
	{
		return CGwRequest::Ptr();
	}


	if ( m_pSubDeviceInfo != 0 )
	{
		return CGwRequest::Ptr();
	}
	//p_u16CmdId == 7 || p_u16CmdId == 8 || p_u16CmdId == 11 || p_u16CmdId == 12
	//|| p_u16CmdId == 13 || p_u16CmdId == 14 ||p_u16CmdId == 15 || p_u16CmdId == 16  || p_u16CmdId == 48

	int	pUnivReadCommands[] = {7, 8, 12, 13, 14, 15, 16, 48};
	int nUnivReadCommandsLen = sizeof(pUnivReadCommands) / sizeof(int);

	CGwRequest::Ptr pReq;
	int i;
	for (i=0; i < nUnivReadCommandsLen; i++)
	{
		int nCmdId = pUnivReadCommands[i];
		if (m_oCachedResponses.find(nCmdId) != m_oCachedResponses.end() || LocalGwReq_IsPending(nCmdId))
		{
			continue;
		}

		if (!pReq)
		{
			pReq = LocalGwReq_Prepare(nCmdId);

			if (!pReq)
			{
				return pReq;
			}
		}

		CHartCmdWrapper::Ptr pCmdReq(new CHartCmdWrapper);
		pCmdReq->LoadRaw(nCmdId, 0, NULL);
		pReq->m_oCmdList.push_back(pCmdReq);
	}


	int	pSimpleBurstsCommands[] = {1, 2, 3};
	int nSimpleBurstsCommandsLen = sizeof(pSimpleBurstsCommands) / sizeof(int);

	for (i=0; i < nSimpleBurstsCommandsLen; i++)
	{
		int nCmdId = pSimpleBurstsCommands[i];
		if (m_oBurstResponses.find(nCmdId) != m_oBurstResponses.end() || LocalGwReq_IsPending(nCmdId))
		{
			continue;
		}

		if (!pReq)
		{
			pReq = LocalGwReq_Prepare(nCmdId);

			if (!pReq)
			{
				return pReq;
			}
		}

		CHartCmdWrapper::Ptr pCmdReq(new CHartCmdWrapper);
		pCmdReq->LoadRaw(nCmdId, 0, NULL);
		pReq->m_oCmdList.push_back(pCmdReq);
	}

	if (pReq)
	{
		LOG_INFO_APP("CLogicalDevice::CheckNeedUnivCmds " << pReq << pReq->m_oCmdList);
	}

	return pReq;
}

void CLogicalDevice::LocalGwReq_Add_C048()
{
	if (LocalGwReq_IsPending(CMDID_C048_ReadAdditionalDeviceStatus))
	{
		return;
	}

	CGwRequest::Ptr pReq = LocalGwReq_Prepare(CMDID_C048_ReadAdditionalDeviceStatus);
	if (!pReq)
	{
		return;
	}

	CHartCmdWrapper::Ptr pCmd(new CHartCmdWrapper);

	CCachedResponseMap::iterator itCachedRsp = m_oCachedResponses.find(CMDID_C048_ReadAdditionalDeviceStatus);

	if (itCachedRsp == m_oCachedResponses.end())
	{
		pCmd->LoadRaw(CMDID_C048_ReadAdditionalDeviceStatus, 0, NULL);
	}
	else
	{
		CCachedResponse::Ptr pChachedRsp = itCachedRsp->second;

		pCmd->LoadRaw(pChachedRsp->m_usCmdId, pChachedRsp->m_u8Len, pChachedRsp->m_pResponse.get());
	}

	pReq->m_oCmdList.push_back(pCmd);
}

CGwRequest::Ptr CLogicalDevice::LocalGwReq_Prepare(int p_nCmdId)
{
	if(m_u16DeviceNickName == Gateway_Nickname())
	{
		return CGwRequest::Ptr();
	}

	CService::Ptr pService;

	LOG_DEBUG_APP ("LocalGwReq_Prepare: m_nGwRequestService=" << m_pDevicesTable->m_pConfig->m_nGwRequestService);
	if (m_pDevicesTable->m_pConfig->m_nGwRequestService)
		pService = ServiceGet(p_nCmdId); // cmd 0 si 20 same category does not matter
	else
		pService = m_pServiceTable->GetServiceIfExist(p_nCmdId); // cmd 0 si 20 same category does not matter

	if (!pService)
	{
		LOG_ERROR_APP("LocalGwReq_Prepare: no service for devId=" << m_oDeviceUniqueID << " and cmdId=" << std::dec << p_nCmdId);
		return CGwRequest::Ptr();
	}
	//LOG_DEBUG_APP("CLogicalDevice::CheckNeedForIdInfo3");
	CGwRequest::Ptr pReq(new CGwRequest);

	pReq->m_nSrc = CGwRequest::SourceLocalGw;

	// read for m_pDevicesTable->GetGatewayConfig().ini file
	//pReq->m_nRetryNo = g_cMaxLocalGwRetries;
	pReq->m_nRetryNo = m_pDevicesTable->m_pConfig->m_nLocalGwRetries;

	pReq->m_nNextTimeout = GetClockTicks();
	pReq->m_u16TransactionID = GetNextTransactionID();
	pReq->m_oDevUniqueID = m_oDeviceUniqueID;
	pReq->m_u16NickNameToSendTo = GetNickToSendTo();
	pReq->m_pService = pService;
	pReq->m_bByPassIOCache = true;

	m_oLocalRequests.insert(std::make_pair(pReq->m_u16TransactionID, pReq));
	m_pDevicesTable->GetGwRequestList()->push_back(pReq);

	LOG_INFO_APP("LocalGw add: " << pReq << " cmdId=" << std::dec << p_nCmdId);
	return pReq;
}

CGwRequest::Ptr CLogicalDevice::LocalGwReq_Prepare(CHartCmdWrapper::Ptr & p_pCmd)
{
	CGwRequest::Ptr pReq = LocalGwReq_Prepare(p_pCmd->GetCmdId());

	if (pReq)
	{
		pReq->m_oCmdList.push_back(p_pCmd);
	}
	return pReq;
}

	//if need cmd #0 response and to req
	//if need cmd #20 response and to req
CGwRequest::Ptr CLogicalDevice::AddIdInfoRequest(bool p_bNeed0, bool p_bNeed20)
{
	if ( (!p_bNeed0 && !p_bNeed20) || (m_pSubDeviceInfo != 0) )
	{
		return CGwRequest::Ptr();
	}
	LOG_DEBUG_APP("AddIdInfoRequest needC0=" << p_bNeed0 << ", needC20=" << p_bNeed20);
	CGwRequest::Ptr pReq = LocalGwReq_Prepare(0);

	if (!pReq)
	{
		return pReq;
	}

	if (p_bNeed0)
	{
		//add cmd 0
		CHartCmdWrapper::Ptr pCmdReq(new CHartCmdWrapper);
		pCmdReq->LoadRaw(CMDID_C000_ReadUniqueIdentifier, 0, NULL);
		pReq->m_oCmdList.push_back(pCmdReq);
	}

	if (p_bNeed20)
	{
		//add cmd 20
		CHartCmdWrapper::Ptr pCmdReq(new CHartCmdWrapper);
		pCmdReq->LoadRaw(CMDID_C020_ReadLongTag, 0,NULL);
		pReq->m_oCmdList.push_back(pCmdReq);
	}

	return pReq;
}

CGwRequest::Ptr CLogicalDevice::CheckNeedRequestService(CService::Ptr & p_pService)
{
	if (m_u16DeviceNickName == NetworkManager_Nickname() || m_u16DeviceNickName ==  Gateway_Nickname())
	{
		LOG_DEBUG_APP("CLogicalDevice::CheckNeedRequestService: nick=" << m_u16DeviceNickName << "(" << (m_u16DeviceNickName == NetworkManager_Nickname()? "NM" : "GW") << ") no need for service") ;
		return CGwRequest::Ptr();
	}

	CHartCmdWrapper::Ptr pCmd = p_pService->CheckNeedReqService();

	if (!pCmd)
	{
		return CGwRequest::Ptr();
	}


	CLogicalDevice::Ptr pNM = m_pDevicesTable->GetLogicalDeviceNM();

	if (!pNM)
	{
		LOG_ERROR_APP("CLogicalDevice::CheckNeedRequestService no NM in logical device table!!!");
		return CGwRequest::Ptr();
	}

	CGwRequest::Ptr pReq = pNM->LocalGwReq_Prepare(CMDID_C799_RequestService);

	if (!pReq)
	{
		return CGwRequest::Ptr();
	}

	pReq->m_oCmdList.push_back(pCmd);

	return pReq;
}


CService::Ptr CLogicalDevice::ServiceGet(int p_nCmdId)
{
	if (m_u16DeviceNickName == g_cAddrNicknameInvalid)
	{
		LOG_ERROR_APP("CLogicalDevice::ServiceGet: nick not set yet -> cannot get service" );
		return CService::Ptr();
	}

	if (!IsActive())
	{
		LOG_ERROR_APP("CLogicalDevice::ServiceGet: device not operational (no session) -> cannot get service" );
		return CService::Ptr();
	}

	CService::Ptr pService = m_pServiceTable->GetService(p_nCmdId);
	if (!pService)
	{
		return pService;
	}

	CGwRequest::Ptr pReq = CheckNeedRequestService(pService);
	if (pReq)
	{
		m_pDevicesTable->GetGwRequestList()->push_back(pReq);
		LOG_DEBUG_APP("CLogicalDevice::ServiceGet: add req for service request for nick=" <<std::hex << m_u16DeviceNickName << " serviceId="<< pService->m_stServiceEntry.m_ucServiceId << " cmdId" << p_nCmdId);
	}
	return pService;
}

uint8_t * CLogicalDevice::GetLongTag()
{
	if (m_u16DeviceNickName == Gateway_Nickname())
	{
		return NULL;
	}

	if (m_pC020_ReadLongTag_Resp == 0)
	{
		return NULL;
	}

	return (uint8_t*)(m_pC020_ReadLongTag_Resp->longTag);
}

void CLogicalDevice::ServiceReqResponse(const C799_RequestService_Req * p_pReq, const C799_RequestService_Resp * p_pResp, int p_nResponseCode)
{
	m_pServiceTable->ServiceReqResponse(p_pReq, p_pResp, p_nResponseCode);
}

uint8_t	CLogicalDevice::ServiceWrite(const C973_WriteService_Req * p_pReq, const C973_WriteService_Resp * p_pResp, int p_nResponseCode)
{
	uint8_t res = m_pServiceTable->ServiceWrite(p_pReq, p_pResp, p_nResponseCode);
	CheckNeedForIdInfo();

	return res;
}

uint8_t	CLogicalDevice::ServiceDelete(const C801_DeleteService_Req * p_pReq, const C801_DeleteService_Resp * p_pResp, int p_nResponseCode)
{
	return m_pServiceTable->ServiceDelete(p_pReq, p_pResp, p_nResponseCode);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace gateway
} // namespace hart7

