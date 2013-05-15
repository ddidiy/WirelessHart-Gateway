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

#include <iostream>
#include <fstream>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <algorithm>

#include <ApplicationLayer/Model/CommonTables.h>

#include "NMSettingsLogic.h"

#include "Security/SecurityManager.h"

namespace hart7 {

namespace util {

typedef boost::tokenizer<boost::char_separator<char> > tokenizer;

template<typename ElemT>
struct HexTo
{
        ElemT value;
        operator ElemT() const
        {
            return value;
        }
        friend std::istream& operator>>(std::istream& in, HexTo& out)
        {
            in >> std::hex >> out.value;
            return in;
        }
};

template<typename T>
void NMSettingsLogic::Parse(std::string optionName, T& value, map<string, string>& options, bool hex)
{
    try
    {
        if (hex)
        {
            value = boost::lexical_cast<HexTo<T> >(options[optionName]);
        }
        else
        {
            value = boost::lexical_cast<T>(options[optionName]);
        }
        LOG_DEBUG(optionName << ":" << value);
    }
    catch (boost::bad_lexical_cast & ex)
    {
        LOG_ERROR(ex.what());
        LOG_ERROR("Option " << optionName << ": invalid value (" << options[optionName] << ").");
        throw NEException("Bad option in config file.");
    }
}

bool NMSettingsLogic::ParseBool(std::string optionName, map<string, string>& options)
{
    string value = options[optionName];
    if (value != "true" && value != "false")
    {
        LOG_ERROR("In " << commonIniFileName << ", option " << optionName << ": invalid value, expected true or false");
        throw NEException("Bad option in config file.");
    }
    LOG_DEBUG(optionName << ":" << value);
    return value == "true";
}

NMSettingsLogic::NMSettingsLogic()
{
    udpTestClientServerListenPort = 8899;

    managerAddress64.loadString("00-1B-1E-F9-80-00-00-01"); // hard-coded

    networkAccessMode = (uint16_t) NetworkAccessModeCode_Open;
    networkOptimizationFlags = 0;
    requestResponseMessagesPer10Seconds = 0; // TODO take this no from somewhere

    advPublishPeriod = NE::Model::Tdma::PublishPeriod::P_1_S;
}

NMSettingsLogic::~NMSettingsLogic()
{

}

uint32_t NMSettingsLogic::ComputeClampedMinPublishPeriod(uint32_t value)
{
    if (value <= 100)
        return 100; // less than 100 msecs, set 100 msecs

    if (value > 3600 * 1000)
    {
        return 3600 * 1000; // greater than 3600 secs, return 3600 secs
    }

    if (value >= 60 * 1000)
    {
        // greater than 60, return an integer number of seconds (61, 65, etc)
        return (value - value % 1000) + 1000;
    }

    uint32_t retVal = 250;
    while (retVal <= 32000)
    {
        if (value <= retVal)
        {
            return retVal;
        }

        retVal *= 2;
    }

    return 60000; // if in interval (32 s, 60 s), return 60 s
}

void NMSettingsLogic::ParseCommonConfigFile(SimpleIni::CSimpleIniCaseA& iniParser)
{
    // build a map of options in the form group->option, defaultValue
    multimap<string, pair<string, string> > expectedOptions;

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER", pair<string, string> ("NETWORK_ID",
                                                                                                         "0")));
    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("JOIN_LAYERS_NO", "1")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("NETWORK_MAX_NODES", "50")));

    expectedOptions.insert(
                           pair<string, pair<string, string> > (
                                                                "NETWORK_MANAGER",
                                                                pair<string, string> ("NETWORK_MANAGER_TAG", "Nivis NM")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("MAX_JOINS_PER_DEVICE", "5")));


    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("MAX_JOINS_PER_AP", "10")));

    expectedOptions.insert(
                           pair<string, pair<string, string> > (
                                                                "NETWORK_MANAGER",
                                                                pair<string, string> (
                                                                                      "MAX_JOINS_IN_PROGRESS_PER_DEVICE",
                                                                                      "5")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("GRAPH_ROUTING_ALGORITHM", "")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("SETTINGS_FACTOR", "2")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("TOPOLOGY_FACTOR", "4")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("MANAGEMENT_BANDWIDTH", "4")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("GATEWAY_BANDWIDTH", "4")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("JOIN_BANDWIDTH", "4")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("HEALTH_REPORTS_PERIOD", "15")));

    expectedOptions.insert(
                           pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("DISCOVERY_REPORTS_PERIOD", "15")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("FORCE_REEVALUATE", "100")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("ADD_CHECK_OPERATION", "1")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("REROUTE_ONLY_ON_FAIL", "1")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("USE_SINGLE_RECEPTION", "1")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("USE_RETRY_ON_PREFFERED", "1")));

    expectedOptions.insert(
                           pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("MAX_DISCOVERY_EVALUATIONS", "2")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("MAX_EVALUATIONS", "6")));

    expectedOptions.insert(
                           pair<string, pair<string, string> > (
                                                                "NETWORK_MANAGER",
                                                                pair<string, string> (
                                                                                      "MAX_EVALUATIONS_STARTED_AT_ONCE",
                                                                                      "2")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("ACTIVATE_MOCK_KEY_GENERATOR",
                                                                                      "true")));

    //    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER", pair<string, string> (
    //                "NETWORK_MANAGER", "10.16.0.139,11100,1")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("NETWORK_MANAGER_MIN_RANGE_PORT",
                                                                                      "11101")));
    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("NETWORK_MANAGER_MAX_RANGE_PORT",
                                                                                      "11105")));

    //    expectedOptions.insert(pair<string, pair<string, string> > ("ACCESS_POINTS", pair<string, string> (
    //                "ACCESS_POINTS_NO", "0")));
    //
    //    expectedOptions.insert(pair<string, pair<string, string> > ("GATEWAY", pair<string, string> ("GATEWAY",
    //                "10.16.0.236,4900")));
    expectedOptions.insert(
                           pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("CHECK_EDGE_VISIBILITY", "false")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("NO_ALLOWED_ALARMS", "1")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("CEV_MAX_FLOWS", "10")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("VISIBLE_EDGE_TIMEOUT", "180")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("ACTIVATE_DEVICE_ALARM_CHECK",
                                                                                      "false")));
    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("ENABLE_SHORT_PROXY", "false")));

    expectedOptions.insert(
                           pair<string, pair<string, string> > (
                                                                "NETWORK_MANAGER",
                                                                pair<string, string> (
                                                                                      "MAX_ALARMS_NO_BEFORE_CHECK_DEVICE",
                                                                                      "3")));

    expectedOptions.insert(
                           pair<string, pair<string, string> > (
                                                                "NETWORK_MANAGER",
                                                                pair<string, string> (
                                                                                      "ALARMS_TIME_INTERVAL_BEFORE_CHECK_DEVICE",
                                                                                      "300")));

    expectedOptions.insert(
                           pair<string, pair<string, string> > (
                                                                "NETWORK_MANAGER",
                                                                pair<string, string> (
                                                                                      "MAX_JOIN_PRIORITY_IN_PROGRESS_ON_PARENT",
                                                                                      "10")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("INTERPRET_ERROR_RESPONSECODES",
                                                                                      "false")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("SEND_NOTIFICATIONS_ON_JOIN",
                                                                                      "false")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("MIN_SERVICE_PERIOD", "4000")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER", pair<string, string> ("CHANNEL_MAP",
                                                                                                         "7FFF")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("KEEP_ALIVE_PERIOD", "10")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("WAVE_DEPENDENCY", "true")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("TRANSCEIVER_ADVERTISE_LINKS_NO",
                                                                                      "6")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("ENABLE_AP_BEST_FIT_ALLOCATION", "false")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("DEVICE_JOIN_AFTER_ALL_OPERATIONS_CONFIRMS", "false")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("MAX_HOPS_ALLOWED", "3")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("HOPS_FACTOR", "2")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                    pair<string, string> ("GENERATE_NICKNAMES_FROM_EUI64", "true")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                        pair<string, string> ("NICKNAMES_FILE_NAME", "DeviceNicknames.txt")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                            pair<string, string> ("WEBLOGGER_IP", "")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                    pair<string, string> ("PERCENT_TRAFFIC_OVERALLOC", "25")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("MAX_NEIGHBORS_TO_EVALUATE", "10")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("MAX_PER_ALLOWED", "10")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("MAX_TR_NEIGHBORS", "99")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("MAX_DEVICE_NEIGHBORS", "99")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("USE_SLOT_ZERO", "true")));

    expectedOptions.insert(pair<string, pair<string, string> > ("NETWORK_MANAGER",
                                                                pair<string, string> ("MAX_JOIN_ATTEMPTS_BEFORE_OVERRIDE", "3")));


    map<string, string> options;

    // read expected options
    map<string, pair<string, string> >::const_iterator it = expectedOptions.begin();
    for (; it != expectedOptions.end(); ++it)
    {
        const char* value = iniParser.GetValue((*it).first.c_str(), (*it).second.first.c_str());
        //std::cout << "name :" << ((*it).first + "." + (*it).second);
        //std::cout << ", value :" << value << std::endl;
        if (value == NULL)
        {
            LOG_ERROR("Option " << (*it).first << '.' << (*it).second.first << " not found in " << commonIniFileName
                        << ". Defaulting to " << it->second.second);
            value = (*it).second.second.c_str();
        }
        string key = (*it).first + "." + (*it).second.first;
        options[key] = string(value);
    }

    listenPort = 11000;

    Parse("NETWORK_MANAGER.NETWORK_ID", networkId, options);
    Parse("NETWORK_MANAGER.JOIN_LAYERS_NO", joinLayersNo, options);
    Parse("NETWORK_MANAGER.NETWORK_MAX_NODES", networkMaxNodes, options);
    Parse("NETWORK_MANAGER.NETWORK_MANAGER_TAG", networkManagerTag, options);

    Parse("NETWORK_MANAGER.MAX_JOINS_PER_DEVICE", maxJoinsPerDevice, options);
    Parse("NETWORK_MANAGER.MAX_JOINS_PER_AP", maxJoinsPerAP, options);

    Parse("NETWORK_MANAGER.MAX_JOINS_IN_PROGRESS_PER_DEVICE", maxJoinsInProgressPerParent, options);

    Parse("NETWORK_MANAGER.MANAGEMENT_BANDWIDTH", managementBandwidth, options);
    Parse("NETWORK_MANAGER.GATEWAY_BANDWIDTH", gatewayBandwidth, options);
    Parse("NETWORK_MANAGER.JOIN_BANDWIDTH", joinBandwidth, options);
    Parse("NETWORK_MANAGER.HEALTH_REPORTS_PERIOD", healthReportPeriod, options);
    Parse("NETWORK_MANAGER.DISCOVERY_REPORTS_PERIOD", discoveryReportPeriod, options);
    Parse("NETWORK_MANAGER.FORCE_REEVALUATE", forceReevaluate, options);
    Parse("NETWORK_MANAGER.ADD_CHECK_OPERATION", addCheckOperation, options);
    Parse("NETWORK_MANAGER.REROUTE_ONLY_ON_FAIL", rerouteOnlyOnFail, options);
    Parse("NETWORK_MANAGER.USE_SINGLE_RECEPTION", useSingleReception, options);
    Parse("NETWORK_MANAGER.USE_RETRY_ON_PREFFERED", useRetryOnPreffered, options);
    Parse("NETWORK_MANAGER.MAX_DISCOVERY_EVALUATIONS", maxDiscoveryEvaluations, options);
    Parse("NETWORK_MANAGER.MAX_EVALUATIONS", maxEvaluations, options);
    Parse("NETWORK_MANAGER.MAX_EVALUATIONS_STARTED_AT_ONCE", maxEvaluationsStartedAtOnce, options);
    maxEvaluationsStartedAtOnce = (maxEvaluationsStartedAtOnce > maxEvaluations) ? maxEvaluations
                : maxEvaluationsStartedAtOnce;
    Parse("NETWORK_MANAGER.GRAPH_ROUTING_ALGORITHM", graphRoutingAlgorithm, options);
    std::string routingType =  cleanString(graphRoutingAlgorithm);
    LOG_DEBUG(routingType);

    //if alg selection is incorrect set graph routing as default
    RoutingType = NE::Model::Topology::RoutingTypes::GRAPH_ROUTING;

    if (routingType == "graphrouting") {
        RoutingType = NE::Model::Topology::RoutingTypes::GRAPH_ROUTING;
    }
    else if (routingType == std::string("graphroutingwithminpath")) {
        RoutingType = NE::Model::Topology::RoutingTypes::GRAPH_ROUTING_WITH_MIN_PATH_SELECTION;
    }

    Parse("NETWORK_MANAGER.SETTINGS_FACTOR", settingsFactor, options);
    Parse("NETWORK_MANAGER.TOPOLOGY_FACTOR", topologyFactor, options);
    Parse("NETWORK_MANAGER.NETWORK_MANAGER_MIN_RANGE_PORT", listenNetworkManagerMinRangePort, options);
    Parse("NETWORK_MANAGER.NETWORK_MANAGER_MAX_RANGE_PORT", listenNetworkManagerMaxRangePort, options);

    checkEdgeVisibility = ParseBool("NETWORK_MANAGER.CHECK_EDGE_VISIBILITY", options);
    Parse("NETWORK_MANAGER.NO_ALLOWED_ALARMS", noAllowedAlarms, options);
    Parse("NETWORK_MANAGER.CEV_MAX_FLOWS", cevMaxFlows, options);
    Parse("NETWORK_MANAGER.VISIBLE_EDGE_TIMEOUT", visibileEdgeTimeOut, options);

    activateDeviceAlarmCheck = ParseBool("NETWORK_MANAGER.ACTIVATE_DEVICE_ALARM_CHECK", options);
    enableShortProxy = ParseBool("NETWORK_MANAGER.ENABLE_SHORT_PROXY", options);
    Parse("NETWORK_MANAGER.MAX_ALARMS_NO_BEFORE_CHECK_DEVICE", maxAlarmsNoBeforeCheckDevice, options);
    Parse("NETWORK_MANAGER.ALARMS_TIME_INTERVAL_BEFORE_CHECK_DEVICE", alarmsTimeIntervalBeforeCheckDevice, options);

    Parse("NETWORK_MANAGER.MAX_JOIN_PRIORITY_IN_PROGRESS_ON_PARENT", maxJoinPriorityInProgressOnParent, options);
    Parse("NETWORK_MANAGER.MIN_SERVICE_PERIOD", minServicePeriod, options);
    Parse("NETWORK_MANAGER.CHANNEL_MAP", channelMap, options, true);

    Parse("NETWORK_MANAGER.KEEP_ALIVE_PERIOD", keepAlivePeriod, options);

    minServicePeriod = ComputeClampedMinPublishPeriod(minServicePeriod);

    LOG_DEBUG("Computed MinServicePeriod:" << minServicePeriod);

    activateMockKeyGenerator = ParseBool("NETWORK_MANAGER.ACTIVATE_MOCK_KEY_GENERATOR", options);
    handleAllReponseCodesAsError = ParseBool("NETWORK_MANAGER.INTERPRET_ERROR_RESPONSECODES", options);
    sendNotificationsOnJoinFlow = ParseBool("NETWORK_MANAGER.SEND_NOTIFICATIONS_ON_JOIN", options);
    useWaves = ParseBool("NETWORK_MANAGER.WAVE_DEPENDENCY", options);


    Parse("NETWORK_MANAGER.TRANSCEIVER_ADVERTISE_LINKS_NO", transceiverAdvertiseLinksNo, options);

    enableApBestFitAllocation = ParseBool("NETWORK_MANAGER.ENABLE_AP_BEST_FIT_ALLOCATION", options);

    deviceJoinAfterAllOperationsConfirms = ParseBool("NETWORK_MANAGER.DEVICE_JOIN_AFTER_ALL_OPERATIONS_CONFIRMS", options);

    Parse("NETWORK_MANAGER.MAX_HOPS_ALLOWED", maxHops, options);

    Parse("NETWORK_MANAGER.MAX_NEIGHBORS_TO_EVALUATE", maxNeighbors, options);

    Parse("NETWORK_MANAGER.HOPS_FACTOR", hopsFactor, options);

    generateNicknamesFromEUI64 = ParseBool("NETWORK_MANAGER.GENERATE_NICKNAMES_FROM_EUI64", options);

    Parse("NETWORK_MANAGER.NICKNAMES_FILE_NAME", nicknamesFileName, options);

    Parse("NETWORK_MANAGER.WEBLOGGER_IP", webLoggerIp, options);

    Parse("NETWORK_MANAGER.PERCENT_TRAFFIC_OVERALLOC", percentTrafficMaxOverAlloc, options);

    Parse("NETWORK_MANAGER.MAX_PER_ALLOWED", perThreshold, options);

    Parse("NETWORK_MANAGER.MAX_TR_NEIGHBORS", maxTRNeighbors, options);

    Parse("NETWORK_MANAGER.MAX_DEVICE_NEIGHBORS", maxDeviceNeighbors, options);

    useSlotZero = ParseBool("NETWORK_MANAGER.USE_SLOT_ZERO", options);

    Parse("NETWORK_MANAGER.MAX_JOIN_ATTEMPTS_BEFORE_OVERRIDE", maxJoinAttemptsBeforeOverride, options);

    // validate
    if (maxJoinsInProgressPerParent > maxJoinsPerDevice)
    {
        LOG_WARN("maxJoinsInProgressPerParent > maxJoinsPerDevice => maxJoinsInProgressPerParent = maxJoinsPerDevice");
        maxJoinsInProgressPerParent = maxJoinsPerDevice;
    }

    percentCostLower = 20;
}

