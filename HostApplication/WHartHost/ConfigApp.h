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

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <nlib/exception.h>
#include <nlib/datetime.h>
#include <boost/cstdint.hpp>





namespace hart7 {

class InvalidConfigAppException : public nlib::Exception
{
public:
	InvalidConfigAppException(const std::string& msg) :
		nlib::Exception(msg)
	{
	}
};

class ConfigApp
{
public:
	ConfigApp();

	void Load();

private:
	std::string appVersion;
	
	//files
	std::string strConfigFile;
	std::string strLogConfigFile;
	std::string strPubConfigFile;
	
	
	//sqlite3 database
	std::string databasePath;
	
	//mysql database
	std::string databaseServer;
	std::string databaseName;
	std::string databaseUser;
	std::string databasePassword; 
		
	//database 
	int databaseTimeout;
	int databaseVacuumPeriodMinutes;
	int databaseRemoveEntriesCheckPeriodMinutes;
	int databaseRemoveOlderEntriesThanMinutes;
	int databaseRemoveEntriesMoreThanMaxCount;

	//gateway
	std::string gatewayHost;
	int gatewayPort;
	int localPort;
	int allowSameCmdOnDev;
	
	//app
	int threadCheckPeriod;
	int commandsTimeout;	
	int topologyPool;
	
	//readings
	int m_nReadingsSavePeriod;
	int m_nMaxReadingsNoPerTransaction;

	//burst counters
	int m_nBurstCountersUpdatePeriod; /*seconds*/
	int m_nPublishPeriodToleranceThreshold; /*seconds*/
	
	//burst configuration
	int m_nBurstSetConfigRetryPeriod; /*seconds*/
	int m_nBurstSetConfigShortDelayRetries; /*seconds*/
	int m_nDiscoveryBurstConfigPeriod; /* seconds */
	int m_nDiscoveryRequestDelayInterval; /* seconds */
	int m_nSetPublishersLogUpdatePeriod; /* seconds */

	//reports
	int m_nReportsCacheFlushPeriod; /*seconds*/

public:
	const std::string ApplicationVersion() const { return appVersion; }
	
	//sqlite3 database
	const std::string DatabaseFilePath() const { return databasePath; }
	
	//mysql database
	const std::string DatabaseServer() const { return databaseServer; }
	const std::string DatabaseName() const { return databaseName; }
	const std::string DatabaseUser() const { return databaseUser; }
	const std::string DatabasePassword() const { return databasePassword; }
	
	//database
	const int DatabaseTimeout() const { return databaseTimeout; }
	const int DatabaseVacuumPeriodM() const { return databaseVacuumPeriodMinutes; }
	const int DatabaseRemoveEntriesPeriodM() const { return databaseRemoveEntriesCheckPeriodMinutes; }
	const int DatabaseRemoveEntriesOlderThanM() const { return databaseRemoveOlderEntriesThanMinutes; }
	const int DatabaseRemoveEntriesMoreThanMaxCount() const { return databaseRemoveEntriesMoreThanMaxCount; }
	const int ConnectToDatabaseRetryInterval() const { return 60; /*seconds*/ }

	//gateway
	const std::string& GatewayHostIP() const { return gatewayHost; }
	int GatewayHostPort() const { return gatewayPort; }
	int GatewayLocalPort() const { return localPort; }
	int GatewayAllowCmd() const { return allowSameCmdOnDev;}

	//app
	const int InternalTasksPeriod() const { return threadCheckPeriod; }
	const nlib::TimeSpan CommandsTimeout() const { return nlib::util::seconds(commandsTimeout); }
	const nlib::TimeSpan TopologyPeriod() const { return nlib::util::seconds(topologyPool); }

	//readings
	const int GetReadingsSavePeriod() const { return m_nReadingsSavePeriod; }
	const int GetMaxReadingsNoPerTransaction() const { return m_nMaxReadingsNoPerTransaction; }

	//publishers
	const std::string& PublishersFile() const { return strPubConfigFile; }

	//burst counters
	int GetBurstCountersUpdatePeriod() { return m_nBurstCountersUpdatePeriod; }
	int GetPublishPeriodToleranceThreshold() { return m_nPublishPeriodToleranceThreshold; }

	//burst configuration
	int GetBurstSetConfigurationRetryPeriod() { return m_nBurstSetConfigRetryPeriod; }
	int GetBurstSetConfigurationShortDelayRetries() { return m_nBurstSetConfigShortDelayRetries; }
    int GetDiscoveryBurstConfigPeriod() { return m_nDiscoveryBurstConfigPeriod; }
    int GetDiscoveryRequestDelayInterval() { return m_nDiscoveryRequestDelayInterval; }
    int GetSetPublishersLogUpdatePeriod() { return m_nSetPublishersLogUpdatePeriod; }

	//reports
	int GetReportsCacheFlushPeriod() { return m_nReportsCacheFlushPeriod; }
};

} //namespace hart7

#endif /*SETTINGS_H_*/
