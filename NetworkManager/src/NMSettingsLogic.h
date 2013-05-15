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

#ifndef WHARTSETTINGSLOGIC_H_
#define WHARTSETTINGSLOGIC_H_

#include "Common/NEAddress.h"
#include "Common/logging.h"
#include "Common/NETypes.h"
#include "Common/SettingsLogic.h"
#include "Model/SecurityKey.h"
//#include "Network/NetworkTypes.h"
#include "Common/simpleini/SimpleIni.h"
#include <stdint.h>
#include <Shared/AnPaths.h>

namespace hart7 {

namespace util {

#define COMMON_INI_FILE_NAME	"config.ini"
#define COMMON_INI_FILE			NIVIS_PROFILE COMMON_INI_FILE_NAME

#define CUSTOM_INI_FILE_NAME	"whart_provisioning.ini"
#define CUSTOM_INI_FILE			NIVIS_PROFILE CUSTOM_INI_FILE_NAME


struct ProvisioningItem
{
        enum DeviceType
        {
            MANAGER, GATEWAY, BACKBONE, DEVICE, NETWORK
        };

        NE::Model::SecurityKey key;

        Uint16 subnetId;

        DeviceType deviceType;

        ProvisioningItem(NE::Model::SecurityKey key_, Uint16 subnetId_, DeviceType deviceType_) :
            key(key_), subnetId(subnetId_), deviceType(deviceType_)
        {
        }

        bool isGateway()
        {
            return deviceType == GATEWAY;
        }

        bool isBackbone()
        {
            return deviceType == BACKBONE;
        }
};

/**
 * Settings class. Reads from the config files.
 */

class NMSettingsLogic: public NE::Common::SettingsLogic
{
        LOG_DEF("h7.u.NMSettingsLogic")
        ;

    public:

        string scriptFileName;

    private:

        /**
         * The name of the common ini file (default config.ini).
         */
        string commonIniFileName;

        /**
         * The name of the system manager ini file (default system_manager.ini).
         */
        string customIniFileName;



        void ParseCommonConfigFile(SimpleIni::CSimpleIniCaseA& iniParser);

        void ParseCustomConfigFile(SimpleIni::CSimpleIniCaseA& iniParser);

        static void ParseProvisioningLine(const string& p_line, Address64& p_ip64, NE::Model::SecurityKey& p_key);

        void ParseProvisioningNetworkLine(const string& p_line, NE::Model::SecurityKey& p_key, bool &flag);

        void updateProvisioningByDeviceType(SimpleIni::CSimpleIniCaseA::TNamesDepend iniItems,
                                            ProvisioningItem::DeviceType deviceType);

        static std::vector<Uint8> getAsChannelsVector(std::string channelsText);

        static string trimSpaces(const string& str);

        NMSettingsLogic& instance();

        template<typename T>
        void Parse(std::string optionName, T& value, map<string, string>& options, bool hex = false);

        bool ParseBool(std::string optionName, map<string, string>& options);

        uint32_t ComputeClampedMinPublishPeriod(uint32_t value);

        string cleanString(string& str);

    public:

        NMSettingsLogic();

        ~NMSettingsLogic();

        typedef map<Address64, NE::Model::SecurityKey> SecurityProvisioning;

        SecurityProvisioning provisioningKeys;

        unsigned int listenPort;

        unsigned int listenNetworkManagerMinRangePort;

        unsigned int listenNetworkManagerMaxRangePort;

        /**
         * The UDP server will listen on this port(UDPTestClient port).
         */
        unsigned int udpTestClientServerListenPort;

        /**
         * Gateway's IPV4 address, used for detecting UDP packets received from gateway.
         */
        Address64 gatewayAddress64;

        std::vector<Address64> backbones;

        std::vector<Address64> devices;

        /**
         * NETWORK_ID.
         */
        uint16_t NetworkID;

        /**
         * NETWORK_MAX_NODES.
         */
        uint16_t networkMaxNodes;

        /**
         * The maximum number of joins per device. This is done to limit ensure that there are sufficient
         * join links on a device.
         */
        uint16_t maxJoinsPerDevice;

        /**
         * The maximum number of joins per AccessNode.
         */
        uint16_t maxJoinsPerAP;

        /**
         * The maximum number of joins in progress per device. This is done to limit ensure that there are sufficient
         * join links on a device.
         */
        uint16_t maxJoinsInProgressPerParent;

        /**
         * Indicates if first it should try to communicate with a reported visible device before adding it to the list
         * of visible neighbors.
         */
        bool checkEdgeVisibility;

        /**
         * Represents the number of allowed alarms when checking a visible edge.
         */
        uint16_t noAllowedAlarms;

        /**
         * The max number of check visibility flows in progress.
         */
        uint16_t cevMaxFlows;

        /**
         * Represents the time a visible edge should be active without receiving any path down alarms.
         */
        uint16_t visibileEdgeTimeOut;

        /**
         * If true it checks the path down alarm received.
         * See also maxAlarmsNoBeforeCheckDevice and alarmsTimeIntervalBeforeCheckDevice properties.
         */
        bool activateDeviceAlarmCheck;

        /**
         * The number of alarms received for a device before trigger the check flow of the device.
         */
        uint16_t maxAlarmsNoBeforeCheckDevice;

        /**
         * The number of seconds in which the maxAlarmsNoBeforeCheckDevice alarms should be received
         * before triggering the check flow.
         */
        uint16_t alarmsTimeIntervalBeforeCheckDevice;

        /**
         * The maximum join priority (considering the resources needed by the joins in progress) on a parent.
         * If it exceeds this value it will refuse the current join.
         */
        uint16_t maxJoinPriorityInProgressOnParent;

        /**
         *If set to true, NM will send notification for 0 20 769 and 832 commands otherwise will forward responses.
         */
        bool handleAllReponseCodesAsError;

        /**
         *
         */
        bool sendNotificationsOnJoinFlow;

        /**
         * The minimum period at which the services will be created. Anything lower than this should be clamped to this value.
         */
        uint32_t minServicePeriod;

        bool useWaves;

        /**
         * The numbers of evaluations started at once (should be less or equal then the max evaluations)
         */
        Uint16 maxEvaluationsStartedAtOnce;

        /**
         * Unique join key per network is ON/OFF
         */
        bool uniqueKey;

        /**
         * Unique join key
         */
        NE::Model::SecurityKey NetworkUniqueKey;

        int maxJoinAttemptsBeforeOverride;


    public:

        /**
         * The name of the file that will contain the status of the network.
         */
        std::string logNetworkStateFileName;

        bool activateMockKeyGenerator;

    public:

        void LoadConfig();

        void LoadProvisioning();

        void ComputeJoinLayers();
};

}
}
#endif /*NMSettingsLogic_H_*/
