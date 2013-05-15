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

//
// C++ Implementation: CmdProcessorWirelessHART
//
// Created on Nov 2, 2009
//
// Author: Catrina Mihailescu <catrina@r2d2>, (C) 2009
//
//
//
#include <WHartGateway/CmdProcessorWirelessHART.h>

#include <WHartStack/util/WHartCmdWrapper.h>
#include <WHartStack/WHartSubApplicationData.h>

#include <WHartGateway/AllGatewayCommands.h>
#include <WHartGateway/GwUtil.h>

#include <boost/foreach.hpp>
#include <boost/bind.hpp> //for binding callbacks
#include <Shared/UtilsSolo.h>

#include <WHartGateway/GatewayConfig.h>
#include <WHartGateway/GatewayStack.h>
#include <WHartStack/util/WHartCmdWrapper.h>
#include <WHartGateway/Gateway.h>
#include <WHartGateway/LogicalDevice.h>
#include <WHartGateway/HostAppSession.h>
#include <WHartGateway/GwRequest.h>


using namespace boost;

namespace hart7 {
namespace gateway{

CmdProcessorWirelessHART::CmdProcessorWirelessHART( Gateway & _gateway):
			CmdProcessor(_gateway)
{
}

bool CmdProcessorWirelessHART::ExecLocalCmdWirelessHART(CHartCmdWrapper::Ptr & p_pCmd, CHartCmdWrapper::Ptr & p_pRsp, CHostAppSession::Ptr & p_pHostSession)
{
	bool bRet =false;

	switch (p_pCmd->GetCmdId())
	{
	case CMDID_C778_ReadBatteryLife:
	{
		bRet = true;

		LOG_DEBUG_APP("ExecLocalCmd: cmdId=" << p_pCmd->GetCmdId());

		C778_ReadBatteryLife_Resp stResp;
		stResp.m_unBatteryLifeRemaining = 0xFFFF;

		p_pRsp.reset(new CHartCmdWrapper);
		p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(stResp), &stResp, RCS_N00_Success);
	}
	break;

	case CMDID_C783_ReadSuperframeList:
	{
		bRet = true;

		LOG_DEBUG_APP("ExecLocalCmd: cmdId=" << p_pCmd->GetCmdId());

		C783_ReadSuperframeList_Req* pReq = (C783_ReadSuperframeList_Req*) p_pCmd->GetParsedData();

		C783_ReadSuperframeList_Resp stResp;
		C783_ReadSuperframeList_Resp* pResp = &stResp;

		//fill the response
		int result = gateway.m_oDevices.Fill_C783_ReadSuperframeList_Resp(pReq, pResp);

		LOG_DEBUG_APP("ExecLocalCmd: cmdId=" << p_pCmd->GetCmdId() << "rsp rsp_code=" << result);

		p_pRsp.reset(new CHartCmdWrapper);
		p_pRsp->LoadParsed(p_pCmd->GetCmdId(),sizeof(stResp),&stResp,result);
		break;
	}

	case CMDID_C784_ReadLinkList:
	{
		bRet = true;

		LOG_DEBUG_APP("ExecLocalCmd: cmdId=" << p_pCmd->GetCmdId());

		C784_ReadLinkList_Req * pReq = (C784_ReadLinkList_Req*) p_pCmd->GetParsedData();

		C784_ReadLinkList_Resp stResp;
		C784_ReadLinkList_Resp * pResp = &stResp;

		// fill the response
		int result = gateway.m_oDevices.Fill_C784_ReadLinkList_Resp(pReq, pResp);

		LOG_DEBUG_APP("ExecLocalCmd: cmdId=" << p_pCmd->GetCmdId() << "rsp rsp_code=" << result);

		p_pRsp.reset(new CHartCmdWrapper);
		p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(stResp), &stResp, result);
		break;
	}

	case CMDID_C800_ReadServiceList:
	{
	    bRet = true;
		C800_ReadServiceList_Req* pReq = (C800_ReadServiceList_Req*) p_pCmd->GetParsedData();

		LOG_DEBUG_APP("ExecLocalCmd: cmdId="<< p_pCmd->GetCmdId() << " service index requested = " <<(int)pReq->m_ucServiceIndex << " no of entries to read = " << (int)pReq->m_ucNoOfEntriesToRead );

		C800_ReadServiceList_Resp stResp;
		C800_ReadServiceList_Resp * pResp = &stResp;

		// fill the response
		int result = gateway.m_oDevices.Fill_C800_ReadServiceList_Resp(pReq, pResp);

		LOG_DEBUG_APP("ExecLocalCmd: cmdId=" << p_pCmd->GetCmdId() << " rsp rsp_code=" << result);

		p_pRsp.reset(new CHartCmdWrapper);
		p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(stResp), pResp, result);
		break;
	}