/**
 * Parse channelsText and returns its content as a vector of Uint8. The channelsText must be in the following format:
 * ch1,ch2,ch3,...,chn. Each channel (ch1..chn) must be in the range: 0-15 inclusive.
 * Ex: 8,11,15,2
 */
std::vector<Uint8> NMSettingsLogic::getAsChannelsVector(std::string channelsText)
{
    // split the input line into tokens using the comma separator
    tokenizer tokens(channelsText, boost::char_separator<char>(","));
    std::vector<Uint8> channelsVector;

    for (tokenizer::const_iterator iter = tokens.begin(); iter != tokens.end(); ++iter)
    {
        Uint8 channel;
        try
        {
            string value = (*iter);
            value = trimSpaces(value);
            channel = (Uint8) boost::lexical_cast<int>(value);
        }
        catch (boost::bad_lexical_cast & ex)
        {
            std::ostringstream stream;
            stream << "Initial channels list contains values outside the range 0-15: " << ex.what();
            throw NEException(stream.str());
        }
        if (channel > 15)
        {
            throw NEException("Initial channels list contains values outside the range 0-15.");
        }
        channelsVector.push_back(channel);
    }

    return channelsVector;
}

string NMSettingsLogic::trimSpaces(const string& str)
{
    // Trim Both leading and trailing spaces
    size_t startpos = str.find_first_not_of(" \t'"); // Find the first character position after excluding leading blank spaces and '
    size_t endpos = str.find_last_not_of(" \t'"); // Find the first character position from reverse af and '

    // if all spaces or empty return an empty string
    if ((string::npos == startpos) || (string::npos == endpos))
    {
        return "";
    }
    else
    {
        return str.substr(startpos, endpos - startpos + 1);
    }
}

