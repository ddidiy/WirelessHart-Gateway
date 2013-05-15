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
 * ServiceAddedOperation.h
 *
 *  Created on: Sep 18, 2009
 *      Author: ioanpocol
 */

#ifndef SERVICEADDEDOPERATION_H_
#define SERVICEADDEDOPERATION_H_

#include "Model/Operations/EngineOperation.h"
#include "Model/Services/ServiceTypes.h"

#include <ApplicationLayer/Model/WirelessNetworkManagerCommands.h>

namespace NE {
namespace Model {
namespace Operations {

class ServiceAddedOperation: public NE::Model::Operations::EngineOperation {

    public:

        Uint32 serviceId;

        bool source;

        bool sink;

        bool intermittent;

        NE::Model::Services::ApplicationDomain::ApplicationDomainEnum applicationDomain;

        Address32 peerAddress32;

        Uint32 period;

        Uint8 routeId;

        C973_WriteService_RespCodes writeServiceResponsecode;

    public:

        ServiceAddedOperation(Address32 owner, ServiceId serviceId, bool source, bool sink, bool intermittent,
                    NE::Model::Services::ApplicationDomain::ApplicationDomainEnum applicationDomain, Address32 peerAddress32,
                    Uint32 period_, Uint8 routeId);

        virtual ~ServiceAddedOperation();

        ServiceId getServiceId();

        std::string getName() {
            return "Service Add";
        }

        void toStringInternal(std::ostringstream &stream);

        EngineOperationType::EngineOperationTypeEnum getOperationType();

        bool accept(NE::Model::Operations::IEngineOperationsVisitor& visitor);

        virtual void setResponseCode(Uint8 responseCode);

        virtual Uint8 getResponseCode();

};

}
}
}

#endif /* SERVICEADDEDOPERATION_H_ */
