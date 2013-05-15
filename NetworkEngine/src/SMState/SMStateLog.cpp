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

#include "SMState/SMStateLog.h"
//#include "SMState/StateNetworkTopology.h"
//#include "SMState/StateDiagnostics.h"
#include "Model/NetworkEngine.h"
#include "RunLib/Version.h"
#include <vector>

/**
 * It should be DEBBUG and not INFO or greater priority because if we will put INFO on root logger
 * then this information will also appear in the main logger.
 */
namespace SMState {

struct StateLinkEngine {
        LOG_DEF("NMState.LinkEngine");

        struct LinkLog {
                friend std::ostream& operator<<(std::ostream& str, const LinkLog& log);
        };

        static void logLinkEngine(std::string reason) {
            if (LOG_DEBUG_ENABLED()) {
                LOG_DEBUG("WHART NETWORK MANAGER VERSION: " << SYSTEM_MANAGER_VERSION);
                LOG_DEBUG("Reason: " << reason);
                LOG_DEBUG(LinkLog());
            }
        }

        static void logErrLinkEngine(std::string reason) {
            LOG_ERROR("WHART NETWORK MANAGER VERSION: " << SYSTEM_MANAGER_VERSION);
            LOG_ERROR("Reason: " << reason);
            LOG_ERROR(LinkLog());
        }
};
std::ostream& operator<<(std::ostream& str, const StateLinkEngine::LinkLog& log) {
    NetworkEngine::instance().writeStateToStream((std::ostringstream&)str, NE::Model::EngineComponents::LinkEngine);
    return str;
}


struct StateSubnetServices {
        LOG_DEF("NMState.SubnetServices");

        struct SubnetLog {
                friend std::ostream& operator<<(std::ostream& str, const SubnetLog& log);
        };
        static void logSubnetServices(std::string reason) {
            try {
                if (LOG_DEBUG_ENABLED()) {
                    LOG_DEBUG("WHART NETWORK MANAGER VERSION: " << SYSTEM_MANAGER_VERSION);
                    LOG_DEBUG("Reason: " << reason);
                    LOG_DEBUG(SubnetLog());
                }
            } catch (NE::Common::NEException& ex) {
                LOG_ERROR("StateSubnetServices::logSubnetServices() : " << ex.what());
            } catch (std::exception& ex) {
                LOG_ERROR("StateSubnetServices::logSubnetServices() : " << ex.what());
            }
        }

        static void logErrSubnetServices(std::string reason) {
            try {
                LOG_ERROR("WHART NETWORK MANAGER VERSION: " << SYSTEM_MANAGER_VERSION);
                LOG_ERROR("Reason: " << reason);
                LOG_ERROR(SubnetLog());
            } catch (NE::Common::NEException& ex) {
                LOG_ERROR("StateSubnetServices::logSubnetServices() : " << ex.what());
            } catch (std::exception& ex) {
                LOG_ERROR("StateSubnetServices::logSubnetServices() : " << ex.what());
            }
        }
};
std::ostream& operator<<(std::ostream& str, const StateSubnetServices::SubnetLog& log) {
    NetworkEngine::instance().writeStateToStream((std::ostringstream&)str, NE::Model::EngineComponents::SubnetServices);
    return str;
}


struct StateDeviceTable {
        LOG_DEF("NMState.DeviceTable");

        struct DeviceTableLog {
                friend std::ostream& operator<<(std::ostream& str, const DeviceTableLog& log);
        };

        static void logDeviceTable(std::string reason) {

            if (LOG_DEBUG_ENABLED()) {

                static int seconds = time (NULL);
                static std::string latestReason;
                static bool activityFlag = false;

                if (reason.compare("wakeup") != 0){
                    latestReason = reason;
                    activityFlag = true;
                }

                int newseconds = time (NULL);
                if (!reason.compare("User induced (SIGUSR2 sent to NM)") ||
                    (seconds + 10 <= newseconds && activityFlag)) {

                    LOG_DEBUG("WHART NETWORK MANAGER VERSION: " << SYSTEM_MANAGER_VERSION);
                    LOG_ERROR("Last reason: " << latestReason << " (regardless of how many reasons does not print more often than 10 seconds)");
                    LOG_DEBUG(DeviceTableLog());
                    seconds =  newseconds;
                    latestReason = "";
                    activityFlag = false;
                }
            }
        }

