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

#include <WHartHost/MainApp.h>

#include <string>
#include <exception>

#include <boost/bind.hpp> //for binding to function

//signals
#include <WHartHost/Signals.h>

//TasksRunner
#include <WHartHost/tasksrunner/TasksRunner.h>

//CommandsProcessor
#include <WHartHost/cmdsprocessor/CommandsProcessor.h>


//PeriodicTasks
#include <WHartHost/periodictasks/DoProcessDBRequests.h>
#include <WHartHost/periodictasks/DoGetTopology.h>
#include <WHartHost/periodictasks/DoSetBurstNotification.h>
#include <WHartHost/periodictasks/DoDBCleanUp.h>
#include <WHartHost/periodictasks/DoSetBurstCounters.h>
#include <WHartHost/periodictasks/DoDiscoveryBurstConfig.h>
#include <WHartHost/periodictasks/DoProcessReadingsCache.h>
#include <WHartHost/periodictasks/DoFlushReportsCache.h>

//
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>


#include <Shared/app.h>

namespace hart7 {


int MainApp::Run(int argc, char* argv[])
{

	//do not allow more than one instance
	char szLockFile[] = "/tmp/MonitorHost.flock";
	int m_nSyncFd = open( szLockFile, O_RDWR | O_CREAT, 0666 );
	if( flock( m_nSyncFd, LOCK_EX | LOCK_NB ) )
	{
		//LOG_ERROR( "Process MonitorHost try to start but another instance of program is running ");
		return 0;
	}

	int resultCode = 0;
	try
	{
		//config 
		config.Load();
		/* commented when using -DACCESS_NODE_LOG_ENABLED 
		LOG_INIT(config.LogConfigurationFilePath());
		*/

		LOG_INFO_APP("Starting HostApplication (version=" << VERSION << ") ...");

		
#ifdef USE_SQLITE_DATABASE
		std::string strParams;
		strParams = config.DatabaseFilePath();
		strParams +=", ";
		char szVal[255];
		sprintf(szVal, "%d", config.DatabaseTimeout());
		strParams+=szVal;

		LOG_DEBUG_APP("Connect to the sqlite database: params=" << strParams);

		//database
		hostapp::DevicesManager devices(config.GetReportsCacheFlushPeriod() == 0 ? false : true, strParams, STORE_TYPE_SQLITE);
		hostapp::DBCommandsManager commands(strParams, STORE_TYPE_SQLITE);

#elif USE_MYSQL_DATABASE
		std::string strParams;
		strParams = config.DatabaseServer();
		strParams +=", ";
		strParams = config.DatabaseUser();
		strParams +=", ";
		strParams = config.DatabasePassword();
		strParams +=", ";
		strParams = config.DatabaseName();
		strParams +=", ";
		char szVal[255];
		sprintf(szVal, "%d", config.ConnectToDatabaseRetryInterval());
		strParams+=szVal;

		LOG_DEBUG_APP("Connect to the msql database: params=" << strParams);

		//database
		hostapp::DevicesManager devices(config.GetReportsCacheFlushPeriod() == 0 ? false : true, strParams, STORE_TYPE_MYSQL);
		hostapp::DBCommandsManager commands(strParams, STORE_TYPE_MYSQL);
#else

		//database
		hostapp::DevicesManager devices("", STORE_TYPE_MEM, config.GetReportsCacheFlushPeriod());
		hostapp::DBCommandsManager commands("", STORE_TYPE_MEM);
#endif

		devices.SetReadingsSavePeriod(config.GetReadingsSavePeriod());

		//Gateway
		gateway::GatewayIO	gateway(config.CommandsTimeout(), config.GatewayAllowCmd() == 1 ? true : false);

		//CommandsProcessor
		hostapp::CommandsProcessor processor(gateway);

		//TasksRunner
		hostapp::TasksRunner tasksRunner(config.InternalTasksPeriod()/*sec*/*1000/*ms*/);

		//Tasks
		hostapp::DoProcessDBRequests processDBReq(commands, devices, processor);
		hostapp::DoGetTopology topology(commands, devices, processor, config.TopologyPeriod());
		hostapp::DoSetBurstNotification setBurstNotif(commands, devices, processor, config.PublishersFile(),
													  config.GetBurstSetConfigurationRetryPeriod(),
													  config.GetBurstSetConfigurationShortDelayRetries(),
													  config.GetPublishPeriodToleranceThreshold(),
													  config.GetSetPublishersLogUpdatePeriod());
		hostapp::DoDBCleanUp dbCleanUp(commands,
									   config.DatabaseVacuumPeriodM(),
									   config.DatabaseRemoveEntriesPeriodM(),
									   config.DatabaseRemoveEntriesOlderThanM(),
									   config.DatabaseRemoveEntriesMoreThanMaxCount());
		hostapp::DoSetBurstCounters	setBurstCounters(devices,
													 config.GetPublishPeriodToleranceThreshold(),
													 config.GetBurstCountersUpdatePeriod());
		hostapp::DoDiscoveryBurstConfig discoveryBurstConfig(commands, devices, processor, setBurstNotif.GetPublishersMapStored(),
		                                                     config.PublishersFile(), config.GetDiscoveryBurstConfigPeriod(),
		                                                     config.GetDiscoveryRequestDelayInterval(),
		                                                     config.GetBurstSetConfigurationRetryPeriod());
		hostapp::DoProcessReadingsCache readingsCache(devices, config.GetReadingsSavePeriod(), config.GetMaxReadingsNoPerTransaction());
		hostapp::DoFlushReportsCache flushReportsCache(devices, config.GetReportsCacheFlushPeriod());

		//dbcommands' observers
		commands.AddCmdResponseHandle(boost::bind(&hostapp::DoGetTopology::HandleRespondedCommand, &topology, _1, _2, _3));

		//devices observers
		devices.SetPubNotificationCallback(boost::bind(&hostapp::DoSetBurstCounters::HandleIncomingNotification, &setBurstCounters, _1, _2, _3));
		devices.SetPubDeviceDeletionCallback(boost::bind(&hostapp::DoSetBurstCounters::HandlePubDeviceDeletion, &setBurstCounters, _1));
		devices.SetPubDeviceBurstsChangeCallback(boost::bind(&hostapp::DoSetBurstCounters::HandlePubDeviceBurstsChange, &setBurstCounters, _1, _2, _3, _4));
		devices.SetUpdatePublishersCacheCallback(boost::bind(&hostapp::DoSetBurstNotification::UpdatePublishersCache, &setBurstNotif, _1, _2));
        devices.SetUpdateStoredPublishersCallback(boost::bind(&hostapp::DoDiscoveryBurstConfig::UpdateStoredPublishers, &discoveryBurstConfig, _1, _2, _3));
        devices.SetUpdatePublisherStateCallback(boost::bind(&hostapp::DoSetBurstNotification::UpdatePublisherState, &setBurstNotif, _1, _2, _3, _4, _5));
        devices.SetIssueTopologyCommandCallback(boost::bind(&hostapp::DoGetTopology::IssueTopologyCommand, &topology));
		devices.SetCacheReadingCallback(boost::bind(&hostapp::DoProcessReadingsCache::CacheReading, &readingsCache, _1));
		devices.SetBurstCounterTaskPeriodCallback(boost::bind(&hostapp::TasksRunner::SetBurstCounterTaskPeriod, &tasksRunner, _1));

		//gw observers
		gateway.AddForGWConnectNotify(boost::bind(&hostapp::DevicesManager::HandleGWConnect, &devices, _1, _2));
		gateway.AddForGWDisconnectNotify(boost::bind(&hostapp::DevicesManager::HandleGWDisconnect, &devices));
		gateway.AddForGWConnectNotify(boost::bind(&hostapp::DoProcessDBRequests::HandleGWConnect, &processDBReq, _1, _2));
		gateway.AddForGWDisconnectNotify(boost::bind(&hostapp::DoProcessDBRequests::HandleGWDisconnect, &processDBReq));
		gateway.AddForGWConnectNotify(boost::bind(&hostapp::DoSetBurstNotification::HandleGWConnect, &setBurstNotif, _1, _2));
		gateway.AddForGWDisconnectNotify(boost::bind(&hostapp::DoSetBurstNotification::HandleGWDisconnect, &setBurstNotif));
        gateway.AddForGWConnectNotify(boost::bind(&hostapp::DoGetTopology::HandleGWConnect, &topology, _1, _2));
        gateway.AddForGWDisconnectNotify(boost::bind(&hostapp::DoGetTopology::HandleGWDisconnect, &topology));
        gateway.AddForGWConnectNotify(boost::bind(&hostapp::DoDiscoveryBurstConfig::HandleGWConnect, &discoveryBurstConfig, _1, _2));
        gateway.AddForGWDisconnectNotify(boost::bind(&hostapp::DoDiscoveryBurstConfig::HandleGWDisconnect, &discoveryBurstConfig));
		gateway.AddForGWConnectNotify(boost::bind(&hostapp::DoSetBurstCounters::HandleGWConnect, &setBurstCounters, _1, _2));
		gateway.AddForGWDisconnectNotify(boost::bind(&hostapp::DoSetBurstCounters::HandleGWDisconnect, &setBurstCounters));

		//gw's msg receiver
		gateway.RegisterForRecvMsg(boost::bind(&hostapp::CommandsProcessor::ProcessResponse, &processor, _1));

		//register periodic tasks
		tasksRunner.RegisterPeriodicTask(boost::bind(&hostapp::DoProcessDBRequests::DoTask, &processDBReq, _1));
		tasksRunner.RegisterPeriodicTask(boost::bind(&hostapp::DoGetTopology::DoTask, &topology, _1));
		// DoSetBurstNotification must runs before DoDiscoveryBurstConfig
		tasksRunner.RegisterPeriodicTask(boost::bind(&hostapp::DoSetBurstNotification::DoTask, &setBurstNotif, _1));
        tasksRunner.RegisterPeriodicTask(boost::bind(&hostapp::DoDiscoveryBurstConfig::DoTask, &discoveryBurstConfig, _1));
		tasksRunner.RegisterPeriodicTask(boost::bind(&hostapp::DoProcessReadingsCache::DoTask, &readingsCache, _1));
		tasksRunner.RegisterBurstCounterTask(boost::bind(&hostapp::DoSetBurstCounters::DoTask, &setBurstCounters, _1));
        tasksRunner.RegisterPeriodicTask(boost::bind(&hostapp::DoDBCleanUp::DoTask, &dbCleanUp, _1));
		if (config.GetReportsCacheFlushPeriod() != 0)
			tasksRunner.RegisterPeriodicTask(boost::bind(&hostapp::DoFlushReportsCache::DoTask, &flushReportsCache, _1));


		//prepare for running tasks
		tasksRunner.RegisterCreateTimer(boost::bind(&gateway::GatewayIO::GetTimer, &gateway, _1));

		//subscribe for signals
		util::Signals signals;
		signals.RegisterSignalHandler(SIGTERM, boost::bind(&gateway::GatewayIO::Stop, &gateway));
		signals.RegisterSignalHandler(SIGHUP, boost::bind(&hostapp::DoDiscoveryBurstConfig::DoLoadPublishers, &discoveryBurstConfig));
		signals.RegisterSignalHandler(SIGHUP, boost::bind(&hostapp::DoSetBurstNotification::DoLoadPublishers, &setBurstNotif));

		//for log to be flushed when segmn.fault
#ifndef HW_VR900
		signal( SIGABRT, HandlerFATAL );/// do NOT use delayed processing with HandlerFATAL. Stack trace must be dumped on event
		signal( SIGSEGV, HandlerFATAL );/// do NOT use delayed processing with HandlerFATAL. Stack trace must be dumped on event
		signal( SIGFPE,  HandlerFATAL );/// do NOT use delayed processing with HandlerFATAL. Stack trace must be dumped on event
#endif

		//connect to gw!
		gateway.Start(config.GatewayHostIP(), config.GatewayHostPort(), config.GatewayLocalPort());
		gateway.Run();

		/*when gateway.Stop() is called the application is closed*/
		resultCode = 0;
	}
	catch(InvalidConfigAppException& ex)
	{
		LOG_ERROR_APP(ex.Message() << std::endl << " exception=" << ex.what());
		std::cout << "FATAL: " << ex.Message() << std::endl << " exception=" << ex.what();
		resultCode = 3;
	}
	catch(std::exception& ex)
	{
		LOG_ERROR_APP("Unhandled exception=" << ex.what() << " in MainApp::Run");
		resultCode = 1;
	}
	catch(...)
	{
		LOG_ERROR_APP("Unknown exception in MainApp::Run");
		resultCode = 2;
	}

	LOG_INFO_APP("Stopped.");
	return resultCode;
}


}