string NMSettingsLogic::cleanString(string& str) {
    std::string s = std::string(str.c_str());
    boost::erase_all(s, " ");
    boost::to_lower(s);
    return s;
}

void NMSettingsLogic::updateProvisioningByDeviceType(SimpleIni::CSimpleIniCaseA::TNamesDepend iniItems,
                                                     ProvisioningItem::DeviceType deviceType)
{

    SimpleIni::CSimpleIniCaseA::TNamesDepend::const_iterator it = iniItems.begin();
    for (; it != iniItems.end(); ++it)
    {
        Address64 ip64;
        NE::Model::SecurityKey key;
        try
        {
            ParseProvisioningLine((*it).pItem, ip64, key);
        }
        catch (const NE::Common::NEException& ex)
        {
            LOG_ERROR("In " << customIniFileName << ", option SECURITY_MANAGER.DEVICE, item no " << (*it).nOrder
                        << ": " << ex.what());
            throw NEException("Bad option in config file.");
        }

        //hart7::security::SecurityManager::instance().addProvisioningKey(ip64, key);
        provisioningKeys[ip64] = key;
        if (deviceType == ProvisioningItem::BACKBONE)
        {
            backbones.push_back(ip64);
        }
        else if (deviceType == ProvisioningItem::DEVICE)
        {
            devices.push_back(ip64);
        }
        else if (deviceType == ProvisioningItem::GATEWAY)
        {
            gatewayAddress64 = ip64;
        }

        LOG_DEBUG("Added provisioning " << ip64.toString() << ": " << key.toString());
    }
}

