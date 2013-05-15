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
 * EngineOperationsList.cpp
 *
 *  Created on: Jun 17, 2008
 *      Author: catalin.pop
 */
#include "EngineOperations.h"
#include "Model/DevicesTable.h"

using namespace NE::Model::Operations;

void toString(std::ostringstream &stream, const EngineOperationsVector& engineOperationsVector) {

    int idx = 0;
    for (EngineOperationsVector::const_iterator it = engineOperationsVector.begin(); it != engineOperationsVector.end(); it++) {

        stream << "Operation " << idx++ << ": ";
        (*it)->toString(stream);
        stream << std::endl;
    }
}

int EngineOperations::nextSetIndex = 1;

EngineOperations::EngineOperations() :
    currentOperationsSetIndex(nextSetIndex++), shortAddress(false), proxyAddress(false), errorCode(ErrorCode::SUCCESS),
                reasonOfOperations("Unknown reason") {

    if (nextSetIndex == 0) {
        nextSetIndex++;//do not used setIndex = 0
    }

    eventEngineType = NetworkEngineEventType::NONE;
}

EngineOperations::EngineOperations(const EngineOperations& other) : currentOperationsSetIndex(other.currentOperationsSetIndex),
            requesterAddress64(other.requesterAddress64), requesterAddress32(other.requesterAddress32),
            proxyAddress32(other.proxyAddress32),
            shortAddress(other.shortAddress), proxyAddress(other.proxyAddress), eventEngineType(other.eventEngineType),
            graphId(other.graphId), errorCode(ErrorCode::SUCCESS),
            reasonOfOperations(other.reasonOfOperations), operations(other.operations)

{
}


EngineOperations::~EngineOperations() {
}

Address64& EngineOperations::getRequesterAddress64() {
    return requesterAddress64;
}

void EngineOperations::setRequesterAddress64(const Address64& requesterAddress64) {
    this->requesterAddress64 = requesterAddress64;
}

Address32 EngineOperations::getRequesterAddress32() {
    return requesterAddress32;
}

void EngineOperations::setRequesterAddress32(Address32 requesterAddress32) {
    this->requesterAddress32 = requesterAddress32;
}

Address32 EngineOperations::getProxyAddress32() {
    return proxyAddress32;
}

void EngineOperations::setProxyAddress32(Address32 proxyAddress32, bool shortAddress) {
    this->proxyAddress32 = proxyAddress32;
    proxyAddress = true;
    this->shortAddress = shortAddress;
}

bool EngineOperations::isShortAddress() {
    return shortAddress;
}

Uint16 EngineOperations::getGraphId() {
    return graphId;
}

void EngineOperations::setGraphId(Uint16 graphId_) {
    graphId = graphId_;
}

bool EngineOperations::isProxyAddress() {
    return proxyAddress;
}

ErrorCode::ErrorCodeEnum EngineOperations::getErrorCode() {
    return errorCode;
}

void EngineOperations::setErrorCode(ErrorCode::ErrorCodeEnum errorCode_) {
    errorCode = errorCode_;
}

void EngineOperations::addOperation(const NE::Model::Operations::IEngineOperationPointer& operation) {

    addOperation(operation, operation->getDependency());

}

void EngineOperations::addOperation(const NE::Model::Operations::IEngineOperationPointer& operation,
            WaveDependency::WaveDependencyEnum operationDependency) {
    operation->setDependency(operationDependency);

    operations.push_back(operation);
}

EngineOperationsVector& EngineOperations::getEngineOperations() {

    return operations;
}

EngineOperationsVector EngineOperations::findOperations(Address32 address32) {
    EngineOperationsVector foundOperations;

    for (EngineOperationsVector::iterator it = operations.begin(); it != operations.end(); it++) {
        if ((*it)->getOwner() == address32) {
            foundOperations.push_back(*it);
        }
    }

    return foundOperations;
}

Uint16 EngineOperations::getSize() {
    return operations.size();
}

int EngineOperations::getOperationsSetIndex() {
    return currentOperationsSetIndex;
}

void EngineOperations::clear() {
    operations.clear();

    errorCode = ErrorCode::SUCCESS;
}

