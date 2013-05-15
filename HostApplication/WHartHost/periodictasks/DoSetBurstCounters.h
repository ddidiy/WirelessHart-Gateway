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

#ifndef _DOSETBURSTCOUNTERS_H__
#define _DOSETBURSTCOUNTERS_H__

#include <WHartHost/database/DevicesManager.h>
#include <WHartHost/ConfigApp.h>

#include <nlib/log.h>



namespace hart7 {
namespace hostapp {


/*
 * it updates the BurstCounters db table
 */
class DoSetBurstCounters
{
	LOG_DEF("hart7.hostapp.DoSetBurstCounters");

private:
	class BurstCounterEntry
	{
	public:
		BurstCounterEntry(BurstMessage& p_rBurstMessage): m_oBurstMessage(p_rBurstMessage), m_nReceived(0), m_nMissed(0) {}

	private:
		BurstMessage m_oBurstMessage;
		CMicroSec m_oLastUpdate;
		CMicroSec m_oLastEvent; // last time we recorded an event (missed or received)
		int m_nReceived;
		int m_nMissed;

	public:

		BurstMessage& GetBurstMessage() { return m_oBurstMessage;}

		int GetReceivedCounter() { return m_nReceived;}

		int GetMissedCounter() { return m_nMissed;}

		const CMicroSec& GetLastUpdateTime() { return m_oLastUpdate;}
		const CMicroSec& GetLastEventTime() { return m_oLastEvent;}

		void RegisterReceivedBurst(const CMicroSec& p_rTime)
		{	//if overflow reset both counters
			if (++m_nReceived == 0)
			{	m_nReceived = m_nMissed = 0;
			}
			m_oLastUpdate = p_rTime;
			m_oLastEvent = p_rTime;
		}

		void RegisterMissedBurst(const CMicroSec& p_rTime)
		{	//if overflow reset both counters
			if (++m_nMissed == 0)
			{	m_nReceived = m_nMissed = 0;
			}
			m_oLastEvent = p_rTime;
		}
		
		void SetBurstMessage(BurstMessage p_oBurstMessage) {m_oBurstMessage = p_oBurstMessage;}
	};

public:
	//map that stores the last time we received a notification for our command	
	typedef std::map<int,std::list<BurstCounterEntry> > BurstCounterEntries;
	//map that stores the cmds recv. time for each device
	typedef std::map<int, BurstCounterEntries> BurstsCache;

public:
	DoSetBurstCounters(DevicesManager& p_rDevices, int p_nPublishPeriodToleranceThreshold, int p_nDbUpdatePeriod) :
		m_rDevices(p_rDevices), 
		m_nPublishPeriodToleranceThreshold(p_nPublishPeriodToleranceThreshold),
		m_nDbUpdatePeriod(p_nDbUpdatePeriod)
	{
		m_isGWConnected = false;
		LOG_INFO_APP("DoSetBurstCounters started");
	}

public:
	void DoTask(int periodTime/*ms*/);

public:
	void HandleGWConnect(const std::string& host, int port);
	void HandleGWDisconnect();
	

//burst notification callbacks
public:
	void HandleIncomingNotification(int p_nDeviceId, 
		const CMicroSec& p_rRecvTime, 
		C839_ChangeNotification_Resp& p_rNotifData);

	void HandlePubDeviceDeletion(int p_nDeviceId);

	void HandlePubDeviceBurstsChange(int p_nDeviceId,
		std::list<BurstMessage>& p_oAddedBursts,
		std::list<BurstMessage>& p_oDeletedBursts,
		std::list<BurstMessage>& p_oChangedBursts);

private:
	DevicesManager& m_rDevices;
	int m_nPublishPeriodToleranceThreshold;
	int m_nDbUpdatePeriod;

private:
	BurstsCache m_oCache;
	CMicroSec m_oLastDBUpdateTime;
	bool m_isGWConnected;
};

} //namespace hostapp
} //namespace hart7

#endif /*_DOSETBURSTCOUNTERS_H__*/
