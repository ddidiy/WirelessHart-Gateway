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

#ifndef NETWORKENGINE_H_
#define NETWORKENGINE_H_

#include <vector>
#include <map>

#include "Common/logging.h"
#include "Common/SettingsLogic.h"
#include "Model/Services/SubnetServices.h"
#include "Model/DevicesTable.h"
#include "Model/IEngine.h"
#include "Model/Tdma/SubnetTdma.h"
#include "Model/Services/Service.h"
#include <WHartStack/WHartTypes.h>

using namespace NE::Model::Operations;
using namespace NE::Model::Services;
using namespace NE::Model::Topology;
using namespace NE::Model::Tdma;

namespace NE {

namespace Model {

typedef std::map<Address64, std::vector<time_t> > RejoinTimestamps;

/**
 * Called when some modification has been made on a device (a new neighbor has been added or removed,
 * a route has been added/removed etc).
 */
typedef boost::function2<void, IEngineOperation&, uint32_t> MarkAsChangedCallback;

namespace EngineComponents {
enum EngineComponentsEnum {
    ALL = 0, SubnetServices = 1, DeviceTable = 2, Topology = 3, LinkEngine = 4
};
}

class ComparePathPriority {
    public:
        bool operator() (Path* lhs, Path* rhs) {
            if (lhs->getEvaluatePathPriority() == rhs->getEvaluatePathPriority()
                        || (lhs->getEvaluatePathPriority() <= EvaluatePathPriority::OutboundDiscovery
                                    && rhs->getEvaluatePathPriority() <= EvaluatePathPriority::OutboundDiscovery)) {

                if (lhs->getLastEvalTime() == rhs->getLastEvalTime()) {
                    return lhs->getGraphId() < rhs->getGraphId();

                }
                else {
                    return lhs->getLastEvalTime() < rhs->getLastEvalTime();
                }

            }
            else {
                return lhs->getEvaluatePathPriority() > rhs->getEvaluatePathPriority();
            }
        }
};

/**
 * NetworkEngine.
 */
class NetworkEngine: boost::noncopyable, public IEngine {

    LOG_DEF("I.M.NetworkEngine");

    private:

        /** The time the NetworkEngine was started. */
        Uint32 startTime;

        /** subnet id */
        Uint16 subnetId;

        /** the settings read from config.ini file */
        SettingsLogic settingsLogic;

        /** the list of devices */
        DevicesTable devicesTable;

        /** the topology contains the routing related code */
        SubnetTopology subnetTopology;

        /** the tdma contains bandwidth management related code */
        SubnetTdma subnetTdma;

        /** the services contains code related to required services by devices */
        SubnetServices subnetServices;

        /** Called when some modifications has occurred on a device. */
        MarkAsChangedCallback markAsChanged;

        std::set<Address32> devicesWithSessionKeys;

        std::set<Path*, ComparePathPriority> priorityPathSet;

        int discoveryCount;
        int evalCount;

    private:

        /**
         *
         */
        NetworkEngine();

        /**
         *
         */
        void setDeviceStatus(Address32 deviceAddress32, DeviceStatus::DeviceStatus deviceStatus);

        /**
         *
         */
        void checkParentConstraints(Model::Topology::SubnetTopology& subnetTopology, Address32 deviceAddress,
                    Address32 parentAddress, Capabilities& capabilities);

        /**
         *
         */
        void generateHealthReportPeriod(Model::Operations::EngineOperations& operations, Address32 deviceAddress);

        /**
         *
         */
        void removeDefaultRoute(NE::Model::Operations::EngineOperations& operations, Address32 deviceAddress);

        /**
         *
         */
        PublishPeriod::PublishPeriodEnum toPublishPeriod(Uint16 traffic);

        /**
         * Searches for secondary clock source for the node with given address.
         */
        bool evaluateSecondaryClkSrc(EngineOperations& engineOperations, Address32 addr, Uint16 maxHops);

        /**
         * Tests if a candidate node (addr) can be used as secondary clock source.
         * @param nodeAddr - potential clk child
         * @param crtAddr - current node address
         * @param currentCount - security count (for the recursion)
         * @param maxCounts - maximum numbers of recursive calls
         * @param level - the level of the potential clk child in the primary inbound tree
         * @return true - if the clk source candidate is good
         */
        bool testSecondaryClkSrc(Address32 nodeAddr, Address32 crtAddr, Uint16 currentCount, Uint16 maxCounts,
                    Uint16 level);

        /**
         * Computes the level on primary inbound tree.
         * @param addr - node address
         * @param currentLevel - curent level (set to 0)
         * @param maxLevel - maximum level to search (set to maximum numbers of hops)
         * @return the node with address addr level (<0 - reaches maxLevel; 0 - backbone
         */
        Int16 getPrimaryInboundLevel(Address32 addr, Uint16 currentLevel, Uint16 maxLevel);

    public:

        /**
         *
         */
        static NetworkEngine& instance();

        /**
         *
         */
        virtual ~NetworkEngine();

        /**
         *
         */
        Uint16 getSubnetId();

        /**
         * Should return the start time of this engine.
         */
        Uint32 getStartTime();

        /**
         *
         */
        NE::Common::SettingsLogic& getSettingsLogic();

        /**
         *
         */
        void setSettingsLogic(const NE::Common::SettingsLogic& settingsLogic);

        /**
         *
         */
        Address32 createAddress32(Address64 address64);

        /**
         * Returns true if there is a device with the specified address32.
         */
        bool existsDevice(Address32 address);