void EngineOperations::setJoinDependencies(Address32 address) {

    for (EngineOperationsVector::iterator it = operations.begin(); it != operations.end(); it++) {
        if ((*it)->getOperationType() == EngineOperationType::CHANGE_NOTIFICATION) {
            // leave the wave dependencies of the change notification operations unchanged;
            // the wave information is used to determine the status for 769 join status.
            continue;
        } else if ((*it)->getOwner() == address) {
            (*it)->setDependency(WaveDependency::SECOND);
        } else {
            (*it)->setDependency(WaveDependency::FIRST);
        }

    }
}

void EngineOperations::setAdvertiseDependencies() {

    for (EngineOperationsVector::iterator it = operations.begin(); it != operations.end(); it++) {
        (*it)->setDependency(WaveDependency::FIRST);
    }
}

void EngineOperations::setAttachInDependencies(Address32 address, Address32 parent) {

    for (EngineOperationsVector::iterator it = operations.begin(); it != operations.end(); it++) {
        if ((*it)->getOperationType() == EngineOperationType::ADD_NEIGHBOR_GRAPH) {
            if ((*it)->getOwner() == address) {
                (*it)->setDependency(WaveDependency::THIRD);
            } else {
                (*it)->setDependency(WaveDependency::SECOND);
            }
        } else if ((*it)->getOperationType() == EngineOperationType::SET_CLOCK_SOURCE) {
            (*it)->setDependency(WaveDependency::THIRD);
        } else if ((*it)->getOperationType() == EngineOperationType::ADD_ROUTE) {
            (*it)->setDependency(WaveDependency::THIRD);
        } else if ((*it)->getOperationType() == EngineOperationType::CHANGE_JOIN_PRIORITY) {
            (*it)->setDependency(WaveDependency::SECOND);
        } else if ((*it)->getOperationType() == EngineOperationType::ADD_SUPERFRAME) {
            if ((*it)->getOwner() == address) {
                (*it)->setDependency(WaveDependency::THIRD);
            } else {
                (*it)->setDependency(WaveDependency::FIRST);
            }
        } else if ((*it)->getOperationType() == EngineOperationType::ADD_LINK) {
            if ((*it)->getDependency() == WaveDependency::SECOND) {
                (*it)->setDependency(WaveDependency::FIRST);
            } else {
                if ((*it)->getOwner() == address) {
                    (*it)->setDependency(WaveDependency::THIRD);
                } else {
                    (*it)->setDependency(WaveDependency::SECOND);
                }
            }
        } else if ((*it)->getOperationType() == EngineOperationType::REMOVE_LINK) {
            if ((*it)->getDependency() == WaveDependency::TENTH) {
                if ((*it)->getOwner() == address) {
                    (*it)->setDependency(WaveDependency::THIRD);
                } else {
                    (*it)->setDependency(WaveDependency::SECOND);
                }
            } else {
                (*it)->setDependency(WaveDependency::THIRD);
            }
        }
    }

    EngineOperationsVector tmp;

    for (EngineOperationsVector::iterator it = operations.begin(); it != operations.end();) {
        if ((*it)->getOperationType() == EngineOperationType::ADD_NEIGHBOR_GRAPH) {
            tmp.push_back(*it);
            it = operations.erase(it);
        } else {
            it++;
        }
    }

    for (EngineOperationsVector::iterator it = operations.begin(); it != operations.end();) {
        if ((*it)->getOperationType() == EngineOperationType::ADD_ROUTE || (*it)->getOperationType()
                    == EngineOperationType::ADD_SOURCEROUTE || (*it)->getOperationType()
                    == EngineOperationType::ADD_SERVICE) {
            tmp.push_back(*it);
            it = operations.erase(it);
        } else {
            it++;
        }
    }

    for (EngineOperationsVector::iterator it = tmp.begin(); it != tmp.end(); it++) {
        operations.push_back(*it);
    }

}

