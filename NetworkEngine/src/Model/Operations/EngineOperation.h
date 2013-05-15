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

/**
 * EngineOperation.h
 *
 *  Created on: Jul 15, 2008
 *      Author: ioan.pocol
 */

#ifndef ENGINEOPERATION_H_
#define ENGINEOPERATION_H_

#include "Model/Operations/IEngineOperation.h"
#include "Common/logging.h"

namespace NE {
namespace Model {
namespace Operations {

/**
 * @author Ioan Pocol
 * @version 1.0
 */
class EngineOperation: public IEngineOperation {

    protected:

        LOG_DEF("I.M.O.EngineOperation");

        /**
         * RequestID(ISA)/Handle(WH), used for request identification
         */
        Uint16 requestID;

        /**
         * The address where the operation is sent
         */
        Address32 owner;

        /**
         * The operation status
         */
        OperationState::OperationStateEnum state;

        /**
         * The operations are ordered by the dependency, an operation can be sent
         * only if all operations with higher dependencies has been applied (confirmed)
         */
        WaveDependency::WaveDependencyEnum operationDependency;

        /**
         * The time when the operation has been sent on the network
         */
        time_t sentTimestamp;

        /**
         * The time when the operation has been applied(confirmed)
         */
        time_t confirmTimestamp;

        /**
         * The confirmation error code
         */
        ErrorCode::ErrorCodeEnum errorCode;

        /**
         * The generated operation ID
         */
        Uint16 operationId;

        /**
         * Contains the operations on which this operation depends on.
         */
        std::list<IEngineOperationPointer> operationsDependencies;

    public:

        EngineOperation();

        virtual ~EngineOperation();

        Uint16 getOperationId();

        Uint16 getProxyAddress();

        Uint16 getRequestID();

        Uint16 getSubnetId();

        void setRequestID(Uint16 requestID);

        Address32 getOwner() const;

        Uint16 getOwnerAddress16() const;

        OperationState::OperationStateEnum getState();

        void setState(OperationState::OperationStateEnum operationState);

        virtual WaveDependency::WaveDependencyEnum getDependency();

        virtual void setDependency(WaveDependency::WaveDependencyEnum operationDependency);

        std::list<IEngineOperationPointer>& getOperationsDependencies();

        void setOperationDependency(IEngineOperationPointer operationDependency, bool allowDependencyOnHigherWave = false);

        std::string getStateDecription();

        time_t getSentTimestamp();

        void setSentTimestamp(time_t sentTimestamp_);

        time_t getConfirmTimestamp();

        void setConfirmTimestamp(time_t confirmTimestamp);

        Uint8 getNrOfRetry();

        void setNrOfRetry(Uint8 nrOfRetry);

        ErrorCode::ErrorCodeEnum getErrorCode();

        void setErrorCode(ErrorCode::ErrorCodeEnum errorCode);

        std::string errorCodeToString(ErrorCode::ErrorCodeEnum& ec);

        void toString(std::ostringstream& stream);

    protected:

        void toStringCommonOperationState(std::ostringstream& stream);

        void toStringDependencies(std::ostringstream& stream);

        virtual void toStringInternal(std::ostringstream& stream) = 0;
};

}
}
}
#endif /* ENGINEOPERATION_H_ */