        /**
         * Returns true if there is a device with the specified address64.
         */
        bool existsDevice(const Address64& address64);

        /**
         *
         */
        bool getDeviceSessionKey(Address32 address);

        /**
         *
         */
        void setDeviceSessionKey(Address32 address, bool hasKey);

        /**
         * Return the device with the given address.
         */
        NE::Model::Device& getDevice(Address32 address);

        /**
         *
         */
        Address32 getAddress32(const Address64& address64);

        /**
         *
         */
        Address64 getAddress64(Address32 address32);

        /**
         *
         */
        SubnetServices& getSubnetServices();

        /**
         *
         */
        DevicesTable& getDevicesTable();

        /**
         *
         */
        SubnetTdma& getSubnetTdma();

        /**
         *
         */
        SubnetTopology& getSubnetTopology();

        /**
         * Called when a join request is received, to send device to loosely coupled.
         */
        bool initJoinDevice(NE::Model::Operations::EngineOperations& operationsList,
                    NE::Model::Capabilities& capabilities, Address32 address32, WHartUniqueID uniqueID,
                    Address32 parentAddress32, const SecurityKey& networkKey, const SecurityKey& key);

        /**
         *
         */
        bool joinDevice(NE::Model::Operations::EngineOperations& operationsList, NE::Model::Capabilities& capabilities,
                    Address32 address32, Address32 parentAddress32, Uint16 parentJoinsInProgressNo);

        /**
         *
         */
        void confirmDeviceQuarantine(Address32 address);

        /**
         *
         */
        void setRemoveStatus(Address32 address);

        /**
         * Return the management service from manager to device.
         */
        NE::Model::Services::Service& findManagementService(const Address32& destination, bool isProxyDestination);

        /**
         * Create a service for a service requested.
         */
        bool createService(Service& serviceRequest, bool sendToRequester = true);

        /**
         * Used for Service termination, deactivation and reactivation request.
         */
        bool terminateService(Address32 address, ServiceId serviceId);

        /**
         * Adds the visible node with the address visibleAddress to the existingAddress.
         */
        void addVisible(Address32 existingAddress, Address32 visibleAddress, Uint8 rsl);

        /**
         *
         */
        void addDiagnostics(Address32 existingAddress, Address32 neighborAddress, Uint8 rsl, Uint16 sent,
                    Uint16 received, Uint16 failed);

        /**
         * Resolve operations:
         * 1. all operation confirmed;
         * 2. an error occurred -> the device(s) with problems are deleted, for unsent operations
         * undo the change and for success operations mark the resources as not used.
         */
        bool resolveOperations(NE::Model::Operations::EngineOperations& operationsList,
                    NE::Model::Operations::EngineOperations& confirmedOperationsList,
                    bool removeOnTimeout);

        /**
         * based on topology trigger path evaluations.
         */
        void triggerTopologyEvaluations();

        /**
         * Reevaluate one route, that is set to be evaluated.
         */
        bool evaluateNextPath(NE::Model::Operations::EngineOperations& engineOperations);
        /**
         * Return true if the device is prepared to act as a router
         */
        bool canSendAdvertise(Address32 address);

        /**
         *
         */
        bool allocateAdvertise(EngineOperations& engineOperations, Address32 address);

        /**
         * Check if there are devices marked for delete.
         */
        void checkRemovedDevices(EngineOperations& engineOperations, AddressSet& addressSet);

        /**
         *
         * @param addressSet
         */
        void getRemovedDevices(AddressSet& addressSet);


        /**
         *
         */
        void evaluateClockSource(EngineOperations& engineOperations, Address32 addr);

        /**
         * Resets all the data structures cached inside the engine.
         */
        void resetEngine();

        /**
         * Registers the call back for mark as changed notifications.
         */
        void registerMarkAsChangedCallback(MarkAsChangedCallback markAsChanged);

        /**
         * If the MarkAsChangedCallback is set call it with the given parameters.
         */
        void forwardMarkAsChanged(IEngineOperation& operation, uint32_t peerAddress);

        /**
         * A new alarm has been received.
         */
        void newAlarm788(Uint16 source, Uint16 destination);

        /**
         *  Marks the edge as failing, and requests reevaluations on paths containing that edge.
         */
        void markFailedEdge(Uint16 source, Uint16 destination);

        /**
         * Allocate a link to check the visibility between the nodes.
         */
        bool allocateCheckLink(NE::Model::Operations::EngineOperations& engineOperations, Address32 address,
                    Address32 peerAddress);

        /**
         * Deallocate a link used to check the visibility between the nodes.
         */
        bool deallocateCheckLink(NE::Model::Operations::EngineOperations& engineOperations, Address32 address,
                    Address32 peerAddress);

        /**
         *
         */
        void toIndentString(std::ostringstream& stream);

        /**
         *
         */
        void writeStateToStream(std::ostringstream& stream, EngineComponents::EngineComponentsEnum component);


        /**
         * Walks through the inbound graph of the node and selects the largest Join Priority.
         * @param address
         * @return
         */
        uint8_t getMaxJoinPriority(Address32 address);

        //        void readFromTimeoutedDevice(NE::Model::Operations::EngineOperations& engineOperations, Address32 address);

        bool existsCSCycle(Node& node, Address32 peer);

        bool deletePathFromPrioritySet(Path *path);

        bool deleteOldEntryInPriorityPathSet(Path* path);
        void insertNewEntryInPriorityPathSet(Path* path);
};

}
}

#endif /* MAIN_H_ */