	case CMDID_C802_ReadRouteList:
	{
	    bRet = true;
		LOG_DEBUG_APP("ExecLocalCmd: cmdId=" << p_pCmd->GetCmdId());

		C802_ReadRouteList_Req * pReq =  (C802_ReadRouteList_Req*) p_pCmd->GetParsedData();

		C802_ReadRouteList_Resp resp;
		int result = gateway.GetStack().GetLocalManagement()->ReadRouteList(pReq, &resp);

		p_pRsp.reset(new CHartCmdWrapper);
		p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(resp), &resp, result);
		break;
	}

	case CMDID_C803_ReadSourceRoute:
	{
	    bRet = true;
		LOG_DEBUG_APP("ExecLocalCmd: cmdId=" << p_pCmd->GetCmdId());

		C803_ReadSourceRoute_Req * pReq =  (C803_ReadSourceRoute_Req*) p_pCmd->GetParsedData();
		C803_ReadSourceRoute_Resp resp;

		int result = gateway.GetStack().GetLocalManagement()->ReadSourceRoute(pReq, &resp);
		p_pRsp.reset(new CHartCmdWrapper);
		p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(resp), &resp, result);
		break;
	}

	case CMDID_C814_ReadDeviceListEntries:
	{
		bRet = true;
		C814_ReadDeviceListEntries_Req * pReq =  (C814_ReadDeviceListEntries_Req*) p_pCmd->GetParsedData();

		LOG_DEBUG_APP("ExecLocalCmd: cmdId=" << p_pCmd->GetCmdId() << " req list_code=" << (int)pReq->m_ucDeviceListCode << " read_no=" << (int)pReq->m_ucNoOfListEntriesToRead << " index=" <<(int)pReq->m_unStartingListIndex);
		if (pReq->m_ucDeviceListCode != DeviceListCode_ActiveDeviceList)
		{
			LOG_DEBUG_APP("ExecLocalCmd: cmdId="<<p_pCmd->GetCmdId() << " req list_code=" << (int)pReq->m_ucDeviceListCode << " list invalid");
			p_pRsp.reset(new CHartCmdWrapper);
			p_pRsp->LoadParsed (p_pCmd->GetCmdId(), 0, 0, RCS_E02_InvalidSelection);
			break;
		}

		C814_ReadDeviceListEntries_Resp stResp;
		C814_ReadDeviceListEntries_Resp * pResp = &stResp;

		pResp->m_ucDeviceListCode = pReq->m_ucDeviceListCode;

		int result = gateway.m_oDevices.Fill_C814_ReadDeviceListEntries_Resp(pReq, pResp, gateway.stack.GetState() == GatewayStack::stOperational);

		int nLen = sizeof(C814_ReadDeviceListEntries_Resp);

		LOG_DEBUG_APP("ExecLocalCmd: cmdId=" << p_pCmd->GetCmdId() << " rsp rsp_code=" << result << " read_no=" << (int)pResp->m_ucNoOfListEntriesRead
									<<" index" << (int)pResp->m_unStartingListIndex << " total=" << (int)pResp->m_unTotalNoOfEntriesInList);

		p_pRsp.reset(new CHartCmdWrapper);
		p_pRsp->LoadParsed(p_pCmd->GetCmdId(), nLen, pResp, result);
		break;
	}

	case CMDID_C815_AddDeviceListTableEntry:
		{
			bRet = true;

			LOG_DEBUG_APP("ExecLocalCmd: cmdId=" << p_pCmd->GetCmdId() << " CMDID_C815_AddDeviceListTableEntry AccessRestricted") ;

			p_pRsp.reset(new CHartCmdWrapper);
			p_pRsp->LoadParsed (p_pCmd->GetCmdId(), 0, 0, RCS_E16_AccessRestricted);
			break;
		}

	case CMDID_C816_DeleteDeviceListTableEntry:
		{
			bRet = true;

			LOG_DEBUG_APP("ExecLocalCmd: cmdId=" << p_pCmd->GetCmdId() << " CMDID_C816_DeleteDeviceListTableEntry AccessRestricted") ;

			p_pRsp.reset(new CHartCmdWrapper);
			p_pRsp->LoadParsed (p_pCmd->GetCmdId(), 0, 0, RCS_E16_AccessRestricted);
			break;
		}
	case CMDID_C832_ReadNetworkDeviceIdentity:
	{
		LOG_ERROR_APP("ExecLocalCmdWirelessHART: C832_ReadNetworkDeviceIdentity -- should not be handled here");
		bRet = true;
		C832_ReadNetworkDeviceIdentity_Req * pReq =  (C832_ReadNetworkDeviceIdentity_Req*) p_pCmd->GetParsedData();

		CLogicalDevice::Ptr pDevice = gateway.m_oDevices.GetLogicalDevice(WHartAddress(pReq->DeviceUniqueID));

		if (!pDevice)
		{
			p_pRsp.reset(new CHartCmdWrapper);
			p_pRsp->LoadParsed(p_pCmd->GetCmdId(), 0, 0, RCS_E02_InvalidSelection);
			break;
		}

		C832_ReadNetworkDeviceIdentity_Resp stResp;

		memcpy(stResp.DeviceUniqueID, pDevice->m_oDeviceUniqueID.bytes, sizeof(stResp.DeviceUniqueID));
		stResp.Nickname = pDevice->m_u16DeviceNickName;

		if (stResp.Nickname == Gateway_Nickname())
		{
			memcpy(stResp.LongTag, gateway.config.m_szLongTag, sizeof(stResp.LongTag));
		}
		else if (stResp.Nickname == NetworkManager_Nickname())
		{
			memset(stResp.LongTag, 0, sizeof(stResp.LongTag));
		}
		else
		{
			memset(stResp.LongTag, 0, sizeof(stResp.LongTag));
			uint8_t * pLongTag = pDevice->GetLongTag();
			if (pLongTag)
			{
				memcpy(stResp.LongTag, pLongTag, sizeof(stResp.LongTag));
			}
		}

		//TODO long tag

		p_pRsp.reset(new CHartCmdWrapper);
		p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(stResp), &stResp, RCS_N00_Success);
		break;
	}

	case CMDID_C835_ReadBurstMessageList:
	{
		bRet = true;
		C835_ReadBurstMessageList_Req * pReq =  static_cast<C835_ReadBurstMessageList_Req*> (p_pCmd->GetParsedData());
		if (!pReq)
		{
			p_pRsp.reset(new CHartCmdWrapper);
			p_pRsp->LoadParsed(p_pCmd->GetCmdId(), 0, 0, RCS_E01_Undefined1);
			break;
		}

		CLogicalDevice::Ptr pDevice = gateway.m_oDevices.GetLogicalDevice(WHartAddress(pReq->UniqueID));

		if (!pDevice)
		{
			p_pRsp.reset(new CHartCmdWrapper);
			p_pRsp->LoadParsed(p_pCmd->GetCmdId(), 0, 0, RCS_E02_InvalidSelection);
			break;
		}

		C835_ReadBurstMessageList_Resp oResp835;
		memcpy( oResp835.UniqueID, pReq->UniqueID, sizeof(_device_address_t) );
		oResp835.DifferentBurstCommandsNo = 0;

		BOOST_FOREACH(const CBurstResponseMap::value_type & burstCmdResponsesPair, pDevice->m_oBurstResponses)
		{
			//each response is a response to the same command but with a different configuration: (cmd9; 1,2,3), (cmd9; 4,5,6), etc
			BOOST_FOREACH(const CCachedResponse::Ptr & burstResponse, *(burstCmdResponsesPair.second))
			{
				oResp835.BurstCommands[oResp835.DifferentBurstCommandsNo].CommandNoBeingBurst = burstCmdResponsesPair.first;
				oResp835.BurstCommands[oResp835.DifferentBurstCommandsNo++].ReceivedBurstPacketsNo = burstResponse->m_nRecvCounter;
			}
			if (oResp835.DifferentBurstCommandsNo >= MaxBurstCommands)
			{
				break;
			}
		}

		p_pRsp.reset(new CHartCmdWrapper);
		p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(C835_ReadBurstMessageList_Resp), &oResp835, RCS_N00_Success);
		break;
	}
	case CMDID_C836_FlushCacheResponses:
	{
		bRet = true;
		C836_FlushCacheResponses_Req * pReq =  static_cast<C836_FlushCacheResponses_Req*> (p_pCmd->GetParsedData());
		CLogicalDevice::Ptr pDevice = gateway.m_oDevices.GetLogicalDevice(WHartAddress(pReq->UniqueID));

		if (!pDevice)
		{
			p_pRsp.reset(new CHartCmdWrapper);
			p_pRsp->LoadParsed(p_pCmd->GetCmdId(), 0, 0, RCS_E02_InvalidSelection);
			return true;
		}

		pDevice->m_oCachedResponses.clear();
		pDevice->m_oBurstResponses.clear();

		p_pRsp.reset(new CHartCmdWrapper);
		p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(C836_FlushCacheResponses_Resp), pReq, RCS_N00_Success);
		break;
	}
	case CMDID_C837_WriteUpdateNotificationBitMask:
	{
		bRet = true;
		C837_WriteUpdateNotificationBitMask_Req * pParsedCmd = (C837_WriteUpdateNotificationBitMask_Req*)p_pCmd->GetParsedData();

		if (!p_pHostSession)
		{
			LOG_ERROR_APP("ExecLocalCmd: PROGRAMMER ERROR !p_pHostSession");
			return true;
		}

        LOG_INFO_APP("CMDID_C837_WriteUpdateNotificationBitMask device=" << WHartAddress(pParsedCmd->UniqueID) << " flags=" << std::hex << pParsedCmd->ChangeNotificationFlags);

        // test also for unknown unique ID
        CLogicalDevice::Ptr pDevice = gateway.m_oDevices.GetLogicalDevice(WHartAddress(pParsedCmd->UniqueID), false);
        if (!pDevice)
        {
            LOG_WARN_APP("CMDID_C837_WriteUpdateNotificationBitMask device=" << WHartAddress(pParsedCmd->UniqueID)
                         << " flags=" << std::hex << pParsedCmd->ChangeNotificationFlags << ", Unknown UniqueID.");
            p_pRsp.reset(new CHartCmdWrapper);
            p_pRsp->LoadParsed(p_pCmd->GetCmdId(), 0, 0, RCM_E65_UnknownUID);
            return true;
        }

		if ( (pParsedCmd->ChangeNotificationFlags & (NotificationMaskCodesMask_NetworkSchedule | NotificationMaskCodesMask_NetworkTopology))
			&& !( pParsedCmd->UniqueID == Gateway_UniqueID() || pParsedCmd->UniqueID == NetworkManager_UniqueID()) )
		{
			p_pRsp.reset(new CHartCmdWrapper);
			p_pRsp->LoadParsed(p_pCmd->GetCmdId(), 0, 0, RCS_E16_AccessRestricted);
			LOG_INFO_APP("CMDID_C837_WriteUpdateNotificationBitMask device=" << WHartAddress(pParsedCmd->UniqueID) << " flags="
			             << std::hex << pParsedCmd->ChangeNotificationFlags << " RCS_E16_AccessRestricted");
			return true;
		}

		if (pParsedCmd->ChangeNotificationFlags >= (NotificationMaskCodesMask_NetworkSchedule << 1) )
		{
			p_pRsp.reset(new CHartCmdWrapper);
			p_pRsp->LoadParsed(p_pCmd->GetCmdId(), 0, 0, RCM_E66_UnknownNotificationFlag);
			LOG_INFO_APP("CMDID_C837_WriteUpdateNotificationBitMask device=" << WHartAddress(pParsedCmd->UniqueID) << " flags=" << std::hex << pParsedCmd->ChangeNotificationFlags
				<< " RCM_E66_UnknownNotificationFlag");

			return true;
		}


		p_pHostSession->m_oDevNotifMaskMap[WHartAddress(pParsedCmd->UniqueID).address.uniqueID] = pParsedCmd->ChangeNotificationFlags;
		p_pHostSession->m_nMessageID = p_pCmd->GetFrameFormat();

		p_pRsp.reset(new CHartCmdWrapper);
		p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(C837_WriteUpdateNotificationBitMask_Resp), pParsedCmd, RCS_N00_Success);
		break;
	}

	case CMDID_C838_ReadUpdateNotificationBitMask:
		{
		    bRet = true;
			C838_ReadUpdateNotificationBitMask_Req * pParsedCmd = (C838_ReadUpdateNotificationBitMask_Req*) p_pCmd->GetParsedData();


			if (!p_pHostSession)
			{
				LOG_ERROR_APP("ExecLocalCmd: PROGRAMMER ERROR !p_pHostSession");
				return true;
			}

			CHostAppDevNotifMap::iterator itNotif = p_pHostSession->m_oDevNotifMaskMap.find(WHartAddress(pParsedCmd->UniqueID).address.uniqueID);

			if (itNotif != p_pHostSession->m_oDevNotifMaskMap.end())
			{
				C838_ReadUpdateNotificationBitMask_Resp stParsedRsp;
				memcpy(stParsedRsp.UniqueID, pParsedCmd->UniqueID, sizeof(pParsedCmd->UniqueID));
				stParsedRsp.ChangeNotificationFlags = itNotif->second;
				p_pRsp.reset(new CHartCmdWrapper);
				p_pRsp->LoadParsed (p_pCmd->GetCmdId(), sizeof(stParsedRsp), &stParsedRsp, RCS_N00_Success);
			}
			else
			{
				p_pRsp.reset(new CHartCmdWrapper);
				p_pRsp->LoadParsed(p_pCmd->GetCmdId(), 0, 0, RCM_E65_UnknownUID);
			}
		}
		break;

	case CMDID_C841_ReadNetworkDeviceIdentityByNickname:
		{
			LOG_ERROR_APP("ExecLocalCmdWirelessHART: C841_ReadNetworkDeviceIdentityByNickname -- should not be handled here");
			bRet = true;
			C841_ReadNetworkDeviceIdentityByNickname_Req* pReq =  (C841_ReadNetworkDeviceIdentityByNickname_Req*) p_pCmd->GetParsedData();

			CLogicalDevice::Ptr pDevice = gateway.m_oDevices.GetLogicalDevice(WHartAddress(pReq->DeviceNickname));


			if (!pDevice)
			{
				p_pRsp.reset(new CHartCmdWrapper);
				p_pRsp->LoadParsed(p_pCmd->GetCmdId(),0,0,RCS_E02_InvalidSelection);
				break;
			}

			C841_ReadNetworkDeviceIdentityByNickname_Resp stResp;

			memcpy(stResp.DeviceUniqueID, pDevice->m_oDeviceUniqueID.bytes, sizeof(stResp.DeviceUniqueID));
			stResp.Nickname = pDevice->m_u16DeviceNickName;

			if (stResp.Nickname == Gateway_Nickname())
			{
				memcpy(stResp.LongTag, gateway.config.m_szLongTag, sizeof(stResp.LongTag));
			}
			else if (stResp.Nickname == NetworkManager_Nickname())
			{
				memset(stResp.LongTag, 0, sizeof(stResp.LongTag));
			}
			else
			{
				memset(stResp.LongTag, 0, sizeof(stResp.LongTag));
				uint8_t* pLongTag = pDevice->GetLongTag();
				if (pLongTag)
				{
					memcpy(stResp.LongTag, pLongTag, sizeof(stResp.LongTag));
				}
			}

			//TODO long tag

			p_pRsp.reset(new CHartCmdWrapper);
			p_pRsp->LoadParsed(p_pCmd->GetCmdId(),sizeof(stResp),&stResp,RCS_N00_Success);
		}
		break;

	default:
		{
            LOG_WARN_APP("ExecLocalCmd: cmdId="<<p_pCmd->GetCmdId()<<" CommandNotImplemented ");
            p_pRsp.reset(new CHartCmdWrapper);
            p_pRsp->LoadParsed (p_pCmd->GetCmdId(), 0, 0, RCS_E64_CommandNotImplemented);
            bRet = true;
		}
		break;
	}

	return bRet;
}


