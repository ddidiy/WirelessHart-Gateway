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
 * WriteTimerIntervalOperation.h
 *
 *  Created on: Jun 8, 2010
 *      Author: radu
 */

#ifndef WRITETIMERINTERVALOPERATION_H_
#define WRITETIMERINTERVALOPERATION_H_

#include "Model/Operations/EngineOperation.h"
#include "Common/NETypes.h"
#include <ApplicationLayer/Model/DataLinkLayerCommands.h>
#include <ApplicationLayer/Model/CommonTables.h>

using namespace NE::Common;

namespace NE {
namespace Model {
namespace Operations {

class WriteTimerIntervalOperation: public NE::Model::Operations::EngineOperation {

    public:

        Uint32 timeIntervalMsecs;

        WirelessTimerCode timerCode;

        C795_WriteTimerInterval_RespCodes responseCode;

    public:

        WriteTimerIntervalOperation(Address32 owner, Uint32 timeIntervalMsecs, WirelessTimerCode timerCode);

        virtual ~WriteTimerIntervalOperation() {
        }

        void toStringInternal(std::ostringstream &stream);

        std::string getName() {
            return "Write Timer Interval";
        }

        NE::Model::Operations::EngineOperationType::EngineOperationTypeEnum getOperationType();

        bool accept(NE::Model::Operations::IEngineOperationsVisitor& visitor);

        virtual void setResponseCode(Uint8 responseCode);

        virtual Uint8 getResponseCode();
};

}
}
}

#endif /* WRITETIMERINTERVALOPERATION_H_ */
