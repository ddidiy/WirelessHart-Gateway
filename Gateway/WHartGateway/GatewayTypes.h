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

#ifndef GatewayTypes_h__
#define GatewayTypes_h__

#include <string.h>
#include <WHartStack/WHartTypes.h>
#include <WHartStack/WHartStack.h>
#include <WHartStack/util/WHartCmdWrapper.h>

#include <ApplicationLayer/Model/NetworkLayerCommands.h>
#include <ApplicationLayer/Model/WirelessNetworkManagerCommands.h>
#include <ApplicationLayer/Model/WirelessApplicationCommands.h>
#include <ApplicationLayer/Model/DataLinkLayerCommands.h>
#include <ApplicationLayer/Model/GatewayCommands.h>
#include <ApplicationLayer/Model/UniversalCommands.h>

/**
 * Types.
 */

#ifndef LOG_TRACKPOINT
#define LOG_TRACKPOINT LOG_DEBUG("TRACK: file="<< __FILE__ <<" function="<< __FUNCTION__ <<" line=" <<__LINE__)
#endif

namespace hart7 {
namespace gateway {
using namespace stack;

const WHartUniqueID			g_cAddrUniqueIdInvalid = {{0}};
const WHartShortAddress		g_cAddrNicknameInvalid = 0xffff;
const int					g_cMaxLocalGwRetries = 10000;


inline bool IsCachedResponseCustom(uint16_t p_u16CmdId)
{
	return (p_u16CmdId == CMDID_C769_ReadJoinStatus 				||
			p_u16CmdId == CMDID_C785_ReadGraphList					||
			p_u16CmdId == CMDID_C784_ReadLinkList					||
			p_u16CmdId == CMDID_C783_ReadSuperframeList 			||
			p_u16CmdId == CMDID_C802_ReadRouteList 					||
			p_u16CmdId == CMDID_C803_ReadSourceRoute 				||
			p_u16CmdId == CMDID_C800_ReadServiceList				||
			p_u16CmdId == CMDID_C084_ReadSubDeviceIdentitySummary	||
			p_u16CmdId == CMDID_C074_ReadIOSystemCapabilities		||
			p_u16CmdId == CMDID_C075_PollSubDevice);

}

inline bool IsNetworkManager(const WHartAddress & p_rSrcAddr)
{
	if (p_rSrcAddr.type == WHartAddress::whartaNickname)
	{
		return p_rSrcAddr.address.nickname == NetworkManager_Nickname();
	}
	else if (p_rSrcAddr.type == WHartAddress::whartaUniqueID)
	{
		return p_rSrcAddr.address.uniqueID == NetworkManager_UniqueID();
	}
	return false;
}

inline bool IsGateway(const WHartAddress & p_rSrcAddr)
{
	if (p_rSrcAddr.type == WHartAddress::whartaNickname)
	{
		return p_rSrcAddr.address.nickname == Gateway_Nickname();
	}
	else if (p_rSrcAddr.type == WHartAddress::whartaUniqueID)
	{
		return p_rSrcAddr.address.uniqueID == Gateway_UniqueID();
	}
	return false;
}

inline bool IsCachedResponseRead(uint16_t p_u16CmdId)
{
	return ( p_u16CmdId == 0 || p_u16CmdId == 7 || p_u16CmdId == 8 || p_u16CmdId == 11 || p_u16CmdId == 12
			|| p_u16CmdId == 13 || p_u16CmdId == 14 ||p_u16CmdId == 15 || p_u16CmdId == 16
			|| p_u16CmdId == 20 || p_u16CmdId == 21 || p_u16CmdId == 48 || p_u16CmdId == 50 || p_u16CmdId == 74);
}

inline bool IsBurstResponse(uint16_t p_u16CmdId)
{
	return ( (1 <= p_u16CmdId && p_u16CmdId <= 3)  || p_u16CmdId == 9 || p_u16CmdId == 33 || p_u16CmdId == 119 || p_u16CmdId == 123 || p_u16CmdId == 178);
}

inline bool ConvertBurstsSentAsRequestToResponses(CHartCmdWrapperList& p_rRequests)
{
	bool onlyBurstResponses = true;
	CHartCmdWrapperList::iterator itCmds = p_rRequests.begin();
	for (itCmds = p_rRequests.begin(); itCmds != p_rRequests.end(); itCmds++)
	{
		CHartCmdWrapper::Ptr pCmd = *itCmds;
		if (!IsBurstResponse(pCmd->GetCmdId()) && (pCmd->GetCmdId() != 48))
		{
		    onlyBurstResponses = false;
			break;
		}
		else
		{
		    pCmd->SetResponseCode(0); //make response
		}
	}
	return onlyBurstResponses;
}

inline bool IsNetMgmtConfigCmd(uint16_t p_u16CmdId)
{
	if (p_u16CmdId == CMDID_C801_DeleteService)
		return true;

	if (CMDID_C960_DisconnectDevice <= p_u16CmdId && p_u16CmdId <= CMDID_C977_DeleteSourceRoute)
	{
		return true;
	}
	return false;
}

inline bool IsCommadDualForGwOrNM(uint16_t p_u16CmdId)
{
	return (832 == p_u16CmdId || p_u16CmdId == 841);
}

inline bool IsCommadForGw(uint16_t p_u16CmdId)
{
	return (835 <= p_u16CmdId && p_u16CmdId <= 838);
}

inline bool IsCommadForNM(uint16_t p_u16CmdId)
{
	return (	(CMDID_C773_WriteNetworkId <= p_u16CmdId && p_u16CmdId <= CMDID_C776_ReadNetworkTag)
				|| p_u16CmdId == CMDID_C794_ReadUTCTime
				|| p_u16CmdId == CMDID_C817_ReadChannelBlacklist
				|| p_u16CmdId == CMDID_C818_WriteChannelBlacklist
				|| p_u16CmdId == CMDID_C821_WriteNetworkAccessMode
				|| p_u16CmdId == CMDID_C822_ReadNetworkAccessMode
				|| p_u16CmdId == CMDID_C833_ReadNetworkDeviceNeighbourHealth
				|| p_u16CmdId == CMDID_C834_ReadNetworkTopologyInformation
				|| p_u16CmdId == CMDID_C840_ReadDeviceStatistics
				|| (CMDID_C842_WriteDeviceSchedulingFlags <= p_u16CmdId && p_u16CmdId <= CMDID_C845_WriteNetworkConstraints) );
}

inline bool IsCommadToRedirectToSM( WHartUniqueID & p_rUniqueId, uint16_t p_u16CmdId)
{
	if (p_rUniqueId == NetworkManager_UniqueID())
	{
	    return false;
	}

	if (p_rUniqueId == Gateway_UniqueID())
	{
	    return p_u16CmdId == 785 || p_u16CmdId == 769 || p_u16CmdId == 780;
	}

	return  p_u16CmdId == 769 || (778 <= p_u16CmdId && p_u16CmdId <= 791) || p_u16CmdId == 800 || p_u16CmdId == 802 || p_u16CmdId == 803 || p_u16CmdId == 833 || p_u16CmdId == 834;
}

inline bool IsNmAlarmCmd(uint16_t p_u16CmdId)
{
	return (778 <= p_u16CmdId && p_u16CmdId <= 791);
}

inline bool IsNmNotifAsBurstCmd(uint16_t p_u16CmdId)
{
	return (p_u16CmdId == CMDID_C000_ReadUniqueIdentifier || p_u16CmdId == CMDID_C832_ReadNetworkDeviceIdentity
	            || p_u16CmdId == CMDID_C020_ReadLongTag
	            || p_u16CmdId == CMDID_C769_ReadJoinStatus
	            || p_u16CmdId == CMDID_C969_WriteGraphNeighbourPair || p_u16CmdId == CMDID_C970_DeleteGraphConnection
	            || p_u16CmdId == CMDID_C967_WriteLink || p_u16CmdId == CMDID_C968_DeleteLink
	            || p_u16CmdId == CMDID_C965_WriteSuperframe || p_u16CmdId == CMDID_C966_DeleteSuperframe
	            || p_u16CmdId == CMDID_C971_WriteNeighbourPropertyFlag
	            || p_u16CmdId == CMDID_C974_WriteRoute || p_u16CmdId == CMDID_C975_DeleteRoute
	            || p_u16CmdId == CMDID_C976_WriteSourceRoute || p_u16CmdId == CMDID_C977_DeleteSourceRoute
	            || p_u16CmdId == CMDID_C973_WriteService || p_u16CmdId == CMDID_C801_DeleteService);
}

inline bool IsNetworkTopologyCmd(uint16_t p_u16CmdId)
{
	return (	p_u16CmdId == CMDID_C785_ReadGraphList ||
				p_u16CmdId == CMDID_C814_ReadDeviceListEntries ||
				p_u16CmdId == CMDID_C833_ReadNetworkDeviceNeighbourHealth ||
				p_u16CmdId == CMDID_C834_ReadNetworkTopologyInformation	||
				p_u16CmdId == CMDID_C832_ReadNetworkDeviceIdentity ||
				p_u16CmdId == CMDID_C769_ReadJoinStatus	);
}

inline bool IsNetworkScheduleCmd(uint16_t p_u16CmdId)
{
	return (	p_u16CmdId == CMDID_C783_ReadSuperframeList	||
				p_u16CmdId == CMDID_C784_ReadLinkList ||
				p_u16CmdId == CMDID_C779_ReportDeviceHealth ||
				p_u16CmdId == CMDID_C780_ReportNeighborHealthList ||
				p_u16CmdId == CMDID_C787_ReportNeighborSignalLevels ||
				p_u16CmdId == CMDID_C800_ReadServiceList ||
				p_u16CmdId == CMDID_C802_ReadRouteList );
}


inline int GetMaxNoForCacheCommand(uint16_t p_u16CmdId)
{
	return 10;
}

inline int GetTimeoutForCacheCommand(uint16_t p_u16CmdId)
{
	return 100000000; //never for now
}

inline bool IsCmdNeedingRetry( int p_nCmdId, int p_nResponseCode)
{
	if (	p_nResponseCode == RCS_E32_Busy
		||	p_nResponseCode == RCS_E33_DelayedResponseInitiated
		||	p_nResponseCode == RCS_E34_DelayedResponseRunning
		||	p_nResponseCode == RCS_E36_DelayedResponseConflict
		)
	{
		return true;
	}

	return false;
}

inline clock_t WhTimeToTicks(uint32_t p_u32Period)
{
	return (p_u32Period >> 5) * sysconf( _SC_CLK_TCK ) / 1000;
}




struct THostAppReqID
{
	WHartUniqueID	m_oHartID;
	uint16_t		m_u16TransID;
};

class CompareHostAppReqID
{
public:
	bool operator()(const THostAppReqID & p_oFirst, const THostAppReqID & p_oSecond)
	{
		int nRet = memcmp(p_oFirst.m_oHartID.bytes, p_oSecond.m_oHartID.bytes, sizeof(p_oSecond.m_oHartID.bytes));

		if (nRet == 0)
		{
			return p_oFirst.m_u16TransID < p_oSecond.m_u16TransID;
		}
		return nRet < 0;
	}

};



} // namespace gateway
} // namespace hart7

#endif // GatewayTypes_h__
