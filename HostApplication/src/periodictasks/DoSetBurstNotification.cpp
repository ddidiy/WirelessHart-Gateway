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


#include <WHartHost/periodictasks/DoSetBurstNotification.h>

#include <list>



namespace hart7 {
namespace hostapp {


static bool UpdateBurstNeeded(int deviceID, BurstMessageSetT &loaded, BurstMessageSetT &stored)
{
	BurstMessageSetT::iterator i = loaded.begin();
	BurstMessageSetT::iterator j = stored.begin();


	LOG_DEBUG_APP("[DoSetBurstNotification]: BurstProcessing -> loaded bursts size = " << loaded.size()
			 << "stored bursts size = " << stored.size() << " for deviceID=" << deviceID);

	if (loaded.size() != stored.size())
		return true;

	for (; i != loaded.end(); ++i, ++j)
	{

		if (j == stored.end())
		{
			LOG_DEBUG_APP("[DoSetBurstNotification]: only in loaded -> i: CmdNo= " << i->cmdNo << " burstMessage = " << (int)i->burstMessage);
			return true;
		}
		
		LOG_DEBUG_APP("[DoSetBurstNotification]: BurstProcessing -> in loaded -> i: CmdNo= " << i->cmdNo << " burstMessage = " << (int)i->burstMessage
							<< " update period=" << i->updatePeriod << "maxUpdatePeriod=" << i->maxUpdatePeriod);
		LOG_DEBUG_APP("[DoSetBurstNotification]: BurstProcessing -> in stored -> j: CmdNo= " << j->cmdNo << " burstMessage = " << (int)j->burstMessage
							<< " update period=" << j->updatePeriod << "maxUpdatePeriod=" << j->maxUpdatePeriod);
		
		if ( (*i) == (*j) )
		{
			//found a match...
		}
		else
		{
			return true;
		}

		if (i->updatePeriod != j->updatePeriod ||
			i->maxUpdatePeriod != j->maxUpdatePeriod)
			return true;
	}

	return false;
}

static bool UpdateTriggerNeeded(int deviceID, TriggerSetT &loaded, TriggerSetT &stored)
{
	TriggerSetT::iterator i = loaded.begin();
	TriggerSetT::iterator j = stored.begin();


	LOG_DEBUG_APP("[DoSetBurstNotification]: TriggerProcessing -> loaded triggers size = " << loaded.size()
		<< "stored triggers size = " << stored.size() << " for deviceID=" << deviceID);

	if (loaded.size() != stored.size())
		return true;

	for (; i != loaded.end(); ++i, ++j)
	{

		if (j == stored.end())
		{
			LOG_DEBUG_APP("[DoSetBurstNotification]: only in loaded -> i: CmdNo= " << i->cmdNo << " burstMessage = " << (int)i->burstMessage);
			return true;
		}

		LOG_DEBUG_APP("[DoSetBurstNotification]: BurstProcessing -> in loaded -> i: CmdNo= " << i->cmdNo << " burstMessage = " << (int)i->burstMessage);
		LOG_DEBUG_APP("[DoSetBurstNotification]: BurstProcessing -> in stored -> j: CmdNo= " << j->cmdNo << " burstMessage = " << (int)j->burstMessage);

		if ( (*i) == (*j) )
		{
			//found a match...
		}
		else
		{
			return true;
		}

		if (i->modeSelection != j->modeSelection ||
			i->classification != j->classification ||
			i->unitCode != j->unitCode ||
			i->triggerLevel != j->triggerLevel)
			return true;
	}

	return false;
}

static bool UpdateChannelNeeded(int deviceID, PublishChannelSetT &loaded, PublishChannelSetT &stored)
{
	LOG_DEBUG_APP("[DoSetBurstNotification]: channels comparison-> loaded channels size = " << loaded.size()
		<< " stored channels size = " << stored.size() << " for deviceID=" << deviceID);

	PublishChannelSetT::const_iterator i = loaded.begin();
	PublishChannelSetT::const_iterator j = stored.begin();

	if (loaded.size() != stored.size())
		return true;

	for (; i != loaded.end(); ++i, ++j)
	{

		if (j == stored.end())
		{
			LOG_DEBUG_APP("[DoSetBurstNotification]: channels comparison-> only in loaded -> i: CmdNo= " << i->cmdNo << " burstMessage = " << (int)i->burstMessage << " deviceVariableSlot = " << i->deviceVariableSlot);
			return true;
		}

		LOG_DEBUG_APP("[DoSetBurstNotification]: channels comparison-> in loaded -> i: CmdNo= " << i->cmdNo << " burstMessage = " << (int)i->burstMessage << " deviceVariableSlot = " << (int)i->deviceVariableSlot);
		LOG_DEBUG_APP("[DoSetBurstNotification]: channels comparison-> in stored -> j: CmdNo= " << j->cmdNo << " burstMessage = " << (int)j->burstMessage << " deviceVariableSlot = " << (int)j->deviceVariableSlot);

		if ( (*i) == (*j) )
		{
			//found a match...
		}
		else
		{
			return true;
		}
		
		if (i->deviceVariable != j->deviceVariable ||
			i->unitCode != j->unitCode ||
			i->classification != j->classification || 
			i->name != j->name)	
		return true;

		//copy channelID to prepare for the case when nothing needs to be done
		i->channelID = j->channelID; 

	}

	return false;
}

static bool UpdateBurstsStateNeeded(int deviceID, PublisherInfo::BurstMessageStateMap &loaded, int burstsize)
{
	LOG_DEBUG_APP("[DoSetBurstNotification]: BurstsStateProcessing -> loaded bursts_state size = " << loaded.size()
		<< "and bursts size = " << burstsize << " for deviceID=" << deviceID);

	//see if we have bursts with an unspecified state (which defaults to NOT_SET)
	if (burstsize != (int)loaded.size())
	{	return true;
	}

	for (PublisherInfo::BurstMessageStateMap::const_iterator i = loaded.begin(); i != loaded.end(); ++i)
	{
		if (i->second != BURST_STATE_SET && i->second != BURST_STATE_ERROR)
			return true;
	}

	return false;
}

//send commands
void DoSetBurstNotification::IssueSetBurstNotificationCmd(const MAC& targetDevice, int devID)
{
	DBCommand doSetBurstNotificationCmd;
	doSetBurstNotificationCmd.commandCode = DBCommand::ccNotifSubscribe;
	doSetBurstNotificationCmd.deviceID = devID;

	doSetBurstNotificationCmd.generatedType = DBCommand::cgtAutomatic;
	m_dbCommands.CreateCommand(doSetBurstNotificationCmd, boost::str(boost::format("system: do set burst notification for device:=%1% ")
		% targetDevice.ToString()));

	m_processor.ProcessRequest(doSetBurstNotificationCmd, m_dbCommands, m_devices);

	LOG_INFO_APP("[DoSetBurstNotification]: Automatic Do set burst notification request was made!"
		<< boost::str(boost::format(" DeviceID=%1%, DeviceMAC=%2%") % doSetBurstNotificationCmd.deviceID % targetDevice.ToString()));
}

void DoSetBurstNotification::IssueUnsetBurstNotificationCmd(const MAC& targetDevice, int devID)
{
	DBCommand doSetBurstNotificationCmd;
	doSetBurstNotificationCmd.commandCode = DBCommand::ccNotifUnSubscribe;
	doSetBurstNotificationCmd.deviceID = devID;

	doSetBurstNotificationCmd.generatedType = DBCommand::cgtAutomatic;
	m_dbCommands.CreateCommand(doSetBurstNotificationCmd, boost::str(boost::format("system: do unset burst notification for device:=%1% ")
		% targetDevice.ToString()));

	m_processor.ProcessRequest(doSetBurstNotificationCmd, m_dbCommands, m_devices);

	LOG_INFO_APP("[DoSetBurstNotification]: Automatic Do unset burst notification request was made!"
		<< boost::str(boost::format(" DeviceID=%1%, DeviceMAC=%2%") % doSetBurstNotificationCmd.deviceID % targetDevice.ToString()));
}

void DoSetBurstNotification::UnsubcribeForNotification(const MAC& targetDevice)
{
	DevicePtr dev = m_devices.FindDevice(targetDevice);
	if(!dev)
	{
		LOG_WARN_APP("[DoSetBurstNotification]: device =" << targetDevice.ToString() << "not found in cache");
		return;
	}
	IssueUnsetBurstNotificationCmd(targetDevice, dev->id);
}

void DoSetBurstNotification::IssueSetBurstConfigurationCmd(const MAC& targetDevice, int devID)
{
 	DBCommand doSetBurstConfigurationCmd;
	doSetBurstConfigurationCmd.commandCode = DBCommand::ccAutodetectBurstsConfig;
	doSetBurstConfigurationCmd.deviceID = devID;

	doSetBurstConfigurationCmd.generatedType = DBCommand::cgtAutomatic;

	doSetBurstConfigurationCmd.parameters.push_back(DBCommandParameter(DBCommandParameter::ConfigureBurst_PubConfFile, m_strPubConfigFile));

	m_dbCommands.CreateCommand(doSetBurstConfigurationCmd, boost::str(boost::format("system: do set burst configuration for device:=%1% ")
		% targetDevice.ToString()));

	m_processor.ProcessRequest(doSetBurstConfigurationCmd, m_dbCommands, m_devices);

	LOG_INFO_APP("[DoSetBurstNotification]: Automatic Do set burst configuration request was made!"
		<< boost::str(boost::format(" DeviceID=%1%, DeviceMAC=%2%") % doSetBurstConfigurationCmd.deviceID % targetDevice.ToString()));
}

//with signal
bool DoSetBurstNotification::IsSubscribeNotifSent()
{
	PublisherInfoMAP_T::iterator i = m_PublishersMapStored.begin();
	DevicesManager::const_iterator_by_mac j = m_devices.BeginAllByMac();
	for (; i != m_PublishersMapStored.end(); ++i)
	{
		if (j == m_devices.EndAllByMac())
		{
			LOG_DEBUG_APP("[DoSetBurstNotification]: (IsSubscribeSent) pub with mac = " << i->first << "not found in cache... Skipping check...");
			continue;
		}
		if (i->first == j->first)
		{
			//found a match...
		}
		else 
		{
			if (j->first < i->first)
			{
				do
				{
					++j;
					if (j == m_devices.EndAllByMac())
						break;
				}while (j->first < i->first);
				if (j == m_devices.EndAllByMac())
				{
					LOG_DEBUG_APP("[DoSetBurstNotification]: (IsSubscribeSent) pub with mac = " << i->first << "not found in cache... Skipping check...");
					continue; // continue 'for'
				}
				if (i->first < j->first)
				{
					LOG_DEBUG_APP("[DoSetBurstNotification]: (IsSubscribeSent) pub with mac = " << i->first << "not found in cache... Skipping check...");
					continue;
				}
				
				//found a match...
			}
			else 
			{
				LOG_DEBUG_APP("[DoSetBurstNotification]: (IsSubscribeSent) pub with mac = " << i->first << "not found in cache... Skipping check...");
				continue;
			}
		}
 
		if(j->second->IssueSetBurstNotificationCmd == false){
			LOG_INFO_APP("[DoSetBurstNotification]: (IsSubscribeSent) pub with mac = " << i->first << " has sent subscriber notification request ...");
			return true;
		}
		
		if(j->second->IssueSetBurstConfigurationCmd == false){
			//cancel command
			j->second->IssueSetBurstConfigurationCmd = true;
			j->second->configBurstDBCmdID = 0;
			LOG_INFO_APP("[DoSetBurstNotification]: (IsSubscribeSent) pub with mac = " << i->first << " has sent bursts config request, but it was set to cancelled state!");
			++j;
			continue;
		}

		if (i->second.hasNotification == true)
			LOG_DEBUG_APP("[DoSetBurstNotification]: (IsSubscribeSent) pub with mac = " << i->first << "has subscriber notification");
		else
			LOG_DEBUG_APP("[DoSetBurstNotification]: (IsSubscribeSent) pub with mac = " << i->first << "has no subscriber notification");

		++j;
	}

	return false;
}

void DoSetBurstNotification::RefreshPublishers()
{
	PublisherInfoMAP_T::iterator i = m_PublishersMapStored.begin();
	DevicesManager::const_iterator_by_mac j = m_devices.BeginAllByMac();
	for (; i != m_PublishersMapStored.end(); ++i)
	{
		if (j == m_devices.EndAllByMac())
		{
			LOG_DEBUG_APP("[DoSetBurstNotification]: (Refresh) pub with mac = " << i->first << "not found in cache... Skipping check...");
			continue;
		}
		if (i->first == j->first)
		{
			//found a match...
		}
		else 
		{
			if (j->first < i->first)
			{
				do
				{
					++j;
					if (j == m_devices.EndAllByMac())
						break;
				}while (j->first < i->first);
				if (j == m_devices.EndAllByMac())
				{
					LOG_DEBUG_APP("[DoSetBurstNotification]: (Refresh) pub with mac = " << i->first << "not found in cache... Skipping check...");
					continue; // continue 'for'
				}
				if (i->first < j->first)
				{
					LOG_DEBUG_APP("[DoSetBurstNotification]: (Refresh) pub with mac = " << i->first << "not found in cache... Skipping check...");
					continue;
				}
				
				//found a match...
			}
			else 
			{
				LOG_DEBUG_APP("[DoSetBurstNotification]: (Refresh) pub with mac = " << i->first << "not found in cache... Skipping check...");
				continue;
			}
		}

		j->second->SetHasNotification(false);
		i->second.hasNotification = false;
		LOG_DEBUG_APP("[DoSetBurstNotification]: (Refresh) pub with mac = " << i->first << "has been refreshed");

		++j;
	}
}

void DoSetBurstNotification::ComputeNewPublishers()
{
	PublisherInfoMAP_T::iterator i = m_PublishersMapStored.begin();
	PublisherInfoMAP_T::iterator j = m_PublishersMapLoaded.begin();


	for (; i != m_PublishersMapStored.end(); ++i)
	{

		if (j == m_PublishersMapLoaded.end())
		{
			LOG_DEBUG_APP("[DoSetBurstNotification]: (Intersect) pub with mac = " << i->first << "not found in loaded list...do unsubscribe");
			UnsubcribeForNotification(i->first);
			continue;
		}
		if (i->first == j->first)
		{
			//found a match...
		}
		else 
		{
			if (j->first < i->first)
			{
				do
				{
					++j;
					if (j == m_PublishersMapLoaded.end())
						break;
				}while (j->first < i->first);
				if (j == m_PublishersMapLoaded.end())
				{
					LOG_DEBUG_APP("[DoSetBurstNotification]: (Intersect) pub with mac = " << i->first << "not found in loaded list...do unsubscribe");
					UnsubcribeForNotification(i->first);
					continue;
				}
				if (i->first < j->first)
				{
					LOG_DEBUG_APP("[DoSetBurstNotification]: (Intersect) pub with mac = " << i->first << "not found in loaded list...do unsubscribe");
					UnsubcribeForNotification(i->first);
					continue;
				}
				
				//found a match...
			}
			else 
			{
				LOG_DEBUG_APP("[DoSetBurstNotification]: (Intersect) pub with mac = " << i->first << "not found in loaded list...do unsubscribe");
				UnsubcribeForNotification(i->first);
				continue;
			}
		}

		PublisherInfo& pubStored = i->second;
		PublisherInfo& pubLoaded = j->second;

		//enable issuing again the notify subscription
		DevicePtr dev = m_devices.FindDevice(j->first);
		if(dev)
		{
			if (dev->HasNotification()) //check the differences
			{
				if (UpdateChannelNeeded(dev->id, pubLoaded.channelList, pubStored.channelList) &&
					!UpdateBurstsStateNeeded(dev->id, pubLoaded.burstMessagesState, pubLoaded.burstNoTotalCmd105))
				{
					//do not allow any change until bursts_flags are set
					pubLoaded = pubStored;
					++j;
					continue;
				}

				if (UpdateBurstsStateNeeded(dev->id, pubLoaded.burstMessagesState, pubLoaded.burstNoTotalCmd105))
				{
					LOG_INFO_APP("[DoSetBurstNotification]: device id=" << dev->id << "has new bursts_states with signal, so do process...");
					pubLoaded.reconfigBurst = true;

					if (UpdateBurstNeeded(dev->id, pubLoaded.burstMessageList, pubStored.burstMessageList))
					{
						LOG_INFO_APP("[DoSetBurstNotification]: device id=" << dev->id << "has new burstmessages with signal, so do process...");
						std::list<BurstMessage> added, updated, deleted;
						m_devices.ProcessDBBursts(dev->id, pubLoaded.burstMessageList, added, updated, deleted);
						m_devices.PubDeviceBurstsChangeReceived(dev->id, added, deleted, updated);
						LOG_INFO_APP("[DoSetBurstNotification]: device id=" << dev->id << "has new burstmessages with signal, process DONE!");
					}
					if (UpdateChannelNeeded(dev->id, pubLoaded.channelList, pubStored.channelList))
					{
						LOG_INFO_APP("[DoSetBurstNotification]: device id=" << dev->id << "has new channelslist with signal, so do process...");
						m_devices.ProcessDBChannels(dev->id, pubLoaded.channelList);
						LOG_INFO_APP("[DoSetBurstNotification]: device id=" << dev->id << "has new channelslist with signal, process DONE!");
					}
					if (UpdateTriggerNeeded(dev->id, pubLoaded.triggersList, pubStored.triggersList))
					{
						LOG_INFO_APP("[DoSetBurstNotification]: device id=" << dev->id << "has new triggerslist with signal, so do process...");
						m_devices.ProcessDBTriggers(dev->id, pubLoaded.triggersList);
						LOG_INFO_APP("[DoSetBurstNotification]: device id=" << dev->id << "has new triggerslist with signal, DONE!");
					}

					LOG_INFO_APP("[DoSetBurstNotification]: device id=" << dev->id << "has new bursts_states with signal, DONE!");
				}
			}

			LOG_WARN_APP("[DoSetBurstNotification]: (Intersect) pub with mac =" << j->first << "found in loaded list");
		}
		else
		{
			LOG_WARN_APP("[DoSetBurstNotification]: (Intersect) pub with mac =" << j->first << "found in loaded list but not found in cache");
		}
		++j;
	}
	m_PublishersMapStored.clear();
	m_PublishersMapStored = m_PublishersMapLoaded;
	m_PublishersMapLoaded.clear();
}

bool DoSetBurstNotification::ReloadPublishers()
{
	if (IsSubscribeNotifSent())
		return false;

	std::list<double> burstUpdatePeriods;
	PublisherConf().LoadPublishers(m_strPubConfigFile.c_str(), m_PublishersMapLoaded, burstUpdatePeriods);

	//set no of retries
	PublisherInfoMAP_T::iterator it = m_PublishersMapLoaded.begin();
	for (; it != m_PublishersMapLoaded.end(); ++it)
	{
		PublisherInfo& pubInfo = it->second;
		pubInfo.burstSetConfigShortDelayRetries = m_nBurstSetConfigShortDelayRetries;
	}

	//notify update period
	std::list<unsigned int> vals;
	for (std::list<double>::const_iterator i = burstUpdatePeriods.begin(); i != burstUpdatePeriods.end(); ++i)
		vals.push_back((unsigned int)((m_nPublishPeriodToleranceThreshold + (*i))*1000)/*ms*/);

	try
    {
        int gcdVal = ProcessGCD(vals);
        m_devices.UpdateBurstCounterTaskPeriod(gcdVal);
        LOG_INFO_APP("[DoSetBurstNotification]: Task will be run at " << gcdVal << " ms period");
    }
    catch (std::exception e)
    {
        LOG_ERROR_APP("[GCD]: Application error: " << e.what());
        return false;
    }
    catch (...)
    {
        LOG_ERROR_APP("[GCD]: System error!");
        return false;
    }

	ComputeNewPublishers();
	return true;
}

//do subscribe
void DoSetBurstNotification::DoTask(int periodTime/*ms*/)
{
    if(m_taskInit)
	{
		//
		std::list<double> burstUpdatePeriods;
		PublisherConf().LoadPublishers(m_strPubConfigFile.c_str(), m_PublishersMapStored, burstUpdatePeriods);

		//set no of retries
		PublisherInfoMAP_T::iterator it = m_PublishersMapStored.begin();
		for (; it != m_PublishersMapStored.end(); ++it)
		{
			PublisherInfo& pubInfo = it->second;
			pubInfo.burstSetConfigShortDelayRetries = m_nBurstSetConfigShortDelayRetries;
		}

		//notify update period
		std::list<unsigned int> vals;
		for (std::list<double>::const_iterator i = burstUpdatePeriods.begin(); i != burstUpdatePeriods.end(); ++i)
		    vals.push_back((unsigned int)((m_nPublishPeriodToleranceThreshold + (*i))*1000/*ms*/));

		try
        {
            int gcdVal = ProcessGCD(vals);
            m_devices.UpdateBurstCounterTaskPeriod(gcdVal);
            LOG_INFO_APP("[DoSetBurstNotification]: Task will be run at " << gcdVal << " ms period");
        }
        catch (std::exception e)
        {
            LOG_ERROR_APP("[GCD]: Application error: " << e.what());
            m_taskInit = false;
        }
        catch (...)
        {
            LOG_ERROR_APP("[GCD]: System error!");
            m_taskInit = false;
        }

		m_taskInit = false;
	}

	if (!m_isGWConnected)
		return; // no reason to send commands to a disconnected GW

	if (m_isGWReconnected)
	{
		// do refresh configuration...
		RefreshPublishers();

		//at the end
		m_isGWReconnected = false;
	}

	try
	{
		int noOfPublishers = 0;
		ConfigurePublishers(noOfPublishers);

		LOG_INFO_APP("[DoSetBurstNotification]: configured publishers = " << (boost::uint32_t)m_PublishersMapStored.size()
					<< " and only " << (boost::uint32_t)noOfPublishers << " of them have notification");
	}
	catch (std::exception& ex)
	{
		LOG_ERROR_APP("[DoSetBurstNotification]: Do Set Notification failed! error=" << ex.what());
	}

	//if signalled
	if (m_doLoadPublishers == true)
	{
		LOG_INFO_APP("[DoSetBurstNotification]: (Signal) reload publishers right now...");
		if (!ReloadPublishers())
		{
			LOG_INFO_APP("[DoSetBurstNotification]: (Signal) reload publishers not right now cause it srtNotification in progress");
			return;
		}
		m_doLoadPublishers = false;
        m_PublishersStateMap.clear();
	}

	// update log for SetPublishers task
    if (m_LastSetPublishersLogUpdateTime.GetElapsedSec() >= m_nSetPublishersLogUpdatePeriod)
    {
        LogPublishersStateInDB();
        m_LastSetPublishersLogUpdateTime.MarkStartTime();
    }
}

void DoSetBurstNotification::ConfigurePublishers(int &noOfPublishers)
{
	PublisherInfoMAP_T::iterator i = m_PublishersMapStored.begin();
	DevicesManager::const_iterator_by_mac j = m_devices.BeginAllByMac();

	for (; i != m_PublishersMapStored.end(); ++i)
	{
		PublisherInfo& pubInfo = i->second;
		noOfPublishers += (pubInfo.hasNotification) ? 1 : 0;

		if ( pubInfo.hasNotification && 
			 pubInfo.noOfBurstMessagesThatFailedToConfigure == 0 &&
			 pubInfo.burstNoTotalCmd105 >= 0 &&
			 !pubInfo.reconfigBurst &&
			 !pubInfo.flushDelayedResponses)
		{	continue;
		}


		if (j == m_devices.EndAllByMac())
		{
			LOG_DEBUG_APP("[DoSetBurstNotification]: (Config) pub with mac = " << i->first << "not found in cache... Skipping check...");
			continue;
		}
		if (i->first == j->first)
		{
			//found a match...
		}
		else 
		{
			if (j->first < i->first)
			{
				do
				{
					++j;
					if (j == m_devices.EndAllByMac())
						break;
				}while (j->first < i->first);
				if (j == m_devices.EndAllByMac())
				{
					LOG_DEBUG_APP("[DoSetBurstNotification]: (Config) pub with mac = " << i->first << "not found in cache... Skipping check...");
					continue; // continue 'for'
				}
				if (i->first < j->first)
				{
					LOG_DEBUG_APP("[DoSetBurstNotification]: (Config) pub with mac = " << i->first << "not found in cache... Skipping check...");
					continue;
				}
				
				//found a match...
			}
			else 
			{
				LOG_DEBUG_APP("[DoSetBurstNotification]: (Config) pub with mac = " << i->first << "not found in cache... Skipping check...");
				continue;
			}
		}

		DevicePtr dev = j->second;

		if ( !dev->IsRegistered())
		{
			LOG_DEBUG_APP("[DoSetBurstNotification]: (Config) pub with mac = " << i->first << "not registered ... Skipping check...");
			
			if (dev->HasNotification() && !pubInfo.hasNotification)
			{
				noOfPublishers++;
				pubInfo.hasNotification = true; //no need to check it again
			}
			
			++j;
			continue;
		}

		pubInfo.hasNotification = dev->HasNotification();
		
		if( !dev->HasNotification() )
		{
			if (dev->IssueSetBurstNotificationCmd)
			{
				LOG_DEBUG_APP("[DoSetBurstNotification]: (Config) pub with mac = " << i->first << " has no subscriber notification, so send subscriber notification request ...");
					
				dev->SetPublisherInfo(pubInfo);
				dev->IssueSetBurstNotificationCmd = false;
				IssueSetBurstNotificationCmd(i->first, dev->id);
			}

			if ( dev->IssueSetBurstConfigurationCmd )
			{
				LOG_DEBUG_APP("[DoSetBurstNotification]: Setting burst notification for device with mac = " << i->first);

				dev->SetPublisherInfo(pubInfo);
				dev->IssueSetBurstConfigurationCmd = false;
				IssueSetBurstConfigurationCmd(i->first, dev->id);
			}
		}

		if (pubInfo.reconfigBurst)
		{
			if (dev->IssueSetBurstConfigurationCmd)
			{
				LOG_INFO_APP("[DoSetBurstNotification]: Setting burst configuration for device with mac = " << i->first);

				pubInfo.reconfigBurst = false;
				dev->SetPublisherInfo(pubInfo);
				dev->IssueSetBurstConfigurationCmd = false;
				IssueSetBurstConfigurationCmd(i->first, dev->id);
			}
		}

		if (pubInfo.flushDelayedResponses)
		{
			if (dev->IssueSetBurstConfigurationCmd)
			{
				LOG_INFO_APP("[DoSetBurstNotification]: Setting burst configuration with flush delayed responses for device with mac = " << i->first);

				dev->SetPublisherInfo(pubInfo);
				dev->IssueSetBurstConfigurationCmd = false;
				IssueSetBurstConfigurationCmd(i->first, dev->id);
			}
		}

		if ( (pubInfo.noOfBurstMessagesThatFailedToConfigure != 0 || pubInfo.burstNoTotalCmd105 < 0) && (dev->IssueSetBurstConfigurationCmd) )
		{
			//use retries and then switch to the other case
			if (pubInfo.burstSetConfigShortDelayRetries > 0)
			{
				pubInfo.burstSetConfigShortDelayRetries--;
				LOG_INFO_APP("[DoSetBurstNotification] - Setting burst configuration for device with mac = " << i->first << " for a nb of "
					<< pubInfo.noOfBurstMessagesThatFailedToConfigure << " burst messages that failed to configure, with left_retry_count="
					<< pubInfo.burstSetConfigShortDelayRetries);

				dev->SetPublisherInfo(pubInfo);
				dev->IssueSetBurstConfigurationCmd = false;

				IssueSetBurstConfigurationCmd(i->first, dev->id);
			}
			else
			{
				if (pubInfo.lastConfigurationTime.GetElapsedSec() >= m_nBurstSetConfigRetryPeriod)
				{	
					dev->SetPublisherInfo(pubInfo);

					LOG_INFO_APP("[DoSetBurstNotification] - Setting burst configuration for device with mac = " << i->first << " for a nb of "
						<< pubInfo.noOfBurstMessagesThatFailedToConfigure << " burst messages that failed to configure");

					dev->IssueSetBurstConfigurationCmd = false;
					IssueSetBurstConfigurationCmd(i->first, dev->id);
				}
			}
		}
		++j;
	}
}

//bursts
void DoSetBurstNotification::UpdatePublishersCache(MAC& p_rMac, PublisherInfo& p_rPubInfo)
{
	LOG_DEBUG_APP("[DoSetBurstNotification]: Updating publishers cache for Mac " << p_rMac );

	PublisherInfoMAP_T::iterator it = m_PublishersMapStored.find(p_rMac);
	if (it == m_PublishersMapStored.end())
	{
		LOG_ERROR_APP("[DoSetBurstNotification]: Cannot update publishers cache for Mac " << p_rMac );
		return;
	}
	it->second = p_rPubInfo;
}

void DoSetBurstNotification::UpdatePublisherState(const MAC& p_rMac, int p_nBurst, SetPublisherState p_eState, SetPublisherError p_eError, std::string p_sMessage)
{
    LOG_INFO_APP("[DoSetBurstNotification]: UpdatePublisherState - Burst=" << (int)p_nBurst << ", "
        << "State='" << GetSetPublishStateText(p_eState) << "(" << (int)p_eState << ")', "
        << "Error='" << GetSetPublishErrorText(p_eError) << "(" << (int)p_eError << ")', "
        << "Message='" << p_sMessage << "' for Mac " << p_rMac );

    if (p_eState != SETPUBLISHER_STATE_UNDEFINED)
    {
        m_PublishersStateMap[p_rMac].stateChanged = true;
        m_PublishersStateMap[p_rMac].setPublisherState = p_eState;
    }
    if (p_eError != SETPUBLISHER_ERROR_UNDEFINED)
    {
        m_PublishersStateMap[p_rMac].stateChanged = true;
        if (p_nBurst > -1)
        {
            PublisherState::BurstErrorLST_T::iterator itBurst = m_PublishersStateMap[p_rMac].burstErrorList.begin();
            bool found = false;
            for (; itBurst != m_PublishersStateMap[p_rMac].burstErrorList.end(); ++itBurst)
            {
                if (itBurst->burstNo == p_nBurst)
                {
                    itBurst->setPublisherError = p_eError;
                    itBurst->setPublisherMessage = p_sMessage;
                    found = true;
                }
            }
            if (!found)
                m_PublishersStateMap[p_rMac].burstErrorList.insert(itBurst, BurstError(p_nBurst, p_eError, p_sMessage));
        }
        else
        {
            m_PublishersStateMap[p_rMac].setPublisherError = p_eError;
            m_PublishersStateMap[p_rMac].setPublisherMessage = p_sMessage;
        }
    }
}

void DoSetBurstNotification::LogPublishersStateInDB()
{
    m_devices.UpdateSetPublishersLog(m_PublishersStateMap);
}

//gw notifications
void DoSetBurstNotification::HandleGWConnect(const std::string& host, int port)
{
	LOG_INFO_APP("[DoSetBurstNotification]:Gateway reconnected ...");
	m_isGWConnected = true;
	m_isGWReconnected = true;

}
void DoSetBurstNotification::HandleGWDisconnect()
{
	LOG_WARN_APP("[DoSetBurstNotification]: Gateway disconnected!");
	m_isGWConnected = false;
}

}//namespace hostapp
}//namespace hart7

