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

#ifndef DEVICE_H_
#define DEVICE_H_

#include <iomanip>
#include <time.h>
#include "Common/NEAddress.h"
#include "Common/NETypes.h"
#include "Common/logging.h"
#include "Model/Topology/TopologyTypes.h"
#include "Model/MetaDataAttributes.h"
#include "Model/Capabilities.h"
#include <stdint.h>
#include <deque>

using namespace NE::Model::Topology;

namespace NE {
namespace Model {

#define MAX_REJOINREASON_SIZE 5

namespace DeviceStatus {
enum DeviceStatus {
    NOT_JOINED = 1, JOIN_REQUEST_RECEIVED = 2, QUARANTINE = 3, OPERATIONAL = 4, MARKED_FOR_REMOVE = 5, DELETED = 6
};

/**
 * Returns the name of the status.
 */
inline std::string toString(DeviceStatus status) {

    if (status == DeviceStatus::NOT_JOINED) {
        return "NOT_JOINED";
    } else if (status == DeviceStatus::JOIN_REQUEST_RECEIVED) {
        return "JOIN_REQ_RCV";
    } else if (status == DeviceStatus::QUARANTINE) {
        return "QUARANTINE";
    } else if (status == DeviceStatus::OPERATIONAL) {
        return "OPERATIONAL";
    } else if (status == DeviceStatus::MARKED_FOR_REMOVE) {
        return "MARK_FOR_DEL";
    } else if (status == DeviceStatus::DELETED) {
        return "DELETED";
    } else {
        return "UNKNOWN";
    }
}

}

namespace DeviceAction {
enum DeviceAction {
    NOT_SET = 1,
    JOIN_REQUEST = 2,
    JOIN_REQUEST_SUCCESS = 3,
    JOIN_REQUEST_FAIL = 4,
    ROUTER = 5,
    ROUTER_SUCCESS = 6,
    ROUTER_FAIL = 7,
    SERVICE_IN = 8,
    SERVICE_IN_SUCCESS = 9,
    SERVICE_IN_FAIL = 10,
    SERVICE_OUT = 11,
    SERVICE_OUT_SUCCESS = 12,
    SERVICE_OUT_FAIL = 13,
    ATTACH_IN = 14,
    ATTACH_IN_SUCCESS = 15,
    ATTACH_IN_FAIL = 16,
    ATTACH_OUT = 17,
    ATTACH_OUT_SUCCESS = 18,
    ATTACH_OUT_FAIL = 19,
    DISCOVERY_IN = 20,
    DISCOVERY_IN_SUCCESS = 21,
    DISCOVERY_IN_FAIL = 22,
    DISCOVERY_OUT = 23,
    DISCOVERY_OUT_SUCCESS = 24,
    DISCOVERY_OUT_FAIL = 25,
    AFFECTED_IN = 26,
    AFFECTED_IN_SUCCESS = 27,
    AFFECTED_IN_FAIL = 28,
    AFFECTED_OUT = 29,
    AFFECTED_OUT_SUCCESS = 30,
    AFFECTED_OUT_FAIL = 31,
    SERVICE_END = 32,
    SERVICE_END_SUCCESS = 33,
    SERVICE_END_FAIL = 34,
    REMOVE = 35,
    REMOVE_SUCCESS = 36,
    REMOVE_FAIL = 37,
    CHECK_PRESENCE = 38,
    CHECK_PRESENCE_SUCCESS = 39,
    CHECK_PRESENCE_FAIL = 40
};

inline std::string toString(DeviceAction action) {
    if (action == DeviceAction::NOT_SET) {
        return "NOT_SET";
    } else if (action == DeviceAction::JOIN_REQUEST) {
        return "JOIN_REQUEST";
    } else if (action == DeviceAction::JOIN_REQUEST_SUCCESS) {
        return "JOIN_REQUEST_OK";
    } else if (action == DeviceAction::JOIN_REQUEST_FAIL) {
        return "JOIN_REQUEST_FAIL";
    } else if (action == DeviceAction::ROUTER) {
        return "ROUTER";
    } else if (action == DeviceAction::ROUTER_SUCCESS) {
        return "ROUTER_OK";
    } else if (action == DeviceAction::ROUTER_FAIL) {
        return "ROUTER_FAIL";
    } else if (action == DeviceAction::SERVICE_IN) {
        return "SERV_IN";
    } else if (action == DeviceAction::SERVICE_IN_SUCCESS) {
        return "SERV_IN_OK";
    } else if (action == DeviceAction::SERVICE_IN_FAIL) {
        return "SERV_IN_FAIL";
    } else if (action == DeviceAction::SERVICE_OUT) {
        return "SERV_OUT";
    } else if (action == DeviceAction::SERVICE_OUT_SUCCESS) {
        return "SERV_OUT_OK";
    } else if (action == DeviceAction::SERVICE_OUT_FAIL) {
        return "SERV_OUT_FAIL";
    } else if (action == DeviceAction::ATTACH_IN) {
        return "ATTACH_IN";
    } else if (action == DeviceAction::ATTACH_IN_SUCCESS) {
        return "ATTACH_IN_OK";
    } else if (action == DeviceAction::ATTACH_IN_FAIL) {
        return "ATTACH_IN_FAIL";
    } else if (action == DeviceAction::ATTACH_OUT) {
        return "ATTACH_OUT";
    } else if (action == DeviceAction::ATTACH_OUT_SUCCESS) {
        return "ATTACH_OUT_OK";
    } else if (action == DeviceAction::ATTACH_OUT_FAIL) {
        return "ATTACH_OUT_FAIL";
    } else if (action == DeviceAction::DISCOVERY_IN) {
        return "DISCOVERY_IN";
    } else if (action == DeviceAction::DISCOVERY_IN_SUCCESS) {
        return "DISCOVERY_IN_OK";
    } else if (action == DeviceAction::DISCOVERY_IN_FAIL) {
        return "DISCOVERY_IN_FAIL";
    } else if (action == DeviceAction::DISCOVERY_OUT) {
        return "DISCOVERY_OUT";
    } else if (action == DeviceAction::DISCOVERY_OUT_SUCCESS) {
        return "DISCOVERY_OUT_OK";
    } else if (action == DeviceAction::DISCOVERY_OUT_FAIL) {
        return "DISCOVERY_OUT_FAIL";
    } else if (action == DeviceAction::AFFECTED_IN) {
        return "AFFECTED_IN";
    } else if (action == DeviceAction::AFFECTED_IN_SUCCESS) {
        return "AFFECTED_IN_OK";
    } else if (action == DeviceAction::AFFECTED_IN_FAIL) {
        return "AFFECTED_IN_FAIL";
    } else if (action == DeviceAction::AFFECTED_OUT) {
        return "AFFECTED_OUT";
    } else if (action == DeviceAction::AFFECTED_OUT_SUCCESS) {
        return "AFFECTED_OUT_OK";
    } else if (action == DeviceAction::AFFECTED_OUT_FAIL) {
        return "AFFECTED_OUT_FAIL";
    } else if (action == DeviceAction::SERVICE_END) {
        return "SERV_END";
    } else if (action == DeviceAction::SERVICE_END_SUCCESS) {
        return "SERV_END_OK";
    } else if (action == DeviceAction::SERVICE_END_FAIL) {
        return "SER_END_FAIL";
    } else if (action == DeviceAction::REMOVE) {
        return "REMOVE";
    } else if (action == DeviceAction::REMOVE_SUCCESS) {
        return "REMOVE_OK";
    } else if (action == DeviceAction::REMOVE_FAIL) {
        return "REMOVE_FAIL";
    } else if (action == DeviceAction::CHECK_PRESENCE) {
        return "CHECK";
    } else if (action == DeviceAction::CHECK_PRESENCE_SUCCESS) {
        return "CHECK_OK";
    } else if (action == DeviceAction::CHECK_PRESENCE_FAIL) {
        return "CHECK_FAIL";
    } else {
        return "UNKNOWN";
    }

}
}

struct DeviceHistory {

