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

#ifndef LogicalDeviceTable_h__
#define LogicalDeviceTable_h__

#include "LogicalDevice.h"

namespace hart7 {
namespace gateway {

using namespace stack;


class GatewayConfig;

/**
 * Aggregates all devices that the GW knows.
 */
class CLogicalDeviceTable
{


public:
	CLogicalDeviceTable(CGwRequests * p_pGwRequestList, GatewayConfig * p_pConfig) : m_pGwRequets(p_pGwRequestList), m_pConfig(p_pConfig)	{ }
	typedef std::map<WHartUniqueID, CLogicalDevice::Ptr, CompareUniqueID> CUniqueIDMap;
	typedef std::map<WHartShortAddress, CLogicalDevice::Ptr> CNickNameMap;
	//load
	//save

	/**
	 * Returns the nickname of a device by uniqueID.
	 */
	WHartShortAddress GetDeviceNick(const WHartUniqueID & p_oUniqueId);

	/**
	 * Returns the device with the requested address.
	 */
	CLogicalDevice::Ptr GetLogicalDevice(const WHartAddress & p_oDeviceAddress, bool p_bOnlyIfActive = true);

	//TODO: add fast access for GetLogicalDeviceNM
	/**
	 * Network Manager device.
	 */
	CLogicalDevice::Ptr GetLogicalDeviceNM() { return GetLogicalDevice(NetworkManager_Nickname()); }
	/**
	 * Gateway device.
	 */
	CLogicalDevice::Ptr GetLogicalDeviceGW() { return GetLogicalDevice(Gateway_Nickname()); }

	/**
	 * Returns all the devices in the table.
	 */
	const CUniqueIDMap & GetDeviceUniqueIDMap();

	/**
	 * Adds a device.
	 */
	CLogicalDevice::Ptr DeviceAdd(const WHartUniqueID & p_oDeviceUniqueID, WHartShortAddress p_u16DeviceNickName = g_cAddrNicknameInvalid, bool p_bIsSubDevice = false);

	/**
	 * Removes a device.
	 */
	void DeviceRemove(const WHartAddress & p_oDeviceAddress);
	bool DeviceRefresh();
	/**
	 * Set device as active and notify.
	 */
	void DeviceNotifyNew(CLogicalDevice::Ptr & p_pDevice);

	/**
	 * Set GW to receive topology changed notifications.
	 */
	void RaiseNotifListChanged();
	/**
	 * Set GW to receive config counter changed notifications.
	 */
	void RaiseNotifConfigCounterChanged();

	/**
	 * Add a response from a device with the specified UniqueID to the GW cache.
	 */
	void ResponseAdd(const WHartAddress& p_oDeviceAddress, uint8_t p_u8DeviceStatus, uint8_t p_u8DeviceStatusExt, const CHartCmdWrapperList & p_oResponses, CGwRequest * p_pGwReq );

	/**
	 * Get a response for a device with the specified UniqueID from the GW cache.
	 */
	bool ResponseGet(const WHartAddress & p_oDeviceAddress, CHartCmdWrapper::Ptr & p_pReq, CHartCmdWrapper::Ptr & p_pRsp);

	/**
	 * /////// Create a series of responses for requests to the GW.
	 */
	int Fill_C783_ReadSuperframeList_Resp(C783_ReadSuperframeList_Req * p_pReq, C783_ReadSuperframeList_Resp * p_pResp);
	int Fill_C784_ReadLinkList_Resp(C784_ReadLinkList_Req * p_pReq, C784_ReadLinkList_Resp * p_pResp);

	int Fill_C800_ReadServiceList_Resp(C800_ReadServiceList_Req * p_pReq,C800_ReadServiceList_Resp * p_pResp);
	int Fill_C814_ReadDeviceListEntries_Resp(C814_ReadDeviceListEntries_Req * p_pReq, C814_ReadDeviceListEntries_Resp * p_pResp, bool p_bJoined );

	int Fill_C075_PollSubDevice(C075_PollSubDevice_Req * p_pReq, C075_PollSubDevice_Resp * p_pResp);
	int Fill_C084_ReadSubDeviceIdentitySummary(C084_ReadSubDeviceIdentitySummary_Req * p_pReq, C084_ReadSubDeviceIdentitySummary_Resp * p_pResp);
	int Fill_C085_ReadIOChannelStatistics(C085_ReadIOChannelStatistics_Req * p_pReq, C085_ReadIOChannelStatistics_Resp * p_pResp);
	int Fill_C086_ReadSubDeviceStatistics(C086_ReadSubDeviceStatistics_Req * p_pReq, C086_ReadSubDeviceStatistics_Resp * p_pResp);

	/**
	 * /////// End create a series of responses for request to the GW.
	 */

	CGwRequests * GetGwRequestList() { return m_pGwRequets; }
	GatewayConfig *	GetGatewayConfig() { return m_pConfig; }


	/**
	 * Processes a response for a Service request.
	 */
	void ServiceReqResponse(const C799_RequestService_Req * p_pReq, const C799_RequestService_Resp * p_pResp, int p_nResponseCode);
	/**
	 * Processes a Write Service request.
	 */
	uint8_t	ServiceWrite(const C973_WriteService_Req * p_pReq, const C973_WriteService_Resp * p_pResp = NULL, int p_nResponseCode = RCS_N00_Success );
	/**
	 * Processes a Delete Service request.
	 */
	uint8_t	ServiceDelete(const C801_DeleteService_Req * p_pReq, const C801_DeleteService_Resp * p_pResp, int p_nResponseCode);
	/**
	 * Set all devices to inactive.
	 */
	void SetInactive();

	/**
	 * Write Nickname for UniqueID.
	 */
	void SetDeviceNickname(CLogicalDevice & p_rDev, WHartShortAddress p_nNickname, WHartUniqueID & p_rUniqueId);

	friend std::ostream & operator<<(std::ostream & stream, const CLogicalDeviceTable & logicalDeviceTable);

private:
	CGwRequests *	m_pGwRequets;
public:
	GatewayConfig *	m_pConfig;
private:
	CUniqueIDMap	m_oDeviceUniqueIDMap;
	CNickNameMap	m_oDeviceNickNameMap;
};


} // namespace gateway
} // namespace hart7

#endif // LogicalDeviceTable_h__
