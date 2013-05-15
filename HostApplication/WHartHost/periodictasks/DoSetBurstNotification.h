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


#ifndef _DOSETBURSTNOTIFICATION_H_
#define _DOSETBURSTNOTIFICATION_H_


#include <WHartHost/database/DBCommandsManager.h>
#include <WHartHost/database/DevicesManager.h>
#include <WHartHost/cmdsprocessor/CommandsProcessor.h>
#include <WHartHost/PublisherConf.h>
#include <WHartHost/Utils.h>
#include <WHartHost/model/PublisherInfo.h>
#include <Shared/MicroSec.h>

#include <nlib/log.h>

#include <boost/format.hpp>


namespace hart7 {
namespace hostapp {

/*
 *	It subscribes for burst notifications
 */
class DoSetBurstNotification
{
	
public:
	DoSetBurstNotification(DBCommandsManager& commands, DevicesManager& devices,
		CommandsProcessor& processor, const std::string &strPubConfigFile,  int p_nBurstSetConfigRetryPeriod,
		int p_nBurstSetConfigShortDelayRetries, int p_nPublishPeriodToleranceThreshold, int p_nSetPublishersLogUpdatePeriod) :
		m_dbCommands(commands),
		m_devices(devices),
		m_processor(processor),
		m_strPubConfigFile(strPubConfigFile),
		m_nBurstSetConfigRetryPeriod(p_nBurstSetConfigRetryPeriod), 
		m_nBurstSetConfigShortDelayRetries(p_nBurstSetConfigShortDelayRetries),
		m_nPublishPeriodToleranceThreshold(p_nPublishPeriodToleranceThreshold),
		m_nSetPublishersLogUpdatePeriod(p_nSetPublishersLogUpdatePeriod)
	{
		m_isGWConnected = false;
		m_isGWReconnected = false;
		
		m_taskInit = true;
		LOG_INFO_APP("DoSetBurstNotification started.");
	}

//send commands
private:
	void IssueSetBurstNotificationCmd(const MAC& targetDevice, int devID);
	void IssueUnsetBurstNotificationCmd(const MAC& targetDevice, int devID);
	void UnsubcribeForNotification(const MAC& targetDevice);

	void IssueSetBurstConfigurationCmd(const MAC& targetDevice, int devID);

//with signal
private:
	bool IsSubscribeNotifSent();
	void RefreshPublishers();
	void ComputeNewPublishers();
	bool ReloadPublishers();

public:
	void DoLoadPublishers() { m_doLoadPublishers = true; }

//do subscribe
public:
	void DoTask(int periodTime/*ms*/);
private:
	void ConfigurePublishers(int &noOfPublishers);
	

// publishing
public:
	hostapp::PublisherInfoMAP_T& GetPublishersMapStored() { return m_PublishersMapStored; }

	void UpdatePublishersCache(MAC& p_rMac, PublisherInfo& p_rPubInfo);
	void UpdatePublisherState(const MAC& p_rMac, int p_nBurst, SetPublisherState p_eState, SetPublisherError p_eError, std::string p_sMessage);

private:
	void LogPublishersStateInDB();

//gw notifications
public:
	void HandleGWConnect(const std::string& host, int port);
	void HandleGWDisconnect();

//
private:
	bool m_doLoadPublishers;

private:
	bool m_isGWConnected;
	bool m_isGWReconnected;

private:
	DBCommandsManager&	m_dbCommands;
	DevicesManager&		m_devices;
	CommandsProcessor&  m_processor;

private:
	const std::string m_strPubConfigFile;
	double	m_nBurstSetConfigRetryPeriod;

	int m_nBurstSetConfigShortDelayRetries; /* seconds */
	int m_nPublishPeriodToleranceThreshold; /* seconds */


	hostapp::PublisherInfoMAP_T	m_PublishersMapStored;
	hostapp::PublisherInfoMAP_T	m_PublishersMapLoaded;

	bool m_taskInit;

	// log for SetPublisher task
	hostapp::PublisherStateMAP_T m_PublishersStateMap;
	int m_nSetPublishersLogUpdatePeriod;    /* seconds */
	CMicroSec m_LastSetPublishersLogUpdateTime;
};

}//namespace hostapp
}//namespace hart7



#endif
