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
 * ReadLinksOperation.h
 *
 *  Created on: May 17, 2010
 *      Author: andrei.petrut
 */

#ifndef READLINKSOPERATION_H_
#define READLINKSOPERATION_H_

#include "Model/Operations/EngineOperation.h"
#include "Model/Tdma/TdmaTypes.h"

#include <ApplicationLayer/Model/DataLinkLayerCommands.h>

namespace NE {
namespace Model {
namespace Operations {

class ReadLinksOperation: public NE::Model::Operations::EngineOperation {

    public:

        int startIndex;

        int count;

        C784_ReadLinkList_RespCodes responseCode;

    public:

        ReadLinksOperation(Address32 owner, int startIndex, int count);

        virtual ~ReadLinksOperation();

        std::string getName() {
            return "Get Links";
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


#endif /* READLINKSOPERATION_H_ */
