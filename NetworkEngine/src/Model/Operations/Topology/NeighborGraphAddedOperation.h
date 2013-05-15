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
 * NeighborGraphAddedOperation.h
 *
 *  Created on: 30.01.2009
 *      Author: radu.pop
 */

#ifndef NEIGHBORGRAPHADDEDOPERATION_H_
#define NEIGHBORGRAPHADDEDOPERATION_H_

#include "Model/Operations/EngineOperation.h"
#include <ApplicationLayer/Model/WirelessNetworkManagerCommands.h>

namespace NE {

namespace Model {

namespace Operations {

class NeighborGraphAddedOperation;
typedef boost::shared_ptr<NeighborGraphAddedOperation> NeighborGraphAddedOperationPointer;

/**
 * A NeighborGraphAddedOperation is generated when a graph is added to a node.
 * This operation is WirelessHart.
 * @author Radu Pop
 * @version 1.0
 */
class NeighborGraphAddedOperation: public NE::Model::Operations::EngineOperation {

    private:

        Uint16 graphId;

        Address32 neighbor;

        C969_WriteGraphNeighbourPair_RespCodes responseCode;

    public:

        NeighborGraphAddedOperation(Address32 owner_, Uint16 graphId_, Address32 neighbor);

        virtual ~NeighborGraphAddedOperation();

        void toStringInternal(std::ostringstream &stream);

        EngineOperationType::EngineOperationTypeEnum getOperationType();

        bool accept(IEngineOperationsVisitor & visitor);

        virtual void setResponseCode(Uint8 responseCode);

        virtual Uint8 getResponseCode();

    public:

        std::string getName() {
            return "NeighborGraphAdded";
        }

        Uint16 getGraphId() const {
            return graphId;
        }

        void setGraphID(Uint16 graphId_) {
            graphId = graphId_;
        }

        Address32 getNeighbor() const {
            return neighbor;
        }

        void setNeighbor(Address32 neighbor_) {
            neighbor = neighbor_;
        }
};

}

}

}

#endif /* NEIGHBORGRAPHADDEDOPERATION_H_ */
