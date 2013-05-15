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

#include <nlib/log.h>

#include <WHartHost/periodictasks/DoSetBurstCounters.h>

namespace hart7 {
namespace hostapp {

void DoSetBurstCounters::HandleIncomingNotification(int p_nDeviceId, const CMicroSec& p_rRecvTime, C839_ChangeNotification_Resp& p_rNotifData)
{
    BurstsCache::iterator cacheIt = m_oCache.find(p_nDeviceId);

    LOG_DEBUG_APP("[DoSetBurstCounters]: HandleIncomingNotification for devID=" << p_nDeviceId);

    if (cacheIt == m_oCache.end())
    {
        LOG_WARN_APP("[DoSetBurstCounters]: Cannot handle a notification from the unknown deviceId=" << p_nDeviceId);
        return;
    }

    DevicePtr dev = m_rDevices.FindDevice(p_nDeviceId);
    if (dev == 0)
        return;

    for (int i = 0 ; i < p_rNotifData.ChangeNotificationNo && i < MaxChangeNotifications ; i++)
    {
        LOG_DEBUG_APP("[DoSetBurstCounters]: HandleIncomingNotification - ChangeNotifications[" << i << "]=" << p_rNotifData.ChangeNotifications[i] << " for devID=" << p_nDeviceId);

        BurstCounterEntries::iterator entriesIt = cacheIt->second.find(p_rNotifData.ChangeNotifications[i]);
        if (entriesIt == cacheIt->second.end())
        {
            LOG_WARN_APP("[DoSetBurstCounters]: HandleIncomingNotification - Cannot find cmd="
                    << p_rNotifData.ChangeNotifications[i] << " in the change notification for deviceId=" << p_nDeviceId);
            continue;
        }
        for(std::list<BurstCounterEntry>::iterator it = entriesIt->second.begin(); it != entriesIt->second.end(); ++it)
        {

            if(it->GetReceivedCounter() == 0)
            {
                PublisherInfo& pubInfo = dev->GetPublisherInfo();
                PublisherInfo::BurstMessageStateMap::const_iterator statesMapIt = pubInfo.burstMessagesState.find(it->GetBurstMessage().burstMessage);

                if (statesMapIt == pubInfo.burstMessagesState.end()/*NOT_SET state*/ || statesMapIt->second != BURST_STATE_SET)
                    continue;
            }
            LOG_DEBUG_APP("[DoSetBurstCounters]: HandleIncomingNotification - RegisterReceivedBurst(" << p_rRecvTime.GetElapsedTimeStr() << "), for devID=" << p_nDeviceId);
            it->RegisterReceivedBurst(p_rRecvTime);
        }
    }
}

void DoSetBurstCounters::HandlePubDeviceDeletion(int p_nDeviceId)
{
    LOG_DEBUG_APP("[DoSetBurstCounters]: HandlePubDeviceDeletion - deleted from cache devID=" << p_nDeviceId);
    m_oCache.erase(p_nDeviceId);
}

void DoSetBurstCounters::HandlePubDeviceBurstsChange(int p_nDeviceId,
        std::list<BurstMessage>& p_oAddedBursts,
        std::list<BurstMessage>& p_oDeletedBursts,
        std::list<BurstMessage>& p_oChangedBursts)
{
    BurstCounterEntries& burstEntries = m_oCache[p_nDeviceId];

    for (std::list<BurstMessage>::iterator i = p_oDeletedBursts.begin() ; i != p_oDeletedBursts.end() ; ++i)
    {
        BurstCounterEntries::iterator burst = burstEntries.find(i->cmdNo);
        if (burst == burstEntries.end())
        {
            LOG_WARN_APP("[DoSetBurstCounters]: HandlePubDeviceBurstsChange - delete burstMessage=" << (int)i->burstMessage << " with cmdNo=" << i->cmdNo
                            << "for devID=" << p_nDeviceId << " was not possible cause there is no burst in cache");
            continue;
        }
        std::list<BurstCounterEntry> &l = burst->second;
        for(std::list<BurstCounterEntry>::iterator it=l.begin();it!=l.end();)
        {
            if((it->GetBurstMessage()).burstMessage == i->burstMessage)
            {
                it = l.erase(it);
                LOG_DEBUG_APP("[DoSetBurstCounters]: HandlePubDeviceBurstsChange - DELETE burstMessage=" << (int)i->burstMessage
                             << " and cmdNo=" << i->cmdNo << " for devID=" << p_nDeviceId);
                continue;
            }
            ++it;
        }
    }

    for (std::list<BurstMessage>::iterator i = p_oChangedBursts.begin() ; i != p_oChangedBursts.end() ; ++i)
    {
        BurstCounterEntries::iterator entryIt = burstEntries.find(i->cmdNo);
        if ( entryIt == burstEntries.end() )
        {   LOG_WARN_APP("[DoSetBurstCounters]: Invalid burst entry specified in the changed bursts list");
            continue;
        }
        std::list<BurstCounterEntry> &l = entryIt->second;
        bool bmGasit = false;
        for(std::list<BurstCounterEntry>::iterator it=l.begin();it!=l.end();++it)
        {
            if(it->GetBurstMessage().burstMessage == (int)(i->burstMessage))
            {
                it->GetBurstMessage() = (*i);
                LOG_DEBUG_APP("[DoSetBurstCounters]: HandlePubDeviceBurstsChange - UPDATE burstMessage=" << (int)i->burstMessage
                             << " and cmdNo=" << i->cmdNo << " for devID=" << p_nDeviceId);
                bmGasit = true;
            }
        }
        if(!bmGasit)
            LOG_WARN_APP("[DoSetBurstCounters]: HandlePubDeviceBurstsChange - Invalid burst entry specified in the changed bursts list");
    }

    for (std::list<BurstMessage>::iterator i = p_oAddedBursts.begin() ; i != p_oAddedBursts.end() ; ++i)
    {
        BurstCounterEntries::iterator entryIt = burstEntries.find(i->cmdNo);
        if ( entryIt == burstEntries.end() )
        {//there is no other bm set for this cmdNo
            std::list<BurstCounterEntry> l;
            l.push_back(BurstCounterEntry(*i));
            burstEntries.insert(BurstCounterEntries::value_type(i->cmdNo, l));
            LOG_DEBUG_APP("[DoSetBurstCounters]: HandlePubDeviceBurstsChange - INSERT burstMessage=" << (int)i->burstMessage
                         << " and cmdNo=" << i->cmdNo << " for devID=" << p_nDeviceId);
        }
        else
        {//there is another bm set for this comdNo
            entryIt->second.push_back(BurstCounterEntry(*i));
            LOG_DEBUG_APP("[DoSetBurstCounters]: HandlePubDeviceBurstsChange - INSERT / UPDATE burstMessage=" << (int)i->burstMessage
                         << " and cmdNo=" << i->cmdNo << " for devID=" << p_nDeviceId);
        }
    }
}

void DoSetBurstCounters::DoTask(int periodTime/*ms*/)
{
    LOG_INFO_APP("[DoSetBurstCounters]: DoTask - task runs with period=" << periodTime << " ms");

    if (!m_isGWConnected)
    {
        LOG_INFO_APP("[DoSetBurstCounters]: DoTask - gw not connected! skip task...");
        return;
    }

//  LOG_DEBUG_APP("[DoSetBurstCounters]: - m_oLastDBUpdateTime.GetElapsedSec()=" << m_oLastDBUpdateTime.GetElapsedSec()  << ", m_nDbUpdatePeriod=" << m_nDbUpdatePeriod);

    bool dumpToDb = false;
    if (m_oLastDBUpdateTime.GetElapsedSec() >= m_nDbUpdatePeriod)
        dumpToDb = true;

    try{
        if (dumpToDb)
            m_rDevices.BeginUpdateBurstMessageCounter();

        for(BurstsCache::iterator deviceMapIt = m_oCache.begin() ; deviceMapIt != m_oCache.end() ; ++deviceMapIt)
        {
            int devId = deviceMapIt->first;
            for(BurstCounterEntries::iterator burstEntriesIt = deviceMapIt->second.begin() ; burstEntriesIt != deviceMapIt->second.end() ; ++burstEntriesIt )
            {
                for(std::list<BurstCounterEntry>::iterator it = burstEntriesIt->second.begin(); it != burstEntriesIt->second.end(); ++it)
                {
                    //we don't count missed bursts until the first burst is received
                    if ((*it).GetReceivedCounter() == 0)
                    {
                        LOG_DEBUG_APP("[DoSetBurstCounters]: DoTask -  NEW BurstMessage[devId=" << devId << "] = {Burst=" << (int)it->GetBurstMessage().burstMessage
                                << ", Command=" << (int)it->GetBurstMessage().cmdNo << ", LastUpdate=" << it->GetLastUpdateTime().GetElapsedTimeStr()
                                << ", LastEvent=" << it->GetLastEventTime().GetElapsedTimeStr() << ", Received=" << it->GetReceivedCounter() << ", Missed=" << it->GetMissedCounter() << "}");
                        continue;
                    }

                    DevicePtr dev = m_rDevices.FindDevice(devId);
                    if (dev == 0)
                        continue;

                    PublisherInfo::BurstMessageStateMap::const_iterator statesMapIt = dev->GetPublisherInfo().burstMessagesState.find(it->GetBurstMessage().burstMessage);
                    //if no publish received within a "threshold" interval, mark as missed
                    if (statesMapIt->second == BURST_STATE_SET &&
                        (*it).GetLastEventTime().GetElapsedSec() > (*it).GetBurstMessage().updatePeriod + m_nPublishPeriodToleranceThreshold)
                    {
                        CMicroSec now;
                        (*it).RegisterMissedBurst(now);
                        if (dev->GetPublishStatus() != Device::PS_NO_DATA)
                            m_rDevices.UpdatePublishFlag(devId, Device::PS_STALE_DATA);
                        LOG_WARN_APP("[DoSetBurstCounters]: DoTask - MISSED BurstMessage[devId=" << devId << "] = {Burst=" << (int)it->GetBurstMessage().burstMessage
                                << ", Command=" << (int)it->GetBurstMessage().cmdNo << ", LastUpdate=" << it->GetLastUpdateTime().GetElapsedTimeStr()
                                << ", LastEvent=" << it->GetLastEventTime().GetElapsedTimeStr() << ", Received=" << it->GetReceivedCounter() << ", Missed=" << it->GetMissedCounter() << "}");
                    }
                    else
                    {
                        LOG_DEBUG_APP("[DoSetBurstCounters]: DoTask - RECEIVED BurstMessage[devId=" << devId << "] = {Burst=" << (int)it->GetBurstMessage().burstMessage
                                << ", Command=" << (int)it->GetBurstMessage().cmdNo << ", LastUpdate=" << it->GetLastUpdateTime().GetElapsedTimeStr()
                                << ", LastEvent=" << it->GetLastEventTime().GetElapsedTimeStr() << ", Received=" << it->GetReceivedCounter() << ", Missed=" << it->GetMissedCounter() << "}");
                    }
                    //dump to db
                    if (dumpToDb)
                    {
                        LOG_DEBUG_APP("[DoSetBurstCounters]: DoTask - UpdateBurstMessageCounter(DeviceID=" << devId
                                << ", BurstMessage=" << (int)it->GetBurstMessage().burstMessage << ", LastUpdate=" << it->GetLastUpdateTime().GetElapsedTimeStr()
                                << ", Received=" << it->GetReceivedCounter() << ", Missed=" << it->GetMissedCounter() << ")");
                        m_rDevices.UpdateBurstMessageCounter(devId, (*it).GetBurstMessage(), (*it).GetLastUpdateTime(), (*it).GetReceivedCounter(), (*it).GetMissedCounter());
                    }
                }
            }
        }

        if (dumpToDb)
        {
            m_rDevices.EndUpdateBurstMessageCounter(true);
            CMicroSec now;
            m_oLastDBUpdateTime = now;
        }
    }
    catch(...)
    {
        if (dumpToDb)
            m_rDevices.EndUpdateBurstMessageCounter(false);
    }
}

void DoSetBurstCounters::HandleGWConnect(const std::string& host, int port)
{
    LOG_INFO_APP("[DoSetBurstCounters]:Gateway reconnected ...");
    m_isGWConnected = true;
}

void DoSetBurstCounters::HandleGWDisconnect()
{
    LOG_WARN_APP("[DoSetBurstCounters]: Gateway disconnected!");
    m_isGWConnected = false;
}

}
}
