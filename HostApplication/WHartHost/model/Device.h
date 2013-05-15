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

#ifndef DEVICE_H_
#define DEVICE_H_

#include <vector>
#include <map>
#include <string>
#include <iostream>

#include <WHartHost/model/DBCommand.h>
#include <WHartHost/model/MAC.h>
#include <WHartHost/model/NickName.h>

#include <iostream>
#include <boost/shared_ptr.hpp>

#include <Shared/MicroSec.h>

namespace hart7 {
namespace hostapp {

const std::string strNOTAVAILABLE = "N/A";


class Device;

typedef std::vector<Device> DeviceList;
typedef boost::shared_ptr<Device> DevicePtr;
typedef std::vector<DevicePtr> DevicesPtrList;

class Device
{
	LOG_DEF("hart7.hostapp.model.Device");

public:
	enum DeviceType
	{
		dtUnknown = -1,
		dtSystemManager = 1,
		dtGateway = 2,
		dtAccessPoint = 4,
		dtRoutingDeviceNode = 10
	};

	enum PublishStatus
	{
		PS_NO_DATA = 0,
		PS_FRESH_DATA,
		PS_STALE_DATA
	};

	enum DeviceStatus
	{
		dsUnregistered = 0x000,
		dsRegistered = 0x400,//NormalOperationCommencing in Common Table 52

		dsNotConnected = 0x900, //when the GW is not connected

		dsNetworkPacketsHeard		   = 0x001,
		dsASNAcquired				   = 0x002,
		dsSynchronizedToSlotTime	   = 0x004,
		dsAdvertismentHeard			   = 0x008,
		dsJoinRequested				   = 0x010,
		dsJoinRetrying	    		   = 0x020,
		dsJoinFailed				   = 0x040,
		dsAuthenticated				   = 0x080,
		dsNetworkJoined				   = 0x100,
		dsNegotiatingNetworkProperties = 0x200

	};

//status
public:
	DeviceStatus Status()const;
	bool IsRegistered() const;
	void Status(DeviceStatus newStatus);
	CMicroSec& GetRegisteredTime();

//for changes
public:
	bool Changed() const;
	void ResetChanged();

//graph
public:
	int Level() const;
	void Level(int level);
	void SetVertexNo(int no);
	int GetVertexNo();
	
//update_notification_bit_mask
	unsigned short GetNotificationBitMask();
	void SetNotificationBitMask(unsigned short val);

//info
public:
	MAC Mac() const;
	void Mac(const MAC& mac_);
	NickName Nickname() const;
	void Nickname(const NickName& nickName_);
	DeviceType Type() const;
	void Type(DeviceType type_);
	boost::uint8_t PowerStatus() const;
	void PowerStatus(boost::uint8_t status_);
	boost::uint16_t BatteryLifeStatus() const;
	void BatteryLifeStatus(boost::uint16_t status_);
	void SetTAG(const std::string& tag_);
	const std::string GetTAG()const;
	void SetSoftwareRevision(int softwareRev_);
	int GetSoftwareRevision();
	void SetDeviceCode(int deviceCode_);
	int GetDeviceCode();
	void SetRejoinCount(int rejoinCount_);
	int GetRejoinCount();
    void SetPublishStatus(PublishStatus publishStatus_);
    int GetPublishStatus();

    int GetDynamicVariableAssignment(uint8_t variableCode);
    void SetDynamicVariableAssignment(uint8_t variableCode, uint8_t assignedCode);

	void SetPublisherInfo(PublisherInfo &p_rInfo);
	PublisherInfo& GetPublisherInfo();

	bool IsAdapter();
	void SetAdapter(bool p_bAdapter);

    int GetMasterMode();
    void SetMasterMode(int p_nMasterMode);

//subscription for notifications
public:
	bool HasNotification();
	void SetHasNotification(bool p_bValue);

public:
	static const int NO_DEVICE_LEVEL = 0xFFFFFFFF;
	static const int NO_VERTEX_NO = 0xFFFFFFFF;

public:
	Device();

//to_string
public:
	const std::string ToLongString() const;
	const std::string ToString() const;
protected:
	void DumpToStream(std::ostream& p_rStream) const;
public:
	friend std::ostream& operator<< (std::ostream& p_rStream, const Device& p_rDevice);

//
public:
	bool			IssueSetBurstNotificationCmd;	//needed in the periodic task "DoSetNotification"
	bool			IssueSetBurstConfigurationCmd;	//needed in the periodic task "DoSetNotification"

	AutodetectState IssueDiscoveryBurstConfigCmd;   //needed in the periodic task "DoDiscoveryBurstConfig"
    CMicroSec       lastTimeoutResponse;            //needed to delay a request to device, if a timeout response was received before

private:
	bool			hasNotification;
//
public:
	int				id;

//
public:
	int				configBurstDBCmdID;

private:
	PublisherInfo m_oPublisherInfo;

private:
	NickName		nickName;
	MAC				mac;
	DeviceStatus	status;
	DeviceType		deviceType;
	std::string		deviceTAG; 
	int				deviceCode;
	int				softwareRev;
    boost::uint8_t  powerStatus;
    boost::uint16_t batteryLifeStatus; // in days
	int				rejoinCount;

	bool            adapter;
	int             masterMode;

	PublishStatus   publishStatus;

private:
	CMicroSec		lastRegistered;

private:
	bool			isChanged;
	
private:
	int				deviceLevel;
	int				vertexNo;

// subDevice cache info
public:
    typedef std::map<MAC, uint16_t> SubDevicesMapT;
    SubDevicesMapT subDevicesMap;

private:
	unsigned short m_notificationBitMask;
	int m_dynamicVariableAssignments[4];
};

std::ostream& operator<< (std::ostream& p_rStream, const Device& p_rDevice);


} // namespace hostapp
} // namespace hart7

#endif /*DEVICE_H_*/