        // the time of the last join
        time_t lastJoinTime;

        std::string lastJoinTimeStr;

        // the total number of commands sent to a device;
        Uint32 cmdsNo;

        // the minimum time a package for a device has spent into the network (the time from send to confirm);
        time_t pckgMinTime;

        // the maximum time a package  in bytes has spent into the network (the time from send to confirm);
        time_t pckgMaxTime;

        // the total number of packages sent to a device;
        uint32_t pckgCounter;

        // the number milliseconds spent into the network by all the packages sent to a device;
        uint64_t allPckgsTime;

        // the minimum package in bytes size sent to a device;
        char pckgMinSize;

        // the maximum package in bytes size sent to a device;
        char pckgMaxSize;

        // the number of bytes occupied by all the packages sent to a device;
        uint64_t allPckgsSize;

        // the time the the last package for a device have been sent;
        time_t lastPckgSentTime;

        DeviceHistory() {
            lastJoinTime = 0; lastJoinTimeStr = "";
            cmdsNo = 0;
            pckgMinTime = 60 * 60 * 24; // one day
            pckgMaxTime = 0;
            pckgCounter = 0;
            allPckgsTime = 0;
            pckgMinSize = 127;
            pckgMaxSize = 0;
            allPckgsSize = 0;
            lastPckgSentTime = 0;
        }

