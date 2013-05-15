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


#include <WHartHost/periodictasks/DoDiscoveryBurstConfig.h>

#include <list>



namespace hart7 {
namespace hostapp {

//send commands
void DoDiscoveryBurstConfig::IssueDiscoveryBurstConfigCmd(const MAC& targetDevice, int devID) {
    try
    {
        DBCommand doDiscoveryBurstConfigCmd;
        doDiscoveryBurstConfigCmd.commandCode = DBCommand::ccReadBurstConfig;
        doDiscoveryBurstConfigCmd.deviceID = devID;
        doDiscoveryBurstConfigCmd.generatedType = DBCommand::cgtAutomatic;

        m_dbCommands.CreateCommand(doDiscoveryBurstConfigCmd, boost::str(boost::format("[DoDiscoveryBurstConfig]: Get burst messages for device:=%1% ") % targetDevice.ToString()));

        m_processor.ProcessRequest(doDiscoveryBurstConfigCmd, m_dbCommands, m_devices);

        LOG_INFO_APP("[DiscoveryBurstConfig]: A new 'Discovery Burst Config' request was made for deviceID=" << devID);
    }
    catch(std::exception& ex)
    {
        LOG_ERROR_APP("[DiscoveryBurstConfig]: Error when generate Discovery burst config request! Error:" << ex.what());
    }
}

//do subscribe
void DoDiscoveryBurstConfig::DoTask(int periodTime/*ms*/) {
    if (m_nDiscoveryBurstConfigPeriod == 0) // discovery process is inactive
        return;

    if (!m_isGWConnected)
        return; // no reason to send commands to a disconnected GW

    if (m_taskInit)
    {
        LOG_INFO_APP("[DiscoveryBurstConfig]: LOAD devices from MH file.");
        PublisherConf().LoadAutoDetectStatus(m_strPubConfigFile.c_str(), m_oDevicesStatus);
        m_taskInit = false;
    }

    if (m_doLoadPublishers)
    {
        LOG_INFO_APP("[DiscoveryBurstConfig]: (Signal) RELOAD devices from MH file.");
        PublisherConf().LoadAutoDetectStatus(m_strPubConfigFile.c_str(), m_oDevicesStatus);
        m_doLoadPublishers = false;
    }

    if (m_oLastUpdateTime.GetElapsedSec() < m_nDiscoveryBurstConfigPeriod)
        return; // too soon to update Publisher cache

    // Init time period
    m_oLastUpdateTime.MarkStartTime();

    DevicesManager::const_iterator_by_mac itDevices = m_devices.BeginAllByMac();
    for (; itDevices != m_devices.EndAllByMac(); ++itDevices)
    {
        DevicePtr device = (DevicePtr)itDevices->second;
        if (device->Status() == Device::dsRegistered &&
            device->Type() != Device::dtSystemManager &&
            device->Type() != Device::dtGateway &&
            device->Type() != Device::dtAccessPoint)
        {
            DevicesAutodetectMAP_T::iterator itDiscovery = m_oDevicesStatus.find(device->Mac());
            if (itDiscovery == m_oDevicesStatus.end())
            {
                switch (device->IssueDiscoveryBurstConfigCmd) {
                case AUTODETECT_IN_PROGRESS: // Discovery process already started for device.
                    if (device->lastTimeoutResponse.GetElapsedSec() >= m_nBurstSetConfigRetryPeriod) // checks if passed 1800 seconds and didn't received any response for this device.
                    {
                        LOG_WARN_APP("[DiscoveryBurstConfig]: The delay period expired for deviceMAC=" << device->Mac() << " and a new request will be processed.");
                    }
                    else
                    {
                        continue;
                    }
                case AUTODETECT_DELAYED: // Timeout status received -> wait 300 seconds before sending a new request for command 105.
                    if (device->lastTimeoutResponse.GetElapsedSec() < m_nDiscoveryRequestDelayInterval)
                    {
                        continue;
                    }
                case AUTODETECT_DELETED: // Burst configuration was delete before for that device.
                    continue;
                default : ;
                }

                LOG_INFO_APP("[DiscoveryBurstConfig]: START discovery for deviceMAC=" << device->Mac());

                device->lastTimeoutResponse.MarkStartTime();
                device->IssueDiscoveryBurstConfigCmd = AUTODETECT_IN_PROGRESS;
                IssueDiscoveryBurstConfigCmd(device->Mac(), device->id);
            }
        }
    }
}

//gw notifications
void DoDiscoveryBurstConfig::HandleGWConnect(const std::string& host, int port)
{
	m_isGWConnected = true;
}

void DoDiscoveryBurstConfig::HandleGWDisconnect()
{
	m_isGWConnected = false;
}

void DoDiscoveryBurstConfig::UpdateStoredPublishers(MAC& p_rMac, PublisherInfo& p_rPubInfo, int p_nIssueCase)
{
    //LOG_INFO_APP("[DiscoveryBurstConfig] - PublisherInfo[" << p_rMac << "]= " << GetPublisherInfoString(p_rPubInfo));

    int retCode;
    DevicePtr device = m_devices.FindDevice(p_rMac);
    if (!device)
    {
        LOG_INFO_APP("[DiscoveryBurstConfig]: UpdateStoredPublishers - Device " << p_rMac << " is invalid or isn't connected!");
        return;
    }

    DevicesAutodetectMAP_T::iterator itPub = m_oDevicesStatus.find(p_rMac);
    if (itPub != m_oDevicesStatus.end())
    {
        LOG_INFO_APP("[DiscoveryBurstConfig]: UpdateStoredPublishers - Publisher was already loaded from file. Discard discovery data for device=" << p_rMac);
        return;
    }

    switch (p_nIssueCase) {
    case AUTODETECT_DONE: // save discovered bursts
        // Insert/update publisher info in Publisers' memory cache
    	m_oDevicesStatus[p_rMac] = AUTODETECT_DONE;
        m_PublishersMapStored.insert(std::make_pair(p_rMac, p_rPubInfo));

        LOG_INFO_APP("[DiscoveryBurstConfig]: UpdateStoredPublishers - Insert/update publisher info for deviceMAC=" << device->Mac());
        // Insert/update publisher info in Publishers' file
        retCode = PublisherConf().SavePublisher(m_strPubConfigFile.c_str(), p_rMac, p_rPubInfo);
        if (retCode < 0) {
            LOG_ERROR_APP("[DiscoveryBurstConfig]:UpdateStoredPublishers - Error on saving new discovered data in Publishing file");
        }
        break;
    case AUTODETECT_NOT_IMPLEMENT_105: // device hasn't implemented command 105
        m_oDevicesStatus[p_rMac] = AUTODETECT_NOT_IMPLEMENT_105;
        LOG_INFO_APP("[DiscoveryBurstConfig]:UpdateStoredPublishers - Insert device in the list of publishers that have not implemented command 105");

        retCode = PublisherConf().SavePublisherNotImpl105(m_strPubConfigFile.c_str(), p_rMac);
        if (retCode < 0) {
            LOG_ERROR_APP("[DiscoveryBurstConfig]: UpdateStoredPublishers - Error on setting deviceMAC=" << p_rMac << " as not implementing command 105");
        }
        break;
    default: ;
    }
}

std::string DoDiscoveryBurstConfig::GetPublishersString(PublisherInfoMAP_T& publishersMap)
{
    std::ostringstream str;
    str << "\n[DiscoveryBurstConfig]: ..................\n";

    PublisherInfoMAP_T::iterator it = publishersMap.begin();
    for (; it != publishersMap.end(); ++it) {
        str << "[DiscoveryBurstConfig]: [" << it->first.ToString() << "]" << GetPublisherString(it->second);
    }
    str << "\n[DiscoveryBurstConfig]: ..................";

    return str.str();
}

std::string DoDiscoveryBurstConfig::GetPublisherString(PublisherInfo& publisherInfo)
{
    std::ostringstream str;

    str << "\nBURSTS={";
    for (BurstMessageSetT::iterator itBurst = publisherInfo.burstMessageList.begin(); itBurst != publisherInfo.burstMessageList.end() ; ++itBurst)
    {
        if (itBurst != publisherInfo.burstMessageList.begin()) { str << "\n        "; }
        str << "[" << (int)itBurst->burstMessage << ", " << (int)itBurst->cmdNo << ", " << itBurst->updatePeriod << ", " << itBurst->maxUpdatePeriod << ", " << itBurst->subDeviceMAC.ToString() << "]";
    }

    str << "}\nVARIABLES={";
    for (PublishChannelSetT::iterator itChannel = publisherInfo.channelList.begin(); itChannel != publisherInfo.channelList.end() ; ++itChannel)
    {
        if (itChannel != publisherInfo.channelList.begin()) { str <<"\n           "; }
        str << "[" << (int)itChannel->burstMessage << ", " << (int)itChannel->cmdNo << ", " <<  (int)itChannel->deviceVariableSlot << ", " << (int)itChannel->deviceVariable << ", " << itChannel->name << ", " << (int)itChannel->classification << ", " << (int)itChannel->unitCode << "]";
    }

    str << "}\nTRIGGERS={";
    for (TriggerSetT::iterator itTrigger = publisherInfo.triggersList.begin(); itTrigger != publisherInfo.triggersList.end() ; ++itTrigger)
    {
        if (itTrigger != publisherInfo.triggersList.begin()) { str <<"\n          "; }
        str << "[" << (int)itTrigger->burstMessage << ", " << (int)itTrigger->cmdNo << ", " << (int)itTrigger->modeSelection << ", " << (int)itTrigger->classification << ", " << (int)itTrigger->unitCode << ", " << itTrigger->triggerLevel << "]";
    }
    str << "}\n";

    return str.str();
}

}//namespace hostapp
}//namespace hart7

