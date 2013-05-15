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
 * WriteNetworkIDOperation.h
 *
 *  Created on: Jan 10, 2011
 *      Author: andy
 */

#ifndef WRITENETWORKIDOPERATION_H_
#define WRITENETWORKIDOPERATION_H_


#include "Model/Operations/EngineOperation.h"
#include <ApplicationLayer/Model/DataLinkLayerCommands.h>

namespace NE {
namespace Model {
namespace Operations {

class WriteNetworkIDOperation;
typedef boost::shared_ptr<WriteNetworkIDOperation> WriteNetworkIDOperationPointer;

class WriteNetworkIDOperation: public NE::Model::Operations::EngineOperation {

    private:

        uint16_t networkID;

        uint8_t responseCode;

    public:

        WriteNetworkIDOperation(Address32 owner_, uint16_t networkID);

        virtual ~WriteNetworkIDOperation();

        void toStringInternal(std::ostringstream &stream);

        EngineOperationType::EngineOperationTypeEnum getOperationType();

        bool accept(IEngineOperationsVisitor & visitor);

        virtual void setResponseCode(Uint8 responseCode);

        virtual Uint8 getResponseCode();

        std::string getName() {
            return "WriteNetworkID";
        }

        uint16_t getNetworkID() { return networkID; }
};

}

}

}

#endif /* WRITENETWORKIDOPERATION_H_ */