        /**
         * A new packages has been sent into the network. Must update the counters.
         */
        void sentPackage(char lastPackageSize, char cmdsSize) {

            cmdsNo += cmdsSize;

            if (pckgMinSize > lastPackageSize) {
                pckgMinSize = lastPackageSize;
            }

            if (pckgMaxSize < lastPackageSize) {
                pckgMaxSize = lastPackageSize;
            }

            allPckgsSize += lastPackageSize;

            lastPckgSentTime = time(NULL);
        }

        /**
         * Called when a confirm is received in Queue.
         */
        void confirmLastPackage() {
            time_t curTime = time(NULL);
            time_t timeSpent = curTime - lastPckgSentTime;
            // the time is in seconds and it is possible that a package returns faster than a second and the timeSpent could be 0.
            timeSpent = timeSpent == 0 ? 1 : timeSpent; // avoid 0 seconds spent into the network; consider a second.

            if (pckgMinTime > timeSpent) {
                pckgMinTime = timeSpent;
            }

            if (pckgMaxTime < timeSpent) {
                pckgMaxTime = timeSpent;
            }

            allPckgsTime += timeSpent;
            ++pckgCounter;
        }

        static std::ostringstream& toHeaderString(std::ostringstream& stream) {
            stream << std::setw(10) << "cmdsNo";
            stream << std::setw(10) << "pckgsNo";
            stream << std::setw(10) << "PckgsSize";
            stream << std::setw(22) << " Pckg_Size_Min/Med/Max";
            stream << std::setw(30) << "Pckg_Time_Min / Med / Max";
            stream << std::setw(20) << " lastPckgSentTime";
            stream << std::setw(4) << "sec";

            return stream;
        }