// returns	true	- the command was processed, a response (even error response) is prepared and it will be sent
//		MUST return false only if the command should be fwd
bool CmdProcessorWirelessHART::ExecCmdNmOrGwDualCmds(CHartCmdWrapper::Ptr & p_pCmd, CHartCmdWrapper::Ptr & p_pRsp, CHostAppSession::Ptr & p_pHostSession)
{
	bool bRet = true;

	switch (p_pCmd->GetCmdId())
	{
        case CMDID_C832_ReadNetworkDeviceIdentity:
        {
            bRet = true;
            C832_ReadNetworkDeviceIdentity_Req * pReq =  (C832_ReadNetworkDeviceIdentity_Req*) p_pCmd->GetParsedData();

            CLogicalDevice::Ptr pDevice = gateway.m_oDevices.GetLogicalDevice(WHartAddress(pReq->DeviceUniqueID),false);

            if (!pDevice)
            {
                p_pRsp.reset(new CHartCmdWrapper);
                p_pRsp->LoadParsed(p_pCmd->GetCmdId(),0,0,RCS_E02_InvalidSelection);
                break;
            }

            if ( ! pDevice->m_pSubDeviceInfo && pDevice->m_u16DeviceNickName == g_cAddrNicknameInvalid )
            {
                bRet = false;
                break;
            }


            C832_ReadNetworkDeviceIdentity_Resp stResp;

            memcpy(stResp.DeviceUniqueID, pDevice->m_oDeviceUniqueID.bytes, sizeof(stResp.DeviceUniqueID));
            stResp.Nickname = pDevice->m_u16DeviceNickName;

            if (stResp.Nickname == Gateway_Nickname())
            {
                memcpy(stResp.LongTag, gateway.config.m_szLongTag, sizeof(stResp.LongTag));
            }
            else
            {
                memset(stResp.LongTag, 0, sizeof(stResp.LongTag));
                uint8_t* pLongTag = pDevice->GetLongTag();

                if (!pLongTag)
                {
                    if ( pDevice->m_pSubDeviceInfo )
                    {
                        p_pRsp.reset(new CHartCmdWrapper);
                        p_pRsp->LoadParsed(p_pCmd->GetCmdId(),0,0,RCS_E02_InvalidSelection);
                        break;
                    }
                    else
                    {
                        bRet = false;
                        break;
                    }
                }
                memcpy(stResp.LongTag, pLongTag, sizeof(stResp.LongTag));
            }

            p_pRsp.reset(new CHartCmdWrapper);
            p_pRsp->LoadParsed(p_pCmd->GetCmdId(),sizeof(stResp),&stResp,RCS_N00_Success);
        }
        break;

        case CMDID_C841_ReadNetworkDeviceIdentityByNickname:
        {
            bRet = true;
            C841_ReadNetworkDeviceIdentityByNickname_Req* pReq =  (C841_ReadNetworkDeviceIdentityByNickname_Req*) p_pCmd->GetParsedData();

            CLogicalDevice::Ptr pDevice = gateway.m_oDevices.GetLogicalDevice(WHartAddress(pReq->DeviceNickname),false);

            if (!pDevice)
            {
                p_pRsp.reset(new CHartCmdWrapper);
                p_pRsp->LoadParsed(p_pCmd->GetCmdId(),0,0,RCS_E02_InvalidSelection);
                break;
            }

            if ( ! pDevice->m_pSubDeviceInfo && pDevice->m_u16DeviceNickName == g_cAddrNicknameInvalid )
            {
                bRet = false;
                break;
            }

            C841_ReadNetworkDeviceIdentityByNickname_Resp stResp;

            memcpy(stResp.DeviceUniqueID, pDevice->m_oDeviceUniqueID.bytes, sizeof(stResp.DeviceUniqueID));
            stResp.Nickname = pDevice->m_u16DeviceNickName;

            if (stResp.Nickname == Gateway_Nickname())
            {
                memcpy(stResp.LongTag, gateway.config.m_szLongTag, sizeof(stResp.LongTag));
            }
            else
            {
                memset(stResp.LongTag, 0, sizeof(stResp.LongTag));
                uint8_t* pLongTag = pDevice->GetLongTag();
                if ( pDevice->m_pSubDeviceInfo )
                {
                    p_pRsp.reset(new CHartCmdWrapper);
                    p_pRsp->LoadParsed(p_pCmd->GetCmdId(),0,0,RCS_E02_InvalidSelection);
                    break;
                }
                else
                {
                    bRet = false;
                    break;
                }
                memcpy(stResp.LongTag, pLongTag, sizeof(stResp.LongTag));
            }

            p_pRsp.reset(new CHartCmdWrapper);
            p_pRsp->LoadParsed(p_pCmd->GetCmdId(),sizeof(stResp),&stResp,RCS_N00_Success);
        }
        break;

        default:
            LOG_WARN_APP("ExecCmdNmOrGwDualCmds: cmdId="<<p_pCmd->GetCmdId()<<" NOT a dual NM or GW cmd ");
            bRet = false;
	}

	return bRet;
}

}
}