void NMSettingsLogic::ParseCustomConfigFile(SimpleIni::CSimpleIniCaseA& iniParser)
{
    SimpleIni::CSimpleIniCaseA::TNamesDepend network;
    SimpleIni::CSimpleIniCaseA::TNamesDepend devices;
    SimpleIni::CSimpleIniCaseA::TNamesDepend gateways;
    SimpleIni::CSimpleIniCaseA::TNamesDepend backbones;

    iniParser.GetAllValues("SECURITY_MANAGER", "NETWORK", network);
    iniParser.GetAllValues("SECURITY_MANAGER", "DEVICE", devices);
    iniParser.GetAllValues("SECURITY_MANAGER", "BACKBONE", backbones);
    iniParser.GetAllValues("SECURITY_MANAGER", "GATEWAY", gateways);

    provisioningKeys.clear();
    this->devices.clear();
    this->backbones.clear();

    updateProvisioningByDeviceType(devices, ProvisioningItem::DEVICE);
    updateProvisioningByDeviceType(gateways, ProvisioningItem::GATEWAY);
    updateProvisioningByDeviceType(backbones, ProvisioningItem::BACKBONE);

    if (1 != network.size())
    {
        //throw NE::Common::NEException("Provisioning: NETWORK option corrupted.");
        uniqueKey = false;
    } else {
        SimpleIni::CSimpleIniCaseA::TNamesDepend::const_iterator it_network = network.begin();
        ParseProvisioningNetworkLine(it_network->pItem, NetworkUniqueKey, uniqueKey);
    }


    //    SimpleIni::CSimpleIniCaseA::TNamesDepend::const_iterator it = devices.begin();
    //    for (; it != devices.end(); ++it) {
    //        Address64 ip64;
    //        NE::Model::SecurityKey key;
    //        Uint16 subnetMask  = 0;
    //        try {
    //            //ParseProvisioningLine((*it).pItem, ip64, key, subnetMask);
    //            ParseProvisioningLine((*it).pItem, ip64, key);
    //        } catch (const NE::Common::NEException& ex) {
    //            LOG_ERROR("In " << customIniFileName << ", option SECURITY_MANAGER.DEVICE, item no "
    //                        << (*it).nOrder << ": " << ex.what());
    //            throw NEException("Bad option in config file.");
    //
    //        }
    //
    //        //hart7::security::SecurityManager::instance().addProvisioningKey(ip64, key);
    //        provisioningKeys[ip64] = key;
    //        LOG_DEBUG("Added provisioning " << ip64.toString() << ": " << key.toString()
    //                    << ": " << subnetMask);
    //    }
}

