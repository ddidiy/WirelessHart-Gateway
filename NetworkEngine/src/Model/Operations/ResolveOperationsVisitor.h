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

#ifndef RESOLVEOPERATIONSVISITOR_H_
#define RESOLVEOPERATIONSVISITOR_H_

#include "Model/Operations/EngineOperations.h"
#include "Model/Operations/IEngineOperationsVisitor.h"
#include "Model/NetworkEngine.h"

namespace NE {
namespace Model {
namespace Operations {

/**
 * The generated operations in the case of an event are sent to devices in the dependence order.
 * In the case of success for the operations that has a correspondent entity in model actualize the
 * entity (add/edit/delete).
 * In the case of fail (error/timeout) do nothing because the device will be deleted anyway
 * In the case of operations not sent to device and has a correspondent entity in model, actualize the
 * entity (add/edit/delete)
 */
class ResolveOperationsVisitor: public NE::Model::Operations::IEngineOperationsVisitor {

        NetworkEngine& networkEngine;

    public:

        ResolveOperationsVisitor(NetworkEngine& networkEngine);

        virtual ~ResolveOperationsVisitor();

        /**
         * In the case of success if the operation has status PENDING set the status ACTIVE
         * In the case of timeout/fail the device will be deleted so no actions
         * In the case the operation was not sent to device:
         * 1. if the device is on joining, do nothing because the device will be deleted
         * 2. otherwise set the status as processing
         */
        bool visitServiceAddedOperation(NE::Model::Operations::ServiceAddedOperation& operation);

        /**
         * In the case of success if the operation has status PENDING set the status ACTIVE
         * In the case of timeout/fail the device will be deleted so no actions
         * In the case the operation was not sent to device set the status as MARK_FOR_DELETE
         */
        bool visitServiceRemovedOperation(NE::Model::Operations::ServiceRemovedOperation& operation);

        /**
         * In the case of success if the operation has status PENDING set the status ACTIVE
         * In the case of timeout/fail the device will be deleted so no actions
         * In the case the operation was not sent to device:
         * 1. if the device is on joining, do nothing because the device will be deleted do nothing
         * 2. otherwise set the status as processing
         */
        bool visitRouteAddedOperation(NE::Model::Operations::RouteAddedOperation& operation);

        /**
         *
         */
        bool visitSourceRouteAddedOperation(NE::Model::Operations::SourceRouteAddedOperation& operation);

        /**
         * In the case of success if the operation has status PENDING set the status ACTIVE
         * In the case of timeout/fail the device will be deleted so no actions
         * In the case the operation was not sent to device set the status as MARK_FOR_DELETE
         */
        bool visitRouteRemovedOperation(NE::Model::Operations::RouteRemovedOperation& operation);

        bool visitSourceRouteRemovedOperation(NE::Model::Operations::SourceRouteRemovedOperation& operation);

        /**
         * In the case of success if the operation has status PENDING set the status ACTIVE
         * In the case of timeout/fail the device will be deleted so no actions
         * In the case the operation was not sent to device:
         * 1. if the device is on joining, do nothing because the device will be deleted
         * 2. otherwise delete the link from model
         */
        bool visitLinkAddedOperation(NE::Model::Operations::LinkAddedOperation& operation);

        /**
         * In the case of success if the operation has status PENDING set the status ACTIVE
         * In the case of timeout/fail the device will be deleted so no actions
         * In the case the operation was not sent to device set the status as ACTIVE
         */
        bool visitLinkRemovedOperation(NE::Model::Operations::LinkRemovedOperation& operation);

        /**
         * In the case of success if the operation has status PENDING set the status ACTIVE
         * In the case of timeout/fail the device will be deleted so no actions
         * In the case the operation was not sent to device set the status as ACTIVE
         */
        bool visitReadLinksOperation(NE::Model::Operations::ReadLinksOperation& operation);

        /**
         * It is generated at the join time, and is not keep in model, so nothing to do
         */
        bool visitSuperframeAddedOperation(NE::Model::Operations::SuperframeAddedOperation& operation);

        /**
         * In the case of success if the operation has status PENDING set the status ACTIVE
         * In the case of timeout/fail the device will be deleted so no actions
         * In the case the operation was not sent to device:
         * 1. if the device is on joining, do nothing because the device will be deleted
         * 2. otherwise delete the graph neighbor from model
         */
        bool visitNeighborGraphAddedOperation(NE::Model::Operations::NeighborGraphAddedOperation& operation);

        /**
         * In the case of success if the operation has status PENDING set the status ACTIVE
         * In the case of timeout/fail the device will be deleted so no actions
         * In the case the operation was not sent to device set the status as ACTIVE
         */
        bool visitNeighborGraphRemovedOperation(NE::Model::Operations::NeighborGraphRemovedOperation& operation);

        /**
         * It is set at join time only on AP and is not keep in model so nothing to do
         */
        bool visitSetChannelsBlacklistOperation(NE::Model::Operations::SetChannelsBlacklistOperation& operation);

        /**
         *
         */
        bool visitWriteTimerIntervalOperation(NE::Model::Operations::WriteTimerIntervalOperation& operation);

        /**
         *
         */
        bool visitSetClockSourceOperation(NE::Model::Operations::SetClockSourceOperation& operation);

        bool visit(NE::Model::Operations::ChangePriorityEngineOperation& operation);

        bool visit(NE::Model::Operations::ChangeNotificationOperation& operation);

        bool visit(NE::Model::Operations::WriteKeyOperation& operation);

        bool visit(NE::Model::Operations::WriteNicknameOperation& operation);

        bool visit(NE::Model::Operations::WriteSessionOperation& operation);

        bool visit(NE::Model::Operations::NeighborHealthReportOperation& operation);

        bool visit(NE::Model::Operations::WriteNetworkIDOperation& operation);

        bool visit(NE::Model::Operations::ReadWirelessDeviceCapabilitiesOperation& operation);

        bool visit(NE::Model::Operations::NivisCustom64765Operation& operation);

};

}
}
}

#endif /* RESOLVEOPERATIONSVISITOR_H_ */