        std::ostringstream& toString(std::ostringstream& stream) {
            stream << std::setw(10) << cmdsNo;
            stream << std::setw(10) << pckgCounter;
            stream << std::setw(10) << allPckgsSize;

            stream << std::setw(6) << (int) pckgMinSize << " ";
            stream << std::setw(6) << (int) ((pckgCounter != 0) ? allPckgsSize / pckgCounter : 0) << "  ";
            stream << std::setw(6) << (int) pckgMaxSize << " ";

            stream << std::setw(9) << pckgMinTime << " ";
            stream << std::setw(9) << (uint64_t) ((pckgCounter != 0) ? allPckgsTime / pckgCounter : 0) << " ";
            stream << std::setw(9) << pckgMaxTime << " ";

            struct tm * ptm = gmtime(&lastPckgSentTime);
            char buffer[33];
            buffer[32] = 0;
            // Format: Mo, 15.06.2009 20:20:00
            strftime(buffer, 32, "%d.%m.%Y %H:%M:%S", ptm);

            stream << std::setw(20) << buffer; //lastPckgSentTime;

            if (lastPckgSentTime > 0) {
                time_t curTime = time(NULL);
                time_t timeSpent = curTime - lastPckgSentTime;
                stream << " " << std::setw(3) << (int) timeSpent << "";
            }

            return stream;
        }

        void setLastJoinTime(time_t lastJoinTime_)
        {
            lastJoinTime = lastJoinTime_;
            struct tm * ptm = gmtime(&lastJoinTime);
            char buffer[33];
            buffer[32] = 0;
            // Format: 15.06.2009 20:20:00
            strftime(buffer, 32, "%d.%m %H:%M:%S", ptm);
            std::ostringstream stream;
            stream << std::setw(16) << buffer; //lastJoinTime;
            lastJoinTimeStr = stream.str();
        }
};

enum PowerSupplyStatus {

};

class Device {
        LOG_DEF("I.M.Device");

        MetaDataAttributesPointer metaDataAttributes;

    public:

#ifdef JOIN_REASON
        struct RejoinReason {
            uint8_t joinReason[4];
        };
#endif
        Address64 address64;

        Address32 address32;

        Address32 joinedBackbone32;

        Address32 parent32;

        bool deviceRequested832;

        // see table 62
        Uint16 deviceSchedulingFlags;

        Capabilities capabilities;

        DeviceStatus::DeviceStatus status;

        DeviceAction::DeviceAction action;

        DeviceHistory deviceHistory;

        Uint32 DPDUsTransmitted;

        Uint32 DPDUsReceived;

        Uint32 DPDUsFailedTransmission;

        Uint32 DPDUsFailedReception;

        Uint32 startTime;

        Uint8 powerSupplyStatus;

        bool onEvaluation;

#ifdef JOIN_REASON
        std::deque<RejoinReason> rejoinReasons;
#endif

        Device() :
            parent32(0) {
            setDiagnostics(0, 0, 0, 0);
            deviceSchedulingFlags = 0;
        }

        Device(const Capabilities& deviceCapabilities) :
            address64(deviceCapabilities.euidAddress), parent32(0), capabilities(deviceCapabilities), status(
                        DeviceStatus::NOT_JOINED), action(DeviceAction::NOT_SET) {

            if (capabilities.isGateway()) {
                deviceRequested832 = true;
            } else {
                deviceRequested832 = false;
            }

            metaDataAttributes.reset(new MetaDataAttributes());
            metaDataAttributes->init(deviceCapabilities);

            deviceSchedulingFlags = 0;

            setDiagnostics(0, 0, 0, 0);
        }

        void setCapabilities(const Capabilities& deviceCapabilities) {
            capabilities = deviceCapabilities;
        }

        void setDiagnostics(Uint8 transmited, Uint8 received, Uint8 failedTransmission, Uint8 failedReception) {
            DPDUsTransmitted = transmited;

            DPDUsReceived = received;

            DPDUsFailedTransmission = failedTransmission;

            DPDUsFailedReception = failedReception;

        }

