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
 * IEngineOperationsVisitor.h
 *
 *  Created on: Jul 28, 2008
 *      Author: ivp
 */

#ifndef IENGINEOPERATIONSVISITOR_H_
#define IENGINEOPERATIONSVISITOR_H_

#include "Model/Operations/Services/ServiceAddedOperation.h"
#include "Model/Operations/Services/ServiceRemovedOperation.h"
#include "Model/Operations/Services/RouteAddedOperation.h"
#include "Model/Operations/Services/SourceRouteAddedOperation.h"
#include "Model/Operations/Services/RouteRemovedOperation.h"
#include "Model/Operations/Services/SourceRouteRemovedOperation.h"

#include "Model/Operations/Topology/NeighborGraphAddedOperation.h"
#include "Model/Operations/Topology/NeighborGraphRemovedOperation.h"
#include "Model/Operations/Topology/SetClockSourceOperation.h"

#include "Model/Operations/Tdma/SuperframeAddedOperation.h"
#include "Model/Operations/Tdma/LinkAddedOperation.h"
#include "Model/Operations/Tdma/LinkRemovedOperation.h"
#include "Model/Operations/Tdma/ReadLinksOperation.h"
#include "Model/Operations/Tdma/SetChannelsBlacklistOperation.h"
#include "Model/Operations/Tdma/WriteTimerIntervalOperation.h"

#include <Model/Operations/Join/ChangePriorityEngineOperation.h>
#include <Model/Operations/Join/ChangeNotificationOperation.h>
#include <Model/Operations/Join/NeighborHealthReportOperation.h>
#include <Model/Operations/Join/WriteKeyOperation.h>
#include <Model/Operations/Join/WriteNicknameOperation.h>
#include <Model/Operations/Join/WriteSessionOperation.h>
#include <Model/Operations/Join/WriteNetworkIDOperation.h>
#include <Model/Operations/Join/ReadWirelessDeviceCapabilitiesOperation.h>

#include  <Model/Operations/NivisCustom64765Operation.h>

namespace NE {
namespace Model {
namespace Operations {

class IEngineOperationsVisitor {

    public:

        virtual ~IEngineOperationsVisitor() {
        }

        virtual bool visitServiceAddedOperation(NE::Model::Operations::ServiceAddedOperation& operation) = 0;

        virtual bool visitServiceRemovedOperation(NE::Model::Operations::ServiceRemovedOperation& operation) = 0;

        virtual bool visitRouteAddedOperation(NE::Model::Operations::RouteAddedOperation& operation) = 0;

        virtual bool visitSourceRouteAddedOperation(NE::Model::Operations::SourceRouteAddedOperation& operation) = 0;

        virtual bool visitRouteRemovedOperation(NE::Model::Operations::RouteRemovedOperation& operation) = 0;

        virtual bool visitSourceRouteRemovedOperation(NE::Model::Operations::SourceRouteRemovedOperation& operation) = 0;

        virtual bool visitLinkAddedOperation(NE::Model::Operations::LinkAddedOperation& operation) = 0;

        virtual bool visitLinkRemovedOperation(NE::Model::Operations::LinkRemovedOperation& operation) = 0;

        virtual bool visitReadLinksOperation(NE::Model::Operations::ReadLinksOperation& operation) = 0;

        virtual bool visitSuperframeAddedOperation(NE::Model::Operations::SuperframeAddedOperation& operation) = 0;

        virtual bool visitNeighborGraphAddedOperation(NE::Model::Operations::NeighborGraphAddedOperation& operation) = 0;

        virtual bool visitNeighborGraphRemovedOperation(NE::Model::Operations::NeighborGraphRemovedOperation& operation) = 0;

        virtual bool visitSetChannelsBlacklistOperation(NE::Model::Operations::SetChannelsBlacklistOperation& operation) = 0;

        virtual bool visitWriteTimerIntervalOperation(NE::Model::Operations::WriteTimerIntervalOperation& operation) = 0;

        virtual bool visitSetClockSourceOperation(NE::Model::Operations::SetClockSourceOperation& operation) = 0;

        virtual bool visit(NE::Model::Operations::ChangePriorityEngineOperation& operation) = 0;

        virtual bool visit(NE::Model::Operations::ChangeNotificationOperation& operation) = 0;

        virtual bool visit(NE::Model::Operations::WriteKeyOperation& operation) = 0;

        virtual bool visit(NE::Model::Operations::WriteNicknameOperation& operation) = 0;

        virtual bool visit(NE::Model::Operations::WriteSessionOperation& operation) = 0;

        virtual bool visit(NE::Model::Operations::NeighborHealthReportOperation& operation) = 0;

        virtual bool visit(NE::Model::Operations::WriteNetworkIDOperation& operation) = 0;

        virtual bool visit(NE::Model::Operations::ReadWirelessDeviceCapabilitiesOperation& operation) = 0;

        virtual bool visit(NE::Model::Operations::NivisCustom64765Operation& operation) = 0;
};

}
}
}

#endif /* IENGINEOPERATIONSVISITOR_H_ */
