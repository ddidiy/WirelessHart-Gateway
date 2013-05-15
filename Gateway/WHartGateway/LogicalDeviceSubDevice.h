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

#ifndef LogicalDeviceSubDevic_h__
#define LogicalDeviceSubDevic_h__

#include <WHartStack/util/WHartCmdWrapper.h>
#include <WHartGateway/AllGatewayCommands.h>
#include <WHartGateway/GatewayTypes.h>
#include <set>
#include <map>


namespace hart7 {
namespace gateway {

using namespace stack;


typedef std::map<uint16_t, boost::shared_ptr<C084_ReadSubDeviceIdentitySummary_Resp> > CSubDeviceIndexMap;

typedef C075_PollSubDevice_Req SubdevKey;

class CompareSubdevKey
{
public:
	bool operator()(const SubdevKey& p_oFirst, const SubdevKey& p_oSecond)
	{	return ((p_oFirst.ioCard < p_oSecond.ioCard) &&
				(p_oFirst.channel < p_oSecond.channel) &&
				(p_oFirst.poolingAddress < p_oSecond.poolingAddress));
	}
};

typedef std::map<SubdevKey, WHartUniqueID, CompareSubdevKey> CSubDeviceKeyMap;

typedef uint16_t SubdevConnectionKey;
typedef std::map<SubdevConnectionKey, int> CSubDevConnectionsNbMap;


class CGwRequest;
class CLogicalDevice;
/**
 * Model of a subdevice in the Gateway. Caches responses from subdevices. Handles discovery flow.
 */
class CLogicalDeviceInfoAdapter
{
public:
	CLogicalDeviceInfoAdapter(CLogicalDevice * p_pLogicalDevice)
		: m_pLogicalDevice(p_pLogicalDevice)
	{
	    ClearDiscoveryFlowState();
	}

	void ResetC075_PollInfo()
	{
		m_oCurrentPollInfo.ioCard = 0;
		m_oCurrentPollInfo.channel = 0;
		m_oCurrentPollInfo.poolingAddress = 0xff; //next will be 0

		m_u16C075CurrentTransId = 0;
	}

	CLogicalDevice * MakeInactiveSubDevice(SubdevKey & p_rPollSubDevice);
	CLogicalDevice * MakeInactiveSubDevice(WHartUniqueID & p_rUniqueId);

	bool ResponseAdd(CHartCmdWrapper::Ptr & p_pRsp, CGwRequest * p_pGwReq );
	bool ResponseAdd_C074(CHartCmdWrapper::Ptr & p_pRsp, CGwRequest * p_pGwReq );
	bool ResponseAdd_C084(CHartCmdWrapper::Ptr & p_pRsp, CGwRequest * p_pGwReq );
	bool ResponseAdd_C075(CHartCmdWrapper::Ptr & p_pRsp, CGwRequest * p_pGwReq );
	bool ResponseAdd_C048(CHartCmdWrapper::Ptr & p_pRsp, CGwRequest * p_pGwReq );

	bool ResponseGet(CHartCmdWrapper::Ptr & p_pReq, CHartCmdWrapper::Ptr & p_pRsp);
	bool ResponseGet_C074(CHartCmdWrapper::Ptr & p_pReq, CHartCmdWrapper::Ptr & p_pRsp);
	bool ResponseGet_C084(CHartCmdWrapper::Ptr & p_pReq, CHartCmdWrapper::Ptr & p_pRsp);
	bool ResponseGet_C075(CHartCmdWrapper::Ptr & p_pReq, CHartCmdWrapper::Ptr & p_pRsp);

	bool CheckNeedAdd_C075_PollSubDevice();
	bool CheckNeedAdd_C084_ReadSubDeviceIdentitySummary();

	bool Refresh();

	void RestartDiscoveryFlow();

private:
	WHartUniqueID MakeCorrectedUniqueID(uint16_t deviceCode, uint32_t deviceID);

	SubdevConnectionKey MakeSubdevConnectionKey(uint8_t p_u8IoCard, uint8_t p_u8Channel)
	{	return  (p_u8IoCard << 8) | p_u8Channel;
	}

	SubdevConnectionKey MakeSubdevConnectionKey(SubdevKey p_nSubdevKey)
	{	return  MakeSubdevConnectionKey(p_nSubdevKey.ioCard, p_nSubdevKey.channel);
	}

	uint8_t GetCard(SubdevConnectionKey key) {return (uint8_t)(key >> 8);}
	uint8_t GetChannel(SubdevConnectionKey key) {return (uint8_t)(key & 0xf);}

	void ClearDiscoveryFlowState();
	void CorrectSubdevMaps();

public:
	boost::shared_ptr<C074_ReadIOSystemCapabilities_Resp>		m_pC074_ReadIOSystemCapabilities_Resp;

	bool														m_bDiscoveryInProgress;
	bool														m_bReadDevicesIdentityInProgress;
	C075_PollSubDevice_Req										m_oCurrentPollInfo;
	uint16_t													m_u16C084CurrentSubdevIndex;
	uint16_t													m_u16C075CurrentTransId;
	uint16_t													m_u16C084CurrentTransId;

	CSubDeviceIndexMap									m_oSubDeviceIndexMap;
	CSubDeviceKeyMap									m_oSubDeviceKeyMap;

private:
	CSubDevConnectionsNbMap									m_oSubDevConnectionsNbMap;

public:
	CLogicalDevice* m_pLogicalDevice;
};

class CLogicalDeviceInfoSubDevice
{
public:
	CLogicalDeviceInfoSubDevice(CLogicalDevice* p_pLogicalDevice) : m_pLogicalDevice(p_pLogicalDevice), m_pAdapter(NULL)
	{
		memset(&m_oSubDeviceKey, 0, sizeof(m_oSubDeviceKey));
	}

public:
	CLogicalDevice*			m_pLogicalDevice;
	SubdevKey				m_oSubDeviceKey;

	CLogicalDevice*			m_pAdapter;
};


} // namespace gateway
} // namespace hart7

#endif // LogicalDeviceSubDevic_h__