        void updateDiagnostics(Uint8 transmited, Uint8 received, Uint8 failedTransmission, Uint8 failedReception) {
            DPDUsTransmitted += transmited;

            DPDUsReceived += received;

            DPDUsFailedTransmission += failedTransmission;

            DPDUsFailedReception += failedReception;

        }

#ifdef JOIN_REASON
        void setJoinReason(uint8_t joinReason[4]) {
            rejoinReasons.push_front(RejoinReason());
            memcpy(rejoinReasons.front().joinReason, joinReason, 4);

            while (rejoinReasons.size() > MAX_REJOINREASON_SIZE) {
                rejoinReasons.pop_back();
            }
        }
#endif
        void setPowerSupplyStatus(Uint8 _powerSupplyStatus) {
            powerSupplyStatus = _powerSupplyStatus;
        }

        bool isUdp() {
            return capabilities.isBackbone() || capabilities.isGateway() || capabilities.isManager();
        }

        bool isJoinConfirmed() {
            return status == DeviceStatus::QUARANTINE || status == DeviceStatus::OPERATIONAL;
        }

        bool isOperational() {
            return status == DeviceStatus::OPERATIONAL;
        }

        bool isNewStatus() {
            return status == DeviceStatus::JOIN_REQUEST_RECEIVED;
        }

        bool isRemoveStatus() {
            return status == DeviceStatus::MARKED_FOR_REMOVE;
        }

        bool isDeletedStatus() {
            return status == DeviceStatus::DELETED;
        }

        static DeviceAction::DeviceAction getAction(EvaluatePathPriority::EvaluatePathPriority evalPriority) {

            if (evalPriority == EvaluatePathPriority::OutboundService) {
                return DeviceAction::SERVICE_OUT;
            } else if (evalPriority == EvaluatePathPriority::InboundService) {
                return DeviceAction::SERVICE_IN;
            } else if (evalPriority == EvaluatePathPriority::OutboundAttach) {
                return DeviceAction::ATTACH_OUT;
            } else if (evalPriority == EvaluatePathPriority::InboundAttach) {
                return DeviceAction::ATTACH_IN;
            } else if (evalPriority == EvaluatePathPriority::OutboundDiscovery) {
                return DeviceAction::DISCOVERY_OUT;
            } else if (evalPriority == EvaluatePathPriority::InboundDiscovery) {
                return DeviceAction::DISCOVERY_IN;
            } else if (evalPriority == EvaluatePathPriority::OutboundAffected) {
                return DeviceAction::AFFECTED_OUT;
            } else if (evalPriority == EvaluatePathPriority::InboundAffected) {
                return DeviceAction::AFFECTED_IN;
            } else {
                return DeviceAction::NOT_SET;
            }
        }

        DeviceAction::DeviceAction getAction() {
            return action;
        }

        void setAction(DeviceAction::DeviceAction action_) {
            LOG_DEBUG("setAction - device=" << ToStr(address32) << ", old deviceAction=" << DeviceAction::toString(
                        action) << ", new action=" << DeviceAction::toString(action_));
            action = action_;
        }

        MetaDataAttributesPointer getMetaDataAttributes() {
            return metaDataAttributes;
        }

        void updateAction(NetworkEngineEventType::NetworkEngineEventTypeEnum eventType, bool checkRemovedDevices) {
            LOG_DEBUG("updateAction - device " << ToStr(address32) << ", eventType=" << (int) eventType
                        << ", checkRemovedDevices=" << (int) checkRemovedDevices);
            LOG_DEBUG("updateAction - old deviceAction=" << DeviceAction::toString(action));

            if (status == DeviceStatus::DELETED) {
                LOG_DEBUG("updateAction - skip deleted");
                return;
            }

            if (eventType == NetworkEngineEventType::JOIN_REQUEST) {
                if (checkRemovedDevices) {
                    action = DeviceAction::JOIN_REQUEST_FAIL;
                } else {
                    action = DeviceAction::JOIN_REQUEST_SUCCESS;

                    if (!capabilities.isDevice()) {
                        action = DeviceAction::ROUTER_SUCCESS;
                    }
                }
            } else if (eventType == NetworkEngineEventType::NONE) {
                if (checkRemovedDevices) {
                    action = (DeviceAction::DeviceAction) (action + 2);
                } else {
                    //DO NOTHING
                }
            } else {

                if (checkRemovedDevices) {
                    action = (DeviceAction::DeviceAction) (action + 2);
                } else {
                    if (action == DeviceAction::SERVICE_IN) {
                        status = DeviceStatus::OPERATIONAL;
                    }

                    action = (DeviceAction::DeviceAction) (action + 1);
                }

            }

            LOG_DEBUG("updateAction - new deviceAction=" << DeviceAction::toString(action));
        }

