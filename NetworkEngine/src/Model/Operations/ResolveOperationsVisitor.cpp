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

#include "ResolveOperationsVisitor.h"

using namespace NE::Model::Operations;
using namespace NE::Model::Services;
using namespace NE::Model;

ResolveOperationsVisitor::ResolveOperationsVisitor(NetworkEngine& networkEngine_) :
    networkEngine(networkEngine_) {
}

ResolveOperationsVisitor::~ResolveOperationsVisitor() {
}

bool ResolveOperationsVisitor::visitServiceAddedOperation(ServiceAddedOperation& operation) {
    networkEngine.forwardMarkAsChanged(operation, operation.peerAddress32);
    return networkEngine.getSubnetServices().resolveOperation(operation);
}

bool ResolveOperationsVisitor::visitServiceRemovedOperation(ServiceRemovedOperation& operation) {
    networkEngine.forwardMarkAsChanged(operation, operation.peerAddress);
    return networkEngine.getSubnetServices().resolveOperation(operation);
}

bool ResolveOperationsVisitor::visitRouteAddedOperation(RouteAddedOperation& operation) {
    networkEngine.forwardMarkAsChanged(operation, operation.peerAddress);
    return networkEngine.getSubnetServices().resolveOperation(operation);
}

bool ResolveOperationsVisitor::visitSourceRouteAddedOperation(SourceRouteAddedOperation& operation) {
    networkEngine.forwardMarkAsChanged(operation, 0);
    return networkEngine.getSubnetServices().resolveOperation(operation);
}

bool ResolveOperationsVisitor::visitRouteRemovedOperation(RouteRemovedOperation& operation) {
    networkEngine.forwardMarkAsChanged(operation, 0);
    return networkEngine.getSubnetServices().resolveOperation(operation);
}

bool ResolveOperationsVisitor::visitSourceRouteRemovedOperation(SourceRouteRemovedOperation& operation) {
    networkEngine.forwardMarkAsChanged(operation, 0);
    return networkEngine.getSubnetServices().resolveOperation(operation);
}

bool ResolveOperationsVisitor::visitLinkAddedOperation(LinkAddedOperation& operation) {
    networkEngine.forwardMarkAsChanged(operation, 0);
    return networkEngine.getSubnetTdma().resolveOperation(operation);
}

bool ResolveOperationsVisitor::visitLinkRemovedOperation(LinkRemovedOperation& operation) {
    networkEngine.forwardMarkAsChanged(operation, 0);
    return networkEngine.getSubnetTdma().resolveOperation(operation);
}

bool ResolveOperationsVisitor::visitReadLinksOperation(NE::Model::Operations::ReadLinksOperation& operation) {
    networkEngine.forwardMarkAsChanged(operation, 0);
    return true;
}

bool ResolveOperationsVisitor::visitSuperframeAddedOperation(SuperframeAddedOperation& operation) {
    return networkEngine.getSubnetTdma().resolveOperation(operation);
}

bool ResolveOperationsVisitor::visitNeighborGraphAddedOperation(NeighborGraphAddedOperation& operation) {
    networkEngine.forwardMarkAsChanged(operation, 0);
    return networkEngine.getSubnetTopology().resolveOperation(operation);
}

bool ResolveOperationsVisitor::visitNeighborGraphRemovedOperation(NeighborGraphRemovedOperation& operation) {
    networkEngine.forwardMarkAsChanged(operation, 0);
    return networkEngine.getSubnetTopology().resolveOperation(operation);
}

bool ResolveOperationsVisitor::visitSetChannelsBlacklistOperation(SetChannelsBlacklistOperation& operation) {
    networkEngine.forwardMarkAsChanged(operation, 0);
    //TODO: andy
    return true;
}

bool ResolveOperationsVisitor::visitWriteTimerIntervalOperation(
            NE::Model::Operations::WriteTimerIntervalOperation& operation) {
    networkEngine.forwardMarkAsChanged(operation, 0);
    if (operation.getErrorCode() == ErrorCode::SUCCESS) {
        return true;
    }
    return false;
}

bool ResolveOperationsVisitor::visitSetClockSourceOperation(NE::Model::Operations::SetClockSourceOperation& operation) {
    networkEngine.forwardMarkAsChanged(operation, 0);
    return true;
}

bool ResolveOperationsVisitor::visit(NE::Model::Operations::ChangePriorityEngineOperation& operation) {
    networkEngine.forwardMarkAsChanged(operation, 0);
    if (operation.getErrorCode() == ErrorCode::SUCCESS) {
        return true;
    }
    if (operation.getState() == OperationState::SENT_IGNORED) {
        return true;
    }
    return false;
}

bool ResolveOperationsVisitor::visit(NE::Model::Operations::ChangeNotificationOperation& operation) {
    networkEngine.forwardMarkAsChanged(operation, 0);
    if (operation.getErrorCode() == ErrorCode::SUCCESS) {
        return true;
    }
    if (operation.getState() == OperationState::SENT_IGNORED) {
        return true;
    }
    return false;
}

bool ResolveOperationsVisitor::visit(NE::Model::Operations::WriteKeyOperation& operation) {
    networkEngine.forwardMarkAsChanged(operation, 0);
    if (operation.getErrorCode() == ErrorCode::SUCCESS) {
        return true;
    }
    return false;
}

bool ResolveOperationsVisitor::visit(NE::Model::Operations::WriteNicknameOperation& operation) {
    networkEngine.forwardMarkAsChanged(operation, 0);
    if (operation.getErrorCode() == ErrorCode::SUCCESS) {
        return true;
    }
    return false;
}

bool ResolveOperationsVisitor::visit(NE::Model::Operations::WriteSessionOperation& operation) {
    networkEngine.forwardMarkAsChanged(operation, 0);
    if (operation.getErrorCode() == ErrorCode::SUCCESS) {
        return true;
    }
    return false;
}

bool ResolveOperationsVisitor::visit(NE::Model::Operations::NeighborHealthReportOperation& operation) {
    networkEngine.forwardMarkAsChanged(operation, 0);
    if (operation.getErrorCode() == ErrorCode::SUCCESS) {
        return true;
    }
    return false;
}

bool ResolveOperationsVisitor::visit(NE::Model::Operations::WriteNetworkIDOperation& operation) {
    if (operation.getErrorCode() == ErrorCode::SUCCESS) {
        return true;
    }
    return false;
}

bool ResolveOperationsVisitor::visit(NE::Model::Operations::ReadWirelessDeviceCapabilitiesOperation& operation){
    if (operation.getErrorCode() == ErrorCode::SUCCESS) {
        return true;
    }
    return false;
}


bool ResolveOperationsVisitor::visit(NE::Model::Operations::NivisCustom64765Operation& operation) {
    if (operation.getErrorCode() == ErrorCode::SUCCESS) {
        return true;
    }
    return false;
}

