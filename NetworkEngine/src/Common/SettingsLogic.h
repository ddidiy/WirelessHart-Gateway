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

#ifndef SETTINGSLOGIC_H_
#define SETTINGSLOGIC_H_

#include "Common/NEAddress.h"
#include "Common/NETypes.h"
#include "Model/Tdma/TdmaTypes.h"
#include "Model/Topology/TopologyTypes.h"
#include "Common/simpleini/SimpleIni.h"
#include "Model/SecurityKey.h"

namespace NE {
namespace Common {

class SettingsLogic {

    public:

        /**
         * The array with all the subnet ids for the configured backbones.
         */
        Uint16 networkId;

        /**
         * NETWORK_MANAGER_TAG.
         */
        std::string networkManagerTag;

        /**
         * This can be used to restrict the access to the network. Only the Network Access Codes 0 and 4 are mandatory.
         */
        Uint16 networkAccessMode;

        /**
         *
         */
        Uint16 networkOptimizationFlags;

        /**
         * Request/Response messages per 10 second.
         */
        Uint16 requestResponseMessagesPer10Seconds;

        /**
         * The EUI64 address of the manager
         */
        Address64 managerAddress64;

        /**
         * The management bandwidth (in seconds)
         */
        Uint16 managementBandwidth;

        /**
         * Bandwidth between device and GW when the device exits quarantine state (in seconds)
         */
        Uint16 gatewayBandwidth;

        /**
         * The join bandwidth (in seconds)
         */
        Uint16 joinBandwidth;

        /**
         * The health report publish period
         */
        Uint16 healthReportPeriod;

        /**
         * The health report publish period
         */
        Uint16 discoveryReportPeriod;

        /*
         * The number of visible neighbors received on an edge
         * after the reevaluate is forced
         */
        Uint16 forceReevaluate;

        /**
         * Add an operation at the end of the evaluation in order to validate the settings
         */
        Uint16 addCheckOperation;

        /*
         * Change the route only if the peers has been deleted
         */
        Uint16 rerouteOnlyOnFail;

        /*
         * Use single reception link on AP
         */
        Uint16 useSingleReception;

        /*
         * User retry on preffered neighbor (allocate retry just after first transmission)
         */
        Uint16 useRetryOnPreffered;

        /**
         * The numbers of discovery evaluations accepted in parallel
         */
        Uint16 maxDiscoveryEvaluations;

        /**
         * The numbers of evaluations accepted in parallel
         */
        Uint16 maxEvaluations;

        /**
         * Represents the name of the graph routing algorithm used to optimize a graph route.
         */
        std::string graphRoutingAlgorithm;

        NE::Model::Topology::RoutingTypes::RoutingTypes_Enum RoutingType;

        /**
         *  The factor used in settings cost; if the resources are low in the device the setting factor should have
         *  higher values
         */
        Uint16 settingsFactor;

        /**
         * The cost for one redundant node is equal with the topologyFactor multiply with
         * the average edge cost in graph. If the factor is higher the graph will be encourage to have more
         * redundant nodes (nodes with multiple edges on graph)
         */
        Uint16 topologyFactor;


        /**
         * The percent that a new path must cost lower than the current cost to change to the new path.
         */
        Uint16 percentCostLower;

        /**
         * The number of layers on which the devices from whart_provisioning.ini will be spread.
         */
        Uint16 joinLayersNo;

        /**
         * true if sending to devices with proxy will be done with peer nickname when available.
         */
        bool enableShortProxy;

        /**
         * The channel map that will be used by the network.
         */
        Uint16 channelMap;

        /**
         * The PENDING channel map that will be used by the network.
         */
        Uint16 pendingChannelMap;

        /**
         * The period at which devices will do a keep-alive. In seconds.
         */
        Uint16 keepAlivePeriod;

        /**
         * The number of initial links number on transceiver. When a new device has joined the number of advertise links
         * on TR are reduced by one and when a transmit link is removed from the TR a new advertise is added (if the no all the
         * transmit links is less then this number).
         * The no of transmit links on TR (normal or advertise) should be constant whatever the number devices joined to TR.
         */
        Uint16 transceiverAdvertiseLinksNo;

        /**
         * Advertise publish period for the advertise link on TR. (not in config.ini)
         */
        NE::Model::Tdma::PublishPeriod::PublishPeriodEnum advPublishPeriod;

        /**
         * When set to true the links allocated for AP will be set so that the links will have a small fragmentation.
         */
        bool enableApBestFitAllocation;

        /**
         * The join of a device will be delayed until all the operations having it as the owner confirms.
         */
        bool deviceJoinAfterAllOperationsConfirms;

        /**
         * Maximum number of hops in a path
         */
        Uint16 maxHops;

        /**
         *
         */
        Uint16 hopsFactor;

        Uint16 maxNeighbors;

        /**
         *  if true, nicknames will be the last two bytes from EUI64
         *  if false, nicknames will be generated. Nickname table will be saved on disk from one session to another to make dev identification easier.
        */
        bool generateNicknamesFromEUI64;

        /**
         *  file on disk to hold table of devices and nicknames
        */
        std::string nicknamesFileName;

        /**
         *  server ip where web logger runs. This feature is entirely optional
        */
        std::string webLoggerIp;


        /**
         * maximum traffic overallocation
        */
        Uint16 percentTrafficMaxOverAlloc;

        /**
         * maximum packet error rate allowed for an edge (in %)
         */
        Uint16 perThreshold;

        int maxTRNeighbors;

        int maxDeviceNeighbors;

        bool useSlotZero;

    public:

        SettingsLogic() {

        }
        virtual ~SettingsLogic() {

        }
};

}
}
#endif /*SETTINGSLOGIC_H_*/