        static void logErrDeviceTable(std::string reason) {
            LOG_ERROR("WHART NETWORK MANAGER VERSION: " << SYSTEM_MANAGER_VERSION);
            LOG_ERROR("Latest reason: " << reason);
            LOG_ERROR(DeviceTableLog());
        }
};
std::ostream& operator<<(std::ostream& str, const StateDeviceTable::DeviceTableLog& log) {
    NetworkEngine::instance().writeStateToStream((std::ostringstream&)str, NE::Model::EngineComponents::DeviceTable);
    return str;
}


struct StateNetworkTopology {
        LOG_DEF("NMState.NetworkTopology");

        struct TopologyLog {
                friend std::ostream& operator<<(std::ostream& str, const TopologyLog& log);
        };

        static void logNetworkTopology(std::string reason) {

            if (LOG_DEBUG_ENABLED()) {

                static int seconds = time (NULL);
                static std::string latestReason;
                static bool activityFlag = false;

                if (reason.compare("wakeup") != 0){
                    latestReason = reason;
                    activityFlag = true;
                }

                int newseconds = time (NULL);
                if (!reason.compare("User induced (SIGUSR2 sent to NM)") ||
                    (seconds + 10 <= newseconds && activityFlag)) {

                    try {
                        LOG_DEBUG("WHART NETWORK MANAGER VERSION: " << SYSTEM_MANAGER_VERSION);
                        LOG_DEBUG("Latest reason: " << latestReason << " (regardless of how many reasons does not print more often than 10 seconds)");
                        LOG_DEBUG(TopologyLog());
                        activityFlag = false;
                    } catch (NE::Common::NEException& ex) {
                        LOG_ERROR("StateNetworkTopology::logNetworkTopology() : " << ex.what());
                    } catch (std::exception& ex) {
                        LOG_ERROR("StateNetworkTopology::logNetworkTopology() : " << ex.what());
                    }

                    seconds =  newseconds;
                    latestReason = "";
                }
            }
        }

        static void logErrNetworkTopology(std::string reason) {
            try {
                LOG_ERROR("WHART NETWORK MANAGER VERSION: " << SYSTEM_MANAGER_VERSION);
                LOG_ERROR("Reason: " << reason);
                LOG_ERROR(TopologyLog());
            } catch (NE::Common::NEException& ex) {
                LOG_ERROR("StateNetworkTopology::logNetworkTopology() : " << ex.what());
            } catch (std::exception& ex) {
                LOG_ERROR("StateNetworkTopology::logNetworkTopology() : " << ex.what());
            }
        }

        static void logSubnetTopology(std::string reason) {
            try {
                if (LOG_DEBUG_ENABLED()) {
                    LOG_DEBUG("WHART NETWORK MANAGER VERSION: " << SYSTEM_MANAGER_VERSION);
                    LOG_DEBUG("Reason: " << reason);
                    LOG_DEBUG(TopologyLog());
                }
            } catch (NE::Common::NEException& ex) {
                LOG_ERROR("StateNetworkTopology::logSubnetTopology() : " << ex.what());
            } catch (std::exception& ex) {
                LOG_ERROR("StateNetworkTopology::logSubnetTopology() : " << ex.what());
            }
        }

        static void logErrSubnetTopology(std::string reason) {
            try {
                LOG_ERROR("WHART NETWORK MANAGER VERSION: " << SYSTEM_MANAGER_VERSION);
                LOG_ERROR("Reason: " << reason);
                LOG_ERROR(TopologyLog());
            } catch (NE::Common::NEException& ex) {
                LOG_ERROR("StateNetworkTopology::logSubnetTopology() : " << ex.what());
            } catch (std::exception& ex) {
                LOG_ERROR("StateNetworkTopology::logSubnetTopology() : " << ex.what());
            }
        }
};
std::ostream& operator<<(std::ostream& str, const StateNetworkTopology::TopologyLog& log) {
    NetworkEngine::instance().writeStateToStream((std::ostringstream&)str, EngineComponents::Topology);
    return str;
}

struct StateOperations {
        LOG_DEF("NMState.Operations");

