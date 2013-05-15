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

#ifndef ENGINEOPERATIONS_H_
#define ENGINEOPERATIONS_H_

#include <map>
#include "Common/logging.h"
#include "Common/NEAddress.h"
#include "Model/Operations/IEngineOperation.h"

namespace NE {
namespace Model {
namespace Operations {

/**
 * @author Catalin Pop
 * @version 1.0
 */
class EngineOperations;
typedef boost::shared_ptr<EngineOperations> EngineOperationsListPointer;

typedef std::vector<NE::Model::Operations::IEngineOperationPointer> EngineOperationsVector;

std::string toString(const EngineOperationsVector& engineOperationsVector);

class EngineOperations {

    LOG_DEF("I.M.EngineOperations");

    private:

        static int nextSetIndex;

        int currentOperationsSetIndex;

        /**
         * The address64 that has been generated the event
         */
        Address64 requesterAddress64;

        /**
         * The assigned address16 of the address64 that has been generated the event
         */
        Address32 requesterAddress32;

        /**
         * The address of the device
         */
        Address32 proxyAddress32;

        /**
         *
         */
        bool shortAddress;

        /**
         *
         */
        bool proxyAddress;

        /**
         * The type of engine event this list of operations
         */
        NetworkEngineEventType::NetworkEngineEventTypeEnum eventEngineType;

        Uint16 graphId;

        /**
         * Error code
         */
        ErrorCode::ErrorCodeEnum errorCode;

    public:

        /**
         * The list of operations grouped by EUID
         */
        EngineOperationsVector operations;

        /**
         *
         */
        std::string reasonOfOperations;

    public:

        EngineOperations();
        EngineOperations(const EngineOperations& other);

        virtual ~EngineOperations();

        Address64& getRequesterAddress64();

        void setRequesterAddress64(const Address64& requesterAddress64);

        Address32 getRequesterAddress32();

        void setRequesterAddress32(Address32 requesterAddress32);

        Address32 getProxyAddress32();

        bool isProxyAddress();

        bool isShortAddress();

        void setProxyAddress32(Address32 proxyAddress32, bool isShortAddress);

        Uint16 getGraphId();

        void setGraphId(Uint16 graphId);

        void addOperation(const NE::Model::Operations::IEngineOperationPointer& operation);

        void addOperation(const NE::Model::Operations::IEngineOperationPointer& operation,
                    WaveDependency::WaveDependencyEnum operationDependency);

        EngineOperationsVector findOperations(Address32 address32);

        EngineOperationsVector& getEngineOperations();

        ErrorCode::ErrorCodeEnum getErrorCode();

        void setErrorCode(ErrorCode::ErrorCodeEnum errorCode);

        Uint16 getSize();

        int getOperationsSetIndex();

        void clear();

        void setNetworkEngineEventType(NetworkEngineEventType::NetworkEngineEventTypeEnum eventEngineType_) {
            this->eventEngineType = eventEngineType_;
        }

        NetworkEngineEventType::NetworkEngineEventTypeEnum getNetworkEngineEventType() {
            return eventEngineType;
        }

        void setJoinDependencies(Address32 address);

        void setAttachInDependencies(Address32 address, Address32 parent);

        void setAttachOutDependencies(Address32 address, Address32 parent);

        void setAdvertiseDependencies();

        void toShortString(std::ostringstream &stream);

        void toIndentString(std::ostringstream &stream);

        void toString(std::ostringstream &stream);
};

}
}
}

#endif /* ENGINEOPERATIONS_H_ */
