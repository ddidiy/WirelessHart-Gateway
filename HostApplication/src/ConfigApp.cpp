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

#include <WHartHost/ConfigApp.h>
#include <WHartHost/Version.h>

#include <fstream>
#include <iostream>
#include <boost/program_options.hpp>
#include <boost/format.hpp>

#include <nlib/log.h>

#include <WHartHost/MyLog.h>
#include <Shared/AnPaths.h>

#include "Config.h"

#ifdef HW_VR900
# define MHOST_CONF	NIVIS_PROFILE "Monitor_Host.conf"
# define MHOST_LOG_INI	NIVIS_PROFILE "Monitor_Host_Log.ini"
# define MHOST_DB3 	NIVIS_TMP "Monitor_Host.db3"
# define MHOST_PUBLISH  NIVIS_PROFILE "Monitor_Host_Publisher.conf"
#else
# define MHOST_CONF	"etc/Monitor_Host.conf"
# define MHOST_LOG_INI	"etc/Monitor_Host_Log.ini"
# define MHOST_DB3 	"etc/Monitor_Host.db3"
# define MHOST_PUBLISH  "etc/Monitor_Host_Publisher.conf"
#endif


namespace hart7 {

ConfigApp::ConfigApp()
{

#ifdef VERSION
	if (!strcmp(VERSION, "local"))
		appVersion = M_H_VERSION;
	else
		appVersion = VERSION;
#else
		appVersion = M_H_VERSION;
#endif
	
	// files
	strConfigFile = MHOST_CONF;
	strLogConfigFile = MHOST_LOG_INI;
	strPubConfigFile = MHOST_PUBLISH;

	// sqlite3 database
	databasePath = MHOST_DB3;

	// mysql database
	databaseServer = "127.0.0.1"; //localhost
	databaseName = "Monitor_Host";
	databaseUser = "root";
	databasePassword = "root";

	// databse
	databaseTimeout = 40;
	databaseVacuumPeriodMinutes = 0;
	databaseRemoveEntriesCheckPeriodMinutes = 4;
	databaseRemoveOlderEntriesThanMinutes = 48;
	databaseRemoveEntriesMoreThanMaxCount = 1000;

	// gateway
	gatewayHost = "127.0.0.1";
	gatewayPort = 4900;
	localPort = 9000;
	
	// app
	threadCheckPeriod = 2;
	commandsTimeout = 60;
	topologyPool = 10;

	// readings
	m_nReadingsSavePeriod = 10;
	m_nMaxReadingsNoPerTransaction = 200;

	// publishers
	m_nBurstSetConfigRetryPeriod = 1800; // seconds
	m_nDiscoveryBurstConfigPeriod = 60; // seconds
    m_nDiscoveryRequestDelayInterval = 300; // seconds
    m_nSetPublishersLogUpdatePeriod = 30; // seconds
}

void ConfigApp::Load()
{

	CConfigExt oConf ;
	if ( ! oConf.Load(strConfigFile.c_str()) )
	{
		THROW_EXCEPTION1(InvalidConfigAppException
				,boost::str( boost::format("Unable to load config file[%1%]\n")%strConfigFile )
			);
	}

	//files
	oConf.GetVar( "LogConfigPath", strLogConfigFile, MHOST_LOG_INI );
	oConf.GetVar( "PubConfigPath", strPubConfigFile, "" );

	// sqlite3 database
	oConf.GetVar( "DatabasePath", databasePath,   MHOST_DB3 );

	// mysql database
	oConf.GetVar( "DatabaseServer", databaseServer, "127.0.0.1" );
	oConf.GetVar( "DatabaseName", databaseName, "Monitor_Host" );
	oConf.GetVar( "DatabaseUser", databaseUser, "root" );
	oConf.GetVar( "DatabasePassword", databasePassword, "" );

	// database
	oConf.GetVar( "DatabaseTimeout", databaseTimeout, 10 );
	oConf.GetVar( "DatabaseVacuumPeriodMinutes", databaseVacuumPeriodMinutes, 30 );
	oConf.GetVar( "DatabaseRemoveEntriesCheckPeriod", databaseRemoveEntriesCheckPeriodMinutes, 10 );
	oConf.GetVar( "DatabaseRemoveEntriesOlderThanMinutes", databaseRemoveOlderEntriesThanMinutes, 30 );
	oConf.GetVar( "DatabaseRemoveEntriesMoreThanMaxCount", databaseRemoveEntriesMoreThanMaxCount, 1000);

	// gateway
	oConf.GetVar( "GatewayHost", gatewayHost, "127.0.0.1" );
	oConf.GetVar( "GatewayPort", gatewayPort, 4900 );
	oConf.GetVar( "LocalPort", localPort, 9000 );
	oConf.GetVar( "AllowSameCmdOnDev", allowSameCmdOnDev, 0);
	
	// app
	oConf.GetVar( "CommandsCheckPeriod", threadCheckPeriod, 2 );
	oConf.GetVar( "CommandsTimeout", commandsTimeout, 60 );
	oConf.GetVar( "TopologyPooling", topologyPool, 500 );
	
	// readings
	oConf.GetVar( "ReadingsSavePeriod", m_nReadingsSavePeriod, 10 ); // ms
	oConf.GetVar( "MaxReadingsNoPerTransaction", m_nMaxReadingsNoPerTransaction, 200 );
	
	// burst counters
	oConf.GetVar( "BurstCountersUpdatePeriod", m_nBurstCountersUpdatePeriod, 2);
	oConf.GetVar( "PublishPeriodToleranceThreshold", m_nPublishPeriodToleranceThreshold, 5);

	// burst configuration
	oConf.GetVar( "BurstSetConfigRetryPeriod", m_nBurstSetConfigRetryPeriod, 1800);
	oConf.GetVar( "BurstSetConfigShortDelayRetries", m_nBurstSetConfigShortDelayRetries, 3);
    oConf.GetVar( "DiscoveryBurstConfigPeriod", m_nDiscoveryBurstConfigPeriod, 60 /* seconds */ );
    oConf.GetVar( "DiscoveryRequestDelayInterval", m_nDiscoveryRequestDelayInterval, 300 /* seconds */ );
    oConf.GetVar( "SetPublishersLogUpdatePeriod", m_nSetPublishersLogUpdatePeriod, 30 /* seconds */ );

	// reports
	oConf.GetVar( "ReportsCacheFlushPeriod", m_nReportsCacheFlushPeriod, 3 /* seconds */ );

	if (InitLogEnv(strLogConfigFile.c_str()) == false)
	{
		THROW_EXCEPTION1(InvalidConfigAppException, boost::str(boost::format("invalid log config file")));
	}

}


}// namespace 
