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

#include <WHartGateway/LogicalDeviceTable.h>

#include <boost/foreach.hpp>
#include <WHartGateway/GwCmdsDescriptor.h>
#include <ApplicationLayer/Model/UniversalCommands.h>
#include <WHartGateway/GatewayConfig.h>
#include <WHartGateway/GwUtil.h>

namespace hart7 {
namespace gateway {

using namespace stack;

CLogicalDevice::Ptr CLogicalDeviceTable::GetLogicalDevice(const WHartAddress & p_oDeviceAddress, bool p_bOnlyIfActive/* = true*/)
{
	CLogicalDevice::Ptr pDev; // 0

	if (p_oDeviceAddress.type == WHartAddress::whartaNickname)
	{
		CNickNameMap::iterator itNick = m_oDeviceNickNameMap.find(p_oDeviceAddress.address.nickname);

		if ( itNick != m_oDeviceNickNameMap.end() )
		{
			if ( (p_bOnlyIfActive && itNick->second->IsActive()) || !p_bOnlyIfActive )
			{
			    pDev = itNick->second;
			}
		}
	}
	else if (p_oDeviceAddress.type == WHartAddress::whartaUniqueID)
	{
		CUniqueIDMap::iterator itUniqueId = m_oDeviceUniqueIDMap.find(p_oDeviceAddress.address.uniqueID);

		if ( itUniqueId != m_oDeviceUniqueIDMap.end() )
		{
			if ( (p_bOnlyIfActive && itUniqueId->second->IsActive()) || !p_bOnlyIfActive )
			{
			    pDev = itUniqueId->second;
			}
		}
	}
	else
	{
		LOG_ERROR_APP("CLogicalDeviceTable::GetLogicalDevice invalid type " << (int)p_oDeviceAddress.type);
	}

	return pDev;
}


const CLogicalDeviceTable::CUniqueIDMap& CLogicalDeviceTable::GetDeviceUniqueIDMap()
{
	return m_oDeviceUniqueIDMap;
}


WHartShortAddress CLogicalDeviceTable::GetDeviceNick(const WHartUniqueID & p_oUniqueId)
{
	CUniqueIDMap::iterator itUniqueId = m_oDeviceUniqueIDMap.find(p_oUniqueId);

	if ( itUniqueId == m_oDeviceUniqueIDMap.end() )
	{
		return g_cAddrNicknameInvalid;
	}

	return itUniqueId->second->m_u16DeviceNickName;
}


bool CLogicalDeviceTable::ResponseGet(const WHartAddress & p_oDeviceAddress, CHartCmdWrapper::Ptr & p_pReq, CHartCmdWrapper::Ptr & p_pRsp)
{
	CLogicalDevice::Ptr pDev = GetLogicalDevice(p_oDeviceAddress);

	if (pDev == NULL)
	{
		// TODO log
		return false;
	}

	uint8_t u8RspCode = RCS_N00_Success;
	return pDev->ResponseGet(p_pReq, p_pRsp, u8RspCode);
}

void CLogicalDeviceTable::ResponseAdd(const WHartAddress & p_oDeviceAddress, uint8_t p_u8DeviceStatus, uint8_t p_u8DeviceStatusExt,
									   const CHartCmdWrapperList & p_oResponses, CGwRequest * p_pGwReq )
{
	CLogicalDevice::Ptr pDev = GetLogicalDevice(p_oDeviceAddress, false);

	if (pDev == NULL)
	{
		LOG_INFO_APP("CLogicalDeviceTable::ResponseAdd   pDev = NULL  ");
		return;
	}

	pDev->ResponseAdd(p_u8DeviceStatus, p_u8DeviceStatusExt, p_oResponses, p_pGwReq);
}

CLogicalDevice::Ptr CLogicalDeviceTable::DeviceAdd(const WHartUniqueID & p_oDeviceUniqueID, WHartShortAddress p_u16DeviceNickName/* = g_cAddrNicknameInvalid*/, bool p_bIsSubDevice/* = false*/)
{
	CUniqueIDMap::iterator itUniqueId = m_oDeviceUniqueIDMap.find(p_oDeviceUniqueID);

	if (p_u16DeviceNickName == g_cAddrNicknameInvalid)
	{
		CLogicalDevice::Ptr pDev;

		if (itUniqueId == m_oDeviceUniqueIDMap.end())
		{
			pDev.reset(new CLogicalDevice(this,
					(p_bIsSubDevice) ? CLogicalDevice::DeviceType_SubDevice : CLogicalDevice::DeviceType_WHart,
					p_oDeviceUniqueID, p_u16DeviceNickName));

			LOG_INFO_APP("DeviceAdd: uniqueID=" << pDev->m_oDeviceUniqueID );
			m_oDeviceUniqueIDMap.insert(std::make_pair(p_oDeviceUniqueID,pDev));
		}
		else
		{
		    pDev = CLogicalDevice::Ptr(itUniqueId->second);
			LOG_DEBUG_APP("CLogicalDeviceTable::DeviceAdd device=" << p_oDeviceUniqueID << " already added");
		}

		if (p_bIsSubDevice)
		{
			DeviceNotifyNew(pDev);
			if (!pDev->m_pSubDeviceInfo)
			{
			    pDev->m_pSubDeviceInfo.reset(new CLogicalDeviceInfoSubDevice(pDev.get()));
			}
			pDev->m_bDeviceActive = true;
			pDev->m_nDeviceType = CLogicalDevice::DeviceType_SubDevice;
		}

		return pDev;
	}

	CNickNameMap::iterator itNick = m_oDeviceNickNameMap.find(p_u16DeviceNickName);

	if (itUniqueId != m_oDeviceUniqueIDMap.end() && itNick != m_oDeviceNickNameMap.end())
	{
		if (itUniqueId->second.get() == itNick->second.get())
		{
			DeviceNotifyNew(itUniqueId->second);
			//log already exist -> success
			return itUniqueId->second;
		}
		//log ERROR -> try to recuperate by deleting the device
		LOG_ERROR_APP("CLogicalDeviceTable::DeviceAdd: nick and unique id exist and point to different devices -> erase");

		m_oDeviceUniqueIDMap.erase(itUniqueId);
		m_oDeviceNickNameMap.erase(itNick);

		itUniqueId = m_oDeviceUniqueIDMap.end();
		itNick = m_oDeviceNickNameMap.end();
	}

	if (itUniqueId == m_oDeviceUniqueIDMap.end() && itNick == m_oDeviceNickNameMap.end())
	{
		CLogicalDevice::Ptr pDev(new CLogicalDevice(this, CLogicalDevice::DeviceType_WHart, p_oDeviceUniqueID, p_u16DeviceNickName));

		LOG_INFO_APP("DeviceAdd: uniqueID=" << WHartAddress(pDev->m_oDeviceUniqueID) << " nick=" << std::hex << pDev->m_u16DeviceNickName);

		m_oDeviceUniqueIDMap.insert(std::make_pair(p_oDeviceUniqueID, pDev));
		m_oDeviceNickNameMap.insert(std::make_pair(p_u16DeviceNickName, pDev));

		DeviceNotifyNew(pDev);
		return pDev;
	}

	if (itNick == m_oDeviceNickNameMap.end())
	{
		LOG_INFO_APP("DeviceAdd: setting nickname=" << (int)p_u16DeviceNickName << " for existent device uniqueId " << p_oDeviceUniqueID);

		itUniqueId->second->m_u16DeviceNickName = p_u16DeviceNickName;
		m_oDeviceNickNameMap.insert(std::make_pair(p_u16DeviceNickName, itUniqueId->second));
		DeviceNotifyNew(itUniqueId->second);

		return itUniqueId->second;
	}

	LOG_ERROR_APP("DeviceAdd: device nick=" << (int)p_u16DeviceNickName <<" exists but uniqueId " << p_oDeviceUniqueID << " NO");

	itNick->second->m_oDeviceUniqueID = p_oDeviceUniqueID;
	m_oDeviceUniqueIDMap.insert(std::make_pair(p_oDeviceUniqueID, itNick->second));
	DeviceNotifyNew(itNick->second);

	return itNick->second;
}

void CLogicalDeviceTable::DeviceRemove(const WHartAddress & p_oDeviceAddress)
{
	CUniqueIDMap::iterator itUniqueId = m_oDeviceUniqueIDMap.end();
	CNickNameMap::iterator itNick = m_oDeviceNickNameMap.end();

	if (p_oDeviceAddress.type == WHartAddress::whartaUniqueID)
	{
		itUniqueId = m_oDeviceUniqueIDMap.find(p_oDeviceAddress.address.uniqueID);

		if (itUniqueId == m_oDeviceUniqueIDMap.end())
		{
			//log
			return;
		}

		CLogicalDevice::Ptr pDev = itUniqueId->second;

		if (pDev->m_u16DeviceNickName != g_cAddrNicknameInvalid)
		{
			itNick = m_oDeviceNickNameMap.find(pDev->m_u16DeviceNickName);
		}
	}

	if (p_oDeviceAddress.type == WHartAddress::whartaNickname)
	{
		itNick = m_oDeviceNickNameMap.find(p_oDeviceAddress.address.nickname);

		if (itNick == m_oDeviceNickNameMap.end())
		{
			//log
			return;
		}

		CLogicalDevice::Ptr pDev = itNick->second;

		itUniqueId = m_oDeviceUniqueIDMap.find(itNick->second->m_oDeviceUniqueID);
	}

	if (itUniqueId != m_oDeviceUniqueIDMap.end())
	{
		m_oDeviceUniqueIDMap.erase(itUniqueId);
	}

	if (itNick != m_oDeviceNickNameMap.end())
	{
		m_oDeviceNickNameMap.erase(itNick);
	}
}

int CLogicalDeviceTable::Fill_C783_ReadSuperframeList_Resp(C783_ReadSuperframeList_Req * p_pReq, C783_ReadSuperframeList_Resp * p_pResp)
{
	memset(p_pResp, 0, sizeof(C783_ReadSuperframeList_Resp));

	p_pResp->m_ucSuperframeIndex = p_pReq->m_ucSuperframeIndex;
	p_pResp->m_ucNoOfEntriesRead = 0;
	p_pResp->m_ucNoOfActiveSuperframes = 0;

	return RCS_N00_Success;
}

int CLogicalDeviceTable::Fill_C784_ReadLinkList_Resp(C784_ReadLinkList_Req * p_pReq, C784_ReadLinkList_Resp * p_pResp)
{
	memset(p_pResp, 0, sizeof(C784_ReadLinkList_Resp));

	p_pResp->m_unLinkIndex = p_pReq->m_unLinkIndex;
	p_pResp->m_unNoOfActiveLinks = 0;
	p_pResp->m_ucNoOfLinksRead = 0;

	return RCS_N00_Success;
}

int CLogicalDeviceTable::Fill_C800_ReadServiceList_Resp(C800_ReadServiceList_Req * p_pReq,C800_ReadServiceList_Resp * p_pResp)
{
	memset(p_pResp, 0, sizeof(C800_ReadServiceList_Resp));

	CUniqueIDMap::iterator itUniqueId = m_oDeviceUniqueIDMap.begin();

	p_pResp->m_ucServiceIndex = p_pReq->m_ucServiceIndex;
	p_pResp->m_ucNoOfEntriesRead = 0;

	for(;itUniqueId != m_oDeviceUniqueIDMap.end(); ++itUniqueId)
	{
	    p_pResp->m_ucNoOfActiveServices += itUniqueId->second->m_pServiceTable->GetServicesMap().size();
	}

	int nIndex = 0;
	int result = 0;

	// correct index
	if(p_pReq->m_ucServiceIndex >= p_pResp->m_ucNoOfActiveServices)
	{
	    LOG_ERROR_APP("Incorrect service index");
	    return RCS_E06_DeviceSpecificCommandError;
	}

	// correct no of Entries to Read
	if ((p_pReq->m_ucServiceIndex + p_pReq->m_ucNoOfEntriesToRead) > p_pResp->m_ucNoOfActiveServices)
	{
	    LOG_ERROR_APP("Too large parameter : number of entries to read => set to nearest possible value");
	    p_pReq->m_ucNoOfEntriesToRead = p_pResp->m_ucNoOfActiveServices - p_pResp->m_ucServiceIndex;
	    result = RCM_W08_SetToNearestPossibleValue;
	}

	for(itUniqueId = m_oDeviceUniqueIDMap.begin();itUniqueId != m_oDeviceUniqueIDMap.end(); ++itUniqueId)
	{
		CServiceDeviceMap::const_iterator itServices = itUniqueId->second->m_pServiceTable->GetServicesMap().begin();

		for(; itServices !=itUniqueId->second->m_pServiceTable->GetServicesMap().end(); ++itServices)
		{
			if((nIndex >= p_pReq->m_ucServiceIndex) && ((p_pResp->m_ucNoOfEntriesRead) < (p_pReq->m_ucNoOfEntriesToRead)))
			{
				p_pResp->m_aServices[p_pResp->m_ucNoOfEntriesRead].serviceId = itServices->first;
				p_pResp->m_aServices[p_pResp->m_ucNoOfEntriesRead].serviceRequestFlags = itServices->second->m_stServiceEntry.m_ucServiceRequestFlags;
				p_pResp->m_aServices[p_pResp->m_ucNoOfEntriesRead].serviceApplicationDomain = itServices->second->m_stServiceEntry.m_ucServiceApplicationDomain;
				p_pResp->m_aServices[p_pResp->m_ucNoOfEntriesRead].nicknameOfPeer = itServices->second->m_stServiceEntry.m_unNicknameOfPeer;
				p_pResp->m_aServices[p_pResp->m_ucNoOfEntriesRead].period = itServices->second->m_stServiceEntry.m_tPeriod;
				p_pResp->m_aServices[p_pResp->m_ucNoOfEntriesRead].routeId = itServices->second->m_nRouteId;
				p_pResp->m_ucNoOfEntriesRead++;
				if (p_pResp->m_ucNoOfEntriesRead >= C800_MAX_SERVICES_LIST)
				{
				    return result;
				}
			}
			else
			{
				nIndex++;
			}
		}
	}

	return result;
}


int CLogicalDeviceTable::Fill_C814_ReadDeviceListEntries_Resp(C814_ReadDeviceListEntries_Req * p_pReq, C814_ReadDeviceListEntries_Resp * p_pResp, bool p_bJoined)
{
	p_pResp->m_ucDeviceListCode = p_pReq->m_ucDeviceListCode;
	p_pResp->m_ucNoOfListEntriesRead = 0;
	p_pResp->m_unStartingListIndex = p_pReq->m_unStartingListIndex;

	p_pResp->m_unTotalNoOfEntriesInList = 0;

	if (!p_bJoined)
	{
		return RCS_N00_Success;
	}

	int nIndex = 0;
	int res = RCM_W08_SetToNearestPossibleValue;

	for (CUniqueIDMap::iterator itUniqueId = m_oDeviceUniqueIDMap.begin(); itUniqueId != m_oDeviceUniqueIDMap.end(); ++itUniqueId, ++nIndex)
	{
		if (!itUniqueId->second->IsActive())
		{
		    continue;
		}

		if (itUniqueId->second->m_nDeviceType == CLogicalDevice::DeviceType_SubDevice )
		{
		    continue;
		}

		if (nIndex >= p_pReq->m_unStartingListIndex)
		{
			memcpy(p_pResp->m_aDeviceUniqueIds [ p_pResp->m_ucNoOfListEntriesRead++], itUniqueId->first.bytes, sizeof(itUniqueId->first.bytes));

			if( p_pResp->m_ucNoOfListEntriesRead == p_pReq->m_ucNoOfListEntriesToRead)
			{
				res = RCS_N00_Success;
				break;
			}
			if (p_pResp->m_ucNoOfListEntriesRead == MAX_DEVICE_LIST_ENTRIES_NO)
			{
				res = RCS_N00_Success;
				break;
			}
		}
	}

   for (CUniqueIDMap::iterator itUniqueId = m_oDeviceUniqueIDMap.begin(); itUniqueId != m_oDeviceUniqueIDMap.end(); ++itUniqueId, ++nIndex)
   {
        if (!itUniqueId->second->IsActive())
        {
            continue;
        }

        if (itUniqueId->second->m_nDeviceType == CLogicalDevice::DeviceType_SubDevice )
        {
            continue;
        }
        p_pResp->m_unTotalNoOfEntriesInList++;
   }

	return res;
}

int CLogicalDeviceTable::Fill_C075_PollSubDevice(C075_PollSubDevice_Req * p_pReq, C075_PollSubDevice_Resp * p_pResp)
{
	/*TODO - replace the polling addresses with the real polling addresses
	 *		 for now, the polling address is computed according to the position in the devices uniqueId map
			 gw always has the polling address 0
	*/

	memset(p_pResp, 0, sizeof(C075_PollSubDevice_Resp));

	if (p_pReq->ioCard != 0 || p_pReq->channel != 0)
	{
	    return RCM_E09_NoSubDeviceFound;
	}

	if (p_pReq->poolingAddress == 0)
	{
	    FillGwIdentityResponse(p_pResp, *GetGatewayConfig());
		return RCS_N00_Success;
	}

	//else request for other subdevs

	int devPollAddress = p_pReq->poolingAddress - 1; /*0 is gw, so the index will be smaller*/
	int currentIndex = 0;

	CLogicalDevice::Ptr pDev;

	CUniqueIDMap::iterator itUniqueId = m_oDeviceUniqueIDMap.begin();
	//advance iterator to the required index (we only count active devices that have the 0 response)
	for ( ;(itUniqueId != m_oDeviceUniqueIDMap.end()); ++itUniqueId)
	{
		pDev = itUniqueId->second;

		if (pDev == 0)
		{
		    continue;
		}

		if ( pDev->m_u16DeviceNickName == Gateway_Nickname() ||
			 pDev->m_u16DeviceNickName == NetworkManager_Nickname() ||
			 pDev->m_nDeviceType == CLogicalDevice::DeviceType_AccessPoint )
		{
		    continue;
		}

		if ( !pDev->IsActive() ||
			 !pDev->m_pC000_ReadUniqueIdentifier_Resp ||
			 !pDev->m_pC020_ReadLongTag_Resp )
		{
		    continue;
		}

		if (currentIndex == devPollAddress)
		{
		    break;
		}

		currentIndex++;
	}

	if (itUniqueId == m_oDeviceUniqueIDMap.end())
	{
	    return RCM_E09_NoSubDeviceFound;
	}

	memcpy(p_pResp, pDev->m_pC000_ReadUniqueIdentifier_Resp.get(), sizeof(C075_PollSubDevice_Resp));

	return RCS_N00_Success;
}

int CLogicalDeviceTable::Fill_C084_ReadSubDeviceIdentitySummary(C084_ReadSubDeviceIdentitySummary_Req * p_pReq, C084_ReadSubDeviceIdentitySummary_Resp * p_pResp)
{
	memset(p_pResp, 0, sizeof(C084_ReadSubDeviceIdentitySummary_Resp));

	if (p_pReq->subDeviceIndex == 0) //request for I/O System Identity
	{
		p_pResp->ioCard = 251; //none
		p_pResp->channel = 251; //none
		memcpy (&(p_pResp->manufacturerID), GetGatewayConfig()->m_u8ManufactCode, sizeof(p_pResp->manufacturerID));

		p_pResp->expandedDeviceType = ExpandedDeviceTypeCode_WirelessHART_Gateway;

		p_pResp->deviceID = Gateway_DeviceID24();
		p_pResp->revisionLevel = 7; //hart protocol major revision
		memcpy(p_pResp->tag, GetGatewayConfig()->m_szLongTag, sizeof(GetGatewayConfig()->m_szLongTag));

		return RCS_N00_Success;
	}
	//else //request for subdevice

	int devIndex = p_pReq->subDeviceIndex - 1 ;
	int currentIndex = 0;

	CUniqueIDMap::iterator itUniqueId = m_oDeviceUniqueIDMap.begin();

	//advance iterator to the required index (we only count active devices that have the 0 and 20 responses)
	for ( ;(itUniqueId != m_oDeviceUniqueIDMap.end()); ++itUniqueId)
	{
	    //LOG_DEBUG_APP("Fill_C084_ReadSubDeviceIdentitySummary - Dev: UniqueID=" << itUniqueId->second->m_oDeviceUniqueID << ", Nick=" << std::hex << (int)itUniqueId->second->m_u16DeviceNickName
	    //              << ", currentIndex=" << currentIndex << ", DeviceIndex=" << devIndex <<
	    //              ", longTag=" << (itUniqueId->second->m_pC020_ReadLongTag_Resp ? itUniqueId->second->m_pC020_ReadLongTag_Resp->longTag : " m_pC020_ReadLongTag_Resp NULL"));

		CLogicalDevice::Ptr pDev = itUniqueId->second;
		if (pDev->m_u16DeviceNickName == Gateway_Nickname() || pDev->m_u16DeviceNickName == NetworkManager_Nickname() || pDev->m_nDeviceType == CLogicalDevice::DeviceType_AccessPoint)
		{
			continue;
		}

		if (!pDev->IsActive() || !pDev->m_pC000_ReadUniqueIdentifier_Resp || !pDev->m_pC020_ReadLongTag_Resp )
		{
			continue;
		}

		if (currentIndex == devIndex)
		{
		    break;
		}
		currentIndex++;
	}

	if (itUniqueId == m_oDeviceUniqueIDMap.end())
	{
	    return RCS_E02_InvalidSelection;
	}

	p_pResp->subDeviceIndex = p_pReq->subDeviceIndex;
	p_pResp->ioCard = 0;
	p_pResp->channel = 0;
	p_pResp->manufacturerID = itUniqueId->second->m_pC000_ReadUniqueIdentifier_Resp->manufacturerIDCode;
	p_pResp->expandedDeviceType = (ExpandedDeviceTypeCodes)(itUniqueId->second->m_pC000_ReadUniqueIdentifier_Resp->expandedDeviceType);
	p_pResp->deviceID = itUniqueId->second->m_pC000_ReadUniqueIdentifier_Resp->deviceID;
	p_pResp->revisionLevel =itUniqueId->second->m_pC000_ReadUniqueIdentifier_Resp->protocolMajorRevNo;
	memcpy(p_pResp->tag, itUniqueId->second->m_pC020_ReadLongTag_Resp->longTag, sizeof(p_pResp->tag));

	return RCS_N00_Success;
}

int CLogicalDeviceTable::Fill_C085_ReadIOChannelStatistics(C085_ReadIOChannelStatistics_Req * p_pReq, C085_ReadIOChannelStatistics_Resp * p_pResp)
{
	memset(p_pResp, 0, sizeof(C085_ReadIOChannelStatistics_Resp));

	if (p_pReq->ioCard != 0)
	{
	    return RCS_E21_InIOCardNumber;
	}

	if (p_pReq->channel != 0)
	{
	    return RCS_E22_InChannelNumber;
	}

	p_pResp->ioCard = p_pReq->ioCard;
	p_pResp->channel = p_pReq->channel;

	CUniqueIDMap::iterator itUniqueId = m_oDeviceUniqueIDMap.begin();
	for ( ;(itUniqueId != m_oDeviceUniqueIDMap.end()); ++itUniqueId)
	{
		CLogicalDevice::Ptr pDev = itUniqueId->second;

		if ( pDev->m_u16DeviceNickName == NetworkManager_Nickname() ||
			 pDev->m_nDeviceType == CLogicalDevice::DeviceType_AccessPoint )
		{
		    continue;
		}

		p_pResp->stxMessagesSentCount 	   +=	pDev->GetStxMessagesSentToDev();
		p_pResp->ackMessagesReceivedCount  +=	pDev->GetAckMessagesReceivedFromDev();
		p_pResp->backMessagesReceivedCount +=	pDev->GetBackMessagesReceivedFromDev();
	}

	return RCS_N00_Success;
}

int CLogicalDeviceTable::Fill_C086_ReadSubDeviceStatistics(C086_ReadSubDeviceStatistics_Req * p_pReq, C086_ReadSubDeviceStatistics_Resp * p_pResp)
{
	memset(p_pResp, 0, sizeof(C086_ReadSubDeviceStatistics_Resp));

	if (p_pReq->subDeviceIndex == 0) //request for I/O System statistics
	{
		CLogicalDevice::Ptr pGw = GetLogicalDeviceGW();
		if (pGw != 0)
		{
		    p_pResp->stxMessagesSentCount 		= pGw->GetStxMessagesSentToDev();
			p_pResp->ackMessagesReceivedCount 	= pGw->GetAckMessagesReceivedFromDev();
			p_pResp->backMessagesReceivedCount 	= pGw->GetBackMessagesReceivedFromDev();
		}

		return RCS_N00_Success;
	}
	//else //request for subdevice statistics

	int devIndex = p_pReq->subDeviceIndex - 1 ;
	int currentIndex = 0;

	CUniqueIDMap::iterator itUniqueId = m_oDeviceUniqueIDMap.begin();
	CLogicalDevice::Ptr pDev;
	//advance iterator to the required index
	for ( ;(itUniqueId != m_oDeviceUniqueIDMap.end()); ++itUniqueId)
	{
		pDev = itUniqueId->second;
		if (pDev == 0 || pDev->m_u16DeviceNickName == Gateway_Nickname() || pDev->m_u16DeviceNickName == NetworkManager_Nickname() || pDev->m_nDeviceType == CLogicalDevice::DeviceType_AccessPoint)
		{
		    continue;
		}

		if (!pDev->IsActive() || !pDev->m_pC000_ReadUniqueIdentifier_Resp || !pDev->m_pC020_ReadLongTag_Resp )
		{
		    continue;
		}

		if (currentIndex == devIndex)
		{
		    break;
		}
		currentIndex++;
	}

	if (itUniqueId == m_oDeviceUniqueIDMap.end())
	{
	    return RCS_E02_InvalidSelection;
	}

	p_pResp->subDeviceIndex 			= p_pReq->subDeviceIndex;
	p_pResp->stxMessagesSentCount 		= pDev->GetStxMessagesSentToDev();
	p_pResp->ackMessagesReceivedCount 	= pDev->GetAckMessagesReceivedFromDev();
	p_pResp->backMessagesReceivedCount 	= pDev->GetBackMessagesReceivedFromDev();

	return RCS_N00_Success;
}

void CLogicalDeviceTable::ServiceReqResponse(const C799_RequestService_Req * p_pReq, const C799_RequestService_Resp * p_pResp, int p_nResponseCode)
{
	CNickNameMap::iterator itNick = m_oDeviceNickNameMap.find(p_pReq->m_unNicknameOfPeer);

	if (itNick == m_oDeviceNickNameMap.end())
	{
		LOG_ERROR_APP("ServiceReqResponse: cannot find device with nick="<<std::hex<<(int)p_pReq->m_unNicknameOfPeer);
		return;
	}
	itNick->second->ServiceReqResponse(p_pReq, p_pResp, p_nResponseCode);
}

uint8_t	CLogicalDeviceTable::ServiceWrite(const C973_WriteService_Req * p_pReq, const C973_WriteService_Resp * p_pResp, int p_nResponseCode)
{
	CNickNameMap::iterator itNick = m_oDeviceNickNameMap.find(p_pReq->m_unPeerNickname);
	if (itNick == m_oDeviceNickNameMap.end())
	{
		LOG_ERROR_APP("ServiceWrite: cannot find device with nick="<<std::hex<<(int)p_pReq->m_unPeerNickname);
		return RCM_E69_UnknownCorrespNickname;
	}

	return itNick->second->ServiceWrite (p_pReq, p_pResp, p_nResponseCode);

}

uint8_t	CLogicalDeviceTable::ServiceDelete(const C801_DeleteService_Req * p_pReq, const C801_DeleteService_Resp * p_pResp, int p_nResponseCode)
{
	CNickNameMap::iterator itNick = m_oDeviceNickNameMap.find(p_pReq->m_peerNickname);
	if (itNick == m_oDeviceNickNameMap.end())
	{
		LOG_ERROR_APP("ServiceDelete: cannot find device with nick="<<std::hex<<(int)p_pReq->m_peerNickname << " reason=" << std::dec << (int)p_pReq->m_ucReason);
		return RCM_E69_UnknownCorrespNickname;
	}

	return itNick->second->ServiceDelete(p_pReq, p_pResp, p_nResponseCode);
}

bool CLogicalDeviceTable::DeviceRefresh()
{
	bool bNewLocalGwReq = false;

	CUniqueIDMap::iterator itDev = m_oDeviceUniqueIDMap.begin();

	for (;itDev != m_oDeviceUniqueIDMap.end(); itDev++)
	{
		bNewLocalGwReq |= itDev->second->Refresh();
	}

	return bNewLocalGwReq;
}

void CLogicalDeviceTable::DeviceNotifyNew(CLogicalDevice::Ptr & p_pDevice)
{
	if (p_pDevice->IsActive())
	{
		return;
	}
	p_pDevice->SetActive(true);

	RaiseNotifListChanged();
}

void CLogicalDeviceTable::RaiseNotifListChanged()
{
    LOG_DEBUG_APP("RaiseNotifListChanged");

	CLogicalDevice::Ptr pGw = GetLogicalDeviceGW();
	if (!pGw)
	{
		LOG_ERROR_APP("Programmer ERROR -- no gw in device list");
		return;
	}
	pGw->m_u16NotificationMask |= NotificationMaskCodesMask_NetworkTopology;
	pGw->m_oNotifNetworkTopology.insert(CMDID_C814_ReadDeviceListEntries);
}

void CLogicalDeviceTable::RaiseNotifConfigCounterChanged()
{
    LOG_DEBUG_APP("RaiseNotifConfigCounterChanged");

    CLogicalDevice::Ptr pGw = GetLogicalDeviceGW();
    if (!pGw)
    {
        LOG_ERROR_APP("Programmer ERROR -- no gw in device list");
        return;
    }

    pGw->m_u16NotificationMask |= NotificationMaskCodesMask_DeviceConfiguration;
}

void CLogicalDeviceTable::SetInactive()
{
	CUniqueIDMap::iterator itDev = m_oDeviceUniqueIDMap.begin();

	for (;itDev != m_oDeviceUniqueIDMap.end(); itDev++)
	{
		if (itDev->second->m_oDeviceUniqueID == Gateway_UniqueID())
		{
			continue;
		}

		itDev->second->SetActive(false);
	}
}


void CLogicalDeviceTable::SetDeviceNickname(CLogicalDevice & p_rDev, WHartShortAddress p_nNickname, WHartUniqueID & p_rUniqueId)
{
	CUniqueIDMap::iterator itUniqueId = m_oDeviceUniqueIDMap.find(p_rUniqueId);

	if ( (memcmp(p_rDev.m_oDeviceUniqueID.bytes, p_rUniqueId.bytes, sizeof (p_rUniqueId.bytes)) != 0) || (itUniqueId == m_oDeviceUniqueIDMap.end()) )
	{
	    LOG_DEBUG_APP("CLogicalDeviceTable::SetDeviceNickname - invalid uniqueId in set device address");
		return;
	}

	if (p_rDev.m_u16DeviceNickName != g_cAddrNicknameInvalid)
	{
	    LOG_DEBUG_APP("CLogicalDeviceTable::SetDeviceNickname - Nickname already set for device");
		return;
	}

	CNickNameMap::iterator itNick = m_oDeviceNickNameMap.find(p_nNickname);

	if ( itNick != m_oDeviceNickNameMap.end() )
	{
	    LOG_DEBUG_APP("CLogicalDeviceTable::SetDeviceNickname - Device with specified nickname already exists");
		if (itUniqueId->second != itNick->second)
		{
		    LOG_DEBUG_APP("CLogicalDeviceTable::SetDeviceNickname - Inconsistent nickname-uniqueId mapping");
		}
		return;
	}

	p_rDev.m_u16DeviceNickName = p_nNickname;
	m_oDeviceNickNameMap[p_nNickname] = CLogicalDevice::Ptr(itUniqueId->second);
}

std::ostream & operator<<(std::ostream & stream, const CLogicalDeviceTable & logicalDeviceTable)
{
    stream << "UniqueIDMap: {";
    for(CLogicalDeviceTable::CUniqueIDMap::const_iterator itUniqueID = logicalDeviceTable.m_oDeviceUniqueIDMap.begin();
        itUniqueID != logicalDeviceTable.m_oDeviceUniqueIDMap.end(); ++itUniqueID)
    {
        stream << "{"<< itUniqueID->first << ", " ;
        if (itUniqueID->second)
        {
            stream << itUniqueID->second->m_oDeviceUniqueID << "}, ";
        }
        else
        {
            stream << "NULL}, ";
        }
    }
    stream << "}" << std::endl;

    stream << "NickNameMap: {";
    for(CLogicalDeviceTable::CNickNameMap::const_iterator itNickName = logicalDeviceTable.m_oDeviceNickNameMap.begin();
        itNickName != logicalDeviceTable.m_oDeviceNickNameMap.end(); ++itNickName)
    {
        stream << "{"<< itNickName->first << ", ";
        if (itNickName->second)
        {
            stream << itNickName->second->m_oDeviceUniqueID << "}, ";
        }
        else
        {
            stream << "NULL}, ";
        }
    }
    stream << "}";

    return stream;
}



} // namespace gateway
} // namespace hart7
