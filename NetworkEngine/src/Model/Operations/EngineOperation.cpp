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

/**
 * EngineOperation.cpp
 *
 *  Created on: Jul 15, 2008
 *      Author: ioan.pocol
 */

#include "EngineOperation.h"
#include <iomanip>
#include <stdint.h>

using namespace NE::Model::Operations;

EngineOperation::EngineOperation() {
    static Uint16 currentOperationId = 0;

    operationId = ++currentOperationId;
    requestID = 0;
    state = OperationState::GENERATED;
    errorCode = ErrorCode::SUCCESS;
}

EngineOperation::~EngineOperation() {
}

Uint16 EngineOperation::getOperationId() {
    return operationId;
}

Uint16 EngineOperation::getRequestID() {
    return requestID;
}

Uint16 EngineOperation::getSubnetId() {
    return (Uint16) (owner >> 16);
}

void EngineOperation::setRequestID(Uint16 requestID_) {
    requestID = requestID_;
}

Address32 EngineOperation::getOwner() const {
    return owner;
}

Uint16 EngineOperation::getOwnerAddress16() const {
    return (Uint16) (owner & 0xFFFF);
}

OperationState::OperationStateEnum EngineOperation::getState() {
    return state;
}

void EngineOperation::setState(OperationState::OperationStateEnum state_) {
    state = state_;
}

WaveDependency::WaveDependencyEnum EngineOperation::getDependency() {
    return operationDependency;
}

void EngineOperation::setDependency(WaveDependency::WaveDependencyEnum operationDependency_) {
    operationDependency = operationDependency_;
}

std::list<IEngineOperationPointer>& EngineOperation::getOperationsDependencies() {
    return operationsDependencies;
}

void EngineOperation::setOperationDependency(IEngineOperationPointer operationDependency,
            bool allowDependencyOnHigherWave) {
    if (!operationDependency) {
        LOG_ERROR("You can not set a dependency on an invalid operation!");
        return;
    }

    if (operationDependency->getOperationId() == this->getOperationId()) {
        LOG_ERROR("You can not set the dependency to be the same with the current operation! operationId= "
                    << (int) this->getOperationId());
        return;
    }

    std::list<IEngineOperationPointer>::const_iterator itOpDep = this->getOperationsDependencies().begin();
    for (; itOpDep != this->getOperationsDependencies().end(); ++itOpDep) {
        if ((*itOpDep)->getOperationId() == operationDependency->getOperationId()) {
            LOG_WARN("Operation " << (int) operationDependency->getOperationId()
                        << " already added to the list of dependencies!");
            return;
        }
    }

    // check if the dependent operation is somehow dependent on the current operation
    const std::list<IEngineOperationPointer>& depOpDependencies = operationDependency->getOperationsDependencies();
    std::list<IEngineOperationPointer>::const_iterator itDep = depOpDependencies.begin();
    for (; itDep != depOpDependencies.end(); ++itDep) {
        if ((*itDep)->getOperationId() == this->getOperationId()) {
            LOG_ERROR("Circular dependency between operations!");
            return;
        }
    }

    this->operationsDependencies.push_back(operationDependency);
}

std::string EngineOperation::getStateDecription() {
    if (this->state == OperationState::GENERATED) {
        return "GENERATED";
    } else if (this->state == OperationState::CONFIRMED) {
        return "CONFIRMED";
    } else if (this->state == OperationState::SENT) {
        return "SENT";
    }

    return "Invalid value!";
}

time_t EngineOperation::getSentTimestamp() {
    return sentTimestamp;
}

void EngineOperation::setSentTimestamp(time_t sentTimestamp_) {
    sentTimestamp = sentTimestamp_;
}

void EngineOperation::setConfirmTimestamp(time_t confirmTimestamp_) {
    confirmTimestamp = confirmTimestamp_;
}

time_t EngineOperation::getConfirmTimestamp() {
    return confirmTimestamp;
}

void EngineOperation::setErrorCode(ErrorCode::ErrorCodeEnum errorCode_) {

    errorCode = errorCode_;
}

ErrorCode::ErrorCodeEnum EngineOperation::getErrorCode() {
    return errorCode;
}

std::string EngineOperation::errorCodeToString(ErrorCode::ErrorCodeEnum& ec) {
    switch (ec) {
        case ErrorCode::ERROR:
            return "E";
        case ErrorCode::SUCCESS:
            return "S";
        case ErrorCode::TIMEOUT:
            return "T";
        default:
            return "U";
    }
}

void EngineOperation::toString(std::ostringstream& stream) {
    toStringCommonOperationState(stream);
    toStringInternal(stream);
    toStringDependencies(stream);
}

void EngineOperation::toStringCommonOperationState(std::ostringstream& stream) {
    stream << std::setw(OPERATION_NAME_WIDTH + 4) << getName();
    stream << "(" << std::setw(2) << getOperationType() << ")";
    stream << " {id=" << std::setw(8) << operationId;
    stream << ", own=" << ToStr(getOwner());
    stream << ", state=" << std::setw(2) << NE::Model::Operations::OperationState::stateToString(getState());
    stream << ", ec=" << std::setw(2) << errorCodeToString(errorCode);
    stream << ", dep=" << std::setw(2) << getDependency();
}

void EngineOperation::toStringDependencies(std::ostringstream& stream) {
    stream << " dep(";
    int i = operationsDependencies.size();
    for (std::list<IEngineOperationPointer>::iterator it = operationsDependencies.begin(); it
                != operationsDependencies.end(); ++it) {
        stream << std::dec << (*it)->getOperationId() << std::setfill(' ');
        if (--i > 0) {
            stream << ", ";
        }
    }
    stream << ")";
}

