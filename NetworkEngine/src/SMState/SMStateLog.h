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

#ifndef SMSTATELOG_H_
#define SMSTATELOG_H_

#include "Model/NetworkEngineTypes.h"
#include "Model/Topology/SubnetTopology.h"
#include "Model/Operations/EngineOperations.h"
#include "Model/Operations/IEngineOperation.h"

using namespace NE::Common;
using namespace NE::Model;

namespace SMState {

/**
 * A utility class that allows the user to log different information about the SystemManager state.
 * Information will be logged to its own file to allow the users to see only the needed data.
 * For each type of information (eg: DeviceTable, SubnetServices ...) there will be a method in this class
 * that calls a method in specific class.
 * We have to have a class for each type of data because the logging mechanism is used to create the file
 * and we have to have a logger for each type of data.
 * @author Radu Pop
 */
class SMStateLog {

    public:

        SMStateLog();

        virtual ~SMStateLog();

        /**
         * makes sure device table and network topology logs are flushed
         */
        static void wakeUpBufferedLogs();

        /**
         * Logs all the available information. The <code>reason</code> parameter represents a short
         * explanation of the motive that caused the changes in the NetworkEngine's model.
         * @param reason
         */
        static void logAllInfo(const std::string& reason);
        static void logErrAllInfo(const std::string& reason);

        /**
         * Logs the state of the SubnetServices in its own file.
         */
        static void logSubnetServices(const std::string& reason);
        static void logErrSubnetServices(const std::string& reason);

        /**
         * Logs the state of the DeviceTable in its own file.
         */
        static void logDeviceTable(const std::string& reason);
        static void logErrDeviceTable(const std::string& reason);

        /**
         * Logs the content of the LinkEngine internal model.
         * @param reason
         */
        static void logLinkEngine(const std::string& reason);
        static void logErrLinkEngine(const std::string& reason);

        /**
         * Logs the state of the NetworkTopology in its own file.
         */
        static void logNetworkTopology(const std::string& reason);
        static void logErrNetworkTopology(const std::string& reason);

        /**
         * Logs the list operations to a specific log file.
         */
        //HACK - [andy] - fakeLog added to log only when dependencies are set, in the Queue
        static void logOperations(const std::string& reason, NE::Model::Operations::EngineOperations& operationsList, bool fakeLog = true);

        /**
         * Logs the state of the SubnetTopology in network topology log file.
         */
        static void logSubnetTopology(const std::string& reason);
        static void logErrSubnetTopology(const std::string& reason);

        static void writeTopologyToFile(NE::Model::Topology::SubnetTopology& topology, std::string fileName);
};

}

#endif /*SMSTATELOG_H_*/
