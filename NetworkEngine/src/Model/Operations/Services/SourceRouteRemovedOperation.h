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
 * SourceRouteRemovedOperation.h
 *
 *  Created on: Oct 30, 2009
 *      Author: Andy
 */

#ifndef SOURCEROUTEREMOVEDOPERATION_H_
#define SOURCEROUTEREMOVEDOPERATION_H_

#include "Model/Operations/IEngineOperation.h"
#include "Model/Operations/EngineOperation.h"

#include <ApplicationLayer/Model/WirelessNetworkManagerCommands.h>

namespace NE {
namespace Model {
namespace Operations {

class SourceRouteRemovedOperation;
typedef boost::shared_ptr<SourceRouteRemovedOperation> SourceRouteRemovedOperationPointer;

/**
 * A RouteRemovedOperation is generated when a route is removed
 * @author Sorin Bidian
 * @version 1.0
 */

class SourceRouteRemovedOperation: public NE::Model::Operations::EngineOperation {

    public:

        Uint32 routeId;

        C977_DeleteSourceRoute_RespCodes deleteSourceRouteResponseCode;

    public:

        SourceRouteRemovedOperation(Address32 owner, Uint32 routeId);

        ~SourceRouteRemovedOperation();

        Uint32 getRouteId();

        std::string getName() {
            return "SourceRoute Removed";
        }

        void toStringInternal(std::ostringstream& stream);

        EngineOperationType::EngineOperationTypeEnum getOperationType();

        bool accept(NE::Model::Operations::IEngineOperationsVisitor& visitor);

        virtual void setResponseCode(Uint8 responseCode);

        virtual Uint8 getResponseCode();

};

}
}
}

#endif /* SOURCEROUTEREMOVEDOPERATION_H_ */