void NMSettingsLogic::ParseProvisioningNetworkLine(const string& p_line, NE::Model::SecurityKey& p_key, bool &flag)
{
    // split the input line into tokens using the comma separator

    tokenizer tokens(p_line, boost::char_separator<char>(","));

    tokenizer::const_iterator tok_iter = tokens.begin();
    int dist = distance(tok_iter, tokens.end());
    if (dist < 2)
    {
        throw NE::Common::NEException("missing elements: need comma separated 64 bit address, security key");
    }

    // read the 64 bit address
    try
    {
        p_key.loadString(trimSpaces(*tok_iter).c_str());
    }
    catch (const InvalidArgumentException& ex)
    {
        throw NEException(string("security key: ") + ex.what());
    }


    std::string option = std::string((++tok_iter)->c_str());
    option = trimSpaces(option);
    flag = ("true" == option);
}


void NMSettingsLogic::ParseProvisioningLine(const string& p_line, Address64& p_ip64, NE::Model::SecurityKey& p_key)
{
    // split the input line into tokens using the comma separator

    tokenizer tokens(p_line, boost::char_separator<char>(","));

    tokenizer::const_iterator tok_iter = tokens.begin();
    int dist = distance(tok_iter, tokens.end());
    if (dist < 2)
    {
        throw NE::Common::NEException("missing elements: need comma separated 64 bit address, security key");
    }

    // read the address
    try
    {
        p_ip64.loadString(*tok_iter);
    }
    catch (const InvalidArgumentException& ex)
    {
        throw NEException(string("invalid 64 bit address: ") + ex.what());
    }

    // read the security key
    try
    {
        p_key.loadString(trimSpaces(*++tok_iter).c_str());
    }
    catch (const InvalidArgumentException& ex)
    {
        throw NEException(string("security key: ") + ex.what());
    }
}
//
//void NMSettingsLogic::ParseAddrPortLine(const string& p_line, NE::Network::Endpoint& ipv4Enpoint) {
//    // split the input line into tokens using the comma separator
//    tokenizer tokens(p_line, boost::char_separator<char>(","));
//    if (distance(tokens.begin(), tokens.end()) < 2) {
//        LOG_ERROR("Throw exception : Missing elements: need comma separated IPv4, port.");
//        throw NE::Common::NEException("Missing elements: need comma separated IPv4, port.");
//    }
//
//    tokenizer::const_iterator tok_iter = tokens.begin();
//
//    // read the IPv4 address
//    ipv4Enpoint.ipv4 = *tok_iter;
//
//    // read the port
//    try {
//        ipv4Enpoint.port = boost::lexical_cast<int>(*(++tok_iter));
//    } catch (boost::bad_lexical_cast &) {
//        throw NE::Common::NEException("invalid port value");
//    }
//}
//
//void NMSettingsLogic::ParseAddrPortLine(const string& p_line, NE::Network::Endpoint& ipv4Enpoint, Uint16& subnetId) {
//    // split the input line into tokens using the comma separator
//    tokenizer tokens(p_line, boost::char_separator<char>(","));
//
//    if (distance(tokens.begin(), tokens.end()) < 3) {
//        LOG_ERROR("Throw exception : Missing elements: need comma separated IPv4, subnetId, port.");
//        throw NE::Common::NEException("Missing elements: need comma separated IPv4, subnetId, port.");
//    }
//
//    tokenizer::const_iterator tok_iter = tokens.begin();
//
//    // read the IPv4 address
//    ipv4Enpoint.ipv4 = *tok_iter;
//
//    // read the port
//    try {
//        ipv4Enpoint.port = boost::lexical_cast<int>(*(++tok_iter));
//    } catch (boost::bad_lexical_cast &) {
//        throw NE::Common::NEException("invalid port value");
//    }
//
//    // read the subnetId
//    try {
//        subnetId = boost::lexical_cast<int>(*(++tok_iter));
//    } catch (boost::bad_lexical_cast &) {
//        throw NE::Common::NEException("invalid subnetId value");
//    }
//
//}
//
//void NMSettingsLogic::ParseAddrPortLine(const string& p_line, NE::Network::Endpoint& ipv4Enpoint,
//            Address64& p_address64) {
//    // split the input line into tokens using the comma separator
//    tokenizer tokens(p_line, boost::char_separator<char>(","));
//    if (distance(tokens.begin(), tokens.end()) < 2) {
//        LOG_ERROR("Throw exception : Missing elements: need comma separated IPV4, PORT, EUID.");
//        throw NE::Common::NEException("Missing elements: need comma separated IPV4, PORT, EUID.");
//    }
//
//    tokenizer::const_iterator tok_iter = tokens.begin();
//
//    // read the IPv4 address
//    ipv4Enpoint.ipv4 = *tok_iter;
//
//    // read the port
//    try {
//        ipv4Enpoint.port = boost::lexical_cast<int>(*(++tok_iter));
//    } catch (boost::bad_lexical_cast &) {
//        throw NE::Common::NEException("invalid port value");
//    }
//
//    // read the EUID address
//    try {
//        p_address64.loadString(*(++tok_iter));
//    } catch (const InvalidArgumentException& ex) {
//        throw NEException(string("invalid EUID address: ") + ex.what());
//    }
//}
NMSettingsLogic& NMSettingsLogic::instance()
{
    static NMSettingsLogic instance;
    return instance;
}

