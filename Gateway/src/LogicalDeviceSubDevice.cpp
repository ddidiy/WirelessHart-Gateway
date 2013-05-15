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

#include <WHartGateway/LogicalDeviceSubDevice.h>

#include <WHartGateway/AllGatewayCommands.h>
#include <WHartGateway/LogicalDevice.h>
#include <WHartGateway/LogicalDeviceTable.h>

namespace hart7 {
namespace gateway {

//returns treated or not
bool CLogicalDeviceInfoAdapter::ResponseAdd(CHartCmdWrapper::Ptr & p_pRsp, CGwRequest * p_pGwReq)
{
	switch (p_pRsp->GetCmdId())
	{
	case CMDID_C074_ReadIOSystemCapabilities:
	{	return ResponseAdd_C074(p_pRsp, p_pGwReq);
	}
	case CMDID_C084_ReadSubDeviceIdentitySummary:
	{	return ResponseAdd_C084(p_pRsp, p_pGwReq);
	}
	case CMDID_C075_PollSubDevice:
	{	return ResponseAdd_C075(p_pRsp, p_pGwReq);
	}
	case CMDID_C048_ReadAdditionalDeviceStatus:
	{	return ResponseAdd_C048(p_pRsp, p_pGwReq);
	}
	default:
		LOG_WARN_APP("CLogicalDeviceInfoAdapter::ResponseAdd: cmdId=" << p_pRsp->GetCmdId()<< " not implemented");
	    return false;
	}

	return false;
}

bool CLogicalDeviceInfoAdapter::ResponseAdd_C074(CHartCmdWrapper::Ptr & p_pRsp, CGwRequest * p_pGwReq )
{
	if ( IsResponseCodeError(p_pRsp->GetResponseCode()) || (!p_pRsp->GetParsedData()) )
	{
	    LOG_ERROR_APP("C074_ReadIOSystemCapabilities_Resp for device " << m_pLogicalDevice->m_oDeviceUniqueID
			<< " responseCode = " << (int)(p_pRsp->GetResponseCode()));
		return true;
	}

	bool bCapDiffer = false;
	if (!m_pC074_ReadIOSystemCapabilities_Resp)
	{
		if (!m_pC074_ReadIOSystemCapabilities_Resp)
		{
		    m_pC074_ReadIOSystemCapabilities_Resp.reset(new C074_ReadIOSystemCapabilities_Resp);
		}
		bCapDiffer = true;
	}
	else
	{
	    bCapDiffer = (memcmp(m_pC074_ReadIOSystemCapabilities_Resp.get(), p_pRsp->GetParsedData(), sizeof(m_pC074_ReadIOSystemCapabilities_Resp) ) != 0);
	}

	memcpy(m_pC074_ReadIOSystemCapabilities_Resp.get(), p_pRsp->GetParsedData(), sizeof(m_pC074_ReadIOSystemCapabilities_Resp) );

	LOG_INFO_APP("C074_ReadIOSystemCapabilities_Resp for device " << m_pLogicalDevice->m_oDeviceUniqueID << " :"
				<<  " cards="			<<(int)m_pC074_ReadIOSystemCapabilities_Resp->noOfIOCards
				<< 	" chann="			<< (int)m_pC074_ReadIOSystemCapabilities_Resp->noOfChannelsPerIOCard
				<< 	" dev_per_chann="	<< (int)m_pC074_ReadIOSystemCapabilities_Resp->noOfSubdevicesPerChannel
				<< 	" dev_total="		<< (int)m_pC074_ReadIOSystemCapabilities_Resp->noOfDevicesDetected);

	if (bCapDiffer)
	{
		ClearDiscoveryFlowState();
		CheckNeedAdd_C084_ReadSubDeviceIdentitySummary();
	}

	return true;
}

bool CLogicalDeviceInfoAdapter::ResponseAdd_C084(CHartCmdWrapper::Ptr & p_pRsp, CGwRequest * p_pGwReq )
{
	if (!p_pGwReq || p_pGwReq->m_nSrc != CGwRequest::SourceLocalGw )
	{
		LOG_INFO_APP("C084_ReadSubDeviceIdentitySummary_Resp "
			<< " for device " << m_pLogicalDevice->m_oDeviceUniqueID << " and subdevIndex=" << (int)(m_u16C084CurrentSubdevIndex)
			<< ": !p_pGwReq || p_pGwReq->m_nSrc != CGwRequest::SourceLocalGw -> Ignore");
		return true;
	}

	if (p_pGwReq->m_u16TransactionID != m_u16C084CurrentTransId)
	{
		LOG_INFO_APP("C084_ReadSubDeviceIdentitySummary_Resp "
			<< " for device " << m_pLogicalDevice->m_oDeviceUniqueID << " and subdevIndex=" << (int)(m_u16C084CurrentSubdevIndex)
			<< " : p_pGwReq->m_u16TransactionID (" << (int)p_pGwReq->m_u16TransactionID
			<< ") != m_u16C084CurrentTransId (" << (int)m_u16C084CurrentTransId << ") -> Ignore");
		return true;
	}

	if (!m_pC074_ReadIOSystemCapabilities_Resp)
	{
	    LOG_WARN_APP("C084_ReadSubDeviceIdentitySummary_Resp "
			<< " for device " << m_pLogicalDevice->m_oDeviceUniqueID << " and subdevIndex=" << (int)(m_u16C084CurrentSubdevIndex)
			<< " : C084 without C074 first -> Ignore");
		return true;
	}

	if ( IsResponseCodeError(p_pRsp->GetResponseCode()) || (!p_pRsp->GetParsedData()) )
	{
		LOG_ERROR_APP("C084_ReadSubDeviceIdentitySummary_Resp "
			<< " for device " << m_pLogicalDevice->m_oDeviceUniqueID << " and subdevIndex=" << (int)(m_u16C084CurrentSubdevIndex)
			<< " : responseCode=" << (int)p_pRsp->GetResponseCode());

		CSubDeviceIndexMap::iterator invalidSubdevIt = m_oSubDeviceIndexMap.find(m_u16C084CurrentSubdevIndex);

		if (invalidSubdevIt != m_oSubDeviceIndexMap.end())
		{
			if (invalidSubdevIt->second != 0)
			{
			    WHartUniqueID uid = MakeCorrectedUniqueID(invalidSubdevIt->second->expandedDeviceType, invalidSubdevIt->second->deviceID);
				MakeInactiveSubDevice(uid);
			}
			m_oSubDeviceIndexMap.erase(invalidSubdevIt);
		}

		CheckNeedAdd_C084_ReadSubDeviceIdentitySummary(); //continue to discover the remaining devices
		return true;
	}

	C084_ReadSubDeviceIdentitySummary_Resp* pCmdRsp = (C084_ReadSubDeviceIdentitySummary_Resp*)p_pRsp->GetParsedData();

	LOG_INFO_APP("C084_ReadSubDeviceIdentitySummary_Resp "
		<< " for device " << m_pLogicalDevice->m_oDeviceUniqueID << " and subdevIndex=" << (int)(m_u16C084CurrentSubdevIndex));

	WHartUniqueID oAddressField =  MakeCorrectedUniqueID(pCmdRsp->expandedDeviceType, pCmdRsp->deviceID);

	CLogicalDevice::Ptr pDev = m_pLogicalDevice->m_pDevicesTable->GetLogicalDevice(oAddressField);

	if (pDev == 0)
	{
		pDev = m_pLogicalDevice->m_pDevicesTable->DeviceAdd(oAddressField, g_cAddrNicknameInvalid, true);

		if (!pDev)
		{
			LOG_ERROR_APP("C084_ReadSubDeviceIdentitySummary "
				<< " for device " << m_pLogicalDevice->m_oDeviceUniqueID << " and subdevIndex=" << (int)(m_u16C084CurrentSubdevIndex)
				<< " : can not insert device" << oAddressField);
			return true;
		}
	}

	//process current 84 resp
	m_oSubDevConnectionsNbMap[MakeSubdevConnectionKey(pCmdRsp->ioCard, pCmdRsp->channel)]++;

    LOG_DEBUG_APP("CLogicalDeviceInfoAdapter::ResponseAdd_C084 - Dev: UniqueID=" << pDev->m_oDeviceUniqueID << ", Nick=" << std::hex << (int)pDev->m_u16DeviceNickName
                  << ", Create/Fill m_pC020_ReadLongTag_Resp");
	pDev->m_pC020_ReadLongTag_Resp.reset(new C020_ReadLongTag_Resp);
	memcpy(pDev->m_pC020_ReadLongTag_Resp->longTag, pCmdRsp->tag, sizeof(pCmdRsp->tag));

	boost::shared_ptr<C084_ReadSubDeviceIdentitySummary_Resp>& p84Resp= m_oSubDeviceIndexMap[m_u16C084CurrentSubdevIndex];
	if (p84Resp == 0)
	{
	    p84Resp.reset(new C084_ReadSubDeviceIdentitySummary_Resp);
	}

	memcpy (p84Resp.get(), pCmdRsp, sizeof(C084_ReadSubDeviceIdentitySummary_Resp));

	//send the next 84 req
	CheckNeedAdd_C084_ReadSubDeviceIdentitySummary();

	return true;
}

bool CLogicalDeviceInfoAdapter::ResponseAdd_C075(CHartCmdWrapper::Ptr & p_pRsp, CGwRequest * p_pGwReq )
{
	if (!p_pGwReq || p_pGwReq->m_nSrc != CGwRequest::SourceLocalGw )
	{
		LOG_INFO_APP("C075_PollSubDevice_Resp for device " << m_pLogicalDevice->m_oDeviceUniqueID
			<< " ioCard=" << (int)m_oCurrentPollInfo.ioCard
			<< " channel=" << (int)m_oCurrentPollInfo.channel
			<< " addr=" << (int)m_oCurrentPollInfo.poolingAddress
			<< " : !p_pGwReq || p_pGwReq->m_nSrc != CGwRequest::SourceLocalGw -> Ignore");
		return true;
	}

	if (p_pGwReq->m_u16TransactionID != m_u16C075CurrentTransId)
	{
		LOG_INFO_APP("C075_PollSubDevice_Resp for device " << m_pLogicalDevice->m_oDeviceUniqueID
			<< " ioCard=" << (int)m_oCurrentPollInfo.ioCard
			<< " channel=" << (int)m_oCurrentPollInfo.channel
			<< " addr=" << (int)m_oCurrentPollInfo.poolingAddress
 			<< " : p_pGwReq->m_u16TransactionID (" << (int)p_pGwReq->m_u16TransactionID
			<< ") != m_u16C075CurrentTransId (" << (int)m_u16C075CurrentTransId << ") -> Ignore");
		return true;
	}

	if (IsResponseCodeError(p_pRsp->GetResponseCode()) || (!p_pRsp->GetParsedData()) )
	{
		LOG_ERROR_APP("C075_PollSubDevice_Resp for device " << m_pLogicalDevice->m_oDeviceUniqueID
			<< " ioCard=" << (int)m_oCurrentPollInfo.ioCard
			<< " channel=" << (int)m_oCurrentPollInfo.channel
			<< " addr=" << (int)m_oCurrentPollInfo.poolingAddress
			<< " : responseCode = " << p_pRsp->GetResponseCode());
		// set inactive device on current poll address
		MakeInactiveSubDevice(m_oCurrentPollInfo);
		m_oSubDeviceKeyMap.erase(m_oCurrentPollInfo);
		CheckNeedAdd_C075_PollSubDevice();
		return true;
	}


	if (!m_pC074_ReadIOSystemCapabilities_Resp)
	{
	    LOG_WARN_APP("C075_PollSubDevice_Resp for device " << m_pLogicalDevice->m_oDeviceUniqueID
			<< " : C075 without C074 first");
		return true;
	}

	C075_PollSubDevice_Resp* pCmdRsp = (C075_PollSubDevice_Resp*)p_pRsp->GetParsedData();

	LOG_INFO_APP("C075_PollSubDevice_Resp for device " << m_pLogicalDevice->m_oDeviceUniqueID
		<< " ioCard=" << (int)m_oCurrentPollInfo.ioCard
		<< " channel=" << (int)m_oCurrentPollInfo.channel
		<< " addr=" << (int)m_oCurrentPollInfo.poolingAddress);

	WHartUniqueID oAddressField =  MakeCorrectedUniqueID(pCmdRsp->expandedDeviceType, pCmdRsp->deviceID);

	CLogicalDevice::Ptr pDev = m_pLogicalDevice->m_pDevicesTable->GetLogicalDevice(oAddressField);

	if (pDev == 0)
	{
	    LOG_ERROR_APP("C075_PollSubDevice_Resp for device " << m_pLogicalDevice->m_oDeviceUniqueID
			<< " ioCard=" << (int)m_oCurrentPollInfo.ioCard
			<< " channel=" << (int)m_oCurrentPollInfo.channel
			<< " addr=" << (int)m_oCurrentPollInfo.poolingAddress
			<< " : subdevice does not exist -> Ignore");
		return true;
	}

	CLogicalDevice * pAdapterOfMySubdev = (pDev->m_pSubDeviceInfo == 0) ? 0 : pDev->m_pSubDeviceInfo->m_pAdapter;
	if ( pAdapterOfMySubdev != 0 && pAdapterOfMySubdev != m_pLogicalDevice/*current adapter*/)
	{
		LOG_INFO_APP("C075_PollSubDevice_Resp for device " << m_pLogicalDevice->m_oDeviceUniqueID
			<< " ioCard=" << (int)m_oCurrentPollInfo.ioCard
			<< " channel=" << (int)m_oCurrentPollInfo.channel
			<< " addr=" << (int)m_oCurrentPollInfo.poolingAddress
			<< " : subdev" << oAddressField << " moved from other Adapter");

		if (pAdapterOfMySubdev->m_pAdapterInfo)
		{
			//delete old key
			pAdapterOfMySubdev->m_pAdapterInfo->m_oSubDeviceKeyMap.erase(pDev->m_pSubDeviceInfo->m_oSubDeviceKey);
		}
	}

	pDev->m_pSubDeviceInfo->m_pAdapter = m_pLogicalDevice;

	CSubDevConnectionsNbMap::iterator pos =	m_oSubDevConnectionsNbMap.find(MakeSubdevConnectionKey(m_oCurrentPollInfo));

	if (pos != m_oSubDevConnectionsNbMap.end())
	{
	    pos->second--;
	}

	memcpy(&pDev->m_pSubDeviceInfo->m_oSubDeviceKey, &m_oCurrentPollInfo, sizeof(m_oCurrentPollInfo));

	if (!pDev->m_pC000_ReadUniqueIdentifier_Resp)
	{
        LOG_DEBUG_APP("CLogicalDeviceInfoAdapter::ResponseAdd_C075 - Dev: UniqueID=" << pDev->m_oDeviceUniqueID << ", Nick=" << std::hex << (int)pDev->m_u16DeviceNickName << ", Create m_pC000_ReadUniqueIdentifier_Resp");
		pDev->m_pC000_ReadUniqueIdentifier_Resp.reset(new C000_ReadUniqueIdentifier_Resp);
	}

    LOG_DEBUG_APP("CLogicalDeviceInfoAdapter::ResponseAdd_C075 - Dev: UniqueID=" << pDev->m_oDeviceUniqueID << ", Nick=" << std::hex << (int)pDev->m_u16DeviceNickName << ", Fill m_pC000_ReadUniqueIdentifier_Resp");
	memcpy(pDev->m_pC000_ReadUniqueIdentifier_Resp.get(), pCmdRsp, sizeof(C000_ReadUniqueIdentifier_Resp));

	WHartUniqueID& uid = m_oSubDeviceKeyMap[pDev->m_pSubDeviceInfo->m_oSubDeviceKey]; //inserts if it doesn't exist
	memcpy(uid.bytes, oAddressField.bytes, sizeof(uid.bytes));

	CheckNeedAdd_C075_PollSubDevice();

	return true;
}

bool CLogicalDeviceInfoAdapter::ResponseAdd_C048(CHartCmdWrapper::Ptr & p_pRsp, CGwRequest * p_pGwReq )
{
	if ( m_pLogicalDevice == 0 || m_pLogicalDevice->m_pC048_ReadAdditionalDeviceStatus_Resp == 0)
	{
	    return true;
	}

	//check Sub-Device list changed bit
	if (m_pLogicalDevice->m_pC048_ReadAdditionalDeviceStatus_Resp->standardizedStatus2 & StandardizedStatus2Mask_SubDeviceListChanged)
	{
	    RestartDiscoveryFlow();
	}

	return true;
}

bool CLogicalDeviceInfoAdapter::ResponseGet(CHartCmdWrapper::Ptr & p_pReq, CHartCmdWrapper::Ptr & p_pRsp)
{
	int nCmdId = p_pReq->GetCmdId();

	switch(nCmdId)
	{
        case CMDID_C075_PollSubDevice:
        {
            return ResponseGet_C075(p_pReq, p_pRsp);
        }
        case CMDID_C084_ReadSubDeviceIdentitySummary:
        {
            return ResponseGet_C084(p_pReq, p_pRsp);
        }
        case CMDID_C074_ReadIOSystemCapabilities:
        {
            return ResponseGet_C074(p_pReq, p_pRsp);
        }
        default:
            return false;
	}

	return false;
}

bool CLogicalDeviceInfoAdapter::ResponseGet_C074(CHartCmdWrapper::Ptr & p_pReq, CHartCmdWrapper::Ptr & p_pRsp)
{
	if (m_pC074_ReadIOSystemCapabilities_Resp == 0 || m_pLogicalDevice == 0)
	{
	    return false;
	}

	p_pRsp.reset(new CHartCmdWrapper);
	p_pRsp->LoadParsed(CMDID_C074_ReadIOSystemCapabilities, sizeof(C074_ReadIOSystemCapabilities_Resp),
	                   m_pC074_ReadIOSystemCapabilities_Resp.get(), RCS_N00_Success, m_pLogicalDevice->m_u8DeviceStatus);

	return true;
}

bool CLogicalDeviceInfoAdapter::ResponseGet_C084(CHartCmdWrapper::Ptr & p_pReq, CHartCmdWrapper::Ptr & p_pRsp)
{
	C084_ReadSubDeviceIdentitySummary_Req * pReq = (C084_ReadSubDeviceIdentitySummary_Req*)p_pReq->GetParsedData();
	if (pReq == 0)
	{
	    return false;
	}

	CSubDeviceIndexMap::iterator it = m_oSubDeviceIndexMap.find(pReq->subDeviceIndex);
	if (it == m_oSubDeviceIndexMap.end() || it->second == 0 || m_pLogicalDevice == 0)
	{
	    return false;
	}

	//TODO - copy long tag from subdev because it is newer
	p_pRsp.reset(new CHartCmdWrapper);
	p_pRsp->LoadParsed(CMDID_C084_ReadSubDeviceIdentitySummary, sizeof(C084_ReadSubDeviceIdentitySummary_Resp),
	                   it->second.get(), RCS_N00_Success, m_pLogicalDevice->m_u8DeviceStatus);
	return true;
}

bool CLogicalDeviceInfoAdapter::ResponseGet_C075(CHartCmdWrapper::Ptr & p_pReq, CHartCmdWrapper::Ptr & p_pRsp)
{
	C075_PollSubDevice_Req * pReq = (C075_PollSubDevice_Req*)p_pReq->GetParsedData();
	if (pReq == 0)
	{
	    return false;
	}

	CSubDeviceKeyMap::iterator it = m_oSubDeviceKeyMap.find(*pReq);
	if (it == m_oSubDeviceKeyMap.end())
	{
	    return false;
	}

	CLogicalDevice::Ptr pDev = m_pLogicalDevice->m_pDevicesTable->GetLogicalDevice(it->second);
	if (pDev == 0 || pDev->m_pC000_ReadUniqueIdentifier_Resp == 0)
	{
	    return false;
	}

	p_pRsp.reset(new CHartCmdWrapper);
	p_pRsp->LoadParsed(CMDID_C075_PollSubDevice, sizeof(C075_PollSubDevice_Resp),
	                   pDev->m_pC000_ReadUniqueIdentifier_Resp.get(), RCS_N00_Success, pDev->m_u8DeviceStatus);

	return true;
}

// return true - cmd C075 sent
//			false - cmd C075 not sent
bool CLogicalDeviceInfoAdapter::CheckNeedAdd_C075_PollSubDevice()
{
	if (!m_bDiscoveryInProgress)
	{
		return false;
	}

	if (!m_pC074_ReadIOSystemCapabilities_Resp)
	{
		LOG_WARN_APP("C075_PollSubDevice_Req for device " << m_pLogicalDevice->m_oDeviceUniqueID
			<< " ioCard=" << (int)m_oCurrentPollInfo.ioCard
			<< " channel=" << (int)m_oCurrentPollInfo.channel
			<< " addr=" << (int)m_oCurrentPollInfo.poolingAddress
			<< " : C075 without C074 first");
		return false;
	}

	CSubDevConnectionsNbMap::iterator initial =  m_oSubDevConnectionsNbMap.find(MakeSubdevConnectionKey(m_oCurrentPollInfo));
	CSubDevConnectionsNbMap::iterator pos = initial;

	while (true)
	{
		if (pos == m_oSubDevConnectionsNbMap.end())
		{
			LOG_INFO_APP("Finished device discovery for adapter " << m_pLogicalDevice->m_oDeviceUniqueID
				<< " no_dev=" << (int)(m_pC074_ReadIOSystemCapabilities_Resp->noOfDevicesDetected - 1/*adapter*/));

			m_bDiscoveryInProgress = false;
			CorrectSubdevMaps();

			LOG_DEBUG_APP("m_oSubDeviceIndexMap.size()=" << m_oSubDeviceIndexMap.size()
				<< " m_oSubDeviceKeyMap.size()=" << m_oSubDeviceKeyMap.size());

			return false;
		}
		else if (pos != initial)
		{
			//if we are on a new connection, reinitialize m_oCurrentPollInfo
			m_oCurrentPollInfo.poolingAddress = 0xff; //next will be 0
			m_oCurrentPollInfo.channel = GetChannel(pos->first);
			m_oCurrentPollInfo.ioCard = GetCard(pos->first);
		}

		if (pos->second == 0) // no more subdev connections for this key
		{	++pos;
			continue;
		}

		break;
	}

	m_oCurrentPollInfo.poolingAddress++;

	LOG_INFO_APP("C075_PollSubDevice_Req for device " << m_pLogicalDevice->m_oDeviceUniqueID
		<< " ioCard=" << (int)m_oCurrentPollInfo.ioCard
		<< " channel=" << (int)m_oCurrentPollInfo.channel
		<< " addr=" << (int)m_oCurrentPollInfo.poolingAddress);

	//send another C075
	CHartCmdWrapper::Ptr pCmd(new CHartCmdWrapper);

	pCmd->LoadParsed(CMDID_C075_PollSubDevice, sizeof(m_oCurrentPollInfo), &m_oCurrentPollInfo);
	CGwRequest::Ptr pReq = m_pLogicalDevice->LocalGwReq_Prepare(pCmd);
	m_u16C075CurrentTransId = pReq->m_u16TransactionID;
	return true;
}

// return true - cmd C084 sent
//	  	  false - cmd C084 not sent
bool CLogicalDeviceInfoAdapter::CheckNeedAdd_C084_ReadSubDeviceIdentitySummary()
{
	if (!m_bReadDevicesIdentityInProgress)
	{
	    return false;
	}

	if (!m_pC074_ReadIOSystemCapabilities_Resp)
	{
		LOG_WARN_APP("C084_ReadSubDeviceIdentitySummary_Req "
			<< " for device " << m_pLogicalDevice->m_oDeviceUniqueID
			<< " C084 without C074 first");
		return false;
	}

	m_u16C084CurrentSubdevIndex++; //0 is skipped ; we already know the identity of the adapter

	if ( m_u16C084CurrentSubdevIndex >= m_pC074_ReadIOSystemCapabilities_Resp->noOfDevicesDetected )
	{
		LOG_INFO_APP("Finished Device Identity Discovery" << " for device " << m_pLogicalDevice->m_oDeviceUniqueID);
		m_bReadDevicesIdentityInProgress = false;
		return false;
	}


	LOG_INFO_APP("C084_ReadSubDeviceIdentitySummary_Req: "
		<< " for device " << m_pLogicalDevice->m_oDeviceUniqueID << " and subdevIndex=" << (int)(m_u16C084CurrentSubdevIndex));

	CHartCmdWrapper::Ptr pCmd(new CHartCmdWrapper);
	C084_ReadSubDeviceIdentitySummary_Req req;
	req.subDeviceIndex = (m_u16C084CurrentSubdevIndex);
	pCmd->LoadParsed(CMDID_C084_ReadSubDeviceIdentitySummary, sizeof(C084_ReadSubDeviceIdentitySummary_Req),&req);
	CGwRequest::Ptr pReq = m_pLogicalDevice->LocalGwReq_Prepare(pCmd);
	m_u16C084CurrentTransId = pReq->m_u16TransactionID;

	return pReq != 0;
}

// return true/false if a command was added to the queue
bool CLogicalDeviceInfoAdapter::Refresh()
{
	if ( (m_pLogicalDevice == 0) || (!m_pLogicalDevice->IsActive()) )
	{	return false;
	}

	if (m_pC074_ReadIOSystemCapabilities_Resp == 0)
	{
		bool bNewLocalGwReq = false;
		if (!m_pLogicalDevice->LocalGwReq_IsPending(CMDID_C074_ReadIOSystemCapabilities))
		{
			CHartCmdWrapper::Ptr pC074(new CHartCmdWrapper);

			pC074->LoadParsed(CMDID_C074_ReadIOSystemCapabilities, 0, 0);

			m_pLogicalDevice->LocalGwReq_Prepare(pC074);
			bNewLocalGwReq = true;

			LOG_INFO_APP("C074_ReadIOSystemCapabilities_Req for device " << m_pLogicalDevice->m_oDeviceUniqueID);
		}
		return bNewLocalGwReq;
	}
	//else we have 74 response

	if (m_bReadDevicesIdentityInProgress)
	{
		if ( !m_pLogicalDevice->LocalGwReq_IsPending(CMDID_C074_ReadIOSystemCapabilities)
		            && !m_pLogicalDevice->LocalGwReq_IsPending(CMDID_C084_ReadSubDeviceIdentitySummary) )
		{
		    return CheckNeedAdd_C084_ReadSubDeviceIdentitySummary();
		}
		return false;
	}
	//else identity discovery is finished

	if (m_bDiscoveryInProgress)
	{
		if ( !m_pLogicalDevice->LocalGwReq_IsPending(CMDID_C074_ReadIOSystemCapabilities)
		            && !m_pLogicalDevice->LocalGwReq_IsPending(CMDID_C075_PollSubDevice) )
		{
		    return CheckNeedAdd_C075_PollSubDevice();
		}
		return false;
	}
	//else discovery is finished



	return false;
}


void CLogicalDeviceInfoAdapter::RestartDiscoveryFlow()
{
	LOG_INFO_APP("CLogicalDeviceInfoAdapter::RestartDiscoveryFlow");

	m_pC074_ReadIOSystemCapabilities_Resp = boost::shared_ptr<C074_ReadIOSystemCapabilities_Resp>();
}

CLogicalDevice * CLogicalDeviceInfoAdapter::MakeInactiveSubDevice(WHartUniqueID & p_rUniqueId)
{
	CLogicalDevice::Ptr pDev = m_pLogicalDevice->m_pDevicesTable->GetLogicalDevice(p_rUniqueId);
	if (!pDev)
	{
	    LOG_ERROR_APP("CLogicalDeviceInfoAdapter::MakeInactiveSubDevice: device=" << p_rUniqueId << " does not exist in table");
		return 0;
	}

	if (!pDev->IsActive() || !pDev->m_pSubDeviceInfo)
	{	return 0;
	}

	pDev->SetActive(false);

	return pDev.get();
}

CLogicalDevice * CLogicalDeviceInfoAdapter::MakeInactiveSubDevice(SubdevKey & p_rPollSubDevice)
{
	CSubDeviceKeyMap::iterator itMap = m_oSubDeviceKeyMap.find(p_rPollSubDevice);

	if (itMap == m_oSubDeviceKeyMap.end())
	{
	    return 0;
	}

	return MakeInactiveSubDevice(itMap->second);
}

WHartUniqueID CLogicalDeviceInfoAdapter::MakeCorrectedUniqueID(uint16_t deviceCode, uint32_t deviceID)
{
	WHartUniqueID oAddressField =  MakeUniqueID(deviceCode, deviceID);
	//first bit is master address : secondary master or primary master
	oAddressField.bytes[0] |= ((m_pC074_ReadIOSystemCapabilities_Resp->masterMode != 0) ? (1<<7) : 0);
	//second bit is burst mode - change to 0
	oAddressField.bytes[0] &= (~(1<<6));

	return oAddressField;
}

void CLogicalDeviceInfoAdapter::ClearDiscoveryFlowState()
{
	if (m_pLogicalDevice != 0)
	{
		std::set<int> cmdsToDelete;
		cmdsToDelete.insert(CMDID_C075_PollSubDevice);
		cmdsToDelete.insert(CMDID_C084_ReadSubDeviceIdentitySummary);
		m_pLogicalDevice->LocalGwReq_ClearPending(cmdsToDelete);
	}


	m_bDiscoveryInProgress = true;
	m_bReadDevicesIdentityInProgress = true;

	m_u16C084CurrentSubdevIndex = 0;

	m_u16C084CurrentTransId = 0;

	ResetC075_PollInfo();

	m_oSubDevConnectionsNbMap.clear();
}

void CLogicalDeviceInfoAdapter::CorrectSubdevMaps()
{
	if (!m_pC074_ReadIOSystemCapabilities_Resp)
	{
	    return;
	}

	if (m_oSubDeviceIndexMap.size() >= m_pC074_ReadIOSystemCapabilities_Resp->noOfDevicesDetected)
	{
		CSubDeviceIndexMap::iterator start = m_oSubDeviceIndexMap.find(m_pC074_ReadIOSystemCapabilities_Resp->noOfDevicesDetected - 1);

		++start;

		//iterate over subdevs whose index is greater then the last valid index
		for (CSubDeviceIndexMap::iterator it = start ; it != m_oSubDeviceIndexMap.end() ; ++it)
		{	if (it->second == 0)
			{
		    continue;
			}

			WHartUniqueID uid = MakeCorrectedUniqueID(it->second->expandedDeviceType, it->second->deviceID);
			CLogicalDevice* pDev = MakeInactiveSubDevice(uid);

			if (pDev == 0 || pDev->m_pSubDeviceInfo == 0)
			{
			    continue;
			}

			CSubDeviceKeyMap::iterator keysIt = m_oSubDeviceKeyMap.find(pDev->m_pSubDeviceInfo->m_oSubDeviceKey);
			if (keysIt == m_oSubDeviceKeyMap.end())
			{
			    continue;
			}

			m_oSubDeviceKeyMap.erase(keysIt);
		}

		m_oSubDeviceIndexMap.erase(start, m_oSubDeviceIndexMap.end());
	}
}



} // namespace gateway
} // namespace hart7