        bool isOnEvaluation() {
            return onEvaluation;
        }

        void setOnEvaluation(bool onEvaluation_) {
            onEvaluation = onEvaluation_;
        }

        void setMetaDataAttributes(MetaDataAttributes& _metaDataAttributes) {
            LOG_DEBUG("setMetaDataAttributes - device " << address64.toString() << "  metadata:"
                        << _metaDataAttributes.toString());

            metaDataAttributes->setJoinPriority(_metaDataAttributes.getJoinPriority());

            if (metaDataAttributes->getTotalServices() > _metaDataAttributes.getTotalServices()
                        || metaDataAttributes->getTotalDiagnostics() > _metaDataAttributes.getTotalDiagnostics()
                        || metaDataAttributes->getTotalGraphs() > _metaDataAttributes.getTotalGraphs()
                        || metaDataAttributes->getTotalLinks() > _metaDataAttributes.getTotalLinks()
                        || metaDataAttributes->getTotalGraphNeighbors() > _metaDataAttributes.getTotalGraphNeighbors()
                        || metaDataAttributes->getTotalRoutes() > _metaDataAttributes.getTotalRoutes()
                        || metaDataAttributes->getTotalSuperframes() > _metaDataAttributes.getTotalSuperframes()) {
                LOG_ERROR("setMetaDataAttributes - curentValues=" << metaDataAttributes->toString() << "  new values="
                            << _metaDataAttributes.toString());
                return;
            }

            metaDataAttributes->setTotalServices(_metaDataAttributes.getTotalServices());
            metaDataAttributes->setTotalDiagnostics(_metaDataAttributes.getTotalDiagnostics());
            metaDataAttributes->setTotalGraphs(_metaDataAttributes.getTotalGraphs());
            metaDataAttributes->setTotalLinks(_metaDataAttributes.getTotalLinks());
            metaDataAttributes->setTotalGraphNeighbors(_metaDataAttributes.getTotalGraphNeighbors());
            metaDataAttributes->setTotalRoutes(_metaDataAttributes.getTotalRoutes());
            metaDataAttributes->setTotalSuperframes(_metaDataAttributes.getTotalSuperframes());
        }

        void setMetaDataAttributesUsedSuperframes(Uint16 usedSuperFrames) {
            metaDataAttributes->setUsedSuperframes(usedSuperFrames);
        }

        void setMetaDataAttributesUsedDiagnostics(Uint16 usedDiagnostics) {
            metaDataAttributes->setUsedDiagnostics(usedDiagnostics);
        }

        void toString(std::ostringstream& stream) {
            stream << "Device {";
            stream << "address32=" << ToStr(address32);
            stream << ", address64=" << address64.toString();
            stream << ", ";
            capabilities.toString(stream);
            stream << ", status=";
            stream << DeviceStatus::toString(status);
            stream << "}";
        }

        friend std::ostream& operator<<(std::ostream& stream, const Device& device) {
            stream << "Device {";
            stream << "address32=" << ToStr(device.address32);
            stream << ", address64=" << device.address64.toString();
            //            stream << ", ";
            //            device.capabilities.toString(stream);
            stream << ", status=";
            stream << DeviceStatus::toString(device.status);
            stream << "}";

            return stream;
        }

