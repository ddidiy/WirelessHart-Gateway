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
 * WHEngineOperationsVisitor.h
 *
 *  Created on: Jan 5, 2009
 *      Author: ioanpocol
 */

#ifndef WHENGINEOPERATIONSVISITOR_H_
#define WHENGINEOPERATIONSVISITOR_H_

#include "WHOperation.h"

#include "../CommonData.h"
#include <Model/Operations/IEngineOperationsVisitor.h>
#include <nlib/log.h>

namespace hart7 {

namespace nmanager {

namespace operations {

/**
 * Creates WHOperations from EngineOperations.
 */
class WHEngineOperationsVisitor: public NE::Model::Operations::IEngineOperationsVisitor
{
        LOG_DEF("h7.n.o.WHEngineOperationsVisitor")
        ;

    private:

        WHartTime40 asn;

        std::vector<WHOperationPointer> whOperations;

        EngineOperationsListPointer operationEventPointer;

        hart7::nmanager::CommonData& commonData;

    public:

        WHEngineOperationsVisitor(EngineOperationsListPointer operationEventPointer,
                                  hart7::nmanager::CommonData& commonData);

        virtual ~WHEngineOperationsVisitor();

        void setASN(Uint32 taiCutover);

        std::vector<WHOperationPointer> getWHOperations();

        void resetWHOperations();

        WHOperationPointer initWHOperation(IEngineOperation& operation);

        virtual bool visitServiceAddedOperation(NE::Model::Operations::ServiceAddedOperation& operation);

        virtual bool visitServiceRemovedOperation(NE::Model::Operations::ServiceRemovedOperation& operation);

        virtual bool visitRouteAddedOperation(NE::Model::Operations::RouteAddedOperation& operation);

        virtual bool visitRouteRemovedOperation(NE::Model::Operations::RouteRemovedOperation& operation);

        virtual bool visitSourceRouteAddedOperation(NE::Model::Operations::SourceRouteAddedOperation& operation);

        virtual bool visitSourceRouteRemovedOperation(NE::Model::Operations::SourceRouteRemovedOperation& operation);

        virtual bool visitLinkAddedOperation(NE::Model::Operations::LinkAddedOperation& operation);

        virtual bool visitLinkRemovedOperation(NE::Model::Operations::LinkRemovedOperation& operation);

        virtual bool visitReadLinksOperation(NE::Model::Operations::ReadLinksOperation& operation);

        virtual bool visitSuperframeAddedOperation(NE::Model::Operations::SuperframeAddedOperation& operation);

        virtual bool visitNeighborGraphAddedOperation(NE::Model::Operations::NeighborGraphAddedOperation& operation);

        virtual bool
                    visitNeighborGraphRemovedOperation(NE::Model::Operations::NeighborGraphRemovedOperation& operation);

        virtual bool
                    visitSetChannelsBlacklistOperation(NE::Model::Operations::SetChannelsBlacklistOperation& operation);

        virtual bool visitWriteTimerIntervalOperation(NE::Model::Operations::WriteTimerIntervalOperation& operation);

        virtual bool visitSetClockSourceOperation(NE::Model::Operations::SetClockSourceOperation& operation);

        bool visit(NE::Model::Operations::ChangePriorityEngineOperation& operation);

        virtual bool visit(NE::Model::Operations::ChangeNotificationOperation& operation);

        bool visit(NE::Model::Operations::WriteKeyOperation& operation);

        bool visit(NE::Model::Operations::WriteNicknameOperation& operation);

        bool visit(NE::Model::Operations::WriteSessionOperation& operation);

        bool visit(NE::Model::Operations::NeighborHealthReportOperation& operation);

        bool visit(NE::Model::Operations::WriteNetworkIDOperation& operation);

        bool visit(NE::Model::Operations::ReadWirelessDeviceCapabilitiesOperation& operation);

        bool visit(NE::Model::Operations::NivisCustom64765Operation & operation);
};

}
}
}
#endif /* WHENGINEOPERATIONSVISITOR_H_ */