void EngineOperations::setAttachOutDependencies(Address32 address, Address32 parent) {

    for (EngineOperationsVector::iterator it = operations.begin(); it != operations.end(); it++) {
        if ((*it)->getOperationType() == EngineOperationType::ADD_NEIGHBOR_GRAPH) {
            (*it)->setDependency(WaveDependency::SECOND);
        } else if ((*it)->getOperationType() == EngineOperationType::ADD_ROUTE) {
            (*it)->setDependency(WaveDependency::SECOND);
        } else if ((*it)->getOperationType() == EngineOperationType::ADD_SUPERFRAME) {
            if ((*it)->getOwner() == address) {
                (*it)->setDependency(WaveDependency::FIFTH);
            } else {
                (*it)->setDependency(WaveDependency::SECOND);
            }
        } else if ((*it)->getOperationType() == EngineOperationType::ADD_LINK) {
            if ((*it)->getDependency() == WaveDependency::THIRD) {
                if ((*it)->getOwner() == address) {
                    (*it)->setDependency(WaveDependency::FIFTH);
                } else if ((*it)->getOwner() == parent) {
                    (*it)->setDependency(WaveDependency::SIXTH);    // send after RX is sent to owner
                }
                else {
                    (*it)->setDependency(WaveDependency::SECOND);
                }
            } else {
                if ((*it)->getOwner() == address) {
                    (*it)->setDependency(WaveDependency::FIFTH);
                } else if ((*it)->getOwner() == parent) {
                    (*it)->setDependency(WaveDependency::FOURTH);
                } else {
                    (*it)->setDependency(WaveDependency::THIRD);
                }
            }
        } else if ((*it)->getOperationType() == EngineOperationType::REMOVE_LINK) {
            if ((*it)->getDependency() == WaveDependency::NINETH) {
                (*it)->setDependency(WaveDependency::THIRD);
            } else if ((*it)->getDependency() == WaveDependency::ELEVENTH) {
                (*it)->setDependency(WaveDependency::SIXTH);
            } else {
                (*it)->setDependency(WaveDependency::SEVENTH);
            }
        }
    }

    EngineOperationsVector tmp;

    for (EngineOperationsVector::iterator it = operations.begin(); it != operations.end();) {
        if ((*it)->getOperationType() == EngineOperationType::ADD_NEIGHBOR_GRAPH) {
            tmp.push_back(*it);
            it = operations.erase(it);
        } else {
            it++;
        }
    }

    for (EngineOperationsVector::iterator it = operations.begin(); it != operations.end();) {
        if ((*it)->getOperationType() == EngineOperationType::ADD_ROUTE || (*it)->getOperationType()
                    == EngineOperationType::ADD_SOURCEROUTE || (*it)->getOperationType()
                    == EngineOperationType::ADD_SERVICE) {
            tmp.push_back(*it);
            it = operations.erase(it);
        } else {
            it++;
        }
    }

    for (EngineOperationsVector::iterator it = tmp.begin(); it != tmp.end(); it++) {
        operations.push_back(*it);
    }
}

void EngineOperations::toShortString(std::ostringstream &stream) {

    int i = 0;
    bool isFirst = true;
    stream << " {";
    for (EngineOperationsVector::iterator it = operations.begin(); it != operations.end(); it++) {
        if (isFirst) {
            isFirst = false;
        } else {
            stream << ", ";
        }
        ++i;

        stream << "0x" << ToStr((*it)->getOwner());
    }
    stream << " }";

    i = 0;
    isFirst = true;
    stream << std::endl;
    stream << " {";
    for (EngineOperationsVector::iterator it = operations.begin(); it != operations.end(); it++) {
        if (isFirst) {
            isFirst = false;
        } else {
            stream << ", ";
        }
        ++i;
        stream << (*it)->getOperationType();
    }
    stream << " }";

}

void EngineOperations::toIndentString(std::ostringstream &stream) {
    stream << std::endl;
    int i = 0;
    for (EngineOperationsVector::iterator it = operations.begin(); it != operations.end(); it++) {
        ++i;
        stream << std::setw(2) << std::dec << i;
        stream << ") ";
        (*it)->toString(stream);
        stream << std::endl;
    }
}

void EngineOperations::toString(std::ostringstream &stream) {
    toIndentString(stream);
}

