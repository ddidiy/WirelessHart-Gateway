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
 * ServiceRemovedOperation.h
 *
 *  Created on: Sep 18, 2009
 *      Author: ioanpocol
 */

#ifndef SERVICEREMOVEDOPERATION_H_
#define SERVICEREMOVEDOPERATION_H_

#include "Model/Operations/EngineOperation.h"
#include "Model/Services/ServiceTypes.h"

#include <ApplicationLayer/Model/NetworkLayerCommands.h>

namespace NE {
namespace Model {
namespace Operations {

/**
 * A ServiceRemovedOperation is generated when a service is terminated.
 */
class ServiceRemovedOperation: public NE::Model::Operations::EngineOperation {

    public:

        Address32 peerAddress;

        Uint32 serviceId;

        Uint32 routeId;

        NE::Model::Services::DeleteReason::DeleteReasonEnum deleteReason;

        C801_DeleteService_RespCodes responseCode;

    public:

        ServiceRemovedOperation(Address32 owner, Address32 peerAddress, Uint32 serviceId, Uint32 routeId,
                    NE::Model::Services::DeleteReason::DeleteReasonEnum deleteReason);

        virtual ~ServiceRemovedOperation();

        std::string getName() {
            return "Service Removed";
        }

        ServiceId getServiceId();

        void toStringInternal(std::ostringstream &stream);

        EngineOperationType::EngineOperationTypeEnum getOperationType();

        bool accept(NE::Model::Operations::IEngineOperationsVisitor& visitor);

        virtual void setResponseCode(Uint8 responseCode);

        virtual Uint8 getResponseCode();

};

}
}
}

#endif /* SERVICEREMOVEDOPERATION_H_ */
