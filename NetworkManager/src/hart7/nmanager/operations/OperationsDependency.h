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
 * OperationsDependency.h
 *
 *  Created on: Nov 9, 2009
 *      Author: andrei.petrut
 */

#ifndef OPERATIONSDEPENDENCY_H_
#define OPERATIONSDEPENDENCY_H_

#include <Model/Operations/IEngineOperationsVisitor.h>
#include <Model/Operations/EngineOperations.h>
#include <Model/Topology/Node.h>
#include <Model/NetworkEngine.h>

using namespace NE::Model::Operations;

namespace hart7 {
namespace nmanager {
namespace operations {

typedef std::multimap<EngineOperationType::EngineOperationTypeEnum, IEngineOperationPointer> OperationsTypeMap;
typedef std::multimap<Address32, IEngineOperationPointer> OperationsAddressMap;

/**
 * Creates dependencies between different operations, depending on what flow they are part of.
 * An operation can be sent into the field if it has no unconfirmed dependent operations, or all of the unconfirmed dependent operations
 * are already in the current packet.
 */
class OperationsDependency//: public NE::Model::Operations::IEngineOperationsVisitor
{
        LOG_DEF("h7.n.o.OperationsDependency");

    public:

        /**
         * Constructor. The input also contains information about the flow the operations are part of.
         */
        OperationsDependency(NE::Model::Operations::EngineOperations& operations);

        /**
         * Processes operations, and sets dependencies.
         */
        void ProcessDependencies();

    private:

        OperationsAddressMap queueByAddress;

        NE::Model::Topology::SubnetTopology& subnetTopology;

        void onJoinDevice();

        void onAllocateAdvertise();

        void onEvaluateNextPath();

        void onOutboundAttach();

        void onCheckRemovedDevices();

        Uint16 getChangeNotOpsByNots(EngineOperationsVector &foundOperations, Address32 address32, uint16_t notie,
                                     NE::Model::Operations::EngineOperations op);

        Uint16 getOperationsByTypeAndAddr(EngineOperationsVector &foundOperations, Address32 address32,
                                          EngineOperationType::EngineOperationTypeEnum opType,
                                          EngineOperationsVector op);

        Uint16 getOperationsByTypeAndAddrs(EngineOperationsVector &foundOperations, std::vector<Address32> address32,
                                           EngineOperationType::EngineOperationTypeEnum opType,
                                           NE::Model::Operations::EngineOperations op);

        Uint16 getOperationsByType(EngineOperationsVector &foundOperations,
                                   EngineOperationType::EngineOperationTypeEnum opType, EngineOperationsVector op);

        void inGeneral(EngineOperationsVector ops);

        void createDependencies(EngineOperationsVector lowPLevel, EngineOperationsVector highPLevel, bool separateUDP = false);

        void createDependenciesForNAOps(EngineOperationsVector ops);

        void createDependenciesForNROps(EngineOperationsVector ops);

        Uint16 findGraphID();

        void determineGraph(Address32 currentAddress, Address32 destination, Uint16 graphId, Uint16 searchId,
                            IEngineOperationPointer lastOp, EngineOperationsVector ops, bool dirFlag);

        void createOnGraphDependencies(Address32 currentAddress, Address32 destination, Uint16 graphId,
                                       Uint16 searchId, OperationsTypeMap lastOp);

         bool existsOperationWithOwner(std::set<NE::Model::Operations::IEngineOperationPointer>& operations,
                                      Address32 owner);

    private:

        NE::Model::Operations::EngineOperations& operations;

        uint8_t _maxPriority;

        std::map<NE::Model::Operations::EngineOperationType::EngineOperationTypeEnum, Uint8> operationsDependencies;
};

}
}
}

#endif /* OPERATIONSDEPENDENCY_H_ */