void NMSettingsLogic::LoadProvisioning()
{
    SimpleIni::CSimpleIniCaseA customIniParser(false, true);
    if (customIniParser.LoadFile(CUSTOM_INI_FILE) == SimpleIni::SI_OK)
    {
        customIniFileName = CUSTOM_INI_FILE;
    }
    else
    {
        if (customIniParser.LoadFile(CUSTOM_INI_FILE_NAME) == SimpleIni::SI_OK)
        {
            customIniFileName = CUSTOM_INI_FILE_NAME;
        }
        else
        {
            LOG_ERROR("Could not open configuration file " CUSTOM_INI_FILE_NAME);
            throw NEException("Could not open configuration file.");
        }
    }
    ParseCustomConfigFile(customIniParser);

}

void NMSettingsLogic::LoadConfig()
{
    SimpleIni::CSimpleIniCaseA commonIniParser;
    if (commonIniParser.LoadFile(COMMON_INI_FILE) == SimpleIni::SI_OK)
    {
        commonIniFileName = COMMON_INI_FILE;
    }
    else
    {
        if (commonIniParser.LoadFile(COMMON_INI_FILE_NAME) == SimpleIni::SI_OK)
        {
            commonIniFileName = COMMON_INI_FILE_NAME;
        }
        else
        {
            LOG_ERROR("Could not open configuration file " COMMON_INI_FILE_NAME);
            throw NEException("Could not open configuration file.");
        }
    }
    ParseCommonConfigFile(commonIniParser);
}

} // namespace
} // namespace
