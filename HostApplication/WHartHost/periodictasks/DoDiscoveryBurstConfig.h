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


#ifndef _DOAUTODETECTBURSTMESSAGES_H_
#define _DOAUTODETECTBURSTMESSAGES_H_


#include <WHartHost/database/DBCommandsManager.h>
#include <WHartHost/database/DevicesManager.h>
#include <WHartHost/cmdsprocessor/CommandsProcessor.h>
#include <WHartHost/PublisherConf.h>
#include <WHartHost/model/PublisherInfo.h>


#include <nlib/log.h>

#include <boost/format.hpp>


namespace hart7 {
namespace hostapp {


/*
 *	It detects the new burst messages
 */
class DoDiscoveryBurstConfig
{

public:
    DoDiscoveryBurstConfig(DBCommandsManager& commands, DevicesManager& devices, CommandsProcessor& processor,
                      hostapp::PublisherInfoMAP_T& storedPublishers, const std::string& strPubConfigFile,
                      int discoveryBurstConfigPeriod, int discoveryRequestDelayInterval, int burstSetConfigRetryPeriod) :
		m_dbCommands(commands),
		m_devices(devices),
		m_processor(processor),
		m_PublishersMapStored(storedPublishers),
        m_strPubConfigFile(strPubConfigFile),
		m_nDiscoveryBurstConfigPeriod(discoveryBurstConfigPeriod),
		m_nDiscoveryRequestDelayInterval(discoveryRequestDelayInterval),
		m_nBurstSetConfigRetryPeriod(burstSetConfigRetryPeriod)
	{
		m_isGWConnected = false;
		m_taskInit = true;
		LOG_INFO_APP("DoDiscoveryBurstConfig started.");
	}

//send commands
private:
	void IssueDiscoveryBurstConfigCmd(const MAC& targetDevice, int devID);
    std::string GetPublisherString(PublisherInfo& publiserInfo);
    std::string GetPublishersString(PublisherInfoMAP_T& publishersMap);

//do subscribe
public:
	void DoTask(int periodTime/*ms*/);
    void UpdateStoredPublishers(MAC& p_rMac, PublisherInfo& p_rPubInfo, int issueCase);
	void DoLoadPublishers() { m_doLoadPublishers = true; }

//gw notifications
public:
	void HandleGWConnect(const std::string& host, int port);
	void HandleGWDisconnect();

private:
	bool m_isGWConnected;

private:
	DBCommandsManager&	m_dbCommands;
	DevicesManager&		m_devices;
	CommandsProcessor&  m_processor;
	PublisherInfoMAP_T& m_PublishersMapStored;

private:
	std::string    m_strPubConfigFile;
	int            m_nDiscoveryBurstConfigPeriod; /* seconds */
	int            m_nDiscoveryRequestDelayInterval; /* seconds */
	int            m_nBurstSetConfigRetryPeriod; /* seconds */
	CMicroSec      m_oLastUpdateTime;
	bool 		   m_taskInit;
	bool 		   m_doLoadPublishers;

	DevicesAutodetectMAP_T m_oDevicesStatus;  // list of processed device IDs
};

}//namespace hostapp
}//namespace hart7



#endif