        struct OperationsLog {
                OperationsLog(NE::Model::Operations::EngineOperations& operations_) : operations(operations_) {}
                friend std::ostream& operator<<(std::ostream& str, const OperationsLog& log);

                NE::Model::Operations::EngineOperations& operations;
        };


        static void logOperations(const std::string& reason, NE::Model::Operations::EngineOperations& operations) {

            if (LOG_DEBUG_ENABLED()) {
                LOG_DEBUG("Reason: " << reason << OperationsLog(operations));
            }
        }

        static void logErrOperations(const std::string& reason, NE::Model::Operations::EngineOperations& operations) {
            LOG_ERROR("Reason: " << reason << OperationsLog(operations));
        }
};
std::ostream& operator<<(std::ostream& str, const StateOperations::OperationsLog& log) {
    log.operations.toIndentString((std::ostringstream&)str);
    return str;
}

SMStateLog::SMStateLog() {
}

SMStateLog::~SMStateLog() {
}

void SMStateLog::wakeUpBufferedLogs(){

    StateNetworkTopology::logNetworkTopology("wakeup");
    StateDeviceTable::logDeviceTable("wakeup");
}

void SMStateLog::writeTopologyToFile(NE::Model::Topology::SubnetTopology& topology, std::string fileName) {
    //    FILE * pFile;

    //    std::ostringstream stream;
    //    stream << "subnetCaptures/";
    //    stream << fileName;
    //    stream << ".dot";
    //
    //    pFile = fopen(stream.str().c_str(), "w");
    //    if (pFile != NULL) {
    //        stream.str("");
    //        stream << topology.toDotString();
    //        fputs(stream.str().c_str(), pFile);
    //        fclose(pFile);
    //    }
}

void SMStateLog::logAllInfo(const std::string& reason) {

    SMStateLog::logSubnetServices(reason);
    SMStateLog::logDeviceTable(reason);
    SMStateLog::logNetworkTopology(reason);
    StateLinkEngine::logLinkEngine(reason);
}

void SMStateLog::logErrAllInfo(const std::string& reason) {

    SMStateLog::logErrSubnetServices(reason);
    SMStateLog::logErrDeviceTable(reason);
    SMStateLog::logErrNetworkTopology(reason);
    SMStateLog::logErrSubnetTopology(reason);
    StateLinkEngine::logErrLinkEngine(reason);
}


void SMStateLog::logSubnetServices(const std::string& reason) {
    StateSubnetServices::logSubnetServices(reason);
}

void SMStateLog::logErrSubnetServices(const std::string& reason) {
    StateSubnetServices::logErrSubnetServices(reason);
}


void SMStateLog::logDeviceTable(const std::string& reason) {
    StateDeviceTable::logDeviceTable(reason);
}

void SMStateLog::logErrDeviceTable(const std::string& reason) {
    StateDeviceTable::logErrDeviceTable(reason);
}

void SMStateLog::logNetworkTopology(const std::string& reason) {
    StateNetworkTopology::logNetworkTopology(reason);
}

void SMStateLog::logErrNetworkTopology(const std::string& reason) {
    StateNetworkTopology::logErrNetworkTopology(reason);
}

void SMStateLog::logOperations(const std::string& reason, NE::Model::Operations::EngineOperations& operationsList, bool fakeLog) {
    if (!fakeLog)
    {
        StateOperations::logOperations(reason, operationsList);
    }
}

void SMStateLog::logLinkEngine(const std::string& reason) {
    StateLinkEngine::logLinkEngine(reason);
}

void SMStateLog::logErrLinkEngine(const std::string& reason) {
    StateLinkEngine::logErrLinkEngine(reason);
}

void SMStateLog::logSubnetTopology(const std::string& reason) {
    StateNetworkTopology::logSubnetTopology(reason);
}

void SMStateLog::logErrSubnetTopology(const std::string& reason) {
    StateNetworkTopology::logErrSubnetTopology(reason);
}

}