        void toTableIndentString(std::ostringstream& stream, Uint16 noJoins) {
            //stream << std::setw(20) << capabilities.euidAddress.toString();
            stream << " " << ToStr(address32);
            stream << " " << ToStr(parent32);
            stream << " " << address64.address64String;
            stream << std::setw(13) << DeviceStatus::toString(status);
            stream << std::setw(3) << noJoins;
            stream << std::setw(3) << (int) metaDataAttributes->getJoinPriority();

            stream << deviceHistory.lastJoinTimeStr;

            if (!(MANAGER_ADDRESS == address32) && !(GATEWAY_ADDRESS == address32) && status != DeviceStatus::DELETED) {
                stream << std::dec;

                stream << "  G:" << std::setw(3) << (int) metaDataAttributes->getTotalGraphs();
                stream << "," << std::setw(3) << (int) metaDataAttributes->getUsedGraphs();

                stream << " L:" << std::setw(3) << (int) metaDataAttributes->getTotalLinks();
                stream << "," << std::setw(3) << (int) metaDataAttributes->getUsedLinks();
              

#ifdef JOIN_REASON
                for (std::deque<RejoinReason>::iterator it= rejoinReasons.begin(); it != rejoinReasons.end(); it++)
                {
                    stream << "  " << std::hex << std::setw(2) << std::setfill('0') << (int) it->joinReason[0] << " "
                                << std::setw(2) << (int)  it->joinReason[1] << " " << std::setw(2) << (int)  it->joinReason[2] << " "
                                << std::setw(2) << (int)  it->joinReason[3] << std::setfill(' ');
                }
                for (int i = rejoinReasons.size(); i < MAX_REJOINREASON_SIZE; i++)
                {
                    //         __XX_XX_XX_XX
                    stream << "             ";
                }
#endif
                stream << "  " << DeviceAction::toString(action);
            } else {

            }
        }

        void toIndentString(std::ostringstream& stream) {
            stream << "Device {";
            stream << "addr32=" << ToStr(address32);
            stream << ", ";
            capabilities.toString(stream);
            stream << ", status=";
            stream << DeviceStatus::toString(status);
            stream << "}";
            stream << std::endl;

            stream << "Res: ";
            stream << " Sv:" << std::setw(2) << (int) metaDataAttributes->getTotalServices();
            stream << "," << std::setw(2) << (int) metaDataAttributes->getUsedServices();
            stream << " R:" << std::setw(2) << (int) metaDataAttributes->getTotalRoutes();
            stream << "," << std::setw(2) << (int) metaDataAttributes->getUsedRoutes();
            stream << " SR:" << std::setw(2) << (int) metaDataAttributes->getTotalSourceRoutes();
            stream << "," << std::setw(2) << (int) metaDataAttributes->getUsedSourceRoutes();
            stream << " G:" << std::setw(3) << (int) metaDataAttributes->getTotalGraphs();
            stream << "," << std::setw(3) << (int) metaDataAttributes->getUsedGraphs();
            stream << " GN:" << std::setw(3) << (int) metaDataAttributes->getTotalGraphNeighbors();
            stream << "," << std::setw(3) << (int) metaDataAttributes->getUsedGraphNeighbors();
            stream << " Sf:" << std::setw(2) << (int) metaDataAttributes->getTotalSuperframes();
            stream << "," << std::setw(2) << (int) metaDataAttributes->getUsedSuperframes();
            stream << " L:" << std::setw(3) << (int) metaDataAttributes->getTotalLinks();
            stream << "," << std::setw(2) << (int) metaDataAttributes->getUsedLinks();
        }

        std::string toShortIndentString() {
            std::ostringstream stream;
            stream << "Device {";
            stream << "addr32=" << ToStr(address32);
            stream << ", addr64=" << address64.toString();
            stream << ", status=";
            stream << DeviceStatus::toString(status);
            stream << "}";

            return stream.str();
        }
};

}
}

#endif /*DEVICE_H_*/
