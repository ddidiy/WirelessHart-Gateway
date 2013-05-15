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

#include <sstream>

#include <WHartHost/model/Device.h>
#include <src/ApplicationLayer/Model/CommonTables.h>

namespace hart7 {
namespace hostapp {


//status
Device::DeviceStatus Device::Status() const
{
	return status;
}

bool Device::IsRegistered() const
{
	return Device::dsRegistered == Status();
}

void Device::Status(DeviceStatus newStatus)
{
	if (newStatus != status)
	{
		if (status == dsRegistered) lastRegistered = CMicroSec();
		status = newStatus;
		isChanged = true;
	}
}

CMicroSec& Device::GetRegisteredTime()
{
	return lastRegistered;
}

//for changes
bool Device::Changed() const
{
	return isChanged;
}

void Device::ResetChanged()
{
	isChanged = false;
}

//graph
int Device::Level() const
{
	return deviceLevel;
}

void Device::Level(int level)
{
	if (level != deviceLevel)
	{
		deviceLevel = level;
		isChanged = true;
	}
}

void Device::SetVertexNo(int no)
{
	vertexNo = no;
}

int Device::GetVertexNo()
{
	return vertexNo;
}

int Device::GetDynamicVariableAssignment(uint8_t variableCode)
{
    switch (variableCode) {
    case DeviceVariableCodes_PrimaryVariable:
        return (m_dynamicVariableAssignments[0] == -1 ? DeviceVariableCodes_PrimaryVariable : m_dynamicVariableAssignments[0]);
    case DeviceVariableCodes_SecondaryVariable:
        return (m_dynamicVariableAssignments[1] == -1 ? DeviceVariableCodes_SecondaryVariable: m_dynamicVariableAssignments[1]);
    case DeviceVariableCodes_TertiaryVariable:
        return (m_dynamicVariableAssignments[2] == -1 ? DeviceVariableCodes_TertiaryVariable : m_dynamicVariableAssignments[2]);
    case DeviceVariableCodes_QuaternaryVariable:
        return (m_dynamicVariableAssignments[3] == -1 ? DeviceVariableCodes_QuaternaryVariable : m_dynamicVariableAssignments[3]);
    default:
        return variableCode;
    }
    return variableCode;
}

void Device::SetDynamicVariableAssignment(uint8_t variableCode, uint8_t assignedCode)
{
    switch (variableCode) {
    case DeviceVariableCodes_PrimaryVariable:
        m_dynamicVariableAssignments[0] = assignedCode;
    case DeviceVariableCodes_SecondaryVariable:
        m_dynamicVariableAssignments[1] = assignedCode;
    case DeviceVariableCodes_TertiaryVariable:
        m_dynamicVariableAssignments[2] = assignedCode;
    case DeviceVariableCodes_QuaternaryVariable:
        m_dynamicVariableAssignments[3] = assignedCode;
    default:
        ;
    }
}

//update_notification_bit_mask
unsigned short Device::GetNotificationBitMask()
{
	return m_notificationBitMask;
}

void Device::SetNotificationBitMask(unsigned short val)
{
	m_notificationBitMask = val;
}

//info
MAC Device::Mac() const
{
	return mac;
}

void Device::Mac(const MAC& mac_)
{
	if (mac_ != mac)
	{
		mac = mac_;
		isChanged = true;
	}
}

NickName Device::Nickname() const
{
	return nickName;
}

void Device::Nickname(const NickName& nickName_)
{
	if (nickName_ != nickName)
	{
		nickName = nickName_;
		isChanged = true;
	}
}

Device::DeviceType Device::Type() const
{
	return deviceType;
}

void Device::Type(DeviceType type_)
{
	if (type_ != deviceType)
	{
		deviceType = type_;
		isChanged = true;
	}
}

boost::uint8_t Device::PowerStatus() const
{
	return powerStatus;
}

void Device::PowerStatus(boost::uint8_t status_)
{
	if (powerStatus != status_)
	{
		powerStatus = status_;
		isChanged = true;
	}
}

boost::uint16_t Device::BatteryLifeStatus() const
{
    return batteryLifeStatus;
}

void Device::BatteryLifeStatus(boost::uint16_t status_)
{
    if (batteryLifeStatus != status_)
    {
        batteryLifeStatus = status_;
        isChanged = true;
    }
}

void Device::SetTAG(const std::string& tag_)
{
	if (deviceTAG != tag_)
	{
		deviceTAG = tag_;
		isChanged = true;
	}
}
const std::string Device::GetTAG()const 
{
	return deviceTAG.size() == 0 ? strNOTAVAILABLE : deviceTAG;
}

void Device::SetSoftwareRevision(int softwareRev_)
{
	if (softwareRev != softwareRev_)
	{
		softwareRev = softwareRev_;
		isChanged = true;
	}
}

int Device::GetSoftwareRevision()
{
	return softwareRev;
}

void Device::SetDeviceCode(int deviceCode_)
{
	if (deviceCode != deviceCode_)
	{
		deviceCode = deviceCode_;
		isChanged = true;
	}
}

int Device::GetDeviceCode()
{
	return deviceCode;
}

void Device::SetRejoinCount(int rejoinCount_)
{
	if (rejoinCount != rejoinCount_)
	{
		rejoinCount = rejoinCount_;
		isChanged = true;
	}
}

int Device::GetRejoinCount()
{
	return rejoinCount;
}

void Device::SetPublishStatus(PublishStatus publishStatus_)
{
    if (publishStatus != publishStatus_)
    {
        publishStatus = publishStatus_;
        isChanged = true;
    }
}

int Device::GetPublishStatus()
{
    return publishStatus;
}



void Device::SetPublisherInfo(PublisherInfo &p_rInfo)
{ 
	m_oPublisherInfo = p_rInfo;
//	for (PublishChannelSetT::const_iterator i = m_oPublisherInfo.channelList.begin(); i != m_oPublisherInfo.channelList.end(); ++i)
//		LOG_DEBUG_APP("[SetPublisherInfo]:  new publisher info with id=" << id <<" has channelID=" << (int)i->channelID);
}

PublisherInfo& Device::GetPublisherInfo()
{ 
	return m_oPublisherInfo;
}

bool Device::IsAdapter()
{
    return adapter;
}

void Device::SetAdapter(bool p_bAdapter)
{
    adapter = p_bAdapter;
}

int Device::GetMasterMode()
{
    return masterMode;
}

void Device::SetMasterMode(int p_nMasterMode)
{
    masterMode = p_nMasterMode;
}

//subscription for notifications
bool Device::HasNotification()
{
	return hasNotification;
}

void Device::SetHasNotification(bool p_bValue)
{
	hasNotification = p_bValue;
}

//constructor
Device::Device()
{
	id = DBCommand::NO_COMMAND_ID;
	deviceType = dtUnknown;
	status = dsUnregistered;
	nickName = NickName::NONE();
	deviceLevel = NO_DEVICE_LEVEL;
	adapter = false;
	masterMode = -1;

	ResetChanged();

	vertexNo = NO_VERTEX_NO;
	IssueSetBurstNotificationCmd = true;
	IssueSetBurstConfigurationCmd = true;
	IssueDiscoveryBurstConfigCmd = hart7::hostapp::AUTODETECT_NONE;
	hasNotification = false;
	m_notificationBitMask = 0;

	deviceCode = 0;
	softwareRev = 0;
	powerStatus = 0;
	batteryLifeStatus = 0;
	rejoinCount = 0;
	configBurstDBCmdID = 0;

	m_dynamicVariableAssignments[0] = -1;
    m_dynamicVariableAssignments[1] = -1;
    m_dynamicVariableAssignments[2] = -1;
    m_dynamicVariableAssignments[3] = -1;
}

//to_string
const std::string Device::ToLongString() const
{
	std::ostringstream str;
	DumpToStream(str);
	return str.str();
}

const std::string Device::ToString() const
{
	std::ostringstream str;
	str << "Device[ DeviceID=" << id << " MAC=" << mac << " Status=" << (int) status << " IP=" << Nickname();
	return str.str();
}

void Device::DumpToStream(std::ostream& p_rStream) const
{
	p_rStream << "Device[ DeviceID=" << id << " MAC=" << mac << " Status=" << (int) status << " IP=" << Nickname() << " Level=" << deviceLevel << " Type=" << (int) deviceType;
}

std::ostream& operator<< (std::ostream& p_rStream, const Device& p_rDevice)
{	p_rDevice.DumpToStream(p_rStream);
	return p_rStream;
}

} //namespace hostapp
} //namespace hart7
