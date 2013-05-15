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

/*
 * ISuperframeOperation.h
 *
 *  Created on: May 15, 2008
 *      Author: catalin.pop
 */

#ifndef IENGINEOPERATION_H_
#define IENGINEOPERATION_H_

#include "Common/NETypes.h"
#include "Common/NEAddress.h"
#include "boost/shared_ptr.hpp"
#include <string>

#define OPERATION_NAME_WIDTH 20

namespace NE {
namespace Model {
namespace Operations {

namespace EngineOperationType {
enum EngineOperationTypeEnum {
    NONE, //this status may be used by Isa100 operations tha do not have attached an EngineOperation
    ADD_LINK,
    ADD_SUPERFRAME,
    ADD_ROUTE,
    ADD_SOURCEROUTE,
    ADD_SERVICE,
    REMOVE_LINK,
    REMOVE_ROUTE,
    REMOVE_SOURCEROUTE,
    REMOVE_CHANNEL,
    REMOVE_SERVICE,
    ADD_NEIGHBOR_GRAPH,
    REMOVE_NEIGHBOR_GRAPH,
    SET_BLACKLIST_CHANNELS,
    SET_HEALTH_REPORT_PERIOD,
    SET_KEEP_ALIVE_PERIOD,
    SET_CLOCK_SOURCE,
    WRITE_KEY,
    WRITE_NICK,
    WRITE_SESSION,
    CHANGE_JOIN_PRIORITY,
    CHANGE_NOTIFICATION,
    READ_LINKLIST,
    NEIGHBOR_HEALTH_REPORT,
    WRITE_NETWORKID,
    READ_WIRELESS_DEVICE_CAPABILITIES,
    NIVIS_CUSTOM_64765
};
} // namespace EngineOperationType

namespace OperationState {
enum OperationStateEnum {
    GENERATED = 0, SENT = 1, SEND_ATTEMPT = 2, SENT_IGNORED = 3, CONFIRMED = 4 //, IGNORED = 5
};

inline std::string stateToString(NE::Model::Operations::OperationState::OperationStateEnum state) {
    switch (state) {
        case NE::Model::Operations::OperationState::GENERATED:
            return "G";
        case NE::Model::Operations::OperationState::SENT:
            return "S";
        case NE::Model::Operations::OperationState::SEND_ATTEMPT:
            return "SA";
        case NE::Model::Operations::OperationState::SENT_IGNORED:
            return "SI";
        case NE::Model::Operations::OperationState::CONFIRMED:
            return "C";
        default:
            return "?";
    }
}
}

namespace WaveDependency {
enum WaveDependencyEnum { //MIN and MAX are used just for iterating. Do not use.
    MIN_DEPENDENCY = 0, //DO NOT USE
    FIRST = 1,
    SECOND = 2,
    THIRD = 3,
    FOURTH = 4,
    FIFTH = 5,
    SIXTH = 6,
    SEVENTH = 7,
    EIGTH = 8,
    NINETH = 9,
    TENTH = 10,
    ELEVENTH = 11,
    TWELFTH = 12,
    MAX_DEPENDENCY
// DO NOT USE
};
}

namespace ErrorCode {
enum ErrorCodeEnum {
    SUCCESS = 0, TIMEOUT = 1, ERROR = 2
};
}

class IEngineOperationsVisitor;

class IEngineOperation;
typedef boost::shared_ptr<IEngineOperation> IEngineOperationPointer;

/**
 * @version 1
 * @author Catalin Pop
 */

class IEngineOperation {

    public:
        virtual ~IEngineOperation() {
        }

        virtual Uint16 getOperationId() = 0;

        virtual std::string getName() = 0;

        virtual void toString(std::ostringstream& stream) = 0;

        virtual Uint16 getSubnetId() = 0;

        virtual Uint16 getOwnerAddress16() const = 0;

        virtual Address32 getOwner() const = 0;

        virtual EngineOperationType::EngineOperationTypeEnum getOperationType() = 0;

        virtual void setState(OperationState::OperationStateEnum operationState) = 0;

        virtual OperationState::OperationStateEnum getState() = 0;

        /**
         * Get the operation dependency(FIRST/SECOND/...).
         */
        virtual WaveDependency::WaveDependencyEnum getDependency() = 0;

        /**
         * Set the operation dependency(FIRST/SECOND/...).
         */
        virtual void setDependency(WaveDependency::WaveDependencyEnum operationDependency) = 0;

        /**
         * Returns a list with the operations dependency.
         */
        virtual std::list<IEngineOperationPointer>& getOperationsDependencies() = 0;

        /**
         * For the current operation add a new dependency to the passed operation.
         */
        virtual void setOperationDependency(IEngineOperationPointer operationDependency, bool allowDependencyOnHigherWave = false) = 0;

        virtual void setConfirmTimestamp(time_t confirmTimestamp) = 0;

        virtual time_t getConfirmTimestamp() = 0;

        /**
         */
        virtual void setErrorCode(ErrorCode::ErrorCodeEnum errorCode) = 0;

        virtual ErrorCode::ErrorCodeEnum getErrorCode() = 0;

        virtual void setResponseCode(Uint8 responseCode) = 0;

        virtual Uint8 getResponseCode() = 0;

        virtual bool accept(IEngineOperationsVisitor& visitor) = 0;
};

}
}
}

#endif /* IENGINEOPERATION_H_ */
